#include "Desktop.h"
#include "imgui.h"
#include "imgui_internal.h"  // BringWindowToDisplayFront (keeps shell above app windows)
#include <cmath>
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
    drawTaskbar();                                           // layer 2
    drawClock();                                             // layer 3
    // ImGui raises the clicked window; re-raise the shell so it always stays on top.
    for (const char* name : {"##taskbar", "##clock"})
        if (ImGuiWindow* win = ImGui::FindWindowByName(name)) ImGui::BringWindowToDisplayFront(win);
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

// ------------------------------ taskbar / tray / clock ------------------------------
static const ImGuiWindowFlags kShellFlags =
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav;

void Desktop::drawTaskbar() {
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - kTaskbarHeight));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, kTaskbarHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.07f, 0.10f, 0.94f));

    ImGui::Begin("##taskbar", nullptr, kShellFlags);
    ImGui::SetCursorPos(ImVec2(10, 8));
    if (taskbarFn_) taskbarFn_(*this);  // Member 2's buttons
    drawTray();                         // VOL / NET / PWR
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(4);
}

void Desktop::drawTray() {
    const ImGuiIO& io = ImGui::GetIO();
    const ImVec2 bs(52, 40);
    const float gap = 8.0f, total = 3 * bs.x + 2 * gap;
    ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - total - 10.0f, (kTaskbarHeight - bs.y) * 0.5f));

    ImGui::Button("VOL", bs);  // placeholders
    ImGui::SameLine();
    ImGui::Button("NET", bs);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.30f, 0.08f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
    if (ImGui::Button("PWR", bs)) requestShutdown();
    ImGui::PopStyleColor(3);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Shut down CSOPESY OS");
}

void Desktop::drawClock() {
    std::time_t now = std::time(nullptr);
    std::tm lt{};
#ifdef _WIN32
    localtime_s(&lt, &now);
#else
    localtime_r(&now, &lt);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%A, %b %d, %Y | %I:%M:%S %p", &lt);

    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.65f);
    ImGui::Begin("##clock", nullptr, kShellFlags | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::TextUnformatted(buf);
    ImGui::End();
}