#pragma once
#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include <cstdlib>
#include <cmath>

const int MAX_GUARDS = 8;
const int MAX_POWERUPS = 14;
const int INITIAL_GUARDS = 3;
const int MAX_ACTIVE_GUARDS = 5;
const int HAMMER_KILL_EXP = 35;

inline void InitializeGuards(Guard guards[], int& guardCount, int& activeGuardCount, char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, float playerX, float playerY, int difficultyStep) {
    (void)playerX; (void)playerY;
    guardCount = 0;
    float spd = 1.8f + difficultyStep * 0.1f;
    for (int c = 18; c < mapCols - 4 && guardCount < MAX_GUARDS; c += 12) {
        int r = 3 + (guardCount % (mapRows - 5));
        if (mapData[r][c] == '.') {
            guards[guardCount] = { MAP_OFFSET_X + c * TILE_SIZE + 8.0f, MAP_OFFSET_Y + r * TILE_SIZE + 8.0f, 0, 0, spd, 0, 0.0f, false, 2, 1, 0.0f, 0.0f };
            guards[guardCount].startX = guards[guardCount].x - 60.0f;
            guards[guardCount].endX = guards[guardCount].x + 60.0f;
            guardCount++;
        }
    }
    activeGuardCount = (guardCount < INITIAL_GUARDS) ? guardCount : INITIAL_GUARDS;
}

inline void InitializePowerUps(char mapData[MAX_ROWS][MAX_COLS], int rows, int cols, PowerUp powerUps[], int& powerUpCount) {
    powerUpCount = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (mapData[i][j] == 'P' && powerUpCount < MAX_POWERUPS) {
                powerUps[powerUpCount] = { MAP_OFFSET_X + j * TILE_SIZE + 8.0f, MAP_OFFSET_Y + i * TILE_SIZE + 8.0f, (powerUpCount % 3) + 1, true };
                powerUpCount++;
            }
        }
    }
}

inline void UpdatePowerUps(Player& player, PowerUp powerUps[], int powerUpCount, Guard guards[], int activeGuardCount) {
    float dt = GetFrameTime();
    if (player.isSpeedBoosted) { player.speedBoostTimer -= dt; if (player.speedBoostTimer <= 0.0f) player.isSpeedBoosted = false; }
    if (player.hammerTimer > 0.0f) { player.hammerTimer -= dt; if (player.hammerTimer < 0.0f) player.hammerTimer = 0.0f; }

    Rectangle pRect = { player.x, player.y, 26.0f, 26.0f };
    for (int i = 0; i < powerUpCount; i++) {
        if (powerUps[i].isActive && CheckCollisionRecs(pRect, { powerUps[i].x, powerUps[i].y, 26.0f, 26.0f })) {
            powerUps[i].isActive = false;
            if (powerUps[i].type == 1) { player.isSpeedBoosted = true; player.speedBoostTimer = 4.2f; }
            else if (powerUps[i].type == 2) { for (int g = 0; g < activeGuardCount; g++) if (!guards[g].isDefeated) guards[g].stunTimer = 3.2f; }
            else if (powerUps[i].type == 3) { player.hammerTimer = 6.5f; }
        }
    }
}

inline void UpdateGuards(Guard guards[], int activeGuardCount, Player& player, GameState& currentState, char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int difficultyStep) {
    (void)mapData; (void)mapRows; (void)mapCols; (void)difficultyStep;
    float dt = GetFrameTime();
    Rectangle pRect = { player.x, player.y, 26.0f, 26.0f };
    for (int i = 0; i < activeGuardCount; i++) {
        if (guards[i].isDefeated) continue;
        if (guards[i].stunTimer > 0.0f) { guards[i].stunTimer -= dt; continue; }
        if (CheckCollisionRecs(pRect, { guards[i].x, guards[i].y, 24.0f, 24.0f })) {
            if (player.hammerTimer > 0.0f) { guards[i].isDefeated = true; player.score += HAMMER_KILL_EXP; continue; }
            currentState = GAME_OVER;
            return;
        }
        float dx = player.x - guards[i].x, dy = player.y - guards[i].y;
        if (sqrtf(dx * dx + dy * dy) < 155.0f) guards[i].state = 1;
        if (guards[i].state == 0) {
            guards[i].x += guards[i].speed;
            if (guards[i].x >= guards[i].endX || guards[i].x <= guards[i].startX) guards[i].speed = -guards[i].speed;
        }
        else {
            if (guards[i].x < player.x) guards[i].x += 1.8f; else if (guards[i].x > player.x) guards[i].x -= 1.8f;
            if (guards[i].y < player.y) guards[i].y += 1.8f; else if (guards[i].y > player.y) guards[i].y -= 1.8f;
        }
    }
}