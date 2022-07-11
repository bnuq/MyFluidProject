#include "vertex_layout.h"

VertexLayoutUPtr VertexLayout::Create()
{
    auto vertexLayout = VertexLayoutUPtr(new VertexLayout());
    vertexLayout->Init();
    return std::move(vertexLayout);
}

void VertexLayout::Init()
{
    glGenVertexArrays(1, &m_vertexArrayObject);
    Bind();
}

void VertexLayout::Bind() const
{
    glBindVertexArray(m_vertexArrayObject);
}

VertexLayout::~VertexLayout()
{
    if (m_vertexArrayObject)
    {
        glDeleteVertexArrays(1, &m_vertexArrayObject);
    }
}


void VertexLayout::SetAttrib // ? ?  ?°?΄?° μ€? ???Έ Attribute ? ???? ? λ³΄λ?? μ§?? 
(
    uint32_t attribIndex,   // Attribute Index = GL VAO ? λͺ? λ²μ§Έ attribute pointer ? ????₯? ?  μ§?
    int count,              // ?΄?Ή attribute κ°? λͺ? κ°μ ?°?΄?°λ‘? κ΅¬μ±?? μ§?
    uint32_t type,          // ?°?΄?°? ??????? λ¬΄μ?Έμ§?
    bool normalized,        // attribute κ°? normalized ?? μ§?
    size_t stride,          // ?€? ? ? ? attribute λ₯? ?½κΈ? ??΄?, ?Όλ§λ ?΄??΄?Ό ?? μ§?
    uint64_t offset         // μ²μ ? ? ? attribute λ₯? ?½?Ό? €λ©?, ?΄??λΆ??° ?½?΄?Ό ?? μ§?
) const
{
    /*
        glEnableVertexAttribArray
            ??¬ bind ? GL VAO ?? 
            λͺ? λ²μ§Έ attrubute pointer λ₯? ?¬?©?κ² λ€? κ²μ ?λ¦°λ€

        ?Έ??κ³ μ ?? attribute pointer λ²νΈλ₯? μ§?? 
        ?΄?Ή attribute pointer λ₯? ?¬?©?κ² λ€κ³? ?λ¦?
     */
    glEnableVertexAttribArray(attribIndex);
    /* 
        ?΄?Ή attribute pointer ? vertex attribute λ₯? ?½? λ°©λ²? ?€? 
        ?΄?  λ²νΌ??, κ°? ? ? ?€λ§λ€ κ°?μ§?κ³? ?? vertex attribute λ₯? ?½?΄?Ό ? ??€
     */
    glVertexAttribPointer(attribIndex, count, type, normalized, stride, (const void*)offset);
}