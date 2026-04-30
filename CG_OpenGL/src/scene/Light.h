#ifndef LIGHT_H
#define LIGHT_H

#include <glm.hpp>

struct Light
{
    glm::vec3 position;
    glm::vec4 color;
    bool enabled;
};

#endif