#pragma once
// ---------------------------------------------------------------------------
// Desktop (MEMBER 1) - the compositor. Renders the layer stack every frame:
//   0 wallpaper -> 1 app windows -> 2 taskbar + tray (PWR) -> 3 clock
//
// HOW OTHER MEMBERS PLUG IN
//   * Any window (Task Manager, app screens) derives from IWindow and is added
//     with desktop.registerWindow(...). Open/close it with desktop.toggle("id").
//   * The taskbar buttons are drawn by Taskbar::draw() (Member 2), which the
//     Desktop calls inside the taskbar panel via setTaskbarContent().
// ---------------------------------------------------------------------------
#include <functional>
#include <memory>
#include <string>
#include <vector>

struct IWindow {
    std::string id;       // unique key, e.g. "taskmgr"
    std::string title;    // ImGui title-bar text
    bool isOpen = false;  // pass &isOpen to ImGui::Begin so the X button works

    IWindow(std::string id_, std::string title_) : id(std::move(id_)), title(std::move(title_)) {}
    virtual ~IWindow() = default;
    virtual void draw() = 0;  // called each frame ONLY while isOpen == true
};

class Desktop {
public:
    static constexpr float kTaskbarHeight = 56.0f;
    using TaskbarFn = std::function<void(Desktop&)>;

    void registerWindow(std::shared_ptr<IWindow> w);
    IWindow* find(const std::string& id) const;
    void open(const std::string& id);
    void close(const std::string& id);
    void toggle(const std::string& id);
    bool isOpen(const std::string& id) const;
    void setTaskbarContent(TaskbarFn fn) { taskbarFn_ = std::move(fn); }

    void requestShutdown() { shutdownRequested_ = true; }  // called by PWR
    bool shutdownRequested() const { return shutdownRequested_; }

    void render(float timeSeconds);  // once per frame from main()

private:
    void drawWallpaper(float t) const;
    void drawTaskbar();
    void drawTray();
    void drawClock();

    std::vector<std::shared_ptr<IWindow>> windows_;
    TaskbarFn taskbarFn_;
    bool shutdownRequested_ = false;
};