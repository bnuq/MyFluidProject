#include "context.h"

#include <vector>
#include <string>
#include <algorithm>


ContextUPtr Context::Create()                                  
{
    auto context = ContextUPtr(new Context());
    if (!context->Init())
        return nullptr;
    return std::move(context);
}


bool Context::Init()
{
    glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);


    /* Initialize Programs */
    // 1. position, velocity, range, density, pressure, neighbor
    FluidDensityPressure = Shader::CreateFromFile("./shader/FluidDensityPressure.compute", GL_COMPUTE_SHADER);
    DensityPressureCompute = Program::Create({ FluidDensityPressure });
    if(!DensityPressureCompute) return false;



    // 2. force
    FluidForce = Shader::CreateFromFile("./shader/FluidForce.compute", GL_COMPUTE_SHADER);
    ForceCompute = Program::Create({ FluidForce });
    if(!ForceCompute) return false;



    // 3. new position, new velocity, new toCamera
    FluidMove = Shader::CreateFromFile("./shader/FluidMove.compute", GL_COMPUTE_SHADER);
    MoveCompute = Program::Create({ FluidMove });
    if(!MoveCompute) return false;



    // 4. new visible
    FluidVisible = Shader::CreateFromFile("./shader/FluidVisible.compute", GL_COMPUTE_SHADER);
    VisibleCompute = Program::Create({ FluidVisible });
    if(!VisibleCompute) return false;



    // 5. Fluid Rendering
    DrawProgram = Program::Create("./shader/fluid.vs", "./shader/fluid.fs");
    if(!DrawProgram) return false;



    // 6. For GPU Instancing
    SimpelProgram = Program::Create("./shader/Simple/simple.vs", "./shader/Simple/simple.fs");
    if(!SimpelProgram) return false;


    // shadow map
    depthFrameBuffer = ShadowMap::Create(1024, 1024);



    // 카메라 객체 초기화
    MainCam = Camera::Create();
    
    // Particle => Box 로 표현하자
    BoxMesh = Mesh::CreateBox();



    // Particle 들의 초기 정보를 초기화 한다
    Init_CoreParticles();



    // GPU 에서 데이터를 저장할 SSBO 버퍼를 생성
    // Core Particle => 데이터를 복사해서 GPU 에 넣는다
    // 정렬된 데이터를 넣는다
    CoreParticleBuffer
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, CoreParticleArray.data(), sizeof(CoreParticle), CoreParticleArray.size());

    // Particle      => 복사할 데이터는 없고, GPU 내에 공간만 할당해 놓는다
    ParticleBuffer 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NULL, sizeof(Particle), CoreParticleArray.size());

    CountBuffer 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, &visibleCount, sizeof(unsigned int), 1);


    
    /* SSBO 버퍼를 각 인덱스에 바인딩 */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CoreParticle_Index, CoreParticleBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Particle_Index,     ParticleBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Count_Index,        CountBuffer->Get());

    
    
    /* 컴퓨트 프로그램과 SSBO 를 연결 */
    GLuint blockIndex{};

    // 1. Density, Pressure
        blockIndex = glGetProgramResourceIndex(DensityPressureCompute->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(DensityPressureCompute->Get(), blockIndex, CoreParticle_Index);
        blockIndex = glGetProgramResourceIndex(DensityPressureCompute->Get(), GL_SHADER_STORAGE_BUFFER, "ParticleBuffer");
        glShaderStorageBlockBinding(DensityPressureCompute->Get(), blockIndex, Particle_Index);


    // 2. Force
        blockIndex = glGetProgramResourceIndex(ForceCompute->Get(), GL_SHADER_STORAGE_BUFFER, "ParticleBuffer");
        glShaderStorageBlockBinding(ForceCompute->Get(), blockIndex, Particle_Index);
    

    // 3. Move
        blockIndex = glGetProgramResourceIndex(MoveCompute->Get(), GL_SHADER_STORAGE_BUFFER, "ParticleBuffer");
        glShaderStorageBlockBinding(MoveCompute->Get(), blockIndex, Particle_Index);
        blockIndex = glGetProgramResourceIndex(MoveCompute->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(MoveCompute->Get(), blockIndex, CoreParticle_Index);


    // 3.5 GPU Instancing
        blockIndex = glGetProgramResourceIndex(SimpelProgram->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(SimpelProgram->Get(), blockIndex, CoreParticle_Index);




    // 4. Visible
        blockIndex = glGetProgramResourceIndex(VisibleCompute->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(VisibleCompute->Get(), blockIndex, CoreParticle_Index);
        blockIndex = glGetProgramResourceIndex(VisibleCompute->Get(), GL_SHADER_STORAGE_BUFFER, "CountBuffer");
        glShaderStorageBlockBinding(VisibleCompute->Get(), blockIndex, Count_Index);




    // 1. density pressure 확정
    {
        DensityPressureCompute->Use();

            // Set Uniforms
            DensityPressureCompute->SetUniform("h", SmoothKernelRadius);
            DensityPressureCompute->SetUniform("hSquare", SmoothKernelRadius * SmoothKernelRadius);

            DensityPressureCompute->SetUniform("particleMass", Particle::ParticleMass);
            DensityPressureCompute->SetUniform("TotalParticleCount", Particle::TotalParticleCount);

            DensityPressureCompute->SetUniform("gasCoeffi", gas.gasCoeffi);
            DensityPressureCompute->SetUniform("gasCoeffi", gas.restDensity);

            glDispatchCompute(Particle::GroupNum, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            // output 인 Particle data 를 확인
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ParticleBuffer->Get());
                glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());

                SPDLOG_INFO("Density Pressure Compute");
                // 일단 로그로 확인하자
                for(unsigned int i = 0; i < Particle::TotalParticleCount; i++)
                {
                    SPDLOG_INFO("{} th position {}, {}, {}, {}", i, ParticleArray[i].position.x, ParticleArray[i].position.y, ParticleArray[i].position.z, ParticleArray[i].position.w);

                    SPDLOG_INFO("{} th velocity {}, {}, {}, {}", i, ParticleArray[i].velocity.x, ParticleArray[i].velocity.y, ParticleArray[i].velocity.z, ParticleArray[i].velocity.w);

                    SPDLOG_INFO("{} th range {} {}", i, ParticleArray[i].range.x, ParticleArray[i].range.y);

                    SPDLOG_INFO("{} th density {}", i, ParticleArray[i].density);

                    SPDLOG_INFO("{} th pressure {}", i, ParticleArray[i].pressure);

                    SPDLOG_INFO("{} th neighbor {}", i, ParticleArray[i].neighbor);

                    SPDLOG_INFO("*** *** *** ***");
                }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glUseProgram(0);
    }


    // 2. force 확정
    {
        ForceCompute->Use();
            // Set Uniforms
            // 0
            ForceCompute->SetUniform("h", SmoothKernelRadius);
            ForceCompute->SetUniform("hSquare", SmoothKernelRadius * SmoothKernelRadius);

            ForceCompute->SetUniform("particleMass", Particle::ParticleMass);
            ForceCompute->SetUniform("particleCount", Particle::TotalParticleCount);
            
            // 1
            ForceCompute->SetUniform("viscosity", viscosity);

            ForceCompute->SetUniform("threshold", threshold);
            ForceCompute->SetUniform("surfCoeffi", surfCoeffi);

            // 2
            ForceCompute->SetUniform("gravityAcel", gravityAcel);


            glDispatchCompute(Particle::GroupNum, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


            // particle buffer data
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ParticleBuffer->Get());
                glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());


                SPDLOG_INFO("Surface Force Compute");
                // 일단 로그로 확인하자
                for(unsigned int i = 0; i < Particle::TotalParticleCount; i++)
                {
                    SPDLOG_INFO("{} th force {}, {}, {}", i, ParticleArray[i].force.x, ParticleArray[i].force.y, ParticleArray[i].force.z);

                    // SPDLOG_INFO("{} th surfNormal {}, {}, {}", i, ParticleArray[i].surfNormal.x, ParticleArray[i].surfNormal.y, ParticleArray[i].surfNormal.z);

                    // SPDLOG_INFO("{} th surf force mag {}", i, ParticleArray[i].surfForceMag);

                    SPDLOG_INFO("--- --- --- ---");
                }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glUseProgram(0);
    }


    return true;
}




// 정해진 Range 에 Particles 를 집어 넣는다
void Context::Init_CoreParticles()
{
    // Particle::Fluid Range 를 균등하게 나눈다, Particle 들이 들어갈 수 있도록
    float xStride = Particle::FluidRange.x / (Particle::ParticleCount.x + 1);
    float yStride = Particle::FluidRange.y / (Particle::ParticleCount.y + 1);
    float zStride = Particle::FluidRange.z / (Particle::ParticleCount.z + 1);


    for(unsigned int xCount = 1; xCount <= Particle::ParticleCount.x; xCount++)
    {
        for(unsigned int yCount = 1; yCount <= Particle::ParticleCount.y; yCount++)
        {
            for(unsigned int zCount = 1; zCount <= Particle::ParticleCount.z; zCount++)
            {
                // Core Particle Initialize
                CoreParticleArray.push_back
                ( 
                    CoreParticle
                    (
                        glm::vec4(xStride * xCount, yStride * yCount, zStride * zCount, 1.0f),
                        MainCam->Position
                    )
                );
            }
        }
    }


    // 카메라까지의 거리를 기준으로 정렬한다 => 멀리 있는 것이 앞으로 온다
    // 처음부터 아예 정렬을 하고 시작
    std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_toCamera_Compare());



    ParticleArray.resize(CoreParticleArray.size());
}


