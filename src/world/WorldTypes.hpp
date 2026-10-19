#pragma once

#include "raylib.h"
#include <vector>

enum class ObstacleType {
    NONE = 0,
    ROOFTOP_SURFACE,
    OBSTACLE_BLOCK,      // HVAC unit / crate (can mantle or jump over)
    SLIDE_PIPE,          // Red low pipe - must slide underneath
    VAULT_BARRIER,       // Red knee-high barrier - fast vault
    WALLRUN_SURFACE,     // Red vertical wall billboard - enables wall run
    SPRINGBOARD,         // Red launch ramp - launches player forward & up
    BALANCE_BEAM,        // Red narrow walkway between towers
    GLASS_SKYLIGHT       // Breakable / slippery surface
};

struct WorldObject {
    BoundingBox box;
    Color color;
    ObstacleType type;
    Vector3 wallNormal;   // Points away from wall (for wall-running)
    bool isWallrunLeft;    // True if wall is to the left of the running line
    bool isActionable;    // True if Runner Vision Red
};

struct BackgroundBuilding {
    Vector3 position;
    Vector3 size;
    Color color;
    Color accentColor;
};

struct RooftopChunk {
    float startZ;
    float endZ;
    BoundingBox mainRoof;
    std::vector<WorldObject> objects;
};
