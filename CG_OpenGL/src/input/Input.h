#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>

class Input
{
public:
    void Update(GLFWwindow* window);
    void EndFrame();

    bool IsKeyDown(int key) const;
    bool IsKeyPressed(int key) const;

private:
    std::unordered_map<int, bool> currentKeys;
    std::unordered_map<int, bool> previousKeys;
};