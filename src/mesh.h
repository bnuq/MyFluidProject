#ifndef __MESH_H__
#define __MESH_H__

#include "common.h"
#include "buffer.h"
#include "vertex_layout.h"
#include "texture.h"
#include "program.h"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

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
    void SetToProgram(const Program* program) const;

    TexturePtr diffuse;
    TexturePtr specular;
    float shininess { 32.0f };

private:
    Material() {}
};

CLASS_PTR(Mesh)
class Mesh
{
public:
    static MeshUPtr Create
    (
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices,
        uint32_t primitiveType
    );
    static MeshUPtr CreateBox();
    static MeshUPtr CreatePlane();


    void Draw(const Program*) const;

    // Mesh 를 GPU Instancing 으로 그리는 메서드
    void GPUInstancingDraw(const Program*, size_t num) const;



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

    uint32_t m_primitiveType { GL_TRIANGLES };
    VertexLayoutUPtr m_vertexLayout;
    BufferPtr m_vertexBuffer;
    BufferPtr m_indexBuffer;
    MaterialPtr m_material;
};

#endif // __MESH_H__