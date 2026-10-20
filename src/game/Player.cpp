#include "Player.hpp"
#include "core/MathUtils.hpp"
#include <cmath>
#include <algorithm>

Player::Player() {
    Reset({ 0.0f, 2.0f, 5.0f });
}

void Player::Reset(Vector3 startPos) {
    position = startPos;
    velocity = { 0.0f, 0.0f, -baseSpeed };
    yaw = 0.0f;
    pitch = 0.0f;
    rollAngle = 0.0f;

    currentEyeHeight = 1.65f;
    targetEyeHeight = 1.65f;
    currentFOV = 75.0f;
    targetFOV = 75.0f;

    headBobTimer = 0.0f;
    headBobOffset = 0.0f;
    pitchAnimOffset = 0.0f;
    rollAnimTimer = 0.0f;
    impactShakeTimer = 0.0f;
    wallImpactCooldown = 0.0f;
    footstepCooldown = 0.0f;
    isBlockedForward = false;

    forwardMomentum = baseSpeed;
    flowMultiplier = 1.0f;
    flowTimer = 0.0f;
    score = 0;
    totalDistance = 0.0f;

    state = MoveState::RUNNING;
    isGrounded = true;
    coyoteTimer = 0.0f;
    jumpBufferTimer = 0.0f;
    slideTimer = 0.0f;
    wallrunTimer = 0.0f;

    stuntNotices.clear();
}

float Player::GetSpeed() const {
    return math::LengthXZ(velocity);
}

void Player::TriggerStunt(const std::string& name, int pts, AudioSystem& audio) {
    int earned = static_cast<int>(pts * flowMultiplier);
    score += earned;
    stuntNotices.push_back({ name, earned, 1.8f });
    audio.PlayStuntBonus();
}

BoundingBox Player::GetBoundingBox(float eyeH) const {
    float radius = 0.40f;
    return {
        .min = { position.x - radius, position.y, position.z - radius },
        .max = { position.x + radius, position.y + eyeH + 0.15f, position.z + radius }
    };
}

