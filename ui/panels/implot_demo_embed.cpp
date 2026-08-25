#include "implot_demo_embed.h"

namespace NedImPlotEmbed
{
namespace
{
bool g_active = false;
ImVec2 g_pos;
ImVec2 g_size;
} // namespace

void setActive(bool active, ImVec2 pos, ImVec2 size)
{
	g_active = active;
	g_pos = pos;
	g_size = size;
}

bool isActive() { return g_active; }

ImVec2 pos() { return g_pos; }

ImVec2 size() { return g_size; }
} // namespace NedImPlotEmbed
