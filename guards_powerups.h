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

inline float GetClampedFrameDelta() {
    float dt = GetFrameTime();
    if (dt > 0.033f) dt = 0.033f;
    return dt;
}

inline bool IsPassableGuardTile(char ch) {
    return (ch == '.' || ch == ' ' || ch == 'S' || ch == 'P');
}

inline bool IsSafeOpenForGuard(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int r, int c) {
    if (r <= 1 || r >= mapRows - 2 || c <= 6 || c >= mapCols - 4) return false;
    if (!IsPassableGuardTile(mapData[r][c])) return false;
    bool topOpen = IsPassableGuardTile(mapData[r - 1][c]);
    bool botOpen = IsPassableGuardTile(mapData[r + 1][c]);
    if (!topOpen || !botOpen) return false;
    bool leftOpen = IsPassableGuardTile(mapData[r][c - 1]) && IsPassableGuardTile(mapData[r][c - 2]);
    bool rightOpen = IsPassableGuardTile(mapData[r][c + 1]) && IsPassableGuardTile(mapData[r][c + 2]);
    if (!leftOpen || !rightOpen) return false;
    int openNeighbors = (topOpen ? 1 : 0) + (botOpen ? 1 : 0) +
        (IsPassableGuardTile(mapData[r][c - 1]) ? 1 : 0) +
        (IsPassableGuardTile(mapData[r][c + 1]) ? 1 : 0);
    return (openNeighbors >= 3);
}

inline void InitializeGuards(Guard guards[], int& guardCount, int& activeGuardCount, char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, float playerX, float playerY, int difficultyStep) {
    guardCount = 0;
    float baseSpeed = 1.90f + (difficultyStep > 6 ? 6 : difficultyStep) * 0.11f;
    int playerCol = (int)((playerX - MAP_OFFSET_X) / TILE_SIZE);
    int startSearchCol = playerCol + 16;
    if (startSearchCol < 16) startSearchCol = 16;
    int midRow = mapRows / 2;

    for (int bandPass = 0; bandPass < 2 && guardCount < MAX_GUARDS; bandPass++) {
        for (int c = startSearchCol; c < mapCols - 4 && guardCount < MAX_GUARDS; c += 5) {
            for (int r = 1; r < mapRows - 1 && guardCount < MAX_GUARDS; r++) {
                bool isLowerOrMid = (r >= midRow - 1);
                if ((bandPass == 0 && !isLowerOrMid) || (bandPass == 1 && isLowerOrMid)) continue;
                if (IsSafeOpenForGuard(mapData, mapRows, mapCols, r, c)) {
                    float sx = MAP_OFFSET_X + c * TILE_SIZE + 8.0f, sy = MAP_OFFSET_Y + r * TILE_SIZE + 8.0f;
                    float dx = sx - playerX, dy = sy - playerY;
                    bool spaced = true;
                    for (int k = 0; k < guardCount; k++) {
                        if (sqrtf((sx - guards[k].x) * (sx - guards[k].x) + (sy - guards[k].y) * (sy - guards[k].y)) < 380.0f) { spaced = false; break; }
                    }
                    if (sqrtf(dx * dx + dy * dy) >= 560.0f && spaced) {
                        guards[guardCount] = { sx, sy, sx - 64.0f, sx + 64.0f, (guardCount % 2 == 0) ? baseSpeed : -baseSpeed, 0, 0.0f, false, 2, 1, 0.0f, 0.0f };
                        guardCount++;
                        break;
                    }
                }
            }
        }
    }
    activeGuardCount = (guardCount < INITIAL_GUARDS) ? guardCount : INITIAL_GUARDS;
}

inline void ShiftGuardsWorldX(Guard guards[], int guardCount, float shiftX) {
    for (int i = 0; i < guardCount; i++) { guards[i].x -= shiftX; guards[i].startX -= shiftX; guards[i].endX -= shiftX; }
}

inline void InitializePowerUps(char mapData[MAX_ROWS][MAX_COLS], int rows, int cols, PowerUp powerUps[], int& powerUpCount) {
    powerUpCount = 0;
    int seenP = 0;
    for (int i = 0; i < rows; i++) for (int j = 0; j < cols; j++) {
        if (mapData[i][j] == 'P' && powerUpCount < MAX_POWERUPS) {
            seenP++;
            if ((seenP % 3 != 1) && (rand() % 100 < 55)) continue;
            int rType = rand() % 10;
            powerUps[powerUpCount] = { MAP_OFFSET_X + j * TILE_SIZE + 8.0f, MAP_OFFSET_Y + i * TILE_SIZE + 8.0f, (rType < 4) ? 1 : ((rType < 7) ? 2 : 3), true };
            powerUpCount++;
        }
    }
}

inline void ShiftPowerUpsWorldX(PowerUp powerUps[], int powerUpCount, float shiftX) {
    for (int i = 0; i < powerUpCount; i++) powerUps[i].x -= shiftX;
}

inline void UpdatePowerUps(Player& player, PowerUp powerUps[], int powerUpCount, Guard guards[], int activeGuardCount) {
    float dt = GetClampedFrameDelta();
    if (player.isSpeedBoosted) { player.speedBoostTimer -= dt; if (player.speedBoostTimer <= 0.0f) player.isSpeedBoosted = false; }
    if (player.hammerTimer > 0.0f) { player.hammerTimer -= dt; if (player.hammerTimer < 0.0f) player.hammerTimer = 0.0f; }
    Rectangle pRect = { player.x, player.y, 26.0f, 26.0f };
    for (int i = 0; i < powerUpCount; i++) {
        if (powerUps[i].isActive && CheckCollisionRecs(pRect, { powerUps[i].x - 2.0f, powerUps[i].y - 2.0f, 28.0f, 28.0f })) {
            powerUps[i].isActive = false;
            if (powerUps[i].type == 1) { player.isSpeedBoosted = true; player.speedBoostTimer = 4.2f; }
            else if (powerUps[i].type == 2) { for (int g = 0; g < activeGuardCount; g++) if (!guards[g].isDefeated) guards[g].stunTimer = 3.2f; }
            else if (powerUps[i].type == 3) { player.hammerTimer = 6.5f; }
        }
    }
}

inline void UpdateGuards(Guard guards[], int activeGuardCount, Player& player, GameState& currentState, char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int difficultyStep) {
    float dt = GetClampedFrameDelta();
    float frameStep = dt * 60.0f;
    Rectangle pRect = { player.x, player.y, 26.0f, 26.0f };
    float chaseSpeed = (1.80f + (difficultyStep > 6 ? 6 : difficultyStep) * 0.10f) * frameStep;
    for (int i = 0; i < activeGuardCount; i++) {
        if (guards[i].isDefeated) continue;
        if (guards[i].state == 2) {
            guards[i].grabTimer -= dt;
            if (guards[i].grabTimer <= 0.0f) { currentState = GAME_OVER; return; }
            continue;
        }
        if (guards[i].stunTimer > 0.0f) { guards[i].stunTimer -= dt; guards[i].animFrame = 1; continue; }
        if (CheckCollisionRecs(pRect, { guards[i].x, guards[i].y, 24.0f, 24.0f })) {
            if (player.hammerTimer > 0.0f) { guards[i].isDefeated = true; player.hammerSwingTimer = 0.25f; player.score += HAMMER_KILL_EXP; continue; }
            guards[i].state = 2; guards[i].grabTimer = 0.45f; continue;
        }
        float oldX = guards[i].x, oldY = guards[i].y;
        if (guards[i].state == 0) {
            guards[i].x += guards[i].speed * frameStep;
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