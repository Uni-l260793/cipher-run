#pragma once
#include "raylib.h"

// Game States for Screen Management
enum GameState {
    MAIN_MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    LEVEL_COMPLETE
};

// Player Structure (Includes 3x4 Walk + Hammer Swing State)
struct Player {
    float x, y;
    float speed;
    int health;
    int score; // EXP
    bool isSpeedBoosted;
    float speedBoostTimer;
    float hammerTimer;

    int dirRow;            // 0 = Down, 1 = Left, 2 = Right, 3 = Up
    int animFrame;         // 0, 1, 2
    float animTimer;
    bool isMoving;
    float hammerSwingTimer;
};

// Guard Structure (Includes 3x4 Walk + Grab Animation State)
struct Guard {
    float x, y;
    float startX, endX;
    float speed;
    int state;       // 0 = Patrol, 1 = Chase, 2 = Grabbing Player
    float stunTimer;
    bool isDefeated;

    int dirRow;      // 0 = Down, 1 = Left, 2 = Right, 3 = Up
    int animFrame;
    float animTimer;
    float grabTimer;
};

// Locker Structure
struct Locker {
    float x, y;
    bool isOpen;
    bool hasDocument;
};

// PowerUp Structure
struct PowerUp {
    float x, y;
    int type;     // 1 = Speed Boost, 2 = EMP Stun, 3 = Hammer
    bool isActive;
};

// Pure PF Asset Container (Textures + Props + Audio)
struct GameAssets {
    Texture2D wallTex;
    Texture2D floorTex;
    Texture2D windowTex;
    Texture2D plantTex;
    Texture2D playerTex;      // 96x128 (3x4 grid, 32x32 per frame)
    Texture2D guardTex;       // 96x128 (3x4 grid, 32x32 per frame)
    Texture2D lockerClosedTex;
    Texture2D lockerOpenTex;
    Texture2D powerUpTex;
    Texture2D powerUpSpeedTex;
    Texture2D powerUpStunTex;
    Texture2D powerUpHammerTex;
    Texture2D menuBgTex;
    Sound smashSound;

    bool hasSmashSound;
    bool hasWallTex;
    bool hasFloorTex;
    bool hasWindowTex;
    bool hasPlantTex;
    bool hasPlayerTex;
    bool hasGuardTex;
    bool hasLockerClosedTex;
    bool hasLockerOpenTex;
    bool hasPowerUpTex;
    bool hasPowerUpSpeedTex;
    bool hasPowerUpStunTex;
    bool hasPowerUpHammerTex;
    bool hasMenuBgTex;

    Sound hackSound;
    Sound powerUpSound;
    Sound gameOverSound;
    Sound winSound;

    bool hasHackSound;
    bool hasPowerUpSound;
    bool hasGameOverSound;
    bool hasWinSound;
};