#include "rl-visualizer/trajectoryRenderer.h"

#include <algorithm>
#include <iostream>

TrajectoryRenderer::~TrajectoryRenderer()
{
    Shutdown();
}

void TrajectoryRenderer::Initialize()
{
    if (vao != 0 || vbo != 0)
    {
        return;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
}

void TrajectoryRenderer::Shutdown()
{
    if (vbo != 0)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if (vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
}

void TrajectoryRenderer::SetPaths(
    const std::vector<TrajectoryPath>* randomPaths,
    const std::vector<TrajectoryPath>* lightBiasedPaths,
    const std::vector<TrajectoryPath>* pqlPaths
)
{
    this->randomPaths = randomPaths;
    this->lightBiasedPaths = lightBiasedPaths;
    this->pqlPaths = pqlPaths;

    std::cout << "[TrajectoryRenderer] Random: "
        << (randomPaths ? randomPaths->size() : 0)
        << ", Light-biased: "
        << (lightBiasedPaths ? lightBiasedPaths->size() : 0)
        << ", PQL: "
        << (pqlPaths ? pqlPaths->size() : 0)
        << std::endl;
}

void TrajectoryRenderer::Draw(Shader& shader, Camera& camera)
{
    if (vao == 0 || vbo == 0)
    {
        return;
    }

    shader.Activate();
    camera.Matrix(shader, "camMatrix");

    glUniform1f(glGetUniformLocation(shader.ID, "uAlpha"), 0.72f);

    const GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
    const GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vao);
    glLineWidth(1.8f);

    if (showRandom && randomPaths)
    {
        DrawPathCollection(*randomPaths);
    }

    if (showLightBiased && lightBiasedPaths)
    {
        DrawPathCollection(*lightBiasedPaths);
    }

    if (showPql && pqlPaths)
    {
        DrawPathCollection(*pqlPaths);
    }

    glBindVertexArray(0);

    if (depthWasEnabled) glEnable(GL_DEPTH_TEST);
    if (!blendWasEnabled) glDisable(GL_BLEND);
    if (cullWasEnabled) glEnable(GL_CULL_FACE);
}

void TrajectoryRenderer::DrawPathCollection(const std::vector<TrajectoryPath>& paths)
{
    if (paths.empty())
    {
        return;
    }

    std::vector<const TrajectoryPath*> selected;
    selected.reserve(paths.size());

    for (const TrajectoryPath& path : paths)
    {
        if (path.hitLight)
        {
            selected.push_back(&path);
        }
    }

    for (const TrajectoryPath& path : paths)
    {
        if (!path.hitLight)
        {
            selected.push_back(&path);
        }
    }

    const int count = std::min<int>(
        static_cast<int>(selected.size()),
        maxVisiblePaths
    );

    for (int i = 0; i < count; ++i)
    {
        const TrajectoryPath& path = *selected[i];

        if (path.points.size() < 2)
        {
            continue;
        }

        if (drawOnlyFirstSegment)
        {
            DrawFirstSegment(path);
        }
        else
        {
            DrawPath(path);
        }

        if (path.hitLight)
        {
            DrawHitMarker(path.points.back());
        }
    }
}

void TrajectoryRenderer::DrawPath(const TrajectoryPath& path)
{
    const glm::vec3 color = GetPathColor(path);

    for (size_t i = 0; i + 1 < path.points.size(); ++i)
    {
        DrawLine(path.points[i], path.points[i + 1], color);
    }
}

void TrajectoryRenderer::DrawFirstSegment(const TrajectoryPath& path)
{
    const glm::vec3 origin = path.points[0];
    glm::vec3 direction = path.points[1] - origin;

    const float length = glm::length(direction);

    if (length < 1e-5f)
    {
        return;
    }

    direction = glm::normalize(direction);

    DrawLine(
        origin,
        origin + direction * firstSegmentLength,
        GetPathColor(path)
    );
}

void TrajectoryRenderer::DrawLine(
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& color
)
{
    const float vertices[] =
    {
        a.x, a.y, a.z, color.r, color.g, color.b,
        b.x, b.y, b.z, color.r, color.g, color.b
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(sizeof(vertices)),
        vertices,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        reinterpret_cast<void*>(0)
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        reinterpret_cast<void*>(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_LINES, 0, 2);
}

void TrajectoryRenderer::DrawHitMarker(const glm::vec3& position)
{
    const glm::vec3 color(0.15f, 1.0f, 0.15f);
    const float s = hitMarkerSize;

    DrawLine(position + glm::vec3(-s, 0.0f, 0.0f), position + glm::vec3(s, 0.0f, 0.0f), color);
    DrawLine(position + glm::vec3(0.0f, -s, 0.0f), position + glm::vec3(0.0f, s, 0.0f), color);
    DrawLine(position + glm::vec3(0.0f, 0.0f, -s), position + glm::vec3(0.0f, 0.0f, s), color);
}

glm::vec3 TrajectoryRenderer::GetPathColor(const TrajectoryPath& path) const
{
    switch (path.method)
    {
    case TrajectoryMethod::Random:
        return glm::vec3(0.90f, 0.90f, 0.88f);

    case TrajectoryMethod::LightBiased:
        return glm::vec3(0.05f, 0.82f, 1.0f);

    case TrajectoryMethod::ParetoQLearning:
        return glm::vec3(1.0f, 0.78f, 0.05f);
    }

    return glm::vec3(1.0f);
}

void TrajectoryRenderer::IncreaseVisiblePaths()
{
    maxVisiblePaths = std::min(maxVisiblePaths + 20, 1000);
    std::cout << "Max visible paths: " << maxVisiblePaths << std::endl;
}

void TrajectoryRenderer::DecreaseVisiblePaths()
{
    maxVisiblePaths = std::max(maxVisiblePaths - 20, 10);
    std::cout << "Max visible paths: " << maxVisiblePaths << std::endl;
}