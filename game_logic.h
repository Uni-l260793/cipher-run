#pragma once
#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include "guards_powerups.h"
#include <fstream>
#include <cstring>
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
        if (c % 5 == 2 && mapData[0][c] == '#' && mapData[1][c] != '#') {
            mapData[0][c] = 'W';
        }
    }
    for (int r = 1; r < mapRows - 1; r++) {
        for (int c = startCol; c <= endCol; c++) {
            if (mapData[r][c] == '.' && (c % 14 == 7) && (r == 1 || r == mapRows - 2)) {
                mapData[r][c] = 'T';
            }
        }
    }
}

inline void UpdateGameTimer(float& levelTimer, GameState& currentState, float dt) {
    if (levelTimer > 0.0f) {
        levelTimer -= dt;
        if (levelTimer < 0.0f) levelTimer = 0.0f;
    }
    else {
        currentState = GAME_OVER;
    }
}

inline void ProcessEndlessEXPObjective(
    Player& player,
    int& objectiveNum,
    int& currentTargetEXP,
    float& levelTimer,
    int& activeGuardCount,
    int totalPoolGuards,
    int loopCount,
    bool& objectiveBannerActive,
    float& objectiveBannerTimer,
    float& lastBonusAdded
) {
    (void)loopCount;
    if (player.score >= currentTargetEXP) {
        objectiveNum++;
        currentTargetEXP += 100 + (objectiveNum - 2) * 20;
        float bonusTime = GetNextObjectiveBonusTime(objectiveNum);
        levelTimer += bonusTime;
        lastBonusAdded = bonusTime;

        int desired = INITIAL_GUARDS + ((objectiveNum - 1) / 2) + 1;
        if (desired > MAX_ACTIVE_GUARDS) desired = MAX_ACTIVE_GUARDS;
        if (desired > totalPoolGuards) desired = totalPoolGuards;
        activeGuardCount = desired;

        objectiveBannerActive = true;
        objectiveBannerTimer = 2.5f;
    }
}