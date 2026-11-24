#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include "game_logic.h"
#include "guards_powerups.h"
#include <ctime>
#include <cmath>

using namespace std;

int main() {
    InitWindow(1280, 720, "Cipher Run");
    SetTargetFPS(60);
    srand((unsigned int)time(0));

    GameState currentState = MAIN_MENU;
    char mapData[MAX_ROWS][MAX_COLS] = { 0 };
    int mapRows = 0, mapCols = 0;

    Player player = { 100.0f, 100.0f, 4.75f, 100, 0, false, 0.0f, 0.0f, 2, 1, 0.0f, false, 0.0f };
    Locker lockers[MAX_LOCKERS];
    int lockerCount = 0;
    Guard guards[MAX_GUARDS];
    int guardCount = 0, activeGuardCount = 0;
    PowerUp powerUps[MAX_POWERUPS];
    int powerUpCount = 0;

    float cameraX = 0.0f, cameraY = 0.0f;
    float levelTimer = 24.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.033f) dt = 0.033f;

        if (currentState == MAIN_MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                if (LoadFacilityMap(mapData, mapRows, mapCols)) {
                    FindPlayerSpawn(mapData, mapRows, mapCols, player);
                    InitializeLockers(mapData, mapRows, mapCols, lockers, lockerCount);
                    InitializeGuards(guards, guardCount, activeGuardCount, mapData, mapRows, mapCols, player.x, player.y, 1);
                    InitializePowerUps(mapData, mapRows, mapCols, powerUps, powerUpCount);
                }
                player.score = 0;
                levelTimer = 24.0f;
                currentState = PLAYING;
            }
        }
        else if (currentState == PLAYING) {
            if (IsKeyPressed(KEY_ESCAPE)) currentState = PAUSED;

            float moveSpeed = player.isSpeedBoosted ? (player.speed * 1.70f) : player.speed;
            float oldX = player.x, oldY = player.y;

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) { player.y -= moveSpeed; player.dirRow = 3; }
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) { player.y += moveSpeed; player.dirRow = 0; }
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) { player.x -= moveSpeed; player.dirRow = 1; }
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { player.x += moveSpeed; player.dirRow = 2; }

            Rectangle rectX = { player.x, oldY, 26.0f, 26.0f };
            for (int i = 0; i < mapRows; i++) {
                for (int j = 0; j < mapCols; j++) {
                    if (mapData[i][j] == '#' || mapData[i][j] == 'W') {
                        Rectangle w = { MAP_OFFSET_X + j * TILE_SIZE, MAP_OFFSET_Y + i * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                        if (CheckCollisionRecs(rectX, w)) player.x = oldX;
                    }
                }
            }
            Rectangle rectY = { player.x, player.y, 26.0f, 26.0f };
            for (int i = 0; i < mapRows; i++) {
                for (int j = 0; j < mapCols; j++) {
                    if (mapData[i][j] == '#' || mapData[i][j] == 'W') {
                        Rectangle w = { MAP_OFFSET_X + j * TILE_SIZE, MAP_OFFSET_Y + i * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                        if (CheckCollisionRecs(rectY, w)) player.y = oldY;
                    }
                }
            }

            CheckLockerInteraction(player, lockers, lockerCount);
            UpdatePowerUps(player, powerUps, powerUpCount, guards, activeGuardCount);
            UpdateGuards(guards, activeGuardCount, player, currentState, mapData, mapRows, mapCols, 1);
            UpdateGameTimer(levelTimer, currentState, dt);

            cameraX = player.x - 450.0f;
            if (cameraX < 0.0f) cameraX = 0.0f;
        }
        else if (currentState == PAUSED) {
            if (IsKeyPressed(KEY_R)) currentState = PLAYING;
            if (IsKeyPressed(KEY_M)) currentState = MAIN_MENU;
        }
        else if (currentState == GAME_OVER) {
            if (IsKeyPressed(KEY_ENTER)) currentState = MAIN_MENU;
        }

        BeginDrawing();
        ClearBackground(Color{ 14, 17, 24, 255 });
        if (currentState == MAIN_MENU) {
            DrawText("CIPHER RUN - PRESS ENTER TO START", 380, 320, 30, GREEN);
        }
        else if (currentState == PLAYING) {
            for (int i = 0; i < mapRows; i++) {
                for (int j = 0; j < mapCols; j++) {
                    float dx = MAP_OFFSET_X + j * TILE_SIZE - cameraX;
                    float dy = MAP_OFFSET_Y + i * TILE_SIZE - cameraY;
                    if (mapData[i][j] == '#' || mapData[i][j] == 'W') {
                        DrawRectangle((int)dx, (int)dy, TILE_SIZE, TILE_SIZE, DARKGRAY);
                    }
                    else {
                        DrawRectangle((int)dx, (int)dy, TILE_SIZE - 1, TILE_SIZE - 1, Color{ 24, 28, 36, 255 });
                    }
                }
            }
            DrawRectangle((int)(player.x - cameraX), (int)(player.y - cameraY), 26, 26, BLUE);
            for (int i = 0; i < activeGuardCount; i++) {
                if (!guards[i].isDefeated) {
                    DrawRectangle((int)(guards[i].x - cameraX), (int)(guards[i].y - cameraY), 24, 24, RED);
                }
            }
            DrawText(TextFormat("EXP: %d | Time: %.1fs", player.score, levelTimer), 20, 16, 22, YELLOW);
        }
        else if (currentState == PAUSED) {
            DrawText("PAUSED - R: Resume | M: Menu", 420, 320, 28, YELLOW);
        }
        else if (currentState == GAME_OVER) {
            DrawText(TextFormat("GAME OVER | EXP: %d | ENTER to Retry", player.score), 360, 320, 28, RED);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}