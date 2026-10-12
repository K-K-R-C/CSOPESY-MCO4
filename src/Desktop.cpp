#include "Desktop.h"
#include "Applications.h"  // GetAppSettings(): clock format, brightness, FPS overlay
#include "imgui.h"
#include "imgui_internal.h"  // BringWindowToDisplayFront (keeps shell above app windows)
#include <cmath>
#include <cstdio>
#include <ctime>

// ------------------------------ window registry ------------------------------
void Desktop::registerWindow(std::shared_ptr<IWindow> w) { windows_.push_back(std::move(w)); }
IWindow* Desktop::find(const std::string& id) const {
    for (auto& w : windows_) if (w->id == id) return w.get();
    return nullptr;
}
void Desktop::open(const std::string& id) {
    if (auto* w = find(id)) { w->isOpen = true; ImGui::SetWindowFocus(w->title.c_str()); }
}
void Desktop::close(const std::string& id) { if (auto* w = find(id)) w->isOpen = false; }
void Desktop::toggle(const std::string& id) {
    if (auto* w = find(id)) { if (w->isOpen) w->isOpen = false; else open(id); }
}
bool Desktop::isOpen(const std::string& id) const { auto* w = find(id); return w && w->isOpen; }

// -------------------------------- frame render --------------------------------
void Desktop::render(float t) {
    drawWallpaper(t);                                        // layer 0
    for (auto& w : windows_) if (w->isOpen) w->draw();       // layer 1
    if (taskbarFn_) taskbarFn_(*this);                       // layer 2 (Member 2's taskbar panel)
    drawTray();                                              // layer 2 (VOL / NET / PWR on top of it)
    drawClock();                                             // layer 3

    // ImGui raises the clicked window; re-raise the shell so it always stays on top.
    for (const char* name : {"##Taskbar", "##tray", "##clock"})
        if (ImGuiWindow* win = ImGui::FindWindowByName(name)) ImGui::BringWindowToDisplayFront(win);
    ImGuiContext& g = *ImGui::GetCurrentContext();  // VOL/NET pop-ups must stay in front of the shell
    for (const ImGuiPopupData& pop : g.OpenPopupStack)
        if (pop.Window) ImGui::BringWindowToDisplayFront(pop.Window);

    // Optional System Settings hooks
    const AppSettings& cfg = GetAppSettings();
    const ImGuiIO& io = ImGui::GetIO();
    ImDrawList* fg = ImGui::GetForegroundDrawList();
    if (cfg.brightness < 100)  // brightness = dark overlay
        fg->AddRectFilled(ImVec2(0, 0), io.DisplaySize,
                          IM_COL32(0, 0, 0, static_cast<int>(255 * (1.0f - cfg.brightness / 100.0f))));
    if (cfg.showFps) {
        char fps[32];
        std::snprintf(fps, sizeof fps, "FPS: %.0f", io.Framerate);
        fg->AddText(ImVec2(10, 10), IM_COL32(255, 255, 0, 255), fps);
    }
}

// ---------------------------------- wallpaper ----------------------------------
namespace {
struct Cloud { float x, y, scale, speed; };  // x,y = screen fractions
const Cloud kClouds[] = {
    {0.10f, 0.12f, 1.0f, 6.0f}, {0.35f, 0.26f, 1.4f, 4.0f}, {0.60f, 0.09f, 1.2f, 5.0f},
    {0.80f, 0.28f, 0.9f, 7.0f}, {0.22f, 0.40f, 0.7f, 3.0f}, {0.93f, 0.17f, 1.1f, 4.5f},
};
void drawCloud(ImDrawList* dl, ImVec2 c, float s) {
    const ImU32 col = IM_COL32(255, 255, 255, 150);
    dl->AddCircleFilled(ImVec2(c.x - 38 * s, c.y + 8 * s),  20 * s, col, 24);
    dl->AddCircleFilled(ImVec2(c.x - 12 * s, c.y - 6 * s),  28 * s, col, 24);
    dl->AddCircleFilled(ImVec2(c.x + 20 * s, c.y),          24 * s, col, 24);
    dl->AddCircleFilled(ImVec2(c.x + 46 * s, c.y + 10 * s), 18 * s, col, 24);
    dl->AddRectFilled(ImVec2(c.x - 38 * s, c.y + 8 * s), ImVec2(c.x + 46 * s, c.y + 28 * s), col, 10 * s);
}
}  // namespace

