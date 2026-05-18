#include "Scene.h"
#include "scene/SceneConfig.h"

void Scene::Initialize(const std::string& modelPath)
{
    model = std::make_unique<ObjModel>(modelPath);

    debugHelpers.Initialize();

    whiteWallMaterial =
    {
        glm::vec4(0.08f, 0.08f, 0.08f, 1.0f),
        glm::vec4(0.80f, 0.78f, 0.70f, 1.0f),
        glm::vec4(0.10f, 0.10f, 0.10f, 1.0f),
        16.0f
    };

    redWallMaterial =
    {
        glm::vec4(0.08f, 0.01f, 0.01f, 1.0f),
        glm::vec4(0.65f, 0.05f, 0.04f, 1.0f),
        glm::vec4(0.05f, 0.02f, 0.02f, 1.0f),
        8.0f
    };

    greenWallMaterial =
    {
        glm::vec4(0.01f, 0.08f, 0.01f, 1.0f),
        glm::vec4(0.08f, 0.45f, 0.10f, 1.0f),
        glm::vec4(0.02f, 0.05f, 0.02f, 1.0f),
        8.0f
    };

    metalSphereMaterial =
    {
        glm::vec4(0.04f, 0.04f, 0.04f, 1.0f),
        glm::vec4(0.75f, 0.72f, 0.65f, 1.0f),
        glm::vec4(1.00f, 0.95f, 0.85f, 1.0f),
        128.0f
    };

    glassSphereMaterial =
    {
        glm::vec4(0.02f, 0.03f, 0.04f, 1.0f),
        glm::vec4(0.45f, 0.70f, 0.90f, 0.35f),
        glm::vec4(0.90f, 0.95f, 1.00f, 1.0f),
        96.0f
    };

    objMaterial =
    {
        glm::vec4(0.04f, 0.04f, 0.04f, 1.0f),
        glm::vec4(0.70f, 0.66f, 0.58f, 1.0f),
        glm::vec4(0.30f, 0.30f, 0.30f, 1.0f),
        32.0f
    };

    mainLight =
    {
        SceneConfig::MAIN_LIGHT.position,
        SceneConfig::MAIN_LIGHT.color,
        SceneConfig::MAIN_LIGHT.intensity,
        SceneConfig::MAIN_LIGHT.enabled
    };

    fillLight =
    {
        SceneConfig::MAIN_LIGHT.position,
        SceneConfig::MAIN_LIGHT.color,
        SceneConfig::MAIN_LIGHT.intensity,
        SceneConfig::MAIN_LIGHT.enabled
    };

    metalSpherePosition = SceneConfig::METAL_SPHERE_POSITION;
    glassSpherePosition = SceneConfig::GLASS_SPHERE_POSITION;

    metalSphereRadius = SceneConfig::METAL_SPHERE_RADIUS;
    glassSphereRadius = SceneConfig::GLASS_SPHERE_RADIUS;

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
    if (input.ToggleMetalShaderPressed())
        metalShaderEnabled = !metalShaderEnabled;

    if (input.ToggleGlassShaderPressed())
        glassShaderEnabled = !glassShaderEnabled;

    if (input.ToggleMainLightPressed())
        mainLight.enabled = !mainLight.enabled;

    if (input.ToggleDebugHelpersPressed())
        showDebugHelpers = !showDebugHelpers;

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

        //modificar Position/Orientation aquí,
        camera.updateMatrix(SceneConfig::CAMERA_FOV,
            SceneConfig::CAMERA_NEAR,
            SceneConfig::CAMERA_FAR);

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

    glUniform1i(
        glGetUniformLocation(shader.ID, "metalShaderEnabled"),
        metalShaderEnabled
    );

    glUniform1i(
        glGetUniformLocation(shader.ID, "glassShaderEnabled"),
        glassShaderEnabled
    );

    SendRaytracingSceneUniforms(shader);

    // Por ahora seguimos dibujando el OBJ con rasterización.
    // Después esto se reemplazará por un full-screen quad para raytracing.
    if (model)
    {
        SendMaterial(shader, objMaterial);

        glm::mat4 modelMatrix = GetObjModelMatrix();

        model->Draw(shader, camera, modelMatrix);
    }
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

void Scene::RenderDebug(Shader& debugShader, Camera& camera, float aspectRatio)
{
    if (!showDebugHelpers)
        return;

    BuildDebugHelpers(aspectRatio);

    glDisable(GL_DEPTH_TEST);

    debugHelpers.Draw(debugShader, camera);

    glEnable(GL_DEPTH_TEST);
}

void Scene::SendRaytracingSceneUniforms(Shader& shader)
{
    glUniform3f(
        glGetUniformLocation(shader.ID, "metalSpherePosition"),
        metalSpherePosition.x,
        metalSpherePosition.y,
        metalSpherePosition.z
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, "metalSphereRadius"),
        metalSphereRadius
    );

    glUniform3f(
        glGetUniformLocation(shader.ID, "glassSpherePosition"),
        glassSpherePosition.x,
        glassSpherePosition.y,
        glassSpherePosition.z
    );

    glUniform1f(
        glGetUniformLocation(shader.ID, "glassSphereRadius"),
        glassSphereRadius
    );

    glUniform3f(
        glGetUniformLocation(shader.ID, "boxMin"),
        SceneConfig::BOX_MIN.x,
        SceneConfig::BOX_MIN.y,
        SceneConfig::BOX_MIN.z
    );

    glUniform3f(
        glGetUniformLocation(shader.ID, "boxMax"),
        SceneConfig::BOX_MAX.x,
        SceneConfig::BOX_MAX.y,
        SceneConfig::BOX_MAX.z
    );
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