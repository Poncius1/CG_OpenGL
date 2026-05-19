#include "Scene.h"

#include <string>

#include "scene/SceneConfig.h"


void Scene::Initialize(const std::string& modelPath)
{
    model = std::make_unique<ObjModel>(modelPath);

    debugHelpers.Initialize();

    
    // Materiales requeridos

    // Material A:
    // Ambiente {0.0, 0.0, 0.0, 1.0}
    // Difusa   {0.50, 0.50, 0.50, 1.0}
    // Especular{0.70, 0.70, 0.70, 1.0}
    // Shininess 32.0
    materialA =
    {
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        glm::vec4(0.50f, 0.50f, 0.50f, 1.0f),
        glm::vec4(0.70f, 0.70f, 0.70f, 1.0f),
        32.0f
    };

    // Material B:
    // Ambiente {0.23125, 0.23125, 0.23125, 1.0}
    // Difusa   {0.2775, 0.2775, 0.2775, 1.0}
    // Especular{0.773911, 0.773911, 0.773911, 1.0}
    // Shininess 89.6
    materialB =
    {
        glm::vec4(0.23125f, 0.23125f, 0.23125f, 1.0f),
        glm::vec4(0.2775f, 0.2775f, 0.2775f, 1.0f),
        glm::vec4(0.773911f, 0.773911f, 0.773911f, 1.0f),
        89.6f
    };

    // --------------------------------------------------
    // Luces desde SceneConfig
    // --------------------------------------------------
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

    // --------------------------------------------------
    // Transform del modelo
    // --------------------------------------------------
    objPosition = SceneConfig::OBJ_POSITION;
    objScale = SceneConfig::OBJ_SCALE;

    currentView = 0;
    cameraPresetChanged = true;

    renderSettings.currentMaterial = 0;
    renderSettings.textureMappingEnabled = true;
    renderSettings.showDebugHelpers = true;
    renderSettings.lightingModel = LightingModel::Phong;
}

void Scene::Shutdown()
{
    debugHelpers.Shutdown();
}

void Scene::Update(const Input& input)
{
    // Cambiar entre material A y B.
    if (input.ToggleMaterialPressed())
    {
        renderSettings.currentMaterial =
            renderSettings.currentMaterial == 0 ? 1 : 0;
    }

    // Encender/apagar la luz azul secundaria.
    if (input.ToggleMainLightPressed())
    {
        fillLight.enabled = !fillLight.enabled;
    }

    // Activar/desactivar textura procedural.
    if (input.ToggleTextureMappingPressed())
    {
        renderSettings.textureMappingEnabled =
            !renderSettings.textureMappingEnabled;
    }

    // Cambiar entre Phong y Blinn-Phong.
    if (input.ToggleLightingModelPressed())
    {
        renderSettings.lightingModel =
            renderSettings.lightingModel == LightingModel::Phong
            ? LightingModel::Blinn
            : LightingModel::Phong;
    }

    // Mostrar/ocultar helpers.
    if (input.ToggleDebugHelpersPressed())
    {
        renderSettings.showDebugHelpers =
            !renderSettings.showDebugHelpers;
    }

    // Presets de cámara.
    if (input.CameraPreset1Pressed())
    {
        currentView = 0;
        cameraPresetChanged = true;
    }

    if (input.CameraPreset2Pressed())
    {
        currentView = 1;
        cameraPresetChanged = true;
    }

    if (input.CameraPreset3Pressed())
    {
        currentView = 2;
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

    if (renderSettings.currentMaterial == 0)
    {
        SendMaterial(shader, materialA);
    }
    else
    {
        SendMaterial(shader, materialB);
    }

    SendLight(shader, mainLight, SceneConfig::MAIN_LIGHT.uniformName);
    SendLight(shader, fillLight, SceneConfig::FILL_LIGHT.uniformName);

    SendRenderSettings(shader);

    if (model)
    {
        glm::mat4 modelMatrix = GetObjModelMatrix();
        model->Draw(shader, camera, modelMatrix);
    }
}

void Scene::RenderDebug(Shader& debugShader, Camera& camera, float aspectRatio)
{
    if (!renderSettings.showDebugHelpers)
        return;

    BuildDebugHelpers(aspectRatio);

    glDisable(GL_DEPTH_TEST);

    debugHelpers.Draw(debugShader, camera);

    glEnable(GL_DEPTH_TEST);
}

void Scene::ApplyCameraView(Camera& camera)
{
    const SceneConfig::CameraPreset& preset =
        SceneConfig::CAMERA_PRESETS[currentView];

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

    for (const SceneConfig::CameraPreset& preset : SceneConfig::CAMERA_PRESETS)
    {
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
}

void Scene::SendMaterial(Shader& shader, const Material& material)
{
    glUniform4f(
        glGetUniformLocation(shader.ID, "material.ambient"),
        material.ambient.x,
        material.ambient.y,
        material.ambient.z,
        material.ambient.w
    );

    glUniform4f(
        glGetUniformLocation(shader.ID, "material.diffuse"),
        material.diffuse.x,
        material.diffuse.y,
        material.diffuse.z,
        material.diffuse.w
    );

    glUniform4f(
        glGetUniformLocation(shader.ID, "material.specular"),
        material.specular.x,
        material.specular.y,
        material.specular.z,
        material.specular.w
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, "material.shininess"),
        material.shininess
    );
}

void Scene::SendLight(
    Shader& shader,
    const Light& light,
    const std::string& uniformName
)
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

void Scene::SendRenderSettings(Shader& shader)
{
    glUniform1i(
        glGetUniformLocation(shader.ID, "textureMappingEnabled"),
        renderSettings.textureMappingEnabled
    );

    glUniform1i(
        glGetUniformLocation(shader.ID, "lightingModel"),
        static_cast<int>(renderSettings.lightingModel)
    );
}

glm::mat4 Scene::GetObjModelMatrix() const
{
    glm::mat4 matrix = glm::mat4(1.0f);

    matrix = glm::translate(matrix, objPosition);
    matrix = glm::scale(matrix, objScale);

    return matrix;
}