#pragma once
// ---------------------------------------------------------------------------
// Both derive from IWindow (see Desktop.h) and start CLOSED. The taskbar opens
// them with:   desktop.toggle("files");   desktop.toggle("settings");
// ---------------------------------------------------------------------------
#include "Desktop.h"
#include <memory>

// Shared state set by System Settings. Other members MAY read this every frame
// (all optional - the settings window works even if nobody reads them):
//   use24HourClock  -> Desktop::drawClock()
//   wallpaperStyle / wallpaper1 / wallpaper2 -> Desktop::drawWallpaper()
//   brightness      -> draw a black full-screen rect with alpha (1 - brightness/100)
//   showFps         -> draw an FPS overlay (not working yet)
struct AppSettings {
    bool  use24HourClock = true;
    bool  showFps        = false;
    int   brightness     = 80;          // 10-100
    float uiScale        = 1.0f;
    int   theme          = 0;           // 0 Dark, 1 Light, 2 Classic (Can be removed if di naman natin gagamitin)
    float accent[3]      = {0.26f, 0.59f, 0.98f};
    int   wallpaperStyle = 0;           // 0 Gradient, 1 Solid, 2 Pattern  (Can be removed if di naman natin gagamitin)
    float wallpaper1[3]  = {0.10f, 0.15f, 0.35f};
    float wallpaper2[3]  = {0.45f, 0.20f, 0.55f};
    bool  wifi           = true;
    bool  bluetooth      = false;
    int   volume         = 70;
    bool  mute           = false;
    int   timezone       = 0;           // 0 Manila, 1 London, 2 New York, 3 Tokyo
};
AppSettings& GetAppSettings();

class FileExplorerWindow : public IWindow {
public:
    FileExplorerWindow();
    ~FileExplorerWindow() override;
    void draw() override;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class SystemInfoWindow : public IWindow {
public:
    SystemInfoWindow();
    ~SystemInfoWindow() override;
    void draw() override;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};