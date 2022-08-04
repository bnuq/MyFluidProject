#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "common.h"

CLASS_PTR(Image)
class Image
{
public:
    static ImageUPtr Load(const std::string& filepath, bool flipVertical = true);
    static ImageUPtr Create(int width, int height, int channelCount = 4);
    static ImageUPtr CreateSingleColorImage(int width, int height, const glm::vec4& color);
    void SetCheckImage(int gridX, int gridY);

    ~Image();


    const uint8_t* GetData() const { return m_data; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetChannelCount() const { return m_channelCount; }

private:
    Image() {};

    bool LoadWithStb(const std::string& filepath, bool flipVertical);
    bool Allocate(int width, int height, int channelCount);


    int m_width { 0 };                  // 이미지 파일의 너비
    int m_height { 0 };                 // 이미지 파일의 높이
    int m_channelCount { 0 };           // 텍셀 하나에 저장되는 정보 개수( RGB or RGBA )
    uint8_t* m_data { nullptr };        // 이미지 데이터가 저장된 주소
};

#endif // __IMAGE_H__