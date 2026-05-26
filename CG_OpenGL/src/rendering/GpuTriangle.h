#pragma once

#include <glm.hpp>

struct GpuTriangle
{
    glm::vec4 v0;
    glm::vec4 v1;
    glm::vec4 v2;

    glm::vec4 n0;
    glm::vec4 n1;
    glm::vec4 n2;

    glm::vec4 albedo;

    // x = materialType
    // y = roughness
    // z = ior
    // w = emissionStrength
    glm::vec4 materialData;
};