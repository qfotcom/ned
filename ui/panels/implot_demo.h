#pragma once

// Full-page ImPlot demo (all chart examples from implot_demo.cpp).
class ImPlotDemo
{
  public:
	static ImPlotDemo &instance();

	void open();
	void close();
	bool isOpen() const { return m_open; }
	void render();

  private:
	ImPlotDemo() = default;

	bool m_open = false;
};

extern ImPlotDemo &gImPlotDemo;
