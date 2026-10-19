#pragma once

#include "raylib.h"
#include "game/Player.hpp"

class HUD {
public:
    HUD();

    void Draw(const Player& player, int screenWidth, int screenHeight, int highScore);
    void DrawSpeedLines(float speedRatio, int screenWidth, int screenHeight);
    void DrawGameOver(const Player& player, int screenWidth, int screenHeight, int highScore);
    void DrawControlsHint(int screenWidth, int screenHeight);

private:
    const Color COLOR_HUD_RED = { 245, 25, 45, 255 };
    const Color COLOR_HUD_WHITE = { 250, 250, 250, 255 };
    const Color COLOR_HUD_BG = { 15, 20, 28, 190 };
    const Color COLOR_HUD_ACCENT = { 255, 195, 0, 255 };
};
