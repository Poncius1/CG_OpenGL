#pragma once

#include <array>

#include <glm.hpp>

namespace SceneConfig
{
    constexpr float CAMERA_FOV = 45.0f;
    constexpr float CAMERA_NEAR = 0.1f;
    constexpr float CAMERA_FAR = 100.0f;

    inline const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

    inline const glm::vec3 BOX_MIN = glm::vec3(-1.0f, -1.0f, -1.0f);
    inline const glm::vec3 BOX_MAX = glm::vec3(1.0f, 1.0f, 1.0f);

    // --------------------------------------------------
// Raytracing analytic objects
// --------------------------------------------------

    inline const glm::vec3 METAL_BOX_MIN = glm::vec3(-0.70f, -1.00f, -0.55f);
    inline const glm::vec3 METAL_BOX_MAX = glm::vec3(-0.22f, 0.35f, -0.05f);

    inline const glm::vec3 METAL_BOX_ALBEDO = glm::vec3(0.86f, 0.82f, 0.72f);
    constexpr float METAL_BOX_ROUGHNESS = 0.04f;

    inline const glm::vec3 GLASS_SPHERE_ALBEDO = glm::vec3(0.90f, 0.97f, 1.00f);
    constexpr float GLASS_SPHERE_IOR = 1.5f;



    struct SceneLight
    {
        const char* uniformName;

        glm::vec3 position;
        glm::vec4 color;
        float intensity;
        bool enabled;

        bool canToggle;

        float helperSize;
        glm::vec3 helperColor;
    };

    inline const SceneLight MAIN_LIGHT =
    {
        "mainLight",

        // Luz principal cerca del techo, ligeramente hacia el frente.
        glm::vec3(0.0f, 0.92f, 0.10f),

        // Blanco con tinte azul.
        glm::vec4(0.78f, 0.84f, 1.00f, 1.0f),

        // Suficiente para iluminar sin quemar demasiado.
        3.2f,

        true,

        // Toggleable.
        true,

        0.12f,
        glm::vec3(0.70f, 0.85f, 1.0f)
    };

    inline const SceneLight FILL_LIGHT =
    {
        "fillLight",

        // Luz tenue fuera de la caja, al frente e izquierda.
        glm::vec3(-1.35f, 0.35f, 2.35f),

        // Azul más suave.
        glm::vec4(0.25f, 0.38f, 1.00f, 1.0f),

        // Muy tenue, siempre encendida.
        0.55f,

        true,

        // No se apaga.
        false,

        0.10f,
        glm::vec3(0.30f, 0.45f, 0.90f)
    };

    inline const std::array<SceneLight, 2> LIGHTS =
    {
        MAIN_LIGHT,
        FILL_LIGHT
    };

    struct CameraPreset
    {
        int index;

        glm::vec3 position;
        glm::vec3 target;
        glm::vec3 up;

        glm::vec3 helperColor;
    };

    inline const std::array<CameraPreset, 3> CAMERA_PRESETS =
    {
        CameraPreset
        {
            0,

            // Cámara frontal. Buena para comparar raster vs raytracing.
            glm::vec3(0.0f, 0.10f, 3.15f),
            glm::vec3(0.0f, 0.00f, 0.0f),

            WORLD_UP,
            glm::vec3(1.0f, 1.0f, 0.0f)
        },

        CameraPreset
        {
            1,

            // Cámara 3/4. Mejor para apreciar metal, vidrio y profundidad.
            glm::vec3(2.25f, 0.45f, 2.85f),
            glm::vec3(0.0f, 0.00f, -0.05f),

            WORLD_UP,
            glm::vec3(1.0f, 0.5f, 0.0f)
        },

        CameraPreset
        {
            2,

            // Cámara lateral/interior. Ayuda a ver sombras y refracción.
            glm::vec3(-2.15f, 0.35f, 2.25f),
            glm::vec3(-0.15f, -0.05f, -0.05f),

            WORLD_UP,
            glm::vec3(0.0f, 1.0f, 1.0f)
        }
    };

    constexpr float DEBUG_CAMERA_NEAR = 0.05f;
    constexpr float DEBUG_CAMERA_FAR = 0.45f;

    inline const glm::vec3 METAL_SPHERE_POSITION = glm::vec3(-0.45f, -0.55f, 0.15f);
    inline const glm::vec3 GLASS_SPHERE_POSITION = glm::vec3(0.45f, -0.55f, -0.20f);

    constexpr float METAL_SPHERE_RADIUS = 0.35f;
    constexpr float GLASS_SPHERE_RADIUS = 0.35f;

    inline const glm::vec3 OBJ_POSITION = glm::vec3(0.0f, 0.0f, 0.0f);
    inline const glm::vec3 OBJ_SCALE = glm::vec3(1.0f);
}