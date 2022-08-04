#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include "texture.h"

CLASS_PTR(Framebuffer)
class Framebuffer
{
public:
    static FramebufferUPtr Create(const TexturePtr colorAttachment);
    FramebufferUPtr ReCreate(const TexturePtr colorAttachment);
    
    ~Framebuffer();
    
    static void BindToDefault();
    void Bind() const;
    
    const uint32_t Get() const { return m_framebuffer; }
    const TexturePtr GetColorAttachment() const { return m_colorAttachment; }

private:
    Framebuffer() {}
    bool InitWithColorAttachment(const TexturePtr colorAttachment);

    /* 
        프레임버퍼
        = color buffer + depth buffer + stencil buffer
        = gl frame buffer 로 관리

            color buffer                        => texture 로 관리
            depth buffer + stencil buffer       => gl redner buffer 로 관리
     */
    uint32_t m_framebuffer { 0 };           // 이하 모든 객체를 관리하는 Frame Buffer
    uint32_t m_depthStencilBuffer { 0 };    // Render Buffer, single image, 여기서는 스텐실과 깊이를 저장하는 용도로 사용
    TexturePtr m_colorAttachment;           // frame buffer ~ color buffer 로 사용될 텍스처
                                            // 텍스처에 색을 저장하는 방식으로, color buffer 처럼 사용한다
};

#endif // __FRAMEBUFFER_H__