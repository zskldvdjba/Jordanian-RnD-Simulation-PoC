#pragma once

#include "VisualAssetTypes.hpp"
#include <unordered_map>

namespace sim::vis {

class AssetManager {
public:
    AssetManager();

    void initialize();

    [[nodiscard]] const AssetMesh& getMesh(AssetType type, float scale = 1.0f) const;
    [[nodiscard]] const AssetMesh& getEmitterMesh() const;

    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

private:
    bool m_initialized{false};
    std::unordered_map<int, AssetMesh> m_meshCache;

    void generateMeshes();
};

} // namespace sim::vis
