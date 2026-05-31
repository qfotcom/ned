#pragma once

class BlueprintsExamplePanel;

class BlueprintsEditor
{
  public:
	BlueprintsEditor();
	~BlueprintsEditor();

	void startup();
	void shutdown();
	void frame();

  private:
	BlueprintsExamplePanel *m_panel = nullptr;
};
