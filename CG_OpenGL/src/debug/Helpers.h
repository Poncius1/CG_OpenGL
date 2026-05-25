#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm.hpp>

#include "camera/Camera.h"
#include "rendering/Shader.h"

struct DebugVertex
{
    glm::vec3 position;
    glm::vec3 color;
};

class DebugHelpers
{
public:
    DebugHelpers() = default;
    ~DebugHelpers();

    void Initialize();
    void Shutdown();

    void Begin();

    void AddLine(
        const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& color
    );

    void AddCross(
        const glm::vec3& position,
        float size,
        const glm::vec3& color
    );

    void AddWireSphere(
        const glm::vec3& center,
        float radius,
        const glm::vec3& color,
        int segments = 32
    );

    void AddLightHelper(
        const glm::vec3& position,
        float size,
        const glm::vec3& color
    );

    void AddCameraFrustum(
        const glm::vec3& position,
        const glm::vec3& forward,
        const glm::vec3& up,
        float fovDegrees,
        float aspectRatio,
        float nearDistance,
        float farDistance,
        const glm::vec3& color
    );

    void Draw(Shader& shader, Camera& camera);

private:
    GLuint VAO = 0;
    GLuint VBO = 0;

    std::vector<DebugVertex> vertices;
};