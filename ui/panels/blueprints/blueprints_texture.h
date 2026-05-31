#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>

ImTextureID loadBlueprintTexture(const char *filename);
void destroyBlueprintTexture(ImTextureID &texture);
int getBlueprintTextureWidth(ImTextureID texture);
int getBlueprintTextureHeight(ImTextureID texture);
