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
    /* 
        glBindVertexArray
            지금부터 사용하고자 하는 GL VAO 를 설정하는 함수
            따로 말하지 않아도 지금부터는 이것을 사용하겠다 => Bind 한다고 말한다

        VertexLayout 객체가 관리하는 GL VAO 를 지금부터 사용 하겠다
        
        사용할 GL VAO 를 먼저 정해놓고 다음 과정을 진행해야 한다
        
        자연스럽게 다음에 등장하는 GL VBO 는, 이 GL VAO 에 의해서 해석되는 버퍼가 된다
            반드시 vao 를 먼저 지정한 후, vbo 를 지정해야 한다

        GL VBO = 바구니, 데이터를 저장
                 정점 하나에 대한 다양한 데이터들이 그냥 한꺼번에 저장되어 있다
                 그리고 정점들에 대한 데이터가 구분없이, 일렬로 저장되어 있다
                 한 정점에 저장된 여러 데이터 들 = Attributes 로 구분
                 Vertex Attribute 라 한다

        GL VAO = 설명서, 바구니에 담긴 데이터를 해석하는 방법
                 Attribute 를 읽는 방법을 알려준다
     */
    glBindVertexArray(m_vertexArrayObject);
}


VertexLayout::~VertexLayout()
{
    if (m_vertexArrayObject)
    {
        glDeleteVertexArrays(1, &m_vertexArrayObject);
    }
}


/*
    Attribute Format 을 알려주는 함수
    알려준 Format 은 GL VAO 의 Attribute Pointer 에 저장하여
    버퍼에서 각 vertex attribute 를 읽을 수 있도록 한다
 */
void VertexLayout::SetAttrib // 정점 데이터 중 하나인 Attribute 에 대한 정보를 지정
(
    uint32_t attribIndex,   // Attribute Index = GL VAO 의 몇 번째 attribute pointer 에 저장을 할 지
    int count,              // 해당 attribute 가 몇 개의 데이터로 구성되는 지
    uint32_t type,          // 데이터의 타입은 무엇인지
    bool normalized,        // attribute 가 normalized 됐는 지
    size_t stride,          // 다음 정점의 attribute 를 읽기 위해서, 얼마나 이동해야 하는 지
    uint64_t offset         // 처음 정점의 attribute 를 읽으려면, 어디서부터 읽어야 하는 지
) const
{
    /*
        glEnableVertexAttribArray
            현재 bind 한 GL VAO 에서 
            몇 번째 attrubute pointer 를 사용하겠다는 것을 알린다

        세팅하고자 하는 attribute pointer 번호를 지정
        해당 attribute pointer 를 사용하겠다고 알림
     */
    glEnableVertexAttribArray(attribIndex);
    /* 
        해당 attribute pointer 에 vertex attribute 를 읽는 방법을 설정
        이제 버퍼에서, 각 정점들마다 가지고 있는 vertex attribute 를 읽어낼 수 있다
     */
    glVertexAttribPointer(attribIndex, count, type, normalized, stride, (const void*)offset);
}