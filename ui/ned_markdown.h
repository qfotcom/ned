#pragma once

#include <imgui.h>
#include <string>

// NED wrapper around lib/imgui_markdown (single-header).
// Business code should include this header only, not imgui_markdown.h directly.
namespace NedMarkdown {

// Render markdown in the current ImGui window (uses content region width).
void Render(const std::string &markdown);

// Render with an explicit wrap width in pixels.
void Render(const std::string &markdown, float wrapWidth);

} // namespace NedMarkdown
