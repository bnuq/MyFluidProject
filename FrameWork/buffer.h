#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "common.h"

CLASS_PTR(Buffer)

class Buffer
{
public:
    static BufferUPtr CreateWithData(
            uint32_t bufferType, 
            uint32_t usage, 
            const void* data, 
            size_t stride, 
            size_t count
        );

    ~Buffer();
    void Bind() const;

    uint32_t Get() const { return m_buffer; }
    size_t GetStride() const { return m_stride; }
    size_t GetCount() const { return m_count; }

private:
    Buffer() {}
    bool Init(uint32_t bufferType, uint32_t usage, const void* data, size_t stride, size_t count);

    uint32_t m_buffer { 0 };        // gl buffer id
    uint32_t m_bufferType { 0 };    // vertex buffer | element buffer
    uint32_t m_usage { 0 };         // 빈도 수 + 데이터 사용 목적
    size_t m_stride { 0 };          // 버퍼에 저장하는 데이터 크기, 다음 요소에 접근하기 위해 얼마나 건너뛰어야 하는 가
    size_t m_count { 0 };           // 버퍼에 저장하는 데이터 개수, 몇 개를 저장하는가
};

#endif // __BUFFER_H__