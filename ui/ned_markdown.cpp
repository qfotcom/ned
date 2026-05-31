#include "ned_markdown.h"

#include "imgui_markdown.h"

#include <cstdlib>
#include <string>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <shellapi.h>
#endif

namespace {

bool openUrl(const std::string &url)
{
	if (url.empty())
		return false;

#ifdef PLATFORM_WINDOWS
	return reinterpret_cast<intptr_t>(
			   ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) >
		   32;
#elif defined(PLATFORM_MACOS)
	std::string cmd = "open \"" + url + "\"";
	return std::system(cmd.c_str()) == 0;
#else
	std::string cmd = "xdg-open \"" + url + "\"";
	return std::system(cmd.c_str()) == 0;
#endif
}

void linkCallback(ImGui::MarkdownLinkCallbackData data)
{
	if (data.isImage || data.linkLength == 0)
		return;
	openUrl(std::string(data.link, data.linkLength));
}

ImGui::MarkdownImageData imageCallback(ImGui::MarkdownLinkCallbackData data)
{
	(void)data;
	ImGui::MarkdownImageData imageData{};
	imageData.isValid = false;
	return imageData;
}

ImGui::MarkdownConfig buildConfig()
{
	ImGui::MarkdownConfig config{};
	config.linkCallback = linkCallback;
	config.tooltipCallback = nullptr;
	config.imageCallback = imageCallback;
	config.linkIcon = nullptr;
	config.formatCallback = ImGui::defaultMarkdownFormatCallback;
	config.userData = nullptr;

	ImFont *font = ImGui::GetFont();
	for (int i = 0; i < ImGui::MarkdownConfig::NUMHEADINGS; ++i)
		config.headingFormats[i] = {font, true};

	return config;
}

} // namespace

namespace NedMarkdown {

void Render(const std::string &markdown)
{
	Render(markdown, ImGui::GetContentRegionAvail().x);
}

void Render(const std::string &markdown, float /*wrapWidth*/)
{
	if (markdown.empty())
		return;

	// imgui_markdown performs its own word wrap; do not use PushTextWrapPos here
	// (it clips long documents to only the first few lines).
	ImGui::MarkdownConfig config = buildConfig();
	ImGui::Markdown(markdown.c_str(), markdown.size(), config);
}

} // namespace NedMarkdown
