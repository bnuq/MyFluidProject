#include "mesh.h"

/* Material PUBLIC */
void Material::SetToProgram(const Program* program) const
{
    int textureCount = 0;   // 텍스처가 둘 중 하나만 존재할 수 있다
    if (diffuse)
    {
        // 텍스처 슬롯을 활성화
        glActiveTexture(GL_TEXTURE0 + textureCount);
            // program -> uniform variable 이 받을 텍스처 대신, 텍스처 슬롯 번호를 넣고
            program->SetUniform("material.diffuse", textureCount);
            diffuse->Bind();    // 들어가는 GL_TEXTURE_2D 를 Bind => Uniform Variable 로 들어가게 한다

        textureCount++;
    }
    if (specular)
    {
        glActiveTexture(GL_TEXTURE0 + textureCount);
            program->SetUniform("material.specular", textureCount); // 텍스처 슬롯 번호를 먼저 넣고
            specular->Bind();   // 들어가는 GL_TEXTURE_2D 를 Bind => Uniform Variable 로 들어가게 한다

        textureCount++;
    }
    glActiveTexture(GL_TEXTURE0);

    program->SetUniform("material.shininess", shininess);
}



/* Mesh PUBLIC */
MeshUPtr Mesh::Create (const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t primitiveType)
{
    auto mesh = MeshUPtr(new Mesh());
    mesh->Init(vertices, indices, primitiveType);
    return std::move(mesh);
}

MeshUPtr Mesh::CreateBox()
{
    std::vector<Vertex> vertices =
    {
        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f) },

        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 1.0f) },

        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 0.0f) },
    };
    std::vector<uint32_t> indices =
    {
        0,  2,  1,  2,  0,  3,
        4,  5,  6,  6,  7,  4,
        8,  9, 10, 10, 11,  8,
        12, 14, 13, 14, 12, 15,
        16, 17, 18, 18, 19, 16,
        20, 22, 21, 22, 20, 23,
    };
    return Create(vertices, indices, GL_TRIANGLES);
}

MeshUPtr Mesh::CreatePlane()        // 말이 plane 이지 quad 구나
{
    std::vector<Vertex> vertices =
    {
        Vertex { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
    };
    std::vector<uint32_t> indices =
    {
        0,  1,  2,  2,  3,  0,
    };
    return Create(vertices, indices, GL_TRIANGLES);
}

void Mesh::Draw(const Program* program) const
{
    m_vertexLayout->Bind(); // Program -> Vertex Shader 에서 정점마다 데이터를 읽어야 하므로
                            // 읽는 방법을 아는 VAO + 데이터가 저장된 VBO 를 Bind 한다
    if (m_material)
    {
        // Material -> 텍스처들도 있다면, Program -> Fragment Shader 의 Uniform Variable 로 연결한다
        m_material->SetToProgram(program);
    }

    // EBO 를 이용 -> 해당 프로그램에서 렌더링 진행
    glDrawElements(m_primitiveType, m_indexBuffer->GetCount(), GL_UNSIGNED_INT, 0);
}

void Mesh::GPUInstancingDraw(const Program* program, int num) const
{
    m_vertexLayout->Bind();

    if (m_material)
    {
        m_material->SetToProgram(program);
    }

    glDrawElementsInstanced(m_primitiveType, m_indexBuffer->GetCount(), GL_UNSIGNED_INT, 0, num);
}




/* Mesh PRIVATE */
void Mesh::Init
    (const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t primitiveType)
{
    // 먼저 VAO 를 Bind 하고
    m_vertexLayout = VertexLayout::Create();
        // VBO 생성 및 Bind => 데이터 읽기
        m_vertexBuffer = Buffer::CreateWithData
                        (GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices.data(), sizeof(Vertex), vertices.size());

        // EBO 생성 및 Bind => 데이터 읽기
        m_indexBuffer = Buffer::CreateWithData
                        (GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices.data(), sizeof(uint32_t), indices.size());

    // VBO 에 저장한 정점 데이터가 어떤 Attributes 로 이루어져 있는 지
    // 어떻게 해석해야 하는 지 지정
    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, false, sizeof(Vertex), 0);
    m_vertexLayout->SetAttrib(1, 3, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, normal));
    m_vertexLayout->SetAttrib(2, 2, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, texCoord));
}