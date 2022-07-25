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
    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);


    /* Initialize Programs */
    // 1. Density, Pressure ���
    FluidDensityPressure = Shader::CreateFromFile("./shader/FluidDensityPressure.compute", GL_COMPUTE_SHADER);
    DensityPressureCompute = Program::Create({ FluidDensityPressure });
    if(!DensityPressureCompute) return false;



    // 2. Force, Surface ���
    FluidForce = Shader::CreateFromFile("./shader/FluidForce.compute", GL_COMPUTE_SHADER);
    ForceCompute = Program::Create({ FluidForce });
    if(!ForceCompute) return false;



    // 3. Velocity, Position, toCamera, visible ���
    FluidMove = Shader::CreateFromFile("./shader/FluidMove.compute", GL_COMPUTE_SHADER);
    MoveCompute = Program::Create({ FluidMove });
    if(!MoveCompute) return false;



    // 4. Fluid Rendering
    DrawProgram = Program::Create("./shader/fluid.vs", "./shader/fluid.fs");
    if(!DrawProgram) return false;




    // ī�޶� ��ü �ʱ�ȭ
    MainCam = Camera::Create();
    
    // Particle => Box �� ǥ������
    BoxMesh = Mesh::CreateBox();



    // Particle ���� �ʱ� ������ �ʱ�ȭ �Ѵ�
    Init_CoreParticles();



    // GPU ���� �����͸� ������ SSBO ���۸� ����
    // Core Particle => �����͸� �����ؼ� GPU �� �ִ´�
    CoreParticleBuffer
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, CoreParticleArray.data(), sizeof(CoreParticle), CoreParticleArray.size());

    // Particle      => ������ �����ʹ� ����, GPU ���� ������ �Ҵ��� ���´�
    ParticleBuffer 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NULL, sizeof(Particle), CoreParticleArray.size());

    CountBuffer 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, &visibleCount, sizeof(unsigned int), 1);


    
    /* SSBO ���۸� �� �ε����� ���ε� */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CoreParticle_Index, CoreParticleBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Particle_Index, ParticleBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Count_Index, CountBuffer->Get());

    
    
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
        blockIndex = glGetProgramResourceIndex(MoveCompute->Get(), GL_SHADER_STORAGE_BUFFER, "CountBuffer");
        glShaderStorageBlockBinding(MoveCompute->Get(), blockIndex, Count_Index);




    // 1. density pressure Ȯ��
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

            // output �� Particle data �� Ȯ��
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ParticleBuffer->Get());
                glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());

                SPDLOG_INFO("Density Pressure Compute");
                // �ϴ� �α׷� Ȯ������
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


    // 2. force Ȯ��
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
                // �ϴ� �α׷� Ȯ������
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


    // 3. velocity, position ���ϱ�



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

    // ī�޶������ �Ÿ��� �������� �����Ѵ�
    // �ָ� �ִ� ���� ������ �´�
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
    if(!m_cameraControl) return;


    auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - m_prevMousePos;

    MainCam->Rotate(deltaPos);
    m_prevMousePos = pos;
}
void Context::MouseButton(int button, int action, double x, double y)
{
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
        //ImGui::DragFloat("surfCoeffi", &forvar.surfCoeffi);
        //ImGui::DragFloat("surfForceThreshold", &forvar.surfForceThreshold);

        ImGui::DragFloat4("gravityAcel", glm::value_ptr(gravityAcel));

        ImGui::DragFloat("deltaTime", &deltaTime);
        ImGui::DragFloat("damping", &damping);

        ImGui::DragFloat("threshold", &threshold, 0.001f, 0.0f);

        
        ImGui::DragFloat("neighborLevel", &neighborLevel, 0.001, 0);
        ImGui::DragFloat("correction", &correction, 0.0001, 0);
    }
    ImGui::End();



    // ī�޶� ����, ��ġ ���� Ȯ��
    auto projection = glm::perspective
    (
        glm::radians(45.0f),
        (float)m_width / (float)m_height,
        0.01f, 500.0f
    );

    auto view = glm::lookAt
    (
        //glm::vec3(0, 0, -5.0f),
        MainCam->Position,
        //glm::vec3(0.0f, 0.0f, 0.0f),
        MainCam->Position + MainCam->DirVec,
        MainCam->UpVec
    );



    /* 
        Input Data
            position
            velocity
            toCamera        �� �̿��ؼ�

        Output Data
            position
            velocity
            range
            density
            pressure
            toCamera        �� ���Ѵ�
     */
    /*
    {
        DensityPressureCompute->Use();

            // Set Uniforms
            DensityPressureCompute->SetUniform("h", SmoothKernelRadius);
            DensityPressureCompute->SetUniform("hSquare", SmoothKernelRadius * SmoothKernelRadius);

            DensityPressureCompute->SetUniform("particleMass", Particle::ParticleMass);
            DensityPressureCompute->SetUniform("TotalParticleCount", Particle::TotalParticleCount);

            DensityPressureCompute->SetUniform("gasCoeffi", gas.gasCoeffi);
            DensityPressureCompute->SetUniform("gasCoeffi", gas.restDensity);

            glDispatchCompute(1, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            
        glUseProgram(0);
    }
    */



    /* 
        ����
        output data ��
            surf normal
            isSurf
            force           �� ���ϰ� �����Ѵ�
     */
    // ForceCompute->Use();
    //     // Set Uniforms
    //     // 0
    //     DensityPressureCompute->SetUniform("h", SmoothKernelRadius);
    //     DensityPressureCompute->SetUniform("hSquare", SmoothKernelRadius * SmoothKernelRadius);

    //     DensityPressureCompute->SetUniform("particleMass", Particle::ParticleMass);
    //     DensityPressureCompute->SetUniform("particleCount", Particle::TotalParticleCount);
        
    //     // 1
    //     ForceCompute->SetUniform("viscosity", forvar.viscosity);
    //     ForceCompute->SetUniform("surfCoeffi", forvar.surfCoeffi);
    //     ForceCompute->SetUniform("surfForceThreshold", forvar.surfForceThreshold);

    //     // 2
    //     ForceCompute->SetUniform("gravityAcel", forvar.gravityAcel);

    //     ForceCompute->SetUniform("pressureRatio", pressureRatio);
    //     ForceCompute->SetUniform("viscosityRatio", viscosityRatio);
    //     ForceCompute->SetUniform("surfaceRatio", surfaceRatio);

    //     glDispatchCompute(1, 1, 1);
    //     glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


    //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, testBuffer->Get());
    //         unsigned int temp;
    //         glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &temp);
    //         //SPDLOG_INFO("{}", temp);

    //         temp = 0;
    //         glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &temp);
    //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // glUseProgram(0);




    
    
    /* 
        Output ���ۿ� ����ִ� �����͵鿡�� �ѹ��� ������ �ؾ��� ��?
        �׳� ������ ���� Ƚ���� ���̴� �� ���� ����, �迭�� ���̴� �� ���� ����

        ǥ�� Particle �� ������ �̸� �˰� �ִٰ� ���� -
            Compute Shader ���� ����� �� �ִٰ� ����
            Draw Call �ϴ� ������ �Ǵ� ��

        �̹� ���ĵ� Particle Array = �ָ� �ִ� �� => ������ �ִ� ��, ������ ����ִ�
            ���ʿ��� ǥ�� Particle �� ���� ~ �� ���� Draw Call �� �ϸ�
            ������ ī�޶󿡼� ����� Particle ���� ��µǴ� ��
                ������ �ִ� �� ǥ���� �ƴ� �����Ͱ� ���� �� �ƴϳ�
                �׷��� �Ÿ��� �ʹ�

            ǥ���� �ƴϸ� ������ �ڷ� ����
            ǥ���̸鼭
                ī�޶�κ��� �Ÿ��� �� ���� ������ ���� ����
            [    ǥ��    ][��ǥ��]
             �� -> �����
             ǥ�� ������ŭ ����

            <------------> �� �� ������ Draw Call �ϸ� ���� ������?
     */


    // ���α׷� ����
    Get_Density_Pressure();
    Get_Force();
    Get_Move();




    // �������� ���� ����
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    Draw_Particles(projection, view);
    

    glDisable(GL_BLEND);
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

        // Core Particle �����͸� �о�´�
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CoreParticleBuffer->Get());
            // ���� ���, Output �� CPU �� �о����
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());

            // Camera ������ �Ÿ��� �������� sort
            std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_toCamera_Compare());

            // �ٽ� �־��ش�
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


        // surface particles ������ �д´�
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CountBuffer->Get());
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &visibleCount);

            SPDLOG_INFO("Surface count is {}", visibleCount);

            unsigned int temp = 0;
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &temp);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(0);
}


void Context::Draw_Particles(const glm::mat4& projection, const glm::mat4& view)
{
    // output ���ۿ� ����� �����͸� �̿��ؼ�, Particles �� �׸���
    DrawProgram->Use();
  
        // ������ �ϱ� ����, visible ������ ������ �߰��� �����Ѵ�
        std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_visible_Compare());

        // ���ĵ� Core Particle �����͸� �̿��ؼ�, ī�޶󿡼� ����� �� ���� �������� ����
        // surface count ���� ��ŭ�� �׸���
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