void Player::Update(float dt, WorldGenerator& world, AudioSystem& audio) {
    if (state == MoveState::DEAD) return;

    // Decay impact camera shake & sound cooldowns
    if (impactShakeTimer > 0.0f) {
        impactShakeTimer = std::max(0.0f, impactShakeTimer - dt);
    }
    if (wallImpactCooldown > 0.0f) {
        wallImpactCooldown = std::max(0.0f, wallImpactCooldown - dt);
    }
    if (footstepCooldown > 0.0f) {
        footstepCooldown = std::max(0.0f, footstepCooldown - dt);
    }

    // Fade stunt notices
    for (auto it = stuntNotices.begin(); it != stuntNotices.end();) {
        it->timer -= dt;
        if (it->timer <= 0.0f) it = stuntNotices.erase(it);
        else ++it;
    }

    // Mouse Look (Free camera rotation)
    Vector2 mouseDelta = GetMouseDelta();
    float sensitivity = 0.0022f;
    yaw -= mouseDelta.x * sensitivity;
    pitch -= mouseDelta.y * sensitivity;
    pitch = std::clamp(pitch, -1.45f, 1.45f); // ~83 degrees limit

    // Track total forward distance run
    totalDistance = std::max(totalDistance, -position.z);

    // Jump Buffering and Coyote Time
    if (IsKeyPressed(KEY_SPACE)) {
        jumpBufferTimer = 0.18f;
    } else {
        jumpBufferTimer = std::max(0.0f, jumpBufferTimer - dt);
    }

    if (isGrounded) {
        coyoteTimer = 0.15f;
    } else {
        coyoteTimer = std::max(0.0f, coyoteTimer - dt);
    }

    // Input checks
    bool wantSlide = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    float strafeInput = 0.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) strafeInput -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) strafeInput += 1.0f;

    // State machine updates
    switch (state) {
        case MoveState::RUNNING: {
            targetEyeHeight = 1.65f;
            rollAngle = math::Approach(rollAngle, -strafeInput * 3.5f, dt * 15.0f);

            // Flow acceleration (only when not blocked by a wall)
            if (!isBlockedForward) {
                forwardMomentum = math::Approach(forwardMomentum, maxSprintSpeed, dt * 1.8f);
                flowTimer += dt;
                if (flowTimer > 3.0f) {
                    flowMultiplier = std::min(flowMultiplier + 0.25f, 3.0f);
                    flowTimer = 0.0f;
                }
            } else {
                forwardMomentum = math::Approach(forwardMomentum, baseSpeed * 0.4f, dt * 5.0f);
            }

            // Head Bob & Footsteps (only when moving and not face-planted in a wall)
            if (!isBlockedForward && isGrounded && forwardMomentum > 3.0f) {
                headBobTimer += dt * (forwardMomentum * 0.95f);
                headBobOffset = std::sin(headBobTimer * 2.0f) * 0.045f;
                if (std::sin(headBobTimer * 2.0f) < -0.90f && footstepCooldown <= 0.0f) {
                    audio.PlayFootstep();
                    footstepCooldown = 0.26f; // Prevent multi-frame spam
                }
            } else {
                headBobOffset = math::Approach(headBobOffset, 0.0f, dt * 10.0f);
            }

            // Slide trigger
            if (wantSlide && forwardMomentum > 8.0f) {
                state = MoveState::SLIDING;
                slideTimer = 0.85f;
                audio.PlaySlide();
                TriggerStunt("POWER SLIDE", 100, audio);
                break;
            }

            // Jump trigger
            if (jumpBufferTimer > 0.0f && coyoteTimer > 0.0f) {
                velocity.y = 10.8f;
                state = MoveState::AIRBORNE;
                jumpBufferTimer = 0.0f;
                coyoteTimer = 0.0f;
                audio.PlayJump();
                break;
            }

            // Vault check
            WorldObject ledge;
            Vector3 lookDir = { -std::sin(yaw), 0.0f, -std::cos(yaw) };
            if (jumpBufferTimer > 0.0f && world.CheckVaultCandidate(position, lookDir, ledge)) {
                state = MoveState::VAULTING;
                vaultTimer = 0.32f;
                vaultStart = position;
                vaultEnd = { position.x, ledge.box.max.y + 0.05f, ledge.box.min.z - 1.2f };
                jumpBufferTimer = 0.0f;
                audio.PlayJump();
                TriggerStunt("CLEAN VAULT", 180, audio);
                break;
            }

            if (!isGrounded) {
                state = MoveState::AIRBORNE;
            }
            break;
        }

        case MoveState::SLIDING: {
            targetEyeHeight = 0.70f; // Duck low to slide under pipes
            slideTimer -= dt;
            headBobOffset = 0.0f;
            rollAngle = math::Approach(rollAngle, 0.0f, dt * 10.0f);

            // Jump out of slide -> Slide kick boost!
            if (jumpBufferTimer > 0.0f) {
                velocity.y = 11.5f;
                forwardMomentum += 2.5f;
                state = MoveState::AIRBORNE;
                jumpBufferTimer = 0.0f;
                audio.PlayJump();
                TriggerStunt("SLIDE KICK", 200, audio);
                break;
            }

            if (slideTimer <= 0.0f || !wantSlide) {
                state = isGrounded ? MoveState::RUNNING : MoveState::AIRBORNE;
            }
            break;
        }

        case MoveState::AIRBORNE: {
            targetEyeHeight = 1.65f;
            headBobOffset = 0.0f;
            rollAngle = math::Approach(rollAngle, 0.0f, dt * 8.0f);

            // Check for Wall-running candidate
            WorldObject wall;
            BoundingBox pBox = GetBoundingBox(currentEyeHeight);
            if (velocity.y < 3.0f && world.CheckWallrunCandidate(pBox, velocity, wall)) {
                state = MoveState::WALL_RUNNING;
                wallrunTimer = 1.6f;
                wallrunIsLeft = wall.isWallrunLeft;
                wallrunNormal = wall.wallNormal;
                audio.PlayWallrunStart();
                TriggerStunt("WALL RUN", 250, audio);
                break;
            }
            break;
        }

        case MoveState::WALL_RUNNING: {
            targetEyeHeight = 1.65f;
            headBobOffset = 0.0f;
            wallrunTimer -= dt;

            // Camera tilt into wall
            float targetRoll = wallrunIsLeft ? -15.0f : 15.0f;
            rollAngle = math::Approach(rollAngle, targetRoll, dt * 20.0f);

            // Low gravity during wallrun
            velocity.y = math::Approach(velocity.y, -1.2f, dt * 6.0f);

            // Wall Kick Jump
            if (jumpBufferTimer > 0.0f) {
                Vector3 kickDir = wallrunNormal * 8.5f;
                velocity.x = kickDir.x;
                velocity.y = 11.2f;
                forwardMomentum += 2.0f;
                state = MoveState::AIRBORNE;
                jumpBufferTimer = 0.0f;
                audio.PlayJump();
                TriggerStunt("WALL KICK", 300, audio);
                break;
            }

            // Wall run time expired or fell off wall
            WorldObject wall;
            BoundingBox pBox = GetBoundingBox(currentEyeHeight);
            if (world.CheckWallrunCandidate(pBox, velocity, wall)) {
                if (wallrunIsLeft) {
                    position.x = wall.box.max.x + 0.42f;
                } else {
                    position.x = wall.box.min.x - 0.42f;
                }
            } else if (wallrunTimer <= 0.0f) {
                state = MoveState::AIRBORNE;
            }
            break;
        }

        case MoveState::VAULTING: {
            vaultTimer -= dt;
            float t = 1.0f - std::max(0.0f, vaultTimer) / 0.32f;
            position = math::Lerp(vaultStart, vaultEnd, t);
            velocity.y = 0.0f;
            if (vaultTimer <= 0.0f) {
                state = MoveState::RUNNING;
                forwardMomentum = std::max(forwardMomentum, 14.0f);
            }
            break;
        }

        case MoveState::ROLLING: {
            rollAnimTimer -= dt;
            float t = 1.0f - std::max(0.0f, rollAnimTimer) / 0.50f;

            // Parabolic tuck curve peaking around mid-roll (0.0 -> 1.0 -> 0.0)
            float tuck = std::sin(t * PI);

            // Eye height drops down to near pavement (0.35m tuck)
            targetEyeHeight = math::Lerp(1.65f, 0.35f, tuck);
            currentEyeHeight = targetEyeHeight;

            // Forward pitch tuck: camera pitches down forward looking at the ground
            pitchAnimOffset = -tuck * 1.15f; // ~ -66 degrees forward pitch

            // Natural shoulder roll tilt
            rollAngle = -tuck * 22.0f;

            headBobOffset = 0.0f;

            if (rollAnimTimer <= 0.0f) {
                rollAngle = 0.0f;
                pitchAnimOffset = 0.0f;
                targetEyeHeight = 1.65f;
                state = MoveState::RUNNING;
            }
            break;
        }

        default:
            break;
    }

    // Apply Physics & Kinematics
    if (state != MoveState::VAULTING) {
        float gravity = 25.0f;
        if (state != MoveState::WALL_RUNNING) {
            velocity.y -= gravity * dt;
        }

        // Forward and lateral speed
        float strafeSpeed = 9.0f;
        velocity.x = math::Approach(velocity.x, strafeInput * strafeSpeed, dt * 35.0f);
        velocity.z = -forwardMomentum;

        // Collision Resolution
        ResolveCollisions(position, velocity, currentEyeHeight, world, audio);
    }

    // Smooth Camera Eye Height
    if (state != MoveState::ROLLING) {
        currentEyeHeight = math::Approach(currentEyeHeight, targetEyeHeight, dt * 12.0f);
        pitchAnimOffset = math::Approach(pitchAnimOffset, 0.0f, dt * 15.0f);
    }

    // Dynamic FOV based on speed (drops to 0 if blocked against a wall)
    float effectiveSpeed = isBlockedForward ? 0.0f : forwardMomentum;
    float speedRatio = (effectiveSpeed - baseSpeed) / (maxSprintSpeed - baseSpeed);
    speedRatio = std::clamp(speedRatio, 0.0f, 1.0f);
    targetFOV = 75.0f + speedRatio * 20.0f; // 75 up to 95 degrees
    currentFOV = math::Approach(currentFOV, targetFOV, dt * 15.0f);

    // Update continuous audio
    audio.Update(speedRatio, state == MoveState::WALL_RUNNING, state == MoveState::SLIDING);

    // Void fall death check
    if (position.y < -35.0f) {
        state = MoveState::DEAD;
        audio.PlayGameOver();
    }
}

