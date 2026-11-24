#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include "game_logic.h"
#include "guards_powerups.h"
#include <ctime>
#include <cmath>

using namespace std;

static void ToggleWindowedFullscreenSafe(
    int& savedWinW,
    int& savedWinH,
    int& savedWinX,
    int& savedWinY,
    bool& isBorderlessFullscreen
) {
    if (!isBorderlessFullscreen) {
        if (!IsWindowFullscreen() && !IsWindowState(FLAG_WINDOW_MAXIMIZED)) {
            savedWinW = GetScreenWidth();
            savedWinH = GetScreenHeight();
            Vector2 pos = GetWindowPosition();
            savedWinX = (int)pos.x;
            savedWinY = (int)pos.y;
            if (savedWinW < 640) savedWinW = 1280;
            if (savedWinH < 360) savedWinH = 720;
        }

        int mon = GetCurrentMonitor();
        int mw = GetMonitorWidth(mon);
        int mh = GetMonitorHeight(mon);
        Vector2 mpos = GetMonitorPosition(mon);

        SetWindowState(FLAG_WINDOW_UNDECORATED);
        SetWindowPosition((int)mpos.x, (int)mpos.y);
        SetWindowSize(mw, mh);
        isBorderlessFullscreen = true;
    }
    else {
        ClearWindowState(FLAG_WINDOW_UNDECORATED);
        if (IsWindowFullscreen()) {
            ToggleFullscreen();
        }
        SetWindowSize(savedWinW, savedWinH);
        SetWindowPosition(savedWinX, savedWinY);
        isBorderlessFullscreen = false;
    }
}

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
        Rectangle snappedDest = {
            roundf(dest.x),
            roundf(dest.y),
            roundf(dest.width),
            roundf(dest.height)
        };
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(tex, src, snappedDest, origin, 0.0f, tint);
    }
    else {
        DrawRectangle((int)roundf(dest.x), (int)roundf(dest.y), (int)roundf(dest.width), (int)roundf(dest.height), fallbackColor);
    }
}

static void DrawPlayerEquippedHammer(const GameAssets& assets, const Player& player, Rectangle playerDest, float viewScale) {
    if (player.hammerTimer <= 0.0f && player.hammerSwingTimer <= 0.0f) {
        return;
    }

    float swingProgress = (player.hammerSwingTimer > 0.0f) ? (1.0f - (player.hammerSwingTimer / 0.25f)) : 0.0f;
    float swingAngle = swingProgress * 85.0f;

    float handX = playerDest.x + playerDest.width * 0.5f;
    float handY = playerDest.y + playerDest.height * 0.55f;
    float baseAngle = 0.0f;

    if (player.dirRow == 1) {          // Left
        handX = playerDest.x + 7.0f * viewScale;
        handY = playerDest.y + playerDest.height * 0.55f;
        baseAngle = -35.0f - swingAngle;
    }
    else if (player.dirRow == 2) {     // Right
        handX = playerDest.x + playerDest.width - 7.0f * viewScale;
        handY = playerDest.y + playerDest.height * 0.55f;
        baseAngle = 35.0f + swingAngle;
    }
    else if (player.dirRow == 3) {     // Up
        handX = playerDest.x + playerDest.width * 0.72f;
        handY = playerDest.y + 12.0f * viewScale;
        baseAngle = -15.0f - swingAngle;
    }
    else {                             // Down
        handX = playerDest.x + playerDest.width * 0.75f;
        handY = playerDest.y + playerDest.height * 0.62f;
        baseAngle = 20.0f + swingAngle;
    }

    float hammerSize = roundf(42.0f * viewScale);
    Rectangle hammerDest = { roundf(handX), roundf(handY), hammerSize, hammerSize };
    Vector2 hammerOrigin = { hammerSize * 0.5f, hammerSize * 0.75f };

    if (assets.hasPowerUpHammerTex && assets.powerUpHammerTex.id > 0) {
        Rectangle src = { 0.0f, 0.0f, (float)assets.powerUpHammerTex.width, (float)assets.powerUpHammerTex.height };
        DrawTexturePro(assets.powerUpHammerTex, src, hammerDest, hammerOrigin, baseAngle, WHITE);
    }
    else if (assets.hasPowerUpTex && assets.powerUpTex.id > 0) {
        Rectangle src = { 0.0f, 0.0f, (float)assets.powerUpTex.width, (float)assets.powerUpTex.height };
        DrawTexturePro(assets.powerUpTex, src, hammerDest, hammerOrigin, baseAngle, GOLD);
    }
    else {
        DrawRectanglePro({ roundf(handX), roundf(handY), 16.0f * viewScale, 34.0f * viewScale }, { 8.0f * viewScale, 26.0f * viewScale }, baseAngle, GOLD);
    }
}

