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

        // Luz principal en el techo, dentro de la caja.
        glm::vec3(0.0f, 0.70f, -0.10f),

        
        glm::vec4(0.78f, 0.84f, 1.00f, 1.0f),

        1.0f,

        // Encendida al iniciar.
        true,

        // Se puede apagar/encender.
        true,

        // Helper.
        0.12f,
        glm::vec3(0.70f, 0.85f, 1.0f)
    };

    inline const SceneLight FILL_LIGHT =
    {
        "fillLight",

        // Luz tenue fuera de la caja.
        glm::vec3(0.0f, 0.45f, 2.0f),

        
        glm::vec4(0.35f, 0.45f, 1.00f, 1.0f),

        // Muy tenue, siempre encendida.
        0.10f,

        // Encendida.
        true,

        // No se apaga.
        false,

        // Helper.
        0.10f,
        glm::vec3(0.30f, 0.45f, 0.80f)
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

            // Cámara frontal.
            glm::vec3(0.0f, 0.32f, 3.20f),
            glm::vec3(0.0f, 0.12f, 0.0f),

            WORLD_UP,
            glm::vec3(1.0f, 1.0f, 0.0f)
        },

        CameraPreset
        {
            1,

            // Cámara diagonal derecha.
            glm::vec3(1.80f, 0.38f, 2.70f),
            glm::vec3(0.0f, 0.12f, 0.0f),

            WORLD_UP,
            glm::vec3(1.0f, 0.5f, 0.0f)
        },

        CameraPreset
        {
            2,

            // Cámara diagonal izquierda.
            glm::vec3(-1.80f, 0.38f, 2.70f),
            glm::vec3(0.0f, 0.12f, 0.0f),

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