void Context::ProcessInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_W);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_S);

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_A);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_D);

    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_Q);
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_E);

}
void Context::Reshape(int width, int height)
{
    m_width = width;
    m_height = height;

    glViewport(0, 0, m_width, m_height);

    //m_framebuffer = Framebuffer::Create(Texture::Create(m_width, m_height, GL_RGBA));
}
void Context::MouseMove(double x, double y)
{
    if(m_cameraControl)
    {
        auto pos = glm::vec2((float)x, (float)y);
        auto deltaPos = pos - m_prevMousePos;

        MainCam->Rotate(deltaPos);
        m_prevMousePos = pos;
    }

    if(m_giveForceMouse)
    {
        auto pos = glm::vec2((float)x, (float)y);
        auto deltaPos = pos - m_giveForceMousePos;

        mouseForce += deltaPos;
        m_giveForceMousePos = pos;
    }
}
void Context::MouseButton(int button, int action, double x, double y)
{
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)  // 중간 키 클릭
    {
        if (action == GLFW_PRESS)
        {
            // 마우스 조작 시작 시점에 현재 마우스 커서 위치 저장
            m_giveForceMousePos = glm::vec2((float)x, (float)y);
            m_giveForceMouse = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_giveForceMouse = false;
        }
    }


    if (button == GLFW_MOUSE_BUTTON_RIGHT)  // 우측 키 클릭
    {
        if (action == GLFW_PRESS)
        {
            // 마우스 조작 시작 시점에 현재 마우스 커서 위치 저장
            m_prevMousePos = glm::vec2((float)x, (float)y);
            m_cameraControl = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_cameraControl = false;
        }
    }
}





