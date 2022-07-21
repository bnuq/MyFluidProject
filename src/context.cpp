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
    // Fluid �� �׸��� ���α׷�
    DrawProgram = Program::Create("./shader/fluid.vs", "./shader/fluid.fs");
    if(!DrawProgram) return false;

    // Particle ���� �������� ����ϴ� ��ǻƮ ���̴�
    FluidComputeShader = Shader::CreateFromFile("./shader/fluid.compute", GL_COMPUTE_SHADER);
    // �ش� ���̴��� ��� ���α׷� ����
    ComputeProgram = Program::Create({ FluidComputeShader });
    if(!ComputeProgram) return false;

    // Particle �� �����̴� ���̴�
    FluidMoveShader = Shader::CreateFromFile("./shader/fluidMove.compute", GL_COMPUTE_SHADER);
    // �ش� ���̴��� ��� ���α׷� ����
    MoveProgram = Program::Create({ FluidMoveShader });
    if(!MoveProgram) return false;



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
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());

    
    /* 
        SSBO ���۸� �� �ε����� ���ε�

        Input Buffer   == 1 index
        Output Buffer  == 2 index
     */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Input_Index, InputBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Output_Index, OutputBuffer->Get());



    /* 
        ��ǻƮ ���α׷��� SSBO �� ����
        
        Input Buffer  => 1
        Output Buffer => 2

        �̷��� ����Ǿ� �ִ� ���� Compute Shader ���� ������ ���̱� ������, �����̴�
        binding = 1 or 2 �� ������ �Ǿ� �ִ�
     */
    GLuint blockIndex{};

    blockIndex = glGetProgramResourceIndex(ComputeProgram->Get(), GL_SHADER_STORAGE_BUFFER, "InputBuffer");
    glShaderStorageBlockBinding(ComputeProgram->Get(), blockIndex, Input_Index);
    blockIndex = glGetProgramResourceIndex(ComputeProgram->Get(), GL_SHADER_STORAGE_BUFFER, "OutputBuffer");
    glShaderStorageBlockBinding(ComputeProgram->Get(), blockIndex, Output_Index);

    blockIndex = glGetProgramResourceIndex(MoveProgram->Get(), GL_SHADER_STORAGE_BUFFER, "OutputBuffer");
    glShaderStorageBlockBinding(MoveProgram->Get(), blockIndex, Output_Index);

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
                ParticleArray.push_back( 
                    Particle(glm::vec4(xStride * xCount, yStride * yCount, zStride * zCount, 1.0f), MainCam->Position)
                );
            }
        }
    }

    // ī�޶������ �Ÿ��� �������� �����Ѵ�
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

   


    // ������ �ϱ� ����, Compute Program �� ���� => ����ϰ�
    ComputeProgram->Use();
        /* Uniform Variables Setting */
        // Particles �� ���� ����
        ComputeProgram->SetUniform("particlelMass", Particle::ParticleMass);
        ComputeProgram->SetUniform("particlelCount", Particle::TotalParticleCount);

        // Smooth Kernel �� ���� ����
        ComputeProgram->SetUniform("h", 1.0f);
        ComputeProgram->SetUniform("hSquare", 1.0f);

        // �е�=>�з� ��꿡 �ʿ��� ����
        ComputeProgram->SetUniform("gasCoeffi", 1.0f);
        ComputeProgram->SetUniform("restDensity", 1.0f);


        // ���α׷��� �����Ų��
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // �ϴ� ���ุ �ϰ�
    glUseProgram(0);



    // ��� ����� �̿��ؼ� �����̰� �Ѵ�
    MoveProgram->Use();
        /* Uniform Variable �ֱ� */
        MoveProgram->SetUniform("deltaTime", 0.0001f);
        MoveProgram->SetUniform("LimitRange", Particle::FluidRange);
        MoveProgram->SetUniform("damping", 0.1f);
        
        // ���� ī�޶��� ��ġ�� Uniform ���� ����
        // Particle �� ���������� ������ ��ġ���� ī�޶������ �Ÿ��� ���ؾ� �Ѵ�
        MoveProgram->SetUniform("CameraPos", MainCam->Position);


        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, OutputBuffer->Get());
            // ���� ���, Output �� CPU �� �о����
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());
            // Camera ������ �Ÿ��� �������� sort
            std::sort(ParticleArray.begin(), ParticleArray.end(), ParticleCompare());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, InputBuffer->Get());
            // sort �� ����� input buffer �� �ִ´�
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(0);
    
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

    // output ���ۿ� ����� �����͸� �̿��ؼ�, Particles �� �׸���
    DrawProgram->Use();
  
            // ������ �����͸� �̿��ؼ�, ī�޶󿡼� ����� �� ���� �������� ����
            for(int i = 0; i < Particle::TotalParticleCount; i++)
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