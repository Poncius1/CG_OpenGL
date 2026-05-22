#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm.hpp>

#include "camera/Camera.h"
#include "rendering/Shader.h"
#include "rl-visualizer/trajectoryPath.h"

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

    void IncreaseVisiblePaths();
    void DecreaseVisiblePaths();

    bool IsShowingRandom() const { return showRandom; }
    bool IsShowingLightBiased() const { return showLightBiased; }
    bool IsShowingPql() const { return showPql; }
    bool IsFirstSegmentMode() const { return drawOnlyFirstSegment; }

private:
    void DrawPathCollection(const std::vector<TrajectoryPath>& paths);
    void DrawPath(const TrajectoryPath& path);
    void DrawFirstSegment(const TrajectoryPath& path);

    void DrawLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);
    void DrawHitMarker(const glm::vec3& position);

    glm::vec3 GetPathColor(const TrajectoryPath& path) const;

private:
    GLuint vao = 0;
    GLuint vbo = 0;

    const std::vector<TrajectoryPath>* randomPaths = nullptr;
    const std::vector<TrajectoryPath>* lightBiasedPaths = nullptr;
    const std::vector<TrajectoryPath>* pqlPaths = nullptr;

    bool showRandom = true;
    bool showLightBiased = true;
    bool showPql = false;

    bool drawOnlyFirstSegment = false;

    int maxVisiblePaths = 120;

    float firstSegmentLength = 1.7f;
    float hitMarkerSize = 0.035f;
};