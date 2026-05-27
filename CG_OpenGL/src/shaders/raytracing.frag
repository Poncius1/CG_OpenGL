#version 430 core

out vec4 FragColor;

in vec2 uv;

// ==================================================
// Constants
// ==================================================

const float INF = 100000.0;
const float EPSILON = 0.0015;

const int MAX_BOUNCES = 5;
const int MAX_BVH_STACK = 64;

const int MATERIAL_DIFFUSE = 0;
const int MATERIAL_METAL = 1;
const int MATERIAL_GLASS = 2;
const int MATERIAL_LIGHT = 3;

// ==================================================
// Data
// ==================================================

struct Ray
{
    vec3 origin;
    vec3 direction;
};

struct Hit
{
    bool hit;
    float t;

    vec3 position;
    vec3 normal;
    vec3 albedo;

    int materialType;

    float roughness;
    float ior;
    float emission;
};

struct Light
{
    vec3 position;
    vec4 color;
    float intensity;
    bool enabled;
};

struct GpuTriangle
{
    vec4 v0;
    vec4 v1;
    vec4 v2;

    vec4 n0;
    vec4 n1;
    vec4 n2;

    vec4 albedo;
    vec4 materialData;
};

struct GpuBvhNode
{
    vec4 boundsMin;
    vec4 boundsMax;
    ivec4 data;
};

// ==================================================
// Buffers
// ==================================================

layout(std430, binding = 0) readonly buffer TriangleBuffer
{
    GpuTriangle objTriangles[];
};

layout(std430, binding = 1) readonly buffer BvhBuffer
{
    GpuBvhNode bvhNodes[];
};

// ==================================================
// Uniforms
// ==================================================

uniform int objTriangleCount;
uniform int bvhNodeCount;
uniform bool useObjTriangles;

uniform vec3 cameraPosition;
uniform vec3 cameraForward;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float cameraFov;
uniform float aspectRatio;

uniform bool metalShaderEnabled;
uniform bool glassShaderEnabled;

uniform Light mainLight;
uniform Light fillLight;

// ==================================================
// Utility
// ==================================================

Hit NoHit()
{
    Hit hit;

    hit.hit = false;
    hit.t = INF;
    hit.position = vec3(0.0);
    hit.normal = vec3(0.0, 1.0, 0.0);
    hit.albedo = vec3(0.0);
    hit.materialType = MATERIAL_DIFFUSE;
    hit.roughness = 1.0;
    hit.ior = 1.0;
    hit.emission = 0.0;

    return hit;
}

void KeepClosest(inout Hit closestHit, Hit candidate)
{
    if (candidate.hit && candidate.t < closestHit.t)
    {
        closestHit = candidate;
    }
}

vec3 FaceNormalAgainstRay(vec3 normal, Ray ray)
{
    return dot(normal, ray.direction) > 0.0 ? -normal : normal;
}

float SchlickFresnel(float cosTheta, float ior)
{
    float r0 = (1.0 - ior) / (1.0 + ior);
    r0 *= r0;

    return r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);
}

Ray GenerateCameraRay(vec2 screenUV)
{
    vec2 ndc = screenUV * 2.0 - 1.0;

    float halfFov = radians(cameraFov) * 0.5;
    float scale = tan(halfFov);

    vec3 direction =
        cameraForward +
        cameraRight * ndc.x * aspectRatio * scale +
        cameraUp * ndc.y * scale;

    Ray ray;
    ray.origin = cameraPosition;
    ray.direction = normalize(direction);

    return ray;
}

Hit CreateHit(
    float t,
    Ray ray,
    vec3 normal,
    vec3 albedo,
    int materialType,
    float roughness,
    float ior,
    float emission
)
{
    Hit hit;

    hit.hit = true;
    hit.t = t;
    hit.position = ray.origin + ray.direction * t;
    hit.normal = normalize(normal);
    hit.albedo = albedo;
    hit.materialType = materialType;
    hit.roughness = roughness;
    hit.ior = ior;
    hit.emission = emission;

    return hit;
}

// ==================================================
// Intersections
// ==================================================

bool IntersectAabb(
    Ray ray,
    vec3 boundsMin,
    vec3 boundsMax,
    float maxDistance
)
{
    vec3 safeDir = ray.direction;

    if (abs(safeDir.x) < 0.00001) safeDir.x = 0.00001;
    if (abs(safeDir.y) < 0.00001) safeDir.y = 0.00001;
    if (abs(safeDir.z) < 0.00001) safeDir.z = 0.00001;

    vec3 invDir = 1.0 / safeDir;

    vec3 t0 = (boundsMin - ray.origin) * invDir;
    vec3 t1 = (boundsMax - ray.origin) * invDir;

    vec3 tMin = min(t0, t1);
    vec3 tMax = max(t0, t1);

    float nearT = max(max(tMin.x, tMin.y), tMin.z);
    float farT = min(min(tMax.x, tMax.y), tMax.z);

    return farT >= max(nearT, 0.0) && nearT < maxDistance;
}

