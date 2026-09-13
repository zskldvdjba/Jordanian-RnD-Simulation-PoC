#pragma once

#include "AssetVertex.hpp"
#include <vector>
#include <cstdint>

namespace sim::vis {

enum class AssetType {
    NONE,
    RADAR_NODE,
    CONTROL_CENTER,
    LAUNCHER_PLATFORM,
    EMITTER_POINT,
    PROJECTILE,
    ENVIRONMENT_GROUND
};

inline const char* assetTypeToString(AssetType t) noexcept {
    switch (t) {
        case AssetType::RADAR_NODE:        return "RADAR_NODE";
        case AssetType::CONTROL_CENTER:    return "CONTROL_CENTER";
        case AssetType::LAUNCHER_PLATFORM: return "LAUNCHER_PLATFORM";
        case AssetType::EMITTER_POINT:     return "EMITTER_POINT";
        case AssetType::PROJECTILE:        return "PROJECTILE";
        case AssetType::ENVIRONMENT_GROUND:return "ENVIRONMENT_GROUND";
        default:                           return "NONE";
    }
}

struct AssetMesh {
    AssetType type{AssetType::NONE};
    std::vector<AssetVertex> vertices;
    float boundingRadius{100.0f};

    void reserve(size_t count) { vertices.reserve(count); }
    [[nodiscard]] size_t vertexCount() const noexcept { return vertices.size(); }
    [[nodiscard]] bool empty() const noexcept { return vertices.empty(); }
};

} // namespace sim::vis
