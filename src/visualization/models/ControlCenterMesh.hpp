#pragma once

#include "../VisualAssetTypes.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace sim::vis {

struct ControlCenterMesh {
    static AssetMesh generate(float scale = 1.0f) {
        AssetMesh mesh;
        mesh.type = AssetType::CONTROL_CENTER;
        mesh.boundingRadius = 250.0f * scale;
        mesh.reserve(400);

        uint8_t wallR = 70, wallG = 75, wallB = 85;
        uint8_t roofR = 55, roofG = 60, roofB = 70;
        uint8_t radomeR = 160, radomeG = 180, radomeB = 200;
        uint8_t baseR = 50, baseG = 55, baseB = 60;

        float bw = 80.0f * scale;
        float bd = 60.0f * scale;
        float bh = 35.0f * scale;
        addBox(mesh, -bw * 0.5f, 0, -bd * 0.5f, bw, bh, bd, wallR, wallG, wallB);

        float rw = bw * 1.05f;
        float rd = bd * 1.05f;
        float rh = 8.0f * scale;
        addBox(mesh, -rw * 0.5f, bh, -rd * 0.5f, rw, rh, rd, roofR, roofG, roofB);

        float towerW = 15.0f * scale;
        float towerH = 40.0f * scale;
        addBox(mesh, bw * 0.2f, bh + rh, -towerW * 0.5f, towerW, towerH, towerW, wallR, wallG, wallB);

        float radomeR2 = 12.0f * scale;
        addSphere(mesh, bw * 0.2f + towerW * 0.5f, bh + rh + towerH, 0, radomeR2, 10, radomeR, radomeG, radomeB);

        float antW = 40.0f * scale;
        float antH = 3.0f * scale;
        float antD = 3.0f * scale;
        addBox(mesh, -bw * 0.3f, bh + rh + 5.0f * scale, -antD * 0.5f, antW, antH, antD, radomeR, radomeG, radomeB);

        float pW = bw * 0.6f;
        float pD = 4.0f * scale;
        float pH = 2.0f * scale;
        addBox(mesh, -pW * 0.5f, -pH, -pD * 0.5f, pW, pH, pD + bd, baseR, baseG, baseB);

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
        else { nx = 0.0f; ny = 1.0f; nz = 0.0f; }
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

    static void addSphere(AssetMesh& mesh,
                          float cx, float cy, float cz,
                          float radius, int segments,
                          uint8_t r, uint8_t g, uint8_t b) {
        for (int lat = 0; lat < segments / 2; ++lat) {
            float theta0 = static_cast<float>(M_PI) * lat / (segments / 2);
            float theta1 = static_cast<float>(M_PI) * (lat + 1) / (segments / 2);
            for (int lon = 0; lon < segments; ++lon) {
                float phi0 = 2.0f * static_cast<float>(M_PI) * lon / segments;
                float phi1 = 2.0f * static_cast<float>(M_PI) * (lon + 1) / segments;

                float x00 = cx + radius * std::sin(theta0) * std::cos(phi0);
                float y00 = cy + radius * std::cos(theta0);
                float z00 = cz + radius * std::sin(theta0) * std::sin(phi0);

                float x10 = cx + radius * std::sin(theta1) * std::cos(phi0);
                float y10 = cy + radius * std::cos(theta1);
                float z10 = cz + radius * std::sin(theta1) * std::sin(phi0);

                float x01 = cx + radius * std::sin(theta0) * std::cos(phi1);
                float y01 = cy + radius * std::cos(theta0);
                float z01 = cz + radius * std::sin(theta0) * std::sin(phi1);

                float x11 = cx + radius * std::sin(theta1) * std::cos(phi1);
                float y11 = cy + radius * std::cos(theta1);
                float z11 = cz + radius * std::sin(theta1) * std::sin(phi1);

                addTri(mesh, x00, y00, z00, x10, y10, z10, x11, y11, z11, r, g, b);
                addTri(mesh, x00, y00, z00, x11, y11, z11, x01, y01, z01, r, g, b);
            }
        }
    }
};

} // namespace sim::vis
