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


void VertexLayout::SetAttrib // ? •?  ?°?´?„° ì¤? ?•˜?‚˜?¸ Attribute ?— ????•œ ? •ë³´ë?? ì§?? •
(
    uint32_t attribIndex,   // Attribute Index = GL VAO ?˜ ëª? ë²ˆì§¸ attribute pointer ?— ????¥?„ ?•  ì§?
    int count,              // ?•´?‹¹ attribute ê°? ëª? ê°œì˜ ?°?´?„°ë¡? êµ¬ì„±?˜?Š” ì§?
    uint32_t type,          // ?°?´?„°?˜ ????…??? ë¬´ì—‡?¸ì§?
    bool normalized,        // attribute ê°? normalized ??Š” ì§?
    size_t stride,          // ?‹¤?Œ ? •? ?˜ attribute ë¥? ?½ê¸? ?œ„?•´?„œ, ?–¼ë§ˆë‚˜ ?´?™?•´?•¼ ?•˜?Š” ì§?
    uint64_t offset         // ì²˜ìŒ ? •? ?˜ attribute ë¥? ?½?œ¼? ¤ë©?, ?–´?””?„œë¶??„° ?½?–´?•¼ ?•˜?Š” ì§?
) const
{
    /*
        glEnableVertexAttribArray
            ?˜„?¬ bind ?•œ GL VAO ?—?„œ 
            ëª? ë²ˆì§¸ attrubute pointer ë¥? ?‚¬?š©?•˜ê² ë‹¤?Š” ê²ƒì„ ?•Œë¦°ë‹¤

        ?„¸?Œ…?•˜ê³ ì ?•˜?Š” attribute pointer ë²ˆí˜¸ë¥? ì§?? •
        ?•´?‹¹ attribute pointer ë¥? ?‚¬?š©?•˜ê² ë‹¤ê³? ?•Œë¦?
     */
    glEnableVertexAttribArray(attribIndex);
    /* 
        ?•´?‹¹ attribute pointer ?— vertex attribute ë¥? ?½?Š” ë°©ë²•?„ ?„¤? •
        ?´? œ ë²„í¼?—?„œ, ê°? ? •? ?“¤ë§ˆë‹¤ ê°?ì§?ê³? ?ˆ?Š” vertex attribute ë¥? ?½?–´?‚¼ ?ˆ˜ ?ˆ?‹¤
     */
    glVertexAttribPointer(attribIndex, count, type, normalized, stride, (const void*)offset);
}