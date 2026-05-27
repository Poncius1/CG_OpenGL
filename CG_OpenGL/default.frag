#version 330 core

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

uniform bool textureMappingEnabled;
uniform bool modelHasUVs;

// 0 = Phong
// 1 = Blinn-Phong
uniform int lightingModel;

vec3 GetCheckerTexture(vec2 uv)
{
    float scale = 8.0;

    float checker =
        mod(floor(uv.x * scale) + floor(uv.y * scale), 2.0);

    vec3 colorA = vec3(1.0, 1.0, 1.0);
    vec3 colorB = vec3(0.18, 0.18, 0.20);

    return mix(colorA, colorB, checker);
}

vec3 GetTriplanarChecker(vec3 position, vec3 normal)
{
    float scale = 2.0;

    vec3 blend = abs(normal);
    blend = max(blend, vec3(0.0001));
    blend /= (blend.x + blend.y + blend.z);

    vec3 xProjection = GetCheckerTexture(position.zy * scale);
    vec3 yProjection = GetCheckerTexture(position.xz * scale);
    vec3 zProjection = GetCheckerTexture(position.xy * scale);

    return
        xProjection * blend.x +
        yProjection * blend.y +
        zProjection * blend.z;
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
            vec3 reflectDir = reflect(-lightDir, normal);

            specularFactor = pow(
                max(dot(viewDir, reflectDir), 0.0),
                material.shininess
            );
        }
        else
        {
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

    if (dot(normal, viewDir) < 0.0)
    {
        normal = -normal;
    }

    vec3 baseColor = color;

    if (textureMappingEnabled)
    {
        vec3 checkerColor;

        if (modelHasUVs)
        {
            checkerColor = GetCheckerTexture(texCoord);
        }
        else
        {
            checkerColor = GetTriplanarChecker(crntPos, normal);
        }

        baseColor *= checkerColor;
    }

    vec3 finalColor = vec3(0.0);

    finalColor += CalculateLight(mainLight, baseColor, normal, viewDir);
    finalColor += CalculateLight(fillLight, baseColor, normal, viewDir);

    finalColor = clamp(finalColor, 0.0, 1.0);
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, 1.0);
}