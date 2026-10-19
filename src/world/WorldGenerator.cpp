#include "WorldGenerator.hpp"
#include "render/ShadowSystem.hpp"
#include "core/MathUtils.hpp"
#include <cstdlib>
#include <cmath>

WorldGenerator::WorldGenerator() {
    Reset();
}

void WorldGenerator::Reset() {
    activeChunks.clear();
    solidCache.clear();
    backgroundSkyline.clear();

    currentGenZ = 15.0f; // Start a little behind player
    currentRoofY = 0.0f;
    chunkCounter = 0;

    GenerateSkyline();
    SpawnInitialChunks();
}

void WorldGenerator::GenerateSkyline() {
    backgroundSkyline.clear();
    // Generate a sprawling city grid of background skyscrapers
    for (int i = 0; i < 90; ++i) {
        float xSide = (i % 2 == 0) ? 1.0f : -1.0f;
        float xDist = 45.0f + (float)(rand() % 160);
        float posX = xSide * xDist;
        float posZ = 100.0f - (float)i * 35.0f;
        float height = 150.0f + (float)(rand() % 220);
        float width = 25.0f + (float)(rand() % 40);
        float length = 25.0f + (float)(rand() % 40);

        Color col = (rand() % 3 == 0) ? COLOR_ROOF_MAIN : COLOR_ROOF_DARK;
        Color accent = (rand() % 4 == 0) ? COLOR_RUNNER_RED : COLOR_GLASS_CYAN;

        backgroundSkyline.push_back({
            .position = { posX, height * 0.5f - 80.0f, posZ },
            .size = { width, height, length },
            .color = col,
            .accentColor = accent
        });
    }
}

void WorldGenerator::SpawnInitialChunks() {
    // Starting runway: flat, wide, safe to build initial momentum
    RooftopChunk startChunk;
    startChunk.startZ = currentGenZ;
    float length = 75.0f;
    startChunk.endZ = currentGenZ - length;

    float width = 22.0f;
    float height = 150.0f;
    startChunk.mainRoof = {
        .min = { -width * 0.5f, currentRoofY - height, startChunk.endZ },
        .max = { width * 0.5f, currentRoofY, startChunk.startZ }
    };

    // Main roof surface
    startChunk.objects.push_back({
        .box = startChunk.mainRoof,
        .color = COLOR_ROOF_MAIN,
        .type = ObstacleType::ROOFTOP_SURFACE,
        .wallNormal = { 0, 1, 0 },
        .isWallrunLeft = false,
        .isActionable = false
    });

    // Add a couple of initial low vault hurdles on the runway to teach the player
    startChunk.objects.push_back({
        .box = {
            .min = { -6.0f, currentRoofY, startChunk.startZ - 35.0f },
            .max = { 6.0f, currentRoofY + 0.9f, startChunk.startZ - 33.5f }
        },
        .color = COLOR_RUNNER_RED,
        .type = ObstacleType::VAULT_BARRIER,
        .wallNormal = { 0, 0, 1 },
        .isWallrunLeft = false,
        .isActionable = true
    });

    activeChunks.push_back(startChunk);
    currentGenZ = startChunk.endZ;

    // Pre-spawn several upcoming rooftops
    for (int i = 0; i < 7; ++i) {
        SpawnNextChunk();
    }

    RebuildSolidCache();
}

