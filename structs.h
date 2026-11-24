#pragma once
#include "raylib.h"

const int MAX_ROWS = 30;
const int MAX_COLS = 140;
const int MAX_LOCKERS = 80;
const int TILE_SIZE = 48;
const float MAP_OFFSET_X = 0.0f;
const float MAP_OFFSET_Y = 0.0f;

enum GameState {
    MAIN_MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

struct Player {
    float x;
    float y;
    float speed;
    int health;
    int score;
    bool isSpeedBoosted;
    float speedBoostTimer;
    float hammerTimer;
    int dirRow;
    int animFrame;
    float animTimer;
    bool isMoving;
    float hammerSwingTimer;
};

struct Locker {
    float x;
    float y;
    bool isOpen;
    bool hasDocument;
};

struct Guard {
    float x;
    float y;
    float startX;
    float endX;
    float speed;
    int state;
    float stunTimer;
    bool isDefeated;
    int dirRow;
    int animFrame;
    float animTimer;
    float grabTimer;
};

struct PowerUp {
    float x;
    float y;
    int type;
    bool isActive;
};

struct GameAssets {
    Texture2D wallTex;
    bool hasWallTex;
    Texture2D floorTex;
    bool hasFloorTex;
    Texture2D windowTex;
    bool hasWindowTex;
    Texture2D plantTex;
    bool hasPlantTex;
    Texture2D playerTex;
    bool hasPlayerTex;
    Texture2D guardTex;
    bool hasGuardTex;
    Texture2D lockerClosedTex;
    bool hasLockerClosedTex;
    Texture2D lockerOpenTex;
    bool hasLockerOpenTex;
    Texture2D powerUpTex;
    bool hasPowerUpTex;
    Texture2D powerUpSpeedTex;
    bool hasPowerUpSpeedTex;
    Texture2D powerUpStunTex;
    bool hasPowerUpStunTex;
    Texture2D powerUpHammerTex;
    bool hasPowerUpHammerTex;
};