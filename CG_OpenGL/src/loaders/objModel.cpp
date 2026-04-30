#include "ObjModel.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <limits>
#include <memory>
#include <algorithm>

struct ObjIndex
{
    int position = -1;
    int uv = -1;
    int normal = -1;

    bool operator==(const ObjIndex& other) const
    {
        return position == other.position &&
            uv == other.uv &&
            normal == other.normal;
    }
};

struct ObjIndexHash
{
    std::size_t operator()(const ObjIndex& index) const
    {
        std::size_t h1 = std::hash<int>()(index.position);
        std::size_t h2 = std::hash<int>()(index.uv);
        std::size_t h3 = std::hash<int>()(index.normal);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

static int ResolveObjIndex(int objIndex, int count)
{
    if (objIndex > 0)
        return objIndex - 1;

    if (objIndex < 0)
        return count + objIndex;

    return -1;
}

static ObjIndex ParseObjIndex(const std::string& text, int positionCount, int uvCount, int normalCount)
{
    ObjIndex result;

    size_t firstSlash = text.find('/');

    if (firstSlash == std::string::npos)
    {
        int p = std::stoi(text);
        result.position = ResolveObjIndex(p, positionCount);
        return result;
    }

    size_t secondSlash = text.find('/', firstSlash + 1);

    int p = std::stoi(text.substr(0, firstSlash));
    result.position = ResolveObjIndex(p, positionCount);

    if (secondSlash == std::string::npos)
    {
        std::string uvText = text.substr(firstSlash + 1);

        if (!uvText.empty())
        {
            int uv = std::stoi(uvText);
            result.uv = ResolveObjIndex(uv, uvCount);
        }

        return result;
    }

    std::string uvText = text.substr(firstSlash + 1, secondSlash - firstSlash - 1);
    std::string normalText = text.substr(secondSlash + 1);

    if (!uvText.empty())
    {
        int uv = std::stoi(uvText);
        result.uv = ResolveObjIndex(uv, uvCount);
    }

    if (!normalText.empty())
    {
        int n = std::stoi(normalText);
        result.normal = ResolveObjIndex(n, normalCount);
    }

    return result;
}

ObjModel::ObjModel(const std::string& path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cout << "ERROR: No se pudo abrir el OBJ: " << path << std::endl;
        return;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    positions.reserve(100000);
    normals.reserve(100000);
    texCoords.reserve(100000);
    vertices.reserve(100000);
    indices.reserve(300000);

    std::unordered_map<ObjIndex, GLuint, ObjIndexHash> uniqueVertices;
    uniqueVertices.reserve(100000);

    glm::vec3 minBounds(std::numeric_limits<float>::max());
    glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v")
        {
            glm::vec3 position;
            ss >> position.x >> position.y >> position.z;

            positions.push_back(position);

            minBounds = glm::min(minBounds, position);
            maxBounds = glm::max(maxBounds, position);
        }
        else if (prefix == "vt")
        {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            texCoords.push_back(uv);
        }
        else if (prefix == "vn")
        {
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;

            if (glm::length(normal) > 0.0f)
                normal = glm::normalize(normal);

            normals.push_back(normal);
        }
        else if (prefix == "f")
        {
            std::vector<ObjIndex> face;
            face.reserve(4);

            std::string vertexText;

            while (ss >> vertexText)
            {
                face.push_back(ParseObjIndex(
                    vertexText,
                    static_cast<int>(positions.size()),
                    static_cast<int>(texCoords.size()),
                    static_cast<int>(normals.size())
                ));
            }

            if (face.size() < 3)
                continue;

            for (size_t i = 1; i < face.size() - 1; i++)
            {
                ObjIndex triangle[3] =
                {
                    face[0],
                    face[i],
                    face[i + 1]
                };

                for (const ObjIndex& objIndex : triangle)
                {
                    auto it = uniqueVertices.find(objIndex);

                    if (it == uniqueVertices.end())
                    {
                        Vertex vertex{};

                        if (objIndex.position >= 0 &&
                            objIndex.position < static_cast<int>(positions.size()))
                        {
                            vertex.position = positions[objIndex.position];
                        }

                        if (objIndex.uv >= 0 &&
                            objIndex.uv < static_cast<int>(texCoords.size()))
                        {
                            vertex.texUV = texCoords[objIndex.uv];
                        }
                        else
                        {
                            vertex.texUV = glm::vec2(0.0f);
                        }

                        if (objIndex.normal >= 0 &&
                            objIndex.normal < static_cast<int>(normals.size()))
                        {
                            vertex.normal = normals[objIndex.normal];
                        }
                        else
                        {
                            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                        }

                        vertex.color = glm::vec3(1.0f);

                        vertices.push_back(vertex);

                        GLuint newIndex = static_cast<GLuint>(vertices.size() - 1);
                        uniqueVertices.emplace(objIndex, newIndex);
                        indices.push_back(newIndex);
                    }
                    else
                    {
                        indices.push_back(it->second);
                    }
                }
            }
        }
    }

    if (!positions.empty())
    {
        glm::vec3 size = maxBounds - minBounds;
        glm::vec3 center = (minBounds + maxBounds) * 0.5f;

        float largestAxis = std::max(size.x, std::max(size.y, size.z));
        float scale = 1.0f;

        if (largestAxis > 0.0f)
            scale = 1.8f / largestAxis;

        normalizationMatrix = glm::mat4(1.0f);
        normalizationMatrix = glm::scale(normalizationMatrix, glm::vec3(scale));
        normalizationMatrix = glm::translate(normalizationMatrix, -center);

        std::cout << "Centro OBJ: "
            << center.x << ", "
            << center.y << ", "
            << center.z << std::endl;

        std::cout << "Escala normalizada: " << scale << std::endl;
    }

    mesh = std::make_unique<Mesh>(vertices, indices, textures);

    std::cout << "OBJ cargado correctamente: " << path << std::endl;
    std::cout << "Vertices originales: " << positions.size() << std::endl;
    std::cout << "Vertices unicos: " << vertices.size() << std::endl;
    std::cout << "Indices: " << indices.size() << std::endl;
}

void ObjModel::Draw(Shader& shader, Camera& camera, glm::mat4 parentTransform)
{
    if (mesh)
        mesh->Draw(shader, camera, parentTransform * normalizationMatrix);
}