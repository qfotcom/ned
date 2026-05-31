#!/usr/bin/env python3
"""Adapt imgui-node-editor blueprints-example.cpp for NED."""
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SUBMODULE = ROOT / "lib" / "imgui-node-editor"
OUT = ROOT / "ui" / "panels" / "blueprints" / "blueprints_example_panel.inc"

src = subprocess.check_output(
    ["git", "-C", str(SUBMODULE), "show", "HEAD:examples/blueprints-example/blueprints-example.cpp"],
    text=True,
    errors="replace",
)

header = """\
#define IMGUI_DEFINE_MATH_OPERATORS
#include "blueprints_texture.h"
#include "utilities/builders.h"
#include "utilities/widgets.h"

#include <imgui.h>
#include <misc/stacklayout/imgui_stacklayout.h>
#include <imgui_node_editor.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

"""

body = src
cut = body.find("static inline ImRect ImGui_GetItemRect")
if cut >= 0:
    body = body[cut:]

body = body.replace("struct Example:\n    public Application", "struct BlueprintsExamplePanel")
body = body.replace("    using Application::Application;\n\n", "")
body = body.replace("void OnStart() override", "void startup()")
body = body.replace("void OnStop() override", "void shutdown()")
body = body.replace("void OnFrame(float deltaTime) override", "void frame()")
body = body.replace("static_cast<Example*>", "static_cast<BlueprintsExamplePanel*>")
body = body.replace("LoadTexture(", "loadBlueprintTexture(")
body = body.replace("DestroyTexture(", "destroyBlueprintTexture(")
body = body.replace("GetTextureWidth(", "getBlueprintTextureWidth(")
body = body.replace("GetTextureHeight(", "getBlueprintTextureHeight(")
body = body.replace("ImGui::GetKeyIndex(ImGuiKey_Z)", "ImGuiKey_Z")
body = body.replace("ImTextureID          m_HeaderBackground = nullptr;",
                    "ImTextureID          m_HeaderBackground = ImTextureID_Invalid;")
body = body.replace("ImTextureID          m_SaveIcon = nullptr;",
                    "ImTextureID          m_SaveIcon = ImTextureID_Invalid;")
body = body.replace("ImTextureID          m_RestoreIcon = nullptr;",
                    "ImTextureID          m_RestoreIcon = ImTextureID_Invalid;")
body = body.replace("                id = nullptr;", "                id = ImTextureID_Invalid;")
body = re.sub(r"\nint Main\(int argc, char\*\* argv\)[\s\S]*\Z", "\n", body)

body = body.replace('loadBlueprintTexture("data/', 'loadBlueprintTexture("')

OUT.write_text(header + body, encoding="utf-8", newline="\n")
print(f"Wrote {OUT} ({OUT.stat().st_size} bytes)")
