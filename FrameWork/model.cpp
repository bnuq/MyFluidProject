#include "model.h"

/* PUBLIC */
/* 
    자세하게는 잘 모르겠지만, assimp library 를 이용
    모델 파일 
        => 정점 배열과 인덱스 배열을 읽어오고
        => Mesh 를 만든다
        => VBO, EBO 를 채운다
        => 연결된 Material 이 있으면 Material 까지 읽어온다
 */
ModelUPtr Model::Load(const std::string& filename)
{
    auto model = ModelUPtr(new Model());
    if (!model->LoadByAssimp(filename))
        return nullptr;
    return std::move(model);
}

void Model::Draw(const Program* program) const
{
    for (auto& mesh: m_meshes)
    {
        // 주어진 program 에서 가지고 있는 Mesh 들을 하나하나 렌더링
        mesh->Draw(program);
    }
}


/* PRIVATE */
bool Model::LoadByAssimp(const std::string& filename)
{
    Assimp::Importer importer;
    auto scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        SPDLOG_ERROR("failed to load model: {}", filename);
        return false;
    }

    auto dirname = filename.substr(0, filename.find_last_of("/"));
    auto LoadTexture = [&](aiMaterial* material, aiTextureType type) -> TexturePtr 
    {
        if (material->GetTextureCount(type) <= 0)
            return nullptr;
        aiString filepath;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &filepath);
        auto image = Image::Load(fmt::format("{}/{}", dirname, filepath.C_Str()));
        if (!image)
            return nullptr;
        return Texture::CreateFromImage(image.get());
    };

    for (uint32_t i = 0; i < scene->mNumMaterials; i++)
    {
        auto material = scene->mMaterials[i];
        auto glMaterial = Material::Create();
        glMaterial->diffuse = LoadTexture(material, aiTextureType_DIFFUSE);
        glMaterial->specular = LoadTexture(material, aiTextureType_SPECULAR);
        m_materials.push_back(std::move(glMaterial));
    }

    ProcessNode(scene->mRootNode, scene);
    return true;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        auto meshIndex = node->mMeshes[i];
        auto mesh = scene->mMeshes[meshIndex];
        ProcessMesh(mesh, scene);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    SPDLOG_INFO("process mesh: {}, #vert: {}, #face: {}",
        mesh->mName.C_Str(), mesh->mNumVertices, mesh->mNumFaces);

    std::vector<Vertex> vertices;
    vertices.resize(mesh->mNumVertices);
    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        auto& v = vertices[i];
        v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        v.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    }

    std::vector<uint32_t> indices;
    indices.resize(mesh->mNumFaces * 3);
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        indices[3*i  ] = mesh->mFaces[i].mIndices[0];
        indices[3*i+1] = mesh->mFaces[i].mIndices[1];
        indices[3*i+2] = mesh->mFaces[i].mIndices[2];
    }

    auto glMesh = Mesh::Create(vertices, indices, GL_TRIANGLES);
    if (mesh->mMaterialIndex >= 0)
        glMesh->SetMaterial(m_materials[mesh->mMaterialIndex]);
    m_meshes.push_back(std::move(glMesh));
}