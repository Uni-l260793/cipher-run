#pragma once
#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include "guards_powerups.h"
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cmath>

inline float GetNextObjectiveBonusTime(int nextObjectiveNum) {
    float bonus = 16.0f - (float)(nextObjectiveNum - 1) * 2.0f;
    if (bonus < 7.5f) bonus = 7.5f;
    return bonus;
}

inline void BakeStaticPropsIntoChunk(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int startCol, int endCol) {
    if (mapRows < 4 || mapCols < 6) return;
    if (startCol < 1) startCol = 1;
    if (endCol > mapCols - 2) endCol = mapCols - 2;

    for (int c = startCol; c <= endCol; c++) {
        if (c % 5 == 2 && mapData[0][c] == '#' && mapData[1][c] != '#') mapData[0][c] = 'W';
    }
    for (int r = 1; r < mapRows - 1; r++) {
        for (int c = startCol; c <= endCol; c++) {
            if (mapData[r][c] == '.' && (c % 14 == 7) && (r == 1 || r == mapRows - 2)) mapData[r][c] = 'T';
        }
    }
}

inline void MutateForwardCorridorChunk(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int startCol, int endCol, int difficultyStep) {
    if (mapRows < 4 || mapCols < 6 || startCol >= endCol) return;
    int coverChance = 28 + (difficultyStep > 5 ? 5 : difficultyStep) * 3;
    int laneA = 1 + (rand() % (mapRows - 2));
    int laneB = (laneA + 2 <= mapRows - 2) ? (laneA + 2) : (1 + ((laneA + 1) % (mapRows - 2)));

    for (int c = startCol; c < endCol; c++) {
        int colPhase = (c - startCol) % 6;
        bool isJunction = (colPhase == 0 || colPhase == 3);
        for (int r = 1; r < mapRows - 1; r++) {
            if (mapData[r][c] == 'S') mapData[r][c] = '.';
            if (isJunction || r == laneA || r == laneB) mapData[r][c] = '.';
            else mapData[r][c] = (rand() % 100 < coverChance) ? '#' : '.';
        }
        if (!isJunction && (c % 3 == 1)) {
            int lr = 1 + (rand() % (mapRows - 2));
            if (mapData[lr][c] == '.') mapData[lr][c] = 'L';
        }
        if (!isJunction && (c % 6 == 4) && (rand() % 100 < 60)) {
            int pr = 1 + (rand() % (mapRows - 2));
            if (mapData[pr][c] == '.') mapData[pr][c] = 'P';
        }
    }
    BakeStaticPropsIntoChunk(mapData, mapRows, mapCols, startCol, endCol);
}

inline void AppendLockersInColumnRange(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int colStart, int colEnd, Locker lockers[], int& lockerCount) {
    for (int i = 1; i < mapRows - 1; i++) {
        for (int j = colStart; j < colEnd; j++) {
            if (mapData[i][j] == 'L' && lockerCount < MAX_LOCKERS) {
                lockers[lockerCount] = { MAP_OFFSET_X + j * TILE_SIZE + 5.0f, MAP_OFFSET_Y + i * TILE_SIZE + 5.0f, false, (rand() % 2 == 0) };
                lockerCount++;
            }
        }
    }
}

inline void AppendPowerUpsInColumnRange(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int colStart, int colEnd, PowerUp powerUps[], int& powerUpCount) {
    for (int i = 1; i < mapRows - 1; i++) {
        for (int j = colStart; j < colEnd; j++) {
            if (mapData[i][j] == 'P' && powerUpCount < MAX_POWERUPS) {
                int rType = rand() % 10;
                int t = (rType < 4) ? 1 : ((rType < 7) ? 2 : 3);
                powerUps[powerUpCount] = { MAP_OFFSET_X + j * TILE_SIZE + 8.0f, MAP_OFFSET_Y + i * TILE_SIZE + 8.0f, t, true };
                powerUpCount++;
            }
        }
    }
}

inline void AdvanceEndlessWorldSeamlessly(
    char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols,
    Player& player, float& cameraX, float& targetCamX,
    Locker lockers[], int& lockerCount,
    Guard guards[], int& guardCount, int& activeGuardCount,
    PowerUp powerUps[], int& powerUpCount,
    int objectiveNum, int& loopCount, int shiftCols
) {
    if (mapCols <= shiftCols + 8) return;
    loopCount++;
    float shiftX = (float)(shiftCols * TILE_SIZE);
    int keepCols = mapCols - shiftCols;

    for (int r = 0; r < mapRows; r++) {
        for (int c = 0; c < keepCols; c++) mapData[r][c] = mapData[r][c + shiftCols];
        for (int c = keepCols; c < mapCols; c++) mapData[r][c] = (r == 0 || r == mapRows - 1 || c == mapCols - 1) ? '#' : '.';
    }

    int difficultyStep = loopCount + (objectiveNum - 1);
    int mutateStart = keepCols + 3;
    MutateForwardCorridorChunk(mapData, mapRows, mapCols, mutateStart, mapCols - 1, difficultyStep);

    player.x -= shiftX;
    cameraX -= shiftX;
    targetCamX -= shiftX;
    if (cameraX < 0.0f) cameraX = 0.0f;
    if (targetCamX < 0.0f) targetCamX = 0.0f;

    ShiftLockersWorldX(lockers, lockerCount, shiftX);
    AppendLockersInColumnRange(mapData, mapRows, mutateStart, mapCols - 1, lockers, lockerCount);

    ShiftPowerUpsWorldX(powerUps, powerUpCount, shiftX);
    AppendPowerUpsInColumnRange(mapData, mapRows, mutateStart, mapCols - 1, powerUps, powerUpCount);

    ShiftGuardsWorldX(guards, guardCount, shiftX);
    int desired = INITIAL_GUARDS + ((objectiveNum - 1) / 2) + 1;
    if (desired > MAX_ACTIVE_GUARDS) desired = MAX_ACTIVE_GUARDS;
    if (desired > guardCount) desired = guardCount;
    activeGuardCount = desired;
}