void Context::Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);


    if(ImGui::Begin("Smooth Kernel"))
    {
        ImGui::DragFloat("Smooth Kernel Radius", &SmoothKernelRadius, 0.001f, 0.01f);


        ImGui::DragFloat("Gas Coeffi", &gas.gasCoeffi);
        ImGui::DragFloat("Gas RestDensity", &gas.restDensity);

        ImGui::DragFloat("viscosity", &viscosity);
        //ImGui::DragFloat("surfCoeffi", &forvar.surfCoeffi);
        //ImGui::DragFloat("surfForceThreshold", &forvar.surfForceThreshold);

        ImGui::DragFloat4("gravityAcel", glm::value_ptr(gravityAcel));

        ImGui::DragFloat("deltaTime", &deltaTime);
        ImGui::DragFloat("damping", &damping);

        ImGui::DragFloat("threshold", &threshold, 0.001f, 0.0f);

        
        ImGui::DragFloat("neighborLevel", &neighborLevel, 0.001, 0);
        ImGui::DragFloat("correction", &correction, 0.0001, 0);

        ImGui::DragFloat("visible coeffi", &visibleCoeffi, 0.0001, 1);
        ImGui::DragFloat("visible thre", &visibleThre, 0.0001, 1);


        ImGui::DragFloat("controlValue", &controlValue, 0.0001, 3);
        ImGui::DragFloat("ratio", &ratio, 0.0001, 1);


        ImGui::Text("Visible Count is %d", visibleCount);
    }

    // 깊이 맵 확인
    ImGui::Image((ImTextureID)depthFrameBuffer->GetShadowMap()->Get(), ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::DragFloat("cameraNear", &cameraNear, 0.01f, 0);
    ImGui::DragFloat("cameraFar", &cameraFar, 0.01f, 0);

    ImGui::DragFloat("offset", &offset, 0.01f, 0);

    ImGui::End();



    // 카메라 설정, 위치 등을 확정
    auto projection = glm::perspective
    (
        glm::radians(45.0f),
        (float)m_width / (float)m_height,
        cameraNear, cameraFar
    );

    auto view = glm::lookAt
    (
        //glm::vec3(0, 0, -5.0f),
        MainCam->Position,
        //glm::vec3(0.0f, 0.0f, 0.0f),
        MainCam->Position + MainCam->DirVec,
        MainCam->UpVec
    );




    // 프로그램 실행
    Get_Density_Pressure();                             // 1. 먼저 입자의 밀도와 압력 값들을 구하고
    Get_Force();                                        // 2. 각 입자들의 알짜힘을 구한다
    Get_Move();                                         // 3. 알짜힘에 맞춰 이동 -> 최종 위치와 속도를 결정


    // SimpelProgram->Use();

    //     SimpelProgram->SetUniform("transform", projection * view);
    //     SimpelProgram->SetUniform("MainColor", glm::vec3(1,1,0));

    //     BoxMesh->GPUInstancingDraw(SimpelProgram.get(), Particle::TotalParticleCount);

    // glUseProgram(0);
    Draw_GPU_Instancing(projection, view);              // 4. 일단 depth map 에 렌더링을 시행



    //Find_Visible(projection, view);                                     // 5. depth map 을 읽고 ~ 스레드가 담당하는 타일이 depth map 에 저장된 깊이와
                                                        // 얼마나 차이가 나는 지를 판단한다


    // 렌더링을 위한 설정
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
    //     Draw_Particles(projection, view);               // 6. 최종적으로 visible particle 들만 렌더링
    // glDisable(GL_BLEND);


    
}




