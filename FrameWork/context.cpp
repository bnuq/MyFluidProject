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
    // 1. position, velocity, range, density, pressure 계산
    FluidDensityPressure = Shader::CreateFromFile("./shader/FluidDensityPressure.compute", GL_COMPUTE_SHADER);
    DensityPressureCompute = Program::Create({ FluidDensityPressure });
    if(!DensityPressureCompute) return false;



    // 2. force, 알짜힘 계산
    FluidForce = Shader::CreateFromFile("./shader/FluidForce.compute", GL_COMPUTE_SHADER);
    ForceCompute = Program::Create({ FluidForce });
    if(!ForceCompute) return false;



    // 3. Particle 을 이동시킴, new position, new velocity, new toCamera
    FluidMove = Shader::CreateFromFile("./shader/FluidMove.compute", GL_COMPUTE_SHADER);
    MoveCompute = Program::Create({ FluidMove });
    if(!MoveCompute) return false;



    // 4. Shadow Map 에 Particles 를 무조건 그린다 => 그려지는 깊이를 먼저 기록
    MakeDepthMap = Program::Create("./shader/Simple/simple.vs", "./shader/Simple/simple.fs");
    if(!MakeDepthMap) return false;

        // shadow map
        depthFrameBuffer = ShadowMap::Create(1024, 1024);



    // 5. 실제 색깔을 렌더링, depth map 을 참고해서 나타낼 픽셀을 찾는다
    DrawProgram = Program::Create("./shader/Fluid/fluid.vs", "./shader/Fluid/fluid.fs");
    if(!DrawProgram) return false;






    // 카메라 객체 초기화
    MainCam = Camera::Create();
    
    // Particle => Box 로 표현하자
    BoxMesh = Mesh::CreateBox();


    


    // Core Particles 의 초기 정보를 초기화 한다
    // 그리고 Core Particles 를 정렬
    Init_CoreParticles();





    // GPU 에서 데이터를 저장할 SSBO 버퍼를 생성 및 할당
    // Core Particle => 데이터를 복사해서 GPU 에 넣는다
    // 정렬된 데이터를 넣는다
    CoreParticleBuffer
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, CoreParticleArray.data(), sizeof(CoreParticle), CoreParticleArray.size());


    // Particle      => 복사할 데이터는 없고, GPU 내에 공간만 할당해 놓는다
    ParticleBuffer 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NULL, sizeof(Particle), CoreParticleArray.size());


    
    /* SSBO 버퍼를 각 인덱스에 바인딩 */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CoreParticle_Index, CoreParticleBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Particle_Index,     ParticleBuffer->Get());

    
    
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


    // 4. Make Depth Map    => Core Particle 사용
        blockIndex = glGetProgramResourceIndex(MakeDepthMap->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(MakeDepthMap->Get(), blockIndex, CoreParticle_Index);


    // 5. Rendering         => Core Particle 사용
        blockIndex = glGetProgramResourceIndex(DrawProgram->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(DrawProgram->Get(), blockIndex, CoreParticle_Index);



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
                        glm::vec3(xStride * xCount, yStride * yCount, zStride * zCount)
                    )
                );
            }
        }
    }


    // 카메라까지의 거리를 기준으로 정렬한다 => 멀리 있는 것이 앞으로 온다
    // 처음부터 아예 정렬을 하고 시작
    std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_Compare());
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
        // 1. Smmoth Kernel
            ImGui::DragFloat("Smooth Kernel Radius", &SmoothKernelRadius, 0.01f, 0.01f);

        // Particle Data
            ImGui::DragFloat("particle mass", &Particle::ParticleMass, 0.01f, 0.0f);

        // 2. Pressure
            ImGui::DragFloat("Gas Coeffi", &PD.gasCoeffi, 1.0f, 0);
            ImGui::DragFloat("Gas RestDensity", &PD.restDensity, 0.001f, 0);

        // 3. Viscosity
            ImGui::DragFloat("viscosity", &viscosity);

        // 4. Surface
            ImGui::DragFloat("surfThreshold", &surfThreshold, 0.001f, 0.0f);
            ImGui::DragFloat("surfCoeffi", &surfCoeffi, 0.01f, 0.0f);

        // 5. Gravity
            ImGui::DragFloat4("gravityAcel", glm::value_ptr(gravityAcel));

        // 6. Wave Poser
            ImGui::DragFloat("wavePower", &wavePower, 1.0f, 0);

        // 7. Delta Time
            ImGui::DragFloat("deltaTime", &deltaTime);

        // 8. Collision
            ImGui::DragFloat("damping", &damping);

        // 9. Renderings
            ImGui::DragFloat("offset", &offset, 0.001f, 0);

        // 10. Alpha Offset
            ImGui::DragFloat("alphaOffset", &alphaOffset, 0.01f, 0.0f, 1.0f);
    }

    // 깊이 맵 확인
    ImGui::Image((ImTextureID)depthFrameBuffer->GetShadowMap()->Get(), ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::DragFloat("cameraNear", &cameraNear, 1.0f, 0);
    ImGui::DragFloat("cameraFar", &cameraFar, 1.0f, 0);



    ImGui::DragFloat3("Camera Position", glm::value_ptr(MainCam->Position));

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
        MainCam->Position,
        MainCam->Position + MainCam->DirVec,
        MainCam->UpVec
    );




    // 프로그램 실행
    Get_Density_Pressure();                             // 1. 먼저 입자의 밀도와 압력 값들을 구하고
    Get_Force();                                        // 2. 각 입자들의 알짜힘을 구한다

    Get_Move();                                         // 3. 알짜힘에 맞춰 이동 -> 최종 위치와 속도를 결정

    Make_Depth_Map(projection, view);                   // 4. 일단 Depth map 에 렌더링 => Depth Map 을 생성하고
    Draw_Particles(projection, view);                   // 5. 최종적으로 visible particle 들만 렌더링
}




