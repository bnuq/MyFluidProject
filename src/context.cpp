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


    // Particle 들의 움직임을 계산하는 셰이더를 만든다

    // 해당 셰이더를 담는 프로그램 생성

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
    // Compute Program 을 실행 => 각 Particle 의 데이터를 계산한다
        // Particles 의 위치를 저장하는 배열을 이용해서 계산, 접근
        // 계산 결과 데이터 => Particles 의 위치가 저장된 3-4 차 배열에 담겨서 나온다

    // 또 다른 Compute Program 을 실행
    // 각 위치에서 스레드가 돌아 ~ 표면이 되는 Particle 인덱스만 따로 알아내서
    // 표면이 되는 Particle 인덱스에만 true 를 저장하는 버퍼를 리턴한다
    // 전체 그리는 개수도 파악한다

    // 그려지는 인덱스를 저장한 버퍼 => 순차대로 돌면서, 카메라 - Particle 거리를 계산
    // Map 에 저장한다

    // 마지막으로 Map 에 저장된 Particle 을 차례대로 돌면서
    // 카메라에서 멀리 떨어진 Particle 부터 그린다

}