Hit IntersectTriangle(Ray ray, GpuTriangle triangle)
{
    vec3 v0 = triangle.v0.xyz;
    vec3 v1 = triangle.v1.xyz;
    vec3 v2 = triangle.v2.xyz;

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;

    vec3 pvec = cross(ray.direction, edge2);
    float det = dot(edge1, pvec);

    if (abs(det) < 0.000001)
    {
        return NoHit();
    }

    float invDet = 1.0 / det;

    vec3 tvec = ray.origin - v0;
    float u = dot(tvec, pvec) * invDet;

    if (u < 0.0 || u > 1.0)
    {
        return NoHit();
    }

    vec3 qvec = cross(tvec, edge1);
    float v = dot(ray.direction, qvec) * invDet;

    if (v < 0.0 || u + v > 1.0)
    {
        return NoHit();
    }

    float t = dot(edge2, qvec) * invDet;

    if (t < EPSILON)
    {
        return NoHit();
    }

    vec3 normal = normalize(
        triangle.n0.xyz * (1.0 - u - v) +
        triangle.n1.xyz * u +
        triangle.n2.xyz * v
    );

    vec4 data = triangle.materialData;

    return CreateHit(
        t,
        ray,
        normal,
        triangle.albedo.rgb,
        int(round(data.x)),
        data.y,
        data.z,
        data.w
    );
}

Hit TraceScene(Ray ray)
{
    Hit closestHit = NoHit();

    if (!useObjTriangles || bvhNodeCount <= 0)
    {
        return closestHit;
    }

    int stack[MAX_BVH_STACK];
    int stackSize = 1;
    stack[0] = 0;

    while (stackSize > 0)
    {
        int nodeIndex = stack[--stackSize];

        if (nodeIndex < 0 || nodeIndex >= bvhNodeCount)
        {
            continue;
        }

        GpuBvhNode node = bvhNodes[nodeIndex];

        if (!IntersectAabb(ray, node.boundsMin.xyz, node.boundsMax.xyz, closestHit.t))
        {
            continue;
        }

        int triangleCount = node.data.w;

        if (triangleCount > 0)
        {
            int firstTriangle = node.data.z;

            for (int i = 0; i < triangleCount; ++i)
            {
                int triangleIndex = firstTriangle + i;

                if (triangleIndex < objTriangleCount)
                {
                    KeepClosest(
                        closestHit,
                        IntersectTriangle(ray, objTriangles[triangleIndex])
                    );
                }
            }

            continue;
        }

        if (stackSize + 2 <= MAX_BVH_STACK)
        {
            stack[stackSize++] = node.data.x;
            stack[stackSize++] = node.data.y;
        }
    }

    return closestHit;
}

// ==================================================
// Shadows
// ==================================================

bool IsOccluded(vec3 origin, vec3 normal, vec3 lightPosition)
{
    vec3 toLight = lightPosition - origin;
    float lightDistance = length(toLight);

    Ray shadowRay;
    shadowRay.origin = origin + normal * EPSILON * 2.0;
    shadowRay.direction = normalize(toLight);

    Hit shadowHit = TraceScene(shadowRay);

    if (!shadowHit.hit || shadowHit.t >= lightDistance)
    {
        return false;
    }

    return shadowHit.materialType != MATERIAL_LIGHT;
}

// ==================================================
// Lighting
// ==================================================

vec3 ShadeLight(Light light, Hit hit)
{
    if (!light.enabled)
    {
        return vec3(0.0);
    }

    vec3 N = normalize(hit.normal);
    vec3 toLight = light.position - hit.position;

    float distanceToLight = length(toLight);
    vec3 L = normalize(toLight);

    float ndotl = max(dot(N, L), 0.0);

    if (ndotl <= 0.0 || IsOccluded(hit.position, N, light.position))
    {
        return vec3(0.0);
    }

    float attenuation = 1.0 /
    (
        1.0 +
        0.08 * distanceToLight +
        0.02 * distanceToLight * distanceToLight
    );

    vec3 radiance = light.color.rgb * light.intensity * attenuation;

    return hit.albedo * radiance * ndotl;
}

vec3 ShadeDirect(Hit hit)
{
    if (hit.materialType == MATERIAL_LIGHT)
    {
        return hit.albedo * hit.emission;
    }

    vec3 color = vec3(0.0);

    color += ShadeLight(mainLight, hit);
    color += ShadeLight(fillLight, hit);

    // Rebote ambiental falso, bajo y cálido.
    color += hit.albedo * vec3(0.012, 0.009, 0.006);

    return color;
}

vec3 SampleFirstHitColor(Ray ray)
{
    Hit hit = TraceScene(ray);

    if (!hit.hit)
    {
        return vec3(0.0);
    }

    hit.normal = FaceNormalAgainstRay(hit.normal, ray);

    if (hit.materialType == MATERIAL_LIGHT)
    {
        return hit.albedo * hit.emission;
    }

    return ShadeDirect(hit);
}

// ==================================================
// Specular helpers
// ==================================================

