#include "Applications.h"
#include "imgui.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

AppSettings& GetAppSettings() {
    static AppSettings s;
    return s;
}

namespace {

// =====================================================================
//  FILE EXPLORER data (fake in-memory file system, with placeholders only, no real data pa)
// =====================================================================
struct Node {
    std::string name, type, modified;
    bool isFolder = false;
    int  sizeKB   = 0;
    std::vector<Node> children;
};

std::string TypeFromName(const std::string& n) {
    size_t dot = n.rfind('.');
    if (dot == std::string::npos) return "File";
    std::string e = n.substr(dot + 1);
    if (e == "txt" || e == "md")  return "Text Document";
    if (e == "cpp" || e == "h")   return "C++ Source";
    if (e == "png" || e == "jpg") return "Image";
    if (e == "mp3" || e == "wav") return "Audio";
    if (e == "mp4")               return "Video";
    if (e == "pdf")               return "PDF Document";
    if (e == "pptx")              return "Presentation";
    if (e == "zip")               return "Archive";
    if (e == "exe")               return "Application";
    return "File";
}

Node MakeFile(const char* name, int kb, const char* date) {
    Node n; n.name = name; n.sizeKB = kb; n.modified = date; n.type = TypeFromName(name);
    return n;
}

Node MakeDir(const char* name, const char* date, std::vector<Node> kids = {}) {
    Node n; n.name = name; n.modified = date; n.isFolder = true; n.type = "File folder";
    n.children = std::move(kids);
    return n;
}

Node BuildTree() {
    return MakeDir("This PC", "", {
        MakeDir("Desktop", "2026-10-01", {
            MakeFile("README.txt",    2,    "2026-09-30"),
            MakeFile("todo.md",       4,    "2026-10-02"),
            MakeFile("wallpaper.png", 2480, "2026-09-12"),
        }),
        MakeDir("Documents", "2026-10-05", {
            MakeDir("CSOPESY", "2026-10-05", {
                MakeFile("main.cpp",        18,    "2026-10-04"),
                MakeFile("scheduler.cpp",   42,    "2026-10-03"),
                MakeFile("Applications.cpp",26,    "2026-10-05"),
                MakeFile("MO4_Report.pptx", 5120,  "2026-10-06"),
                MakeFile("demo.mp4",        88400, "2026-10-06"),
            }),
            MakeDir("Reports", "2026-09-28", {
                MakeFile("Networking_MP.pdf", 940, "2026-09-20"),
                MakeFile("Stats_Lab3.pdf",    610, "2026-09-28"),
            }),
            MakeFile("Resume.pdf", 120, "2026-08-15"),
        }),
        MakeDir("Downloads", "2026-10-07", {
            MakeFile("imgui-master.zip", 3900, "2026-10-07"),
            MakeFile("glfw-setup.exe",   1450, "2026-10-07"),
        }),
        MakeDir("Pictures", "2026-09-01", {
            MakeFile("group_photo.jpg", 3100, "2026-09-01"),
            MakeFile("diagram.png",     540,  "2026-09-18"),
        }),
        MakeDir("Music", "2026-07-20", {
            MakeFile("lofi_study.mp3", 6200, "2026-07-20"),
        }),
    });
}

using Path = std::vector<const Node*>;

std::string FmtSize(int kb) {
    char b[32];
    if (kb >= 1024) std::snprintf(b, sizeof b, "%.1f MB", kb / 1024.0);
    else            std::snprintf(b, sizeof b, "%d KB", kb);
    return b;
}

bool ContainsNoCase(const std::string& hay, const char* needle) {
    if (!*needle) return true;
    std::string a = hay, b = needle;
    for (auto& c : a) c = (char)std::tolower((unsigned char)c);
    for (auto& c : b) c = (char)std::tolower((unsigned char)c);
    return a.find(b) != std::string::npos;
}

// =====================================================================
//  SYSTEM SETTINGS helpers
// =====================================================================
void ApplyTheme(const AppSettings& s) {
    switch (s.theme) {
        case 1:  ImGui::StyleColorsLight();   break;
        case 2:  ImGui::StyleColorsClassic(); break;
        default: ImGui::StyleColorsDark();    break;
    }
    ImVec4* c = ImGui::GetStyle().Colors;
    auto A = [&](float a) { return ImVec4(s.accent[0], s.accent[1], s.accent[2], a); };
    c[ImGuiCol_CheckMark]        = A(1.0f);
    c[ImGuiCol_SliderGrab]       = A(0.9f);
    c[ImGuiCol_SliderGrabActive] = A(1.0f);
    c[ImGuiCol_Button]           = A(0.40f);
    c[ImGuiCol_ButtonHovered]    = A(0.80f);
    c[ImGuiCol_ButtonActive]     = A(1.00f);
    c[ImGuiCol_Header]           = A(0.31f);
    c[ImGuiCol_HeaderHovered]    = A(0.80f);
    c[ImGuiCol_HeaderActive]     = A(1.00f);
}

struct SettingsUI {
    int  page = 0;
    char deviceName[32] = "CSOPESY-PC";
    int  timezone     = 0;
    int  resolution   = 1;
    int  orientation  = 0;
    int  outputDevice = 0;
    int  selectedNet  = -1;
    std::string connectedNet = "HomeWiFi_5G";
};

} // anonymous namespace

