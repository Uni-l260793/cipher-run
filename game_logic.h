#pragma once
#include "raylib.h"
#include "structs.h"
#include "lockers.h"
#include "guards_powerups.h"
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cmath>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

inline Sound GenerateRetroSfx(int mode) {
    int sampleRate = 44100;
    float duration = 0.16f;
    if (mode == 1) duration = 0.14f;      // Hack / EXP
    else if (mode == 2) duration = 0.22f; // Power-up
    else if (mode == 3) duration = 0.48f; // Game over
    else if (mode == 4) duration = 0.34f; // Objective clear
    else if (mode == 5) duration = 0.18f; // Hammer smash

    int frameCount = (int)(duration * (float)sampleRate);
    Wave wave = { 0 };
    wave.frameCount = (unsigned int)frameCount;
    wave.sampleRate = (unsigned int)sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = malloc((size_t)frameCount * sizeof(short));
    if (wave.data == nullptr) return Sound{ 0 };

    short* samples = (short*)wave.data;
    for (int i = 0; i < frameCount; i++) {
        float t = (float)i / (float)sampleRate;
        float progress = (float)i / (float)frameCount;
        float env = 1.0f - progress;
        float freq = 440.0f;

        if (mode == 1) freq = 520.0f + progress * 380.0f;
        else if (mode == 2) freq = 360.0f + sinf(progress * PI * 2.0f) * 240.0f + progress * 220.0f;
        else if (mode == 3) freq = 310.0f - progress * 210.0f;
        else if (mode == 4) freq = (progress < 0.5f) ? 587.33f : 880.0f;
        else if (mode == 5) freq = 240.0f - progress * 140.0f;

        float waveVal = (sinf(2.0f * PI * freq * t) >= 0.0f) ? 0.55f : -0.55f;
        waveVal = waveVal * 0.65f + sinf(2.0f * PI * freq * t) * 0.35f;
        samples[i] = (short)(waveVal * env * 14000.0f);
    }

    Sound snd = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return snd;
}

inline AudioStream GenerateRetroBgmStream() {
    if (!IsAudioDeviceReady()) return AudioStream{ 0 };
    int sampleRate = 44100;
    AudioStream stream = LoadAudioStream((unsigned int)sampleRate, 16, 1);
    if (stream.buffer != nullptr) {
        SetAudioStreamVolume(stream, 0.22f);
        PlayAudioStream(stream);
    }
    return stream;
}

inline void PumpRetroBgmStream(AudioStream& stream, float& bgmPhase, float& stepTimer, int& stepIndex, bool musicMuted, float dt) {
    (void)dt;
    if (!IsAudioDeviceReady() || stream.buffer == nullptr) return;
    SetAudioStreamVolume(stream, musicMuted ? 0.0f : 0.22f);
    if (!IsAudioStreamPlaying(stream)) PlayAudioStream(stream);

    const int notes[16] = {
        110, 110, 146, 110, 164, 146, 130, 110,
        98,  98,  146, 130, 164, 196, 164, 146
    };

    int guardLoops = 0;
    while (IsAudioStreamProcessed(stream) && guardLoops < 8) {
        guardLoops++;
        const int chunkFrames = 735;
        short buffer[735];
        float sampleRate = 44100.0f;

        for (int i = 0; i < chunkFrames; i++) {
            stepTimer += 1.0f / sampleRate;
            if (stepTimer >= 0.22f) {
                stepTimer -= 0.22f;
                stepIndex = (stepIndex + 1) % 16;
            }
            float freq = (float)notes[stepIndex];
            bgmPhase += (2.0f * PI * freq) / sampleRate;
            if (bgmPhase > 2.0f * PI) bgmPhase -= 2.0f * PI;

            float sub = sinf(bgmPhase * 0.5f) * 0.35f;
            float pulse = (sinf(bgmPhase) >= 0.0f) ? 0.22f : -0.22f;
            float gate = 0.55f + 0.45f * sinf((stepTimer / 0.22f) * PI);
            buffer[i] = (short)((sub + pulse) * gate * 7000.0f);
        }
        UpdateAudioStream(stream, buffer, chunkFrames);
    }
}

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

