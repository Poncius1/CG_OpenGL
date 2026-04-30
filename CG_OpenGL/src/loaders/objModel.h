#pragma once

#include <string>
#include <memory>

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "rendering/Mesh.h"
#include "camera/Camera.h"
#include "rendering/Shader.h"

class ObjModel
{
public:
    ObjModel(const std::string& path);

    void Draw(
        Shader& shader,
        Camera& camera,
        glm::mat4 parentTransform = glm::mat4(1.0f)
    );

private:
    std::unique_ptr<Mesh> mesh;
    glm::mat4 normalizationMatrix = glm::mat4(1.0f);
};