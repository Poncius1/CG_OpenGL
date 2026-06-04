#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>

#include "camera/Camera.h"
#include "input/Input.h"
#include "scene/Scene.h"
#include "scene/SceneConfig.h"
#include "rendering/Shader.h"

#include "rl-visualizer/trajectoryLoader.h"
#include "rl-visualizer/trajectoryRenderer.h"

constexpr unsigned int WINDOW_WIDTH = 960;
constexpr unsigned int WINDOW_HEIGHT = 540;

const std::string MODEL_PATH = "models/CornellBox.obj";
const std::string TRAJECTORY_JSON_PATH = "rl-paths/paths_debug.json";

GLFWwindow* CreateWindow()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW\n";
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "MORL Visualizer",
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

    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    return window;
}

bool IsKeyPressedOnce(GLFWwindow* window, int key)
{
    static bool previousKeyState[GLFW_KEY_LAST + 1] = {};

    const bool isPressed = glfwGetKey(window, key) == GLFW_PRESS;
    const bool pressedOnce = isPressed && !previousKeyState[key];

    previousKeyState[key] = isPressed;

    return pressedOnce;
}

bool IsAnyKeyPressedOnce(GLFWwindow* window, int keyA, int keyB)
{
    return IsKeyPressedOnce(window, keyA) || IsKeyPressedOnce(window, keyB);
}

void PrintControls()
{
    std::cout << "\n=== MORL Visualizer ===\n";
    std::cout << "Gold = reached light | Red = failed\n\n";

    std::cout << "R : Toggle random\n";
    std::cout << "B : Toggle light-biased\n";
    std::cout << "P : Toggle Pareto Q-Learning\n";
    std::cout << "T : Filter all / success only / failures only\n";
    std::cout << "F : Full path / first segment only\n";
    std::cout << "+ : Increase visible rays per method\n";
    std::cout << "- : Decrease visible rays per method\n";
    std::cout << "J : Reload JSON\n";
    std::cout << "H : Help + current statistics\n";
    std::cout << "ESC : Exit\n";
    std::cout << "=======================\n\n";
}

bool LoadTrajectories(
    const std::string& jsonPath,
    std::vector<TrajectoryPath>& randomPaths,
    std::vector<TrajectoryPath>& lightBiasedPaths,
    std::vector<TrajectoryPath>& pqlPaths,
    TrajectoryRenderer& trajectoryRenderer
)
{
    const bool loaded = TrajectoryLoader::LoadFromJson(
        jsonPath,
        randomPaths,
        lightBiasedPaths,
        pqlPaths
    );

    if (!loaded)
    {
        std::cout << "WARNING: Could not load trajectories from: "
            << jsonPath << std::endl;
        return false;
    }

    trajectoryRenderer.SetPaths(
        &randomPaths,
        &lightBiasedPaths,
        &pqlPaths
    );

    std::cout << "\nLoaded trajectory JSON: " << jsonPath << "\n";
    trajectoryRenderer.PrintVisibleStatistics();

    return true;
}

int main()
{
    GLFWwindow* window = CreateWindow();

    if (!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Needed to see the Cornell Box interior.
    glDisable(GL_CULL_FACE);

    Shader shaderProgram("default.vert", "default.frag");
    Shader debugShader("debug.vert", "debug.frag");
    Shader trajectoryShader("trajectory.vert", "trajectory.frag");

    Camera camera(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SceneConfig::CAMERA_PRESETS[0].position
    );

    Input input;

    Scene scene;
    scene.Initialize(MODEL_PATH);

    TrajectoryRenderer trajectoryRenderer;
    trajectoryRenderer.Initialize();

    std::vector<TrajectoryPath> randomPaths;
    std::vector<TrajectoryPath> lightBiasedPaths;
    std::vector<TrajectoryPath> pqlPaths;

    LoadTrajectories(
        TRAJECTORY_JSON_PATH,
        randomPaths,
        lightBiasedPaths,
        pqlPaths,
        trajectoryRenderer
    );

    PrintControls();

    const float aspectRatio =
        static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

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

        if (IsKeyPressedOnce(window, GLFW_KEY_H))
        {
            PrintControls();
            trajectoryRenderer.PrintVisibleStatistics();
        }

        if (IsKeyPressedOnce(window, GLFW_KEY_R))
        {
            trajectoryRenderer.ToggleRandom();
            std::cout << "Random: "
                << (trajectoryRenderer.IsShowingRandom() ? "ON" : "OFF")
                << std::endl;
        }

        if (IsKeyPressedOnce(window, GLFW_KEY_B))
        {
            trajectoryRenderer.ToggleLightBiased();
            std::cout << "Light-biased: "
                << (trajectoryRenderer.IsShowingLightBiased() ? "ON" : "OFF")
                << std::endl;
        }

        if (IsKeyPressedOnce(window, GLFW_KEY_P))
        {
            trajectoryRenderer.TogglePql();
            std::cout << "Pareto Q-Learning: "
                << (trajectoryRenderer.IsShowingPql() ? "ON" : "OFF")
                << std::endl;
        }

        if (IsKeyPressedOnce(window, GLFW_KEY_T))
        {
            trajectoryRenderer.CycleFilterMode();
            trajectoryRenderer.PrintVisibleStatistics();
        }

        if (IsKeyPressedOnce(window, GLFW_KEY_F))
        {
            trajectoryRenderer.ToggleFirstSegmentMode();
            std::cout << "Draw mode: "
                << (trajectoryRenderer.IsFirstSegmentMode() ? "first segment only" : "full path")
                << std::endl;
        }

        if (IsKeyPressedOnce(window, GLFW_KEY_J))
        {
            LoadTrajectories(
                TRAJECTORY_JSON_PATH,
                randomPaths,
                lightBiasedPaths,
                pqlPaths,
                trajectoryRenderer
            );
        }

        if (IsAnyKeyPressedOnce(window, GLFW_KEY_EQUAL, GLFW_KEY_KP_ADD))
        {
            trajectoryRenderer.IncreaseVisiblePaths();
            trajectoryRenderer.PrintVisibleStatistics();
        }

        if (IsAnyKeyPressedOnce(window, GLFW_KEY_MINUS, GLFW_KEY_KP_SUBTRACT))
        {
            trajectoryRenderer.DecreaseVisiblePaths();
            trajectoryRenderer.PrintVisibleStatistics();
        }

        scene.Update(input);
        camera.Inputs(window, deltaTime);

        camera.updateMatrix(
            SceneConfig::CAMERA_FOV,
            SceneConfig::CAMERA_NEAR,
            SceneConfig::CAMERA_FAR
        );

        glClearColor(0.018f, 0.017f, 0.016f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.Render(shaderProgram, camera);
        trajectoryRenderer.Draw(trajectoryShader, camera);
        scene.RenderDebug(debugShader, camera, aspectRatio);

        glfwSwapBuffers(window);
        glfwPollEvents();

        input.EndFrame();
    }

    trajectoryRenderer.Shutdown();
    scene.Shutdown();

    trajectoryShader.Delete();
    debugShader.Delete();
    shaderProgram.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}