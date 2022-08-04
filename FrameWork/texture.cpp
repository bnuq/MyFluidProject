#include "texture.h"

TextureUPtr Texture::Create(int width, int height, uint32_t format, uint32_t type)
{
    auto texture = TextureUPtr(new Texture());
    
    texture->CreateTexture();
    texture->SetTextureFormat(width, height, format, type);

    // 이미지 없이 texture 를 만드는 경우 => mipmap 이 없다
    texture->SetFilter(GL_LINEAR, GL_LINEAR);

    return std::move(texture);
}


TextureUPtr Texture::CreateFromImage(const Image* image)
{
    auto texture = TextureUPtr(new Texture());

    texture->CreateTexture();
    texture->SetTextureFromImage(image);

    return std::move(texture);
}


void Texture::CreateTexture()
{
    glGenTextures(1, &m_texture);
    Bind();
        /* 해당 텍스처를 모델에 적용할 때 필요한 설정들을 진행한다 */
        /*
            1. 텍스처와 적용 모델의 크기가 현저히 다를 때 ~ 를 대비해서
            Texture Filtering 설정을 진행한다
        */
        SetFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
        /* 
            2. 적용 모델의 정점 중 정점의 텍스처 좌표가 [0, 1] 을 벗어나는 경우를 대비해서
            Texture Wrapping 설정을 진행한다
        */
        SetWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}


Texture::~Texture()
{
    if (m_texture)
    {
        // gl texture obj 리소스 해제
        glDeleteTextures(1, &m_texture);
    }
}


void Texture::Bind() const
{
    /* 
        지금부터 별말 없어도, 해당 텍스처를 사용하겠다고 선언

        해당 텍스처에 접근할 때, GL texture handle id 가 아닌 용도로 말해도
        무조건 해당 텍스처를 사용하게 된다

        여기서는, 지금부터 m_texture 를 GL_TEXTURE_2D 로 사용하겠다~ 는 뜻
            m_texture 에 설정된 텍스처 => GL_TEXTURE_2D 라는 이름으로 사용하겠다
     */
    glBindTexture(GL_TEXTURE_2D, m_texture);
}


