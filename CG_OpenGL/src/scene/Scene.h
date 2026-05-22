#pragma once

#include <memory>
#include <string>

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "camera/Camera.h"
#include "input/Input.h"
#include "rendering/Shader.h"
#include "loaders/ObjModel.h"
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
    void SendLight(Shader& shader, const Light& light, const std::string& uniformName);
    void ApplyCameraView(Camera& camera);
    void BuildDebugHelpers(float aspectRatio);

    glm::mat4 GetObjModelMatrix() const;

private:
    std::unique_ptr<ObjModel> model;

    Light mainLight;
    Light fillLight;

    glm::vec3 objPosition = glm::vec3(0.0f);
    glm::vec3 objScale = glm::vec3(1.0f);

    int currentView = 0;
    bool cameraPresetChanged = true;

    bool showDebugHelpers = false;
    DebugHelpers debugHelpers;
};