
#include "trajectoryLoader.h"
#include <fstream>
#include <iostream>
#include "json.hpp"

bool TrajectoryLoader::LoadFromJson(
    const std::string& filePath,
    std::vector<TrajectoryPath>& randomPaths,
    std::vector<TrajectoryPath>& lightBiasedPaths,
    std::vector<TrajectoryPath>& pqlPaths
)
{
    randomPaths.clear();
    lightBiasedPaths.clear();
    pqlPaths.clear();

    std::ifstream file(filePath);

    if (!file.is_open())
    {
        std::cout << "ERROR: No se pudo abrir JSON de trayectorias: "
            << filePath << std::endl;
        return false;
    }

    nlohmann::json root;

    try
    {
        file >> root;
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: JSON invalido: " << e.what() << std::endl;
        return false;
    }

    try
    {
        LoadPathArray(root, "random_paths", TrajectoryMethod::Random, randomPaths);
        LoadPathArray(root, "light_biased_paths", TrajectoryMethod::LightBiased, lightBiasedPaths);
        LoadPathArray(root, "pql_paths", TrajectoryMethod::ParetoQLearning, pqlPaths);
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: No se pudieron parsear trayectorias: "
            << e.what() << std::endl;
        return false;
    }

    std::cout << "JSON de trayectorias cargado correctamente." << std::endl;
    std::cout << "Random paths: " << randomPaths.size() << std::endl;
    std::cout << "Light-biased paths: " << lightBiasedPaths.size() << std::endl;
    std::cout << "PQL paths: " << pqlPaths.size() << std::endl;

    return true;
}

void TrajectoryLoader::LoadPathArray(
    const nlohmann::json& root,
    const std::string& arrayName,
    TrajectoryMethod method,
    std::vector<TrajectoryPath>& outputPaths
)
{
    if (!root.contains(arrayName))
    {
        return;
    }

    const auto& jsonPaths = root.at(arrayName);

    if (!jsonPaths.is_array())
    {
        return;
    }

    outputPaths.reserve(jsonPaths.size());

    for (const auto& jsonPath : jsonPaths)
    {
        TrajectoryPath path;
        path.method = method;

        if (jsonPath.contains("id"))
            path.id = jsonPath.at("id").get<int>();

        if (jsonPath.contains("hit_light"))
            path.hitLight = jsonPath.at("hit_light").get<bool>();

        if (jsonPath.contains("hit_obstacle"))
            path.hitObstacle = jsonPath.at("hit_obstacle").get<bool>();

        if (jsonPath.contains("out_of_bounds"))
            path.outOfBounds = jsonPath.at("out_of_bounds").get<bool>();

        if (jsonPath.contains("steps"))
            path.steps = jsonPath.at("steps").get<int>();

        if (jsonPath.contains("reward") && jsonPath.at("reward").is_array())
        {
            const auto& reward = jsonPath.at("reward");

            if (reward.size() >= 2)
            {
                path.reward.x = reward.at(0).get<float>();
                path.reward.y = reward.at(1).get<float>();
            }
        }

        if (jsonPath.contains("points") && jsonPath.at("points").is_array())
        {
            const auto& points = jsonPath.at("points");

            path.points.reserve(points.size());

            for (const auto& point : points)
            {
                if (!point.is_array() || point.size() < 3)
                    continue;

                glm::vec3 p(
                    point.at(0).get<float>(),
                    point.at(1).get<float>(),
                    point.at(2).get<float>()
                );

                path.points.push_back(p);
            }
        }

        if (path.points.size() >= 2)
        {
            outputPaths.push_back(path);
        }
    }
}