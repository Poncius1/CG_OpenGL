#version 430 core

out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;

struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

struct Light
{
    vec3 position;
    vec4 color;
    float intensity;
    bool enabled;
};

uniform Material material;

uniform Light mainLight;
uniform Light fillLight;

uniform vec3 camPos;

// false = sin textura procedural
// true  = con textura procedural basada en UVs
uniform bool textureMappingEnabled;

// 0 = Phong
// 1 = Blinn-Phong
uniform int lightingModel;

vec3 GetCheckerTexture(vec2 uv)
{
    float scale = 12.0;

    float checker =
        mod(floor(uv.x * scale) + floor(uv.y * scale), 2.0);

    vec3 colorA = vec3(1.0, 1.0, 1.0);
    vec3 colorB = vec3(0.18, 0.18, 0.20);

    return mix(colorA, colorB, checker);
}

vec3 CalculateLight(
    Light light,
    vec3 baseColor,
    vec3 normal,
    vec3 viewDir
)
{
    if (!light.enabled)
    {
        return vec3(0.0);
    }

    vec3 lightVector = light.position - crntPos;
    vec3 lightDir = normalize(lightVector);

    float distanceToLight = length(lightVector);

    float attenuation = 1.0 /
    (
        1.0 +
        0.09 * distanceToLight +
        0.032 * distanceToLight * distanceToLight
    );

    float diffuseFactor = max(dot(normal, lightDir), 0.0);

    float specularFactor = 0.0;

    if (diffuseFactor > 0.0)
    {
        if (lightingModel == 0)
        {
            // Phong:
            // usa el vector reflejado de la luz.
            vec3 reflectDir = reflect(-lightDir, normal);

            specularFactor = pow(
                max(dot(viewDir, reflectDir), 0.0),
                material.shininess
            );
        }
        else
        {
            // Blinn-Phong:
            // usa el halfway vector entre luz y cámara.
            vec3 halfwayDir = normalize(lightDir + viewDir);

            specularFactor = pow(
                max(dot(normal, halfwayDir), 0.0),
                material.shininess
            );
        }
    }

    vec3 lightRadiance =
        light.color.rgb *
        light.intensity *
        attenuation;

    vec3 ambient =
        material.ambient.rgb *
        baseColor *
        light.color.rgb *
        light.intensity;

    vec3 diffuse =
        material.diffuse.rgb *
        baseColor *
        diffuseFactor *
        lightRadiance;

    vec3 specular =
        material.specular.rgb *
        specularFactor *
        lightRadiance;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(camPos - crntPos);

    // Ayuda con modelos exportados con normales hacia afuera
    // cuando se observan superficies internas.
    if (dot(normal, viewDir) < 0.0)
    {
        normal = -normal;
    }

    // Color base:
    // - color viene del loader/OBJ.
    // - Si tu modelo no trae colores, normalmente será vec3(1.0).
    vec3 baseColor = color;

    // Si la textura procedural está activa, se combina con el color base.
    if (textureMappingEnabled)
    {
        vec3 checkerColor = GetCheckerTexture(texCoord);
        baseColor *= checkerColor;
    }

    vec3 finalColor = vec3(0.0);

    finalColor += CalculateLight(mainLight, baseColor, normal, viewDir);
    finalColor += CalculateLight(fillLight, baseColor, normal, viewDir);

    // Evita valores fuera de rango antes de gamma.
    finalColor = clamp(finalColor, 0.0, 1.0);

    // Gamma correction simple para que el resultado no se vea tan oscuro.
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, 1.0);
}