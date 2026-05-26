#pragma once

#include <memory>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "camera/Camera.h"
#include "rendering/Mesh.h"
#include "rendering/Shader.h"

enum RayMaterialType
{
    RAY_MATERIAL_DIFFUSE = 0,
    RAY_MATERIAL_METAL = 1,
    RAY_MATERIAL_GLASS = 2,
    RAY_MATERIAL_LIGHT = 3
};

struct RayTriangle
{
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;

    glm::vec3 n0;
    glm::vec3 n1;
    glm::vec3 n2;

    glm::vec3 albedo = glm::vec3(1.0f);

    int materialType = RAY_MATERIAL_DIFFUSE;

    float roughness = 1.0f;
    float ior = 1.0f;
    float emissionStrength = 0.0f;
};

class ObjModel
{
public:
    ObjModel(const std::string& path);

    void Draw(
        Shader& shader,
        Camera& camera,
        glm::mat4 parentTransform = glm::mat4(1.0f)
    );

    const std::vector<RayTriangle>& GetTriangles() const;
    const glm::mat4& GetNormalizationMatrix() const;

private:
    struct GroupMaterial
    {
        glm::vec3 albedo = glm::vec3(1.0f);

        int materialType = RAY_MATERIAL_DIFFUSE;

        float roughness = 1.0f;
        float ior = 1.0f;
        float emissionStrength = 0.0f;
    };

private:
    bool LoadWithAssimp(const std::string& path);

    void ProcessNode(
        struct aiNode* node,
        const struct aiScene* scene,
        std::vector<Vertex>& vertices,
        std::vector<GLuint>& indices,
        glm::vec3& minBounds,
        glm::vec3& maxBounds
    );

    void ProcessMesh(
        struct aiMesh* assimpMesh,
        const struct aiScene* scene,
        const std::string& nodeName,
        std::vector<Vertex>& vertices,
        std::vector<GLuint>& indices,
        glm::vec3& minBounds,
        glm::vec3& maxBounds
    );

    GroupMaterial GetMaterialFromGroupName(const std::string& groupName) const;

    void ComputeNormalizationMatrix(
        const glm::vec3& minBounds,
        const glm::vec3& maxBounds
    );

private:
    std::unique_ptr<Mesh> mesh;

    glm::mat4 normalizationMatrix = glm::mat4(1.0f);

    std::vector<RayTriangle> triangles;
};