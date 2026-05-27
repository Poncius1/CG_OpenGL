#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>

#include "camera/Camera.h"
#include "input/Input.h"
#include "rendering/GpuTriangle.h"
#include "rendering/RaytracingRenderer.h"
#include "rendering/Shader.h"
#include "scene/Scene.h"
#include "scene/SceneConfig.h"

constexpr unsigned int WINDOW_WIDTH = 960;
constexpr unsigned int WINDOW_HEIGHT = 540;

enum class RenderMode
{
    Rasterized = 0,
    Raytracing = 1
};

GLFWwindow* CreateWindow()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW\n";
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "Proyecto 2 - Cornell Box Raytracing",
        nullptr,
        nullptr
    );

    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    std::cout << "OpenGL Vendor: "
        << glGetString(GL_VENDOR)
        << "\n";

    std::cout << "OpenGL Renderer: "
        << glGetString(GL_RENDERER)
        << "\n";

    std::cout << "OpenGL Version: "
        << glGetString(GL_VERSION)
        << "\n";

    std::cout << "GLSL Version: "
        << glGetString(GL_SHADING_LANGUAGE_VERSION)
        << "\n";

    if (GLEW_VERSION_4_3)
    {
        std::cout << "OpenGL 4.3 supported.\n";
    }
    else
    {
        std::cout << "OpenGL 4.3 NOT supported.\n";
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    return window;
}

void PrintControls()
{
    std::cout << "\nControls:\n";
    std::cout << "  ESC   - Exit\n";
    std::cout << "  R     - Toggle Rasterized / Raytracing\n";
    std::cout << "  1/2/3 - Camera presets\n";
    std::cout << "  L     - Toggle main light\n";
    std::cout << "  M     - Toggle metal material in raytracing\n";
    std::cout << "  G     - Toggle glass material in raytracing\n";
    std::cout << "  H     - Toggle debug helpers\n";
    std::cout << "  T     - Toggle texture mapping in raster mode\n";
    std::cout << "  B     - Toggle Phong / Blinn in raster mode\n";
    std::cout << "\n";
}

void PrintRenderMode(RenderMode mode)
{
    if (mode == RenderMode::Rasterized)
        std::cout << "Render mode: Rasterized\n";
    else
        std::cout << "Render mode: Raytracing\n";
}

int main()
{
    GLFWwindow* window = CreateWindow();

    if (!window)
        return -1;

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    Shader rasterShader(
        "src/shaders/default.vert",
        "src/shaders/default.frag"
    );

    Shader debugShader(
        "src/shaders/debug.vert",
        "src/shaders/debug.frag"
    );

    Shader raytracingShader(
        "src/shaders/raytracing.vert",
        "src/shaders/raytracing.frag"
    );

    Camera camera(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SceneConfig::CAMERA_PRESETS[0].position
    );

    Input input;

    Scene scene;
    scene.Initialize("models/CornellBox.obj");

    RaytracingRenderer raytracingRenderer;
    raytracingRenderer.Initialize();

    const std::vector<GpuTriangle> gpuTriangles =
        scene.BuildGpuTriangles();

    raytracingRenderer.UploadTriangles(gpuTriangles);

    RenderMode renderMode = RenderMode::Raytracing;

    const float aspectRatio =
        static_cast<float>(WINDOW_WIDTH) /
        static_cast<float>(WINDOW_HEIGHT);

    PrintControls();
    PrintRenderMode(renderMode);

    float lastTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window))
    {
        const float currentTime = static_cast<float>(glfwGetTime());
        const float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        input.Update(window);

        if (input.ShouldClose())
        {
            glfwSetWindowShouldClose(window, true);
        }

        if (input.ToggleRenderModePressed())
        {
            renderMode =
                renderMode == RenderMode::Rasterized
                ? RenderMode::Raytracing
                : RenderMode::Rasterized;

            PrintRenderMode(renderMode);
        }

        scene.Update(input);

        camera.Inputs(window, deltaTime);

        scene.ApplyPendingCameraPreset(camera);

        camera.updateMatrix(
            SceneConfig::CAMERA_FOV,
            SceneConfig::CAMERA_NEAR,
            SceneConfig::CAMERA_FAR
        );

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (renderMode == RenderMode::Rasterized)
        {
            scene.Render(rasterShader, camera);
        }
        else
        {
            raytracingRenderer.Render(
                raytracingShader,
                camera,
                aspectRatio,
                scene.IsMainLightEnabled(),
                scene.IsMetalShaderEnabled(),
                scene.IsGlassShaderEnabled()
            );
        }

        scene.RenderDebug(debugShader, camera, aspectRatio);

        glfwSwapBuffers(window);
        glfwPollEvents();

        input.EndFrame();
    }

    raytracingRenderer.Shutdown();
    scene.Shutdown();

    raytracingShader.Delete();
    debugShader.Delete();
    rasterShader.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}