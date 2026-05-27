#include "loaders/ObjModel.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

static std::string ToLower(std::string text)
{
    for (char& character : text)
    {
        character = static_cast<char>(
            std::tolower(static_cast<unsigned char>(character))
            );
    }

    return text;
}

ObjModel::ObjModel(const std::string& path)
{
    if (!LoadWithAssimp(path))
    {
        std::cout << "Failed to load OBJ model: " << path << "\n";
    }
}

bool ObjModel::LoadWithAssimp(const std::string& path)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_FlipUVs
    );

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        std::cout << "Assimp error: "
            << importer.GetErrorString()
            << "\n";

        return false;
    }

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    glm::vec3 minBounds(std::numeric_limits<float>::max());
    glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

    triangles.clear();

    ProcessNode(
        scene->mRootNode,
        scene,
        vertices,
        indices,
        minBounds,
        maxBounds
    );

    if (vertices.empty() || indices.empty())
    {
        std::cout << "Model has no renderable geometry: "
            << path
            << "\n";

        return false;
    }

    ComputeNormalizationMatrix(minBounds, maxBounds);

    mesh = std::make_unique<Mesh>(vertices, indices, textures);

    std::cout << "OBJ loaded with Assimp: " << path << "\n";
    std::cout << "Meshes: " << scene->mNumMeshes << "\n";
    std::cout << "Vertices: " << vertices.size() << "\n";
    std::cout << "Indices: " << indices.size() << "\n";
    std::cout << "Ray triangles: " << triangles.size() << "\n";

    return true;
}

void ObjModel::ProcessNode(
    aiNode* node,
    const aiScene* scene,
    std::vector<Vertex>& vertices,
    std::vector<GLuint>& indices,
    glm::vec3& minBounds,
    glm::vec3& maxBounds
)
{
    const std::string nodeName = node->mName.C_Str();

    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];

        ProcessMesh(
            assimpMesh,
            scene,
            nodeName,
            vertices,
            indices,
            minBounds,
            maxBounds
        );
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(
            node->mChildren[i],
            scene,
            vertices,
            indices,
            minBounds,
            maxBounds
        );
    }
}

