#pragma once
#include "raylib.h"
#include "structs.h"
#include "lockers.h"

const int MAX_GUARDS = 6;
const int MAX_POWERUPS = 10;
const int INITIAL_GUARDS = 2;
const int MAX_ACTIVE_GUARDS = 4;
const int HAMMER_KILL_EXP = 35;

inline void InitializeGuards(Guard guards[], int& guardCount, int& activeGuardCount, char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, float playerX, float playerY, int difficultyStep) {
    (void)mapData; (void)mapRows; (void)mapCols; (void)playerX; (void)playerY; (void)difficultyStep;
    guardCount = 2;
    activeGuardCount = 2;
    for (int i = 0; i < guardCount; i++) {
        guards[i].x = MAP_OFFSET_X + (float)((20 + i * 12) * TILE_SIZE);
        guards[i].y = MAP_OFFSET_Y + (float)(4 * TILE_SIZE);
        guards[i].startX = guards[i].x - 50.0f;
        guards[i].endX = guards[i].x + 50.0f;
        guards[i].speed = 1.5f;
        guards[i].state = 0;
        guards[i].stunTimer = 0.0f;
        guards[i].isDefeated = false;
        guards[i].dirRow = 2;
        guards[i].animFrame = 1;
        guards[i].animTimer = 0.0f;
        guards[i].grabTimer = 0.0f;
    }
}

inline void InitializePowerUps(char mapData[MAX_ROWS][MAX_COLS], int rows, int cols, PowerUp powerUps[], int& powerUpCount) {
    powerUpCount = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (mapData[i][j] == 'P' && powerUpCount < MAX_POWERUPS) {
                powerUps[powerUpCount] = { MAP_OFFSET_X + j * TILE_SIZE + 8.0f, MAP_OFFSET_Y + i * TILE_SIZE + 8.0f, 1, true };
                powerUpCount++;
            }
        }
    }
}

inline void UpdatePowerUps(Player& player, PowerUp powerUps[], int powerUpCount, Guard guards[], int activeGuardCount) {
    (void)guards; (void)activeGuardCount;
    Rectangle pRect = { player.x, player.y, 26.0f, 26.0f };
    for (int i = 0; i < powerUpCount; i++) {
        if (powerUps[i].isActive && CheckCollisionRecs(pRect, { powerUps[i].x, powerUps[i].y, 24.0f, 24.0f })) {
            powerUps[i].isActive = false;
            player.isSpeedBoosted = true;
            player.speedBoostTimer = 4.0f;
        }
    }
}

inline void UpdateGuards(Guard guards[], int activeGuardCount, Player& player, GameState& currentState, char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int difficultyStep) {
    (void)mapData; (void)mapRows; (void)mapCols; (void)difficultyStep;
    Rectangle pRect = { player.x, player.y, 26.0f, 26.0f };
    for (int i = 0; i < activeGuardCount; i++) {
        if (guards[i].isDefeated) continue;
        guards[i].x += guards[i].speed;
        if (guards[i].x > guards[i].endX || guards[i].x < guards[i].startX) guards[i].speed = -guards[i].speed;
        if (CheckCollisionRecs(pRect, { guards[i].x, guards[i].y, 24.0f, 24.0f })) currentState = GAME_OVER;
    }
}