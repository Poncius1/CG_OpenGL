#pragma once

#include <array>
#include <glm.hpp>

namespace SceneConfig
{
    constexpr float CAMERA_FOV = 42.0f;
    constexpr float CAMERA_NEAR = 0.1f;
    constexpr float CAMERA_FAR = 100.0f;

    inline const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

    inline const glm::vec3 BOX_MIN = glm::vec3(-1.021492f, 0.002552f, -1.031970f);
    inline const glm::vec3 BOX_MAX = glm::vec3(1.005428f, 1.615839f, 0.994950f);

    inline const glm::vec3 AREA_LIGHT_MIN = glm::vec3(-0.247764f, 1.55f, -0.235461f);
    inline const glm::vec3 AREA_LIGHT_MAX = glm::vec3(0.231701f, 1.62f, 0.174748f);
    inline const glm::vec3 AREA_LIGHT_CENTER = (AREA_LIGHT_MIN + AREA_LIGHT_MAX) * 0.5f;

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

    // Main warm rectangular light position.
    inline const SceneLight MAIN_LIGHT =
    {
        "mainLight",
        AREA_LIGHT_CENTER,
       glm::vec4(1.0f, 0.82f, 0.58f, 1.0f),
        8.0f,
        true,
        true,
        0.10f,
        glm::vec3(1.0f, 0.75f, 0.25f)
    };

    // Very soft warm fill so dark areas are still visible.
    inline const SceneLight FILL_LIGHT =
    {
        "fillLight",
        glm::vec3(0.0f, 1.15f, 1.25f),
        glm::vec4(1.0f, 0.72f, 0.45f, 1.0f),
        0.35f,
        true,
        false,
        0.07f,
        glm::vec3(1.0f, 0.55f, 0.25f)
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

    // Single presentation camera. It covers the full Cornell Box.
    inline const CameraPreset MAIN_CAMERA =
    {
        0,
        glm::vec3(0.0f, 0.82f, 3.05f),
        glm::vec3(0.0f, 0.78f, -0.05f),
        WORLD_UP,
        glm::vec3(1.0f, 1.0f, 0.0f)
    };

    inline const std::array<CameraPreset, 1> CAMERA_PRESETS =
    {
        MAIN_CAMERA
    };

    constexpr float DEBUG_CAMERA_NEAR = 0.05f;
    constexpr float DEBUG_CAMERA_FAR = 0.45f;

    inline const glm::vec3 OBJ_POSITION = glm::vec3(0.0f);
    inline const glm::vec3 OBJ_SCALE = glm::vec3(1.0f);
}