inline void LoadSoundSafe(const char* path, Sound& snd, bool& loadedFlag, int fallbackMode) {
    loadedFlag = false;
    if (IsAudioDeviceReady() && FileExists(path)) {
        snd = LoadSound(path);
        if (snd.frameCount > 0) loadedFlag = true;
    }
    if (!loadedFlag && IsAudioDeviceReady()) {
        snd = GenerateRetroSfx(fallbackMode);
        if (snd.frameCount > 0) loadedFlag = true;
    }
}

inline void LoadGameAssets(GameAssets& assets) {
    LoadTextureSafe("assets/wall.png", assets.wallTex, assets.hasWallTex);
    LoadTextureSafe("assets/floor.png", assets.floorTex, assets.hasFloorTex);
    LoadTextureSafe("assets/window.png", assets.windowTex, assets.hasWindowTex);
    LoadTextureSafe("assets/plant.png", assets.plantTex, assets.hasPlantTex);
    LoadTextureSafe("assets/player.png", assets.playerTex, assets.hasPlayerTex);
    LoadTextureSafe("assets/guard.png", assets.guardTex, assets.hasGuardTex);
    LoadTextureSafe("assets/locker_closed.png", assets.lockerClosedTex, assets.hasLockerClosedTex);
    LoadTextureSafe("assets/locker_open.png", assets.lockerOpenTex, assets.hasLockerOpenTex);
    LoadTextureSafe("assets/powerup.png", assets.powerUpTex, assets.hasPowerUpTex);
    LoadTextureSafe("assets/powerup_speed.png", assets.powerUpSpeedTex, assets.hasPowerUpSpeedTex);
    LoadTextureSafe("assets/powerup_stun.png", assets.powerUpStunTex, assets.hasPowerUpStunTex);
    LoadTextureSafe("assets/powerup_hammer.png", assets.powerUpHammerTex, assets.hasPowerUpHammerTex);
    LoadTextureSafe("assets/menu_bg.png", assets.menuBgTex, assets.hasMenuBgTex);

    LoadSoundSafe("assets/hack.wav", assets.hackSound, assets.hasHackSound, 1);
    LoadSoundSafe("assets/powerup.wav", assets.powerUpSound, assets.hasPowerUpSound, 2);
    LoadSoundSafe("assets/gameover.wav", assets.gameOverSound, assets.hasGameOverSound, 3);
    LoadSoundSafe("assets/win.wav", assets.winSound, assets.hasWinSound, 4);
    LoadSoundSafe("assets/smash.wav", assets.smashSound, assets.hasSmashSound, 5);
}

