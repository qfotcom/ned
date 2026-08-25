#include "implot_demo.h"

#include "implot_demo_embed.h"

#include <imgui.h>
#include "implot.h"

ImPlotDemo &ImPlotDemo::instance()
{
	static ImPlotDemo demo;
	return demo;
}

ImPlotDemo &gImPlotDemo = ImPlotDemo::instance();

void ImPlotDemo::open() { m_open = true; }

void ImPlotDemo::close()
{
	NedImPlotEmbed::setActive(false);
	m_open = false;
}

void ImPlotDemo::render()
{
	if (!m_open)
		return;

	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	ImGui::Begin("##ImPlotDemoHost",
				 nullptr,
				 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

	if (ImGui::Button("Back"))
		close();

	ImGui::SameLine();
	ImGui::TextDisabled("| ImPlot Demo  (Welcome / Ctrl+Shift+H)");

	ImGui::Separator();

	ImVec2 demoPos = ImGui::GetCursorScreenPos();
	ImVec2 demoSize = ImGui::GetContentRegionAvail();
	NedImPlotEmbed::setActive(true, demoPos, demoSize);
	ImPlot::ShowDemoWindow(nullptr);

	ImGui::End();
}
