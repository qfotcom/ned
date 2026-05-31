#include "markdown_file.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

namespace NedMarkdownFile {

bool isMarkdownPath(const std::string &path)
{
	if (path.empty())
		return false;

	std::string ext = fs::path(path).extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return ext == ".md" || ext == ".markdown";
}

} // namespace NedMarkdownFile