// Core Particle 을 input 으로 사용 => 무조건 여기서 값을 꺼내와서 사용한다
// Particle 은 output => 무조건 이곳에 저장한다
void Context::Get_Density_Pressure()
{
    DensityPressureCompute->Use();
        
        // Smmoth Kernel
            DensityPressureCompute->SetUniform("h", SmoothKernelRadius);

        // Particle Data
            DensityPressureCompute->SetUniform("particleMass", Particle::ParticleMass);
            DensityPressureCompute->SetUniform("TotalParticleCount", Particle::TotalParticleCount);

        // Pressure Data
            DensityPressureCompute->SetUniform("gasCoeffi", PD.gasCoeffi);
            DensityPressureCompute->SetUniform("restDensity", PD.restDensity);

        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


    glUseProgram(0);
}



// 계산이 완료된 Particle 데이터를 input 으로 사용
// 채워지지 않은 Particle 데이터를 output 으로 채운다
void Context::Get_Force()
{
    ForceCompute->Use();

        // Smmoth Kernel
            ForceCompute->SetUniform("h", SmoothKernelRadius);

        // Particle Data
            ForceCompute->SetUniform("particleMass", Particle::ParticleMass);
            ForceCompute->SetUniform("TotalParticleCount", Particle::TotalParticleCount);
            ForceCompute->SetUniform("xRange", Particle::FluidRange.x);
            ForceCompute->SetUniform("yRange", Particle::FluidRange.y);
        
        // Viscosity
            ForceCompute->SetUniform("viscosity", viscosity);

        // Surface
            ForceCompute->SetUniform("surfThreshold", surfThreshold);
            ForceCompute->SetUniform("surfCoeffi", surfCoeffi);

        // Gravity
            ForceCompute->SetUniform("gravityAcel", gravityAcel);

        // Mouse Force
        // 마우스 좌클릭이 없는데, 힘이 있을 때, 힘을 넘기고 힘 초기화
            if(!m_giveForceMouse && (glm::length(mouseForce) != 0))
            {
                ForceCompute->SetUniform("mouseForce", mouseForce);
                mouseForce = glm::vec2(0);
            }
            else
                ForceCompute->SetUniform("mouseForce", glm::vec2(0));

        // Wave Force
            ForceCompute->SetUniform("wavePower", wavePower);

        // Total Time
        ForceCompute->SetUniform("time", (float)glfwGetTime());


        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


    glUseProgram(0);
}


void Context::Get_Move()
{
    MoveCompute->Use();

        // Particle Data
            MoveCompute->SetUniform("TotalParticleCount", Particle::TotalParticleCount);
            MoveCompute->SetUniform("LimitRange", Particle::FluidRange);

        // Delta Time
            MoveCompute->SetUniform("deltaTime", deltaTime);

        // Collision
            MoveCompute->SetUniform("damping", damping);


        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        
        // 새롭게 계산한 Core Particle 데이터 => 카메라까지 거리에 맞춰서 정렬을 다시 해준다
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CoreParticleBuffer->Get());

            // 연산 결과, Output 을 CPU 로 읽어오고
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());

            // Camera 까지의 거리를 기준으로 sort
            std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_Compare());

            // 다시 넣어준다 => 다음 프레임 때 input 으로 사용한다
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(0);
}




void Context::Make_Depth_Map(const glm::mat4& projection, const glm::mat4& view)
{
    // 화면에 그리지 않고, shadow map 에 그려서, 카메라에서 봤을 때 각 픽셀 depth 값을 텍스처에 저장한다
    depthFrameBuffer->Bind();
        
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport  // view port 설정
        (
            0, 
            0,
            depthFrameBuffer->GetShadowMap()->GetWidth(),
            depthFrameBuffer->GetShadowMap()->GetHeight()
        );

        MakeDepthMap->Use();

            // vertex shader
                MakeDepthMap->SetUniform("transform", projection * view);

            BoxMesh->GPUInstancingDraw(MakeDepthMap.get(), Particle::TotalParticleCount);

        glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, m_width, m_height);
}





void Context::Draw_Particles(const glm::mat4& projection, const glm::mat4& view)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Particle Buffer 에 저장된 데이터를 이용해서, Particles 를 그린다
    DrawProgram->Use();
        
        // vertex shader
            DrawProgram->SetUniform("transform", projection * view);
            DrawProgram->SetUniform("CameraPos", MainCam->Position);

        // fragment shader
            glActiveTexture(GL_TEXTURE0);
            depthFrameBuffer->GetShadowMap()->Bind();
            DrawProgram->SetUniform("depthMap", 0);

            DrawProgram->SetUniform("offset", offset);
            DrawProgram->SetUniform("alphaOffset", alphaOffset);           
            DrawProgram->SetUniform("CameraDir", MainCam->DirVec);
            
        BoxMesh->GPUInstancingDraw(DrawProgram.get(), Particle::TotalParticleCount);

    glUseProgram(0);

    glDisable(GL_BLEND);
}