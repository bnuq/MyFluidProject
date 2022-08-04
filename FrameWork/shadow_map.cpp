#include "shadow_map.h"

ShadowMapUPtr ShadowMap::Create(int width, int height) 
{
    auto shadowMap = ShadowMapUPtr(new ShadowMap());
    if (!shadowMap->Init(width, height))
        return nullptr;
    return std::move(shadowMap);
}


ShadowMap::~ShadowMap() {
    if (m_framebuffer) {
        glDeleteFramebuffers(1, &m_framebuffer);
    }
}


void ShadowMap::Bind() const
{
    // shadow map 의 프레임버퍼를 사용 => 해당 프레임 버퍼에 렌더링한다
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
}


bool ShadowMap::Init(int width, int height)
{
    // 렌더링할 프레임 버퍼를 생성하고 bind
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

        /* 
            깊이 값을 저장할 shadow map 을 생성
            텍스처에 color 가 아닌 깊이 값을 저장할 것이다
            
            포맷 = 깊이 값 => GL_DEPTH_COMPONENT
            타입 = 깊이 값이 저장되어야 하므로, unsigned byte 로는 부족 => GL_FLOAT
         */
        m_shadowMap = Texture::Create(width, height, GL_DEPTH_COMPONENT, GL_FLOAT);
        m_shadowMap->SetFilter(GL_NEAREST, GL_NEAREST);
        m_shadowMap->SetWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
        
        
        m_shadowMap->SetBorderColor(glm::vec4(1.0f));
        

        // 프레임 버퍼에 텍스처를 연결
        glFramebufferTexture2D
        (
            GL_FRAMEBUFFER, 
            GL_DEPTH_ATTACHMENT,                // 프레임 버퍼의 Depth 쪽에 텍스처를 연결
            GL_TEXTURE_2D, 
            m_shadowMap->Get(), 
            0
        );

        // color buffer 를 사용하지 않을 것이라는 걸 알림
        glDrawBuffer(GL_NONE);      // 어떤 color buffer 에 그릴 것이냐                     => 안 그림
        glReadBuffer(GL_NONE);      // 이후 어떤 color buffer 에서 값을 읽고 작업할 것이냐   => 안 읽음


        auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            SPDLOG_ERROR("failed to complete shadow map framebuffer: {:x}", status);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return true;
}