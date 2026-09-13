#include "Frustum.hpp"
#include <cmath>

namespace sim::vis {

void Plane::normalize() noexcept {
    double mag = std::sqrt(a * a + b * b + c * c);
    if (mag > 1e-12) {
        a /= mag;
        b /= mag;
        c /= mag;
        d /= mag;
    }
}

double Plane::distanceToPoint(const Vector3D& pt) const noexcept {
    return a * pt.x + b * pt.y + c * pt.z + d;
}

void Frustum::updateFromMatrices(const double* m, const double* p) noexcept {
    double clip[16];

    // Multiply Projection * Modelview
    clip[0]  = m[0] * p[0]  + m[1] * p[4]  + m[2] * p[8]   + m[3] * p[12];
    clip[1]  = m[0] * p[1]  + m[1] * p[5]  + m[2] * p[9]   + m[3] * p[13];
    clip[2]  = m[0] * p[2]  + m[1] * p[6]  + m[2] * p[10]  + m[3] * p[14];
    clip[3]  = m[0] * p[3]  + m[1] * p[7]  + m[2] * p[11]  + m[3] * p[15];

    clip[4]  = m[4] * p[0]  + m[5] * p[4]  + m[6] * p[8]   + m[7] * p[12];
    clip[5]  = m[4] * p[1]  + m[5] * p[5]  + m[6] * p[9]   + m[7] * p[13];
    clip[6]  = m[4] * p[2]  + m[5] * p[6]  + m[6] * p[10]  + m[7] * p[14];
    clip[7]  = m[4] * p[3]  + m[5] * p[7]  + m[6] * p[11]  + m[7] * p[15];

    clip[8]  = m[8] * p[0]  + m[9] * p[4]  + m[10] * p[8]  + m[11] * p[12];
    clip[9]  = m[8] * p[1]  + m[9] * p[5]  + m[10] * p[9]  + m[11] * p[13];
    clip[10] = m[8] * p[2]  + m[9] * p[6]  + m[10] * p[10] + m[11] * p[14];
    clip[11] = m[8] * p[3]  + m[9] * p[7]  + m[10] * p[11] + m[11] * p[15];

    clip[12] = m[12] * p[0] + m[13] * p[4] + m[14] * p[8]  + m[15] * p[12];
    clip[13] = m[12] * p[1] + m[13] * p[5] + m[14] * p[9]  + m[15] * p[13];
    clip[14] = m[12] * p[2] + m[13] * p[6] + m[14] * p[10] + m[15] * p[14];
    clip[15] = m[12] * p[3] + m[13] * p[7] + m[14] * p[11] + m[15] * p[15];

    // Left
    m_planes[0].a = clip[3]  + clip[0];
    m_planes[0].b = clip[7]  + clip[4];
    m_planes[0].c = clip[11] + clip[8];
    m_planes[0].d = clip[15] + clip[12];

    // Right
    m_planes[1].a = clip[3]  - clip[0];
    m_planes[1].b = clip[7]  - clip[4];
    m_planes[1].c = clip[11] - clip[8];
    m_planes[1].d = clip[15] - clip[12];

    // Bottom
    m_planes[2].a = clip[3]  + clip[1];
    m_planes[2].b = clip[7]  + clip[5];
    m_planes[2].c = clip[11] + clip[9];
    m_planes[2].d = clip[15] + clip[13];

    // Top
    m_planes[3].a = clip[3]  - clip[1];
    m_planes[3].b = clip[7]  - clip[5];
    m_planes[3].c = clip[11] - clip[9];
    m_planes[3].d = clip[15] - clip[13];

    // Near
    m_planes[4].a = clip[3]  + clip[2];
    m_planes[4].b = clip[7]  + clip[6];
    m_planes[4].c = clip[11] + clip[10];
    m_planes[4].d = clip[15] + clip[14];

    // Far
    m_planes[5].a = clip[3]  - clip[2];
    m_planes[5].b = clip[7]  - clip[6];
    m_planes[5].c = clip[11] - clip[10];
    m_planes[5].d = clip[15] - clip[14];

    for (int i = 0; i < 6; ++i) {
        m_planes[i].normalize();
    }
}

bool Frustum::isSphereInside(const Vector3D& center, double radius) const noexcept {
    for (int i = 0; i < 6; ++i) {
        if (m_planes[i].distanceToPoint(center) < -radius) {
            return false; // Culled: outside this plane
        }
    }
    return true; // Inside or intersecting
}

LodLevel Frustum::getLodForDistance(double distance) const noexcept {
    if (distance < m_lodNear) {
        return LodLevel::HIGH;
    } else if (distance < m_lodFar) {
        return LodLevel::MEDIUM;
    }
    return LodLevel::LOW;
}

void Frustum::setLodDistances(double nearDist, double farDist) noexcept {
    m_lodNear = nearDist;
    m_lodFar = farDist;
}

} // namespace sim::vis
