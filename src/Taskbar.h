#pragma once
class Desktop;

// Draws the taskbar panel. Called once per frame by the Desktop (see Desktop::render).
// Opening/closing apps goes through the Desktop: desktop.toggle("files"|"settings"|"taskmgr").
void DrawTaskbar(Desktop& desktop);