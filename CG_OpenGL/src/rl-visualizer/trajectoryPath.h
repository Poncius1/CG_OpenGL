#pragma once

#include <string>
#include <vector>

#include <glm.hpp>

enum class TrajectoryMethod
{
    Random,
    LightBiased,
    ParetoQLearning
};

enum class TrajectoryTerminalEvent
{
    Unknown,
    HitLight,
    HitObstacle,
    OutOfBounds,
    MaxSteps,
    Failed
};

struct TrajectorySegment
{
    glm::vec3 from = glm::vec3(0.0f);
    glm::vec3 to = glm::vec3(0.0f);

    std::string event = "unknown";

    int bounce = 0;
    int action = -1;

    float distance = 0.0f;

    // x = quality contribution, y = negative cost.
    glm::vec2 reward = glm::vec2(0.0f);
};

struct TrajectoryPath
{
    int id = 0;

    TrajectoryMethod method = TrajectoryMethod::Random;
    std::string methodName = "unknown";

    std::vector<glm::vec3> points;
    std::vector<TrajectorySegment> segments;

    bool hitLight = false;
    bool hitObstacle = false;
    bool outOfBounds = false;

    int steps = 0;

    // Legacy reward support: x = quality, y = negative cost.
    glm::vec2 reward = glm::vec2(0.0f);

    // New JSON fields exported by the MORL project.
    float totalQuality = 0.0f;
    float totalCost = 0.0f;

    std::string visualClass = "unknown";
    TrajectoryTerminalEvent terminalEvent = TrajectoryTerminalEvent::Unknown;
    std::string terminalEventName = "unknown";
};
