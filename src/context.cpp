#include "context.h"
#include <vector>
#include <string>


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

    // ī�޶� ��ü �ʱ�ȭ
    MainCam = Camera::Create();
    if(MainCam == nullptr)
        SPDLOG_INFO("NULLPTR camera");


    SPDLOG_INFO("size of float is {}", sizeof(float));
    SPDLOG_INFO("size of Particle is {} and count is {}", sizeof(Particle), (sizeof(Particle) / sizeof(float)));

    // Fluid �� �׸��� ���α׷�
    FluidProgram = Program::Create("./shader/fluid.vs", "./shader/fluid.fs");
    if(!FluidProgram) return false;


    // Particle ���� �������� ����ϴ� ��ǻƮ ���̴�
    FluidComputeShader = Shader::CreateFromFile("./shader/fluid.compute", GL_COMPUTE_SHADER);
    // �ش� ���̴��� ��� ���α׷� ����
    ComputeProgram = Program::Create({FluidComputeShader});
    if(!ComputeProgram) return false;


    
    // Particle => Box �� ǥ������
    BoxMesh = Mesh::CreateBox();


    // Particle ���� �ʱ� ������ �ʱ�ȭ �Ѵ�
    InitParticles();


    // GPU ���� �����͸� ������ SSBO ���۸� ����, �ʱ�ȭ�� Particles ������ ���� => GPU �� �ִ´�
    ParticleBuffer = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());
    
    // SSBO ���� ���ε�
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Particle_Binding_Index, ParticleBuffer->Get());

    
    // ��ǻƮ ���α׷��� SSBO �� ����
    auto blockIndex = glGetProgramResourceIndex(ComputeProgram->Get(), GL_SHADER_STORAGE_BUFFER, "ParticleBuffer");
    glShaderStorageBlockBinding(ComputeProgram->Get(), blockIndex, Particle_Binding_Index);


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
                ParticleArray.push_back( Particle(glm::vec3(xStride * xCount, yStride * yCount, zStride * zCount)) );
            }
        }
    }



    for(auto p : ParticleArray)
    {
        SPDLOG_INFO("{}, {}, {}", p.Position.x, p.Position.y, p.Position.z);
    }
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

    // Compute Program �� ���� => �� Particle �� �����͸� ����Ѵ�
        // Particles �� ��ġ�� �����ϴ� �迭�� �̿��ؼ� ���, ����
        // ��� ��� ������ => Particles �� ��ġ�� ����� 3-4 �� �迭�� ��ܼ� ���´�

    // �� �ٸ� Compute Program �� ����
    // �� ��ġ���� �����尡 ���� ~ ǥ���� �Ǵ� Particle �ε����� ���� �˾Ƴ���
    // ǥ���� �Ǵ� Particle �ε������� true �� �����ϴ� ���۸� �����Ѵ�
    // ��ü �׸��� ������ �ľ��Ѵ�

    // �׷����� �ε����� ������ ���� => ������� ���鼭, ī�޶� - Particle �Ÿ��� ���
    // Map �� �����Ѵ�

    // ���������� Map �� ����� Particle �� ���ʴ�� ���鼭
    // ī�޶󿡼� �ָ� ������ Particle ���� �׸���


    FluidProgram->Use();
        for(auto p : ParticleArray)
        {
            auto modelTransform = glm::translate(glm::mat4(1.0f), p.Position);
            auto transform = projection * view * modelTransform;

            FluidProgram->SetUniform("transform", transform);

            BoxMesh->Draw(FluidProgram.get());
        }
    glUseProgram(0);

    
}