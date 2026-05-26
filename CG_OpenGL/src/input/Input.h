#pragma once

#include <unordered_map>
#include <vector>

#include <GLFW/glfw3.h>

class Input
{
public:
    void Update(GLFWwindow* window);
    void EndFrame();

    bool IsKeyDown(int key) const;
    bool IsKeyPressed(int key) const;
    bool IsKeyReleased(int key) const;

    bool ShouldClose() const;

    bool ToggleRenderModePressed() const;
    bool ToggleDebugHelpersPressed() const;

    bool ToggleMainLightPressed() const;
    bool ToggleMetalShaderPressed() const;
    bool ToggleGlassShaderPressed() const;

    bool ToggleMaterialPressed() const;
    bool ToggleTextureMappingPressed() const;
    bool ToggleLightingModelPressed() const;

    bool CameraPreset1Pressed() const;
    bool CameraPreset2Pressed() const;
    bool CameraPreset3Pressed() const;

private:
    void RegisterTrackedKeys();

private:
    std::unordered_map<int, bool> currentKeys;
    std::unordered_map<int, bool> previousKeys;

    std::vector<int> trackedKeys;

    bool keysRegistered = false;
};