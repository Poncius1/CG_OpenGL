#pragma once

#include <vector>

#include "rendering/GpuBvhNode.h"
#include "rendering/GpuTriangle.h"

class BvhBuilder
{
public:
    struct Result
    {
        std::vector<GpuTriangle> triangles;
        std::vector<GpuBvhNode> nodes;
    };

public:
    static Result Build(
        const std::vector<GpuTriangle>& inputTriangles,
        int maxTrianglesPerLeaf = 4
    );

private:
    struct BuildTriangle
    {
        GpuTriangle triangle;

        glm::vec3 boundsMin;
        glm::vec3 boundsMax;
        glm::vec3 centroid;
    };

private:
    static int BuildNode(
        std::vector<BuildTriangle>& triangles,
        int begin,
        int end,
        Result& result,
        int maxTrianglesPerLeaf
    );

    static BuildTriangle MakeBuildTriangle(const GpuTriangle& triangle);

    static void ComputeRangeBounds(
        const std::vector<BuildTriangle>& triangles,
        int begin,
        int end,
        glm::vec3& boundsMin,
        glm::vec3& boundsMax,
        glm::vec3& centroidMin,
        glm::vec3& centroidMax
    );

    static int LongestAxis(const glm::vec3& extent);
};