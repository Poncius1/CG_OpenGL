#pragma once

#include <array>

#include <glm.hpp>

namespace SceneConfig
{
    constexpr float CAMERA_FOV = 48.0f;
    constexpr float CAMERA_NEAR = 0.1f;
    constexpr float CAMERA_FAR = 100.0f;

    inline const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

    inline const glm::vec3 BOX_MIN = glm::vec3(-1.0f, -1.0f, -1.0f);
    inline const glm::vec3 BOX_MAX = glm::vec3(1.0f, 1.0f, 1.0f);

    // Estos valores eran para la escena analítica.
    // Se conservan por compatibilidad, pero el raytracer actual usa el OBJ.
    inline const glm::vec3 METAL_BOX_MIN = glm::vec3(-0.70f, -1.00f, -0.55f);
    inline const glm::vec3 METAL_BOX_MAX = glm::vec3(-0.22f, 0.35f, -0.05f);

    inline const glm::vec3 METAL_BOX_ALBEDO = glm::vec3(0.86f, 0.82f, 0.72f);
    constexpr float METAL_BOX_ROUGHNESS = 0.04f;

    inline const glm::vec3 GLASS_SPHERE_ALBEDO = glm::vec3(0.98f, 0.995f, 1.00f);
    constexpr float GLASS_SPHERE_IOR = 1.35f;

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

        // Cerca del panel del techo.
        glm::vec3(0.0f, 0.70f, 0.02f),

        // Blanco ligeramente azul
        glm::vec4(0.85f, 0.92f, 1.00f, 1.0f),

        // Intensidad moderada. 
        1.5f,

        true,

      
        true,

        0.10f,
        glm::vec3(0.70f, 0.85f, 1.0f)
    };

    inline const SceneLight FILL_LIGHT =
    {
        "fillLight",

        // Luz tenue fuera de la caja, al frente.
        glm::vec3(0.0f, 0.15f, 1.50f),

        // Cálida
        glm::vec4(0.87f, 0.31f, 0.10f, 1.0f),

        0.3f,

        true,

        // Siempre encendida.
        false,

        0.10f,
        glm::vec3(1.0f, 0.78f, 0.45f)
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

            // Vista principal estilo Cornell Box.
            // Un poco más centrada y menos abierta que tu configuración actual.
            glm::vec3(0.0f, 0.02f, 2.45f),
            glm::vec3(0.0f, -0.04f, -0.12f),

            WORLD_UP,
            glm::vec3(1.0f, 1.0f, 0.0f)
        },

        CameraPreset
        {
            1,

            // Vista 3/4 derecha para apreciar profundidad, vidrio y metal.
            glm::vec3(1.75f, 0.25f, 2.35f),
            glm::vec3(0.0f, -0.05f, -0.15f),

            WORLD_UP,
            glm::vec3(1.0f, 0.5f, 0.0f)
        },

        CameraPreset
        {
            2,

            // Vista 3/4 izquierda para ver sombras y pared roja.
            glm::vec3(-1.75f, 0.25f, 2.35f),
            glm::vec3(-0.05f, -0.05f, -0.15f),

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