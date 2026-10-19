#pragma once

#include "WorldTypes.hpp"
#include <deque>
#include <vector>

class ShadowSystem;

class WorldGenerator {
public:
    WorldGenerator();

    void Reset();
    void Update(float playerZ);
    void Draw(const Vector3& playerPos, ShadowSystem& shadows);

    float GetRoofYAt(float z) const;

    const std::deque<RooftopChunk>& GetChunks() const { return activeChunks; }
    const std::vector<WorldObject>& GetAllSolidObjects() const { return solidCache; }

    // Helpers to find actionable objects near the player
    bool CheckWallrunCandidate(const BoundingBox& playerBox, Vector3 moveDir, WorldObject& outWall);
    bool CheckVaultCandidate(const Vector3& playerPos, const Vector3& lookDir, WorldObject& outLedge);

private:
    std::deque<RooftopChunk> activeChunks;
    std::vector<WorldObject> solidCache;
    std::vector<BackgroundBuilding> backgroundSkyline;

    float currentGenZ = 0.0f;
    float currentRoofY = 0.0f;
    int chunkCounter = 0;

    void SpawnInitialChunks();
    void SpawnNextChunk();
    void GenerateSkyline();
    void RebuildSolidCache();

    // Visual Palette (Mirror's Edge Aesthetic)
    const Color COLOR_ROOF_MAIN = { 240, 242, 245, 255 };      // Stark white concrete
    const Color COLOR_ROOF_DARK = { 210, 215, 222, 255 };      // Shaded concrete
    const Color COLOR_RUNNER_RED = { 245, 25, 45, 255 };       // Iconic Runner Vision Red
    const Color COLOR_ACCENT_YELLOW = { 255, 195, 0, 255 };    // Caution stripe / booster
    const Color COLOR_HVAC_STEEL = { 180, 186, 194, 255 };     // Metal ventilation
    const Color COLOR_GLASS_CYAN = { 40, 140, 190, 255 };      // Skyscraper glass
};
