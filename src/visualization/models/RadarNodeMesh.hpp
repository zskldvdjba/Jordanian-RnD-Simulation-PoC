#pragma once

#include "../VisualAssetTypes.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace sim::vis {

struct RadarNodeMesh {
    static AssetMesh generate(float scale = 1.0f) {
        AssetMesh mesh;
        mesh.type = AssetType::RADAR_NODE;
        mesh.boundingRadius = 180.0f * scale;
        mesh.reserve(320);

        uint8_t baseR = 60, baseG = 140, baseB = 180;
        uint8_t dishR = 80, dishG = 180, dishB = 220;
        uint8_t poleR = 90, poleG = 95, poleB = 100;

        float poleR2 = 8.0f * scale;
        float poleH = 120.0f * scale;
        addCylinder(mesh, 0, 0, 0, poleR2, poleH, 8, poleR, poleG, poleB);

        float dishR2 = 50.0f * scale;
        float dishH = 10.0f * scale;
        float dishY = poleH;
        addCylinder(mesh, 0, dishY, 0, dishR2, dishH, 12, dishR, dishG, dishB);

        addDish(mesh, 0, dishY + dishH, 0, dishR2 * 0.9f, 8, dishR, dishG, dishB);

        float armLen = 30.0f * scale;
        addBox(mesh, -armLen, dishY + dishH * 0.5f, -3.0f * scale,
               armLen * 2, 4.0f * scale, 6.0f * scale, baseR, baseG, baseB);

        return mesh;
    }

private:
    static void addTriangle(AssetMesh& mesh,
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

            addTriangle(mesh, x0, cy, z0, x1, cy, z1, x1, cy + height, z1, r, g, b);
            addTriangle(mesh, x0, cy, z0, x1, cy + height, z1, x0, cy + height, z0, r, g, b);

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

    static void addDish(AssetMesh& mesh,
                        float cx, float cy, float cz,
                        float radius, int segments,
                        uint8_t r, uint8_t g, uint8_t b) {
        float depth = radius * 0.3f;
        mesh.vertices.emplace_back(cx, cy + depth, cz, 0.0f, 1.0f, 0.0f, r, g, b);
        for (int i = 0; i < segments; ++i) {
            float a0 = 2.0f * static_cast<float>(M_PI) * i / segments;
            float a1 = 2.0f * static_cast<float>(M_PI) * (i + 1) / segments;
            float x0 = cx + radius * std::cos(a0);
            float z0 = cz + radius * std::sin(a0);
            float x1 = cx + radius * std::cos(a1);
            float z1 = cz + radius * std::sin(a1);

            float nx0 = std::cos(a0) * 0.7f, ny0 = 0.7f, nz0 = std::sin(a0) * 0.7f;
            float nx1 = std::cos(a1) * 0.7f, ny1 = 0.7f, nz1 = std::sin(a1) * 0.7f;

            mesh.vertices.emplace_back(cx, cy + depth, cz, 0.0f, 1.0f, 0.0f, r, g, b);
            mesh.vertices.emplace_back(x0, cy, z0, nx0, ny0, nz0, r, g, b);
            mesh.vertices.emplace_back(x1, cy, z1, nx1, ny1, nz1, r, g, b);
        }
    }

    static void addBox(AssetMesh& mesh,
                       float x, float y, float z,
                       float w, float h, float d,
                       uint8_t r, uint8_t g, uint8_t b) {
        float x1 = x + w, y1 = y + h, z1 = z + d;
        addTriangle(mesh, x, y, z, x1, y, z, x1, y1, z, r, g, b);
        addTriangle(mesh, x, y, z, x1, y1, z, x, y1, z, r, g, b);
        addTriangle(mesh, x1, y, z, x1, y, z1, x1, y1, z1, r, g, b);
        addTriangle(mesh, x1, y, z, x1, y1, z1, x1, y1, z, r, g, b);
        addTriangle(mesh, x1, y, z1, x, y, z1, x, y1, z1, r, g, b);
        addTriangle(mesh, x1, y, z1, x, y1, z1, x1, y1, z1, r, g, b);
        addTriangle(mesh, x, y, z1, x, y, z, x, y1, z, r, g, b);
        addTriangle(mesh, x, y, z1, x, y1, z, x, y1, z1, r, g, b);
        addTriangle(mesh, x, y1, z, x1, y1, z, x1, y1, z1, r, g, b);
        addTriangle(mesh, x, y1, z, x1, y1, z1, x, y1, z1, r, g, b);
        addTriangle(mesh, x, y, z1, x1, y, z1, x1, y, z, r, g, b);
        addTriangle(mesh, x, y, z1, x1, y, z, x, y, z, r, g, b);
    }
};

} // namespace sim::vis
