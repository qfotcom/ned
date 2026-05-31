#pragma once

#include <imgui.h>
#include <imgui_node_editor.h>

#include <memory>
#include <string>

// NED wrapper around lib/imgui-node-editor.
// Playbook / workflow graph panels should use these types, not ax::NodeEditor directly.
namespace NedNodeEditor {

namespace ed = ax::NodeEditor;

// Owns an ax::NodeEditor::EditorContext; applies NED-aligned default colors.
class Context
{
  public:
	explicit Context(const char *settingsFile = "ned_node_editor.json");
	~Context();

	Context(const Context &) = delete;
	Context &operator=(const Context &) = delete;

	ed::EditorContext *get() const { return m_context; }
	void applyNedTheme();

  private:
	ed::EditorContext *m_context = nullptr;
};

// RAII: SetCurrentEditor + Begin("id") / End + restore previous editor.
class Session
{
  public:
	Session(ed::EditorContext *context, const char *id, const ImVec2 &size = ImVec2(0, 0));
	~Session();

	Session(const Session &) = delete;
	Session &operator=(const Session &) = delete;

  private:
	ed::EditorContext *m_previous = nullptr;
};

} // namespace NedNodeEditor
