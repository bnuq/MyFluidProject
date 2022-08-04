#ifndef __SHADOW_MAP_H__
#define __SHADOW_MAP_H__

#include "texture.h"

CLASS_PTR(ShadowMap);
class ShadowMap {
public:
    static ShadowMapUPtr Create(int width, int height);
    ~ShadowMap();

    const uint32_t Get() const { return m_framebuffer; }
    void Bind() const;
    const TexturePtr GetShadowMap() const { return m_shadowMap; }

private:
    ShadowMap() {}
    bool Init(int width, int height);

    uint32_t m_framebuffer { 0 };           // 렌더링을 진행할 프레임 버퍼
    TexturePtr m_shadowMap;                 // 해당 프레임 버퍼에서 렌더링한 값을 저장할 버퍼 => 텍스처
};

#endif // __SHADOW_MAP_H__