void Player::ResolveCollisions(Vector3& pos, Vector3& vel, float eyeH, WorldGenerator& world, AudioSystem& audio) {
    float dt = GetFrameTime();
    float radius = 0.40f;
    float playerH = eyeH + 0.15f;
    float floorEpsilon = 0.04f;

    isBlockedForward = false;

    // Check trigger objects (e.g. Springboard launchpad)
    BoundingBox triggerBox = {
        .min = { pos.x - radius, pos.y, pos.z - radius },
        .max = { pos.x + radius, pos.y + playerH, pos.z + radius }
    };
    for (const auto& obj : world.GetAllSolidObjects()) {
        if (obj.type == ObstacleType::SPRINGBOARD && CheckCollisionBoxes(triggerBox, obj.box)) {
            vel.y = 16.5f;
            forwardMomentum = std::min(forwardMomentum * 1.25f, maxSprintSpeed + 4.0f);
            state = MoveState::AIRBORNE;
            audio.PlaySpringboard();
            TriggerStunt("SPRINGBOARD LAUNCH", 400, audio);
            pos.y = obj.box.max.y + 0.1f;
            return;
        }
    }

    // --- STEP 1: VERTICAL (Y) COLLISION ---
    bool wasGroundedBefore = isGrounded;
    isGrounded = false;

    float desiredY = pos.y + vel.y * dt;
    BoundingBox boxY = {
        .min = { pos.x - radius + 0.05f, desiredY, pos.z - radius + 0.05f },
        .max = { pos.x + radius - 0.05f, desiredY + playerH, pos.z + radius - 0.05f }
    };

    for (const auto& obj : world.GetAllSolidObjects()) {
        if (obj.type == ObstacleType::WALLRUN_SURFACE) continue;
        if (!CheckCollisionBoxes(boxY, obj.box)) continue;

        // Landing on top surface
        if (vel.y <= 0.0f && pos.y >= obj.box.max.y - 0.45f) {
            desiredY = obj.box.max.y;
            vel.y = 0.0f;
            isGrounded = true;

            // Landing roll / stumble check
            if (!wasGroundedBefore) {
                bool wantRoll = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
                if (wantRoll) {
                    state = MoveState::ROLLING;
                    rollAnimTimer = 0.50f;
                    forwardMomentum += 2.0f;
                    audio.PlayRoll();
                    TriggerStunt("SKILL ROLL", 300, audio);
                } else if (vel.y < -17.0f) {
                    forwardMomentum = std::max(baseSpeed * 0.7f, 7.0f);
                    flowMultiplier = 1.0f;
                    impactShakeTimer = 0.25f;
                    audio.PlayLand(true);
                } else {
                    audio.PlayLand(false);
                }
            }

            if (state == MoveState::AIRBORNE) {
                state = MoveState::RUNNING;
            }
            break;
        }
        // Ceiling impact (head bump)
        else if (vel.y > 0.0f && (pos.y + playerH) <= obj.box.min.y + 0.35f) {
            desiredY = obj.box.min.y - playerH;
            vel.y = 0.0f;
            break;
        }
    }
    pos.y = desiredY;

    // --- STEP 2: LATERAL (X) WALL COLLISION (Left & Right Obstacles / Parapets) ---
    float desiredX = pos.x + vel.x * dt;
    BoundingBox boxX = {
        .min = { desiredX - radius, pos.y + floorEpsilon, pos.z - radius + 0.08f },
        .max = { desiredX + radius, pos.y + playerH - 0.08f, pos.z + radius - 0.08f }
    };

    for (const auto& obj : world.GetAllSolidObjects()) {
        if (state == MoveState::WALL_RUNNING && obj.type == ObstacleType::WALLRUN_SURFACE) continue;
        if (!CheckCollisionBoxes(boxX, obj.box)) continue;

        // Push player outside the wall on X axis
        if (vel.x > 0.0f) {
            desiredX = obj.box.min.x - radius - 0.002f;
            vel.x = 0.0f;
        } else if (vel.x < 0.0f) {
            desiredX = obj.box.max.x + radius + 0.002f;
            vel.x = 0.0f;
        } else {
            float toMin = std::abs((desiredX + radius) - obj.box.min.x);
            float toMax = std::abs((desiredX - radius) - obj.box.max.x);
            if (toMin < toMax) {
                desiredX = obj.box.min.x - radius - 0.002f;
            } else {
                desiredX = obj.box.max.x + radius + 0.002f;
            }
        }
    }
    pos.x = desiredX;

    // --- STEP 3: FORWARD / BACKWARD (Z) WALL COLLISION (Head-on Obstacles & Building Faces) ---
    float desiredZ = pos.z + vel.z * dt;
    BoundingBox boxZ = {
        .min = { pos.x - radius + 0.08f, pos.y + floorEpsilon, desiredZ - radius },
        .max = { pos.x + radius - 0.08f, pos.y + playerH - 0.08f, desiredZ + radius }
    };

    for (const auto& obj : world.GetAllSolidObjects()) {
        if (!CheckCollisionBoxes(boxZ, obj.box)) continue;

        // Low Slide Pipe check:
        if (obj.type == ObstacleType::SLIDE_PIPE) {
            if (state == MoveState::SLIDING) {
                // Sliding low underneath the pipe!
                continue;
            }
        }

        // Running forward into a wall face (vel.z is negative)
        if (vel.z < 0.0f) {
            desiredZ = obj.box.max.z + radius + 0.002f;
            vel.z = 0.0f;
            isBlockedForward = true;

            // Physical Head-on Wall Impact (debounced: plays only on initial contact)
            if (wallImpactCooldown <= 0.0f) {
                audio.PlayLand(true);
                impactShakeTimer = 0.18f;
                wallImpactCooldown = 0.40f;
            }

            forwardMomentum = std::max(baseSpeed * 0.25f, 3.0f);
            flowMultiplier = 1.0f;
            break;
        } else if (vel.z > 0.0f) {
            desiredZ = obj.box.min.z - radius - 0.002f;
            vel.z = 0.0f;
            break;
        }
    }
    pos.z = desiredZ;
}

