#include "Scene.h"

void Scene::Initialize(const std::string& modelPath)
{
    model = std::make_unique<ObjModel>(modelPath);

    materialA =
    {
        glm::vec4(0.12f, 0.12f, 0.12f, 1.0f),
        glm::vec4(0.95f, 0.92f, 0.86f, 1.0f),
        glm::vec4(1.0f, 0.95f, 0.85f, 1.0f),
        32.0f
    };

    materialB =
    {
        glm::vec4(0.02f, 0.03f, 0.08f, 1.0f),
        glm::vec4(0.02f, 0.08f, 0.35f, 1.0f),
        glm::vec4(0.45f, 0.75f, 1.0f, 1.0f),
        96.0f
    };

    whiteLight =
    {
        glm::vec3(0.0f, 2.5f, 2.8f), 
        glm::vec4(1.8f, 1.7f, 1.6f, 1.0f),
        true
    };

    blueLight =
    {
        glm::vec3(-1.5f, 0.8f, 1.2f),
        glm::vec4(0.1f, 0.35f, 1.0f, 1.0f),
        true
    };
}

void Scene::Update(const Input& input)
{
    if (input.IsKeyPressed(GLFW_KEY_M))
        currentMaterial = currentMaterial == 0 ? 1 : 0;

    if (input.IsKeyPressed(GLFW_KEY_L))
        blueLight.enabled = !blueLight.enabled;

    if (input.IsKeyPressed(GLFW_KEY_1))
        currentView = 0;

    if (input.IsKeyPressed(GLFW_KEY_2))
        currentView = 1;

    if (input.IsKeyPressed(GLFW_KEY_3))
        currentView = 2;
}

void Scene::Render(Shader& shader, Camera& camera)
{
    shader.Activate();

    camera.Matrix(shader, "camMatrix");

    glUniform3f(
        glGetUniformLocation(shader.ID, "camPos"),
        camera.Position.x,
        camera.Position.y,
        camera.Position.z
    );

    if (currentMaterial == 0)
        SendMaterial(shader, materialA);
    else
        SendMaterial(shader, materialB);

    SendLight(shader, whiteLight, "whiteLight");
    SendLight(shader, blueLight, "blueLight");

    glm::mat4 modelMatrix = GetModelMatrix();

    if (model)
        model->Draw(shader, camera, modelMatrix);
}

void Scene::SendMaterial(Shader& shader, const Material& material)
{
    glUniform4f(glGetUniformLocation(shader.ID, "material.ambient"),
        material.ambient.x, material.ambient.y, material.ambient.z, material.ambient.w);

    glUniform4f(glGetUniformLocation(shader.ID, "material.diffuse"),
        material.diffuse.x, material.diffuse.y, material.diffuse.z, material.diffuse.w);

    glUniform4f(glGetUniformLocation(shader.ID, "material.specular"),
        material.specular.x, material.specular.y, material.specular.z, material.specular.w);

    glUniform1f(glGetUniformLocation(shader.ID, "material.shininess"),
        material.shininess);
}

void Scene::SendLight(Shader& shader, const Light& light, const std::string& uniformName)
{
    glUniform3f(glGetUniformLocation(shader.ID, (uniformName + ".position").c_str()),
        light.position.x, light.position.y, light.position.z);

    glUniform4f(glGetUniformLocation(shader.ID, (uniformName + ".color").c_str()),
        light.color.x, light.color.y, light.color.z, light.color.w);

    glUniform1i(glGetUniformLocation(shader.ID, (uniformName + ".enabled").c_str()),
        light.enabled);
}

glm::mat4 Scene::GetModelMatrix() const
{
    glm::mat4 matrix = glm::mat4(1.0f);

    matrix = glm::scale(matrix, glm::vec3(1.0f));

    if (currentView == 0)
    {
        matrix = glm::rotate(matrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (currentView == 1)
    {
        matrix = glm::rotate(matrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (currentView == 2)
    {
        matrix = glm::rotate(matrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    return matrix;
}