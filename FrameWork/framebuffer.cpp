#include "framebuffer.h"

/* PUBLIC */
FramebufferUPtr Framebuffer::Create(const TexturePtr colorAttachment)
{
    auto framebuffer = FramebufferUPtr(new Framebuffer());
    if (!framebuffer->InitWithColorAttachment(colorAttachment))
        return nullptr;
    return std::move(framebuffer);
}


// color buffer 로 사용하던 텍스처를 교체
// render buffer, framebuffer 를 모두 새로 할당한다
FramebufferUPtr Framebuffer::ReCreate(const TexturePtr another)
{
    glDeleteRenderbuffers(1, &m_depthStencilBuffer);
    glDeleteFramebuffers(1, &m_framebuffer);

    return Create(another);
}


Framebuffer::~Framebuffer()
{
    if (m_depthStencilBuffer)
        glDeleteRenderbuffers(1, &m_depthStencilBuffer);
    if (m_framebuffer)
        glDeleteFramebuffers(1, &m_framebuffer);
}


void Framebuffer::BindToDefault()
{
    // Default Frame Buffer => 화면에 출력하는 것
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Framebuffer::Bind() const
{
    // Frame Buffer 만 Bind 하면, 이하의 render buffer 와 texture 는 모두 자동으로 사용할 수 있다
    // 둘다 Frame Buffer 아래에 속하니까
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
}



/* PRIVATE */
bool Framebuffer::InitWithColorAttachment(const TexturePtr colorAttachment)
{
    m_colorAttachment = colorAttachment;    // = color buffer 로 사용될 텍스처

    glGenFramebuffers(1, &m_framebuffer);               // FBO 생성
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);   // FBO Bind

        // 프레임 버퍼 -> Color Buffer 에 텍스처 붙이기
        glFramebufferTexture2D
        (
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            colorAttachment->Get(),     // return texture handle id
            0                           // mipmap level
        );

    // Render Buffer = depth + stencil buffer 생성
    glGenRenderbuffers(1, &m_depthStencilBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilBuffer);
        glRenderbufferStorage
        (
            GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8,                // depth=>24bits, stencil=>8bits 사용
            colorAttachment->GetWidth(),        // color buffer 와 같은 너비, 높이의 이미지 사용
            colorAttachment->GetHeight()
        );
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    glFramebufferRenderbuffer                   // Frame Buffer 에 Render Buffer 붙이기
    (
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,            // depth + stencil 위치에 연결한다
        GL_RENDERBUFFER,
        m_depthStencilBuffer                    // 붙이려는 Render Buffer 의 id
    );


    // frame buffer 의 상태 체크
    auto result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE)
    {
        SPDLOG_ERROR("failed to create framebuffer: {}", result);
        return false;
    }  

    
    // 화면에 그리는 프레임 버퍼로 다시 초기화 한다
    BindToDefault();

    return true;
}