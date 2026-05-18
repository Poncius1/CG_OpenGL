#pragma once

#include <memory>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "camera/Camera.h"
#include "input/Input.h"
#include "rendering/Shader.h"
#include "loaders/ObjModel.h"
#include "scene/Material.h"
#include "scene/Light.h"
#include "debug/Helpers.h"

class Scene
{
public:
    void Initialize(const std::string& modelPath);
    void Shutdown();

    void Update(const Input& input);
    void Render(Shader& shader, Camera& camera);
    void RenderDebug(Shader& debugShader, Camera& camera, float aspectRatio);

private:
    void SendMaterial(Shader& shader, const Material& material);
    void SendLight(Shader& shader, const Light& light, const std::string& uniformName);

    void ApplyCameraView(Camera& camera);
    void SendRaytracingSceneUniforms(Shader& shader);

    void BuildDebugHelpers(float aspectRatio);

    glm::mat4 GetObjModelMatrix() const;

private:
    std::unique_ptr<ObjModel> model;

    Material whiteWallMaterial;
    Material redWallMaterial;
    Material greenWallMaterial;
    Material metalSphereMaterial;
    Material glassSphereMaterial;
    Material objMaterial;

    Light mainLight;
    Light fillLight;

    glm::vec3 metalSpherePosition;
    glm::vec3 glassSpherePosition;

    float metalSphereRadius = 0.35f;
    float glassSphereRadius = 0.35f;

    glm::vec3 objPosition = glm::vec3(0.0f);
    glm::vec3 objScale = glm::vec3(1.0f);

    bool metalShaderEnabled = true;
    bool glassShaderEnabled = true;

    int currentView = 0;
    bool cameraPresetChanged = true;

    bool showDebugHelpers = true;
    DebugHelpers debugHelpers;
};