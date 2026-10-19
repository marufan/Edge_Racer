#include "HUD.hpp"
#include <format>
#include <cmath>
#include <algorithm>

HUD::HUD() = default;

void HUD::DrawSpeedLines(float speedRatio, int screenWidth, int screenHeight) {
    if (speedRatio < 0.65f) return;

    float intensity = (speedRatio - 0.65f) / 0.35f;
    int lineCount = static_cast<int>(18 * intensity);
    Vector2 center = { screenWidth * 0.5f, screenHeight * 0.5f };

    for (int i = 0; i < lineCount; ++i) {
        float angle = (float)(i * 360 / lineCount) * DEG2RAD;
        float innerDist = screenWidth * 0.35f;
        float outerDist = screenWidth * 0.55f;

        Vector2 start = {
            center.x + std::cos(angle) * innerDist,
            center.y + std::sin(angle) * innerDist
        };
        Vector2 end = {
            center.x + std::cos(angle) * outerDist,
            center.y + std::sin(angle) * outerDist
        };

        Color lineCol = { 255, 255, 255, static_cast<unsigned char>(110 * intensity) };
        DrawLineEx(start, end, 2.0f, lineCol);
    }
}

void HUD::Draw(const Player& player, int screenWidth, int screenHeight, int highScore) {
    (void)highScore;

    // Center Crosshair Dot (Mirror's Edge Motion Sickness prevention dot)
    DrawCircle(screenWidth / 2, screenHeight / 2, 3, COLOR_HUD_RED);
    DrawCircleLines(screenWidth / 2, screenHeight / 2, 4, COLOR_HUD_WHITE);

    // Top-Left: Digital Speedometer & Distance
    float speedKmh = player.GetSpeed() * 3.6f;
    float distMeters = player.GetDistance();

    DrawRectangle(25, 25, 250, 90, COLOR_HUD_BG);
    DrawRectangle(25, 25, 4, 90, COLOR_HUD_RED); // Red accent border

    std::string speedStr = std::format("{:04.1f} KM/H", speedKmh);
    DrawText(speedStr.c_str(), 40, 36, 28, COLOR_HUD_WHITE);

    std::string distStr = std::format("DIST: {:05.0f} M", distMeters);
    DrawText(distStr.c_str(), 40, 72, 18, { 200, 205, 215, 255 });

    // Speed bar meter
    float speedRatio = (player.GetSpeed() - 12.0f) / (player.GetMaxSpeed() - 12.0f);
    speedRatio = std::clamp(speedRatio, 0.0f, 1.0f);

    DrawRectangle(40, 96, 220, 6, { 60, 65, 75, 255 });
    DrawRectangle(40, 96, static_cast<int>(220 * speedRatio), 6, COLOR_HUD_RED);

    // Speed lines effect
    DrawSpeedLines(speedRatio, screenWidth, screenHeight);

    // Top-Right: Score & FLOW Multiplier
    DrawRectangle(screenWidth - 275, 25, 250, 90, COLOR_HUD_BG);
    DrawRectangle(screenWidth - 29, 25, 4, 90, COLOR_HUD_ACCENT);

    std::string scoreStr = std::format("PTS: {:06d}", player.GetScore());
    DrawText(scoreStr.c_str(), screenWidth - 260, 36, 24, COLOR_HUD_WHITE);

    float flow = player.GetFlowMultiplier();
    std::string flowStr = std::format("FLOW: {:.1f}x", flow);
    Color flowColor = (flow >= 2.5f) ? COLOR_HUD_RED : COLOR_HUD_ACCENT;
    DrawText(flowStr.c_str(), screenWidth - 260, 68, 20, flowColor);

    // Current State Indicator Tag
    std::string stateStr = "SPRINTING";
    Color stateCol = COLOR_HUD_WHITE;
    switch (player.GetState()) {
        case MoveState::SLIDING: stateStr = "SLIDING"; stateCol = COLOR_HUD_ACCENT; break;
        case MoveState::WALL_RUNNING: stateStr = "WALL RUN"; stateCol = COLOR_HUD_RED; break;
        case MoveState::AIRBORNE: stateStr = "AIRBORNE"; stateCol = { 100, 200, 255, 255 }; break;
        case MoveState::VAULTING: stateStr = "VAULT"; stateCol = COLOR_HUD_RED; break;
        case MoveState::ROLLING: stateStr = "SKILL ROLL"; stateCol = COLOR_HUD_RED; break;
        default: break;
    }
    DrawText(stateStr.c_str(), screenWidth - 260, 92, 14, stateCol);

    // Right Side: Stunt Combo Feed Popups
    int noticeY = 140;
    for (const auto& notice : player.GetStuntNotices()) {
        float alpha = std::clamp(notice.timer / 1.8f, 0.0f, 1.0f);
        unsigned char aByte = static_cast<unsigned char>(alpha * 255);

        DrawRectangle(screenWidth - 280, noticeY, 255, 36, { 15, 20, 28, static_cast<unsigned char>(180 * alpha) });
        DrawRectangle(screenWidth - 280, noticeY, 4, 36, { 245, 25, 45, aByte });

        std::string stuntText = std::format("{} +{}", notice.text, notice.points);
        DrawText(stuntText.c_str(), screenWidth - 265, noticeY + 8, 18, { 255, 255, 255, aByte });
        noticeY += 44;
    }

    DrawControlsHint(screenWidth, screenHeight);
}