inline void UnloadGameAssets(GameAssets& assets) {
    if (assets.hasWallTex) UnloadTexture(assets.wallTex);
    if (assets.hasFloorTex) UnloadTexture(assets.floorTex);
    if (assets.hasWindowTex) UnloadTexture(assets.windowTex);
    if (assets.hasPlantTex) UnloadTexture(assets.plantTex);
    if (assets.hasPlayerTex) UnloadTexture(assets.playerTex);
    if (assets.hasGuardTex) UnloadTexture(assets.guardTex);
    if (assets.hasLockerClosedTex) UnloadTexture(assets.lockerClosedTex);
    if (assets.hasLockerOpenTex) UnloadTexture(assets.lockerOpenTex);
    if (assets.hasPowerUpTex) UnloadTexture(assets.powerUpTex);
    if (assets.hasPowerUpSpeedTex) UnloadTexture(assets.powerUpSpeedTex);
    if (assets.hasPowerUpStunTex) UnloadTexture(assets.powerUpStunTex);
    if (assets.hasPowerUpHammerTex) UnloadTexture(assets.powerUpHammerTex);
    if (assets.hasMenuBgTex) UnloadTexture(assets.menuBgTex);

    if (assets.hasHackSound) UnloadSound(assets.hackSound);
    if (assets.hasPowerUpSound) UnloadSound(assets.powerUpSound);
    if (assets.hasGameOverSound) UnloadSound(assets.gameOverSound);
    if (assets.hasWinSound) UnloadSound(assets.winSound);
    if (assets.hasSmashSound) UnloadSound(assets.smashSound);
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
    for (int r = 0; r < MAX_ROWS; r++)
        for (int c = 0; c < MAX_COLS; c++) mapData[r][c] = '\0';

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

inline float GetNextObjectiveBonusTime(int nextObjectiveNum) {
    float bonus = 16.0f - (float)(nextObjectiveNum - 1) * 2.0f;
    return (bonus < 7.5f) ? 7.5f : bonus;
}

inline void BakeStaticPropsIntoChunk(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int startCol, int endCol) {
    if (mapRows < 4 || mapCols < 6) return;
    if (startCol < 1) startCol = 1;
    if (endCol > mapCols - 2) endCol = mapCols - 2;

    for (int c = startCol; c <= endCol; c++) {
        if (c % 5 == 2 && mapData[0][c] == '#' && mapData[1][c] != '#') mapData[0][c] = 'W';
    }
    for (int r = 1; r < mapRows - 1; r++) {
        for (int c = startCol; c <= endCol; c++) {
            if (mapData[r][c] == '.' && (c % 14 == 7) && (r == 1 || r == mapRows - 2)) mapData[r][c] = 'T';
        }
    }
}

inline void MutateForwardCorridorChunk(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols, int startCol, int endCol, int difficultyStep) {
    if (mapRows < 4 || mapCols < 6 || startCol >= endCol) return;
    int coverChance = 28 + (difficultyStep > 5 ? 5 : difficultyStep) * 3;
    int laneA = 1 + (rand() % (mapRows - 2));
    int laneB = (laneA + 2 <= mapRows - 2) ? (laneA + 2) : (1 + ((laneA + 1) % (mapRows - 2)));

    for (int c = startCol; c < endCol; c++) {
        int colPhase = (c - startCol) % 6;
        bool isJunction = (colPhase == 0 || colPhase == 3);
        for (int r = 1; r < mapRows - 1; r++) {
            if (mapData[r][c] == 'S') mapData[r][c] = '.';
            if (isJunction || r == laneA || r == laneB) mapData[r][c] = '.';
            else mapData[r][c] = (rand() % 100 < coverChance) ? '#' : '.';
        }
        if (!isJunction && (c % 3 == 1)) {
            int lr = 1 + (rand() % (mapRows - 2));
            if (mapData[lr][c] == '.') mapData[lr][c] = 'L';
        }
        if (!isJunction && (c % 6 == 4) && (rand() % 100 < 60)) {
            int pr = 1 + (rand() % (mapRows - 2));
            if (mapData[pr][c] == '.') mapData[pr][c] = 'P';
        }
    }
    BakeStaticPropsIntoChunk(mapData, mapRows, mapCols, startCol, endCol);
}

inline void AppendLockersInColumnRange(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int colStart, int colEnd, Locker lockers[], int& lockerCount) {
    for (int i = 1; i < mapRows - 1; i++) {
        for (int j = colStart; j < colEnd; j++) {
            if (mapData[i][j] == 'L' && lockerCount < MAX_LOCKERS) {
                lockers[lockerCount] = { MAP_OFFSET_X + j * TILE_SIZE + 5.0f, MAP_OFFSET_Y + i * TILE_SIZE + 5.0f, false, (rand() % 2 == 0) };
                lockerCount++;
            }
        }
    }
}

inline void AppendPowerUpsInColumnRange(char mapData[MAX_ROWS][MAX_COLS], int mapRows, int colStart, int colEnd, PowerUp powerUps[], int& powerUpCount) {
    for (int i = 1; i < mapRows - 1; i++) {
        for (int j = colStart; j < colEnd; j++) {
            if (mapData[i][j] == 'P' && powerUpCount < MAX_POWERUPS) {
                int rType = rand() % 10;
                int t = (rType < 4) ? 1 : ((rType < 7) ? 2 : 3);
                powerUps[powerUpCount] = { MAP_OFFSET_X + j * TILE_SIZE + 8.0f, MAP_OFFSET_Y + i * TILE_SIZE + 8.0f, t, true };
                powerUpCount++;
            }
        }
    }
}

inline void AdvanceEndlessWorldSeamlessly(
    char mapData[MAX_ROWS][MAX_COLS], int mapRows, int mapCols,
    Player& player, float& cameraX, float& targetCamX,
    Locker lockers[], int& lockerCount,
    Guard guards[], int& guardCount, int& activeGuardCount,
    PowerUp powerUps[], int& powerUpCount,
    int objectiveNum, int& loopCount, int shiftCols
) {
    if (mapCols <= shiftCols + 8) return;
    loopCount++;
    float shiftX = (float)(shiftCols * TILE_SIZE);
    int keepCols = mapCols - shiftCols;

    for (int r = 0; r < mapRows; r++) {
        for (int c = 0; c < keepCols; c++) mapData[r][c] = mapData[r][c + shiftCols];
        for (int c = keepCols; c < mapCols; c++) mapData[r][c] = (r == 0 || r == mapRows - 1 || c == mapCols - 1) ? '#' : '.';
    }

    int difficultyStep = loopCount + (objectiveNum - 1);
    int mutateStart = keepCols + 3;
    MutateForwardCorridorChunk(mapData, mapRows, mapCols, mutateStart, mapCols - 1, difficultyStep);

    player.x -= shiftX;
    cameraX -= shiftX;
    targetCamX -= shiftX;
    if (cameraX < 0.0f) cameraX = 0.0f;
    if (targetCamX < 0.0f) targetCamX = 0.0f;

    ShiftLockersWorldX(lockers, lockerCount, shiftX);
    AppendLockersInColumnRange(mapData, mapRows, mutateStart, mapCols - 1, lockers, lockerCount);

    ShiftPowerUpsWorldX(powerUps, powerUpCount, shiftX);
    AppendPowerUpsInColumnRange(mapData, mapRows, mutateStart, mapCols - 1, powerUps, powerUpCount);

    ShiftGuardsWorldX(guards, guardCount, shiftX);
    int desired = INITIAL_GUARDS + ((objectiveNum - 1) / 2) + 1;
    if (desired > MAX_ACTIVE_GUARDS) desired = MAX_ACTIVE_GUARDS;
    if (desired > guardCount) desired = guardCount;
    activeGuardCount = desired;
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

inline void ProcessEndlessEXPObjective(
    Player& player,
    int& objectiveNum,
    int& currentTargetEXP,
    float& levelTimer,
    int& activeGuardCount,
    int totalPoolGuards,
    int loopCount,
    bool& objectiveBannerActive,
    float& objectiveBannerTimer,
    float& lastBonusAdded
) {
    (void)loopCount;
    if (player.score >= currentTargetEXP) {
        objectiveNum++;
        currentTargetEXP += 100 + (objectiveNum - 2) * 20;
        float bonusTime = GetNextObjectiveBonusTime(objectiveNum);
        levelTimer += bonusTime;
        lastBonusAdded = bonusTime;

        int desired = INITIAL_GUARDS + ((objectiveNum - 1) / 2) + 1;
        if (desired > MAX_ACTIVE_GUARDS) desired = MAX_ACTIVE_GUARDS;
        if (desired > totalPoolGuards) desired = totalPoolGuards;
        activeGuardCount = desired;

        objectiveBannerActive = true;
        objectiveBannerTimer = 2.5f;
    }
}