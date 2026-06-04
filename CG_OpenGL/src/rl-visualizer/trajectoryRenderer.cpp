#include "rl-visualizer/trajectoryRenderer.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

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

    PrintVisibleStatistics();
}

void TrajectoryRenderer::Draw(Shader& shader, Camera& camera)
{
    if (vao == 0 || vbo == 0)
    {
        return;
    }

    shader.Activate();
    camera.Matrix(shader, "camMatrix");

    glUniform1f(glGetUniformLocation(shader.ID, "uAlpha"), 0.78f);

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

    const int count = std::min<int>(
        static_cast<int>(paths.size()),
        maxVisiblePaths
    );

    for (int i = 0; i < count; ++i)
    {
        const TrajectoryPath& path = paths[i];

        if (!PassesFilter(path))
        {
            continue;
        }

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
    const glm::vec3 color(1.0f, 0.92f, 0.15f);
    const float s = hitMarkerSize;

    DrawLine(position + glm::vec3(-s, 0.0f, 0.0f), position + glm::vec3(s, 0.0f, 0.0f), color);
    DrawLine(position + glm::vec3(0.0f, -s, 0.0f), position + glm::vec3(0.0f, s, 0.0f), color);
    DrawLine(position + glm::vec3(0.0f, 0.0f, -s), position + glm::vec3(0.0f, 0.0f, s), color);
}

glm::vec3 TrajectoryRenderer::GetPathColor(const TrajectoryPath& path) const
{
    if (path.hitLight)
    {
        // Dorado = llegó a la luz
        return glm::vec3(1.0f, 0.82f, 0.18f);
    }

    // Rojo = falló
    return glm::vec3(0.92f, 0.18f, 0.18f);
}

bool TrajectoryRenderer::PassesFilter(const TrajectoryPath& path) const
{
    switch (filterMode)
    {
    case TrajectoryFilterMode::All:
        return true;

    case TrajectoryFilterMode::SuccessOnly:
        return path.hitLight;

    case TrajectoryFilterMode::FailedOnly:
        return !path.hitLight;
    }

    return true;
}

void TrajectoryRenderer::CycleFilterMode()
{
    switch (filterMode)
    {
    case TrajectoryFilterMode::All:
        filterMode = TrajectoryFilterMode::SuccessOnly;
        break;

    case TrajectoryFilterMode::SuccessOnly:
        filterMode = TrajectoryFilterMode::FailedOnly;
        break;

    case TrajectoryFilterMode::FailedOnly:
        filterMode = TrajectoryFilterMode::All;
        break;
    }
}

const char* TrajectoryRenderer::GetFilterModeName() const
{
    switch (filterMode)
    {
    case TrajectoryFilterMode::All:
        return "All";
    case TrajectoryFilterMode::SuccessOnly:
        return "Success only";
    case TrajectoryFilterMode::FailedOnly:
        return "Failed only";
    }

    return "Unknown";
}

void TrajectoryRenderer::PrintMethodStatistics(
    const std::string& label,
    const std::vector<TrajectoryPath>* paths,
    bool enabled
) const
{
    if (!paths || paths->empty())
    {
        std::cout << label << ": no paths loaded\n";
        return;
    }

    const int budget = std::min<int>(static_cast<int>(paths->size()), maxVisiblePaths);

    int successCount = 0;
    int failureCount = 0;

    for (int i = 0; i < budget; ++i)
    {
        if ((*paths)[i].hitLight)
        {
            ++successCount;
        }
        else
        {
            ++failureCount;
        }
    }

    const float successRate = budget > 0
        ? (100.0f * static_cast<float>(successCount) / static_cast<float>(budget))
        : 0.0f;

    const float failureRate = budget > 0
        ? (100.0f * static_cast<float>(failureCount) / static_cast<float>(budget))
        : 0.0f;

    int displayedCount = 0;
    for (int i = 0; i < budget; ++i)
    {
        if (PassesFilter((*paths)[i]))
        {
            ++displayedCount;
        }
    }

    std::cout
        << label
        << (enabled ? " [ON] " : " [OFF] ")
        << "budget=" << budget
        << " | success=" << successCount << " (" << std::fixed << std::setprecision(1) << successRate << "%)"
        << " | fail=" << failureCount << " (" << std::fixed << std::setprecision(1) << failureRate << "%)"
        << " | displayed_with_filter=" << displayedCount
        << '\n';
}

void TrajectoryRenderer::PrintVisibleStatistics() const
{
    std::cout << "\n=== Visible trajectory statistics ===\n";
    std::cout << "Budget per method: " << maxVisiblePaths << '\n';
    std::cout << "Filter mode: " << GetFilterModeName() << '\n';
    std::cout << "Colors: gold = reached light | red = failed\n\n";

    PrintMethodStatistics("Random", randomPaths, showRandom);
    PrintMethodStatistics("Light-biased", lightBiasedPaths, showLightBiased);
    PrintMethodStatistics("Pareto Q-Learning", pqlPaths, showPql);

    std::cout << "=====================================\n\n";
}

void TrajectoryRenderer::IncreaseVisiblePaths()
{
    maxVisiblePaths = std::min(maxVisiblePaths + 100, 5000);
    std::cout << "Max visible paths per method: " << maxVisiblePaths << std::endl;
    PrintVisibleStatistics();
}

void TrajectoryRenderer::DecreaseVisiblePaths()
{
    maxVisiblePaths = std::max(maxVisiblePaths - 100, 100);
    std::cout << "Max visible paths per method: " << maxVisiblePaths << std::endl;
    PrintVisibleStatistics();
}