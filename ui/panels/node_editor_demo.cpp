#include "node_editor_demo.h"

#include "blueprints/blueprints_editor.h"

#include <imgui.h>

NodeEditorDemo &NodeEditorDemo::instance()
{
	static NodeEditorDemo demo;
	return demo;
}

NodeEditorDemo &gNodeEditorDemo = NodeEditorDemo::instance();

void NodeEditorDemo::open()
{
	if (!m_editor)
		m_editor = new BlueprintsEditor();
	m_editor->startup();
	m_open = true;
}

void NodeEditorDemo::close()
{
	if (m_editor)
		m_editor->shutdown();
	m_open = false;
}

void NodeEditorDemo::render()
{
	if (!m_open || !m_editor)
		return;

	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	ImGui::Begin("##NodeEditorDemo",
				 nullptr,
				 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

	if (ImGui::Button("Back"))
		close();

	ImGui::SameLine();
	ImGui::TextDisabled("| Blueprint Editor  (Welcome / Node / Ctrl+Shift+N)");

	ImGui::Separator();

	m_editor->frame();

	ImGui::End();
}
