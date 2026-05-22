#include "Scene.h"
#include "scene/SceneConfig.h"

void Scene::Initialize(const std::string& modelPath)
{
    model = std::make_unique<ObjModel>(modelPath);

    debugHelpers.Initialize();

    mainLight =
    {
        SceneConfig::MAIN_LIGHT.position,
        SceneConfig::MAIN_LIGHT.color,
        SceneConfig::MAIN_LIGHT.intensity,
        SceneConfig::MAIN_LIGHT.enabled
    };

    fillLight =
    {
        SceneConfig::FILL_LIGHT.position,
        SceneConfig::FILL_LIGHT.color,
        SceneConfig::FILL_LIGHT.intensity,
        SceneConfig::FILL_LIGHT.enabled
    };

    objPosition = SceneConfig::OBJ_POSITION;
    objScale = SceneConfig::OBJ_SCALE;

    currentView = 0;
    cameraPresetChanged = true;
}

void Scene::Shutdown()
{
    debugHelpers.Shutdown();
}

void Scene::Update(const Input& input)
{
    if (input.ToggleMainLightPressed())
    {
        mainLight.enabled = !mainLight.enabled;
    }

    if (input.ToggleDebugHelpersPressed())
    {
        showDebugHelpers = !showDebugHelpers;
    }

    // Only one camera in this branch.
    if (input.CameraPreset1Pressed())
    {
        currentView = 0;
        cameraPresetChanged = true;
    }
}

void Scene::Render(Shader& shader, Camera& camera)
{
    shader.Activate();

    if (cameraPresetChanged)
    {
        ApplyCameraView(camera);

        camera.updateMatrix(
            SceneConfig::CAMERA_FOV,
            SceneConfig::CAMERA_NEAR,
            SceneConfig::CAMERA_FAR
        );

        cameraPresetChanged = false;
    }

    camera.Matrix(shader, "camMatrix");

    glUniform3f(
        glGetUniformLocation(shader.ID, "camPos"),
        camera.Position.x,
        camera.Position.y,
        camera.Position.z
    );

    SendLight(shader, mainLight, SceneConfig::MAIN_LIGHT.uniformName);
    SendLight(shader, fillLight, SceneConfig::FILL_LIGHT.uniformName);

    if (model)
    {
        model->Draw(shader, camera, GetObjModelMatrix());
    }
}

void Scene::ApplyCameraView(Camera& camera)
{
    currentView = 0;

    const SceneConfig::CameraPreset& preset =
        SceneConfig::CAMERA_PRESETS[0];

    camera.Position = preset.position;
    camera.Orientation = glm::normalize(preset.target - preset.position);
    camera.Up = preset.up;
}

void Scene::BuildDebugHelpers(float aspectRatio)
{
    debugHelpers.Begin();

    for (const SceneConfig::SceneLight& light : SceneConfig::LIGHTS)
    {
        debugHelpers.AddLightHelper(
            light.position,
            light.helperSize,
            light.helperColor
        );
    }

    const SceneConfig::CameraPreset& preset = SceneConfig::CAMERA_PRESETS[0];

    debugHelpers.AddCameraFrustum(
        preset.position,
        glm::normalize(preset.target - preset.position),
        preset.up,
        SceneConfig::CAMERA_FOV,
        aspectRatio,
        SceneConfig::DEBUG_CAMERA_NEAR,
        SceneConfig::DEBUG_CAMERA_FAR,
        preset.helperColor
    );
}

void Scene::RenderDebug(Shader& debugShader, Camera& camera, float aspectRatio)
{
    if (!showDebugHelpers)
    {
        return;
    }

    BuildDebugHelpers(aspectRatio);

    glDisable(GL_DEPTH_TEST);
    debugHelpers.Draw(debugShader, camera);
    glEnable(GL_DEPTH_TEST);
}

void Scene::SendLight(Shader& shader, const Light& light, const std::string& uniformName)
{
    glUniform3f(
        glGetUniformLocation(shader.ID, (uniformName + ".position").c_str()),
        light.position.x,
        light.position.y,
        light.position.z
    );

    glUniform4f(
        glGetUniformLocation(shader.ID, (uniformName + ".color").c_str()),
        light.color.x,
        light.color.y,
        light.color.z,
        light.color.w
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, (uniformName + ".intensity").c_str()),
        light.intensity
    );

    glUniform1i(
        glGetUniformLocation(shader.ID, (uniformName + ".enabled").c_str()),
        light.enabled
    );
}

glm::mat4 Scene::GetObjModelMatrix() const
{
    glm::mat4 matrix = glm::mat4(1.0f);
    matrix = glm::translate(matrix, objPosition);
    matrix = glm::scale(matrix, objScale);
    return matrix;
}