void WorldGenerator::SpawnNextChunk() {
    chunkCounter++;

    // Gap calculation: expands as the player progresses
    float minGap = 8.0f;
    float maxGap = std::min(11.0f + (float)chunkCounter * 0.4f, 22.0f);
    float gap = minGap + (float)(rand() % (int)(maxGap - minGap + 1.0f));

    // Slight elevation variation
    float yDelta = 0.0f;
    if (chunkCounter > 2) {
        int r = rand() % 5;
        if (r == 0) yDelta = 2.0f;      // Slight step up
        else if (r == 1) yDelta = -3.0f; // Step down (big leap!)
        else if (r == 2) yDelta = -1.5f;
    }
    currentRoofY += yDelta;

    float roofLength = 40.0f + (float)(rand() % 35);
    float roofWidth = 18.0f + (float)(rand() % 10);
    float buildingDepth = 180.0f;

    float chunkStartZ = currentGenZ - gap;
    float chunkEndZ = chunkStartZ - roofLength;

    RooftopChunk chunk;
    chunk.startZ = chunkStartZ;
    chunk.endZ = chunkEndZ;
    chunk.mainRoof = {
        .min = { -roofWidth * 0.5f, currentRoofY - buildingDepth, chunkEndZ },
        .max = { roofWidth * 0.5f, currentRoofY, chunkStartZ }
    };

    // Add the main building body
    chunk.objects.push_back({
        .box = chunk.mainRoof,
        .color = (chunkCounter % 3 == 0) ? COLOR_ROOF_DARK : COLOR_ROOF_MAIN,
        .type = ObstacleType::ROOFTOP_SURFACE,
        .wallNormal = { 0, 1, 0 },
        .isWallrunLeft = false,
        .isActionable = false
    });

    // Decorative edge strips (Runner Vision red accent on the front edge)
    chunk.objects.push_back({
        .box = {
            .min = { -roofWidth * 0.5f, currentRoofY - 0.2f, chunkStartZ - 0.6f },
            .max = { roofWidth * 0.5f, currentRoofY + 0.05f, chunkStartZ }
        },
        .color = COLOR_RUNNER_RED,
        .type = ObstacleType::NONE,
        .wallNormal = { 0, 0, 1 },
        .isWallrunLeft = false,
        .isActionable = false
    });

    // Special Gap Features (Bridging the gap before this roof)
    int gapFeature = rand() % 4;
    if (gap > 13.0f || gapFeature == 1) {
        // Red Springboard Ramp on the previous roof edge
        chunk.objects.push_back({
            .box = {
                .min = { -3.0f, currentRoofY - yDelta, currentGenZ - 3.5f },
                .max = { 3.0f, currentRoofY - yDelta + 0.8f, currentGenZ }
            },
            .color = COLOR_RUNNER_RED,
            .type = ObstacleType::SPRINGBOARD,
            .wallNormal = { 0, 1, 0 },
            .isWallrunLeft = false,
            .isActionable = true
        });
    }

    if (gap > 12.0f && (gapFeature == 2 || gapFeature == 3)) {
        // Wall-run billboard bridging the gap!
        bool wallOnLeft = (rand() % 2 == 0);
        float wallX = wallOnLeft ? (-roofWidth * 0.5f - 1.0f) : (roofWidth * 0.5f + 1.0f);
        float wallNormalX = wallOnLeft ? 1.0f : -1.0f;
        float wallHeight = 7.0f;

        chunk.objects.push_back({
            .box = {
                .min = { wallX - 0.3f, currentRoofY - 2.0f, chunkStartZ - 5.0f },
                .max = { wallX + 0.3f, currentRoofY + wallHeight, currentGenZ + 5.0f }
            },
            .color = COLOR_RUNNER_RED,
            .type = ObstacleType::WALLRUN_SURFACE,
            .wallNormal = { wallNormalX, 0, 0 },
            .isWallrunLeft = wallOnLeft,
            .isActionable = true
        });
    }

    // Rooftop Obstacles
    int obstaclePreset = rand() % 5;
    float midZ = (chunkStartZ + chunkEndZ) * 0.5f;

    if (obstaclePreset == 0) {
        // Red Low Slide Pipe: Suspended at 1.1m - crouching/sliding slides right under!
        chunk.objects.push_back({
            .box = {
                .min = { -roofWidth * 0.45f, currentRoofY + 0.85f, midZ - 0.4f },
                .max = { roofWidth * 0.45f, currentRoofY + 1.25f, midZ + 0.4f }
            },
            .color = COLOR_RUNNER_RED,
            .type = ObstacleType::SLIDE_PIPE,
            .wallNormal = { 0, 0, 1 },
            .isWallrunLeft = false,
            .isActionable = true
        });
        // Support posts
        chunk.objects.push_back({
            .box = {
                .min = { -roofWidth * 0.45f, currentRoofY, midZ - 0.4f },
                .max = { -roofWidth * 0.40f, currentRoofY + 1.25f, midZ + 0.4f }
            },
            .color = COLOR_HVAC_STEEL,
            .type = ObstacleType::OBSTACLE_BLOCK,
            .wallNormal = { 1, 0, 0 },
            .isWallrunLeft = false,
            .isActionable = false
        });
        chunk.objects.push_back({
            .box = {
                .min = { roofWidth * 0.40f, currentRoofY, midZ - 0.4f },
                .max = { roofWidth * 0.45f, currentRoofY + 1.25f, midZ + 0.4f }
            },
            .color = COLOR_HVAC_STEEL,
            .type = ObstacleType::OBSTACLE_BLOCK,
            .wallNormal = { -1, 0, 0 },
            .isWallrunLeft = false,
            .isActionable = false
        });
    } else if (obstaclePreset == 1) {
        // Red Vault Barriers across the runway
        float barrierZ = chunkStartZ - 14.0f;
        chunk.objects.push_back({
            .box = {
                .min = { -5.0f, currentRoofY, barrierZ - 0.4f },
                .max = { 5.0f, currentRoofY + 0.95f, barrierZ + 0.4f }
            },
            .color = COLOR_RUNNER_RED,
            .type = ObstacleType::VAULT_BARRIER,
            .wallNormal = { 0, 0, 1 },
            .isWallrunLeft = false,
            .isActionable = true
        });
    } else if (obstaclePreset == 2) {
        // Industrial HVAC ventilation units forming a parkour chicane
        chunk.objects.push_back({
            .box = {
                .min = { -7.0f, currentRoofY, midZ - 5.0f },
                .max = { -1.0f, currentRoofY + 2.2f, midZ + 5.0f }
            },
            .color = COLOR_HVAC_STEEL,
            .type = ObstacleType::OBSTACLE_BLOCK,
            .wallNormal = { 1, 0, 0 },
            .isWallrunLeft = false,
            .isActionable = false
        });
        // Red runner marker on top of the AC unit for a high-line route
        chunk.objects.push_back({
            .box = {
                .min = { -6.8f, currentRoofY + 2.2f, midZ - 4.8f },
                .max = { -1.2f, currentRoofY + 2.25f, midZ + 4.8f }
            },
            .color = COLOR_RUNNER_RED,
            .type = ObstacleType::NONE,
            .wallNormal = { 0, 1, 0 },
            .isWallrunLeft = false,
            .isActionable = true
        });
    } else if (obstaclePreset == 3) {
        // Narrow Red Balance Beam connecting sections
        chunk.objects.push_back({
            .box = {
                .min = { -0.8f, currentRoofY + 0.3f, midZ - 8.0f },
                .max = { 0.8f, currentRoofY + 0.5f, midZ + 8.0f }
            },
            .color = COLOR_RUNNER_RED,
            .type = ObstacleType::BALANCE_BEAM,
            .wallNormal = { 0, 1, 0 },
            .isWallrunLeft = false,
            .isActionable = true
        });
    }

    activeChunks.push_back(chunk);
    currentGenZ = chunkEndZ;
}

