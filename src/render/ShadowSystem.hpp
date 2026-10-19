#pragma once

#include "raylib.h"
#include "world/WorldTypes.hpp"
#include <vector>

class ShadowSystem {
public:
    ShadowSystem();
    ~ShadowSystem();

    void Init();
    void Close();

    // Lighting hooks (kept for API compatibility)
    void BeginLighting(const Camera3D& camera) { (void)camera; }
    void EndLighting() {}

    // Renders real-time planar cast shadows onto rooftop surfaces
    void DrawObstacleShadows(const std::vector<WorldObject>& objects, float roofY);
    void DrawSkyscraperShadowBands(float roofY, float startZ, float endZ, float roofWidth);
    void DrawPlayerShadow(const Vector3& playerPos, float groundY);

    Vector3 GetSunDirection() const { return sunDir; }

private:
    // Directional Sun Light Vector (angled high above from left/back)
    Vector3 sunDir = { 0.55f, 0.90f, -0.32f };

    // Directional Shadow Projection Factors (shift per unit height)
    float shadowShiftX = 0.61f;  // sunDir.x / sunDir.y
    float shadowShiftZ = -0.35f; // -sunDir.z / sunDir.y

    const Color COLOR_CAST_SHADOW = { 25, 38, 62, 125 };      // Cool slate-blue shadow
    const Color COLOR_BAND_SHADOW = { 20, 32, 54, 90 };       // Skyscraper ambient shadow
};
