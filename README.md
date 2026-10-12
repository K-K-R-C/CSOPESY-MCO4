# CSOPESY MCO4 - Desktop OS Emulator
Group: 8   
Members: 
Alain Zuriel Marcos
Karl Kristoffer Camato
Ryan James Malapitan
Trisha Bendol

ENTRY CLASS FILE: src/main.cpp (contains int main)

## REQUIREMENTS: CMake 3.20+, a C++17 compiler, internet on first build (downloads GLFW 3.4 + Dear ImGui 1.91.9)

## BUILD & RUN
  Visual Studio 2022: File > Open > Folder (repo root). Select target "csopesy_os.exe", press Run/Debug.
  CLion: open the repo root folder, press Run/Debug.
  Terminal: cmake -S . -B build && cmake --build build --config Debug
            Windows: build\Debug\csopesy_os.exe   Linux/macOS: ./build/csopesy_os

## USAGE: BIOS -> splash -> desktop. SPACE skips boot screens. Quit ONLY with the PWR button.