void Texture::SetFilter(uint32_t minFilter, uint32_t magFilter) const
{
    /* 
        1. Texture Filtering 설정
        텍스처와 적용해야 하는 mesh 간 크기 차이가 날 때, 어떻게 텍스처링을 할 것인가
        먼저 GL_TEXTURE_2D 룰 통해서 Bind 한 GL_TEXTURE_2D 텍스처에 접근
            GL_TEXTURE_MIN_FILTER
                텍스처가 메쉬에 비해서 너무 크다, 텍스처를 줄여야 하는 경우

                [ MipMapping 을 사용하지 않는 경우 ]
                GL_NEAREST
                    기존의 level 0 텍스처에서, 정점의 텍스처 좌표에서 가장 가까운 텍셀 색깔 가져옴
                GL_NEAREST
                    기존의 level 0 텍스처에서, 이중보간을 통해 주변 텍셀의 중간 색을 가져옴

                [ MipMapping 을 사용하는 경우 ]
                    1. 선택한 레벨 내에서 텍셀을 어떻게 선택할 지 
                    2. MipMap 내에서 어떻게 level 을 선택할 지
                    모두 설정한다
                GL_NEAREST_MIPMAP_NEAREST
                                  그리려는 메쉬와 크기가 가장 가까운 레벨을 선택한 후
                    해당 레벨에서 가장 가까운 텍셀을 가져온다
                GL_LINEAR_MIPMAP_NEAREST
                                  그리려는 메쉬와 크기가 가장 가까운 레벨을 선택한 후
                    해당 레벨에서 텍셀간 linear 방식을 통해 색을 가져온다

                GL_LINEAR_MIPMAP_LINEAR
                                 텍스처링 하려는 메쉬와 크기가 가장 비슷한 레벨을 2개 선택한다
                   각 레벨에서 linear 방식을 통해 색을 결정한 뒤 (각 레벨에서 이중보간)
                                 두 레벨에서 나온 결과를 다시 linear 보간 해 최종 색을 결정한다
                                 = Trilinear Interpolation, 삼중 보간

                GL_NEAREST_MIPMAP_LINEAR
                                  텍스처링 하려는 메쉬와 그키가 가장 비슷한 레벨을 2개 선택한다
                    각 레벨에서 정점의 텍스처 좌표와 가장 가까운 텍셀을 선택하고
                                  두 레벨 결과를 linear 보간 해 최종 색을 결정한다

            GL_TEXTURE_MAG_FILTER
                텍스처가 메쉬에 비해서 너무 작아, 텍스처를 확대해야 하는 경우
                GL_NEAREST 
                    정점의 텍스처 좌표에서, 그나마 가장 가까이에 있는 텍셀의 색깔을 가져온다
                GL_LINEAR
                    정점의 텍스처 좌표를 이용, 선형보간을 통해서 주변 텍셀들의 중간 색을 가져온다
                    이중 보간
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}


void Texture::SetWrap(uint32_t sWrap, uint32_t tWrap) const
{
    /* 
        Texture Wrapping 설정
        텍스처 좌표가 0~1 을 벗어날 때, 어떻게 처리할 것인가
        < 가능한 설정 >
            GL_REPEAT
            GL_MIRRORED_REPEAT
            GL_CLAMP_TO_EDGE
            GL_CLAMP_TO_BORDER

        먼저 Bind 한 GL_TEXTURE_2D 텍스처에 접근
            GL_TEXTURE_WRAP_S
                텍스처의 가로 방향, S 축에 대해서 텍스처 좌표가
                범위를 벗어나는 경우 어떻게 처리할 것인지 설정
            GL_TEXTURE_WRAP_T
                텍스처의 세로 방향, T 축에 대해서 텍스처 좌표가 
                범위를 벗어나는 경우 어떻게 처리할 것인지 설정
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tWrap);
}


// gl Texture 를 만들기만 하고, 내부 이미지 데이터는 없다
void Texture::SetTextureFormat(int width, int height, uint32_t format, uint32_t type)
{
    m_width = width;
    m_height = height;
    m_format = format;
    m_type = type;

    glTexImage2D                // specify a two-dimensional texture image
    (
        GL_TEXTURE_2D,          // target
        // texture data
        0,                      // level
        m_format,               // inter format => 텍셀에 저장되는 포맷을 설정
        m_width,
        m_height,

        0,                      // boarder, must be zero

        m_format,               // image data 의 포맷, 여기서는 없으니까 저장할 포맷과 동일하게 설정
        m_type,                 // 포맷의 각 채널에 할당되는 data type
        // format * type => 한 텍셀의 사이즈가 결정된다, 여기서는 이미지가 없기 때문에
        // 타입을 지정한대로 텍셀의 크기가 할당될 것이다
        
        nullptr                 // data 가 없다, 메모리만 할당하고 Copy 는 하지 않는다
    );
}


// 이미지 데이터를, GL texture 에 채운다
void Texture::SetTextureFromImage(const Image* image)
{
    // 기본 channel count
    GLenum format = GL_RGBA;
    switch (image->GetChannelCount())
    {
        case 1: format = GL_RED; break;
        case 2: format = GL_RG; break;
        case 3: format = GL_RGB; break;
        default: break;
    }

    m_width = image->GetWidth();
    m_height = image->GetHeight();
    m_format = format;
    m_type = GL_UNSIGNED_BYTE;     // 이미지 파일의 경우, 픽셀이 담긴 데이터는 무조건 UNSIGNED_BYTE type 이다

    // gl Texture 의 메모리를 할당한다
    glTexImage2D
    (
        GL_TEXTURE_2D, 
        0,              // Target, texture level
        // GPU 쪽, 받는 GL texture obj 설정
        m_format,            // internal format ~ 이미지의 포맷을 텍스처에 어떤 포맷으로 저장할 것인가
        m_width,
        m_height,

        0,                  // boarder

        // CPU 쪽, 주는 이미지 데이터의 정보
        format,             // format
        m_type,             // type
        image->GetData()    // *data
    );
    // 이미지로 부터 텍스처를 만드는 경우에만, mipmap 을 만든다
    glGenerateMipmap(GL_TEXTURE_2D);
}


void Texture::SetBorderColor(const glm::vec4& color) const
{
    // texture 의 boader color 를 설정
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
}



/* ***************** */



// 큐브 맵 = 2D 이미지 * 6개
CubeTextureUPtr CubeTexture::CreateFromImages(const std::vector<Image*>& images)
{
    auto texture = CubeTextureUPtr(new CubeTexture());
    if (!texture->InitFromImages(images))
        return nullptr;
    return std::move(texture);
}


CubeTexture::~CubeTexture()
{
    if (m_texture)
    {
        glDeleteTextures(1, &m_texture);
    }
}


void CubeTexture::Bind() const
{
    // 현재 관리하는 텍스처를 CUBE MAP 으로 사용하겠다
    // GL 에서 관리하는 cube map texture 가 따로 존재한다
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
}


bool CubeTexture::InitFromImages(const std::vector<Image*>& images)
{
    glGenTextures(1, &m_texture);
    Bind();

        // 텍스처 파라미터 설정
        // Filtering
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Wrapping
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


        // 6개의 이미지에 접근
        for (uint32_t i = 0; i < (uint32_t)images.size(); i++)
        {
            auto image = images[i];
            GLenum format = GL_RGBA;
            switch (image->GetChannelCount())
            {
                default: break;
                case 1: format = GL_RED; break;
                case 2: format = GL_RG; break;
                case 3: format = GL_RGB; break;
            }


            // cube map texture 생성
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,     // GL_TEXTURE_CUBE_MAP 는 6 개로 구성되어 있다
                                                        // bind 는 한번만 하고 6 개에 접근 가능
                0, 
                GL_RGB,                     // RGB 데이터만 저장
                image->GetWidth(), 
                image->GetHeight(),
                
                0,

                format,
                GL_UNSIGNED_BYTE,           // 이미지 픽셀 => 각 채널 데이터는 항상 unsigned byte 에 담겨있다

                image->GetData()
            );
        }


    return true;
}