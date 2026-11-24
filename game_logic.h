#pragma once
#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include "guards_powerups.h"
#include <fstream>
#include <cstring>
#include <cmath>

inline void LoadTextureSafe(const char* path, Texture2D& tex, bool& loadedFlag) {
    loadedFlag = false;
    if (FileExists(path)) {
        tex = LoadTexture(path);
        if (tex.id > 0) {
            SetTextureFilter(tex, TEXTURE_FILTER_POINT);
            loadedFlag = true;
        }
    }
}

inline void LoadGameAssets(GameAssets& assets) {
    LoadTextureSafe("assets/wall.png", assets.wallTex, assets.hasWallTex);
    LoadTextureSafe("assets/floor.png", assets.floorTex, assets.hasFloorTex);
    LoadTextureSafe("assets/player.png", assets.playerTex, assets.hasPlayerTex);
    LoadTextureSafe("assets/guard.png", assets.guardTex, assets.hasGuardTex);
    LoadTextureSafe("assets/locker_closed.png", assets.lockerClosedTex, assets.hasLockerClosedTex);
    LoadTextureSafe("assets/locker_open.png", assets.lockerOpenTex, assets.hasLockerOpenTex);
}

inline void UnloadGameAssets(GameAssets& assets) {
    if (assets.hasWallTex) UnloadTexture(assets.wallTex);
    if (assets.hasFloorTex) UnloadTexture(assets.floorTex);
    if (assets.hasPlayerTex) UnloadTexture(assets.playerTex);
    if (assets.hasGuardTex) UnloadTexture(assets.guardTex);
    if (assets.hasLockerClosedTex) UnloadTexture(assets.lockerClosedTex);
    if (assets.hasLockerOpenTex) UnloadTexture(assets.lockerOpenTex);
}

inline void DrawSpriteFrame3x4(bool hasTex, Texture2D tex, int frameCol, int dirRow, Rectangle dest, Color fallbackColor, Color tint) {
    if (hasTex && tex.width >= 3 && tex.height >= 4) {
        float frameW = floorf((float)tex.width / 3.0f);
        float frameH = floorf((float)tex.height / 4.0f);
        Rectangle src = {
            floorf((float)(frameCol % 3) * frameW),
            floorf((float)(dirRow % 4) * frameH),
            frameW,
            frameH
        };
        DrawTexturePro(tex, src, dest, { 0.0f, 0.0f }, 0.0f, tint);
    }
    else {
        DrawRectangle((int)dest.x, (int)dest.y, (int)dest.width, (int)dest.height, fallbackColor);
    }
}

inline bool LoadFacilityMap(char mapData[MAX_ROWS][MAX_COLS], int& mapRows, int& mapCols) {
    mapRows = 0;
    mapCols = 0;
    std::ifstream file("level1.txt");
    if (!file.is_open()) return false;

    while (mapRows < MAX_ROWS && file.getline(mapData[mapRows], MAX_COLS)) {
        int len = (int)strlen(mapData[mapRows]);
        if (len > mapCols) mapCols = len;
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

inline void UpdateGameTimer(float& levelTimer, GameState& currentState, float dt) {
    if (levelTimer > 0.0f) {
        levelTimer -= dt;
        if (levelTimer < 0.0f) levelTimer = 0.0f;
    }
    else {
        currentState = GAME_OVER;
    }
}