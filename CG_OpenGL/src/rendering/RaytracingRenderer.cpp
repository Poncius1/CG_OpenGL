#include "rendering/RaytracingRenderer.h"

#include <iostream>

#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include "camera/Camera.h"
#include "rendering/BvhBuilder.h"
#include "rendering/Shader.h"
#include "scene/SceneConfig.h"

void RaytracingRenderer::Initialize()
{
    CreateFullScreenQuad();
}

void RaytracingRenderer::Shutdown()
{
    DeleteSceneBuffers();
    DeleteFullScreenQuad();
}

void RaytracingRenderer::UploadTriangles(
    const std::vector<GpuTriangle>& triangles
)
{
    DeleteSceneBuffers();

    if (triangles.empty())
    {
        std::cout << "No OBJ triangles uploaded to raytracer.\n";
        return;
    }

    BvhBuilder::Result bvh = BvhBuilder::Build(triangles, 4);

    triangleCount = static_cast<int>(bvh.triangles.size());
    bvhNodeCount = static_cast<int>(bvh.nodes.size());
    hasUploadedScene = triangleCount > 0 && bvhNodeCount > 0;

    if (!hasUploadedScene)
    {
        std::cout << "BVH build failed.\n";
        return;
    }

    UploadTriangleBuffer(bvh.triangles);
    UploadBvhBuffer(bvh.nodes);

    std::cout << "Uploaded "
        << triangleCount
        << " triangles and "
        << bvhNodeCount
        << " BVH nodes to GPU.\n";
}

void RaytracingRenderer::Render(
    Shader& shader,
    const Camera& camera,
    float aspectRatio,
    bool mainLightEnabled,
    bool metalShaderEnabled,
    bool glassShaderEnabled
)
{
    shader.Activate();

    if (triangleSSBO != 0)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, triangleSSBO);
    }

    if (bvhSSBO != 0)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bvhSSBO);
    }

    SendCameraUniforms(shader, camera, aspectRatio);
    SendLightUniforms(shader, mainLightEnabled);
    SendSceneUniforms(shader, metalShaderEnabled, glassShaderEnabled);

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void RaytracingRenderer::CreateFullScreenQuad()
{
    if (quadVAO != 0)
    {
        return;
    }

    const float vertices[] =
    {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        reinterpret_cast<void*>(0)
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        reinterpret_cast<void*>(2 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void RaytracingRenderer::DeleteFullScreenQuad()
{
    if (quadVBO != 0)
    {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }

    if (quadVAO != 0)
    {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
}

void RaytracingRenderer::DeleteSceneBuffers()
{
    if (triangleSSBO != 0)
    {
        glDeleteBuffers(1, &triangleSSBO);
        triangleSSBO = 0;
    }

    if (bvhSSBO != 0)
    {
        glDeleteBuffers(1, &bvhSSBO);
        bvhSSBO = 0;
    }

    triangleCount = 0;
    bvhNodeCount = 0;
    hasUploadedScene = false;
}

void RaytracingRenderer::UploadTriangleBuffer(
    const std::vector<GpuTriangle>& triangles
)
{
    glGenBuffers(1, &triangleSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);

    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        triangles.size() * sizeof(GpuTriangle),
        triangles.data(),
        GL_STATIC_DRAW
    );

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, triangleSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void RaytracingRenderer::UploadBvhBuffer(
    const std::vector<GpuBvhNode>& nodes
)
{
    glGenBuffers(1, &bvhSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhSSBO);

    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        nodes.size() * sizeof(GpuBvhNode),
        nodes.data(),
        GL_STATIC_DRAW
    );

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bvhSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void RaytracingRenderer::SendCameraUniforms(
    Shader& shader,
    const Camera& camera,
    float aspectRatio
)
{
    const glm::vec3 forward = glm::normalize(camera.Orientation);
    const glm::vec3 right = glm::normalize(glm::cross(forward, camera.Up));
    const glm::vec3 up = glm::normalize(glm::cross(right, forward));

    glUniform3fv(
        glGetUniformLocation(shader.ID, "cameraPosition"),
        1,
        glm::value_ptr(camera.Position)
    );

    glUniform3fv(
        glGetUniformLocation(shader.ID, "cameraForward"),
        1,
        glm::value_ptr(forward)
    );

    glUniform3fv(
        glGetUniformLocation(shader.ID, "cameraRight"),
        1,
        glm::value_ptr(right)
    );

    glUniform3fv(
        glGetUniformLocation(shader.ID, "cameraUp"),
        1,
        glm::value_ptr(up)
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, "cameraFov"),
        SceneConfig::CAMERA_FOV
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, "aspectRatio"),
        aspectRatio
    );
}

void RaytracingRenderer::SendLightUniforms(
    Shader& shader,
    bool mainLightEnabled
)
{
    const SceneConfig::SceneLight& mainLight = SceneConfig::MAIN_LIGHT;
    const SceneConfig::SceneLight& fillLight = SceneConfig::FILL_LIGHT;

    glUniform3fv(
        glGetUniformLocation(shader.ID, "mainLight.position"),
        1,
        glm::value_ptr(mainLight.position)
    );

    glUniform4fv(
        glGetUniformLocation(shader.ID, "mainLight.color"),
        1,
        glm::value_ptr(mainLight.color)
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, "mainLight.intensity"),
        mainLight.intensity
    );

    glUniform1i(
        glGetUniformLocation(shader.ID, "mainLight.enabled"),
        mainLight.enabled && mainLightEnabled
    );

    glUniform3fv(
        glGetUniformLocation(shader.ID, "fillLight.position"),
        1,
        glm::value_ptr(fillLight.position)
    );

    glUniform4fv(
        glGetUniformLocation(shader.ID, "fillLight.color"),
        1,
        glm::value_ptr(fillLight.color)
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, "fillLight.intensity"),
        fillLight.intensity
    );

    glUniform1i(
        glGetUniformLocation(shader.ID, "fillLight.enabled"),
        fillLight.enabled
    );
}

void RaytracingRenderer::SendSceneUniforms(
    Shader& shader,
    bool metalShaderEnabled,
    bool glassShaderEnabled
)
{
    glUniform1i(
        glGetUniformLocation(shader.ID, "metalShaderEnabled"),
        metalShaderEnabled
    );

    glUniform1i(
        glGetUniformLocation(shader.ID, "glassShaderEnabled"),
        glassShaderEnabled
    );

    glUniform1i(
        glGetUniformLocation(shader.ID, "objTriangleCount"),
        triangleCount
    );

    glUniform1i(
        glGetUniformLocation(shader.ID, "bvhNodeCount"),
        bvhNodeCount
    );

    glUniform1i(
        glGetUniformLocation(shader.ID, "useObjTriangles"),
        hasUploadedScene
    );
}