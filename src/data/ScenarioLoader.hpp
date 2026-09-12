#pragma once

#include "core/Types.hpp"
#include <string>
#include <optional>

namespace sim {

class ScenarioLoader {
public:
    static std::optional<ScenarioConfig> loadFromFile(const std::string& filepath);
    static std::optional<ScenarioConfig> parseJson(const std::string& json_content);
};

} // namespace sim
