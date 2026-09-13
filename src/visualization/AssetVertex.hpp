#pragma once

#include <cstdint>

namespace sim::vis {

struct AssetVertex {
    float px{0.0f};
    float py{0.0f};
    float pz{0.0f};
    float nx{0.0f};
    float ny{0.0f};
    float nz{1.0f};
    uint8_t r{255};
    uint8_t g{255};
    uint8_t b{255};
    uint8_t a{255};

    AssetVertex() = default;

    AssetVertex(float x, float y, float z,
                float nnx, float nny, float nnz,
                uint8_t rr, uint8_t gg, uint8_t bb, uint8_t aa = 255)
        : px(x), py(y), pz(z), nx(nnx), ny(nny), nz(nnz),
          r(rr), g(gg), b(bb), a(aa) {}

    AssetVertex(float x, float y, float z, uint8_t rr, uint8_t gg, uint8_t bb, uint8_t aa = 255)
        : px(x), py(y), pz(z), nx(0.0f), ny(0.0f), nz(1.0f),
          r(rr), g(gg), b(bb), a(aa) {}
};

} // namespace sim::vis
