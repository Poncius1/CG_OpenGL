#include "trajectoryLoader.h"

#include <fstream>
#include <iostream>

namespace
{
    glm::vec3 ReadVec3(const nlohmann::json& value)
    {
        if (!value.is_array() || value.size() < 3)
        {
            return glm::vec3(0.0f);
        }

        return glm::vec3(
            value.at(0).get<float>(),
            value.at(1).get<float>(),
            value.at(2).get<float>()
        );
    }

    glm::vec2 ReadReward2(const nlohmann::json& value)
    {
        if (!value.is_array() || value.size() < 2)
        {
            return glm::vec2(0.0f);
        }

        return glm::vec2(
            value.at(0).get<float>(),
            value.at(1).get<float>()
        );
    }

    TrajectoryTerminalEvent ParseTerminalEvent(const std::string& value)
    {
        if (value == "hit_light" || value == "light" || value == "success_hit_light")
            return TrajectoryTerminalEvent::HitLight;

        if (value == "hit_obstacle" || value == "obstacle" || value == "failure_hit_obstacle")
            return TrajectoryTerminalEvent::HitObstacle;

        if (value == "out_of_bounds" || value == "failure_out_of_bounds")
            return TrajectoryTerminalEvent::OutOfBounds;

        if (value == "max_steps" || value == "failure_max_steps")
            return TrajectoryTerminalEvent::MaxSteps;

        if (value.rfind("failure", 0) == 0)
            return TrajectoryTerminalEvent::Failed;

        return TrajectoryTerminalEvent::Unknown;
    }

    const char* MethodName(TrajectoryMethod method)
    {
        switch (method)
        {
        case TrajectoryMethod::Random:
            return "random";
        case TrajectoryMethod::LightBiased:
            return "light_biased";
        case TrajectoryMethod::ParetoQLearning:
            return "pql";
        }

        return "unknown";
    }
}

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
    if (!root.contains(arrayName) || !root.at(arrayName).is_array())
    {
        return;
    }

    const auto& jsonPaths = root.at(arrayName);
    outputPaths.reserve(jsonPaths.size());

    for (const auto& jsonPath : jsonPaths)
    {
        TrajectoryPath path;
        path.method = method;
        path.methodName = MethodName(method);

        if (jsonPath.contains("id"))
            path.id = jsonPath.at("id").get<int>();

        if (jsonPath.contains("method") && jsonPath.at("method").is_string())
            path.methodName = jsonPath.at("method").get<std::string>();

        if (jsonPath.contains("visual_class") && jsonPath.at("visual_class").is_string())
            path.visualClass = jsonPath.at("visual_class").get<std::string>();

        if (jsonPath.contains("terminal_event") && jsonPath.at("terminal_event").is_string())
        {
            path.terminalEventName = jsonPath.at("terminal_event").get<std::string>();
            path.terminalEvent = ParseTerminalEvent(path.terminalEventName);
        }

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
            path.reward = ReadReward2(jsonPath.at("reward"));
            path.totalQuality = path.reward.x;
            path.totalCost = -path.reward.y;
        }

        if (jsonPath.contains("total_quality"))
            path.totalQuality = jsonPath.at("total_quality").get<float>();

        if (jsonPath.contains("total_cost"))
            path.totalCost = jsonPath.at("total_cost").get<float>();

        if (jsonPath.contains("points") && jsonPath.at("points").is_array())
        {
            const auto& points = jsonPath.at("points");
            path.points.reserve(points.size());

            for (const auto& point : points)
            {
                if (point.is_array() && point.size() >= 3)
                {
                    path.points.push_back(ReadVec3(point));
                }
            }
        }

        if (jsonPath.contains("segments") && jsonPath.at("segments").is_array())
        {
            const auto& segments = jsonPath.at("segments");
            path.segments.reserve(segments.size());

            for (const auto& jsonSegment : segments)
            {
                TrajectorySegment segment;

                if (jsonSegment.contains("from"))
                    segment.from = ReadVec3(jsonSegment.at("from"));

                if (jsonSegment.contains("to"))
                    segment.to = ReadVec3(jsonSegment.at("to"));

                if (jsonSegment.contains("event") && jsonSegment.at("event").is_string())
                    segment.event = jsonSegment.at("event").get<std::string>();

                if (jsonSegment.contains("bounce"))
                    segment.bounce = jsonSegment.at("bounce").get<int>();

                if (jsonSegment.contains("action"))
                    segment.action = jsonSegment.at("action").get<int>();

                if (jsonSegment.contains("distance"))
                    segment.distance = jsonSegment.at("distance").get<float>();

                if (jsonSegment.contains("reward") && jsonSegment.at("reward").is_array())
                    segment.reward = ReadReward2(jsonSegment.at("reward"));

                path.segments.push_back(segment);
            }

            // New JSON can be visualized even if the legacy points array is absent.
            if (path.points.empty() && !path.segments.empty())
            {
                path.points.push_back(path.segments.front().from);

                for (const TrajectorySegment& segment : path.segments)
                {
                    path.points.push_back(segment.to);
                }
            }
        }

        if (path.steps <= 0)
        {
            if (!path.segments.empty())
                path.steps = static_cast<int>(path.segments.size());
            else if (path.points.size() >= 2)
                path.steps = static_cast<int>(path.points.size() - 1);
        }

        if (path.terminalEvent == TrajectoryTerminalEvent::Unknown)
        {
            if (path.hitLight)
                path.terminalEvent = TrajectoryTerminalEvent::HitLight;
            else if (path.hitObstacle)
                path.terminalEvent = TrajectoryTerminalEvent::HitObstacle;
            else if (path.outOfBounds)
                path.terminalEvent = TrajectoryTerminalEvent::OutOfBounds;
            else
                path.terminalEvent = TrajectoryTerminalEvent::Failed;
        }

        if (path.points.size() >= 2 || !path.segments.empty())
        {
            outputPaths.push_back(path);
        }
    }
}
