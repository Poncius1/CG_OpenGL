#include "Input.h"

void Input::RegisterTrackedKeys()
{
    if (keysRegistered)
        return;

    trackedKeys =
    {
        GLFW_KEY_ESCAPE,

        GLFW_KEY_W,
        GLFW_KEY_A,
        GLFW_KEY_S,
        GLFW_KEY_D,

        GLFW_KEY_SPACE,
        GLFW_KEY_LEFT_CONTROL,
        GLFW_KEY_LEFT_SHIFT,

        GLFW_KEY_M,
        GLFW_KEY_T,
        GLFW_KEY_L,
        GLFW_KEY_H,
        GLFW_KEY_B,

        GLFW_KEY_1,
        GLFW_KEY_2,
        GLFW_KEY_3
    };

    for (int key : trackedKeys)
    {
        currentKeys[key] = false;
        previousKeys[key] = false;
    }

    keysRegistered = true;
}

void Input::Update(GLFWwindow* window)
{
    RegisterTrackedKeys();

    for (int key : trackedKeys)
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

bool Input::IsKeyReleased(int key) const
{
    bool current = false;
    bool previous = false;

    auto currentIt = currentKeys.find(key);
    if (currentIt != currentKeys.end())
        current = currentIt->second;

    auto previousIt = previousKeys.find(key);
    if (previousIt != previousKeys.end())
        previous = previousIt->second;

    return !current && previous;
}

bool Input::ShouldClose() const
{
    return IsKeyPressed(GLFW_KEY_ESCAPE);
}

bool Input::ToggleDebugHelpersPressed() const
{
    return IsKeyPressed(GLFW_KEY_H);
}

bool Input::ToggleMainLightPressed() const
{
    return IsKeyPressed(GLFW_KEY_L);
}

bool Input::CameraPreset1Pressed() const
{
    return IsKeyPressed(GLFW_KEY_1);
}

bool Input::CameraPreset2Pressed() const
{
    return IsKeyPressed(GLFW_KEY_2);
}

bool Input::CameraPreset3Pressed() const
{
    return IsKeyPressed(GLFW_KEY_3);
}

bool Input::ToggleMaterialPressed() const
{
    return IsKeyPressed(GLFW_KEY_M);
}

bool Input::ToggleTextureMappingPressed() const
{
    return IsKeyPressed(GLFW_KEY_T);
}

bool Input::ToggleLightingModelPressed() const
{
    return IsKeyPressed(GLFW_KEY_B);
}