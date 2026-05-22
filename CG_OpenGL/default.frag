#version 330 core

out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;

struct Light
{
    vec3 position;
    vec4 color;
    float intensity;
    bool enabled;
};

uniform Light mainLight;
uniform Light fillLight;
uniform vec3 camPos;

bool IsLightPanelColor(vec3 baseColor)
{
    return baseColor.r > 0.95 && baseColor.g > 0.80 && baseColor.b < 0.75;
}

vec3 CalculatePointLight(
    Light light,
    vec3 baseColor,
    vec3 specularColor,
    float shininess
)
{
    if (!light.enabled)
    {
        return vec3(0.0);
    }

    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(camPos - crntPos);

    // Cornell Box is viewed from inside, so two-sided lighting helps.
    if (dot(normal, viewDir) < 0.0)
    {
        normal = -normal;
    }

    vec3 lightVector = light.position - crntPos;
    vec3 lightDir = normalize(lightVector);

    float distanceToLight = length(lightVector);

    float attenuation = 1.0 /
    (
        1.0 +
        0.18 * distanceToLight +
        0.12 * distanceToLight * distanceToLight
    );

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    vec3 radiance = light.color.rgb * light.intensity * attenuation;

    vec3 diffuse = baseColor * diff * radiance;
    vec3 specular = specularColor * spec * radiance * 0.18;

    return diffuse + specular;
}

void main()
{
    vec3 baseColor = color;

    // Visual emissive light panel.
    if (IsLightPanelColor(baseColor))
    {
        vec3 emissive = vec3(1.0, 0.88, 0.58) * 1.55;
        FragColor = vec4(clamp(emissive, 0.0, 1.0), 1.0);
        return;
    }

    float shininess = 24.0;
    vec3 specularColor = vec3(0.06);

    // Sphere / brighter objects.
    if (baseColor.r > 0.70 && baseColor.g > 0.70 && baseColor.b > 0.70)
    {
        shininess = 42.0;
        specularColor = vec3(0.14);
    }

    vec3 warmAmbient = vec3(0.19, 0.155, 0.115);
    vec3 ambient = baseColor * warmAmbient;

    vec3 directLighting =
        CalculatePointLight(mainLight, baseColor, specularColor, shininess) +
        CalculatePointLight(fillLight, baseColor, specularColor, 16.0);

    // Fake warm indirect bounce for Cornell Box feel.
    vec3 indirectBounce = baseColor * vec3(0.13, 0.095, 0.065);

    // Subtle top warm gradient.
    float heightFactor = clamp((crntPos.y - 0.05) / 1.55, 0.0, 1.0);
    vec3 ceilingWarmth = baseColor * vec3(0.08, 0.055, 0.035) * heightFactor;

    vec3 finalColor = ambient + directLighting + indirectBounce + ceilingWarmth;

    // Filmic compression.
    finalColor = finalColor / (finalColor + vec3(1.0));

    // Gamma correction.
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, 1.0);
}