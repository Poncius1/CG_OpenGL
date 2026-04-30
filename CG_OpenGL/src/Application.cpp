#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera/Camera.h"
#include "input/Input.h"
#include "scene/Scene.h"
#include "rendering/Shader.h"

constexpr unsigned int WINDOW_WIDTH = 800;
constexpr unsigned int WINDOW_HEIGHT = 800;

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

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CG OpenGL", nullptr, nullptr);

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

    Camera camera(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        glm::vec3(0.0f, 0.0f, 3.0f)
    );

    Input input;

    Scene scene;
    scene.Initialize("models/bunny.obj");

    float lastTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        input.Update(window);

        if (input.IsKeyPressed(GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(window, true);

        camera.Inputs(window, deltaTime);
        camera.updateMatrix(45.0f, 0.1f, 100.0f);

        scene.Update(input);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.Render(shaderProgram, camera);

        glfwSwapBuffers(window);
        glfwPollEvents();

        input.EndFrame();
    }

    shaderProgram.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}