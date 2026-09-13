#include "AssetManager.hpp"
#include "models/RadarNodeMesh.hpp"
#include "models/ControlCenterMesh.hpp"
#include "models/LauncherPlatformMesh.hpp"
#include <cmath>

namespace sim::vis {

AssetManager::AssetManager() = default;

void AssetManager::initialize() {
    if (m_initialized) return;
    generateMeshes();
    m_initialized = true;
}

void AssetManager::generateMeshes() {
    m_meshCache[static_cast<int>(AssetType::RADAR_NODE)] = RadarNodeMesh::generate(1.0f);
    m_meshCache[static_cast<int>(AssetType::CONTROL_CENTER)] = ControlCenterMesh::generate(1.0f);
    m_meshCache[static_cast<int>(AssetType::LAUNCHER_PLATFORM)] = LauncherPlatformMesh::generate(1.0f);

    AssetMesh emitterMesh;
    emitterMesh.type = AssetType::EMITTER_POINT;
    emitterMesh.boundingRadius = 25.0f;
    emitterMesh.reserve(24);

    uint8_t er = 200, eg = 60, eb = 140;
    float er2 = 20.0f;
    for (int i = 0; i < 8; ++i) {
        float a0 = 2.0f * 3.14159265f * i / 8;
        float a1 = 2.0f * 3.14159265f * (i + 1) / 8;
        float x0 = er2 * std::cos(a0), z0 = er2 * std::sin(a0);
        float x1 = er2 * std::cos(a1), z1 = er2 * std::sin(a1);

        emitterMesh.vertices.emplace_back(0, er2, 0, 0, 1, 0, er, eg, eb);
        emitterMesh.vertices.emplace_back(x0, 0, z0, x0 / er2, 0, z0 / er2, er, eg, eb);
        emitterMesh.vertices.emplace_back(x1, 0, z1, x1 / er2, 0, z1 / er2, er, eg, eb);

        emitterMesh.vertices.emplace_back(0, -er2, 0, 0, -1, 0, er, eg, eb);
        emitterMesh.vertices.emplace_back(x1, 0, z1, x1 / er2, 0, z1 / er2, er, eg, eb);
        emitterMesh.vertices.emplace_back(x0, 0, z0, x0 / er2, 0, z0 / er2, er, eg, eb);
    }
    m_meshCache[static_cast<int>(AssetType::EMITTER_POINT)] = std::move(emitterMesh);

    AssetMesh projectileMesh;
    projectileMesh.type = AssetType::PROJECTILE;
    projectileMesh.boundingRadius = 15.0f;
    projectileMesh.reserve(36);

    uint8_t pr = 255, pg = 180, pb = 40;
    float pl = 15.0f, pr2 = 3.0f;
    for (int i = 0; i < 6; ++i) {
        float a0 = 2.0f * 3.14159265f * i / 6;
        float a1 = 2.0f * 3.14159265f * (i + 1) / 6;
        float x0 = pr2 * std::cos(a0), z0 = pr2 * std::sin(a0);
        float x1 = pr2 * std::cos(a1), z1 = pr2 * std::sin(a1);

        projectileMesh.vertices.emplace_back(x0, 0, z0, x0 / pr2, 0, z0 / pr2, pr, pg, pb);
        projectileMesh.vertices.emplace_back(x1, 0, z1, x1 / pr2, 0, z1 / pr2, pr, pg, pb);
        projectileMesh.vertices.emplace_back(0, pl, 0, 0, 1, 0, pr, pg, pb);

        projectileMesh.vertices.emplace_back(x0, 0, z0, x0 / pr2, -0.5f, z0 / pr2, pr, pg, pb);
        projectileMesh.vertices.emplace_back(x1, 0, z1, x1 / pr2, -0.5f, z1 / pr2, pr, pg, pb);
        projectileMesh.vertices.emplace_back(0, -pl * 0.3f, 0, 0, -1, 0, pr, pg, pb);
    }
    m_meshCache[static_cast<int>(AssetType::PROJECTILE)] = std::move(projectileMesh);
}

const AssetMesh& AssetManager::getMesh(AssetType type, float scale) const {
    (void)scale;
    auto it = m_meshCache.find(static_cast<int>(type));
    if (it != m_meshCache.end()) {
        return it->second;
    }
    static const AssetMesh empty;
    return empty;
}

const AssetMesh& AssetManager::getEmitterMesh() const {
    return getMesh(AssetType::EMITTER_POINT);
}

} // namespace sim::vis
