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
    bool enabled;
};

uniform Material material;
uniform Light whiteLight;
uniform Light blueLight;
uniform vec3 camPos;

float hash(vec3 p)
{
    p = fract(p * 0.3183099 + vec3(0.1, 0.2, 0.3));
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float noise(vec3 p)
{
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float n000 = hash(i + vec3(0, 0, 0));
    float n100 = hash(i + vec3(1, 0, 0));
    float n010 = hash(i + vec3(0, 1, 0));
    float n110 = hash(i + vec3(1, 1, 0));
    float n001 = hash(i + vec3(0, 0, 1));
    float n101 = hash(i + vec3(1, 0, 1));
    float n011 = hash(i + vec3(0, 1, 1));
    float n111 = hash(i + vec3(1, 1, 1));

    float nx00 = mix(n000, n100, f.x);
    float nx10 = mix(n010, n110, f.x);
    float nx01 = mix(n001, n101, f.x);
    float nx11 = mix(n011, n111, f.x);

    float nxy0 = mix(nx00, nx10, f.y);
    float nxy1 = mix(nx01, nx11, f.y);

    return mix(nxy0, nxy1, f.z);
}

float fbm(vec3 p)
{
    float value = 0.0;
    float amplitude = 0.5;

    for (int i = 0; i < 5; i++)
    {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }

    return value;
}

vec4 CalculateLight(Light light, vec3 baseColor, vec3 specColor, float shininess)
{
    if (!light.enabled)
        return vec4(0.0);

    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light.position - crntPos);
    vec3 viewDir = normalize(camPos - crntPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    float distance = length(light.position - crntPos);
    float attenuation = 1.0 / (1.0 + 0.04 * distance + 0.012 * distance * distance);

    float diff = max(dot(normal, lightDir), 0.0);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient = baseColor * 0.12 * light.color.rgb;
    vec3 diffuse = baseColor * diff * light.color.rgb;
    vec3 specular = specColor * spec * light.color.rgb * 1.8;

    return vec4((ambient + diffuse + specular) * attenuation, 1.0);
}

vec3 MarbleMaterial()
{
    // Escala más grande = vetas más elegantes
    vec3 p = crntPos * 1.5;

    // Ruido base
    float n = fbm(p * 2.0);

    // Dirección de vetas (esto es clave)
    float veins = sin(p.x * 6.0 + n * 3.0);

    // Suavizar vetas
    veins = smoothstep(-0.3, 0.8, veins);

    // Colores mármol más realistas
    vec3 base = vec3(0.85, 0.84, 0.82);      // blanco roto
    vec3 light = vec3(0.95, 0.94, 0.90);     // zonas claras
    vec3 dark = vec3(0.35, 0.33, 0.30);      // vetas suaves

    // Mezcla principal
    vec3 marble = mix(base, light, n * 0.4);

    // Aplicar vetas (menos agresivas)
    marble = mix(marble, dark, veins * 0.35);

    // ligero tono cálido tipo mármol italiano
    marble *= vec3(1.0, 0.98, 0.95);
    return marble;
}

vec3 DarkBlueMetalMaterial()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(camPos - crntPos);

    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 3.0);

    vec3 deepBlue = vec3(0.01, 0.03, 0.12);
    vec3 electricBlue = vec3(0.0, 0.25, 1.0);
    vec3 edgeGlow = vec3(0.2, 0.65, 1.0);

    float pattern = fbm(crntPos * 8.0);

    vec3 metal = mix(deepBlue, electricBlue, pattern * 0.35);
    metal += edgeGlow * fresnel * 0.8;

    return metal;
}

void main()
{
    vec3 baseColor;
    vec3 specColor;
    float shininess;

    if (material.shininess < 60.0)
    {
        baseColor = MarbleMaterial();
        specColor = vec3(0.8, 0.8, 0.8);
        shininess = 128.0;
    }
    else
    {
        baseColor = DarkBlueMetalMaterial();
        specColor = vec3(0.45, 0.75, 1.0);
        shininess = 160.0;
    }

    vec4 lighting =
        CalculateLight(whiteLight, baseColor, specColor, shininess) +
        CalculateLight(blueLight, baseColor, specColor, shininess);

    FragColor = vec4(lighting.rgb, 1.0);
}