vec3 ComputeSpecular(
    Hit hit,
    Ray ray,
    float mainPower,
    float fillPower,
    float mainStrength,
    float fillStrength
)
{
    vec3 result = vec3(0.0);

    vec3 N = normalize(hit.normal);
    vec3 V = normalize(-ray.direction);

    if (mainLight.enabled)
    {
        vec3 L = normalize(mainLight.position - hit.position);
        vec3 H = normalize(L + V);

        float spec = pow(max(dot(N, H), 0.0), mainPower);

        result +=
            mainLight.color.rgb *
            mainLight.intensity *
            spec *
            mainStrength;
    }

    if (fillLight.enabled)
    {
        vec3 L = normalize(fillLight.position - hit.position);
        vec3 H = normalize(L + V);

        float spec = pow(max(dot(N, H), 0.0), fillPower);

        result +=
            fillLight.color.rgb *
            fillLight.intensity *
            spec *
            fillStrength;
    }

    return result;
}

// ==================================================
// Materials
// ==================================================

vec3 TraceMetal(
    Ray ray,
    Hit hit
)
{
    // Acero satinado: base difusa + brillo fuerte + reflejo leve.
    vec3 base = ShadeDirect(hit) * 0.55;

    vec3 specular = ComputeSpecular(
        hit,
        ray,
        96.0,
        48.0,
        0.20,
        0.05
    );

    vec3 reflectedDirection = normalize(reflect(ray.direction, hit.normal));

    Ray reflectionRay;
    reflectionRay.origin = hit.position + hit.normal * EPSILON * 2.0;
    reflectionRay.direction = reflectedDirection;

    vec3 reflectionColor = SampleFirstHitColor(reflectionRay);

    // Reflejo muy reducido para evitar aspecto de espejo.
    vec3 softReflection = reflectionColor * 0.08;

    return base + specular + softReflection;
}

vec3 TraceGlass(
    inout Ray ray,
    Hit hit,
    inout vec3 throughput
)
{
    vec3 N = normalize(hit.normal);

    bool frontFace = dot(ray.direction, N) < 0.0;
    vec3 normal = frontFace ? N : -N;

    float eta = frontFace ? (1.0 / hit.ior) : hit.ior;

    float cosTheta = min(dot(-ray.direction, normal), 1.0);
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));

    bool cannotRefract = eta * sinTheta > 1.0;

    vec3 reflectedDirection = normalize(reflect(ray.direction, normal));
    vec3 refractedDirection = normalize(refract(ray.direction, normal, eta));

    float fresnel = SchlickFresnel(cosTheta, hit.ior);
    fresnel = clamp(fresnel * 1.8, 0.04, 0.85);

    Ray reflectionRay;
    reflectionRay.origin = hit.position + normal * EPSILON * 2.0;
    reflectionRay.direction = reflectedDirection;

    vec3 reflectionColor = SampleFirstHitColor(reflectionRay);

    vec3 highlight = ComputeSpecular(
        hit,
        ray,
        180.0,
        90.0,
        0.10,
        0.035
    );

    // Vidrio claro con borde Fresnel y reflejo superficial.
    vec3 surfaceGlass =
        reflectionColor * fresnel * 0.65 +
        highlight +
        vec3(0.70, 0.85, 1.0) * fresnel * 0.035;

    vec3 nextDirection =
        cannotRefract
        ? reflectedDirection
        : refractedDirection;

    ray.origin = hit.position + nextDirection * EPSILON * 2.0;
    ray.direction = nextDirection;

    // Conserva energía para que la esfera no se vea gris/opaca.
    throughput *= mix(hit.albedo, vec3(1.0), 0.985);

    return surfaceGlass;
}

// ==================================================
// Ray loop
// ==================================================

vec3 TraceRay(Ray ray)
{
    vec3 finalColor = vec3(0.0);
    vec3 throughput = vec3(1.0);

    for (int bounce = 0; bounce < MAX_BOUNCES; ++bounce)
    {
        Hit hit = TraceScene(ray);

        if (!hit.hit)
        {
            break;
        }

        hit.normal = FaceNormalAgainstRay(hit.normal, ray);

        if (hit.materialType == MATERIAL_LIGHT)
        {
            finalColor += throughput * ShadeDirect(hit);
            break;
        }

        bool useMetal =
            hit.materialType == MATERIAL_METAL &&
            metalShaderEnabled;

        bool useGlass =
            hit.materialType == MATERIAL_GLASS &&
            glassShaderEnabled;

        if (useMetal)
        {
            finalColor += throughput * TraceMetal(ray, hit);
            break;
        }

        if (useGlass)
        {
            finalColor += throughput * TraceGlass(ray, hit, throughput);
            continue;
        }

        finalColor += throughput * ShadeDirect(hit);
        break;
    }

    return finalColor;
}

// ==================================================
// Post process
// ==================================================

vec3 ToneMap(vec3 color)
{
    return color / (color + vec3(1.0));
}

vec3 GammaCorrect(vec3 color)
{
    return pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));
}

// ==================================================
// Main
// ==================================================

void main()
{
    Ray ray = GenerateCameraRay(uv);

    vec3 color = TraceRay(ray);

    color = ToneMap(color);
    color = GammaCorrect(color);

    FragColor = vec4(color, 1.0);
}