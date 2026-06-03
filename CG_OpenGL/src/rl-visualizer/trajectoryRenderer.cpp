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
    activeShaderId = shader.ID;

    const GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
    const GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vao);

    if (showRandom && randomPaths)
        DrawPathCollection(*randomPaths);

    if (showLightBiased && lightBiasedPaths)
        DrawPathCollection(*lightBiasedPaths);

    if (showPql && pqlPaths)
        DrawPathCollection(*pqlPaths);

    glBindVertexArray(0);
    activeShaderId = 0;

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

    // Show successful paths first. This makes the contribution story visible
    // even when maxVisiblePaths is small.
    for (const TrajectoryPath& path : paths)
    {
        if (path.hitLight && ShouldDrawPath(path))
            selected.push_back(&path);
    }

    for (const TrajectoryPath& path : paths)
    {
        if (!path.hitLight && ShouldDrawPath(path))
            selected.push_back(&path);
    }

    const int count = std::min<int>(
        static_cast<int>(selected.size()),
        maxVisiblePaths
    );

    for (int i = 0; i < count; ++i)
    {
        const TrajectoryPath& path = *selected[i];

        if (drawOnlyFirstSegment)
            DrawFirstSegment(path);
        else
            DrawPath(path);

        DrawPathMarkers(path);
    }
}

void TrajectoryRenderer::DrawPath(const TrajectoryPath& path)
{
    if (!path.segments.empty())
    {
        for (size_t i = 0; i < path.segments.size(); ++i)
        {
            DrawSegment(path, path.segments[i], static_cast<int>(i));
        }
        return;
    }

    if (path.points.size() < 2)
    {
        return;
    }

    const glm::vec3 color = GetPathColor(path);
    const float alpha = GetPathAlpha(path);

    for (size_t i = 0; i + 1 < path.points.size(); ++i)
    {
        DrawLine(path.points[i], path.points[i + 1], color, alpha, 1.8f);
    }
}

void TrajectoryRenderer::DrawFirstSegment(const TrajectoryPath& path)
{
    glm::vec3 origin(0.0f);
    glm::vec3 end(0.0f);

    if (!path.segments.empty())
    {
        origin = path.segments.front().from;
        end = path.segments.front().to;
    }
    else if (path.points.size() >= 2)
    {
        origin = path.points[0];
        end = path.points[1];
    }
    else
    {
        return;
    }

    glm::vec3 direction = end - origin;
    const float length = glm::length(direction);

    if (length < 1e-5f)
    {
        return;
    }

    direction = glm::normalize(direction);

    DrawLine(
        origin,
        origin + direction * firstSegmentLength,
        GetPathColor(path),
        GetPathAlpha(path),
        path.hitLight ? 3.0f : 1.6f
    );
}

void TrajectoryRenderer::DrawSegment(
    const TrajectoryPath& path,
    const TrajectorySegment& segment,
    int segmentIndex
)
{
    const glm::vec3 color = GetSegmentColor(path, segment, segmentIndex);
    const float alpha = GetSegmentAlpha(path, segmentIndex);

    const float lineWidth = path.hitLight ? 2.4f : 1.4f;
    DrawLine(segment.from, segment.to, color, alpha, lineWidth);
}

void TrajectoryRenderer::DrawLine(
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& color,
    float alpha,
    float lineWidth
)
{
    if (activeShaderId != 0)
    {
        glUniform1f(glGetUniformLocation(activeShaderId, "uAlpha"), alpha);
    }

    glLineWidth(lineWidth);

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_LINES, 0, 2);
}

void TrajectoryRenderer::DrawMarker(const glm::vec3& position, const glm::vec3& color, float size)
{
    DrawLine(position + glm::vec3(-size, 0.0f, 0.0f), position + glm::vec3(size, 0.0f, 0.0f), color, 0.95f, 2.8f);
    DrawLine(position + glm::vec3(0.0f, -size, 0.0f), position + glm::vec3(0.0f, size, 0.0f), color, 0.95f, 2.8f);
    DrawLine(position + glm::vec3(0.0f, 0.0f, -size), position + glm::vec3(0.0f, 0.0f, size), color, 0.95f, 2.8f);
}

void TrajectoryRenderer::DrawPathMarkers(const TrajectoryPath& path)
{
    if (path.points.empty())
    {
        return;
    }

    // Blue marker = first surface point / beginning of agent decisions.
    DrawMarker(path.points.front(), glm::vec3(0.25f, 0.45f, 1.0f), hitMarkerSize * 0.55f);

    const glm::vec3& end = path.points.back();

    if (path.hitLight)
    {
        DrawMarker(end, glm::vec3(0.2f, 1.0f, 0.25f), hitMarkerSize);
    }
    else if (path.hitObstacle)
    {
        DrawMarker(end, glm::vec3(1.0f, 0.15f, 0.12f), hitMarkerSize * 0.85f);
    }
}

bool TrajectoryRenderer::ShouldDrawPath(const TrajectoryPath& path) const
{
    switch (filterMode)
    {
    case TrajectoryFilterMode::All:
        return true;
    case TrajectoryFilterMode::SuccessOnly:
        return path.hitLight;
    case TrajectoryFilterMode::FailureOnly:
        return !path.hitLight;
    }

    return true;
}

