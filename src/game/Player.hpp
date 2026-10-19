#pragma once

#include "raylib.h"
#include "world/WorldGenerator.hpp"
#include "core/AudioSystem.hpp"
#include <string>
#include <vector>

enum class MoveState {
    RUNNING,
    AIRBORNE,
    SLIDING,
    WALL_RUNNING,
    VAULTING,
    ROLLING,
    DEAD
};

struct ParkourStuntNotice {
    std::string text;
    int points;
    float timer;
};

class Player {
public:
    Player();

    void Reset(Vector3 startPos);
    void Update(float dt, WorldGenerator& world, AudioSystem& audio);
    void Draw();

    Camera3D GetCamera() const;
    Vector3 GetPosition() const { return position; }
    Vector3 GetVelocity() const { return velocity; }
    float GetSpeed() const;
    float GetMaxSpeed() const { return maxSprintSpeed; }
    float GetFlowMultiplier() const { return flowMultiplier; }
    int GetScore() const { return score; }
    float GetDistance() const { return totalDistance; }
    MoveState GetState() const { return state; }
    bool IsDead() const { return state == MoveState::DEAD; }
    const std::vector<ParkourStuntNotice>& GetStuntNotices() const { return stuntNotices; }

private:
    Vector3 position;
    Vector3 velocity;

    // First person mouse look
    float yaw = 0.0f;
    float pitch = 0.0f;
    float rollAngle = 0.0f;

    // Dynamic Camera Effects
    float currentEyeHeight = 1.65f;
    float targetEyeHeight = 1.65f;
    float currentFOV = 75.0f;
    float targetFOV = 75.0f;
    float headBobTimer = 0.0f;
    float headBobOffset = 0.0f;
    float pitchAnimOffset = 0.0f;
    float rollAnimTimer = 0.0f;
    float impactShakeTimer = 0.0f;
    float wallImpactCooldown = 0.0f;
    float footstepCooldown = 0.0f;
    bool isBlockedForward = false;

    // Momentum & Speeds
    float baseSpeed = 12.0f;
    float maxSprintSpeed = 24.0f;
    float forwardMomentum = 12.0f;
    float flowMultiplier = 1.0f;
    float flowTimer = 0.0f;
    int score = 0;
    float totalDistance = 0.0f;

    // Parkour States
    MoveState state = MoveState::RUNNING;
    bool isGrounded = true;
    float coyoteTimer = 0.0f;
    float jumpBufferTimer = 0.0f;
    float slideTimer = 0.0f;
    float wallrunTimer = 0.0f;
    bool wallrunIsLeft = false;
    Vector3 wallrunNormal = { 0, 0, 0 };

    // Vaulting animation
    float vaultTimer = 0.0f;
    Vector3 vaultStart = { 0, 0, 0 };
    Vector3 vaultEnd = { 0, 0, 0 };

    // Stunt notifications
    std::vector<ParkourStuntNotice> stuntNotices;

    void TriggerStunt(const std::string& name, int pts, AudioSystem& audio);
    BoundingBox GetBoundingBox(float eyeH) const;
    void ResolveCollisions(Vector3& pos, Vector3& vel, float eyeH, WorldGenerator& world, AudioSystem& audio);
};
