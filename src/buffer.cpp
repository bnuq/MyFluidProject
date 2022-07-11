#include "buffer.h"

/* PUBLIC */
BufferUPtr Buffer::CreateWithData
(
    uint32_t bufferType,
    uint32_t usage,
    const void* data,
    size_t stride,
    size_t count
)
{
    auto buffer = BufferUPtr(new Buffer());

    if (!buffer->Init(bufferType, usage, data, stride, count))
        return nullptr;

    return std::move(buffer);
}

Buffer::~Buffer() 
{
    if (m_buffer)
    {
        glDeleteBuffers(1, &m_buffer);
    }
}

void Buffer::Bind() const
{   
    /* 
        ARRAY BUFFER    정점 데이터 저장
        ELEMENT BUFFER  인덱스 정보 저장
     */
    glBindBuffer(m_bufferType, m_buffer);
}


/* PRIVATE */
bool Buffer::Init
(
    uint32_t bufferType,    // GL_ARRAY_BUFFER | GL_ELEMENT_BUFFER , 만들려는 버퍼 종류 설정
    uint32_t usage,         /* 
                                저장한 데이터를 얼마나 바꾸는 지
                                STATIC, DYNAMIC, STREAM

                                사용 목적이 무엇인지
                                DRAW, COPY, READ
                            */
    const void* data,       // GPU 버퍼에 넣으려는, CPU 에 저장된 데이터 주소
    size_t stride,
    size_t count
)
{
    m_bufferType = bufferType;
    m_usage = usage;
    m_stride = stride;
    m_count = count;
    glGenBuffers(1, &m_buffer);

    Bind();

    glBufferData(m_bufferType, m_stride * m_count, data, usage);

    return true;
}