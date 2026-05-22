#version 330 core

in vec3 lineColor;
out vec4 FragColor;

uniform float uAlpha;

void main()
{
    FragColor = vec4(lineColor, uAlpha);
}