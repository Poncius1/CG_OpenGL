#pragma once

#include <string>
#include <vector>

#include "json.hpp"
#include "trajectoryPath.h"

class TrajectoryLoader
{
public:
    static bool LoadFromJson(
        const std::string& filePath,
        std::vector<TrajectoryPath>& randomPaths,
        std::vector<TrajectoryPath>& lightBiasedPaths,
        std::vector<TrajectoryPath>& pqlPaths
    );

private:
    static void LoadPathArray(
        const nlohmann::json& root,
        const std::string& arrayName,
        TrajectoryMethod method,
        std::vector<TrajectoryPath>& outputPaths
    );
};
