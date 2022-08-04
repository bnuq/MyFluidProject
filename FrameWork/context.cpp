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
    // 1. position, velocity, range, density, pressure ���
    FluidDensityPressure = Shader::CreateFromFile("./shader/FluidDensityPressure.compute", GL_COMPUTE_SHADER);
    DensityPressureCompute = Program::Create({ FluidDensityPressure });
    if(!DensityPressureCompute) return false;



    // 2. force, ��¥�� ���
    FluidForce = Shader::CreateFromFile("./shader/FluidForce.compute", GL_COMPUTE_SHADER);
    ForceCompute = Program::Create({ FluidForce });
    if(!ForceCompute) return false;



    // 3. Particle �� �̵���Ŵ, new position, new velocity, new toCamera
    FluidMove = Shader::CreateFromFile("./shader/FluidMove.compute", GL_COMPUTE_SHADER);
    MoveCompute = Program::Create({ FluidMove });
    if(!MoveCompute) return false;



    // 5. Shadow Map �� Particles �� ������ �׸��� => �׷����� ���̸� ���� ���
    SimpelProgram = Program::Create("./shader/Simple/simple.vs", "./shader/Simple/simple.fs");
    if(!SimpelProgram) return false;

        // shadow map
        depthFrameBuffer = ShadowMap::Create(1024, 1024);



    // 6. ���� ������ ������, depth map �� �����ؼ� ��Ÿ�� �ȼ��� ã�´�
    DrawProgram = Program::Create("./shader/Fluid/fluid.vs", "./shader/Fluid/fluid.fs");
    if(!DrawProgram) return false;






    // ī�޶� ��ü �ʱ�ȭ
    MainCam = Camera::Create();
    
    // Particle => Box �� ǥ������
    BoxMesh = Mesh::CreateBox();





    // Core Particles �� �ʱ� ������ �ʱ�ȭ �Ѵ�
    // �׸��� Core Particles �� ����
    Init_CoreParticles();





    // GPU ���� �����͸� ������ SSBO ���۸� ����
    // Core Particle => �����͸� �����ؼ� GPU �� �ִ´�
    // ���ĵ� �����͸� �ִ´�
    CoreParticleBuffer
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, CoreParticleArray.data(), sizeof(CoreParticle), CoreParticleArray.size());


    // Particle      => ������ �����ʹ� ����, GPU ���� ������ �Ҵ��� ���´�
    ParticleBuffer 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NULL, sizeof(Particle), CoreParticleArray.size());


    CountBuffer 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, &visibleCount, sizeof(unsigned int), 1);


    
    /* SSBO ���۸� �� �ε����� ���ε� */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CoreParticle_Index, CoreParticleBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Particle_Index,     ParticleBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Count_Index,        CountBuffer->Get());

    
    
    /* ��ǻƮ ���α׷��� SSBO �� ���� */
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


    // 4. Make Depth Map
        blockIndex = glGetProgramResourceIndex(SimpelProgram->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(SimpelProgram->Get(), blockIndex, CoreParticle_Index);


    // 5. Rendering
        blockIndex = glGetProgramResourceIndex(DrawProgram->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(DrawProgram->Get(), blockIndex, CoreParticle_Index);



    return true;
}




// ������ Range �� Particles �� ���� �ִ´�
void Context::Init_CoreParticles()
{
    // Particle::Fluid Range �� �յ��ϰ� ������, Particle ���� �� �� �ֵ���
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


    // ī�޶������ �Ÿ��� �������� �����Ѵ� => �ָ� �ִ� ���� ������ �´�
    // ó������ �ƿ� ������ �ϰ� ����
    std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_toCamera_Compare());
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
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)  // �߰� Ű Ŭ��
    {
        if (action == GLFW_PRESS)
        {
            // ���콺 ���� ���� ������ ���� ���콺 Ŀ�� ��ġ ����
            m_giveForceMousePos = glm::vec2((float)x, (float)y);
            m_giveForceMouse = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_giveForceMouse = false;
        }
    }


    if (button == GLFW_MOUSE_BUTTON_RIGHT)  // ���� Ű Ŭ��
    {
        if (action == GLFW_PRESS)
        {
            // ���콺 ���� ���� ������ ���� ���콺 Ŀ�� ��ġ ����
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

    // ���� �� Ȯ��
    ImGui::Image((ImTextureID)depthFrameBuffer->GetShadowMap()->Get(), ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::DragFloat("cameraNear", &cameraNear, 0.01f, 0);
    ImGui::DragFloat("cameraFar", &cameraFar, 0.01f, 0);

    ImGui::DragFloat("offset", &offset, 0.01f, 0);

    ImGui::End();





    // ī�޶� ����, ��ġ ���� Ȯ��
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




    // ���α׷� ����
    Get_Density_Pressure();                             // 1. ���� ������ �е��� �з� ������ ���ϰ�
    Get_Force();                                        // 2. �� ���ڵ��� ��¥���� ���Ѵ�
    Get_Move();                                         // 3. ��¥���� ���� �̵� -> ���� ��ġ�� �ӵ��� ����


    Draw_GPU_Instancing(projection, view);              // 4. �ϴ� Depth map �� ������ => Depth Map �� �����ϰ�



    Draw_Particles(projection, view);                   // 5. ���������� visible particle �鸸 ������
}




// Program ����
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


        // // output �� Particle data �� Ȯ��
        //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, ParticleBuffer->Get());
        //         glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());

        //         SPDLOG_INFO("Density Pressure Compute");
        //         // �ϴ� �α׷� Ȯ������
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

        

        // ���콺 ��Ŭ���� ���µ�, ���� ���� ��, ���� �ѱ�� �� �ʱ�ȭ
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
        /* Uniform Variable �ֱ� */
        MoveCompute->SetUniform("deltaTime", deltaTime);
        MoveCompute->SetUniform("LimitRange", Particle::FluidRange);
        MoveCompute->SetUniform("damping", damping);

        MoveCompute->SetUniform("threshold", threshold);


        //if(tempNei >= 0) neighborLevel = (unsigned int)tempNei;
        MoveCompute->SetUniform("neighborLevel", neighborLevel);
        
        // ���� ī�޶��� ��ġ�� Uniform ���� ����
        // Particle �� ���������� ������ ��ġ���� ī�޶������ �Ÿ��� ���ؾ� �Ѵ�
        MoveCompute->SetUniform("CameraPos", MainCam->Position);


        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        
        // ������ => ���ο� ��ġ�� �����, toCamera ���� �ٲ�����Ƿ� ������ ���� ���ش�
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CoreParticleBuffer->Get());
            // ���� ���, Output �� CPU �� �о����
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());

            // Camera ������ �Ÿ��� �������� sort
            std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_toCamera_Compare());

            // �ٽ� �־��ش�
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(0);
}


void Context::Draw_GPU_Instancing(const glm::mat4& projection, const glm::mat4& view)
{
    // ȭ�鿡 �׸��� �ʰ�, shadow map �� �׷���, ī�޶󿡼� ���� �� �� �ȼ� depth ���� �ؽ�ó�� �����Ѵ�
    depthFrameBuffer->Bind();
        
        glClear(GL_DEPTH_BUFFER_BIT);

        // view port ����
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





void Context::Draw_Particles(const glm::mat4& projection, const glm::mat4& view)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // output ���ۿ� ����� �����͸� �̿��ؼ�, Particles �� �׸���
    DrawProgram->Use();
        
        auto transform = projection * view;

        // vertex shader
            DrawProgram->SetUniform("transform", transform);
            DrawProgram->SetUniform("CameraPos", MainCam->Position);

        // fragment shader
            glActiveTexture(GL_TEXTURE0);
            depthFrameBuffer->GetShadowMap()->Bind();
            DrawProgram->SetUniform("depthMap", 0);

            DrawProgram->SetUniform("offset", offset);
            
            
        BoxMesh->GPUInstancingDraw(DrawProgram.get(), Particle::TotalParticleCount);

    glUseProgram(0);

    glDisable(GL_BLEND);
}