glm::vec3 TrajectoryRenderer::GetPathColor(const TrajectoryPath& path) const
{
    if (colorMode == TrajectoryColorMode::Outcome)
    {
        if (path.hitLight)
            return glm::vec3(0.2f, 1.0f, 0.25f);
        if (path.hitObstacle)
            return glm::vec3(1.0f, 0.15f, 0.12f);
        if (path.outOfBounds)
            return glm::vec3(0.55f, 0.55f, 0.58f);
        return glm::vec3(0.95f, 0.35f, 0.85f);
    }

    if (colorMode == TrajectoryColorMode::Cost)
    {
        const float t = Clamp01(path.totalCost / 10.0f);
        return LerpColor(glm::vec3(0.25f, 1.0f, 0.25f), glm::vec3(1.0f, 0.16f, 0.08f), t);
    }

    if (colorMode == TrajectoryColorMode::Quality)
    {
        const float t = Clamp01(path.totalQuality / 1.2f);
        return LerpColor(glm::vec3(0.2f, 0.25f, 0.8f), glm::vec3(1.0f, 0.85f, 0.12f), t);
    }

    switch (path.method)
    {
    case TrajectoryMethod::Random:
        return glm::vec3(0.88f, 0.88f, 0.86f);
    case TrajectoryMethod::LightBiased:
        return glm::vec3(0.05f, 0.82f, 1.0f);
    case TrajectoryMethod::ParetoQLearning:
        return glm::vec3(1.0f, 0.78f, 0.05f);
    }

    return glm::vec3(1.0f);
}

glm::vec3 TrajectoryRenderer::GetSegmentColor(
    const TrajectoryPath& path,
    const TrajectorySegment& segment,
    int segmentIndex
) const
{
    if (colorMode == TrajectoryColorMode::Bounce)
    {
        const float t = Clamp01(static_cast<float>(segmentIndex) / 5.0f);
        return LerpColor(glm::vec3(0.05f, 0.85f, 1.0f), glm::vec3(1.0f, 0.25f, 0.10f), t);
    }

    if (colorMode == TrajectoryColorMode::Outcome && segment.event == "light")
    {
        return glm::vec3(0.95f, 1.0f, 0.2f);
    }

    return GetPathColor(path);
}

float TrajectoryRenderer::GetPathAlpha(const TrajectoryPath& path) const
{
    if (path.hitLight)
        return 0.92f;

    if (path.hitObstacle)
        return 0.52f;

    return 0.30f;
}

float TrajectoryRenderer::GetSegmentAlpha(const TrajectoryPath& path, int segmentIndex) const
{
    const float baseAlpha = GetPathAlpha(path);

    // Later bounces fade slightly, so path depth is readable.
    const float fade = 1.0f - Clamp01(static_cast<float>(segmentIndex) * 0.10f);
    return std::max(0.18f, baseAlpha * fade);
}

void TrajectoryRenderer::NextColorMode()
{
    switch (colorMode)
    {
    case TrajectoryColorMode::Method:
        colorMode = TrajectoryColorMode::Outcome;
        break;
    case TrajectoryColorMode::Outcome:
        colorMode = TrajectoryColorMode::Cost;
        break;
    case TrajectoryColorMode::Cost:
        colorMode = TrajectoryColorMode::Quality;
        break;
    case TrajectoryColorMode::Quality:
        colorMode = TrajectoryColorMode::Bounce;
        break;
    case TrajectoryColorMode::Bounce:
        colorMode = TrajectoryColorMode::Method;
        break;
    }

    std::cout << "Trajectory color mode: " << GetColorModeName() << std::endl;
}

void TrajectoryRenderer::NextFilterMode()
{
    switch (filterMode)
    {
    case TrajectoryFilterMode::All:
        filterMode = TrajectoryFilterMode::SuccessOnly;
        break;
    case TrajectoryFilterMode::SuccessOnly:
        filterMode = TrajectoryFilterMode::FailureOnly;
        break;
    case TrajectoryFilterMode::FailureOnly:
        filterMode = TrajectoryFilterMode::All;
        break;
    }

    std::cout << "Trajectory filter mode: " << GetFilterModeName() << std::endl;
}

std::string TrajectoryRenderer::GetColorModeName() const
{
    switch (colorMode)
    {
    case TrajectoryColorMode::Method:
        return "Method";
    case TrajectoryColorMode::Outcome:
        return "Outcome";
    case TrajectoryColorMode::Cost:
        return "Cost";
    case TrajectoryColorMode::Quality:
        return "Quality";
    case TrajectoryColorMode::Bounce:
        return "Bounce";
    }

    return "Unknown";
}

std::string TrajectoryRenderer::GetFilterModeName() const
{
    switch (filterMode)
    {
    case TrajectoryFilterMode::All:
        return "All";
    case TrajectoryFilterMode::SuccessOnly:
        return "Success only";
    case TrajectoryFilterMode::FailureOnly:
        return "Failures only";
    }

    return "Unknown";
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

glm::vec3 TrajectoryRenderer::LerpColor(const glm::vec3& a, const glm::vec3& b, float t)
{
    return a + (b - a) * Clamp01(t);
}

float TrajectoryRenderer::Clamp01(float value)
{
    return std::max(0.0f, std::min(1.0f, value));
}
