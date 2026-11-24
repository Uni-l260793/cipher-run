#pragma once
#include "raylib.h"
#include "structs.h"
#include <cstdlib>

inline void InitializeLockers(char mapData[MAX_ROWS][MAX_COLS], int rows, int cols, Locker lockers[], int& lockerCount) {
    lockerCount = 0;
    if (rows > MAX_ROWS) rows = MAX_ROWS;
    if (cols > MAX_COLS) cols = MAX_COLS;

    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            if (mapData[i][j] == 'L' && lockerCount < MAX_LOCKERS) {
                lockers[lockerCount].x = MAP_OFFSET_X + (float)(j * TILE_SIZE) + 5.0f;
                lockers[lockerCount].y = MAP_OFFSET_Y + (float)(i * TILE_SIZE) + 5.0f;
                lockers[lockerCount].isOpen = false;
                lockers[lockerCount].hasDocument = (rand() % 3 == 0);
                lockerCount++;
            }
        }
    }
}

inline void ShiftLockersWorldX(Locker lockers[], int lockerCount, float shiftX) {
    for (int i = 0; i < lockerCount; i++) {
        lockers[i].x -= shiftX;
    }
}

inline void CheckLockerInteraction(Player& player, Locker lockers[], int lockerCount) {
    Rectangle playerRect = { player.x - 6.0f, player.y - 6.0f, 38.0f, 38.0f };
    for (int i = 0; i < lockerCount; i++) {
        if (!lockers[i].isOpen) {
            Rectangle lockerRect = { lockers[i].x - 2.0f, lockers[i].y - 2.0f, 30.0f, 30.0f };
            if (CheckCollisionRecs(playerRect, lockerRect)) {
                lockers[i].isOpen = true;
                player.score += lockers[i].hasDocument ? 30 : 20;
            }
        }
    }
}