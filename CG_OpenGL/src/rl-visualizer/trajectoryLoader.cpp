#include "rl-visualizer/trajectoryLoader.h"

#include <fstream>
#include <iostream>

namespace
{
    bool IsNumber(const nlohmann::json& value)
    {
        return value.is_number_float() || value.is_number_integer() || value.is_number_unsigned();
    }

    float ReadFloatOrDefault(const nlohmann::json& object, const std::string& key, float fallback = 0.0f)
    {
        if (!object.contains(key))
            return fallback;

        const auto& value = object.at(key);
        if (value.is_null() || !IsNumber(value))
            return fallback;

        return value.get<float>();
    }

    int ReadIntOrDefault(const nlohmann::json& object, const std::string& key, int fallback = 0)
    {
        if (!object.contains(key))
            return fallback;

        const auto& value = object.at(key);
        if (value.is_null() || !IsNumber(value))
            return fallback;

        return value.get<int>();
    }

    bool ReadBoolOrDefault(const nlohmann::json& object, const std::string& key, bool fallback = false)
    {
        if (!object.contains(key))
            return fallback;

        const auto& value = object.at(key);
        if (value.is_null() || !value.is_boolean())
            return fallback;

        return value.get<bool>();
    }

    std::string ReadStringOrDefault(
        const nlohmann::json& object,
        const std::string& key,
        const std::string& fallback = "unknown"
    )
    {
        if (!object.contains(key))
            return fallback;

        const auto& value = object.at(key);
        if (value.is_null() || !value.is_string())
            return fallback;

        return value.get<std::string>();
    }

    float ReadArrayFloatOrDefault(const nlohmann::json& value, size_t index, float fallback = 0.0f)
    {
        if (!value.is_array() || value.size() <= index)
            return fallback;

        const auto& element = value.at(index);
        if (element.is_null() || !IsNumber(element))
            return fallback;

        return element.get<float>();
    }

    glm::vec3 ReadVec3(const nlohmann::json& value)
    {
        return glm::vec3(
            ReadArrayFloatOrDefault(value, 0, 0.0f),
            ReadArrayFloatOrDefault(value, 1, 0.0f),
            ReadArrayFloatOrDefault(value, 2, 0.0f)
        );
    }

    glm::vec2 ReadReward2(const nlohmann::json& value)
    {
        return glm::vec2(
            ReadArrayFloatOrDefault(value, 0, 0.0f),
            ReadArrayFloatOrDefault(value, 1, 0.0f)
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
        return;

    const auto& jsonPaths = root.at(arrayName);
    outputPaths.reserve(jsonPaths.size());

    for (const auto& jsonPath : jsonPaths)
    {
        if (!jsonPath.is_object())
            continue;

        TrajectoryPath path;
        path.method = method;
        path.methodName = ReadStringOrDefault(jsonPath, "method", MethodName(method));

        path.id = ReadIntOrDefault(jsonPath, "id", 0);
        path.visualClass = ReadStringOrDefault(jsonPath, "visual_class", "unknown");
        path.terminalEventName = ReadStringOrDefault(jsonPath, "terminal_event", "unknown");
        path.terminalEvent = ParseTerminalEvent(path.terminalEventName);

        path.hitLight = ReadBoolOrDefault(jsonPath, "hit_light", false);
        path.hitObstacle = ReadBoolOrDefault(jsonPath, "hit_obstacle", false);
        path.outOfBounds = ReadBoolOrDefault(jsonPath, "out_of_bounds", false);
        path.steps = ReadIntOrDefault(jsonPath, "steps", 0);

        if (jsonPath.contains("reward") && jsonPath.at("reward").is_array())
        {
            path.reward = ReadReward2(jsonPath.at("reward"));
            path.totalQuality = path.reward.x;
            path.totalCost = -path.reward.y;
        }

        path.totalQuality = ReadFloatOrDefault(jsonPath, "total_quality", path.totalQuality);
        path.totalCost = ReadFloatOrDefault(jsonPath, "total_cost", path.totalCost);

        if (jsonPath.contains("points") && jsonPath.at("points").is_array())
        {
            const auto& points = jsonPath.at("points");
            path.points.reserve(points.size());

            for (const auto& point : points)
            {
                if (!point.is_array() || point.size() < 3)
                    continue;

                path.points.push_back(ReadVec3(point));
            }
        }

        if (jsonPath.contains("segments") && jsonPath.at("segments").is_array())
        {
            const auto& segments = jsonPath.at("segments");
            path.segments.reserve(segments.size());

            for (const auto& jsonSegment : segments)
            {
                if (!jsonSegment.is_object())
                    continue;

                TrajectorySegment segment;

                if (jsonSegment.contains("from") && jsonSegment.at("from").is_array())
                    segment.from = ReadVec3(jsonSegment.at("from"));

                if (jsonSegment.contains("to") && jsonSegment.at("to").is_array())
                    segment.to = ReadVec3(jsonSegment.at("to"));

                segment.event = ReadStringOrDefault(jsonSegment, "event", "unknown");
                segment.bounce = ReadIntOrDefault(jsonSegment, "bounce", 0);
                segment.action = ReadIntOrDefault(jsonSegment, "action", -1);
                segment.distance = ReadFloatOrDefault(jsonSegment, "distance", 0.0f);

                if (jsonSegment.contains("reward") && jsonSegment.at("reward").is_array())
                    segment.reward = ReadReward2(jsonSegment.at("reward"));

                path.segments.push_back(segment);
            }

            if (path.points.empty() && !path.segments.empty())
            {
                path.points.push_back(path.segments.front().from);
                for (const TrajectorySegment& segment : path.segments)
                    path.points.push_back(segment.to);
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
            {
                path.terminalEvent = TrajectoryTerminalEvent::HitLight;
                path.terminalEventName = "hit_light";
            }
            else if (path.hitObstacle)
            {
                path.terminalEvent = TrajectoryTerminalEvent::HitObstacle;
                path.terminalEventName = "hit_obstacle";
            }
            else if (path.outOfBounds)
            {
                path.terminalEvent = TrajectoryTerminalEvent::OutOfBounds;
                path.terminalEventName = "out_of_bounds";
            }
            else
            {
                path.terminalEvent = TrajectoryTerminalEvent::Failed;
                path.terminalEventName = "failed";
            }
        }

        if (path.points.size() >= 2 || !path.segments.empty())
            outputPaths.push_back(path);
    }
}
