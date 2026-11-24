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

// Ensures a guard tile has vertical & horizontal breathing room so no corridor chokehold forms
inline bool IsSafeOpenForGuard(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int r, int c) {
    if (r <= 1 || r >= mapRows - 2 || c <= 6 || c >= mapCols - 4) return false;
    if (!IsPassableGuardTile(mapData[r][c])) return false;

    bool topOpen = IsPassableGuardTile(mapData[r - 1][c]);
    bool botOpen = IsPassableGuardTile(mapData[r + 1][c]);
    if (!topOpen || !botOpen) return false;

    bool leftOpen = IsPassableGuardTile(mapData[r][c - 1]) && IsPassableGuardTile(mapData[r][c - 2]);
    bool rightOpen = IsPassableGuardTile(mapData[r][c + 1]) && IsPassableGuardTile(mapData[r][c + 2]);
    if (!leftOpen || !rightOpen) return false;

    int openNeighbors = 0;
    if (topOpen) openNeighbors++;
    if (botOpen) openNeighbors++;
    if (IsPassableGuardTile(mapData[r][c - 1])) openNeighbors++;
    if (IsPassableGuardTile(mapData[r][c + 1])) openNeighbors++;

    return (openNeighbors >= 3);
}

// Places initial guards well past starting corridors with clean spacing between guards
inline void InitializeGuards(
    Guard guards[],
    int& guardCount,
    int& activeGuardCount,
    char mapData[MAX_ROWS][MAX_COLS],
    int mapRows,
    int mapCols,
    float playerX,
    float playerY,
    int difficultyStep
) {
    guardCount = 0;
    int clampedDiff = (difficultyStep > 6) ? 6 : difficultyStep;
    float baseSpeed = 1.90f + (float)clampedDiff * 0.11f;

    int playerCol = (int)((playerX - MAP_OFFSET_X) / TILE_SIZE);
    int startSearchCol = playerCol + 16;
    if (startSearchCol < 16) startSearchCol = 16;

    int midRow = mapRows / 2;
    int bandPass = 0;

    while (bandPass < 2 && guardCount < MAX_GUARDS) {
        for (int c = startSearchCol; c < mapCols - 4 && guardCount < MAX_GUARDS; c += 5) {
            for (int r = 1; r < mapRows - 1 && guardCount < MAX_GUARDS; r++) {
                bool isLowerOrMid = (r >= midRow - 1);
                if ((bandPass == 0 && !isLowerOrMid) || (bandPass == 1 && isLowerOrMid)) {
                    continue;
                }

                if (IsSafeOpenForGuard(mapData, mapRows, mapCols, r, c)) {
                    float spawnX = MAP_OFFSET_X + (float)(c * TILE_SIZE) + 8.0f;
                    float spawnY = MAP_OFFSET_Y + (float)(r * TILE_SIZE) + 8.0f;

                    float dx = spawnX - playerX;
                    float dy = spawnY - playerY;
                    float distFromPlayer = sqrtf(dx * dx + dy * dy);

                    bool spacedFromGuards = true;
                    for (int k = 0; k < guardCount; k++) {
                        float gdx = spawnX - guards[k].x;
                        float gdy = spawnY - guards[k].y;
                        if (sqrtf(gdx * gdx + gdy * gdy) < 380.0f) {
                            spacedFromGuards = false;
                            break;
                        }
                    }

                    if (distFromPlayer >= 560.0f && spacedFromGuards) {
                        guards[guardCount].x = spawnX;
                        guards[guardCount].y = spawnY;
                        guards[guardCount].startX = spawnX - 64.0f;
                        guards[guardCount].endX = spawnX + 64.0f;
                        guards[guardCount].speed = (guardCount % 2 == 0) ? baseSpeed : -baseSpeed;
                        guards[guardCount].state = 0;
                        guards[guardCount].stunTimer = 0.0f;
                        guards[guardCount].isDefeated = false;

                        guards[guardCount].dirRow = (guards[guardCount].speed >= 0.0f) ? 2 : 1;
                        guards[guardCount].animFrame = 1;
                        guards[guardCount].animTimer = 0.0f;
                        guards[guardCount].grabTimer = 0.0f;

                        guardCount++;
                        break;
                    }
                }
            }
        }
        bandPass++;
    }

    activeGuardCount = INITIAL_GUARDS;
    if (activeGuardCount > MAX_ACTIVE_GUARDS) activeGuardCount = MAX_ACTIVE_GUARDS;
    if (activeGuardCount > guardCount) activeGuardCount = guardCount;
}

