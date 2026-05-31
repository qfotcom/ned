#include "ned_node_editor.h"

namespace NedNodeEditor {

Context::Context(const char *settingsFile)
{
	ed::Config config;
	if (settingsFile && settingsFile[0] != '\0')
		config.SettingsFile = settingsFile;
	else
		config.SettingsFile = nullptr;

	m_context = ed::CreateEditor(&config);
	if (!m_context)
		return;

	ed::SetCurrentEditor(m_context);
	applyNedTheme();
}

Context::~Context()
{
	if (m_context)
	{
		ed::DestroyEditor(m_context);
		m_context = nullptr;
	}
}

void Context::applyNedTheme()
{
	if (!m_context)
		return;

	const ImVec4 accent = ImGui::GetStyle().Colors[ImGuiCol_Header];
	const ImVec4 surface = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	const ImVec4 border = ImGui::GetStyle().Colors[ImGuiCol_Border];
	const ImVec4 childBg = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];

	ed::Style &style = ed::GetStyle();
	style.NodeRounding = ImGui::GetStyle().ChildRounding;

	style.Colors[ed::StyleColor_Bg] = childBg;
	style.Colors[ed::StyleColor_Grid] = ImVec4(border.x, border.y, border.z, 0.35f);
	style.Colors[ed::StyleColor_NodeBg] =
		ImVec4(surface.x * 1.12f, surface.y * 1.12f, surface.z * 1.12f, 1.0f);
	style.Colors[ed::StyleColor_NodeBorder] = border;
	style.Colors[ed::StyleColor_HovNodeBorder] = accent;
	style.Colors[ed::StyleColor_SelNodeBorder] = accent;
	style.Colors[ed::StyleColor_LinkSelRect] =
		ImVec4(accent.x, accent.y, accent.z, 0.25f);
	style.Colors[ed::StyleColor_Flow] = accent;
	style.Colors[ed::StyleColor_FlowMarker] = accent;
}

Session::Session(ed::EditorContext *context, const char *id, const ImVec2 &size)
{
	m_previous = ed::GetCurrentEditor();
	ed::SetCurrentEditor(context);
	ed::Begin(id, size);
}

Session::~Session()
{
	ed::End();
	ed::SetCurrentEditor(m_previous);
}

} // namespace NedNodeEditor
