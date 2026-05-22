#include "ObjModel.h"

#include <iostream>
#include <limits>
#include <algorithm>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

ObjModel::ObjModel(const std::string& path)
{
    if (!LoadWithAssimp(path))
    {
        std::cout << "ERROR: No se pudo cargar el modelo con Assimp: "
            << path << std::endl;
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
        aiProcess_ImproveCacheLocality
    );

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        std::cout << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    vertices.reserve(100000);
    indices.reserve(300000);
    triangles.reserve(100000);

    glm::vec3 minBounds(std::numeric_limits<float>::max());
    glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

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
        std::cout << "Assimp cargo el archivo, pero no encontro geometria renderizable: "
            << path << std::endl;
        return false;
    }

    // No normalize.
    // The model must keep its original OBJ coordinates so it matches Python.
    normalizationMatrix = glm::mat4(1.0f);

    mesh = std::make_unique<Mesh>(vertices, indices, textures);

    glm::vec3 center = (minBounds + maxBounds) * 0.5f;
    glm::vec3 size = maxBounds - minBounds;

    std::cout << "OBJ cargado correctamente con Assimp: " << path << std::endl;
    std::cout << "Meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "Vertices unicos/render: " << vertices.size() << std::endl;
    std::cout << "Indices: " << indices.size() << std::endl;
    std::cout << "Triangulos para raytracing: " << triangles.size() << std::endl;

    std::cout << "Bounds min: "
        << minBounds.x << ", " << minBounds.y << ", " << minBounds.z << std::endl;

    std::cout << "Bounds max: "
        << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << std::endl;

    std::cout << "Centro modelo: "
        << center.x << ", " << center.y << ", " << center.z << std::endl;

    std::cout << "Tamano modelo: "
        << size.x << ", " << size.y << ", " << size.z << std::endl;

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
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];

        ProcessMesh(
            assimpMesh,
            scene,
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
    std::vector<Vertex>& vertices,
    std::vector<GLuint>& indices,
    glm::vec3& minBounds,
    glm::vec3& maxBounds
)
{
    const GLuint baseVertex = static_cast<GLuint>(vertices.size());

    std::string meshName = assimpMesh->mName.C_Str();
    glm::vec3 meshColor = GetColorForMeshName(meshName);

    std::cout << "Mesh: " << meshName << std::endl;

    for (unsigned int i = 0; i < assimpMesh->mNumVertices; ++i)
    {
        Vertex vertex{};

        vertex.position = glm::vec3(
            assimpMesh->mVertices[i].x,
            assimpMesh->mVertices[i].y,
            assimpMesh->mVertices[i].z
        );

        minBounds = glm::min(minBounds, vertex.position);
        maxBounds = glm::max(maxBounds, vertex.position);

        if (assimpMesh->HasNormals())
        {
            vertex.normal = glm::vec3(
                assimpMesh->mNormals[i].x,
                assimpMesh->mNormals[i].y,
                assimpMesh->mNormals[i].z
            );

            if (glm::length(vertex.normal) > 0.0f)
                vertex.normal = glm::normalize(vertex.normal);
        }
        else
        {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (assimpMesh->HasTextureCoords(0))
        {
            vertex.texUV = glm::vec2(
                assimpMesh->mTextureCoords[0][i].x,
                assimpMesh->mTextureCoords[0][i].y
            );
        }
        else
        {
            vertex.texUV = glm::vec2(0.0f);
        }

        // We ignore MTL. Color comes from group/mesh name.
        vertex.color = meshColor;

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < assimpMesh->mNumFaces; ++i)
    {
        const aiFace& face = assimpMesh->mFaces[i];

        if (face.mNumIndices != 3)
            continue;

        GLuint i0 = baseVertex + face.mIndices[0];
        GLuint i1 = baseVertex + face.mIndices[1];
        GLuint i2 = baseVertex + face.mIndices[2];

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

        triangles.push_back(triangle);
    }
}

glm::vec3 ObjModel::GetColorForMeshName(const std::string& meshName) const
{
    if (meshName.find("LeftWall") != std::string::npos)
        return glm::vec3(0.72f, 0.06f, 0.04f);

    if (meshName.find("RightWall") != std::string::npos)
        return glm::vec3(0.05f, 0.48f, 0.10f);

    if (meshName.find("Light") != std::string::npos)
        return glm::vec3(1.0f, 0.92f, 0.62f);

    if (meshName.find("RightSphere") != std::string::npos)
        return glm::vec3(0.78f, 0.78f, 0.75f);

    if (meshName.find("TallBox") != std::string::npos)
        return glm::vec3(0.68f, 0.65f, 0.58f);

    if (meshName.find("Floor") != std::string::npos)
        return glm::vec3(0.78f, 0.76f, 0.68f);

    if (meshName.find("Ceiling") != std::string::npos)
        return glm::vec3(0.78f, 0.76f, 0.68f);

    if (meshName.find("BackWall") != std::string::npos)
        return glm::vec3(0.78f, 0.76f, 0.68f);

    return glm::vec3(0.75f, 0.73f, 0.66f);
}

void ObjModel::Draw(
    Shader& shader,
    Camera& camera,
    glm::mat4 parentTransform
)
{
    if (mesh)
    {
        mesh->Draw(
            shader,
            camera,
            parentTransform * normalizationMatrix
        );
    }
}

const std::vector<RayTriangle>& ObjModel::GetTriangles() const
{
    return triangles;
}