void ObjModel::ProcessMesh(
    aiMesh* assimpMesh,
    const aiScene* scene,
    const std::string& nodeName,
    std::vector<Vertex>& vertices,
    std::vector<GLuint>& indices,
    glm::vec3& minBounds,
    glm::vec3& maxBounds
)
{
    const GLuint baseVertex = static_cast<GLuint>(vertices.size());

    const std::string meshName = assimpMesh->mName.C_Str();
    const std::string groupName = meshName + " " + nodeName;

    const GroupMaterial groupMaterial =
        GetMaterialFromGroupName(groupName);

    std::cout << "Group: " << groupName
        << " | Material: " << groupMaterial.materialType
        << "\n";

    for (unsigned int i = 0; i < assimpMesh->mNumVertices; ++i)
    {
        Vertex vertex{};

        vertex.position =
        {
            assimpMesh->mVertices[i].x,
            assimpMesh->mVertices[i].y,
            assimpMesh->mVertices[i].z
        };

        minBounds = glm::min(minBounds, vertex.position);
        maxBounds = glm::max(maxBounds, vertex.position);

        if (assimpMesh->HasNormals())
        {
            vertex.normal =
            {
                assimpMesh->mNormals[i].x,
                assimpMesh->mNormals[i].y,
                assimpMesh->mNormals[i].z
            };

            if (glm::length(vertex.normal) > 0.0f)
            {
                vertex.normal = glm::normalize(vertex.normal);
            }
        }
        else
        {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (assimpMesh->HasTextureCoords(0))
        {
            vertex.texUV =
            {
                assimpMesh->mTextureCoords[0][i].x,
                assimpMesh->mTextureCoords[0][i].y
            };
        }
        else
        {
            vertex.texUV = glm::vec2(0.0f);
        }

        vertex.color = groupMaterial.albedo;

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < assimpMesh->mNumFaces; ++i)
    {
        const aiFace& face = assimpMesh->mFaces[i];

        if (face.mNumIndices != 3)
            continue;

        const GLuint i0 = baseVertex + face.mIndices[0];
        const GLuint i1 = baseVertex + face.mIndices[1];
        const GLuint i2 = baseVertex + face.mIndices[2];

        indices.push_back(i0);
        indices.push_back(i1);
        indices.push_back(i2);

        RayTriangle triangle{};

        triangle.v0 = vertices[i0].position;
        triangle.v1 = vertices[i1].position;
        triangle.v2 = vertices[i2].position;

        triangle.n0 = vertices[i0].normal;
        triangle.n1 = vertices[i1].normal;
        triangle.n2 = vertices[i2].normal;

        triangle.albedo = groupMaterial.albedo;
        triangle.materialType = groupMaterial.materialType;
        triangle.roughness = groupMaterial.roughness;
        triangle.ior = groupMaterial.ior;
        triangle.emissionStrength = groupMaterial.emissionStrength;

        triangles.push_back(triangle);
    }
}

ObjModel::GroupMaterial ObjModel::GetMaterialFromGroupName(
    const std::string& groupName
) const
{
    const std::string name = ToLower(groupName);

    GroupMaterial material{};

    // --------------------------------------------------
    // Light / emissive panel
    // --------------------------------------------------
    if (name.find("light") != std::string::npos &&
        name.find("sphere") == std::string::npos)
    {
        material.materialType = RAY_MATERIAL_LIGHT;
        material.albedo = glm::vec3(0.85f, 0.92f, 1.00f);
        material.emissionStrength = 5.5f;
        material.roughness = 0.0f;
        material.ior = 1.0f;
        return material;
    }

    // Metal
    if (name.find("tallbox") != std::string::npos ||
        name.find("tall_box") != std::string::npos ||
        name.find("metalbox") != std::string::npos ||
        name.find("metal_box") != std::string::npos ||
        name.find("box") != std::string::npos)
    {
        material.materialType = RAY_MATERIAL_METAL;
        material.albedo = glm::vec3(0.46f, 0.48f, 0.50f);
        material.roughness = 0.20f;
        material.ior = 1.0f;
        material.emissionStrength = 0.0f;
        return material;
    }

   
    // Glass sphere
    if (name.find("glasssphere") != std::string::npos ||
        name.find("glass_sphere") != std::string::npos ||
        name.find("sphere") != std::string::npos ||
        name.find("glass") != std::string::npos)
    {
        // Glass
        material.materialType = RAY_MATERIAL_GLASS;
        material.albedo = glm::vec3(0.985f, 0.992f, 1.00f);
        material.roughness = 0.0f;
        material.ior = 1.40f;
        material.emissionStrength = 0.0f;
        return material;
    }

    // --------------------------------------------------
    // Cornell Box walls
    // --------------------------------------------------
    if (name.find("leftwall") != std::string::npos ||
        name.find("left_wall") != std::string::npos)
    {
        material.materialType = RAY_MATERIAL_DIFFUSE;
        material.albedo = glm::vec3(0.65f, 0.05f, 0.04f);
        material.roughness = 1.0f;
        return material;
    }

    if (name.find("rightwall") != std::string::npos ||
        name.find("right_wall") != std::string::npos)
    {
        material.materialType = RAY_MATERIAL_DIFFUSE;
        material.albedo = glm::vec3(0.08f, 0.45f, 0.10f);
        material.roughness = 1.0f;
        return material;
    }

    if (name.find("floor") != std::string::npos ||
        name.find("ceiling") != std::string::npos ||
        name.find("backwall") != std::string::npos ||
        name.find("back_wall") != std::string::npos)
    {
        material.materialType = RAY_MATERIAL_DIFFUSE;
        material.albedo = glm::vec3(0.78f, 0.76f, 0.70f);
        material.roughness = 1.0f;
        return material;
    }

    // --------------------------------------------------
    // Fallback
    // --------------------------------------------------
    material.materialType = RAY_MATERIAL_DIFFUSE;
    material.albedo = glm::vec3(0.75f);
    material.roughness = 1.0f;
    material.ior = 1.0f;
    material.emissionStrength = 0.0f;

    return material;
}

void ObjModel::ComputeNormalizationMatrix(
    const glm::vec3& minBounds,
    const glm::vec3& maxBounds
)
{
    const glm::vec3 size = maxBounds - minBounds;
    const glm::vec3 center = (minBounds + maxBounds) * 0.5f;

    const float largestAxis =
        std::max(size.x, std::max(size.y, size.z));

    float scale = 1.0f;

    if (largestAxis > 0.0f)
    {
        scale = 1.8f / largestAxis;
    }

    normalizationMatrix = glm::mat4(1.0f);
    normalizationMatrix = glm::scale(normalizationMatrix, glm::vec3(scale));
    normalizationMatrix = glm::translate(normalizationMatrix, -center);
}

void ObjModel::Draw(
    Shader& shader,
    Camera& camera,
    glm::mat4 parentTransform
)
{
    if (!mesh)
        return;

    mesh->Draw(
        shader,
        camera,
        parentTransform * normalizationMatrix
    );
}

const std::vector<RayTriangle>& ObjModel::GetTriangles() const
{
    return triangles;
}
const glm::mat4& ObjModel::GetNormalizationMatrix() const
{
    return normalizationMatrix;
}