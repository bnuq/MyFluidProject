#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "image.h"

CLASS_PTR(Texture)
class Texture
{
public:
    /*
        텍스처를 만드는 방법 2가지
        1. 그냥 만들거나
        2. 이미지를 통해서 만들거나
     */
    static TextureUPtr Create(int width, int height, uint32_t format, uint32_t type = GL_UNSIGNED_BYTE);
    static TextureUPtr CreateFromImage(const Image* image);

    ~Texture();

    void Bind() const;

    /* Getter, Setter */
    const uint32_t Get() const { return m_texture; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    uint32_t GetType() const { return m_type; }
    uint32_t GetFormat() const { return m_format; }


    /* Setter */
    /* 
        텍스처를 모델에 적용할 때
        텍스처와 모델의 크기 차이가 너무 많이 나는 경우 -> 어떻게 처리할 것인가

        이때 사용하는 게 Texture Filtering
        Texture Filtering Setting
     */
    void SetFilter(uint32_t minFilter, uint32_t magFilter) const;
    /* 
        텍스처를 모델에 적용할 때
        모델 정점의 텍스처 좌표가, [0,1] 을 벗어나는 경우
        텍스처 좌표에 대응되는 텍스처가 없다 -> 어떻게 처리할 것인가

        이때 사용하는 게 Texture Wrapping
        Texture Wrapping Setting
     */
    void SetWrap(uint32_t sWrap, uint32_t tWrap) const;
    void SetTextureFormat(int width, int height, uint32_t format, uint32_t type);
    void SetBorderColor(const glm::vec4& color) const;

private:
    Texture() {}
    // 초기화 함수 => 무조건 성공
    void CreateTexture();
    // 이미지 데이터 => 텍스처를 생성
    void SetTextureFromImage(const Image* image);


    uint32_t m_texture { 0 };               // gl texture obj id
    int m_width { 0 };                      // texture width
    int m_height { 0 };                     // texture height
    uint32_t m_format { GL_RGBA };          // texel 을 구성하는 포맷 => 여러 개의 channel 로 이루어져 있다
    uint32_t m_type { GL_UNSIGNED_BYTE };   // 각 채널을 구성하는 data type
};




CLASS_PTR(CubeTexture)
class CubeTexture
{
public:
    static CubeTextureUPtr CreateFromImages(const std::vector<Image*>& images);
    ~CubeTexture();

    void Bind() const;

    const uint32_t Get() const { return m_texture; }

private:
    CubeTexture() {}
    bool InitFromImages(const std::vector<Image*>& images);

    uint32_t m_texture { 0 };       // cube map 은 cube texture 라는 gl texture 로 관리한다
};

#endif // __TEXTURE_H__