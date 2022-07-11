#include "image.h"


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
    glm::vec4 clamped = glm::clamp(color * 255.0f, 0.0f, 255.0f);
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

// ?´ë¯¸ì?? ë©”ëª¨ë¦¬ë?? ì§ì ‘ ?• ?‹¹?•œ ?›„, Check Image ë¥? ì§ì ‘ ë§Œë“œ?Š” ?•¨?ˆ˜
void Image::SetCheckImage(int gridX, int gridY)
{
    for (int j = 0; j < m_height; j++)
    {
        for (int i = 0; i < m_width; i++)
        {
            int pos = (j * m_height + i) * m_channelCount;          // ?•˜?‚˜?˜ ?…?????? m_channelCount ë§Œí¼ ?¬ê¸°ë?? ì°¨ì???•œ?‹¤
            bool even = ((i / gridX) + (j / gridY)) % 2 == 0;       // ê²©ì
            uint8_t value = even ? 255 : 0;                         // ?°?ƒ‰ or ê²?????ƒ‰
            
            for (int k = 0; k < m_channelCount; k++)
                m_data[pos + k] = value;                            // ?…????— (1,1,1) ?„ ?„£ê±°ë‚˜ (0,0,0) ?„ ?„£?Š”?‹¤

            if (m_channelCount > 3) m_data[3] = 255;                // ?•Œ?ŒŒ ê°’ì´ ?”°ë¡? ?—†?œ¼ë©?, ë¶ˆíˆ¬ëª…í•˜ê²? ?„¤? •
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
// ?™¸ë¶? ?¼?´ë¸ŒëŸ¬ë¦¬ì¸ stb ë¥? ?™œ?š©?•´?„œ ?´ë¯¸ì?? ?ŒŒ?¼?„ ë¡œë“œ?•˜?Š” ?•¨?ˆ˜
bool Image::LoadWithStb(const std::string& filepath, bool flipVertical)
{
    /* 
        ?´ë¯¸ì???Š” ì¢Œìƒ?‹¨?´ ?›? ?¸?°, opengl ??? ì¢Œí•˜?‹¨?´ ?›? ?´?¼ ê¸°ì??? ?´?‹¤ë¥´ë‹¤
            ?´ë¯¸ì??ê°? ?’¤ì§‘ì–´? ¸?„œ ë³´ì„
        ?´ë¯¸ì?? ë¡œë”© ? „, ?’¤ì§‘ì„ ì§? ?—¬ë¶?ë¥? ?™•?¸?•œ?‹¤
     */
    stbi_set_flip_vertically_on_load(flipVertical);
    /* 
        ?´ë¯¸ì?? ?ŒŒ?¼?„ ë¡œë”© => ?´ë¯¸ì?? ?ŒŒ?¼?˜ ê°? ?…????´ ê°?ì§?ê³? ?ˆ?Š” RGB ê°’ë“¤?´ 1ì°¨ì› ë°°ì—´ ?˜•?ƒœë¡? ????¥?˜?–´ ?ˆ?Š”?°
                            ê·? ?´ë¯¸ì?? ?ŒŒ?¼ ?°?´?„° ë°°ì—´?„ CPU ë©”ëª¨ë¦¬ì— ????¥?•˜ê³?
                            ê°?ë¦¬í‚¤?Š” ì£¼ì†Œë¥? ë¦¬í„´ == m_data
        ?•œ ?…????˜ r,g,b ê°ê° 8ë¹„íŠ¸ ì°¨ì?? => ê·¸ë˜?„œ m_data ê°? 8ë¹„íŠ¸ë¥? ê°?ë¦¬í‚¤?Š” ?¬?¸?„°
        ?´ë¯¸ì??ë¥? ë¡œë”©?•˜ë©´ì„œ, ?´ë¯¸ì???˜ ? •ë³´ë“¤?„ ??™?œ¼ë¡? ?–»?–´?‚¼ ?ˆ˜ ?ˆ?‹¤
            ë©¤ë²„ ë³??ˆ˜?— ????¥
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
    m_channelCount = channelCount;  // ?•œ ?”½????„ ?´ë£¨ëŠ” ?°?´?„°?— ????•œ ? •ë³?
    // ì§ì ‘ Heap ë©”ëª¨ë¦? ?• ?‹¹
    // R or G ê°™ì?? ?•˜?‚˜?˜ ?š”?†Œ = 8ë¹„íŠ¸ = signed char = 1 byte
    m_data = (uint8_t*)malloc(m_width * m_height * m_channelCount);
    return m_data ? true : false;
}