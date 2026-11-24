#pragma once
#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include <cstdlib>
#include <cmath>

const int MAX_GUARDS = 10;
const int MAX_POWERUPS = 16;
const int INITIAL_GUARDS = 3;
const int MAX_ACTIVE_GUARDS = 5;
const int HAMMER_KILL_EXP = 35;

inline void InitializeGuards(Guard guards[], int& guardCount, int& activeGuardCount, char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, float playerX, float playerY, int difficultyStep) {
    (void)playerX; (void)playerY;
    guardCount = 0;
    float baseSpeed = 1.90f + (difficultyStep > 6 ? 6 : difficultyStep) * 0.11f;
    for (int c = 18; c < mapCols - 4 && guardCount < MAX_GUARDS; c += 10) {
        for (int r = 2; r < mapRows - 2 && guardCount < MAX_GUARDS; r++) {
            if (mapData[r][c] == '.') {
                float sx = MAP_OFFSET_X + c * TILE_SIZE + 8.0f;
                float sy = MAP_OFFSET_Y + r * TILE_SIZE + 8.0f;
                guards[guardCount] = { sx, sy, sx - 64.0f, sx + 64.0f, (guardCount % 2 == 0) ? baseSpeed : -baseSpeed, 0, 0.0f, false, 2, 1, 0.0f, 0.0f };
                guardCount++;
                break;
            }
        }
    }
    activeGuardCount = (guardCount < INITIAL_GUARDS) ? guardCount : INITIAL_GUARDS;
}

inline void InitializePowerUps(char mapData[MAX_ROWS][MAX_COLS], int rows, int cols, PowerUp powerUps[], int& powerUpCount) {
    powerUpCount = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (mapData[i][j] == 'P' && powerUpCount < MAX_POWERUPS) {
                powerUps[powerUpCount] = { MAP_OFFSET_X + j * TILE_SIZE + 8.0f, MAP_OFFSET_Y + i * TILE_SIZE + 8.0f, (rand() % 3) + 1, true };
                powerUpCount++;
            }
        }
    }
}

inline void UpdatePowerUps(Player& player, PowerUp powerUps[], int powerUpCount, Guard guards[], int activeGuardCount) {
    float dt = GetFrameTime();
    if (player.isSpeedBoosted) { player.speedBoostTimer -= dt; if (player.speedBoostTimer <= 0.0f) player.isSpeedBoosted = false; }
    if (player.hammerTimer > 0.0f) { player.hammerTimer -= dt; if (player.hammerTimer < 0.0f) player.hammerTimer = 0.0f; }
    Rectangle playerRect = { player.x, player.y, 26.0f, 26.0f };
    for (int i = 0; i < powerUpCount; i++) {
        if (powerUps[i].isActive && CheckCollisionRecs(playerRect, { powerUps[i].x - 2.0f, powerUps[i].y - 2.0f, 28.0f, 28.0f })) {
            powerUps[i].isActive = false;
            if (powerUps[i].type == 1) { player.isSpeedBoosted = true; player.speedBoostTimer = 4.2f; }
            else if (powerUps[i].type == 2) { for (int g = 0; g < activeGuardCount; g++) if (!guards[g].isDefeated) guards[g].stunTimer = 3.2f; }
            else if (powerUps[i].type == 3) { player.hammerTimer = 6.5f; }
        }
    }
}

inline void UpdateGuards(Guard guards[], int activeGuardCount, Player& player, GameState& currentState, char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int difficultyStep) {
    float dt = GetFrameTime();
    Rectangle playerRect = { player.x, player.y, 26.0f, 26.0f };
    float chaseSpeed = 1.80f + (difficultyStep > 6 ? 6 : difficultyStep) * 0.10f;
    for (int i = 0; i < activeGuardCount; i++) {
        if (guards[i].isDefeated) continue;
        if (guards[i].state == 2) {
            guards[i].grabTimer -= dt;
            if (guards[i].grabTimer <= 0.0f) { currentState = GAME_OVER; return; }
            continue;
        }
        if (guards[i].stunTimer > 0.0f) { guards[i].stunTimer -= dt; guards[i].animFrame = 1; continue; }
        if (CheckCollisionRecs(playerRect, { guards[i].x, guards[i].y, 24.0f, 24.0f })) {
            if (player.hammerTimer > 0.0f) { guards[i].isDefeated = true; player.hammerSwingTimer = 0.25f; player.score += HAMMER_KILL_EXP; continue; }
            guards[i].state = 2; guards[i].grabTimer = 0.45f; continue;
        }
        float oldX = guards[i].x, oldY = guards[i].y;
        if (guards[i].state == 0) {
            guards[i].x += guards[i].speed;
            if (guards[i].x >= guards[i].endX || guards[i].x <= guards[i].startX) guards[i].speed = -guards[i].speed;
            float dx = player.x - guards[i].x, dy = player.y - guards[i].y;
            if (sqrtf(dx * dx + dy * dy) < 155.0f) guards[i].state = 1;
        }
        else {
            if (guards[i].x < player.x) guards[i].x += chaseSpeed; else if (guards[i].x > player.x) guards[i].x -= chaseSpeed;
            if (guards[i].y < player.y) guards[i].y += chaseSpeed; else if (guards[i].y > player.y) guards[i].y -= chaseSpeed;
        }
        for (int r = 0; r < mapRows; r++) for (int c = 0; c < mapCols; c++) {
            if (mapData[r][c] == '#' || mapData[r][c] == 'W' || mapData[r][c] == 'T') {
                Rectangle w = { MAP_OFFSET_X + c * TILE_SIZE, MAP_OFFSET_Y + r * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                if (CheckCollisionRecs({ guards[i].x, guards[i].y, 24.0f, 24.0f }, w)) { guards[i].x = oldX; guards[i].y = oldY; }
            }
        }
    }
}