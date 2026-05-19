#pragma once

#include <memory>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "camera/Camera.h"
#include "debug/Helpers.h"
#include "input/Input.h"
#include "loaders/ObjModel.h"
#include "rendering/Shader.h"
#include "scene/Light.h"
#include "scene/Material.h"

enum class LightingModel
{
    Phong = 0,
    Blinn = 1
};

struct SceneRenderSettings
{
    int currentMaterial = 0;

    bool textureMappingEnabled = true;
    bool showDebugHelpers = true;

    LightingModel lightingModel = LightingModel::Phong;
};

class Scene
{
public:
    void Initialize(const std::string& modelPath);
    void Shutdown();

    void Update(const Input& input);

    void Render(Shader& shader, Camera& camera);
    void RenderDebug(Shader& debugShader, Camera& camera, float aspectRatio);

private:
    void ApplyCameraView(Camera& camera);

    void BuildDebugHelpers(float aspectRatio);

    void SendMaterial(Shader& shader, const Material& material);
    void SendLight(Shader& shader, const Light& light, const std::string& uniformName);
    void SendRenderSettings(Shader& shader);

    glm::mat4 GetObjModelMatrix() const;

private:
    std::unique_ptr<ObjModel> model;

    DebugHelpers debugHelpers;

    Material materialA;
    Material materialB;

    Light mainLight;
    Light fillLight;

    glm::vec3 objPosition = glm::vec3(0.0f);
    glm::vec3 objScale = glm::vec3(1.0f);

    int currentView = 0;
    bool cameraPresetChanged = true;

    SceneRenderSettings renderSettings;
};