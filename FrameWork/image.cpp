#include "image.h"
/* 
    stb library 의 함수 정의는 헤더파일에 있지만, 실제 거기서 정의되지 않는다
        왜냐하면 정의는 헤더 파일에 있으면 안되고, 소스 파일에 모여있어야 재정의되는 것을 막을 수 있으니까

    stb library 함수 정의를 이 소스파일에서 진행할 것인데, 매크로를 이용해서 정의 코드를 가져온다
    이 매크로가 정의되어 있으면, 헤더파일에서 정의 코드 부분이 활성화된다 => 함수 사용 가능
    이 매크로가 없으면 => 정의 없음 + 소스 파일 없음 => 링커 에러
 */
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


/* PUBLIC */
ImageUPtr Image::Load(const std::string& filepath, bool flipVertical)
{
    auto image = ImageUPtr(new Image());
    if (!image->LoadWithStb(filepath, flipVertical))
        return nullptr;
    return std::move(image);
}


ImageUPtr Image::Create(int width, int height, int channelCount)
{
    auto image = ImageUPtr(new Image());
    if (!image->Allocate(width, height, channelCount))
        return nullptr;
    return std::move(image);
}


ImageUPtr Image::CreateSingleColorImage(int width, int height, const glm::vec4& color)
{
    glm::vec4 clamped = glm::clamp(color * 255.0f, 0.0f, 255.0f); // 0-1 값을 0-255 값으로 변경
    uint8_t rgba[4] =
    {
        (uint8_t)clamped.r,
        (uint8_t)clamped.g, 
        (uint8_t)clamped.b, 
        (uint8_t)clamped.a, 
    };
    auto image = Create(width, height, 4);
    for (int i = 0; i < width * height; i++)
    {
        memcpy(image->m_data + 4 * i, rgba, 4);
    }
    return std::move(image);
}


void Image::SetCheckImage(int gridX, int gridY)
{
    for (int j = 0; j < m_height; j++)
    {
        for (int i = 0; i < m_width; i++)
        {
            int pos = (j * m_height + i) * m_channelCount;          // 하나의 텍셀은 m_channelCount 만큼 크기를 차지한다
            bool even = ((i / gridX) + (j / gridY)) % 2 == 0;       // 격자
            uint8_t value = even ? 255 : 0;                         // 흰색 or 검은색
            
            for (int k = 0; k < m_channelCount; k++)
                m_data[pos + k] = value;                            // 텍셀에 (1,1,1) 을 넣거나 (0,0,0) 을 넣는다

            if (m_channelCount > 3) m_data[3] = 255;                // 알파 값이 따로 없으면, 불투명하게 설정
        }
    }
}


Image::~Image()
{
    if (m_data)
    {
        stbi_image_free(m_data);
    }
}


/* PRIVATE */
// 외부 라이브러리인 stb 를 활용해서 이미지 파일을 로드하는 함수
bool Image::LoadWithStb(const std::string& filepath, bool flipVertical)
{
    /* 
        이미지는 좌상단이 원점인데, opengl 은 좌하단이 원점이라 기준점이다르다
            이미지가 뒤집어져서 보임
        이미지 로딩 전, 뒤집을 지 여부를 확인한다
     */
    stbi_set_flip_vertically_on_load(flipVertical);
    /* 
        이미지 파일을 로딩 => 이미지 파일의 각 텍셀이 가지고 있는 RGB 값들이 1차원 배열 형태로 저장되어 있는데
                            그 이미지 파일 데이터 배열을 CPU 메모리에 저장하고
                            가리키는 주소를 리턴 == m_data
        한 텍셀의 r,g,b 각각 8비트 차지 => 그래서 m_data 가 8비트를 가리키는 포인터
        이미지를 로딩하면서, 이미지의 정보들도 자동으로 얻어낼 수 있다
            멤버 변수에 저장
     */
    m_data = stbi_load(filepath.c_str(), &m_width, &m_height, &m_channelCount, 0);
    if (!m_data)
    {
        SPDLOG_ERROR("failed to load image: {}", filepath);
        return false;
    }
    return true;
}


bool Image::Allocate(int width, int height, int channelCount)
{
    m_width = width;
    m_height = height;
    m_channelCount = channelCount;
    
    // 직접 Heap 메모리 할당
    // R or G 같은 하나의 요소 = 8비트 = signed char = 1 byte
    m_data = (uint8_t*)malloc(m_width * m_height * m_channelCount);
    return m_data ? true : false;
}