void HUD::DrawControlsHint(int screenWidth, int screenHeight) {
    const char* hint = "[A/D] Strafe | [SPACE] Jump/Vault/WallKick | [CTRL/C/RMB] Slide & SkillRoll";
    int textW = MeasureText(hint, 16);
    DrawRectangle(screenWidth / 2 - textW / 2 - 15, screenHeight - 40, textW + 30, 28, { 15, 20, 28, 160 });
    DrawText(hint, screenWidth / 2 - textW / 2, screenHeight - 34, 16, { 220, 225, 235, 220 });
}

void HUD::DrawGameOver(const Player& player, int screenWidth, int screenHeight, int highScore) {
    // Dark blur overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, { 10, 12, 18, 220 });

    int panelW = 540;
    int panelH = 360;
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2;

    DrawRectangle(panelX, panelY, panelW, panelH, { 22, 26, 36, 255 });
    DrawRectangle(panelX, panelY, panelW, 8, COLOR_HUD_RED);
    DrawRectangleLines(panelX, panelY, panelW, panelH, { 60, 68, 85, 255 });

    DrawText("RUN TERMINATED", panelX + 35, panelY + 35, 36, COLOR_HUD_RED);

    std::string distText = std::format("DISTANCE SURVIVED:  {:.0f} METERS", player.GetDistance());
    DrawText(distText.c_str(), panelX + 35, panelY + 105, 22, COLOR_HUD_WHITE);

    std::string scoreText = std::format("FINAL FLOW SCORE:   {} PTS", player.GetScore());
    DrawText(scoreText.c_str(), panelX + 35, panelY + 145, 22, COLOR_HUD_ACCENT);

    std::string bestText = std::format("HIGH SCORE RECORD:  {} PTS", highScore);
    DrawText(bestText.c_str(), panelX + 35, panelY + 185, 22, { 160, 170, 185, 255 });

    // Restart prompt with pulsing color
    float pulse = (std::sin((float)GetTime() * 6.0f) + 1.0f) * 0.5f;
    Color promptCol = { 255, static_cast<unsigned char>(100 + 155 * pulse), static_cast<unsigned char>(100 + 155 * pulse), 255 };

    DrawText("PRESS [SPACE] OR [R] TO QUICK RESTART", panelX + 35, panelY + 270, 20, promptCol);
    DrawText("PRESS [ESC] TO QUIT", panelX + 35, panelY + 305, 16, { 120, 130, 145, 255 });
}
