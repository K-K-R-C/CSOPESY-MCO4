#include "Taskbar.h"
#include "Desktop.h"
#include "imgui.h"

// Draws one taskbar button. Returns true on the frame it is clicked.
// If the app is open (active), the button is highlighted and gets a small underline.
static bool TaskbarButton(const char* label, bool active, ImVec2 size)
{
    if (active)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.45f, 0.80f, 1.0f));

    bool clicked = ImGui::Button(label, size);

    if (active)
    {
        ImGui::PopStyleColor();
        ImVec2 p0 = ImGui::GetItemRectMin();
        ImVec2 p1 = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p0.x, p1.y - 3), p1, IM_COL32(120, 200, 255, 255));
    }
    return clicked;
}

void DrawTaskbar(Desktop& desktop)
{
    const float barHeight = Desktop::kTaskbarHeight;  // 48, shared with the Desktop
    ImGuiViewport* vp = ImGui::GetMainViewport();

    // Pin the bar to the bottom of the screen (full width)
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - barHeight));
    ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, barHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.10f, 0.92f));
    ImGui::Begin("##Taskbar", nullptr, flags);

    // The three icon buttons. The Desktop owns the open/closed state of every app window.
    ImVec2 btn(100, 36);

    if (TaskbarButton("Files", desktop.isOpen("files"), btn))
        desktop.toggle("files");

    ImGui::SameLine();
    if (TaskbarButton("Settings", desktop.isOpen("settings"), btn))
        desktop.toggle("settings");

    ImGui::SameLine();
    if (TaskbarButton("Tasks", desktop.isOpen("taskmgr"), btn))
        desktop.toggle("taskmgr");

    ImGui::End();
    ImGui::PopStyleColor();
}