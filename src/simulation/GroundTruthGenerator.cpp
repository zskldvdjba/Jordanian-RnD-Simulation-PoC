#include "GroundTruthGenerator.hpp"
#include <iomanip>
#include <sstream>

namespace sim {

GroundTruthGenerator::GroundTruthGenerator(uint64_t seed) noexcept
    : m_initialSeed(seed), m_rng(seed) {}

void GroundTruthGenerator::setSeed(uint64_t seed) noexcept {
    m_initialSeed = seed;
    m_rng.seed(seed);
}

uint64_t GroundTruthGenerator::getSeed() const noexcept {
    return m_initialSeed;
}

void GroundTruthGenerator::reset() noexcept {
    m_rng.seed(m_initialSeed);
}

double GroundTruthGenerator::uniformReal(double min_val, double max_val) {
    std::uniform_real_distribution<double> dist(min_val, max_val);
    return dist(m_rng);
}

std::vector<VirtualTarget> GroundTruthGenerator::generateSyntheticTargets(size_t count, double initial_time) {
    std::vector<VirtualTarget> targets;
    targets.reserve(count);

    std::uniform_real_distribution<double> posDistXY(-10000.0, 10000.0);
    std::uniform_real_distribution<double> posDistZ(500.0, 3000.0);
    std::uniform_real_distribution<double> velDistXY(-250.0, 250.0);
    std::uniform_real_distribution<double> velDistZ(-10.0, 10.0);

    for (size_t i = 1; i <= count; ++i) {
        std::ostringstream oss;
        oss << "VT-" << std::setfill('0') << std::setw(3) << i;

        Vector3D pos{posDistXY(m_rng), posDistXY(m_rng), posDistZ(m_rng)};
        Vector3D vel{velDistXY(m_rng), velDistXY(m_rng), velDistZ(m_rng)};

        targets.emplace_back(oss.str(), initial_time, pos, vel);
    }

    return targets;
}

} // namespace sim
