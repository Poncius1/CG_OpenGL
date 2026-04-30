#include "Input.h"

void Input::Update(GLFWwindow* window)
{
    int keys[] =
    {
        GLFW_KEY_ESCAPE,
        GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
        GLFW_KEY_SPACE, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_SHIFT,
        GLFW_KEY_M, GLFW_KEY_L,
        GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3
    };

    for (int key : keys)
    {
        currentKeys[key] = glfwGetKey(window, key) == GLFW_PRESS;
    }
}

void Input::EndFrame()
{
    previousKeys = currentKeys;
}

bool Input::IsKeyDown(int key) const
{
    auto it = currentKeys.find(key);
    return it != currentKeys.end() && it->second;
}

bool Input::IsKeyPressed(int key) const
{
    bool current = false;
    bool previous = false;

    auto currentIt = currentKeys.find(key);
    if (currentIt != currentKeys.end())
        current = currentIt->second;

    auto previousIt = previousKeys.find(key);
    if (previousIt != previousKeys.end())
        previous = previousIt->second;

    return current && !previous;
}