void WorldGenerator::RebuildSolidCache() {
    solidCache.clear();
    for (const auto& chunk : activeChunks) {
        for (const auto& obj : chunk.objects) {
            if (obj.type != ObstacleType::NONE) {
                solidCache.push_back(obj);
            }
        }
    }
}

void WorldGenerator::Update(float playerZ) {
    // Check if the oldest chunk is far behind player
    bool changed = false;
    while (!activeChunks.empty() && activeChunks.front().endZ > playerZ + 45.0f) {
        activeChunks.pop_front();
        changed = true;
    }

    // Keep ahead chunks populated up to 250m ahead
    while (activeChunks.empty() || activeChunks.back().endZ > playerZ - 260.0f) {
        SpawnNextChunk();
        changed = true;
    }

    if (changed) {
        RebuildSolidCache();
    }
}

bool WorldGenerator::CheckWallrunCandidate(const BoundingBox& playerBox, Vector3 moveDir, WorldObject& outWall) {
    (void)moveDir;
    // Check solidCache for any WALLRUN_SURFACE close to the player
    float testDist = 1.35f;
    BoundingBox expandedPlayer = {
        .min = { playerBox.min.x - testDist, playerBox.min.y, playerBox.min.z - 0.5f },
        .max = { playerBox.max.x + testDist, playerBox.max.y, playerBox.max.z + 0.5f }
    };

    for (const auto& obj : solidCache) {
        if (obj.type == ObstacleType::WALLRUN_SURFACE) {
            if (CheckCollisionBoxes(expandedPlayer, obj.box)) {
                outWall = obj;
                return true;
            }
        }
    }
    return false;
}

