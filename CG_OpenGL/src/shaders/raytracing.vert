#version 430 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

out vec2 uv;

void main()
{
    uv = aUV;

    // Full-screen quad: cada fragmento/pixel ejecuta el raytracer.
    gl_Position = vec4(aPos, 0.0, 1.0);
}