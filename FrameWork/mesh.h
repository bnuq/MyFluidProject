#ifndef __MESH_H__
#define __MESH_H__

#include "common.h"
#include "buffer.h"
#include "vertex_layout.h"
#include "texture.h"
#include "program.h"


// 한 정점에 저장된 정보
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};



// 한 Mesh 가 가지는 텍스처들 = Material
CLASS_PTR(Material)
class Material
{
public:
    static MaterialUPtr Create() { return MaterialUPtr(new Material()); }
    void SetTextures(TexturePtr diff, TexturePtr spec)
    {
        diffuse = diff;
        specular = spec;
    }

    /* 
        Material 이 적용되려면 Fragment Shader 에 전달되어야 한다
        적용될 Fragment Shader -> 가 포함된 Program 을 지정해서 사용
     */
    void SetToProgram(const Program* program) const;

    
    // 2 개의 텍스처들로 구성
    TexturePtr diffuse;         // 1. Diffuse Texture
    TexturePtr specular;        // 2. Specular Texture
    float shininess { 32.0f };

private:
    Material() {}
};



// 하나의 모델 = 하나의 Mesh
CLASS_PTR(Mesh)
class Mesh
{
public:
    static MeshUPtr Create
    (
        const std::vector<Vertex>& vertices,        // Mesh 를 구성하는 정점 배열
        const std::vector<uint32_t>& indices,       // 정점 배열의 순서를 나타내는 인덱스 배열
        uint32_t primitiveType                      // Mesh 를 구성하는 기본 도형이 무엇인지
    );
    static MeshUPtr CreateBox();
    static MeshUPtr CreatePlane();

    void Draw(const Program*) const;
    void GPUInstancingDraw(const Program* program, int num) const;


    const VertexLayout* GetVertexLayout() const { return m_vertexLayout.get(); }
    BufferPtr GetVertexBuffer() const { return m_vertexBuffer; }
    BufferPtr GetIndexBuffer() const { return m_indexBuffer; }
    MaterialPtr GetMaterial() const { return m_material; }
    void SetMaterial(MaterialPtr material) { m_material = material; }

private:
    Mesh() {}

    void Init
    (
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices,
        uint32_t primitiveType
    );


    uint32_t m_primitiveType { GL_TRIANGLES };      // 도형을 이루는 기본 도형
    VertexLayoutUPtr m_vertexLayout;                // 각 정점에 저장된 Attributes 를 읽기 위한 VAO
    BufferPtr m_vertexBuffer;                       // Mesh 의 정점들에 저장된 데이터들을 저장하는 버퍼, VBO
    BufferPtr m_indexBuffer;                        // Mesh 의 인덱스 배열에 저장된 데이터들을 저장하는 버퍼, EBO

    MaterialPtr m_material;                         // Mesh 에 적용되는 텍스처들
};

#endif // __MESH_H__