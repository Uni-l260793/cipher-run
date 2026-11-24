#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include "game_logic.h"
#include "guards_powerups.h"
#include <ctime>
#include <cmath>

using namespace std;

static float ComputeCrispIntegerScale(int screenWidth, int screenHeight, int mapRows) {
    float mapPixelH = (float)(mapRows > 0 ? mapRows : 12) * TILE_SIZE;
    if (mapPixelH < 1.0f) mapPixelH = 1.0f;
    int rawScale = (int)floorf((float)screenHeight / mapPixelH);
    if (rawScale < 2) rawScale = 2;
    if (rawScale > 3) rawScale = 3;
    return (float)rawScale;
}

static void DrawEntityVisual(bool hasTex, Texture2D tex, Rectangle dest, Color fallbackColor, Color tint) {
    if (hasTex && tex.id > 0) {
        Rectangle src = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
        Rectangle snappedDest = { roundf(dest.x), roundf(dest.y), roundf(dest.width), roundf(dest.height) };
        DrawTexturePro(tex, src, snappedDest, { 0.0f, 0.0f }, 0.0f, tint);
    }
    else {
        DrawRectangle((int)roundf(dest.x), (int)roundf(dest.y), (int)roundf(dest.width), (int)roundf(dest.height), fallbackColor);
    }
}

static void DrawPlayerEquippedHammer(const GameAssets& assets, const Player& player, Rectangle playerDest, float viewScale) {
    if (player.hammerTimer <= 0.0f && player.hammerSwingTimer <= 0.0f) return;
    float swingAngle = (player.hammerSwingTimer > 0.0f) ? ((1.0f - (player.hammerSwingTimer / 0.25f)) * 85.0f) : 0.0f;
    float hx = playerDest.x + playerDest.width * 0.5f;
    float hy = playerDest.y + playerDest.height * 0.55f;
    float baseAngle = (player.dirRow == 1) ? (-35.0f - swingAngle) : (35.0f + swingAngle);
    float hSize = roundf(42.0f * viewScale);
    Rectangle hDest = { roundf(hx), roundf(hy), hSize, hSize };
    if (assets.hasPowerUpHammerTex && assets.powerUpHammerTex.id > 0) {
        DrawTexturePro(assets.powerUpHammerTex, { 0.0f, 0.0f, (float)assets.powerUpHammerTex.width, (float)assets.powerUpHammerTex.height }, hDest, { hSize * 0.5f, hSize * 0.75f }, baseAngle, WHITE);
    }
    else {
        DrawRectanglePro({ roundf(hx), roundf(hy), 16.0f * viewScale, 34.0f * viewScale }, { 8.0f * viewScale, 26.0f * viewScale }, baseAngle, GOLD);
    }
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Cipher Run - Endless Facility");
    InitAudioDevice();
    SetTargetFPS(60);
    srand((unsigned int)time(0));

    GameAssets assets = { 0 };
    LoadGameAssets(assets);

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

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        float viewScale = ComputeCrispIntegerScale(screenWidth, screenHeight, mapRows);

        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

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
            currentState = PLAYING;
        }
        else if (currentState == PLAYING) {
            if (IsKeyPressed(KEY_ESCAPE)) currentState = PAUSED;
            if (player.hammerSwingTimer > 0.0f) {
                player.hammerSwingTimer -= dt;
                if (player.hammerSwingTimer < 0.0f) player.hammerSwingTimer = 0.0f;
            }

            float frameStep = dt * 60.0f;
            float moveSpeed = (player.isSpeedBoosted ? (player.speed * 1.70f) : player.speed) * frameStep;
            float oldX = player.x, oldY = player.y;
            bool moving = false;

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) { player.y -= moveSpeed; player.dirRow = 3; moving = true; }
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) { player.y += moveSpeed; player.dirRow = 0; moving = true; }
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) { player.x -= moveSpeed; player.dirRow = 1; moving = true; }
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { player.x += moveSpeed; player.dirRow = 2; moving = true; }

            player.isMoving = moving;
            if (moving) {
                player.animTimer += dt;
                if (player.animTimer >= 0.10f) { player.animTimer = 0.0f; player.animFrame = (player.animFrame + 1) % 3; }
            }
            else {
                player.animFrame = 1;
            }

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

            int prevScore = player.score;
            CheckLockerInteraction(player, lockers, lockerCount);
            UpdatePowerUps(player, powerUps, powerUpCount, guards, activeGuardCount);
            UpdateGuards(guards, activeGuardCount, player, currentState, mapData, mapRows, mapCols, difficultyStep);
            if (player.score > prevScore && assets.hasHackSound) PlaySound(assets.hackSound);

            ProcessEndlessEXPObjective(
                player, objectiveNum, currentTargetEXP, levelTimer,
                activeGuardCount, guardCount, loopCount,
                objectiveBannerActive, objectiveBannerTimer, lastBonusAdded
            );
            UpdateGameTimer(levelTimer, currentState, dt);

            targetCamX = player.x - ((float)screenWidth / viewScale) * 0.40f;
            if (targetCamX < 0.0f) targetCamX = 0.0f;
            cameraX += (targetCamX - cameraX) * 0.14f;
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
            DrawText("CIPHER RUN - PRESS ENTER TO START", screenWidth / 2 - 240, screenHeight / 2 - 20, 30, GREEN);
        }
        else if (currentState == PLAYING) {
            float tileW = roundf(TILE_SIZE * viewScale);
            float tileH = roundf(TILE_SIZE * viewScale);
            for (int i = 0; i < mapRows; i++) {
                for (int j = 0; j < mapCols; j++) {
                    float dx = roundf((MAP_OFFSET_X + j * TILE_SIZE - cameraX) * viewScale);
                    float dy = roundf((MAP_OFFSET_Y + i * TILE_SIZE - cameraY) * viewScale);
                    if (mapData[i][j] == '#' || mapData[i][j] == 'W') {
                        DrawEntityVisual(assets.hasWallTex, assets.wallTex, { dx, dy, tileW, tileH }, DARKGRAY, WHITE);
                    }
                    else {
                        DrawEntityVisual(assets.hasFloorTex, assets.floorTex, { dx, dy, tileW, tileH }, Color{ 24, 28, 36, 255 }, WHITE);
                    }
                }
            }
            Rectangle pDest = { roundf((player.x - cameraX - 15.0f) * viewScale), roundf((player.y - cameraY - 24.0f) * viewScale), roundf(60.0f * viewScale), roundf(68.0f * viewScale) };
            DrawSpriteFrame3x4(assets.hasPlayerTex, assets.playerTex, player.animFrame, player.dirRow, pDest, BLUE, WHITE);
            DrawPlayerEquippedHammer(assets, player, pDest, viewScale);
            DrawText(TextFormat("Obj #%d: %d EXP | EXP: %d | Time: %.1fs", objectiveNum, currentTargetEXP, player.score, levelTimer), 20, 16, 22, YELLOW);
        }
        else if (currentState == PAUSED) {
            DrawText("PAUSED - R: Resume | M: Menu", screenWidth / 2 - 190, screenHeight / 2 - 20, 28, YELLOW);
        }
        else if (currentState == GAME_OVER) {
            DrawText(TextFormat("GAME OVER | EXP: %d | ENTER to Retry", player.score), screenWidth / 2 - 220, screenHeight / 2 - 20, 28, RED);
        }
        EndDrawing();
    }

    UnloadGameAssets(assets);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}