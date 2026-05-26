#version 430 core

out vec4 FragColor;

in vec2 uv;

const float INF = 100000.0;
const float EPSILON = 0.001;
const int MAX_BOUNCES = 3;

const int MATERIAL_DIFFUSE = 0;
const int MATERIAL_METAL = 1;
const int MATERIAL_GLASS = 2;
const int MATERIAL_LIGHT = 3;

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

    // x = materialType
    // y = roughness
    // z = ior
    // w = emission
    vec4 materialData;
};

layout(std430, binding = 0) readonly buffer TriangleBuffer
{
    GpuTriangle objTriangles[];
};

uniform int objTriangleCount;
uniform bool useObjTriangles;
uniform bool useAnalyticScene;

uniform vec3 cameraPosition;
uniform vec3 cameraForward;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float cameraFov;
uniform float aspectRatio;

uniform vec3 boxMin;
uniform vec3 boxMax;

uniform vec3 metalBoxMin;
uniform vec3 metalBoxMax;
uniform vec3 metalBoxAlbedo;
uniform float metalBoxRoughness;

uniform vec3 glassSpherePosition;
uniform float glassSphereRadius;
uniform vec3 glassSphereAlbedo;
uniform float glassSphereIOR;

uniform bool metalShaderEnabled;
uniform bool glassShaderEnabled;

uniform Light mainLight;
uniform Light fillLight;