// Program 실행
void Context::Get_Density_Pressure()
{
    DensityPressureCompute->Use();
        // Set Uniforms
        DensityPressureCompute->SetUniform("h", SmoothKernelRadius);
        DensityPressureCompute->SetUniform("hSquare", SmoothKernelRadius * SmoothKernelRadius);

        DensityPressureCompute->SetUniform("particleMass", Particle::ParticleMass);
        DensityPressureCompute->SetUniform("TotalParticleCount", Particle::TotalParticleCount);

        DensityPressureCompute->SetUniform("gasCoeffi", gas.gasCoeffi);
        DensityPressureCompute->SetUniform("gasCoeffi", gas.restDensity);

        DensityPressureCompute->SetUniform("correction", correction);

        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


        // // output 인 Particle data 를 확인
        //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, ParticleBuffer->Get());
        //         glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());

        //         SPDLOG_INFO("Density Pressure Compute");
        //         // 일단 로그로 확인하자
        //         for(unsigned int i = 0; i < Particle::TotalParticleCount; i++)
        //         {
        //             SPDLOG_INFO("{} th neighbor {}", i, ParticleArray[i].neighbor);
        //             SPDLOG_INFO("*** *** *** ***");
        //         }

        //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUseProgram(0);
}


void Context::Get_Force()
{
    ForceCompute->Use();
        // Set Uniforms
        // 0
        ForceCompute->SetUniform("h", SmoothKernelRadius);
        ForceCompute->SetUniform("hSquare", SmoothKernelRadius * SmoothKernelRadius);

        ForceCompute->SetUniform("particleMass", Particle::ParticleMass);
        ForceCompute->SetUniform("particleCount", Particle::TotalParticleCount);
        
        // 1
        ForceCompute->SetUniform("viscosity", viscosity);

        ForceCompute->SetUniform("threshold", threshold);
        ForceCompute->SetUniform("surfCoeffi", surfCoeffi);

        // 2
        ForceCompute->SetUniform("gravityAcel", gravityAcel);

        ForceCompute->SetUniform("xRange", Particle::FluidRange.x);
        ForceCompute->SetUniform("yRange", Particle::FluidRange.y);
        ForceCompute->SetUniform("time", (float)glfwGetTime());
        ForceCompute->SetUniform("wavePower", 3.0f);

        

        // 마우스 좌클릭이 없는데, 힘이 있을 때, 힘을 넘기고 힘 초기화
        if(!m_giveForceMouse && (glm::length(mouseForce) != 0))
        {
            ForceCompute->SetUniform("mouseForce", mouseForce);
            mouseForce = glm::vec2(0);
        }
        else
            ForceCompute->SetUniform("mouseForce", glm::vec2(0));


        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glUseProgram(0);
}


