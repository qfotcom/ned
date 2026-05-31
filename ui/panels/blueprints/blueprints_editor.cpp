#define IMGUI_DEFINE_MATH_OPERATORS
#include "blueprints_editor.h"

#include "blueprints_texture.h"

#include "blueprints_example_panel.inc"

BlueprintsEditor::BlueprintsEditor() = default;

BlueprintsEditor::~BlueprintsEditor()
{
	shutdown();
}

void BlueprintsEditor::startup()
{
	if (!m_panel)
		m_panel = new BlueprintsExamplePanel();
	m_panel->startup();
}

void BlueprintsEditor::shutdown()
{
	if (!m_panel)
		return;
	m_panel->shutdown();
	delete m_panel;
	m_panel = nullptr;
}

void BlueprintsEditor::frame()
{
	if (m_panel)
		m_panel->frame();
}
