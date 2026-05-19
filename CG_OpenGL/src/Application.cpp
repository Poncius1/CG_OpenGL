#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm.hpp>

#include "camera/Camera.h"
#include "input/Input.h"
#include "scene/Scene.h"
#include "scene/SceneConfig.h"
#include "rendering/Shader.h"

constexpr unsigned int WINDOW_WIDTH = 960;
constexpr unsigned int WINDOW_HEIGHT = 540;

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
        "Tarea 7 - Blinn Phong/Mapeo",
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

int main()
{
    GLFWwindow* window = CreateWindow();

    if (!window)
        return -1;

    glEnable(GL_DEPTH_TEST);

    Shader shaderProgram("default.vert", "default.frag");
    Shader debugShader("debug.vert", "debug.frag");

    Camera camera(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SceneConfig::CAMERA_PRESETS[0].position
    );

    Input input;

    Scene scene;
    scene.Initialize("models/teapot.obj");

    const float aspectRatio =
        static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

    float lastTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        input.Update(window);

        if (input.ShouldClose())
            glfwSetWindowShouldClose(window, true);

        scene.Update(input);

        camera.Inputs(window, deltaTime);

        camera.updateMatrix(
            SceneConfig::CAMERA_FOV,
            SceneConfig::CAMERA_NEAR,
            SceneConfig::CAMERA_FAR
        );

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.Render(shaderProgram, camera);
        scene.RenderDebug(debugShader, camera, aspectRatio);

        glfwSwapBuffers(window);
        glfwPollEvents();

        input.EndFrame();
    }

    scene.Shutdown();

    debugShader.Delete();
    shaderProgram.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}