Camera3D Player::GetCamera() const {
    Vector3 eyePos = {
        position.x,
        position.y + currentEyeHeight + headBobOffset,
        position.z
    };

    // Calculate view target from yaw & pitch (including forward roll tuck)
    float effectivePitch = std::clamp(pitch + pitchAnimOffset, -1.48f, 1.48f);
    Vector3 forward = {
        -std::sin(yaw) * std::cos(effectivePitch),
        std::sin(effectivePitch),
        -std::cos(yaw) * std::cos(effectivePitch)
    };

    Vector3 target = eyePos + forward;

    // Up vector with roll angle
    float rollRad = rollAngle * DEG2RAD;
    Vector3 right = { std::cos(yaw), 0.0f, -std::sin(yaw) };
    Vector3 up = { -std::sin(rollRad) * right.x, std::cos(rollRad), -std::sin(rollRad) * right.z };

    // Visceral impact camera shake when bumping walls
    Vector3 shake = { 0.0f, 0.0f, 0.0f };
    if (impactShakeTimer > 0.0f) {
        float shakeMag = (impactShakeTimer / 0.22f) * 0.10f;
        shake.x = ((float)(rand() % 100) / 50.0f - 1.0f) * shakeMag;
        shake.y = ((float)(rand() % 100) / 50.0f - 1.0f) * shakeMag;
    }

    Camera3D camera = { 0 };
    camera.position = eyePos + shake;
    camera.target = target + shake;
    camera.up = up;
    camera.fovy = currentFOV;
    camera.projection = CAMERA_PERSPECTIVE;

    return camera;
}

void Player::Draw() {
    // First person camera view
}