inline void ScaleGuardsFairly(int& activeGuardCount, int totalPoolGuards, int desiredCount) {
    if (desiredCount > MAX_ACTIVE_GUARDS) desiredCount = MAX_ACTIVE_GUARDS;
    if (desiredCount > totalPoolGuards) desiredCount = totalPoolGuards;
    if (desiredCount > activeGuardCount) activeGuardCount = desiredCount;
}

inline void ShiftGuardsWorldX(Guard guards[], int guardCount, float shiftX) {
    for (int i = 0; i < guardCount; i++) {
        guards[i].x -= shiftX;
        guards[i].startX -= shiftX;
        guards[i].endX -= shiftX;
    }
}

inline void InitializePowerUps(char mapData[MAX_ROWS][MAX_COLS], int rows, int cols, PowerUp powerUps[], int& powerUpCount) {
    powerUpCount = 0;
    int seenP = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (mapData[i][j] == 'P' && powerUpCount < MAX_POWERUPS) {
                seenP++;
                if ((seenP % 3 != 1) && (rand() % 100 < 55)) {
                    continue;
                }

                powerUps[powerUpCount].x = MAP_OFFSET_X + (float)(j * TILE_SIZE) + 8.0f;
                powerUps[powerUpCount].y = MAP_OFFSET_Y + (float)(i * TILE_SIZE) + 8.0f;

                int rType = rand() % 10;
                if (rType < 4) powerUps[powerUpCount].type = 1;
                else if (rType < 7) powerUps[powerUpCount].type = 2;
                else powerUps[powerUpCount].type = 3;

                powerUps[powerUpCount].isActive = true;
                powerUpCount++;
            }
        }
    }
}

inline void ShiftPowerUpsWorldX(PowerUp powerUps[], int powerUpCount, float shiftX) {
    for (int i = 0; i < powerUpCount; i++) {
        powerUps[i].x -= shiftX;
    }
}

inline void UpdatePowerUps(Player& player, PowerUp powerUps[], int powerUpCount, Guard guards[], int activeGuardCount) {
    float dt = GetClampedFrameDelta();

    if (player.isSpeedBoosted) {
        player.speedBoostTimer -= dt;
        if (player.speedBoostTimer <= 0.0f) {
            player.isSpeedBoosted = false;
            player.speedBoostTimer = 0.0f;
        }
    }

    if (player.hammerTimer > 0.0f) {
        player.hammerTimer -= dt;
        if (player.hammerTimer < 0.0f) {
            player.hammerTimer = 0.0f;
        }
    }

    Rectangle playerRect = { player.x, player.y, 26.0f, 26.0f };

    for (int i = 0; i < powerUpCount; i++) {
        if (powerUps[i].isActive) {
            Rectangle powerUpRect = { powerUps[i].x - 2.0f, powerUps[i].y - 2.0f, 28.0f, 28.0f };

            if (CheckCollisionRecs(playerRect, powerUpRect)) {
                powerUps[i].isActive = false;

                if (powerUps[i].type == 1) {
                    player.isSpeedBoosted = true;
                    player.speedBoostTimer = 4.2f;
                }
                else if (powerUps[i].type == 2) {
                    for (int g = 0; g < activeGuardCount; g++) {
                        if (!guards[g].isDefeated) {
                            guards[g].stunTimer = 3.2f;
                        }
                    }
                }
                else if (powerUps[i].type == 3) {
                    player.hammerTimer = 6.5f;
                }
            }
        }
    }
}

