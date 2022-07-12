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

    SPDLOG_INFO("size of float is {}", sizeof(float));
    SPDLOG_INFO("size of Particle is {} and count is {}", sizeof(Particle), (sizeof(Particle) / sizeof(float)));

    // Fluid �� �׸��� ���α׷�
    FluidProgram = Program::Create("./shader/fluid.vs", "./shader/fluid.fs");
    if(!FluidProgram) return false;


    // Particle => Box �� ǥ������
    BoxMesh = Mesh::CreateBox();


    // Particle ���� �������� ����ϴ� ��ǻƮ ���̴�
    FluidComputeShader = Shader::CreateFromFile("./shader/fluid.compute", GL_COMPUTE_SHADER);
    // �ش� ���̴��� ��� ���α׷� ����
    ComputeProgram = Program::Create({FluidComputeShader});
    if(!ComputeProgram) return false;


    // Particle ���� �ʱ� ������ �ʱ�ȭ �Ѵ�
    InitParticles();


    // GPU ���� �����͸� ������ SSBO ���۸� ����, �ʱ�ȭ�� Particles ������ ���� => GPU �� �ִ´�
    ParticleBuffer = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());
    
    // SSBO ���� ���ε�
    


    return true;
}




// ������ Range �� Particles �� ���� �ִ´�
void Context::InitParticles()
{
    // Particle::Fluid Range �������� �ؼ�, ��ƼŬ���� �յ��ϰ� ��ġ ��Ų��
    float xStride = Particle::FluidRange.x / (Particle::ParticleCount.x + 1);
    float yStride = Particle::FluidRange.y / (Particle::ParticleCount.y + 1);
    float zStride = Particle::FluidRange.z / (Particle::ParticleCount.z + 1);


    for(int xCount = 1; xCount <= Particle::ParticleCount.x; xCount++)
    {
        for(int yCount = 1; yCount <= Particle::ParticleCount.y; yCount++)
        {
            for(int zCount = 1; zCount <= Particle::ParticleCount.z; zCount++)
            {
                ParticleArray.push_back( Particle(glm::vec3((xStride * xCount, yStride * yCount, zStride * zCount))) );
            }
        }
    }
}




void Context::ProcessInput(GLFWwindow* window)
{
    
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
    
}
void Context::MouseButton(int button, int action, double x, double y)
{
    
}





void Context::Render()
{
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

}