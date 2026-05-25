 #include "debug/Helpers.h"
#include "scene/SceneConfig.h"
#include <cmath>
#include <gtc/matrix_transform.hpp>

DebugHelpers::~DebugHelpers()
{
    Shutdown();
}

void DebugHelpers::Initialize()
{
    if (VAO != 0 || VBO != 0)
        return;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // position
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(DebugVertex),
        reinterpret_cast<void*>(offsetof(DebugVertex, position))
    );
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(DebugVertex),
        reinterpret_cast<void*>(offsetof(DebugVertex, color))
    );
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void DebugHelpers::Shutdown()
{
    if (VBO != 0)
    {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }

    if (VAO != 0)
    {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
}

void DebugHelpers::Begin()
{
    vertices.clear();
}

void DebugHelpers::AddLine(
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& color
)
{
    vertices.push_back({ a, color });
    vertices.push_back({ b, color });
}

void DebugHelpers::AddCross(
    const glm::vec3& position,
    float size,
    const glm::vec3& color
)
{
    AddLine(
        position + glm::vec3(-size, 0.0f, 0.0f),
        position + glm::vec3(size, 0.0f, 0.0f),
        color
    );

    AddLine(
        position + glm::vec3(0.0f, -size, 0.0f),
        position + glm::vec3(0.0f, size, 0.0f),
        color
    );

    AddLine(
        position + glm::vec3(0.0f, 0.0f, -size),
        position + glm::vec3(0.0f, 0.0f, size),
        color
    );
}

void DebugHelpers::AddWireSphere(
    const glm::vec3& center,
    float radius,
    const glm::vec3& color,
    int segments
)
{
    const float twoPi = 6.28318530718f;

    for (int i = 0; i < segments; ++i)
    {
        float a0 = twoPi * static_cast<float>(i) / static_cast<float>(segments);
        float a1 = twoPi * static_cast<float>(i + 1) / static_cast<float>(segments);

        // XY circle
        AddLine(
            center + glm::vec3(std::cos(a0) * radius, std::sin(a0) * radius, 0.0f),
            center + glm::vec3(std::cos(a1) * radius, std::sin(a1) * radius, 0.0f),
            color
        );

        // XZ circle
        AddLine(
            center + glm::vec3(std::cos(a0) * radius, 0.0f, std::sin(a0) * radius),
            center + glm::vec3(std::cos(a1) * radius, 0.0f, std::sin(a1) * radius),
            color
        );

        // YZ circle
        AddLine(
            center + glm::vec3(0.0f, std::cos(a0) * radius, std::sin(a0) * radius),
            center + glm::vec3(0.0f, std::cos(a1) * radius, std::sin(a1) * radius),
            color
        );
    }
}

void DebugHelpers::AddLightHelper(
    const glm::vec3& position,
    float size,
    const glm::vec3& color
)
{
    AddCross(position, size, color);
    AddWireSphere(position, size * 0.65f, color, 24);
}

void DebugHelpers::AddCameraFrustum(
    const glm::vec3& position,
    const glm::vec3& forward,
    const glm::vec3& up,
    float fovDegrees,
    float aspectRatio,
    float nearDistance,
    float farDistance,
    const glm::vec3& color
)
{
    glm::vec3 f = glm::normalize(forward);
    glm::vec3 r = glm::normalize(glm::cross(f, up));
    glm::vec3 u = glm::normalize(glm::cross(r, f));

    float fovRadians = glm::radians(fovDegrees);

    float nearHeight = 2.0f * std::tan(fovRadians * 0.5f) * nearDistance;
    float nearWidth = nearHeight * aspectRatio;

    float farHeight = 2.0f * std::tan(fovRadians * 0.5f) * farDistance;
    float farWidth = farHeight * aspectRatio;

    glm::vec3 nearCenter = position + f * nearDistance;
    glm::vec3 farCenter = position + f * farDistance;

    glm::vec3 ntl = nearCenter + u * (nearHeight * 0.5f) - r * (nearWidth * 0.5f);
    glm::vec3 ntr = nearCenter + u * (nearHeight * 0.5f) + r * (nearWidth * 0.5f);
    glm::vec3 nbl = nearCenter - u * (nearHeight * 0.5f) - r * (nearWidth * 0.5f);
    glm::vec3 nbr = nearCenter - u * (nearHeight * 0.5f) + r * (nearWidth * 0.5f);

    glm::vec3 ftl = farCenter + u * (farHeight * 0.5f) - r * (farWidth * 0.5f);
    glm::vec3 ftr = farCenter + u * (farHeight * 0.5f) + r * (farWidth * 0.5f);
    glm::vec3 fbl = farCenter - u * (farHeight * 0.5f) - r * (farWidth * 0.5f);
    glm::vec3 fbr = farCenter - u * (farHeight * 0.5f) + r * (farWidth * 0.5f);

    // Near plane
    AddLine(ntl, ntr, color);
    AddLine(ntr, nbr, color);
    AddLine(nbr, nbl, color);
    AddLine(nbl, ntl, color);

    // Far plane
    AddLine(ftl, ftr, color);
    AddLine(ftr, fbr, color);
    AddLine(fbr, fbl, color);
    AddLine(fbl, ftl, color);

    // Connections
    AddLine(ntl, ftl, color);
    AddLine(ntr, ftr, color);
    AddLine(nbl, fbl, color);
    AddLine(nbr, fbr, color);

    // Direction line
    AddLine(position, position + f * 0.35f, color);

    // Small camera body marker
    AddCross(position, 0.04f, color);
}

void DebugHelpers::Draw(Shader& shader, Camera& camera)
{
    if (vertices.empty())
        return;

    shader.Activate();

    camera.Matrix(shader, "camMatrix");

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(DebugVertex),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );

    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