// =====================================================================
//  FileExplorerWindow
// =====================================================================
struct FileExplorerWindow::Impl {
    Node root = BuildTree();
    Path path;
    std::vector<Path> back, fwd;
    const Node* selected = nullptr;
    const Node* props    = nullptr;
    bool openProps = false;
    bool gridView  = false;
    char search[64] = "";
    std::string status = "Ready";

    Impl() { path = { &root }; }

    void NavigateTo(const Path& p) {
        back.push_back(path);
        fwd.clear();
        path = p;
        selected = nullptr;
        search[0] = '\0';
    }
};

FileExplorerWindow::FileExplorerWindow()
    : IWindow("files", "File Explorer"), impl_(std::make_unique<Impl>()) {}
FileExplorerWindow::~FileExplorerWindow() = default;

void FileExplorerWindow::draw() {
    Impl& ex = *impl_;

    ImGui::SetNextWindowPos(ImVec2(90, 70), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(780, 500), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title.c_str(), &isOpen)) { ImGui::End(); return; }

    const Node* cur = ex.path.back();
    const Node* toOpen = nullptr;

    // ---- Toolbar: back / forward / up ----
    ImGui::BeginDisabled(ex.back.empty());
    if (ImGui::Button("<")) {
        ex.fwd.push_back(ex.path);
        ex.path = ex.back.back();
        ex.back.pop_back();
        ex.selected = nullptr;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(ex.fwd.empty());
    if (ImGui::Button(">")) {
        ex.back.push_back(ex.path);
        ex.path = ex.fwd.back();
        ex.fwd.pop_back();
        ex.selected = nullptr;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(ex.path.size() <= 1);
    if (ImGui::Button("Up")) ex.NavigateTo(Path(ex.path.begin(), ex.path.end() - 1));
    ImGui::EndDisabled();

    // ---- Breadcrumb address bar ----
    int crumbTarget = -1;
    for (size_t i = 0; i < ex.path.size(); ++i) {
        ImGui::SameLine();
        ImGui::PushID((int)i);
        if (ImGui::SmallButton(ex.path[i]->name.c_str()) && i + 1 != ex.path.size())
            crumbTarget = (int)i;
        ImGui::PopID();
        if (i + 1 != ex.path.size()) { ImGui::SameLine(); ImGui::TextDisabled(">"); }
    }
    if (crumbTarget >= 0) ex.NavigateTo(Path(ex.path.begin(), ex.path.begin() + crumbTarget + 1));

    // ---- Search + view toggle ----
    ImGui::SetNextItemWidth(200);
    ImGui::InputTextWithHint("##search", "Search this folder...", ex.search, sizeof ex.search);
    ImGui::SameLine();
    if (ImGui::Button(ex.gridView ? "List view" : "Grid view")) ex.gridView = !ex.gridView;

    ImGui::Separator();

    const float footer = ImGui::GetFrameHeightWithSpacing();

    // ---- Sidebar ----
    ImGui::BeginChild("sidebar", ImVec2(140, -footer), ImGuiChildFlags_Borders);
    ImGui::TextDisabled("Quick access");
    ImGui::Separator();
    if (ImGui::Selectable("This PC", ex.path.size() == 1)) ex.NavigateTo({ &ex.root });
    for (const Node& child : ex.root.children) {
        if (!child.isFolder) continue;
        bool isHere = ex.path.size() >= 2 && ex.path[1] == &child;
        if (ImGui::Selectable(child.name.c_str(), isHere)) ex.NavigateTo({ &ex.root, &child });
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // ---- Content pane ----
    ImGui::BeginChild("content", ImVec2(0, -footer), ImGuiChildFlags_Borders);

    std::vector<const Node*> items;
    for (const Node& n : cur->children)
        if (ContainsNoCase(n.name, ex.search)) items.push_back(&n);
    std::stable_partition(items.begin(), items.end(), [](const Node* n) { return n->isFolder; });

    auto itemMenu = [&](const Node* n) {
        if (ImGui::BeginPopupContextItem("ctx")) {
            ex.selected = n;
            if (ImGui::MenuItem("Open"))       toOpen = n;
            if (ImGui::MenuItem("Properties")) { ex.props = n; ex.openProps = true; }
            ImGui::EndPopup();
        }
    };

    if (items.empty()) {
        ImGui::TextDisabled(ex.search[0] ? "No items match your search." : "This folder is empty.");
    } else if (!ex.gridView) {
        if (ImGui::BeginTable("files", 4,
                ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 3.0f);
            ImGui::TableSetupColumn("Date modified", ImGuiTableColumnFlags_WidthStretch, 1.5f);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 1.5f);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            for (const Node* n : items) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::PushID(n);
                std::string label = std::string(n->isFolder ? "[DIR]  " : "[FILE] ") + n->name;
                if (ImGui::Selectable(label.c_str(), ex.selected == n,
                        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
                    ex.selected = n;
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) toOpen = n;
                itemMenu(n);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(n->modified.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(n->type.c_str());
                ImGui::TableNextColumn();
                if (!n->isFolder) ImGui::TextUnformatted(FmtSize(n->sizeKB).c_str());
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    } else {
        const float cell = 96.0f;
        const float availW = ImGui::GetContentRegionAvail().x;
        const int cols = std::max(1, (int)(availW / (cell + ImGui::GetStyle().ItemSpacing.x)));
        int i = 0;
        for (const Node* n : items) {
            if (i++ % cols != 0) ImGui::SameLine();
            ImGui::PushID(n);
            std::string label = std::string(n->isFolder ? "[DIR]" : "[FILE]") + "\n" + n->name;
            if (ImGui::Selectable(label.c_str(), ex.selected == n,
                    ImGuiSelectableFlags_AllowDoubleClick, ImVec2(cell, 56)))
                ex.selected = n;
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) toOpen = n;
            itemMenu(n);
            ImGui::PopID();
        }
    }
    ImGui::EndChild();

    // ---- Deferred "open" so the path never changes mid-iteration ----
    if (toOpen) {
        if (toOpen->isFolder) {
            Path p = ex.path; p.push_back(toOpen);
            ex.NavigateTo(p);
        } else {
            ex.status = "Opening \"" + toOpen->name + "\" (placeholder - no real file)";
        }
    }

    // ---- Status bar ----
    ImGui::Text("%d item(s)", (int)items.size());
    ImGui::SameLine();
    if (ex.selected) ImGui::TextDisabled("| Selected: %s", ex.selected->name.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("| %s", ex.status.c_str());

    // ---- Properties dialog ----
    if (ex.openProps) { ImGui::OpenPopup("Properties"); ex.openProps = false; }
    if (ImGui::BeginPopupModal("Properties", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ex.props) {
            ImGui::Text("Name:      %s", ex.props->name.c_str());
            ImGui::Text("Type:      %s", ex.props->type.c_str());
            ImGui::Text("Modified:  %s", ex.props->modified.c_str());
            if (!ex.props->isFolder) ImGui::Text("Size:      %s", FmtSize(ex.props->sizeKB).c_str());
            else                     ImGui::Text("Contains:  %d item(s)", (int)ex.props->children.size());
        }
        ImGui::Separator();
        if (ImGui::Button("Close", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::End();
}

// =====================================================================
//  SystemInfoWindow  ("System Settings")
// =====================================================================
struct SystemInfoWindow::Impl {
    SettingsUI ui;
};

SystemInfoWindow::SystemInfoWindow()
    : IWindow("settings", "System Settings"), impl_(std::make_unique<Impl>()) {}
SystemInfoWindow::~SystemInfoWindow() = default;

void SystemInfoWindow::draw() {
    SettingsUI& ui = impl_->ui;
    AppSettings& s = GetAppSettings();

    ImGui::SetNextWindowPos(ImVec2(150, 110), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(700, 440), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title.c_str(), &isOpen)) { ImGui::End(); return; }

    static const char* pages[] = { "System", "Display", "Personalization", "Network", "Sound", "About" };

    // ---- Left navigation ----
    ImGui::BeginChild("nav", ImVec2(160, 0), ImGuiChildFlags_Borders);
    for (int i = 0; i < IM_ARRAYSIZE(pages); ++i)
        if (ImGui::Selectable(pages[i], ui.page == i)) ui.page = i;
    ImGui::EndChild();

    ImGui::SameLine();

    // ---- Right page ----
    ImGui::BeginChild("page", ImVec2(0, 0), ImGuiChildFlags_Borders);
    ImGui::SeparatorText(pages[ui.page]);

    switch (ui.page) {
    case 0: { // System
        ImGui::InputText("Device name", ui.deviceName, sizeof ui.deviceName);
        const char* tz[] = { "UTC+08:00 Manila", "UTC+00:00 London", "UTC-05:00 New York", "UTC+09:00 Tokyo" };
        ImGui::Combo("Time zone", &ui.timezone, tz, IM_ARRAYSIZE(tz));
        ImGui::Checkbox("Use 24-hour clock", &s.use24HourClock);
        ImGui::Checkbox("Show FPS overlay", &s.showFps);
        break;
    }
    case 1: { // Display
        ImGui::SliderInt("Brightness", &s.brightness, 10, 100, "%d%%");
        const char* res[] = { "1280 x 720", "1600 x 900", "1920 x 1080" };
        ImGui::Combo("Resolution (placeholder)", &ui.resolution, res, IM_ARRAYSIZE(res));
        ImGui::RadioButton("Landscape", &ui.orientation, 0); ImGui::SameLine();
        ImGui::RadioButton("Portrait",  &ui.orientation, 1);
        ImGui::SliderFloat("UI scale", &s.uiScale, 0.75f, 1.5f, "%.2fx");
        if (ImGui::IsItemDeactivatedAfterEdit()) ImGui::GetIO().FontGlobalScale = s.uiScale;
        ImGui::TextDisabled("UI scale applies when you release the slider.");
        break;
    }
    case 2: { // Personalization
        const char* themes[] = { "Dark", "Light", "Classic" };
        if (ImGui::Combo("Theme", &s.theme, themes, IM_ARRAYSIZE(themes))) ApplyTheme(s);
        if (ImGui::ColorEdit3("Accent color", s.accent, ImGuiColorEditFlags_NoInputs)) ApplyTheme(s);
        ImGui::Spacing();
        const char* wp[] = { "Gradient", "Solid color", "Pattern" };
        ImGui::Combo("Wallpaper style", &s.wallpaperStyle, wp, IM_ARRAYSIZE(wp));
        ImGui::ColorEdit3("Wallpaper color 1", s.wallpaper1, ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit3("Wallpaper color 2", s.wallpaper2, ImGuiColorEditFlags_NoInputs);
        break;
    }
    case 3: { // Network
        ImGui::Checkbox("Wi-Fi", &s.wifi);
        ImGui::Checkbox("Bluetooth", &s.bluetooth);
        ImGui::Spacing();
        if (!s.wifi) { ImGui::TextDisabled("Wi-Fi is turned off."); break; }
        ImGui::Text("Connected to: %s", ui.connectedNet.c_str());
        static const char* nets[]   = { "HomeWiFi_5G", "DLSU-Student", "CSOPESY_Lab", "Free_WiFi_Totally_Safe" };
        static const char* signal[] = { "Excellent", "Good", "Fair", "Weak" };
        ImGui::BeginChild("nets", ImVec2(0, 140), ImGuiChildFlags_Borders);
        for (int i = 0; i < IM_ARRAYSIZE(nets); ++i) {
            char label[96];
            std::snprintf(label, sizeof label, "%s  (%s)", nets[i], signal[i]);
            if (ImGui::Selectable(label, ui.selectedNet == i)) ui.selectedNet = i;
        }
        ImGui::EndChild();
        ImGui::BeginDisabled(ui.selectedNet < 0);
        if (ImGui::Button("Connect")) ui.connectedNet = nets[ui.selectedNet];
        ImGui::EndDisabled();
        break;
    }
    case 4: { // Sound
        ImGui::SliderInt("Volume", &s.volume, 0, 100, "%d%%");
        ImGui::Checkbox("Mute", &s.mute);
        const char* out[] = { "Speakers (placeholder)", "Headphones (placeholder)", "HDMI Audio (placeholder)" };
        ImGui::Combo("Output device", &ui.outputDevice, out, IM_ARRAYSIZE(out));
        break;
    }
    case 5: { // About / system info
        ImGui::Text("CSOPESY Desktop OS Emulator v0.1");
        ImGui::TextDisabled("Built with GLFW + OpenGL + Dear ImGui %s", ImGui::GetVersion());
        ImGui::Spacing();
        ImGui::Text("Device name:  %s", ui.deviceName);
        ImGui::Text("Processor:    CSOPESY vCPU @ 3.20 GHz (placeholder)");
        ImGui::Text("Memory:       16.0 GB (placeholder)");
        ImGui::Text("Frame rate:   %.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Spacing();
        if (ImGui::Button("Reset all settings")) {
            s = AppSettings{};
            ui = SettingsUI{};
            ImGui::GetIO().FontGlobalScale = 1.0f;
            ApplyTheme(s);
        }
        break;
    }
    }

    ImGui::EndChild();
    ImGui::End();
}
