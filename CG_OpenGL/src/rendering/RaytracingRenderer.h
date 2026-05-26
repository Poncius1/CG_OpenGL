#pragma once

#include <vector>

#include <GL/glew.h>

#include "rendering/GpuTriangle.h"

class Shader;
class Camera;

class RaytracingRenderer
{
public:
    RaytracingRenderer() = default;
    ~RaytracingRenderer() = default;

    void Initialize();
    void Shutdown();

    void UploadTriangles(const std::vector<GpuTriangle>& triangles);

    void Render(
        Shader& shader,
        const Camera& camera,
        float aspectRatio,
        bool mainLightEnabled,
        bool metalShaderEnabled,
        bool glassShaderEnabled
    );

private:
    void CreateFullScreenQuad();
    void DeleteFullScreenQuad();

    void DeleteTriangleBuffer();

    void SendCameraUniforms(
        Shader& shader,
        const Camera& camera,
        float aspectRatio
    );

    void SendLightUniforms(
        Shader& shader,
        bool mainLightEnabled
    );

    void SendSceneUniforms(
        Shader& shader,
        bool metalShaderEnabled,
        bool glassShaderEnabled
    );

private:
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    GLuint triangleSSBO = 0;

    int triangleCount = 0;
    bool hasUploadedTriangles = false;
};