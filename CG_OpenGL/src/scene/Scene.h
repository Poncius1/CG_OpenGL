#pragma once

#include <memory>
#include <string>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "camera/Camera.h"
#include "input/Input.h"
#include "scene/Light.h"
#include "scene/Material.h"
#include "loaders/ObjModel.h"
#include "rendering/Shader.h"

class Scene
{
public:
    void Initialize(const std::string& modelPath);
    void Update(const Input& input);
    void Render(Shader& shader, Camera& camera);

private:
    std::unique_ptr<ObjModel> model;

    Material materialA;
    Material materialB;

    Light whiteLight;
    Light blueLight;

    int currentMaterial = 0;
    int currentView = 0;

private:
    void SendMaterial(Shader& shader, const Material& material);
    void SendLight(Shader& shader, const Light& light, const std::string& uniformName);
    glm::mat4 GetModelMatrix() const;
};