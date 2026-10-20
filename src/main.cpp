#include "raylib.h"
#include "core/AudioSystem.hpp"
#include "render/ShadowSystem.hpp"
#include "world/WorldGenerator.hpp"
#include "game/Player.hpp"
#include "ui/HUD.hpp"
#include <algorithm>

enum class GameState {
    INTRO,
    PLAYING,
    GAME_OVER,
    PAUSE
};

int main() {
    const int initialWidth = 1280;
    const int initialHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(initialWidth, initialHeight, "EDGE RACER - Parkour Endless Runner");
    SetTargetFPS(60);

    // Audio system initialization
    AudioSystem audio;
    audio.Init();

    // Shadow and Lighting system
    ShadowSystem shadows;
    shadows.Init();

    // Game systems
    WorldGenerator world;
    Player player;
    HUD hud;

    int highScore = 0;
    GameState gameState = GameState::PLAYING;

    // Lock and hide mouse cursor for first-person control
    DisableCursor();

    // Visual atmosphere colors
    const Color SKY_COLOR = { 145, 205, 255, 255 }; // Brilliant azure runner sky

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        // Prevent huge delta spikes on hitching
        dt = std::min(dt, 0.05f);

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        // Game State Logic
        if (gameState == GameState::PLAYING) {
            player.Update(dt, world, audio);
            world.Update(player.GetPosition().z);

            if (player.IsDead()) {
                gameState = GameState::GAME_OVER;
                EnableCursor();
                highScore = std::max(highScore, player.GetScore());
            }

            // Quick restart button
            if (IsKeyPressed(KEY_R)) {
                player.Reset({ 0.0f, 2.0f, 5.0f });
                world.Reset();
            }

            if (IsKeyPressed(KEY_P)) {
                gameState = GameState::PAUSE;
            }
        } else if (gameState == GameState::GAME_OVER) {
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_R)) {
                player.Reset({ 0.0f, 2.0f, 5.0f });
                world.Reset();
                gameState = GameState::PLAYING;
                DisableCursor();
            }
        } else if (gameState == GameState::PAUSE) {
            if (IsKeyPressed(KEY_P)) {
                gameState = GameState::PLAYING;
            }
        }

        // Render Pipeline
        BeginDrawing();
        ClearBackground(SKY_COLOR);

        // 3D Scene
        Camera3D camera = player.GetCamera();
        BeginMode3D(camera);

        // Architectural directional lighting & cast shadows
        shadows.BeginLighting(camera);
        world.Draw(player.GetPosition(), shadows);
        player.Draw();

        EndMode3D();

        // 2D HUD / Post Overlay
        if (gameState == GameState::PLAYING) {
            hud.Draw(player, screenWidth, screenHeight, highScore);
        } else if (gameState == GameState::GAME_OVER) {
            hud.DrawGameOver(player, screenWidth, screenHeight, highScore);
        }

        EndDrawing();
    }

    shadows.Close();
    audio.Close();
    CloseWindow();
    return 0;
}
