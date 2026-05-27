#pragma once

#include <string>
#include <memory>
#include <vector>

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "rendering/Mesh.h"
#include "camera/Camera.h"
#include "rendering/Shader.h"

struct RayTriangle
{
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;

    glm::vec3 n0;
    glm::vec3 n1;
    glm::vec3 n2;
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

    bool HasTextureCoordinates() const;

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
        std::vector<Vertex>& vertices,
        std::vector<GLuint>& indices,
        glm::vec3& minBounds,
        glm::vec3& maxBounds
    );

    void ComputeNormalizationMatrix(
        const glm::vec3& minBounds,
        const glm::vec3& maxBounds
    );

private:
    std::unique_ptr<Mesh> mesh;

    glm::mat4 normalizationMatrix = glm::mat4(1.0f);

    std::vector<RayTriangle> triangles;

    // Si el modelo trae UVs, se usan en el shader.
    // Si no trae UVs, el shader usa triplanar mapping.
    bool hasTextureCoordinates = false;
};