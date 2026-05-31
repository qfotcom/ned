#pragma once

// Full-page imgui-node-editor blueprints-example (opened from Welcome / header).
class BlueprintsEditor;

class NodeEditorDemo
{
  public:
	static NodeEditorDemo &instance();

	void open();
	void close();
	bool isOpen() const { return m_open; }
	void render();

  private:
	NodeEditorDemo() = default;

	bool m_open = false;
	BlueprintsEditor *m_editor = nullptr;
};

extern NodeEditorDemo &gNodeEditorDemo;