static void DrawMainMenuUI(
    int screenWidth,
    int screenHeight,
    float uiTime,
    bool musicMuted,
    bool hasCustomMenuBg,
    Texture2D customMenuBg,
    bool& startClicked,
    bool& muteClicked,
    bool& quitClicked
) {
    startClicked = false;
    muteClicked = false;
    quitClicked = false;

    ClearBackground(Color{ 10, 14, 22, 255 });

    if (hasCustomMenuBg && customMenuBg.id > 0) {
        Rectangle src = { 0.0f, 0.0f, (float)customMenuBg.width, (float)customMenuBg.height };
        Rectangle dst = { 0.0f, 0.0f, (float)screenWidth, (float)screenHeight };
        DrawTexturePro(customMenuBg, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
        DrawRectangle(0, 0, screenWidth, screenHeight, Color{ 10, 14, 22, 130 });
    }
    else {
        int gridStep = 48;
        int scrollX = ((int)(uiTime * 28.0f)) % gridStep;
        for (int x = -scrollX; x < screenWidth; x += gridStep) {
            DrawLine(x, 0, x, screenHeight, Color{ 22, 32, 48, 120 });
        }
        for (int y = 0; y < screenHeight; y += gridStep) {
            DrawLine(0, y, screenWidth, y, Color{ 22, 32, 48, 120 });
        }
    }

    int panelW = 680;
    int panelH = 445;
    if (screenWidth < 740) panelW = screenWidth - 48;
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2;

    DrawRectangle(panelX, panelY, panelW, panelH, Color{ 16, 22, 34, 238 });
    DrawRectangleLines(panelX, panelY, panelW, panelH, Color{ 45, 212, 130, 195 });
    DrawRectangle(panelX, panelY, panelW, 6, Color{ 45, 212, 130, 255 });

    const char* titleText = "CIPHER RUN";
    int titleW = MeasureText(titleText, 52);
    DrawText(titleText, (screenWidth - titleW) / 2, panelY + 24, 52, Color{ 74, 240, 155, 255 });

    const char* subText = "Steal data. Dodge grumpy hall monitors. Don't run out of clock.";
    int subW = MeasureText(subText, 18);
    DrawText(subText, (screenWidth - subW) / 2, panelY + 82, 18, SKYBLUE);

    DrawLine(panelX + 36, panelY + 114, panelX + panelW - 36, panelY + 114, Color{ 55, 72, 98, 200 });

    int leftX = panelX + 44;
    DrawText("HOW TO SURVIVE", leftX, panelY + 130, 18, GOLD);
    DrawText("- Raid lockers for EXP before the timer roasts your run.", leftX, panelY + 156, 18, LIGHTGRAY);
    DrawText("- Grab Hammer to bonk guards for bonus EXP; avoid chokeholds.", leftX, panelY + 182, 18, LIGHTGRAY);
    DrawText("- Move: WASD / Arrows  |  Pause: ESC  |  Fullscreen: F11", leftX, panelY + 214, 18, LIGHTGRAY);

    Vector2 mouse = GetMousePosition();
    float pulse = 0.65f + 0.35f * sinf(uiTime * 4.2f);
    Color ctaColor = Color{
        (unsigned char)(250 * pulse),
        (unsigned char)(215 * pulse),
        60,
        255
    };

    Rectangle startRect = { (float)(panelX + 44), (float)(panelY + 252), (float)(panelW - 88), 48.0f };
    bool hoverStart = CheckCollisionPointRec(mouse, startRect);
    DrawRectangleRec(startRect, hoverStart ? Color{ 36, 68, 54, 245 } : Color{ 26, 44, 38, 230 });
    DrawRectangleLinesEx(startRect, 2.0f, ctaColor);

    const char* startPrompt = "START RUN  (ENTER)";
    int startW = MeasureText(startPrompt, 23);
    DrawText(startPrompt, (screenWidth - startW) / 2, (int)startRect.y + 12, 23, ctaColor);
    if (hoverStart && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        startClicked = true;
    }

    int rowW = panelW - 88;
    int halfW = (rowW - 14) / 2;
    Rectangle muteRect = { (float)(panelX + 44), (float)(panelY + 314), (float)halfW, 38.0f };
    bool hoverMute = CheckCollisionPointRec(mouse, muteRect);
    DrawRectangleRec(muteRect, hoverMute ? Color{ 38, 52, 74, 245 } : Color{ 24, 33, 48, 235 });
    DrawRectangleLinesEx(muteRect, 1.5f, hoverMute ? SKYBLUE : GRAY);
    const char* muteText = musicMuted ? "Music: OFF (M)" : "Music: ON (M)";
    int muteTextW = MeasureText(muteText, 16);
    DrawText(muteText, (int)muteRect.x + (halfW - muteTextW) / 2, (int)muteRect.y + 11, 16, musicMuted ? Color{ 245, 155, 155, 255 } : Color{ 155, 225, 255, 255 });
    if (hoverMute && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        muteClicked = true;
    }

    Rectangle quitRect = { (float)(panelX + 44 + halfW + 14), (float)(panelY + 314), (float)halfW, 38.0f };
    bool hoverQuit = CheckCollisionPointRec(mouse, quitRect);
    DrawRectangleRec(quitRect, hoverQuit ? Color{ 82, 34, 40, 245 } : Color{ 48, 24, 28, 235 });
    DrawRectangleLinesEx(quitRect, 1.5f, hoverQuit ? RED : MAROON);
    const char* quitText = "QUIT GAME (Q)";
    int quitTextW = MeasureText(quitText, 16);
    DrawText(quitText, (int)quitRect.x + (halfW - quitTextW) / 2, (int)quitRect.y + 11, 16, Color{ 255, 175, 175, 255 });
    if (hoverQuit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        quitClicked = true;
    }

    const char* audioStatus = musicMuted ? "Audio: SFX ON | Music: MUTED (Click or Press M)" : "Audio: SFX ON | Music: ACTIVE (Click or Press M)";
    int audioW = MeasureText(audioStatus, 16);
    DrawText(audioStatus, (screenWidth - audioW) / 2, panelY + panelH - 34, 16, Color{ 140, 175, 205, 255 });
}

static void StartContinuousRun(
    char mapData[MAX_ROWS][MAX_COLS],
    int& mapRows,
    int& mapCols,
    Player& player,
    float& cameraX,
    float& cameraY,
    float& targetCamX,
    float& targetCamY,
    Locker lockers[],
    int& lockerCount,
    Guard guards[],
    int& guardCount,
    int& activeGuardCount,
    PowerUp powerUps[],
    int& powerUpCount,
    float& levelTimer,
    int& objectiveNum,
    int& currentTargetEXP,
    int& loopCount,
    bool& objectiveBannerActive,
    float& objectiveBannerTimer,
    float& lastBonusAdded,
    float& hammerKOTimer,
    GameState& currentState
) {
    if (LoadFacilityMap(mapData, mapRows, mapCols)) {
        BakeStaticPropsIntoChunk(mapData, mapRows, mapCols, 1, mapCols - 2);
        FindPlayerSpawn(mapData, mapRows, mapCols, player);
        InitializeLockers(mapData, mapRows, mapCols, lockers, lockerCount);
        InitializeGuards(guards, guardCount, activeGuardCount, mapData, mapRows, mapCols, player.x, player.y, 1);
        InitializePowerUps(mapData, mapRows, mapCols, powerUps, powerUpCount);
    }

    cameraX = 0.0f;
    cameraY = 0.0f;
    targetCamX = 0.0f;
    targetCamY = 0.0f;
    player.score = 0;
    player.isSpeedBoosted = false;
    player.speedBoostTimer = 0.0f;
    player.hammerTimer = 0.0f;
    player.dirRow = 2;
    player.animFrame = 1;
    player.animTimer = 0.0f;
    player.isMoving = false;
    player.hammerSwingTimer = 0.0f;

    objectiveNum = 1;
    currentTargetEXP = 100;
    loopCount = 1;
    levelTimer = 24.0f;
    lastBonusAdded = 0.0f;
    objectiveBannerActive = false;
    objectiveBannerTimer = 0.0f;
    hammerKOTimer = 0.0f;

    currentState = PLAYING;
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Cipher Run - Endless Facility");
    SetExitKey(KEY_NULL);
    InitAudioDevice();
    SetTargetFPS(60);
    srand((unsigned int)time(0));

    int savedWinW = 1280;
    int savedWinH = 720;
    Vector2 initPos = GetWindowPosition();
    int savedWinX = (int)initPos.x;
    int savedWinY = (int)initPos.y;
    bool isBorderlessFullscreen = false;
    bool shouldQuit = false;

    int mon = GetCurrentMonitor();
    int monW = GetMonitorWidth(mon);
    int monH = GetMonitorHeight(mon);
    Vector2 monPos = GetMonitorPosition(mon);
    if (monW > 0 && monH > 0) {
        SetWindowState(FLAG_WINDOW_UNDECORATED);
        SetWindowPosition((int)monPos.x, (int)monPos.y);
        SetWindowSize(monW, monH);
        isBorderlessFullscreen = true;
    }

    GameAssets assets = { 0 };
    LoadGameAssets(assets);

    Texture2D customMenuBg = { 0 };
    bool hasCustomMenuBg = false;
    LoadTextureSafe("assets/menu_bg.png", customMenuBg, hasCustomMenuBg);

    Music customBgm = { 0 };
    bool hasCustomBgm = false;
    TryLoadCustomMusic(customBgm, hasCustomBgm);

    AudioStream fallbackBgmStream = { 0 };
    if (!hasCustomBgm) {
        fallbackBgmStream = GenerateRetroBgmStream();
    }

    float bgmPhase = 0.0f;
    float bgmStepTimer = 0.0f;
    int bgmStepIndex = 0;
    bool musicMuted = false;
    float uiTime = 0.0f;

    GameState currentState = MAIN_MENU;

    char mapData[MAX_ROWS][MAX_COLS] = { 0 };
    int mapRows = 0;
    int mapCols = 0;

    Player player;
    player.x = MAP_OFFSET_X + TILE_SIZE + 7.0f;
    player.y = MAP_OFFSET_Y + TILE_SIZE + 7.0f;
    player.speed = 4.75f;
    player.health = 100;
    player.score = 0;
    player.isSpeedBoosted = false;
    player.speedBoostTimer = 0.0f;
    player.hammerTimer = 0.0f;
    player.dirRow = 2;
    player.animFrame = 1;
    player.animTimer = 0.0f;
    player.isMoving = false;
    player.hammerSwingTimer = 0.0f;

    Locker lockers[MAX_LOCKERS];
    int lockerCount = 0;

    Guard guards[MAX_GUARDS];
    int guardCount = 0;
    int activeGuardCount = 0;

    PowerUp powerUps[MAX_POWERUPS];
    int powerUpCount = 0;

    float cameraX = 0.0f;
    float cameraY = 0.0f;
    float targetCamX = 0.0f;
    float targetCamY = 0.0f;
    float levelTimer = 24.0f;
    int objectiveNum = 1;
    int currentTargetEXP = 100;
    int loopCount = 1;
    bool objectiveBannerActive = false;
    float objectiveBannerTimer = 0.0f;
    float lastBonusAdded = 0.0f;
    float hammerKOTimer = 0.0f;
    bool gameOverSoundPlayed = false;

    while (!WindowShouldClose() && !shouldQuit) {
        float dt = GetFrameTime();
        if (dt > 0.033f) dt = 0.033f;
        uiTime += dt;

        bool altDown = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
        if (IsKeyPressed(KEY_F11) || (altDown && IsKeyPressed(KEY_ENTER))) {
            ToggleWindowedFullscreenSafe(savedWinW, savedWinH, savedWinX, savedWinY, isBorderlessFullscreen);
        }

        if (IsKeyPressed(KEY_M) && currentState != PAUSED) {
            musicMuted = !musicMuted;
        }

        if ((currentState == MAIN_MENU || currentState == PAUSED) && IsKeyPressed(KEY_Q)) {
            shouldQuit = true;
            continue;
        }

        if (hasCustomBgm) {
            SetMusicVolume(customBgm, musicMuted ? 0.0f : 0.35f);
            if (!IsMusicStreamPlaying(customBgm)) {
                PlayMusicStream(customBgm);
            }
            UpdateMusicStream(customBgm);
        }
        else {
            PumpRetroBgmStream(fallbackBgmStream, bgmPhase, bgmStepTimer, bgmStepIndex, musicMuted, dt);
        }

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        float viewScale = ComputeCrispIntegerScale(screenWidth, screenHeight, mapRows);

        bool startClicked = false;
        bool muteClicked = false;
        bool quitClicked = false;

        if (currentState == MAIN_MENU) {
            if (muteClicked) {
                musicMuted = !musicMuted;
            }
            if (!altDown && IsKeyPressed(KEY_ENTER)) {
                gameOverSoundPlayed = false;
                StartContinuousRun(
                    mapData, mapRows, mapCols,
                    player, cameraX, cameraY, targetCamX, targetCamY,
                    lockers, lockerCount,
                    guards, guardCount, activeGuardCount,
                    powerUps, powerUpCount,
                    levelTimer, objectiveNum, currentTargetEXP, loopCount,
                    objectiveBannerActive, objectiveBannerTimer, lastBonusAdded,
                    hammerKOTimer,
                    currentState
                );
            }
        }
        else if (currentState == PLAYING) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentState = PAUSED;
            }

            if (objectiveBannerActive) {
                objectiveBannerTimer -= dt;
                if (objectiveBannerTimer <= 0.0f) {
                    objectiveBannerActive = false;
                    objectiveBannerTimer = 0.0f;
                }
            }

            if (hammerKOTimer > 0.0f) {
                hammerKOTimer -= dt;
                if (hammerKOTimer < 0.0f) {
                    hammerKOTimer = 0.0f;
                }
            }

            if (player.hammerSwingTimer > 0.0f) {
                player.hammerSwingTimer -= dt;
                if (player.hammerSwingTimer < 0.0f) {
                    player.hammerSwingTimer = 0.0f;
                }
            }

            bool isAnyGuardGrabbing = false;
            for (int g = 0; g < activeGuardCount; g++) {
                if (!guards[g].isDefeated && guards[g].state == 2) {
                    isAnyGuardGrabbing = true;
                    break;
                }
            }

            float frameStep = dt * 60.0f;
            float moveSpeed = (player.isSpeedBoosted ? (player.speed * 1.70f) : player.speed) * frameStep;
            float oldX = player.x;
            float oldY = player.y;

            if (!isAnyGuardGrabbing) {
                bool movingThisFrame = false;
                if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
                    player.y -= moveSpeed;
                    player.dirRow = 3;
                    movingThisFrame = true;
                }
                if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
                    player.y += moveSpeed;
                    player.dirRow = 0;
                    movingThisFrame = true;
                }
                if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
                    player.x -= moveSpeed;
                    player.dirRow = 1;
                    movingThisFrame = true;
                }
                if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
                    player.x += moveSpeed;
                    player.dirRow = 2;
                    movingThisFrame = true;
                }

                player.isMoving = movingThisFrame;
                if (player.isMoving) {
                    player.animTimer += dt;
                    if (player.animTimer >= 0.10f) {
                        player.animTimer = 0.0f;
                        player.animFrame = (player.animFrame + 1) % 3;
                    }
                }
                else {
                    player.animFrame = 1;
                    player.animTimer = 0.0f;
                }

                Rectangle playerRectX = { player.x, oldY, 26.0f, 26.0f };
                for (int i = 0; i < mapRows; i++) {
                    for (int j = 0; j < mapCols; j++) {
                        if (mapData[i][j] == '#' || mapData[i][j] == 'W') {
                            Rectangle wallRect = {
                                MAP_OFFSET_X + (float)(j * TILE_SIZE),
                                MAP_OFFSET_Y + (float)(i * TILE_SIZE),
                                TILE_SIZE,
                                TILE_SIZE
                            };
                            if (CheckCollisionRecs(playerRectX, wallRect)) {
                                player.x = oldX;
                            }
                        }
                        else if (mapData[i][j] == 'T' && assets.hasPlantTex) {
                            Rectangle plantSolidRect = {
                                MAP_OFFSET_X + (float)(j * TILE_SIZE) + 7.0f,
                                MAP_OFFSET_Y + (float)(i * TILE_SIZE) + 7.0f,
                                TILE_SIZE - 14.0f,
                                TILE_SIZE - 14.0f
                            };
                            if (CheckCollisionRecs(playerRectX, plantSolidRect)) {
                                player.x = oldX;
                            }
                        }
                    }
                }

                Rectangle playerRectY = { player.x, player.y, 26.0f, 26.0f };
                for (int i = 0; i < mapRows; i++) {
                    for (int j = 0; j < mapCols; j++) {
                        if (mapData[i][j] == '#' || mapData[i][j] == 'W') {
                            Rectangle wallRect = {
                                MAP_OFFSET_X + (float)(j * TILE_SIZE),
                                MAP_OFFSET_Y + (float)(i * TILE_SIZE),
                                TILE_SIZE,
                                TILE_SIZE
                            };
                            if (CheckCollisionRecs(playerRectY, wallRect)) {
                                player.y = oldY;
                            }
                        }
                        else if (mapData[i][j] == 'T' && assets.hasPlantTex) {
                            Rectangle plantSolidRect = {
                                MAP_OFFSET_X + (float)(j * TILE_SIZE) + 7.0f,
                                MAP_OFFSET_Y + (float)(i * TILE_SIZE) + 7.0f,
                                TILE_SIZE - 14.0f,
                                TILE_SIZE - 14.0f
                            };
                            if (CheckCollisionRecs(playerRectY, plantSolidRect)) {
                                player.y = oldY;
                            }
                        }
                    }
                }

                if (player.x < MAP_OFFSET_X + 4.0f) {
                    player.x = MAP_OFFSET_X + 4.0f;
                }
            }

            int difficultyStep = loopCount + (objectiveNum - 1);

            if (!isAnyGuardGrabbing) {
                float triggerX = MAP_OFFSET_X + (float)((mapCols - 24) * TILE_SIZE);
                if (mapCols > 32 && player.x >= triggerX) {
                    AdvanceEndlessWorldSeamlessly(
                        mapData, mapRows, mapCols,
                        player, cameraX, targetCamX,
                        lockers, lockerCount,
                        guards, guardCount, activeGuardCount,
                        powerUps, powerUpCount,
                        objectiveNum, loopCount,
                        12
                    );
                    difficultyStep = loopCount + (objectiveNum - 1);
                }
            }

            int prevScore = player.score;
            int prevObjective = objectiveNum;
            bool prevBoost = player.isSpeedBoosted;
            float prevHammer = player.hammerTimer;
            float prevStun = (activeGuardCount > 0) ? guards[0].stunTimer : 0.0f;

            if (!isAnyGuardGrabbing) {
                CheckLockerInteraction(player, lockers, lockerCount);
                UpdatePowerUps(player, powerUps, powerUpCount, guards, activeGuardCount);
            }
            UpdateGuards(guards, activeGuardCount, player, currentState, mapData, mapRows, mapCols, difficultyStep);

            if (player.hammerTimer > 0.0f && player.score > prevScore) {
                hammerKOTimer = 1.5f;
                if (assets.hasSmashSound) PlaySound(assets.smashSound);
                else if (assets.hasHackSound) PlaySound(assets.hackSound);
            }
            else if (objectiveNum > prevObjective && assets.hasWinSound) {
                PlaySound(assets.winSound);
            }
            else if (player.score > prevScore && assets.hasHackSound) {
                PlaySound(assets.hackSound);
            }

            ProcessEndlessEXPObjective(
                player, objectiveNum, currentTargetEXP, levelTimer,
                activeGuardCount, guardCount,
                loopCount, objectiveBannerActive, objectiveBannerTimer, lastBonusAdded
            );

            bool nowStunned = (activeGuardCount > 0 && guards[0].stunTimer > prevStun);
            bool pickedHammer = (player.hammerTimer > prevHammer);
            if (((!prevBoost && player.isSpeedBoosted) || nowStunned || pickedHammer) && assets.hasPowerUpSound) {
                PlaySound(assets.powerUpSound);
            }

            float scaledScreenW = (float)screenWidth / viewScale;
            float scaledScreenH = (float)screenHeight / viewScale;

            targetCamX = player.x - (scaledScreenW * 0.40f);
            targetCamY = player.y - (scaledScreenH * 0.50f);

            float worldWidth = MAP_OFFSET_X * 2.0f + (float)(mapCols * TILE_SIZE);
            float worldHeight = MAP_OFFSET_Y * 2.0f + (float)(mapRows * TILE_SIZE);

            float maxCamX = worldWidth - scaledScreenW;
            float maxCamY = worldHeight - scaledScreenH;
            if (maxCamX < 0.0f) maxCamX = 0.0f;
            if (maxCamY < 0.0f) maxCamY = 0.0f;

            if (targetCamX < 0.0f) targetCamX = 0.0f;
            if (targetCamX > maxCamX) targetCamX = maxCamX;
            if (targetCamY < 0.0f) targetCamY = 0.0f;
            if (targetCamY > maxCamY) targetCamY = maxCamY;

            float camLerp = player.isSpeedBoosted ? (1.0f - powf(0.000004f, dt)) : (1.0f - powf(0.00004f, dt));
            cameraX += (targetCamX - cameraX) * camLerp;
            cameraY += (targetCamY - cameraY) * camLerp;

            if (!isAnyGuardGrabbing) {
                UpdateGameTimer(levelTimer, currentState, dt);
            }

            if (currentState == GAME_OVER && !gameOverSoundPlayed) {
                gameOverSoundPlayed = true;
                if (assets.hasGameOverSound) PlaySound(assets.gameOverSound);
            }
        }
        else if (currentState == PAUSED) {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_R)) currentState = PLAYING;
            if (IsKeyPressed(KEY_M)) currentState = MAIN_MENU;
        }
        else if (currentState == GAME_OVER) {
            if (IsKeyPressed(KEY_ENTER)) {
                gameOverSoundPlayed = false;
                StartContinuousRun(
                    mapData, mapRows, mapCols,
                    player, cameraX, cameraY, targetCamX, targetCamY,
                    lockers, lockerCount,
                    guards, guardCount, activeGuardCount,
                    powerUps, powerUpCount,
                    levelTimer, objectiveNum, currentTargetEXP, loopCount,
                    objectiveBannerActive, objectiveBannerTimer, lastBonusAdded,
                    hammerKOTimer,
                    currentState
                );
            }
        }

        BeginDrawing();

        if (currentState == MAIN_MENU) {
            DrawMainMenuUI(screenWidth, screenHeight, uiTime, musicMuted, hasCustomMenuBg, customMenuBg, startClicked, muteClicked, quitClicked);
        }
        else if (currentState == PLAYING) {
            ClearBackground(Color{ 14, 17, 24, 255 });

            float drawCamX = roundf(cameraX);
            float drawCamY = roundf(cameraY);
            float tileW = roundf(TILE_SIZE * viewScale);
            float tileH = roundf(TILE_SIZE * viewScale);

            // 1. Full-Screen Floor, Walls, Baked Windows ('W') & Baked Plants ('T')
            for (int i = 0; i < mapRows; i++) {
                for (int j = 0; j < mapCols; j++) {
                    float drawX = roundf((MAP_OFFSET_X + (float)(j * TILE_SIZE) - drawCamX) * viewScale);
                    float drawY = roundf((MAP_OFFSET_Y + (float)(i * TILE_SIZE) - drawCamY) * viewScale);

                    if (drawX + tileW >= 0.0f && drawX <= (float)screenWidth &&
                        drawY + tileH >= 0.0f && drawY <= (float)screenHeight) {
                        char tileChar = mapData[i][j];

                        if (tileChar == '#' || tileChar == 'W') {
                            Rectangle wallDest = { drawX, drawY, tileW, tileH };
                            DrawEntityVisual(assets.hasWallTex, assets.wallTex, wallDest, DARKGRAY, WHITE);

                            if (tileChar == 'W' && assets.hasWindowTex) {
                                float pad = roundf(3.0f * viewScale);
                                Rectangle winDest = { drawX + pad, drawY + pad, tileW - pad * 2.0f, tileH - pad * 2.0f };
                                DrawEntityVisual(true, assets.windowTex, winDest, SKYBLUE, WHITE);
                            }
                        }
                        else {
                            if (assets.hasFloorTex) {
                                Rectangle floorDest = { drawX, drawY, tileW, tileH };
                                DrawEntityVisual(assets.hasFloorTex, assets.floorTex, floorDest, Color{ 24, 28, 36, 255 }, WHITE);
                            }
                            else {
                                DrawRectangle((int)drawX, (int)drawY, (int)tileW - 1, (int)tileH - 1, Color{ 24, 28, 36, 255 });
                            }

                            if (tileChar == 'T' && assets.hasPlantTex) {
                                float plantSize = roundf(54.0f * viewScale);
                                float px = roundf((MAP_OFFSET_X + (float)(j * TILE_SIZE) - drawCamX - 7.0f) * viewScale);
                                float py = roundf((MAP_OFFSET_Y + (float)(i * TILE_SIZE) - drawCamY - 7.0f) * viewScale);
                                Rectangle plantDest = { px, py, plantSize, plantSize };
                                DrawEntityVisual(true, assets.plantTex, plantDest, GREEN, WHITE);
                            }
                        }
                    }
                }
            }

            // 2. Large Lockers
            for (int i = 0; i < lockerCount; i++) {
                float lw = roundf(60.0f * viewScale);
                float lh = roundf(66.0f * viewScale);
                Rectangle lockerDest = {
                    roundf((lockers[i].x - drawCamX - 10.0f) * viewScale),
                    roundf((lockers[i].y - drawCamY - 20.0f) * viewScale),
                    lw,
                    lh
                };
                if (lockerDest.x + lockerDest.width >= 0.0f && lockerDest.x <= (float)screenWidth &&
                    lockerDest.y + lockerDest.height >= 0.0f && lockerDest.y <= (float)screenHeight) {
                    if (lockers[i].isOpen) {
                        DrawEntityVisual(assets.hasLockerOpenTex, assets.lockerOpenTex, lockerDest, ORANGE, WHITE);
                    }
                    else {
                        DrawEntityVisual(assets.hasLockerClosedTex, assets.lockerClosedTex, lockerDest, BROWN, WHITE);
                    }
                }
            }

            // 3. Large Power-Ups
            for (int i = 0; i < powerUpCount; i++) {
                if (powerUps[i].isActive) {
                    float ps = roundf(50.0f * viewScale);
                    Rectangle pDest = {
                        roundf((powerUps[i].x - drawCamX - 12.0f) * viewScale),
                        roundf((powerUps[i].y - drawCamY - 12.0f) * viewScale),
                        ps,
                        ps
                    };
                    if (pDest.x + pDest.width >= 0.0f && pDest.x <= (float)screenWidth &&
                        pDest.y + pDest.height >= 0.0f && pDest.y <= (float)screenHeight) {
                        if (powerUps[i].type == 1 && assets.hasPowerUpSpeedTex) {
                            DrawEntityVisual(true, assets.powerUpSpeedTex, pDest, SKYBLUE, WHITE);
                        }
                        else if (powerUps[i].type == 2 && assets.hasPowerUpStunTex) {
                            DrawEntityVisual(true, assets.powerUpStunTex, pDest, PURPLE, WHITE);
                        }
                        else if (powerUps[i].type == 3 && assets.hasPowerUpHammerTex) {
                            DrawEntityVisual(true, assets.powerUpHammerTex, pDest, GOLD, WHITE);
                        }
                        else if (assets.hasPowerUpTex) {
                            Color pTint = SKYBLUE;
                            if (powerUps[i].type == 2) pTint = VIOLET;
                            else if (powerUps[i].type == 3) pTint = GOLD;
                            DrawEntityVisual(true, assets.powerUpTex, pDest, GOLD, pTint);
                        }
                        else {
                            Color pFallback = SKYBLUE;
                            if (powerUps[i].type == 2) pFallback = PURPLE;
                            else if (powerUps[i].type == 3) pFallback = GOLD;
                            DrawRectangle((int)pDest.x, (int)pDest.y, (int)pDest.width, (int)pDest.height, pFallback);
                        }
                    }
                }
            }

            // 4. Guards
            for (int i = 0; i < activeGuardCount; i++) {
                if (guards[i].isDefeated) continue;

                float gw = roundf(60.0f * viewScale);
                float gh = roundf(68.0f * viewScale);
                Rectangle gDest = {
                    floorf((guards[i].x - drawCamX - 15.0f) * viewScale),
                    floorf((guards[i].y - drawCamY - 24.0f) * viewScale),
                    gw,
                    gh
                };
                if (gDest.x + gDest.width >= 0.0f && gDest.x <= (float)screenWidth &&
                    gDest.y + gDest.height >= 0.0f && gDest.y <= (float)screenHeight) {
                    Color gTint = (guards[i].stunTimer > 0.0f) ? GOLD : WHITE;
                    DrawSpriteFrame3x4(assets.hasGuardTex, assets.guardTex, guards[i].animFrame, guards[i].dirRow, gDest, RED, gTint);
                }
            }

            // 5. Player + Equipped Hammer Sprite
            float pw = roundf(60.0f * viewScale);
            float ph = roundf(68.0f * viewScale);
            Rectangle playerDest = {
                roundf((player.x - drawCamX - 15.0f) * viewScale),
                roundf((player.y - drawCamY - 24.0f) * viewScale),
                pw,
                ph
            };
            Color pTint = WHITE;
            if (player.hammerTimer > 0.0f) pTint = GOLD;
            else if (player.isSpeedBoosted) pTint = LIME;

            DrawSpriteFrame3x4(assets.hasPlayerTex, assets.playerTex, player.animFrame, player.dirRow, playerDest, BLUE, pTint);
            DrawPlayerEquippedHammer(assets, player, playerDest, viewScale);

            // 6. Clean Floating HUD Overlay (No Loop #)
            DrawRectangle(14, 10, screenWidth - 28, 56, Color{ 10, 13, 20, 185 });
            DrawText(TextFormat("Goal #%d: %d EXP", objectiveNum, currentTargetEXP), 28, 16, 22, GREEN);
            DrawText(TextFormat("EXP: %d", player.score), 245, 16, 22, SKYBLUE);
            DrawText(TextFormat("Guards: %d/%d", activeGuardCount, MAX_ACTIVE_GUARDS), 395, 16, 21, ORANGE);
            DrawText(TextFormat("Time: %.1fs", levelTimer), screenWidth - 195, 16, 22, YELLOW);

            if (objectiveBannerActive) {
                DrawText(TextFormat("NICE! +%.1fs added -> Next target: %d EXP", lastBonusAdded, currentTargetEXP), 28, 42, 18, LIME);
            }
            else if (hammerKOTimer > 0.0f) {
                DrawText(TextFormat("BONK! Guard down (+%d EXP)", HAMMER_KILL_EXP), 28, 42, 18, GOLD);
            }
            else if (player.hammerTimer > 0.0f) {
                DrawText(TextFormat("HAMMER TIME: %.1fs — Bonk a guard for +%d EXP!", player.hammerTimer, HAMMER_KILL_EXP), 28, 42, 18, GOLD);
            }
            else if (player.isSpeedBoosted) {
                DrawText(TextFormat("TURBO BOOTS: %.1fs — Zooming!", player.speedBoostTimer), 28, 42, 18, LIME);
            }
            else {
                DrawText(musicMuted ? "Music OFF (M) | ESC: Pause | F11: Fullscreen" : "Music ON (M) | ESC: Pause | F11: Fullscreen", 28, 42, 18, LIGHTGRAY);
            }
        }
        else if (currentState == PAUSED) {
            ClearBackground(Color{ 14, 17, 24, 255 });
            DrawText("PAUSED — TAKE A BREATHER", screenWidth / 2 - 220, screenHeight / 2 - 85, 34, YELLOW);
            DrawText("ESC or R : Resume Run", screenWidth / 2 - 125, screenHeight / 2 - 20, 22, LIGHTGRAY);
            DrawText("M : Back to Main Menu", screenWidth / 2 - 135, screenHeight / 2 + 16, 22, LIGHTGRAY);
            DrawText("Q : Quit Game", screenWidth / 2 - 90, screenHeight / 2 + 52, 22, Color{ 255, 165, 165, 255 });
            DrawText("F11 : Toggle Fullscreen", screenWidth / 2 - 130, screenHeight / 2 + 88, 20, SKYBLUE);
        }
        else if (currentState == GAME_OVER) {
            ClearBackground(Color{ 14, 17, 24, 255 });
            DrawText("BUSTED / TIME'S UP!", screenWidth / 2 - 185, screenHeight / 2 - 80, 36, RED);
            DrawText(TextFormat("Goal Reached: #%d  |  Total EXP: %d", objectiveNum, player.score), screenWidth / 2 - 220, screenHeight / 2 - 20, 22, LIGHTGRAY);
            DrawText("Press ENTER to Run Again", screenWidth / 2 - 145, screenHeight / 2 + 35, 22, YELLOW);
        }

        EndDrawing();

        if (currentState == MAIN_MENU) {
            if (muteClicked) {
                musicMuted = !musicMuted;
            }
            if (quitClicked) {
                shouldQuit = true;
            }
            if (startClicked) {
                gameOverSoundPlayed = false;
                StartContinuousRun(
                    mapData, mapRows, mapCols,
                    player, cameraX, cameraY, targetCamX, targetCamY,
                    lockers, lockerCount,
                    guards, guardCount, activeGuardCount,
                    powerUps, powerUpCount,
                    levelTimer, objectiveNum, currentTargetEXP, loopCount,
                    objectiveBannerActive, objectiveBannerTimer, lastBonusAdded,
                    hammerKOTimer,
                    currentState
                );
            }
        }
    }

    if (hasCustomBgm) {
        StopMusicStream(customBgm);
        UnloadMusicStream(customBgm);
    }
    else if (fallbackBgmStream.buffer != nullptr) {
        UnloadAudioStream(fallbackBgmStream);
    }

    if (hasCustomMenuBg && customMenuBg.id > 0) {
        UnloadTexture(customMenuBg);
    }

    UnloadGameAssets(assets);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}