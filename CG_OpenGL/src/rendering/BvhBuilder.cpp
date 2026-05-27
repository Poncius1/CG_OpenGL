#include "rendering/BvhBuilder.h"

#include <algorithm>
#include <limits>

namespace
{
    glm::vec3 MinVec3(const glm::vec3& a, const glm::vec3& b)
    {
        return glm::min(a, b);
    }

    glm::vec3 MaxVec3(const glm::vec3& a, const glm::vec3& b)
    {
        return glm::max(a, b);
    }

    float GetAxisValue(const glm::vec3& value, int axis)
    {
        if (axis == 0)
        {
            return value.x;
        }

        if (axis == 1)
        {
            return value.y;
        }

        return value.z;
    }
}

BvhBuilder::Result BvhBuilder::Build(
    const std::vector<GpuTriangle>& inputTriangles,
    int maxTrianglesPerLeaf
)
{
    Result result;

    if (inputTriangles.empty())
    {
        return result;
    }

    maxTrianglesPerLeaf = std::max(1, maxTrianglesPerLeaf);

    std::vector<BuildTriangle> buildTriangles;
    buildTriangles.reserve(inputTriangles.size());

    for (const GpuTriangle& triangle : inputTriangles)
    {
        buildTriangles.push_back(MakeBuildTriangle(triangle));
    }

    result.triangles.reserve(inputTriangles.size());
    result.nodes.reserve(inputTriangles.size() * 2);

    BuildNode(
        buildTriangles,
        0,
        static_cast<int>(buildTriangles.size()),
        result,
        maxTrianglesPerLeaf
    );

    return result;
}

int BvhBuilder::BuildNode(
    std::vector<BuildTriangle>& triangles,
    int begin,
    int end,
    Result& result,
    int maxTrianglesPerLeaf
)
{
    const int nodeIndex = static_cast<int>(result.nodes.size());
    result.nodes.push_back({});

    glm::vec3 boundsMin;
    glm::vec3 boundsMax;
    glm::vec3 centroidMin;
    glm::vec3 centroidMax;

    ComputeRangeBounds(
        triangles,
        begin,
        end,
        boundsMin,
        boundsMax,
        centroidMin,
        centroidMax
    );

    const int count = end - begin;
    const glm::vec3 centroidExtent = centroidMax - centroidMin;

    const float centroidExtentSquared =
        glm::dot(centroidExtent, centroidExtent);

    const bool makeLeaf =
        count <= maxTrianglesPerLeaf ||
        centroidExtentSquared <= 0.0000001f;

    if (makeLeaf)
    {
        const int firstTriangle =
            static_cast<int>(result.triangles.size());

        for (int i = begin; i < end; ++i)
        {
            result.triangles.push_back(triangles[i].triangle);
        }

        result.nodes[nodeIndex].boundsMin = glm::vec4(boundsMin, 0.0f);
        result.nodes[nodeIndex].boundsMax = glm::vec4(boundsMax, 0.0f);
        result.nodes[nodeIndex].data =
            glm::ivec4(-1, -1, firstTriangle, count);

        return nodeIndex;
    }

    const int axis = LongestAxis(centroidExtent);
    const int middle = begin + count / 2;

    std::nth_element(
        triangles.begin() + begin,
        triangles.begin() + middle,
        triangles.begin() + end,
        [axis](const BuildTriangle& a, const BuildTriangle& b)
        {
            return GetAxisValue(a.centroid, axis) <
                GetAxisValue(b.centroid, axis);
        }
    );

    const int leftChild = BuildNode(
        triangles,
        begin,
        middle,
        result,
        maxTrianglesPerLeaf
    );

    const int rightChild = BuildNode(
        triangles,
        middle,
        end,
        result,
        maxTrianglesPerLeaf
    );

    result.nodes[nodeIndex].boundsMin = glm::vec4(boundsMin, 0.0f);
    result.nodes[nodeIndex].boundsMax = glm::vec4(boundsMax, 0.0f);
    result.nodes[nodeIndex].data =
        glm::ivec4(leftChild, rightChild, -1, 0);

    return nodeIndex;
}

BvhBuilder::BuildTriangle BvhBuilder::MakeBuildTriangle(
    const GpuTriangle& triangle
)
{
    const glm::vec3 v0 = glm::vec3(triangle.v0);
    const glm::vec3 v1 = glm::vec3(triangle.v1);
    const glm::vec3 v2 = glm::vec3(triangle.v2);

    BuildTriangle result;

    result.triangle = triangle;
    result.boundsMin = MinVec3(v0, MinVec3(v1, v2));
    result.boundsMax = MaxVec3(v0, MaxVec3(v1, v2));
    result.centroid = (v0 + v1 + v2) / 3.0f;

    return result;
}

void BvhBuilder::ComputeRangeBounds(
    const std::vector<BuildTriangle>& triangles,
    int begin,
    int end,
    glm::vec3& boundsMin,
    glm::vec3& boundsMax,
    glm::vec3& centroidMin,
    glm::vec3& centroidMax
)
{
    const float maxFloat = std::numeric_limits<float>::max();
    const float minFloat = std::numeric_limits<float>::lowest();

    boundsMin = glm::vec3(maxFloat);
    boundsMax = glm::vec3(minFloat);
    centroidMin = glm::vec3(maxFloat);
    centroidMax = glm::vec3(minFloat);

    for (int i = begin; i < end; ++i)
    {
        boundsMin = MinVec3(boundsMin, triangles[i].boundsMin);
        boundsMax = MaxVec3(boundsMax, triangles[i].boundsMax);

        centroidMin = MinVec3(centroidMin, triangles[i].centroid);
        centroidMax = MaxVec3(centroidMax, triangles[i].centroid);
    }
}

int BvhBuilder::LongestAxis(const glm::vec3& extent)
{
    if (extent.x > extent.y && extent.x > extent.z)
    {
        return 0;
    }

    if (extent.y > extent.z)
    {
        return 1;
    }

    return 2;
}