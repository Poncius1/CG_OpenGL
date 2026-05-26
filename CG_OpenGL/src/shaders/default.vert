#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexUV;

out vec3 crntPos;
out vec3 Normal;
out vec3 color;
out vec2 texCoord;

uniform mat4 camMatrix;
uniform mat4 model;

void main()
{
    vec4 worldPosition = model * vec4(aPos, 1.0);

    crntPos = worldPosition.xyz;

    // Matriz correcta para transformar normales aunque haya escalado.
    Normal = mat3(transpose(inverse(model))) * aNormal;

    color = aColor;
    texCoord = aTexUV;

    gl_Position = camMatrix * worldPosition;
}