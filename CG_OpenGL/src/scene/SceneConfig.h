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

        // Luz blanca siempre encendida.
        glm::vec3(0.0f, 1.0f, 1.8f),

        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),

        // Intensidad requerida.
        1.0f,

        true,

        // No se apaga.
        false,

        0.12f,
        glm::vec3(1.0f, 1.0f, 1.0f)
    };

    inline const SceneLight FILL_LIGHT =
    {
        "fillLight",

        // Luz azul secundaria.
        glm::vec3(-1.4f, 0.7f, 1.2f),

        glm::vec4(0.25f, 0.45f, 1.0f, 1.0f),

        // Intensidad requerida.
        1.0f,

        true,

        // Esta sí se puede apagar/encender.
        true,

        0.10f,
        glm::vec3(0.30f, 0.45f, 1.0f)
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
            glm::vec3(0.0f, 0.35f, 3.20f),
            glm::vec3(0.0f, 0.15f, 0.0f),

            WORLD_UP,
            glm::vec3(1.0f, 1.0f, 0.0f)
        },

        CameraPreset
        {
            1,

            // Cámara trasera.
            glm::vec3(0.0f, 0.35f, -3.20f),
            glm::vec3(0.0f, 0.15f, 0.0f),

            WORLD_UP,
            glm::vec3(1.0f, 0.5f, 0.0f)
        },

        CameraPreset
        {
            2,

            // Cámara 3/4
            glm::vec3(2.20f, 0.65f, 2.80f),
            glm::vec3(0.0f, 0.20f, 0.0f),

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