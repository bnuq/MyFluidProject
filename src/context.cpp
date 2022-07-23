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


    // Initialize Programs
    // �е��� �з��� ����ϴ� compute shader
    FluidDensityPressure = Shader::CreateFromFile("./shader/FluidDensityPressure.compute", GL_COMPUTE_SHADER);
    DensityPressureCompute = Program::Create({ FluidDensityPressure });
    if(!DensityPressureCompute) return false;

    // Particle ���� �������� ����ϴ� compute shader
    FluidForce = Shader::CreateFromFile("./shader/FluidForce.compute", GL_COMPUTE_SHADER);
    ForceCompute = Program::Create({ FluidForce });
    if(!ForceCompute) return false;

    // Particle �� �����̴� ���̴�
    FluidMove = Shader::CreateFromFile("./shader/FluidMove.compute", GL_COMPUTE_SHADER);
    MoveCompute = Program::Create({ FluidMove });
    if(!MoveCompute) return false;


    // Fluid �� �׸��� ���α׷�
    DrawProgram = Program::Create("./shader/fluid.vs", "./shader/fluid.fs");
    if(!DrawProgram) return false;

    /************************************************/

    // ī�޶� ��ü �ʱ�ȭ
    MainCam = Camera::Create();
    
    // Particle => Box �� ǥ������
    BoxMesh = Mesh::CreateBox();



    // Particle ���� �ʱ� ������ �ʱ�ȭ �Ѵ�
    InitParticles();


    // GPU ���� �����͸� ������ SSBO ���۸� ����, �ʱ�ȭ�� Particles ������ ���� => GPU �� �ִ´�
    // ���� 2�� ��� ó���� �����ϰ� �ʱ�ȭ�Ѵ�
    InputBuffer 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());

    OutputBuffer
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NULL, sizeof(Particle), ParticleArray.size());

    unsigned int temp = 0;
    testBuffer = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, &temp, sizeof(unsigned int), 1);

    /* 
        SSBO ���۸� �� �ε����� ���ε�

        Input Buffer   == 1 index
        Output Buffer  == 2 index
     */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Input_Index, InputBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Output_Index, OutputBuffer->Get());

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, testBuffer->Get());

    /* 
        ��ǻƮ ���α׷��� SSBO �� ����
        
        input buffer  => density pressure compute => output buffer
        output buffer => force compute            => output buffer
        output buffer => move compute             => output buffer
        
        output buffer ���� �о�� => sort => ������ && input buffer �� �ֱ�
     */
    GLuint blockIndex{};

    // for density pressure compute program
    blockIndex = glGetProgramResourceIndex(DensityPressureCompute->Get(), GL_SHADER_STORAGE_BUFFER, "InputBuffer");
    glShaderStorageBlockBinding(DensityPressureCompute->Get(), blockIndex, Input_Index);
    blockIndex = glGetProgramResourceIndex(DensityPressureCompute->Get(), GL_SHADER_STORAGE_BUFFER, "OutputBuffer");
    glShaderStorageBlockBinding(DensityPressureCompute->Get(), blockIndex, Output_Index);

    // for force compute program
    blockIndex = glGetProgramResourceIndex(ForceCompute->Get(), GL_SHADER_STORAGE_BUFFER, "OutputBuffer");
    glShaderStorageBlockBinding(ForceCompute->Get(), blockIndex, Output_Index);
    blockIndex = glGetProgramResourceIndex(ForceCompute->Get(), GL_SHADER_STORAGE_BUFFER, "TestBuffer");
    glShaderStorageBlockBinding(ForceCompute->Get(), blockIndex, 5);

    // for move compute program
    blockIndex = glGetProgramResourceIndex(MoveCompute->Get(), GL_SHADER_STORAGE_BUFFER, "OutputBuffer");
    glShaderStorageBlockBinding(MoveCompute->Get(), blockIndex, Output_Index);




    // test
    // ������ ��ġ, toCamera ���� �̿��ؼ� �е��� �з��� �ʱ�ȭ �Ѵ�
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

        // output buffer ������ �а�, �ٽ� input buffer �� �־��ش�
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, OutputBuffer->Get());
            // ���� ���, Output �� CPU �� �о����
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());

            // �ϴ� �α׷� Ȯ������
            for(unsigned int i = 0; i < Particle::TotalParticleCount; i++)
            {
                SPDLOG_INFO("{} th Particle Density {}", i, ParticleArray[i].density);
                SPDLOG_INFO("{} th Particle Pressure {}", i, ParticleArray[i].pressure);
            }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, InputBuffer->Get());
        
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        
    glUseProgram(0);

    return true;
}




