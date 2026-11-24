# Cipher Run

## Project Purpose and Overview

**Cipher Run** is a 2D side-scrolling endless facility infiltration game written in C++ using raylib. The player is a spy who infiltrates the enemy facility whose corridor layout is loaded from `level1.txt`, hacks closed lockers for EXP (granting `+50 EXP` when a document is present, 50/50 chance hehe), collects power-ups  for Speed Boost (`1.70x` movement speed for `4.2s`), stun clock (`3.2s` guard stun), or Hammer (`6.5s` active duration allowing to kill guards for `+35 EXP`), and races against a countdown timer while advancing EXP goals.

As the player moves forward past the corridor trigger, the facility world seamlessly shifts and  upcoming corridor chunks come  while scaling active guards and objective targets.

You'll have fun playing this. (GOOD LUCK)
### Screenshots

Add your project screenshots under `screenshots/` and update the paths below as needed:

* Main Menu: `![Main Menu](screenshots/main_menu.png)`
* Gameplay & HUD: screenshots/gameplay.png
* Power-Ups & Pause Screen: `![Pause Screen](screenshots/pause_menu.png)`

---

## Prerequisites

* **Operating System**: Windows (primary setup for Visual Studio / MinGW raylib builds) or a desktop environment supported by raylib.
* **Compiler / IDE**: A C++17-compatible compiler such as MSVC (Visual Studio) or `g++` (MinGW).
* **External Library**: **raylib** (placed in the project's `lib/` directory, with its names and versions documented in `libraries.txt`).
* **Game Data & Assets**: `level1.txt` and the `assets/` directory containing sprites, menu background, and audio files.

---

## Installation and Setup Instructions

1. Place all project source files (`main.cpp`, `structs.h`, `lockers.h`, `guards_powerups.h`, `game_logic.h`) in the project root directory e.g  a folder named cipher-run.
2. Place the **raylib** library files (headers and `.lib`/`.a` static or import libraries matching your architecture, e.g., x64) inside the `lib/`.
3. `libraries.txt` in the repository lists the names and versions of the libraries used.
4. Keep `level1.txt` and the `assets/` folder alongside the source files so they are accessible when running the executable (.exe file).

---

## Steps to Compile and Run the Project on a New PC

1. Copy the complete project folder (including `lib/`, `assets/`, `level1.txt`, `libraries.txt`, `README.md`, and all `.h`/`.cpp` files) onto the new PC.
2. Open the project in your IDE (such as Visual Studio) or terminal:
   * **Visual Studio**:
     1. Add `main.cpp` and all `.h` files (`structs.h`, `lockers.h`, `guards_powerups.h`, `game_logic.h`) to the project.
     2. Under **Project Properties -> C/C++ -> General -> Additional Include Directories**, point to the raylib `include` path inside `lib/`.
     3. Under **Project Properties -> Linker -> General -> Additional Library Directories**, point to the raylib library path inside `lib/`.
     4. Under **Project Properties -> Linker -> Input -> Additional Dependencies**, add `raylib.lib` along with required Windows system libraries (`winmm.lib`, `gdi32.lib`, `opengl32.lib`).
     5. Build and run (`Ctrl + F5`).
   * **Command Line (`g++` with `lib/` folder layout)**:
     ```bash
     g++ main.cpp -Ilib/include -Llib/lib -o CipherRun -lraylib -lopengl32 -lgdi32 -lwinmm -std=c++17
     ./CipherRun
     ```
3. Run the compiled binary from the project root so relative paths to `level1.txt` and `assets/` resolve properly.

4. You can also just simply create a new folder place all cpp and .h files in it along with executable and assests folder and lib folder and just run the exedcutable file.
---

## Any Specific Configuration or Environment Setup

* **Working Directory**: The game loads `level1.txt` and `assets/*` (`wall.png`, `floor.png`, `window.png`, `plant.png`, `player.png`, `guard.png`, `locker_closed.png`, `locker_open.png`, `powerup*.png`, `menu_bg.png`, `.wav` SFX, and optional `bgm.*` / `music.*` streams) using relative paths. Ensure the working directory is the folder containing `level1.txt` and `assets/`.
* **Display & Controls Configuration**:
  * Starts in borderless fullscreen on the active monitor and supports toggling fullscreen via `F11` or `ALT + ENTER`.
  * `ESC` is mapped to pause  rather than immediately closing the window; press `ESC` or `R` to resume, `M` to return to the main menu, and `Q` (or the menu/pause quit option) to exit (you can only quit after pausing the game or in main menu not from the running game).
  * `M` toggles background music mute/unmute when not on the pause screen.

---

## Troubleshooting Common Issues

* **Map fails to load / empty facility on start**: Verify that `level1.txt` exists in the current working directory and has at least one non-empty row.
* **Missing textures or fallback colored blocks/sprites**: Check that image files exist inside `assets/` with exact filenames (`wall.png`, `floor.png`, `window.png`, `plant.png`, `player.png`, `guard.png`, `locker_closed.png`, `locker_open.png`, `powerup.png`, `powerup_speed.png`, `powerup_stun.png`, `powerup_hammer.png`, `menu_bg.png`). When a texture is missing, the game safely falls back to colored shapes or retro procedural audio where applicable.
* **No custom background music or SFX playing**: Confirm  that audio files are placed in `assets/` (`bgm.ogg` / `bgm.wav` / `bgm.mp3` / `music.ogg` / `music.mp3` and `hack.wav`, `powerup.wav`, `gameover.wav`, `win.wav`, `smash.wav`), and check that `musicMuted` is not toggled off (`M`).
* **Linker or header errors during compilation**: Verify that the raylib headers and library binaries in `lib/` match your compiler's target architecture (e.g., x64 vs x86) and that `libraries.txt` reflects the exact raylib version in `lib/`.
