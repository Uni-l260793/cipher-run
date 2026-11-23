#pragma once
#include "raylib.h"
#include "structs.h"
#include <cstdlib>

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
    Rectangle playerRect = { player.x - 4.0f, player.y - 4.0f, 34.0f, 34.0f };
    for (int i = 0; i < lockerCount; i++) {
        if (!lockers[i].isOpen) {
            Rectangle lockerRect = { lockers[i].x, lockers[i].y, 26.0f, 26.0f };
            if (CheckCollisionRecs(playerRect, lockerRect)) {
                lockers[i].isOpen = true;
                player.score += 15;
            }
        }
    }
}
