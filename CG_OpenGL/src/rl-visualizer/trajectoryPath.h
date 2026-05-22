#pragma once

#include <vector>
#include <glm.hpp>

enum class TrajectoryMethod
{
    Random,
    LightBiased,
    ParetoQLearning
};

struct TrajectoryPath
{
    int id = 0;

    TrajectoryMethod method = TrajectoryMethod::Random;

    std::vector<glm::vec3> points;

    bool hitLight = false;
    bool hitObstacle = false;
    bool outOfBounds = false;

    int steps = 0;

    // x = quality / light contribution
    // y = negative cost
    glm::vec2 reward = glm::vec2(0.0f);
};