// ������ Range �� Particles �� ���� �ִ´�
void Context::InitParticles()
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
                // position, velocity, toCamera ���� �ʱ�ȭ
                ParticleArray.push_back( 
                    Particle(glm::vec4(xStride * xCount, yStride * yCount, zStride * zCount, 1.0f), MainCam->Position)
                );
            }
        }
    }

    // ī�޶������ �Ÿ��� �������� �����Ѵ�
    // �ָ� �ִ� ���� ������ �´�
    std::sort(ParticleArray.begin(), ParticleArray.end(), ParticleCompare());
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
        ImGui::DragFloat("Smooth Kernel Radius", &SmoothKernelRadius);


        ImGui::DragFloat("Gas Coeffi", &gas.gasCoeffi);
        ImGui::DragFloat("Gas RestDensity", &gas.restDensity);

        ImGui::DragFloat("viscosity", &forvar.viscosity);
        ImGui::DragFloat("surfCoeffi", &forvar.surfCoeffi);
        ImGui::DragFloat("surfForceThreshold", &forvar.surfForceThreshold);

        ImGui::DragFloat4("gravityAcel", glm::value_ptr(forvar.gravityAcel));

        ImGui::DragFloat("deltaTime", &movvar.deltaTime);
        ImGui::DragFloat("damping", &movvar.damping);


        ImGui::DragFloat("pressureRatio", &pressureRatio, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("viscosityRatio", &viscosityRatio, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("surfaceRatio", &surfaceRatio, 0.001f, 0.0f, 1.0f);
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


    // // ���� ���� ���ĵ� �迭 => �е��� �з��� ���Ѵ�
    // DensityPressureCompute->Use();

    //     // Set Uniforms
    //     DensityPressureCompute->SetUniform("h", SmoothKernelRadius);
    //     DensityPressureCompute->SetUniform("hSquare", SmoothKernelRadius * SmoothKernelRadius);

    //     DensityPressureCompute->SetUniform("particleMass", Particle::ParticleMass);
    //     DensityPressureCompute->SetUniform("particleCount", Particle::TotalParticleCount);

    //     DensityPressureCompute->SetUniform("gasCoeffi", gas.gasCoeffi);
    //     DensityPressureCompute->SetUniform("gasCoeffi", gas.restDensity);

    //     glDispatchCompute(1, 1, 1);
    //     glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, OutputBuffer->Get());
    //         Particle tempParticle{};
    //         tempParticle.density = -100.0f;

    //         // ���� ���, Output �� CPU �� �о����
    //         glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle), &tempParticle);
            
    //         SPDLOG_INFO("temp particle density {}", tempParticle.density);

    //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        
    // glUseProgram(0);


    // // ���� �� ������ ��¥���� ����
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


    // // ��� ����� �̿��ؼ� �����̰� �Ѵ�
    // MoveCompute->Use();
    //     /* Uniform Variable �ֱ� */
    //     MoveCompute->SetUniform("deltaTime", movvar.deltaTime);
    //     MoveCompute->SetUniform("LimitRange", Particle::FluidRange);
    //     MoveCompute->SetUniform("damping", movvar.damping);
        
    //     // ���� ī�޶��� ��ġ�� Uniform ���� ����
    //     // Particle �� ���������� ������ ��ġ���� ī�޶������ �Ÿ��� ���ؾ� �Ѵ�
    //     MoveCompute->SetUniform("CameraPos", MainCam->Position);


    //     glDispatchCompute(1, 1, 1);
    //     glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, OutputBuffer->Get());
    //         // ���� ���, Output �� CPU �� �о����
    //         glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());
    //         // Camera ������ �Ÿ��� �������� sort
    //         std::sort(ParticleArray.begin(), ParticleArray.end(), ParticleCompare());
    //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, InputBuffer->Get());
    //         // sort �� ����� input buffer �� �ִ´�
    //         glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());
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




    // �������� ���� ����
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //SPDLOG_INFO("First surfNormal is {}, {}, {}", ParticleArray[0].surfNormal.x, ParticleArray[0].surfNormal.y, ParticleArray[0].surfNormal.z);
    //SPDLOG_INFO("First Force is {}, {}, {}", ParticleArray[0].force.x, ParticleArray[0].force.y, ParticleArray[0].force.z);
    

    // output ���ۿ� ����� �����͸� �̿��ؼ�, Particles �� �׸���
    DrawProgram->Use();
  
            // ������ �����͸� �̿��ؼ�, ī�޶󿡼� ����� �� ���� �������� ����
            for(unsigned int i = 0; i < Particle::TotalParticleCount; i++)
            {
                auto ParticlePos = glm::vec3(ParticleArray[i].Position.x, ParticleArray[i].Position.y, ParticleArray[i].Position.z);

                auto modelTransform = glm::translate(glm::mat4(1.0f), ParticlePos);
                auto transform = projection * view * modelTransform;

                DrawProgram->SetUniform("transform", transform);
                DrawProgram->SetUniform("ViewVec", glm::normalize(MainCam->Position - ParticlePos));


                BoxMesh->Draw(DrawProgram.get());

            }

    glUseProgram(0);

    glDisable(GL_BLEND);
}