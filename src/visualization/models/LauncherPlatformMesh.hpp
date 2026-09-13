#pragma once

#include "../VisualAssetTypes.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace sim::vis {

struct LauncherPlatformMesh {
    static AssetMesh generate(float scale = 1.0f) {
        AssetMesh mesh;
        mesh.type = AssetType::LAUNCHER_PLATFORM;
        mesh.boundingRadius = 160.0f * scale;
        mesh.reserve(360);

        uint8_t chassisR = 85, chassisG = 90, chassisB = 95;
        uint8_t turretR = 100, turretG = 110, turretB = 120;
        uint8_t barrelR = 70, barrelG = 75, barrelB = 80;
        uint8_t wheelR = 40, wheelG = 42, wheelB = 45;

        float pw = 60.0f * scale;
        float pd = 40.0f * scale;
        float ph = 12.0f * scale;
        addBox(mesh, -pw * 0.5f, 0, -pd * 0.5f, pw, ph, pd, chassisR, chassisG, chassisB);

        float wheelR2 = 8.0f * scale;
        float wheelY = wheelR2;
        addCylinder(mesh, -pw * 0.35f, wheelY, -pd * 0.5f - 2.0f * scale, wheelR2, 4.0f * scale, 8, wheelR, wheelG, wheelB);
        addCylinder(mesh, pw * 0.35f, wheelY, -pd * 0.5f - 2.0f * scale, wheelR2, 4.0f * scale, 8, wheelR, wheelG, wheelB);
        addCylinder(mesh, -pw * 0.35f, wheelY, pd * 0.5f + 2.0f * scale, wheelR2, 4.0f * scale, 8, wheelR, wheelG, wheelB);
        addCylinder(mesh, pw * 0.35f, wheelY, pd * 0.5f + 2.0f * scale, wheelR2, 4.0f * scale, 8, wheelR, wheelG, wheelB);

        float tr = 15.0f * scale;
        float th = 10.0f * scale;
        addCylinder(mesh, 0, ph, 0, tr, th, 10, turretR, turretG, turretB);

        float bl = 50.0f * scale;
        float bw = 4.0f * scale;
        float bh = 4.0f * scale;
        addBox(mesh, -bw * 0.5f, ph + th, -bl * 0.25f, bw, bh, bl, barrelR, barrelG, barrelB);

        addBox(mesh, -bw * 0.5f - 6.0f * scale, ph + th, -bl * 0.25f, bw, bh, bl * 0.6f, barrelR, barrelG, barrelB);

        return mesh;
    }

private:
    static void addTri(AssetMesh& mesh,
                       float x0, float y0, float z0,
                       float x1, float y1, float z1,
                       float x2, float y2, float z2,
                       uint8_t r, uint8_t g, uint8_t b) {
        float ux = x1 - x0, uy = y1 - y0, uz = z1 - z0;
        float vx = x2 - x0, vy = y2 - y0, vz = z2 - z0;
        float nx = uy * vz - uz * vy;
        float ny = uz * vx - ux * vz;
        float nz = ux * vy - uy * vx;
        float len = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 1e-8f) { nx /= len; ny /= len; nz /= len; }
        mesh.vertices.emplace_back(x0, y0, z0, nx, ny, nz, r, g, b);
        mesh.vertices.emplace_back(x1, y1, z1, nx, ny, nz, r, g, b);
        mesh.vertices.emplace_back(x2, y2, z2, nx, ny, nz, r, g, b);
    }

    static void addBox(AssetMesh& mesh,
                       float x, float y, float z,
                       float w, float h, float d,
                       uint8_t r, uint8_t g, uint8_t b) {
        float x1 = x + w, y1 = y + h, z1 = z + d;
        addTri(mesh, x, y, z, x1, y, z, x1, y1, z, r, g, b);
        addTri(mesh, x, y, z, x1, y1, z, x, y1, z, r, g, b);
        addTri(mesh, x1, y, z, x1, y, z1, x1, y1, z1, r, g, b);
        addTri(mesh, x1, y, z, x1, y1, z1, x1, y1, z, r, g, b);
        addTri(mesh, x1, y, z1, x, y, z1, x, y1, z1, r, g, b);
        addTri(mesh, x1, y, z1, x, y1, z1, x1, y1, z1, r, g, b);
        addTri(mesh, x, y, z1, x, y, z, x, y1, z, r, g, b);
        addTri(mesh, x, y, z1, x, y1, z, x, y1, z1, r, g, b);
        addTri(mesh, x, y1, z, x1, y1, z, x1, y1, z1, r, g, b);
        addTri(mesh, x, y1, z, x1, y1, z1, x, y1, z1, r, g, b);
        addTri(mesh, x, y, z1, x1, y, z1, x1, y, z, r, g, b);
        addTri(mesh, x, y, z1, x1, y, z, x, y, z, r, g, b);
    }

    static void addCylinder(AssetMesh& mesh,
                            float cx, float cy, float cz,
                            float radius, float height, int segments,
                            uint8_t r, uint8_t g, uint8_t b) {
        for (int i = 0; i < segments; ++i) {
            float a0 = 2.0f * static_cast<float>(M_PI) * i / segments;
            float a1 = 2.0f * static_cast<float>(M_PI) * (i + 1) / segments;
            float x0 = cx + radius * std::cos(a0);
            float z0 = cz + radius * std::sin(a0);
            float x1 = cx + radius * std::cos(a1);
            float z1 = cz + radius * std::sin(a1);

            addTri(mesh, x0, cy, z0, x1, cy, z1, x1, cy + height, z1, r, g, b);
            addTri(mesh, x0, cy, z0, x1, cy + height, z1, x0, cy + height, z0, r, g, b);

            float nnx0 = std::cos(a0), nnz0 = std::sin(a0);
            float nnx1 = std::cos(a1), nnz1 = std::sin(a1);
            mesh.vertices.emplace_back(x0, cy, z0, nnx0, 0.0f, nnz0, r, g, b);
            mesh.vertices.emplace_back(x1, cy, z1, nnx1, 0.0f, nnz1, r, g, b);
            mesh.vertices.emplace_back(x1, cy + height, z1, nnx1, 0.0f, nnz1, r, g, b);
            mesh.vertices.emplace_back(x0, cy, z0, nnx0, 0.0f, nnz0, r, g, b);
            mesh.vertices.emplace_back(x1, cy + height, z1, nnx1, 0.0f, nnz1, r, g, b);
            mesh.vertices.emplace_back(x0, cy + height, z0, nnx0, 0.0f, nnz0, r, g, b);
        }
    }
};

} // namespace sim::vis
