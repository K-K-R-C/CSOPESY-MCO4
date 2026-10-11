#include "Boot.h"
#include "imgui.h"
#include <algorithm>

namespace {
// Figlet-style "CSOPESY": 7 glyphs x 5 rows, each 7 chars wide
const char* kGlyphs[7][5] = {
    {R"(  ____ )", R"( / ___|)", R"(| |    )", R"(| |___ )", R"( \____|)"},  // C
    {R"( ____  )", R"(/ ___| )", R"(\___ \ )", R"( ___) |)", R"(|____/ )"},  // S
    {R"(  ___  )", R"( / _ \ )", R"(| | | |)", R"(| |_| |)", R"( \___/ )"},  // O
    {R"( ____  )", R"(|  _ \ )", R"(| |_) |)", R"(|  __/ )", R"(|_|    )"},  // P
    {R"( _____ )", R"(| ____|)", R"(|  _|  )", R"(| |___ )", R"(|_____|)"},  // E
    {R"( ____  )", R"(/ ___| )", R"(\___ \ )", R"( ___) |)", R"(|____/ )"},  // S
    {R"(__   __)", R"(\ \ / /)", R"( \ V / )", R"(  | |  )", R"(  |_|  )"},  // Y
};

void beginFullscreenBlack(const char* name) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 255));
    ImGui::Begin(name, nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav);
    ImGui::PopStyleColor();
}

void centered(const std::string& s) {
    float w = ImGui::CalcTextSize(s.c_str()).x;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - w) * 0.5f);
    ImGui::TextUnformatted(s.c_str());
}
}  // namespace

BootSequence::BootSequence() {
    // EDIT THESE LINES to personalise your group's BIOS screen.
    struct Src { const char* text; bool ram; };
    const Src src[] = {
        {"CSOPESY Megatrends", false},
        {"Released: 12/01/94", false},
        {"DLSU GAME Lab (C)1994 CSOPESY Megatrends Inc.", false},
        {"", false},
        {"Memory Test:", false},
        {"Checking RAM :", true},  // animated counter
        {"", false},
        {"CPU Type: Pentium III", false},
        {"BIOS Version: CSOPESY Special Edition", false},
        {"Main Processor: Pentium III", false},
        {"Numeric Processor: Built-in", false},
        {"", false},
        {"Primary Master:     420 MB WDCAC2420H", false},
        {"Primary Slave:      None", false},
        {"Secondary Master:   CD-ROM LTN-305A", false},
        {"Secondary Slave:    None", false},
        {"", false},
        {"Group <NN>: <member names here>", false},
    };
    float at = 0.4f;
    for (const Src& s : src) {
        lines_.push_back({s.text, s.ram, at});
        at += s.ram ? 1.6f : (s.text[0] ? 0.18f : 0.08f);
    }
    biosEnd_ = at + 1.0f;
    for (int r = 0; r < 5; ++r)
        for (int g = 0; g < 7; ++g) logo_[r] += kGlyphs[g][r];
}

bool BootSequence::drawBios(float dt) {
    t_ += dt;
    beginFullscreenBlack("##bios");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
    ImGui::SetCursorPos(ImVec2(24, 24));
    ImGui::BeginGroup();
    for (const BiosLine& l : lines_) {
        if (t_ < l.at) break;
        if (l.text.empty()) { ImGui::Dummy(ImVec2(0, ImGui::GetTextLineHeight())); continue; }
        if (l.ram) {
            float p = std::min(1.0f, (t_ - l.at) / 1.3f);
            ImGui::Text("%s %dK%s", l.text.c_str(), static_cast<int>(p * 64000), p >= 1.0f ? " OK" : "");
        } else {
            ImGui::TextUnformatted(l.text.c_str());
        }
    }
    ImGui::EndGroup();
    ImGui::PopStyleColor();
    ImGui::End();

    bool done = t_ >= biosEnd_ || ImGui::IsKeyPressed(ImGuiKey_Space, false);
    if (done) t_ = 0.0f;
    return done;
}

bool BootSequence::drawSplash(float dt) {
    t_ += dt;
    const float duration = 3.5f, alpha = std::min(1.0f, t_ / 0.8f);  // fade in

    beginFullscreenBlack("##splash");
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.32f);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.75f, 0.95f, 1.0f));
    for (const auto& row : logo_) centered(row);
    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0, 24));
    centered("CSOPESY Operating System Emulator v1.0");
    centered("Created By: Dr. Neil Patrick Del Gallego");
    ImGui::Dummy(ImVec2(0, 24));

    static const char* dots[] = {"Loading", "Loading.", "Loading..", "Loading..."};
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.9f, 0.4f, 1.0f));
    centered(dots[static_cast<int>(t_ * 3.0f) % 4]);
    ImGui::PopStyleColor();

    ImGui::PopStyleVar();
    ImGui::End();

    bool done = t_ >= duration || ImGui::IsKeyPressed(ImGuiKey_Space, false);
    if (done) t_ = 0.0f;
    return done;
}