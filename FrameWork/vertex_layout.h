#ifndef __VERTEX_LAYOUT_H__
#define __VERTEX_LAYOUT_H__

#include "common.h"

CLASS_PTR(VertexLayout)
class VertexLayout
{
public:
    static VertexLayoutUPtr Create();
    ~VertexLayout();
    void Bind() const;

    uint32_t Get() const { return m_vertexArrayObject; }
    void SetAttrib(uint32_t attribIndex, int count, uint32_t type, bool normalized, size_t stride, uint64_t offset) const;
    //void DisableAttrib(int attribIndex) const;

private:
    VertexLayout() {}
    void Init();
    
    uint32_t m_vertexArrayObject { 0 }; // gl vao id
};

#endif // __VERTEX_LAYOUT_H__