inline void UpdateGuards(Guard guards[], int activeGuardCount, Player& player, GameState& currentState, char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int difficultyStep) {
    float dt = GetClampedFrameDelta();
    float frameStep = dt * 60.0f;

    Rectangle playerRect = { player.x, player.y, 26.0f, 26.0f };
    int clampedDiff = (difficultyStep > 6) ? 6 : difficultyStep;
    float chaseSpeed = (1.80f + (float)clampedDiff * 0.10f) * frameStep;

    for (int i = 0; i < activeGuardCount; i++) {
        if (guards[i].isDefeated) continue;

        if (guards[i].state == 2) {
            guards[i].grabTimer -= dt;
            guards[i].animFrame = ((int)(fabsf(guards[i].grabTimer) * 18.0f)) % 3;

            player.x = guards[i].x + ((guards[i].dirRow == 1) ? -14.0f : 14.0f);
            player.y = guards[i].y;
            player.isMoving = false;
            player.animFrame = 1;

            if (guards[i].grabTimer <= 0.0f) {
                currentState = GAME_OVER;
                return;
            }
            continue;
        }

        if (guards[i].stunTimer > 0.0f) {
            guards[i].stunTimer -= dt;
            if (guards[i].stunTimer < 0.0f) {
                guards[i].stunTimer = 0.0f;
            }
        }

        Rectangle guardRect = { guards[i].x, guards[i].y, 24.0f, 24.0f };
        if (CheckCollisionRecs(playerRect, guardRect)) {
            if (player.hammerTimer > 0.0f) {
                guards[i].isDefeated = true;
                player.hammerSwingTimer = 0.25f;
                player.score += HAMMER_KILL_EXP;
                continue;
            }
            else {
                guards[i].state = 2;
                guards[i].grabTimer = 0.45f;
                if (fabsf(player.x - guards[i].x) >= fabsf(player.y - guards[i].y)) {
                    guards[i].dirRow = (player.x >= guards[i].x) ? 2 : 1;
                }
                else {
                    guards[i].dirRow = (player.y >= guards[i].y) ? 0 : 3;
                }
                continue;
            }
        }

        if (guards[i].stunTimer > 0.0f) {
            guards[i].animFrame = 1;
            continue;
        }

        float oldX = guards[i].x;
        float oldY = guards[i].y;

        if (guards[i].state == 0) {
            guards[i].x += guards[i].speed * frameStep;

            bool hitWall = false;
            Rectangle checkRect = { guards[i].x, guards[i].y, 24.0f, 24.0f };
            for (int r = 0; r < mapRows; r++) {
                for (int c = 0; c < mapCols; c++) {
                    if (mapData[r][c] == '#' || mapData[r][c] == 'W' || mapData[r][c] == 'T') {
                        Rectangle wallRect = {
                            MAP_OFFSET_X + (float)(c * TILE_SIZE),
                            MAP_OFFSET_Y + (float)(r * TILE_SIZE),
                            TILE_SIZE,
                            TILE_SIZE
                        };
                        if (CheckCollisionRecs(checkRect, wallRect)) {
                            hitWall = true;
                        }
                    }
                }
            }

            if (hitWall || guards[i].x >= guards[i].endX || guards[i].x <= guards[i].startX) {
                guards[i].x = oldX;
                guards[i].speed = -guards[i].speed;
            }

            float dx = player.x - guards[i].x;
            float dy = player.y - guards[i].y;
            if (sqrtf(dx * dx + dy * dy) < 155.0f) {
                guards[i].state = 1;
            }
        }
        else if (guards[i].state == 1) {
            if (guards[i].x < player.x) guards[i].x += chaseSpeed;
            else if (guards[i].x > player.x) guards[i].x -= chaseSpeed;

            Rectangle guardRectX = { guards[i].x, guards[i].y, 24.0f, 24.0f };
            for (int r = 0; r < mapRows; r++) {
                for (int c = 0; c < mapCols; c++) {
                    if (mapData[r][c] == '#' || mapData[r][c] == 'W' || mapData[r][c] == 'T') {
                        Rectangle wallRect = {
                            MAP_OFFSET_X + (float)(c * TILE_SIZE),
                            MAP_OFFSET_Y + (float)(r * TILE_SIZE),
                            TILE_SIZE,
                            TILE_SIZE
                        };
                        if (CheckCollisionRecs(guardRectX, wallRect)) {
                            guards[i].x = oldX;
                        }
                    }
                }
            }

            if (guards[i].y < player.y) guards[i].y += chaseSpeed;
            else if (guards[i].y > player.y) guards[i].y -= chaseSpeed;

            Rectangle guardRectY = { guards[i].x, guards[i].y, 24.0f, 24.0f };
            for (int r = 0; r < mapRows; r++) {
                for (int c = 0; c < mapCols; c++) {
                    if (mapData[r][c] == '#' || mapData[r][c] == 'W' || mapData[r][c] == 'T') {
                        Rectangle wallRect = {
                            MAP_OFFSET_X + (float)(c * TILE_SIZE),
                            MAP_OFFSET_Y + (float)(r * TILE_SIZE),
                            TILE_SIZE,
                            TILE_SIZE
                        };
                        if (CheckCollisionRecs(guardRectY, wallRect)) {
                            guards[i].y = oldY;
                        }
                    }
                }
            }
        }

        float movedX = guards[i].x - oldX;
        float movedY = guards[i].y - oldY;
        if (fabsf(movedX) > 0.01f || fabsf(movedY) > 0.01f) {
            if (fabsf(movedX) >= fabsf(movedY)) {
                guards[i].dirRow = (movedX > 0.0f) ? 2 : 1;
            }
            else {
                guards[i].dirRow = (movedY > 0.0f) ? 0 : 3;
            }
            guards[i].animTimer += dt;
            if (guards[i].animTimer >= 0.14f) {
                guards[i].animTimer = 0.0f;
                guards[i].animFrame = (guards[i].animFrame + 1) % 3;
            }
        }
        else {
            guards[i].animFrame = 1;
        }
    }
}