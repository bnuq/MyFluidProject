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

    uint32_t m_framebuffer { 0 };
    uint32_t m_depthStencilBuffer { 0 };
    TexturePtr m_colorAttachment;           // color buffer ??????????? ???????? ???????? ??????
};

#endif // __FRAMEBUFFER_H__