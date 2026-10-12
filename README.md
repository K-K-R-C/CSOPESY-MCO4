# CSOPESY Semi-Major Output 2 - Desktop-Style OS Mock-up
 
This program is a desktop-style OS mock-up built with GLFW, OpenGL 3, and Dear ImGui. It simulates a minimal graphical OS shell with a boot sequence, an animated desktop, a taskbar, and application windows, all drawn every frame in immediate mode.
 
 
## Group Developers
 - Bendol, Trisha Mae
 - Camato, Karl Kristoffer
 - Malapitan, Ryan James
 - Marcos, Alain Zuriel
## Entry Point
Main source file: **`src/main.cpp`** - contains the `main()` function as the program's entry point.
 
 
## Repository Contents
 - `src/main.cpp` - entry point, window setup, main loop, boot/desktop/shutdown stages
 - `src/Boot.cpp`, `src/Boot.h` - BIOS screen and splash screen
 - `src/Desktop.cpp`, `src/Desktop.h` - compositor: wallpaper, clock, tray (VOL / NET / PWR), window registry
 - `src/Taskbar.cpp`, `src/Taskbar.h` - taskbar and app buttons
 - `src/Applications.cpp`, `src/Applications.h` - File Explorer and System Settings
 - `src/TaskManager.cpp`, `src/TaskManager.h` - Task Manager
 - `assets/` - asset files
 - `CMakeLists.txt` - build configuration
## Requirements
 - C++17-compatible compiler
 - CMake 3.20 or later
 - Git (CMake uses it to download the libraries)
 - An internet connection on the first build only
 - OpenGL 3.0 or later (provided by your graphics driver)
GLFW (3.4) and Dear ImGui (v1.91.9) do not need to be installed manually. CMake downloads them automatically the first time the project is configured.
 
 
## Building & Running
**Windows / macOS / Linux (CMake)**
```
cmake -S . -B build
cmake --build build
```
 
Then run the generated executable `csopesy_os` (`csopesy_os.exe` on Windows) from inside the `build` folder (or its `Debug`/`Release` subfolder with Visual Studio).
 
If your IDE (Visual Studio, CLion, VS Code with CMake Tools) opens `CMakeLists.txt` directly, you can also just press Run/Debug on the `csopesy_os` target.
 
Every `.cpp` file under `src/` is picked up automatically, so there is no source list to edit in `CMakeLists.txt`.
 
 
## Features
**Desktop**
 - Full-screen animated wallpaper (sky, drifting clouds, and hills) drawn only with ImGui draw commands
 - Real-time clock in the top-right corner, updated every frame
 - **PWR** button in the bottom-right tray for shutting down
**Taskbar**
 - Fixed, full-width bar at the bottom of the screen
 - **Files**, **Settings**, and **Tasks** buttons toggle their windows open and closed
 - Open apps are highlighted with a blue background and an underline
**Applications**
 - **File Explorer** - fake in-memory file system with back/forward/up, breadcrumbs, search, list and grid views, and right-click Open/Properties
 - **System Settings** - System, Display, Personalization, Network, Sound, and About pages
 - **Task Manager** - Processes table with dummy CPU and memory values, plus summary totals
**Tray**
 - **VOL** - volume slider and mute
 - **NET** - Wi-Fi and Bluetooth toggles
 - **PWR** - graceful shutdown
## Controls
| Input | Action |
|---|---|
| Taskbar button | Opens or closes the matching app window |
| Window **X** button | Closes that app window only |
| **PWR** button | Starts the shutdown sequence and closes the program |
| `SPACE` | Skips the current boot stage (BIOS or splash) |
 
 
## Notes
 - The program can only be closed with the **PWR** button. The window's close button and Alt+F4 are ignored.
 - Startup runs through stages: BIOS -> Splash -> Desktop -> Shutting Down.
 - All data in the File Explorer, Task Manager, and Network page is placeholder data. No real files or processes are read.
 - System Settings changes apply to the whole OS:
    - Theme and accent color change the look of every window
    - Time zone and 12/24-hour setting change the desktop clock
    - Wallpaper style and colors change the desktop background
    - Brightness dims the whole screen
    - **Show FPS overlay** shows the frame rate in the top-left corner
    - Resolution resizes the application window
    - UI scale is applied when the slider is released
 - **Reset all settings** (About page) restores every setting to its default
 - The default wallpaper is drawn in code, so no image file is required to run the program
