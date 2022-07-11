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


    // Particle ���� �������� ����ϴ� ���̴��� �����

    // �ش� ���̴��� ��� ���α׷� ����

    return true;
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