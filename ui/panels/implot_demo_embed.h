#pragma once

#include <imgui.h>

// Lets NED host ImPlot::ShowDemoWindow inside a full-page panel.
namespace NedImPlotEmbed
{
void setActive(bool active, ImVec2 pos = ImVec2(0, 0), ImVec2 size = ImVec2(0, 0));
bool isActive();
ImVec2 pos();
ImVec2 size();
} // namespace NedImPlotEmbed
