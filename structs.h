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
};

struct PowerUp {
    float x;
    float y;
    int type;
    bool isActive;
};