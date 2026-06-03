#pragma once

#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm.hpp>

#include "camera/Camera.h"
#include "rendering/Shader.h"
#include "rl-visualizer/trajectoryPath.h"

enum class TrajectoryColorMode
{
    Method,
    Outcome,
    Cost,
    Quality,
    Bounce
};

enum class TrajectoryFilterMode
{
    All,
    SuccessOnly,
    FailureOnly
};

class TrajectoryRenderer
{
public:
    TrajectoryRenderer() = default;
    ~TrajectoryRenderer();

    void Initialize();
    void Shutdown();

    void SetPaths(
        const std::vector<TrajectoryPath>* randomPaths,
        const std::vector<TrajectoryPath>* lightBiasedPaths,
        const std::vector<TrajectoryPath>* pqlPaths
    );

    void Draw(Shader& shader, Camera& camera);

    void ToggleRandom() { showRandom = !showRandom; }
    void ToggleLightBiased() { showLightBiased = !showLightBiased; }
    void TogglePql() { showPql = !showPql; }
    void ToggleFirstSegmentMode() { drawOnlyFirstSegment = !drawOnlyFirstSegment; }

    void NextColorMode();
    void NextFilterMode();

    void IncreaseVisiblePaths();
    void DecreaseVisiblePaths();

    bool IsShowingRandom() const { return showRandom; }
    bool IsShowingLightBiased() const { return showLightBiased; }
    bool IsShowingPql() const { return showPql; }
    bool IsFirstSegmentMode() const { return drawOnlyFirstSegment; }

    std::string GetColorModeName() const;
    std::string GetFilterModeName() const;

private:
    void DrawPathCollection(const std::vector<TrajectoryPath>& paths);
    void DrawPath(const TrajectoryPath& path);
    void DrawFirstSegment(const TrajectoryPath& path);
    void DrawSegment(const TrajectoryPath& path, const TrajectorySegment& segment, int segmentIndex);

    void DrawLine(
        const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& color,
        float alpha,
        float lineWidth
    );

    void DrawMarker(const glm::vec3& position, const glm::vec3& color, float size);
    void DrawPathMarkers(const TrajectoryPath& path);

    bool ShouldDrawPath(const TrajectoryPath& path) const;

    glm::vec3 GetPathColor(const TrajectoryPath& path) const;
    glm::vec3 GetSegmentColor(const TrajectoryPath& path, const TrajectorySegment& segment, int segmentIndex) const;

    float GetPathAlpha(const TrajectoryPath& path) const;
    float GetSegmentAlpha(const TrajectoryPath& path, int segmentIndex) const;

    static glm::vec3 LerpColor(const glm::vec3& a, const glm::vec3& b, float t);
    static float Clamp01(float value);

private:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint activeShaderId = 0;

    const std::vector<TrajectoryPath>* randomPaths = nullptr;
    const std::vector<TrajectoryPath>* lightBiasedPaths = nullptr;
    const std::vector<TrajectoryPath>* pqlPaths = nullptr;

    bool showRandom = true;
    bool showLightBiased = true;
    bool showPql = false;

    bool drawOnlyFirstSegment = false;

    TrajectoryColorMode colorMode = TrajectoryColorMode::Outcome;
    TrajectoryFilterMode filterMode = TrajectoryFilterMode::All;

    int maxVisiblePaths = 120;

    float firstSegmentLength = 1.7f;
    float hitMarkerSize = 0.045f;
};