bool WorldGenerator::CheckVaultCandidate(const Vector3& playerPos, const Vector3& lookDir, WorldObject& outLedge) {
    // Cast a probe forward 1.8m
    Vector3 forwardCheck = playerPos + lookDir * 1.8f;
    for (const auto& obj : solidCache) {
        if (obj.type == ObstacleType::VAULT_BARRIER || obj.type == ObstacleType::OBSTACLE_BLOCK) {
            if (forwardCheck.x >= obj.box.min.x && forwardCheck.x <= obj.box.max.x &&
                forwardCheck.z >= obj.box.min.z && forwardCheck.z <= obj.box.max.z) {
                // Check if top of object is waist/chest level
                float topY = obj.box.max.y;
                if (topY > playerPos.y + 0.2f && topY < playerPos.y + 2.2f) {
                    outLedge = obj;
                    return true;
                }
            }
        }
    }
    return false;
}

float WorldGenerator::GetRoofYAt(float z) const {
    for (const auto& chunk : activeChunks) {
        if (z <= chunk.startZ + 2.0f && z >= chunk.endZ - 2.0f) {
            return chunk.mainRoof.max.y;
        }
    }
    if (!activeChunks.empty()) {
        return activeChunks.front().mainRoof.max.y;
    }
    return 0.0f;
}

void WorldGenerator::Draw(const Vector3& playerPos, ShadowSystem& shadows) {
    // 1. Draw background skyline and skyscrapers
    for (const auto& bg : backgroundSkyline) {
        DrawCube(bg.position, bg.size.x, bg.size.y, bg.size.z, bg.color);
        DrawCubeWires(bg.position, bg.size.x, bg.size.y, bg.size.z, { 180, 185, 195, 180 });

        // Accent roof bands
        Vector3 bandPos = { bg.position.x, bg.position.y + bg.size.y * 0.49f, bg.position.z };
        DrawCube(bandPos, bg.size.x * 1.01f, 1.5f, bg.size.z * 1.01f, bg.accentColor);
    }

    // 2. Draw active rooftop chunks and obstacles
    for (const auto& chunk : activeChunks) {
        for (const auto& obj : chunk.objects) {
            Vector3 size = {
                obj.box.max.x - obj.box.min.x,
                obj.box.max.y - obj.box.min.y,
                obj.box.max.z - obj.box.min.z
            };
            Vector3 center = {
                (obj.box.min.x + obj.box.max.x) * 0.5f,
                (obj.box.min.y + obj.box.max.y) * 0.5f,
                (obj.box.min.z + obj.box.max.z) * 0.5f
            };

            DrawCube(center, size.x, size.y, size.z, obj.color);

            // Give stark architectural outlines like Mirror's Edge
            if (obj.type == ObstacleType::ROOFTOP_SURFACE) {
                DrawCubeWires(center, size.x, size.y, size.z, { 190, 195, 205, 255 });
            } else if (obj.isActionable) {
                // High contrast white wire highlights on red actionable objects
                DrawCubeWires(center, size.x, size.y, size.z, { 255, 255, 255, 220 });
            }
        }
    }

    // 3. End architectural 3D lighting shader before rendering transparent cast shadows
    shadows.EndLighting();

    // 4. Render real-time directional cast shadows onto rooftop concrete
    for (const auto& chunk : activeChunks) {
        float roofY = chunk.mainRoof.max.y;
        float roofW = chunk.mainRoof.max.x - chunk.mainRoof.min.x;

        // Skyscraper diagonal shadow bands
        shadows.DrawSkyscraperShadowBands(roofY, chunk.startZ, chunk.endZ, roofW);

        // Directional cast shadows for all rooftop obstacles (HVAC units, low pipes, vault hurdles)
        shadows.DrawObstacleShadows(chunk.objects, roofY);
    }

    // 5. Render dynamic player runner shadow on the roof surface
    float groundY = GetRoofYAt(playerPos.z);
    shadows.DrawPlayerShadow(playerPos, groundY);
}
