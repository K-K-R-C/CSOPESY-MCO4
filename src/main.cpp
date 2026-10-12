// ---------------------------------------------------------------------------
// CSOPESY Desktop OS Emulator - ENTRY POINT (int main)
// Stack: GLFW (window/input) + OpenGL 3 (GPU) + Dear ImGui (immediate-mode UI)
// Stages: BIOS -> SPLASH -> DESKTOP -> SHUTTING_DOWN -> exit
// The app can only be closed with the PWR button (window X / Alt+F4 ignored).
// ---------------------------------------------------------------------------
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "Applications.h"
#include "Boot.h"
#include "Desktop.h"
#include "TaskManager.h"
#include "Taskbar.h"
#include <cstdio>
#include <memory>

namespace {
bool g_allowClose = false;  // flipped to true only by the PWR shutdown sequence
enum class Stage { Bios, Splash, Desktop, ShuttingDown };

// Member 3's Task Manager is a free function DrawTaskManager(bool*), so wrap it as an IWindow.
struct TaskManagerWindow : IWindow {
    TaskManagerWindow() : IWindow("taskmgr", "Task Manager") {}
    void draw() override { DrawTaskManager(&isOpen); }
};
}

int main() {
    if (!glfwInit()) { std::fprintf(stderr, "Failed to init GLFW\n"); return 1; }

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "CSOPESY Desktop OS Emulator", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {  // block force-exit
        if (!g_allowClose) glfwSetWindowShouldClose(w, GLFW_FALSE);
    });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImFontConfig fontCfg;
    fontCfg.SizePixels = 18.0f;  // readable in 720p video
    io.Fonts->AddFontDefault(&fontCfg);
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // ---- compose the OS ----
    Desktop desktop;
    BootSequence boot;
    desktop.registerWindow(std::make_shared<FileExplorerWindow>());
    desktop.registerWindow(std::make_shared<SystemInfoWindow>());
    desktop.registerWindow(std::make_shared<TaskManagerWindow>());
    desktop.setTaskbarDrawer([](Desktop& d) { DrawTaskbar(d); });

    Stage stage = Stage::Bios;
    double last = glfwGetTime(), shutdownStart = 0.0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - last);
        last = now;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        switch (stage) {
        case Stage::Bios:   if (boot.drawBios(dt))   stage = Stage::Splash;  break;
        case Stage::Splash: if (boot.drawSplash(dt)) stage = Stage::Desktop; break;
        case Stage::Desktop:
            desktop.render(static_cast<float>(now));
            if (desktop.shutdownRequested()) { stage = Stage::ShuttingDown; shutdownStart = now; }
            break;
        case Stage::ShuttingDown: {
            desktop.render(static_cast<float>(now));
            const float p = static_cast<float>((now - shutdownStart) / 2.0);  // 2 s fade-out
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            fg->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, static_cast<int>(255 * (p > 1 ? 1 : p))));
            const char* msg = "Shutting down...";
            const ImVec2 ts = ImGui::CalcTextSize(msg);
            if (p > 0.4f)
                fg->AddText(ImVec2((io.DisplaySize.x - ts.x) * 0.5f, (io.DisplaySize.y - ts.y) * 0.5f),
                            IM_COL32(220, 220, 220, 255), msg);
            if (p >= 1.0f) { g_allowClose = true; glfwSetWindowShouldClose(window, GLFW_TRUE); }
            break;
        }
        }

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}