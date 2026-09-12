#pragma once

#include "VirtualTarget.hpp"
#include <random>
#include <vector>
#include <cstdint>

namespace sim {

class GroundTruthGenerator {
public:
    explicit GroundTruthGenerator(uint64_t seed = 42) noexcept;

    void setSeed(uint64_t seed) noexcept;
    [[nodiscard]] uint64_t getSeed() const noexcept;
    void reset() noexcept;

    [[nodiscard]] std::vector<VirtualTarget> generateSyntheticTargets(size_t count, double initial_time = 0.0);

    [[nodiscard]] double uniformReal(double min_val, double max_val);

private:
    uint64_t m_initialSeed{42};
    std::mt19937_64 m_rng;
};

} // namespace sim
