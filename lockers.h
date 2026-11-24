#pragma once
#include "raylib.h"
#include "structs.h"
#include <cstdlib>

// Side-scrolling multi-row facility map limits (Pure PF)
const int MAX_ROWS = 12;
const int MAX_COLS = 80;
const int MAX_LOCKERS = 60;

const float TILE_SIZE = 40.0f;
const float MAP_OFFSET_X = 80.0f;
const float MAP_OFFSET_Y = 80.0f;

inline void InitializeLockers(char mapData[MAX_ROWS][MAX_COLS], int rows, int cols, Locker lockers[], int& lockerCount) {
    lockerCount = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (mapData[i][j] == 'L' && lockerCount < MAX_LOCKERS) {
                lockers[lockerCount].x = MAP_OFFSET_X + (float)(j * TILE_SIZE) + 5.0f;
                lockers[lockerCount].y = MAP_OFFSET_Y + (float)(i * TILE_SIZE) + 5.0f;
                lockers[lockerCount].isOpen = false;
                lockers[lockerCount].hasDocument = (rand() % 2 == 0);

                lockerCount++;
            }
        }
    }
}

inline void CheckLockerInteraction(Player& player, Locker lockers[], int lockerCount) {
    Rectangle playerRect = { player.x, player.y, 26.0f, 26.0f };

    for (int i = 0; i < lockerCount; i++) {
        if (!lockers[i].isOpen) {
            Rectangle lockerRect = { lockers[i].x - 2.0f, lockers[i].y - 2.0f, 34.0f, 34.0f };

            if (CheckCollisionRecs(playerRect, lockerRect)) {
                lockers[i].isOpen = true;

                if (lockers[i].hasDocument) {
                    player.score += 50;
                }
            }
        }
    }
}

inline void ShiftLockersWorldX(Locker lockers[], int lockerCount, float shiftX) {
    float minSafeX = MAP_OFFSET_X + TILE_SIZE + 5.0f;

    for (int i = 0; i < lockerCount; i++) {
        lockers[i].x -= shiftX;
        if (lockers[i].x < minSafeX) {
            lockers[i].x = minSafeX;
        }
    }
}