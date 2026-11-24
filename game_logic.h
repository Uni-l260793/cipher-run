#pragma once
#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include "guards_powerups.h"
#include <fstream>
#include <cstring>

inline bool LoadFacilityMap(char mapData[MAX_ROWS][MAX_COLS], int& mapRows, int& mapCols) {
    mapRows = 0;
    mapCols = 0;

    std::ifstream file("level1.txt");
    if (!file.is_open()) {
        return false;
    }

    while (mapRows < MAX_ROWS && file.getline(mapData[mapRows], MAX_COLS)) {
        int len = (int)strlen(mapData[mapRows]);
        if (len > mapCols) {
            mapCols = len;
        }
        mapRows++;
    }

    file.close();
    return (mapRows > 0);
}

inline void FindPlayerSpawn(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, Player& player) {
    player.x = MAP_OFFSET_X + TILE_SIZE + 7.0f;
    player.y = MAP_OFFSET_Y + TILE_SIZE + 7.0f;

    for (int i = 0; i < mapRows; i++) {
        for (int j = 0; j < mapCols; j++) {
            if (mapData[i][j] == 'S') {
                player.x = MAP_OFFSET_X + (float)(j * TILE_SIZE) + 7.0f;
                player.y = MAP_OFFSET_Y + (float)(i * TILE_SIZE) + 7.0f;
                return;
            }
        }
    }
}