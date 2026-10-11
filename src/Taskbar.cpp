#include "Taskbar.h"
#include "Applications.h"
#include "TaskManager.h"
#include "imgui.h"


// Open/closed state of each app. The taskbar owns these flags.
// They are passed to the app windows so ImGui can set them to false when the user clicks a window's X button.
static bool showFileExplorer = false;
static bool showSettings = false;
static bool showTaskManager = false;


// Draws one taskbar button. Returns true on the frame it is clicked. 
// If the app is open (active), the button is highlighted and gets a small underline, which shows which apps are running.
static bool TaskbarButton(const char* label, bool active, ImVec2 size)
{
    // Blue background while the app is open
    if (active)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.45f, 0.80f, 1.0f));
    }

    bool clicked = ImGui::Button(label, size);

    if (active)
    {
        ImGui::PopStyleColor();

        // Underline along the bottom edge of the button
        ImVec2 p0 = ImGui::GetItemRectMin();
        ImVec2 p1 = ImGui::GetItemRectMax();
        
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p0.x, p1.y - 3), p1, IM_COL32(120, 200, 255, 255));
    }

    return clicked;
}

void DrawTaskbar()
{
    const float barHeight = 48.0f;
    ImGuiViewport* vp = ImGui::GetMainViewport();


    // Pin the bar to the bottom of the screen (full width)
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - barHeight));
    ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, barHeight));


    // No title bar, no dragging, no saved layout.
    // NoBringToFrontOnFocus stops the taskbar from jumping in front of app windows when clicked.
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;


    // Dark, slightly transparent background
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.10f, 0.92f));
    ImGui::Begin("##Taskbar", nullptr, flags);


    // The three icon buttons. Clicking one toggles its app open/closed.
    ImVec2 btn(100, 36);

    if (TaskbarButton("Files", showFileExplorer, btn))
    {
        showFileExplorer = !showFileExplorer;
    }

    ImGui::SameLine();

    if (TaskbarButton("Settings", showSettings, btn))
    {
        showSettings = !showSettings;
    }

    ImGui::SameLine();

    if (TaskbarButton("Tasks", showTaskManager, btn))
    {
        showTaskManager = !showTaskManager;
    }

    ImGui::End();
    ImGui::PopStyleColor();


    // Visibility management: an app window is only drawn while its flag is true
    if (showFileExplorer)
    {
        DrawFileExplorer(&showFileExplorer);
    }

    if (showSettings)
    {
        DrawSettings(&showSettings);
    }

    if (showTaskManager)
    {
        DrawTaskManager(&showTaskManager);
    }
}