Hit NoHit()
{
    Hit hit;

    hit.hit = false;
    hit.t = INF;
    hit.position = vec3(0.0);
    hit.normal = vec3(0.0, 1.0, 0.0);
    hit.albedo = vec3(0.0);
    hit.materialType = MATERIAL_DIFFUSE;
    hit.roughness = 0.0;
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

bool IsInsideBounds(vec3 point, vec3 minBounds, vec3 maxBounds)
{
    return
        point.x >= minBounds.x - EPSILON &&
        point.x <= maxBounds.x + EPSILON &&
        point.y >= minBounds.y - EPSILON &&
        point.y <= maxBounds.y + EPSILON &&
        point.z >= minBounds.z - EPSILON &&
        point.z <= maxBounds.z + EPSILON;
}

Hit IntersectPlane(
    Ray ray,
    vec3 pointOnPlane,
    vec3 planeNormal,
    vec3 minBounds,
    vec3 maxBounds,
    vec3 albedo
)
{
    float denominator = dot(ray.direction, planeNormal);

    if (abs(denominator) < 0.00001)
    {
        return NoHit();
    }

    float t = dot(pointOnPlane - ray.origin, planeNormal) / denominator;

    if (t < EPSILON)
    {
        return NoHit();
    }

    vec3 position = ray.origin + ray.direction * t;

    if (!IsInsideBounds(position, minBounds, maxBounds))
    {
        return NoHit();
    }

    return CreateHit(
        t,
        ray,
        planeNormal,
        albedo,
        MATERIAL_DIFFUSE,
        1.0,
        1.0,
        0.0
    );
}

Hit IntersectRectangle(
    Ray ray,
    vec3 pointOnPlane,
    vec3 planeNormal,
    vec3 minBounds,
    vec3 maxBounds,
    vec3 albedo,
    float emission
)
{
    Hit hit = IntersectPlane(
        ray,
        pointOnPlane,
        planeNormal,
        minBounds,
        maxBounds,
        albedo
    );

    if (!hit.hit)
    {
        return hit;
    }

    hit.materialType = MATERIAL_LIGHT;
    hit.emission = emission;

    return hit;
}

Hit IntersectSphere(
    Ray ray,
    vec3 center,
    float radius,
    vec3 albedo,
    int materialType,
    float roughness,
    float ior
)
{
    vec3 oc = ray.origin - center;

    float a = dot(ray.direction, ray.direction);
    float halfB = dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;

    float discriminant = halfB * halfB - a * c;

    if (discriminant < 0.0)
    {
        return NoHit();
    }

    float sqrtD = sqrt(discriminant);

    float t = (-halfB - sqrtD) / a;

    if (t < EPSILON)
    {
        t = (-halfB + sqrtD) / a;
    }

    if (t < EPSILON)
    {
        return NoHit();
    }

    vec3 position = ray.origin + ray.direction * t;
    vec3 normal = normalize(position - center);

    return CreateHit(
        t,
        ray,
        normal,
        albedo,
        materialType,
        roughness,
        ior,
        0.0
    );
}

vec3 GetBoxNormal(vec3 position, vec3 minBounds, vec3 maxBounds)
{
    vec3 distanceToMin = abs(position - minBounds);
    vec3 distanceToMax = abs(position - maxBounds);

    float closest = distanceToMin.x;
    vec3 normal = vec3(-1.0, 0.0, 0.0);

    if (distanceToMax.x < closest)
    {
        closest = distanceToMax.x;
        normal = vec3(1.0, 0.0, 0.0);
    }

    if (distanceToMin.y < closest)
    {
        closest = distanceToMin.y;
        normal = vec3(0.0, -1.0, 0.0);
    }

    if (distanceToMax.y < closest)
    {
        closest = distanceToMax.y;
        normal = vec3(0.0, 1.0, 0.0);
    }

    if (distanceToMin.z < closest)
    {
        closest = distanceToMin.z;
        normal = vec3(0.0, 0.0, -1.0);
    }

    if (distanceToMax.z < closest)
    {
        normal = vec3(0.0, 0.0, 1.0);
    }

    return normal;
}

Hit IntersectBox(
    Ray ray,
    vec3 minBounds,
    vec3 maxBounds,
    vec3 albedo,
    int materialType,
    float roughness
)
{
    vec3 safeDirection = ray.direction;

    if (abs(safeDirection.x) < 0.00001) safeDirection.x = 0.00001;
    if (abs(safeDirection.y) < 0.00001) safeDirection.y = 0.00001;
    if (abs(safeDirection.z) < 0.00001) safeDirection.z = 0.00001;

    vec3 invDir = 1.0 / safeDirection;

    vec3 t0 = (minBounds - ray.origin) * invDir;
    vec3 t1 = (maxBounds - ray.origin) * invDir;

    vec3 tMin = min(t0, t1);
    vec3 tMax = max(t0, t1);

    float tNear = max(max(tMin.x, tMin.y), tMin.z);
    float tFar = min(min(tMax.x, tMax.y), tMax.z);

    if (tNear > tFar || tFar < EPSILON)
    {
        return NoHit();
    }

    float t = tNear > EPSILON ? tNear : tFar;

    vec3 position = ray.origin + ray.direction * t;
    vec3 normal = GetBoxNormal(position, minBounds, maxBounds);

    return CreateHit(
        t,
        ray,
        normal,
        albedo,
        materialType,
        roughness,
        1.0,
        0.0
    );
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

    vec3 n0 = triangle.n0.xyz;
    vec3 n1 = triangle.n1.xyz;
    vec3 n2 = triangle.n2.xyz;

    vec3 normal = normalize(n0 * (1.0 - u - v) + n1 * u + n2 * v);

    int materialType = int(round(triangle.materialData.x));
    float roughness = triangle.materialData.y;
    float ior = triangle.materialData.z;
    float emission = triangle.materialData.w;

    return CreateHit(
        t,
        ray,
        normal,
        triangle.albedo.rgb,
        materialType,
        roughness,
        ior,
        emission
    );
}

Hit IntersectObjTriangles(Ray ray)
{
    Hit closestHit = NoHit();

    if (!useObjTriangles)
    {
        return closestHit;
    }

    for (int i = 0; i < objTriangleCount; ++i)
    {
        KeepClosest(
            closestHit,
            IntersectTriangle(ray, objTriangles[i])
        );
    }

    return closestHit;
}

Hit IntersectCornellBox(Ray ray)
{
    Hit closestHit = NoHit();

    vec3 whiteWall = vec3(0.78, 0.76, 0.70);
    vec3 redWall = vec3(0.65, 0.05, 0.04);
    vec3 greenWall = vec3(0.08, 0.45, 0.10);

    KeepClosest(
        closestHit,
        IntersectPlane(
            ray,
            vec3(0.0, boxMin.y, 0.0),
            vec3(0.0, 1.0, 0.0),
            boxMin,
            boxMax,
            whiteWall
        )
    );

    KeepClosest(
        closestHit,
        IntersectPlane(
            ray,
            vec3(0.0, boxMax.y, 0.0),
            vec3(0.0, -1.0, 0.0),
            boxMin,
            boxMax,
            whiteWall
        )
    );

    KeepClosest(
        closestHit,
        IntersectPlane(
            ray,
            vec3(0.0, 0.0, boxMin.z),
            vec3(0.0, 0.0, 1.0),
            boxMin,
            boxMax,
            whiteWall
        )
    );

    KeepClosest(
        closestHit,
        IntersectPlane(
            ray,
            vec3(boxMin.x, 0.0, 0.0),
            vec3(1.0, 0.0, 0.0),
            boxMin,
            boxMax,
            redWall
        )
    );

    KeepClosest(
        closestHit,
        IntersectPlane(
            ray,
            vec3(boxMax.x, 0.0, 0.0),
            vec3(-1.0, 0.0, 0.0),
            boxMin,
            boxMax,
            greenWall
        )
    );

    // Panel visual de luz en el techo.
    KeepClosest(
        closestHit,
        IntersectRectangle(
            ray,
            vec3(0.0, boxMax.y - 0.002, -0.10),
            vec3(0.0, -1.0, 0.0),
            vec3(-0.28, boxMax.y - 0.01, -0.34),
            vec3( 0.28, boxMax.y + 0.01,  0.14),
            vec3(0.78, 0.84, 1.0),
            2.5
        )
    );

    return closestHit;
}

Hit TraceScene(Ray ray)
{
    Hit closestHit = NoHit();

    if (useAnalyticScene)
    {
        KeepClosest(closestHit, IntersectCornellBox(ray));

        KeepClosest(
            closestHit,
            IntersectBox(
                ray,
                metalBoxMin,
                metalBoxMax,
                metalBoxAlbedo,
                MATERIAL_METAL,
                metalBoxRoughness
            )
        );

        KeepClosest(
            closestHit,
            IntersectSphere(
                ray,
                glassSpherePosition,
                glassSphereRadius,
                glassSphereAlbedo,
                MATERIAL_GLASS,
                0.0,
                glassSphereIOR
            )
        );
    }

    KeepClosest(closestHit, IntersectObjTriangles(ray));

    return closestHit;
}

bool IsOccluded(vec3 origin, vec3 normal, vec3 lightPosition)
{
    vec3 toLight = lightPosition - origin;
    float lightDistance = length(toLight);

    Ray shadowRay;
    shadowRay.origin = origin + normal * EPSILON;
    shadowRay.direction = normalize(toLight);

    Hit shadowHit = TraceScene(shadowRay);

    if (!shadowHit.hit || shadowHit.t >= lightDistance)
    {
        return false;
    }

    // La luz visible no debe bloquearse a sí misma.
    if (shadowHit.materialType == MATERIAL_LIGHT)
    {
        return false;
    }

    // El vidrio no debería comportarse como sombra negra sólida.
    // Para esta versión simple, dejamos que la luz lo atraviese.
    if (shadowHit.materialType == MATERIAL_GLASS)
    {
        return false;
    }

    return true;
}

vec3 ShadeLight(Light light, Hit hit)
{
    if (!light.enabled)
    {
        return vec3(0.0);
    }

    vec3 toLight = light.position - hit.position;
    float distanceToLight = length(toLight);
    vec3 lightDir = normalize(toLight);

    float ndotl = max(dot(hit.normal, lightDir), 0.0);

    if (ndotl <= 0.0)
    {
        return vec3(0.0);
    }

    if (IsOccluded(hit.position, hit.normal, light.position))
    {
        return vec3(0.0);
    }

    float attenuation = 1.0 /
    (
        1.0 +
        0.15 * distanceToLight +
        0.05 * distanceToLight * distanceToLight
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

    color += hit.albedo * 0.025;

    return color;
}

float Schlick(float cosine, float ior)
{
    float r0 = (1.0 - ior) / (1.0 + ior);
    r0 = r0 * r0;

    return r0 + (1.0 - r0) * pow(1.0 - cosine, 5.0);
}

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

        bool useMetal = hit.materialType == MATERIAL_METAL && metalShaderEnabled;
        bool useGlass = hit.materialType == MATERIAL_GLASS && glassShaderEnabled;

        if (hit.materialType == MATERIAL_LIGHT)
        {
            finalColor += throughput * ShadeDirect(hit);
            break;
        }

        if (!useMetal && !useGlass)
        {
            finalColor += throughput * ShadeDirect(hit);
            break;
        }

        if (useMetal)
        {
            // Pequeña contribución directa para que el metal no quede negro.
            finalColor += throughput * ShadeDirect(hit) * 0.10;

            vec3 reflectedDirection = reflect(ray.direction, hit.normal);

            ray.origin = hit.position + hit.normal * EPSILON;
            ray.direction = normalize(reflectedDirection);

            // El metal conserva más energía que un difuso normal.
            throughput *= mix(hit.albedo, vec3(1.0), 0.15);

            continue;
        }

        if (useGlass)
        {
            finalColor += throughput * ShadeDirect(hit) * 0.03;

            bool frontFace = dot(ray.direction, hit.normal) < 0.0;
            vec3 normal = frontFace ? hit.normal : -hit.normal;

            float refractionRatio = frontFace ? 1.0 / hit.ior : hit.ior;

            float cosTheta = min(dot(-ray.direction, normal), 1.0);
            float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));

            bool cannotRefract = refractionRatio * sinTheta > 1.0;
            float reflectance = Schlick(cosTheta, hit.ior);

            vec3 nextDirection;

            if (cannotRefract || reflectance > 0.45)
            {
                nextDirection = reflect(ray.direction, normal);
            }
            else
            {
                nextDirection = refract(ray.direction, normal, refractionRatio);
            }

            ray.origin = hit.position + nextDirection * EPSILON;
            ray.direction = normalize(nextDirection);

            throughput *= mix(hit.albedo, vec3(1.0), 0.65);
            continue;
        }
    }

    return finalColor;
}

vec3 ToneMap(vec3 color)
{
    return color / (color + vec3(1.0));
}

void main()
{
    Ray ray = GenerateCameraRay(uv);

    vec3 color = TraceRay(ray);

    color = ToneMap(color);
    color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}