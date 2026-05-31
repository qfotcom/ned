#include "blueprints_texture.h"

#include "config.h"

#include <GL/glew.h>
#include <lib/stb_image.h>

#include <filesystem>
#include <iostream>
#include <map>

namespace {

struct TextureEntry
{
	GLuint id = 0;
	int width = 0;
	int height = 0;
};

std::map<ImTextureID, TextureEntry> &textureMap()
{
	static std::map<ImTextureID, TextureEntry> textures;
	return textures;
}

std::string resolveBlueprintDataPath(const char *filename)
{
	const std::string rel = std::string(SOURCE_DIR) +
							"/lib/imgui-node-editor/examples/blueprints-example/data/" +
							filename;
	return rel;
}

} // namespace

ImTextureID loadBlueprintTexture(const char *path)
{
	const std::string file = resolveBlueprintDataPath(path);
	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char *data = stbi_load(file.c_str(), &width, &height, &channels, 4);
	if (!data)
	{
		std::cerr << "Blueprints: failed to load texture: " << file << std::endl;
		return ImTextureID{};
	}

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	const ImTextureID id = static_cast<ImTextureID>(tex);
	textureMap()[id] = TextureEntry{tex, width, height};
	return id;
}

void destroyBlueprintTexture(ImTextureID &texture)
{
	if (!texture)
		return;

	auto &map = textureMap();
	const auto it = map.find(texture);
	if (it != map.end())
	{
		glDeleteTextures(1, &it->second.id);
		map.erase(it);
	}
	texture = ImTextureID{};
}

int getBlueprintTextureWidth(ImTextureID texture)
{
	const auto it = textureMap().find(texture);
	return it != textureMap().end() ? it->second.width : 0;
}

int getBlueprintTextureHeight(ImTextureID texture)
{
	const auto it = textureMap().find(texture);
	return it != textureMap().end() ? it->second.height : 0;
}
