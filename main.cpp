#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include "game_logic.h"
#include "guards_powerups.h"
#include <ctime>
#include <cmath>

using namespace std;

int main() {
    InitWindow(1280, 720, "Cipher Run - Endless Facility");
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

    float cameraX = 0.0f, cameraY = 0.0f, targetCamX = 0.0f, targetCamY = 0.0f;
    float levelTimer = 24.0f;
    int objectiveNum = 1, currentTargetEXP = 100, loopCount = 1;
    bool objectiveBannerActive = false;
    float objectiveBannerTimer = 0.0f, lastBonusAdded = 0.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.033f) dt = 0.033f;

        if (currentState == MAIN_MENU && IsKeyPressed(KEY_ENTER)) {
            if (LoadFacilityMap(mapData, mapRows, mapCols)) {
                BakeStaticPropsIntoChunk(mapData, mapRows, mapCols, 1, mapCols - 2);
                FindPlayerSpawn(mapData, mapRows, mapCols, player);
                InitializeLockers(mapData, mapRows, mapCols, lockers, lockerCount);
                InitializeGuards(guards, guardCount, activeGuardCount, mapData, mapRows, mapCols, player.x, player.y, 1);
                InitializePowerUps(mapData, mapRows, mapCols, powerUps, powerUpCount);
            }
            cameraX = cameraY = targetCamX = targetCamY = 0.0f;
            player.score = 0;
            objectiveNum = 1;
            currentTargetEXP = 100;
            loopCount = 1;
            levelTimer = 24.0f;
            objectiveBannerActive = false;
            currentState = PLAYING;
        }
        else if (currentState == PLAYING) {
            if (IsKeyPressed(KEY_ESCAPE)) currentState = PAUSED;
            if (objectiveBannerActive) {
                objectiveBannerTimer -= dt;
                if (objectiveBannerTimer <= 0.0f) objectiveBannerActive = false;
            }

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

            int difficultyStep = loopCount + (objectiveNum - 1);
            float triggerX = MAP_OFFSET_X + (float)((mapCols - 24) * TILE_SIZE);
            if (mapCols > 32 && player.x >= triggerX) {
                AdvanceEndlessWorldSeamlessly(
                    mapData, mapRows, mapCols, player, cameraX, targetCamX,
                    lockers, lockerCount, guards, guardCount, activeGuardCount,
                    powerUps, powerUpCount, objectiveNum, loopCount, 12
                );
                difficultyStep = loopCount + (objectiveNum - 1);
            }

            CheckLockerInteraction(player, lockers, lockerCount);
            UpdatePowerUps(player, powerUps, powerUpCount, guards, activeGuardCount);
            UpdateGuards(guards, activeGuardCount, player, currentState, mapData, mapRows, mapCols, difficultyStep);
            ProcessEndlessEXPObjective(
                player, objectiveNum, currentTargetEXP, levelTimer,
                activeGuardCount, guardCount, loopCount,
                objectiveBannerActive, objectiveBannerTimer, lastBonusAdded
            );
            UpdateGameTimer(levelTimer, currentState, dt);

            targetCamX = player.x - 480.0f;
            if (targetCamX < 0.0f) targetCamX = 0.0f;
            cameraX += (targetCamX - cameraX) * 0.12f;
        }
        else if (currentState == PAUSED) {
            if (IsKeyPressed(KEY_R)) currentState = PLAYING;
            if (IsKeyPressed(KEY_M)) currentState = MAIN_MENU;
        }
        else if (currentState == GAME_OVER && IsKeyPressed(KEY_ENTER)) {
            currentState = MAIN_MENU;
        }

        BeginDrawing();
        ClearBackground(Color{ 14, 17, 24, 255 });
        if (currentState == MAIN_MENU) {
            DrawText("CIPHER RUN - PRESS ENTER", 420, 320, 32, GREEN);
        }
        else if (currentState == PLAYING) {
            for (int i = 0; i < mapRows; i++) {
                for (int j = 0; j < mapCols; j++) {
                    float dx = MAP_OFFSET_X + j * TILE_SIZE - cameraX;
                    float dy = MAP_OFFSET_Y + i * TILE_SIZE - cameraY;
                    if (mapData[i][j] == '#' || mapData[i][j] == 'W') DrawRectangle((int)dx, (int)dy, TILE_SIZE, TILE_SIZE, DARKGRAY);
                    else DrawRectangle((int)dx, (int)dy, TILE_SIZE - 1, TILE_SIZE - 1, Color{ 24, 28, 36, 255 });
                }
            }
            for (int i = 0; i < lockerCount; i++) {
                DrawRectangle((int)(lockers[i].x - cameraX), (int)(lockers[i].y - cameraY), 26, 26, lockers[i].isOpen ? ORANGE : BROWN);
            }
            for (int i = 0; i < powerUpCount; i++) {
                if (powerUps[i].isActive) DrawRectangle((int)(powerUps[i].x - cameraX), (int)(powerUps[i].y - cameraY), 24, 24, GOLD);
            }
            for (int i = 0; i < activeGuardCount; i++) {
                if (!guards[i].isDefeated) DrawRectangle((int)(guards[i].x - cameraX), (int)(guards[i].y - cameraY), 24, 24, RED);
            }
            DrawRectangle((int)(player.x - cameraX), (int)(player.y - cameraY), 26, 26, BLUE);
            DrawText(TextFormat("Obj #%d: %d EXP | EXP: %d | Loop #%d | Time: %.1fs", objectiveNum, currentTargetEXP, player.score, loopCount, levelTimer), 20, 16, 20, YELLOW);
        }
        else if (currentState == PAUSED) {
            DrawText("PAUSED - R: Resume | M: Menu", 420, 320, 28, YELLOW);
        }
        else if (currentState == GAME_OVER) {
            DrawText(TextFormat("GAME OVER | Obj #%d | EXP: %d", objectiveNum, player.score), 380, 320, 28, RED);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}