#include "ShadowSystem.hpp"
#include "core/MathUtils.hpp"
#include <cmath>
#include <algorithm>

ShadowSystem::ShadowSystem() = default;
ShadowSystem::~ShadowSystem() = default;

void ShadowSystem::Init() {
    sunDir = math::Normalize(sunDir);
    shadowShiftX = sunDir.x / sunDir.y;
    shadowShiftZ = -sunDir.z / sunDir.y;
}

void ShadowSystem::Close() {
    // Cleanup if needed
}

static void DrawQuad3D(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4, Color col) {
    DrawTriangle3D(p1, p2, p3, col);
    DrawTriangle3D(p1, p3, p4, col);
}

void ShadowSystem::DrawObstacleShadows(const std::vector<WorldObject>& objects, float roofY) {
    float groundY = roofY + 0.015f; // Raised slightly to prevent z-fighting with the roof slab

    for (const auto& obj : objects) {
        float h = obj.box.max.y - roofY;
        if (h <= 0.05f) continue;

        float ox = h * shadowShiftX;
        float oz = h * shadowShiftZ;

        // Base rectangle vertices on the roof
        Vector3 b1 = { obj.box.min.x, groundY, obj.box.min.z };
        Vector3 b2 = { obj.box.max.x, groundY, obj.box.min.z };
        Vector3 b3 = { obj.box.max.x, groundY, obj.box.max.z };
        Vector3 b4 = { obj.box.min.x, groundY, obj.box.max.z };

        // Projected top rectangle vertices on the roof
        Vector3 t1 = { obj.box.min.x + ox, groundY, obj.box.min.z + oz };
        Vector3 t2 = { obj.box.max.x + ox, groundY, obj.box.min.z + oz };
        Vector3 t3 = { obj.box.max.x + ox, groundY, obj.box.max.z + oz };
        Vector3 t4 = { obj.box.min.x + ox, groundY, obj.box.max.z + oz };

        if (obj.type == ObstacleType::SLIDE_PIPE) {
            // Low pipe shadow: a sharp rectangular shadow strip on the pavement
            DrawQuad3D(t1, t2, t3, t4, COLOR_CAST_SHADOW);
        } else if (obj.type == ObstacleType::OBSTACLE_BLOCK ||
                   obj.type == ObstacleType::VAULT_BARRIER ||
                   obj.type == ObstacleType::WALLRUN_SURFACE ||
                   obj.type == ObstacleType::SPRINGBOARD) {
            // Projected top footprint
            DrawQuad3D(t1, t2, t3, t4, COLOR_CAST_SHADOW);

            // Connect base to projected top to complete shadow polygon
            if (ox > 0.0f) {
                // Right side shadow fan
                DrawQuad3D(b2, t2, t3, b3, COLOR_CAST_SHADOW);
            }
            if (oz < 0.0f) {
                // Forward side shadow fan
                DrawQuad3D(b1, t1, t2, b2, COLOR_CAST_SHADOW);
            } else {
                // Rear side shadow fan
                DrawQuad3D(b3, t3, t4, b4, COLOR_CAST_SHADOW);
            }
        }
    }
}

void ShadowSystem::DrawSkyscraperShadowBands(float roofY, float startZ, float endZ, float roofWidth) {
    float groundY = roofY + 0.008f;
    float hw = roofWidth * 0.5f;

    // Draw 1-2 diagonal skyscraper shadow bands crossing the roof
    float bandMidZ = (startZ + endZ) * 0.5f;
    float bandWidth = 14.0f;
    float skew = 12.0f; // Diagonal tilt across rooftop

    Vector3 p1 = { -hw, groundY, bandMidZ - bandWidth * 0.5f - skew };
    Vector3 p2 = {  hw, groundY, bandMidZ - bandWidth * 0.5f + skew };
    Vector3 p3 = {  hw, groundY, bandMidZ + bandWidth * 0.5f + skew };
    Vector3 p4 = { -hw, groundY, bandMidZ + bandWidth * 0.5f - skew };

    DrawQuad3D(p1, p2, p3, p4, COLOR_BAND_SHADOW);
}

void ShadowSystem::DrawPlayerShadow(const Vector3& playerPos, float groundY) {
    float h = playerPos.y - groundY;
    if (h < -0.1f || h > 22.0f) return;

    // Position of runner shadow on the pavement
    float ox = h * shadowShiftX * 0.65f;
    float oz = h * shadowShiftZ * 0.65f;
    Vector3 center = { playerPos.x + ox, groundY + 0.02f, playerPos.z + oz };

    float radius = 0.50f * std::clamp(1.0f - h * 0.035f, 0.35f, 1.0f);
    unsigned char alpha = static_cast<unsigned char>(std::clamp(135 - (int)(h * 4.5f), 30, 135));
    Color pCol = { 20, 32, 54, alpha };

    // 8-segment circular runner shadow disc
    const int segments = 8;
    for (int i = 0; i < segments; ++i) {
        float a1 = (float)i * (2.0f * PI / segments);
        float a2 = (float)(i + 1) * (2.0f * PI / segments);

        Vector3 v1 = center;
        Vector3 v2 = { center.x + std::cos(a1) * radius, center.y, center.z + std::sin(a1) * radius * 1.35f };
        Vector3 v3 = { center.x + std::cos(a2) * radius, center.y, center.z + std::sin(a2) * radius * 1.35f };

        DrawTriangle3D(v1, v2, v3, pCol);
    }
}
