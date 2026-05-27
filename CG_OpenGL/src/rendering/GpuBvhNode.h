#pragma once

#include <glm.hpp>

// Nodo compacto para recorrer el BVH en GPU.
// Si data.w > 0: hoja. data.z = primer triangulo, data.w = cantidad.
// Si data.w == 0: nodo interno. data.x = hijo izquierdo, data.y = hijo derecho.
struct GpuBvhNode
{
    glm::vec4 boundsMin;
    glm::vec4 boundsMax;
    glm::ivec4 data;
};