void Context::Get_Move()
{
    MoveCompute->Use();
        /* Uniform Variable 넣기 */
        MoveCompute->SetUniform("deltaTime", deltaTime);
        MoveCompute->SetUniform("LimitRange", Particle::FluidRange);
        MoveCompute->SetUniform("damping", damping);

        MoveCompute->SetUniform("threshold", threshold);


        //if(tempNei >= 0) neighborLevel = (unsigned int)tempNei;
        MoveCompute->SetUniform("neighborLevel", neighborLevel);
        
        // 현재 카메라의 위치를 Uniform 으로 설정
        // Particle 이 최종적으로 움직인 위치에서 카메라까지의 거리를 구해야 한다
        MoveCompute->SetUniform("CameraPos", MainCam->Position);


        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        
        // 움직임 => 새로운 위치를 얻고나서, toCamera 값이 바뀌었으므로 정렬을 새로 해준다
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CoreParticleBuffer->Get());
            // 연산 결과, Output 을 CPU 로 읽어오고
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());

            // Camera 까지의 거리를 기준으로 sort
            std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_toCamera_Compare());

            // 다시 넣어준다
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(0);
}


void Context::Draw_GPU_Instancing(const glm::mat4& projection, const glm::mat4& view)
{
    // 화면에 그리지 않고, shadow map 에 그려서, 카메라에서 봤을 때 각 픽셀 depth 값을 텍스처에 저장한다
    depthFrameBuffer->Bind();
        
        glClear(GL_DEPTH_BUFFER_BIT);

        // view port 설정
        glViewport
        (
            0, 
            0,
            depthFrameBuffer->GetShadowMap()->GetWidth(),
            depthFrameBuffer->GetShadowMap()->GetHeight()
        );

        SimpelProgram->Use();

            SimpelProgram->SetUniform("transform", projection * view);
            SimpelProgram->SetUniform("MainColor", glm::vec3(1,1,0));

            BoxMesh->GPUInstancingDraw(SimpelProgram.get(), Particle::TotalParticleCount);

        glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_width, m_height);
}


void Context::Find_Visible(const glm::mat4& projection, const glm::mat4& view)
{
    VisibleCompute->Use();
        // VisibleCompute->SetUniform("h", SmoothKernelRadius);
        // VisibleCompute->SetUniform("TotalParticleCount", Particle::TotalParticleCount);

        // VisibleCompute->SetUniform("n", visibleCoeffi);
        // VisibleCompute->SetUniform("visibleThre", visibleThre);

        // VisibleCompute->SetUniform("camPos", MainCam->Position);
        // VisibleCompute->SetUniform("controlValue", controlValue);
        // VisibleCompute->SetUniform("ratio", ratio);


        // depth map 을 uniform variable 로 넘긴다
        glActiveTexture(GL_TEXTURE0);
        depthFrameBuffer->GetShadowMap()->Bind();
        VisibleCompute->SetUniform("depthMap", 0);

        VisibleCompute->SetUniform("transform", projection * view);
        VisibleCompute->SetUniform("offset", offset);


        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // 렌더링하기 위해 Core Particle 데이터를 읽어온다
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CoreParticleBuffer->Get());
            // 연산 결과, Output 을 CPU 로 읽어오고
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());

            // Camera + visible 을 기준으로 sort
            std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_visible_Compare());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CountBuffer->Get());
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &visibleCount);

            //SPDLOG_INFO("visible count {}", visibleCount);

            unsigned int temp = 0;
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &temp);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUseProgram(0);
}



void Context::Draw_Particles(const glm::mat4& projection, const glm::mat4& view)
{
    // output 버퍼에 저장된 데이터를 이용해서, Particles 를 그린다
    DrawProgram->Use();
  
        // 렌더링 하기 전에, visible 유무로 정렬을 추가로 진행한다
        //std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_visible_Compare());

        // 정렬된 Core Particle 데이터를 이용해서, 카메라에서 가까운 것 부터 렌더링을 진행
        // surface count 개수 만큼만 그린다
        for(unsigned int i = 0; i < visibleCount; i++)
        //for(unsigned int i = 0; i < Particle::TotalParticleCount; i++)
        {
            auto ParticlePos = glm::vec3(
                    CoreParticleArray[i].xpos, CoreParticleArray[i].ypos, CoreParticleArray[i].zpos
                );

            auto modelTransform = glm::translate(glm::mat4(1.0f), ParticlePos);
            auto transform = projection * view * modelTransform;

            DrawProgram->SetUniform("transform", transform);
            DrawProgram->SetUniform("ViewVec", glm::normalize(MainCam->Position - ParticlePos));


            BoxMesh->Draw(DrawProgram.get());
        }

    glUseProgram(0);
}



