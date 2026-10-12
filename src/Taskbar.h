#pragma once

// Draws the taskbar and for each app that is currently open, draws that app's window too.
// Call once per frame from main.cpp, after the desktop is drawn and before ImGui::Render().
void DrawTaskbar();