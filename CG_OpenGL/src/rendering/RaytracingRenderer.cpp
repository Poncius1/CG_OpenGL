#include "rendering/RaytracingRenderer.h"

#include <iostream>

#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include "camera/Camera.h"
#include "rendering/Shader.h"
#include "scene/SceneConfig.h"

void RaytracingRenderer::Initialize()
{
    CreateFullScreenQuad();
}

void RaytracingRenderer::Shutdown()
{
    DeleteTriangleBuffer();
    DeleteFullScreenQuad();
}

void RaytracingRenderer::UploadTriangles(
    const std::vector<GpuTriangle>& triangles
)
{
    DeleteTriangleBuffer();

    triangleCount = static_cast<int>(triangles.size());
    hasUploadedTriangles = triangleCount > 0;

    if (!hasUploadedTriangles)
    {
        std::cout << "No OBJ triangles uploaded to raytracer.\n";
        return;
    }

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

    std::cout << "Uploaded "
        << triangleCount
        << " triangles to SSBO.\n";
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
        return;

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

void RaytracingRenderer::DeleteTriangleBuffer()
{
    if (triangleSSBO != 0)
    {
        glDeleteBuffers(1, &triangleSSBO);
        triangleSSBO = 0;
    }

    triangleCount = 0;
    hasUploadedTriangles = false;
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
    glUniform3fv(
        glGetUniformLocation(shader.ID, "boxMin"),
        1,
        glm::value_ptr(SceneConfig::BOX_MIN)
    );

    glUniform3fv(
        glGetUniformLocation(shader.ID, "boxMax"),
        1,
        glm::value_ptr(SceneConfig::BOX_MAX)
    );

    glUniform3fv(
        glGetUniformLocation(shader.ID, "metalBoxMin"),
        1,
        glm::value_ptr(SceneConfig::METAL_BOX_MIN)
    );

    glUniform3fv(
        glGetUniformLocation(shader.ID, "metalBoxMax"),
        1,
        glm::value_ptr(SceneConfig::METAL_BOX_MAX)
    );

    glUniform3fv(
        glGetUniformLocation(shader.ID, "metalBoxAlbedo"),
        1,
        glm::value_ptr(SceneConfig::METAL_BOX_ALBEDO)
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, "metalBoxRoughness"),
        SceneConfig::METAL_BOX_ROUGHNESS
    );

    glUniform3fv(
        glGetUniformLocation(shader.ID, "glassSpherePosition"),
        1,
        glm::value_ptr(SceneConfig::GLASS_SPHERE_POSITION)
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, "glassSphereRadius"),
        SceneConfig::GLASS_SPHERE_RADIUS
    );

    glUniform3fv(
        glGetUniformLocation(shader.ID, "glassSphereAlbedo"),
        1,
        glm::value_ptr(SceneConfig::GLASS_SPHERE_ALBEDO)
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, "glassSphereIOR"),
        SceneConfig::GLASS_SPHERE_IOR
    );

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
        glGetUniformLocation(shader.ID, "useObjTriangles"),
        hasUploadedTriangles
    );

    // Si el OBJ está cargado, el OBJ reemplaza la escena analítica.
    glUniform1i(
        glGetUniformLocation(shader.ID, "useAnalyticScene"),
        !hasUploadedTriangles
    );
}