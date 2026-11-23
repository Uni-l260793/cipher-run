#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include "game_logic.h"
#include "guards_powerups.h"
#include <ctime>

using namespace std;

int main() {
    InitWindow(1280, 720, "Cipher Run");
    SetTargetFPS(60);
    srand((unsigned int)time(0));

    GameState currentState = MAIN_MENU;
    char mapData[MAX_ROWS][MAX_COLS] = { 0 };
    int mapRows = 0, mapCols = 0;

    Player player = { 100.0f, 100.0f, 4.0f, 100, 0 };
    Locker lockers[MAX_LOCKERS];
    int lockerCount = 0;
    Guard guards[MAX_GUARDS];
    int guardCount = 0, activeGuardCount = 0;
    PowerUp powerUps[MAX_POWERUPS];
    int powerUpCount = 0;
    float levelTimer = 30.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (currentState == MAIN_MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                if (LoadFacilityMap(mapData, mapRows, mapCols)) {
                    FindPlayerSpawn(mapData, mapRows, mapCols, player);
                    InitializeLockers(mapData, mapRows, mapCols, lockers, lockerCount);
                    InitializeGuards(guards, guardCount, activeGuardCount, mapData, mapRows, mapCols, player.x, player.y, 1);
                    InitializePowerUps(mapData, mapRows, mapCols, powerUps, powerUpCount);
                }
                currentState = PLAYING;
            }
        }
        else if (currentState == PLAYING) {
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) player.y -= player.speed;
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) player.y += player.speed;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) player.x -= player.speed;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) player.x += player.speed;

            CheckLockerInteraction(player, lockers, lockerCount);
            UpdatePowerUps(player, powerUps, powerUpCount, guards, activeGuardCount);
            UpdateGuards(guards, activeGuardCount, player, currentState, mapData, mapRows, mapCols, 1);
            UpdateGameTimer(levelTimer, currentState, dt);
        }

        BeginDrawing();
        ClearBackground(BLACK);
        if (currentState == MAIN_MENU) {
            DrawText("CIPHER RUN - PRESS ENTER", 420, 320, 32, GREEN);
        }
        else if (currentState == PLAYING) {
            for (int r = 0; r < mapRows; r++) {
                for (int c = 0; c < mapCols; c++) {
                    if (mapData[r][c] == '#') DrawRectangle(c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE, DARKGRAY);
                }
            }
            DrawRectangle((int)player.x, (int)player.y, 26, 26, BLUE);
            for (int i = 0; i < activeGuardCount; i++) {
                if (!guards[i].isDefeated) DrawRectangle((int)guards[i].x, (int)guards[i].y, 24, 24, RED);
            }
        }
        else if (currentState == GAME_OVER) {
            DrawText("GAME OVER - PRESS ENTER", 420, 320, 32, RED);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}