void Desktop::drawWallpaper(float t) const {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();  // always behind every ImGui window
    const ImVec2 sz = ImGui::GetIO().DisplaySize;

    // Wallpaper chosen in System Settings: 0 Bliss (default), 1 Gradient, 2 Solid, 3 Pattern
    const AppSettings& cfg = GetAppSettings();
    if (cfg.wallpaperStyle != 0) {
        auto toCol = [](const float c[3]) { return IM_COL32(int(c[0] * 255), int(c[1] * 255), int(c[2] * 255), 255); };
        const ImU32 c1 = toCol(cfg.wallpaper1), c2 = toCol(cfg.wallpaper2);
        if (cfg.wallpaperStyle == 1) {          // vertical gradient
            dl->AddRectFilledMultiColor(ImVec2(0, 0), sz, c1, c1, c2, c2);
        } else if (cfg.wallpaperStyle == 2) {   // solid color
            dl->AddRectFilled(ImVec2(0, 0), sz, c1);
        } else {                                // scrolling checkerboard
            const float cell = 64.0f, off = std::fmod(t * 20.0f, cell * 2.0f);
            dl->AddRectFilled(ImVec2(0, 0), sz, c1);
            for (int j = -2; j * cell < sz.y; ++j)
                for (int i = -2; i * cell < sz.x; ++i)
                    if (((i + j) & 1) != 0)
                        dl->AddRectFilled(ImVec2(i * cell + off, j * cell + off),
                                          ImVec2((i + 1) * cell + off, (j + 1) * cell + off), c2);
        }
        dl->AddText(ImVec2(20, sz.y - kTaskbarHeight - 30), IM_COL32(255, 255, 255, 170), "CSOPESY OS v1.0");
        return;
    }

    const ImU32 skyTop = IM_COL32(28, 96, 200, 255), skyBot = IM_COL32(160, 208, 248, 255);
    dl->AddRectFilledMultiColor(ImVec2(0, 0), sz, skyTop, skyTop, skyBot, skyBot);  // sky

    const float span = sz.x + 400.0f;  // clouds drift right and wrap
    for (const Cloud& c : kClouds) {
        float x = std::fmod(c.x * sz.x + t * c.speed * 4.0f, span) - 200.0f;
        drawCloud(dl, ImVec2(x, c.y * sz.y), c.scale * (sz.y / 720.0f));
    }

    const ImU32 hillTop = IM_COL32(124, 196, 52, 255), hillBot = IM_COL32(38, 108, 24, 255);
    const float step = 4.0f;  // hill = vertical gradient strips following a sine curve
    for (float x = 0; x < sz.x; x += step) {
        float y = sz.y * (0.62f - 0.14f * std::sin(0.4f + (x / sz.x) * 2.4f));
        dl->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + step + 1.0f, sz.y), hillTop, hillTop, hillBot, hillBot);
    }
    dl->AddText(ImVec2(20, sz.y - kTaskbarHeight - 30), IM_COL32(255, 255, 255, 170), "CSOPESY OS v1.0");
}

// ------------------------------------ tray / clock ------------------------------------
static const ImGuiWindowFlags kShellFlags =
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav;

// Transparent window pinned to the bottom-right corner, floating over the taskbar.
void Desktop::drawTray() {
    const ImGuiIO& io = ImGui::GetIO();
    const ImVec2 bs(52, 36);
    const float gap = 8.0f, margin = 10.0f, total = 3 * bs.x + 2 * gap;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - total - margin, io.DisplaySize.y - kTaskbarHeight));
    ImGui::SetNextWindowSize(ImVec2(total + margin, kTaskbarHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(gap, 0));

    ImGui::Begin("##tray", nullptr, kShellFlags | ImGuiWindowFlags_NoBackground);
    ImGui::SetCursorPos(ImVec2(0, (kTaskbarHeight - bs.y) * 0.5f));
    AppSettings& cfg = GetAppSettings();  // shared with the Settings window

    // Pop-ups open above the button, right-aligned to it.
    auto placePopupAbove = []() {
        const ImVec2 mn = ImGui::GetItemRectMin(), mx = ImGui::GetItemRectMax();
        ImGui::SetNextWindowPos(ImVec2(mx.x, mn.y - 6.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    };

    // ---- VOL: volume slider + mute ----
    if (ImGui::Button(cfg.mute ? "MUTE###vol" : "VOL###vol", bs)) ImGui::OpenPopup("##volpop");
    placePopupAbove();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
    if (ImGui::BeginPopup("##volpop")) {
        ImGui::TextUnformatted("Volume");
        ImGui::SetNextItemWidth(200);
        ImGui::SliderInt("##volume", &cfg.volume, 0, 100, "%d%%");
        ImGui::Checkbox("Mute", &cfg.mute);
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);

    // ---- NET: Wi-Fi / Bluetooth ----
    ImGui::SameLine();
    if (!cfg.wifi) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.55f, 1.0f));  // greyed when off
    const bool netClicked = ImGui::Button("NET###net", bs);
    if (!cfg.wifi) ImGui::PopStyleColor();
    if (netClicked) ImGui::OpenPopup("##netpop");
    placePopupAbove();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
    if (ImGui::BeginPopup("##netpop")) {
        ImGui::TextUnformatted("Network");
        ImGui::Checkbox("Wi-Fi", &cfg.wifi);
        ImGui::Checkbox("Bluetooth", &cfg.bluetooth);
        ImGui::TextDisabled(cfg.wifi ? "Status: Connected" : "Status: Wi-Fi is off");
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.30f, 0.08f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
    if (ImGui::Button("PWR", bs)) requestShutdown();
    ImGui::PopStyleColor(3);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Shut down CSOPESY OS");
    ImGui::End();

    ImGui::PopStyleVar(3);
}

void Desktop::drawClock() {
    // Time zone chosen in System Settings: 0 Manila, 1 London, 2 New York, 3 Tokyo
    static const int         kOffsetHours[] = {8, 0, -5, 9};
    static const char* const kTzName[]      = {"PHT", "GMT", "EST", "JST"};
    int tz = GetAppSettings().timezone;
    if (tz < 0 || tz > 3) tz = 0;

    std::time_t shifted = std::time(nullptr) + kOffsetHours[tz] * 3600;  // UTC + offset
    std::tm lt = *std::gmtime(&shifted);

    char buf[64], full[96];
    std::strftime(buf, sizeof(buf),
                  GetAppSettings().use24HourClock ? "%A, %b %d, %Y | %H:%M:%S" : "%A, %b %d, %Y | %I:%M:%S %p", &lt);
    std::snprintf(full, sizeof full, "%s %s", buf, kTzName[tz]);

    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.65f);
    ImGui::Begin("##clock", nullptr, kShellFlags | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::TextUnformatted(full);
    ImGui::End();
}