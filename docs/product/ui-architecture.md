# UI 架构：ImGui 组件化与 Design System

> **状态：** 规划文档（尚未实施）  
> **目的：** 为 NED 界面从「各页面散落 `PushStyleColor`」迁移到 **可复用组件 + 统一主题 + 面板边界**，供后续重构时对照。  
> **关联：** [ops-core-architecture.md](./ops-core-architecture.md)（`ops_core` 无 ImGui；Ops 面板走本架构）

---

## 1. 背景与问题

### 1.1 目标

建立接近 Qt/QML **Design System + 可复用控件 + 清晰布局边界** 的体验，同时保留 Dear ImGui 即时模式的优势（调试友好、与 C++ 同仓、无 Electron 开销）。

### 1.2 现状（2026-05 审计）

| 现象 | 位置示例 | 影响 |
|------|----------|------|
| 全局 ImGui 样式在启动时写死 | `util/init.cpp` | Bootstrap 蓝、圆角等与 Settings 不同源 |
| 运行时主题由 Settings 再改一遍 | `util/settings.cpp` | 与 `init.cpp` 重复、难追踪 |
| Embed 模式再复制一套背景色 | `ned_embed.cpp` | 第三处主题入口 |
| 编辑器选区/当前行色硬编码 | `editor/editor_render.cpp` | 与 Settings 中 `TextSelectedBg` 不一致 |
| 20+ 文件各自 `PushStyleColor/Var` | `welcome.cpp`、`ai_agent.cpp`、`file_tree.cpp` 等 | 无统一视觉，新面板只能复制粘贴 |
| 无 `ui/` 组件层 | — | 谈不上「可迁移组件化」 |

### 1.3 ImGui 与 Qt/QML 的差异（预期管理）

| Qt/QML | NED / Dear ImGui |
|--------|------------------|
| 声明式 `Button { }`、QML Style | 每帧 `ImGui::Button()` + 临时改样式 |
| 主题文件换肤全站生效 | 需自建 **语义 Token + Theme 模块** |
| 控件树可预览 | UI = C++ 代码，迁移靠组件 API 而非 QML 文件 |
| 成熟产品级控件库 | 工具型即时 GUI，扩展靠 wiki 插件或自封装 |

**结论：** 不能指望「拉一个库 = 整套 Qt Quick Controls」；模块化必须 **自建 `ui/` 层**，第三方库只作为 widgets 内部实现。

---

## 2. 目标架构（五层）

```
┌─────────────────────────────────────────────────────────────┐
│  L4  Shell          util/render.cpp — 分栏、Dock、调 Panel   │
├─────────────────────────────────────────────────────────────┤
│  L3  Panels         ui/panels/* — Settings / Agent / Ops   │
├─────────────────────────────────────────────────────────────┤
│  L2  Widgets        ui/widgets/* — Button、Card、Toggle…   │
├─────────────────────────────────────────────────────────────┤
│  L1  Theme          ui/ned_theme.* — 唯一 ImGuiStyle 入口    │
├─────────────────────────────────────────────────────────────┤
│  L0  Foundation     Dear ImGui + util/splitter、font、scroll │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
                    ops_core / editor 核心（无 ImGui 或仅编辑器内绘）
```

### 2.1 与 Qt 概念对照

| Qt/QML | NED 规划 |
|--------|----------|
| `Material.theme` / `Palette` | `NedTheme::ApplyGlobal()` / `ApplyFromSettings()` |
| `Button`、`Switch` | `NedButton()`、`NedToggle()` |
| `ApplicationWindow` | Shell（现有 `util/render.cpp` 演进） |
| `Page` | `ui/panels/settings_panel.cpp` 等 |
| 业务 Model / Service | `ops/`、`ai/mcp/`、`lsp/`（Panel 只调用 API） |

---

## 3. 目录规划（重构后）

```
ned/
├── ui/                              # 新建：UI 规范层（本仓库代码，非 submodule）
│   ├── ned_theme.h
│   ├── ned_theme.cpp
│   ├── widgets/
│   │   ├── style_scope.h            # RAII PushStyle（可自写，~30 行）
│   │   ├── button.h / .cpp
│   │   ├── card.h
│   │   ├── toggle.h
│   │   ├── input.h
│   │   └── notify.h                 # Phase 2：封装 imgui-notify
│   ├── layout/
│   │   └── splitter.h               # 自 util/ 迁入或 re-export
│   ├── panels/
│   │   ├── settings_panel.cpp       # 从 util/settings.cpp 拆 UI 部分
│   │   ├── agent_panel.cpp
│   │   ├── welcome_panel.cpp
│   │   └── ops_panel.cpp            # 与 ops_core 对接
│   └── shell.h                      # 主窗口布局入口
├── util/                            # 保留：与 UI 无关的基础设施
├── editor/                          # 编辑器核心；chrome 走 ui/widgets
├── ops/                             # ops_core：禁止 ImGui（见 ops-core-architecture）
└── lib/                             # 第三方 submodule
    ├── imgui/                       # 已有
    ├── nativefiledialog/            # 已有
    ├── imgui_toggle/                # Phase 2 可选
    ├── imgui-notify/                # Phase 2 可选
    └── implot/                      # Phase 3 可选
```

---

## 4. 必须模块（规范化前提）

以下五项为 **必须**，Wiki 扩展库 **不是** 架构前提。

### 4.1 NedTheme（L1）— 第一优先级

**职责：** 全项目唯一修改 `ImGuiStyle` 与语义色的入口。

**语义 Token 示例：**

```cpp
namespace NedTheme {
namespace Color {
  ImVec4 Surface();
  ImVec4 SurfaceElevated();
  ImVec4 Accent();
  ImVec4 Border();
  ImVec4 EditorSelection();   // 从 editor_render 迁出
  ImVec4 EditorCurrentLine();
}
namespace Metric {
  float RadiusMd();           // 对应 FrameRounding
  ImVec2 PaddingPanel();
}
void ApplyBootstrapDefaults();           // 替代 init.cpp 内联配色
void ApplyFromSettings(const nlohmann::json& settings);
}
```

**迁移来源：**

| 现有文件 | 迁到 NedTheme |
|----------|----------------|
| `util/init.cpp` `initializeImGui()` 配色块 | `ApplyBootstrapDefaults()` |
| `util/settings.cpp` `ApplySettings()` / `applyImGuiStyles()` | `ApplyFromSettings()` |
| `ned_embed.cpp` 背景色 | 调同一 API |
| `editor/editor_render.cpp` 选区/当前行常量 | `Color::EditorSelection` 等 |

**规则：** 除 `ned_theme.cpp` 与 **编辑器语法高亮主题**（`EditorHighlight`）外，禁止直接写 `ImGui::GetStyle().Colors[...]`。

### 4.2 ui/widgets（L2）— 最小控件集

| 组件 | 职责 |
|------|------|
| `NedStyleScope` | RAII 包 `PushStyleColor/Var`，防漏 Pop |
| `NedButton` / `NedIconButton` | 主 / 次 / 危险按钮 |
| `NedToggle` | 设置开关（先包 Checkbox，后可换 imgui_toggle） |
| `NedInput` / `NedCombo` | 表单 |
| `NedSection` / `NedCard` | 面板标题 + 内边距 |
| `NedScrollPanel` | 统一滚动区样式 |

**规则：** `panels/`、`editor/` 外壳只 `#include "ui/widgets/..."`；**禁止**在 panel 内 `PushStyleColor`（除非在 widgets 实现内部）。

### 4.3 Panels（L3）

- 每个 major UI 区域一个文件、一个 `DrawXxxPanel()`。
- Panel **不互相 include** UI 细节；共享只通过 `widgets` + `NedTheme`。
- `util/settings.cpp` 长期目标：只保留持久化与 `ApplyFromSettings()` 调用，绘制逻辑迁入 `ui/panels/settings_panel.cpp`。

### 4.4 Shell（L4）

- 现有 `util/render.cpp` + `util/splitter.cpp` 负责主布局。
- Shell 只负责：分栏、可见性、调用各 `DrawXxxPanel()`。
- `Splitter` 可保留在 `util/` 或迁至 `ui/layout/`，但 **只有一个** 布局权威。

### 4.5 业务 / UI 分离

与 [ops-core-architecture.md](./ops-core-architecture.md) 一致：

| 模块 | ImGui | 内容 |
|------|-------|------|
| `ops/`（ops_core） | ❌ | Playbook、状态机、SQLite、进程 |
| `ui/panels/ops_panel.cpp` | ✅ | 调 ops_core API，只渲染 |
| `ned-runner` | ❌ | CLI |
| `ai/mcp/`、`lsp/` | ❌ | Panel 通过 service 接口调用 |

---

## 5. 编码规范（重构期强制执行）

1. **颜色** 只来自 `NedTheme::Color::*`，禁止 magic `ImVec4` 散落在业务文件。
2. **控件** 只来自 `ui/widgets/`，禁止 panel 直接调第三方 widget 头文件。
3. **新窗口** = 新 `ui/panels/xxx_panel.cpp`，不在 `render.cpp` 堆数百行 UI。
4. **第三方 ImGui 扩展** 只允许在 `ui/widgets/` 内 `#include` 并封装（如 `NedNotify` 包 `imgui-notify`）。
5. **ImGui 窗口名** 用命名空间常量，避免字符串散落。
6. **Editor 语法色** 继续走 `EditorHighlight` / theme JSON；**Editor 壳层**（行号、书签 UI）走 `NedTheme`。

---

## 6. 已有能力 — 不必替换

| 能力 | 现状 | 重构策略 |
|------|------|----------|
| 文本编辑器 | 自研 + Tree-sitter | **保留**；不引入 ImGuiColorTextEdit |
| 文本选择 | `ai/textselect.*`（ImGuiTextSelect vendored） | 保留；可选改为 submodule 跟踪版本 |
| 分栏 | `util/splitter.cpp` | 保留，纳入 layout 层 |
| 文件对话框 | `lib/nativefiledialog` | 保留；内置风格文件树非必须 |
| Docking | ImGui 内置 | 继续用 |

---

## 7. 外部仓库：分阶段拉取清单

> 参考：[Dear ImGui Useful Extensions](https://github.com/ocornut/imgui/wiki/Useful-Extensions)  
> **原则：** 第三方库一律放在 `lib/` 正式 submodule；业务封装在 `ui/widgets/`。  
> **勿使用** 仓库根下 `module/` 嵌套 clone（已废弃并移除）。

### 7.0 已纳入 submodule（Agent / Ops）

| 路径 | 上游 | 用途 | 封装 |
|------|------|------|------|
| `lib/imgui_markdown` | [enkisoftware/imgui_markdown](https://github.com/enkisoftware/imgui_markdown) | Agent Markdown | `ui/ned_markdown.*` → CMake `ned_ui` |
| `lib/imgui-node-editor` | [thedmd/imgui-node-editor](https://github.com/thedmd/imgui-node-editor) | Playbook 节点图 | `ui/ned_node_editor.*` → CMake `imgui_node_editor` + `ned_ui` |

**CMake target：**

- `imgui_node_editor` — 静态库（4 个 `.cpp`，不编 `external/imgui`）
- `ned_ui` — NED 封装层，链接 `imgui_node_editor`；`ned` / `ned_embed` 链接 `ned_ui`

**业务接入：**

- Agent assistant 消息 → `NedMarkdown::Render()`（`ai/ai_agent.cpp`）
- Playbook 面板 → 使用 `NedNodeEditor::Context` / `Session`（待 Ops UI PR）

克隆本仓库后初始化：

```bash
git submodule update --init lib/imgui_markdown lib/imgui-node-editor
```

**集成约束（后续 CMake PR）：**

- `imgui_markdown`：header-only，include `lib/imgui_markdown`；仅在 `ui/widgets/ned_markdown.*` 引用。
- `imgui-node-editor`：只编译根目录 4 个 `.cpp`（**不要**编 `external/imgui`）；链接 NED 已有 `lib/imgui`。

### 7.1 不要拉

| 仓库 | 原因 |
|------|------|
| ImGuiColorTextEdit 各 fork | 替换整个 editor，与目标冲突 |
| aiekick/ImGuiFileDialog 等 | 已有 NFD |
| ImRAD / FellowImGui / Imery | 设计器/声明式；widgets 稳定后再评估 |
| netImgui / imgui-ws | 远程 UI，非当前痛点 |

### 7.2 Phase 0 — 规范化地基（必须，0 个新仓库）

在 NED 内新建 `ui/ned_theme.*`、`ui/widgets/*`，不 submodule。

**只读参考（不引入代码）：**

- [Adobe imgui Spectrum 文档](https://github.com/adobe/imgui/blob/master/docs/Spectrum.md) — Token 命名与色板思路

### 7.3 Phase 1 — 样式 RAII（0～1 个）

| 仓库 | 用途 | 建议 |
|------|------|------|
| [mnesarco/imgui_sugar](https://github.com/mnesarco/imgui_sugar) | RAII 包 PushStyle | 可选；**更推荐自写 `NedStyleScope`** |

```bash
# 仅当不自写 StyleScope 时：
git submodule add https://github.com/mnesarco/imgui_sugar.git lib/imgui_sugar
```

### 7.4 Phase 2 — 反馈与开关（0～3 个）

| 仓库 | 用途 |
|------|------|
| [patrickcjk/imgui-notify](https://github.com/patrickcjk/imgui-notify) | Toast（保存失败、API 错误） |
| [dalerank/imspinner](https://github.com/dalerank/imspinner) | AI / Runner loading |
| [cmdwft/imgui_toggle](https://github.com/cmdwft/imgui_toggle) | 现代开关样式 |

```bash
git submodule add https://github.com/patrickcjk/imgui-notify.git lib/imgui-notify
git submodule add https://github.com/dalerank/imspinner.git lib/imspinner
git submodule add https://github.com/cmdwft/imgui_toggle.git lib/imgui_toggle
```

CMake：仅 `target_include_directories` + 在 `ui/widgets/` 封装；业务代码不直接 include。

### 7.5 Phase 3 — Ops / Agent 可视化

| 仓库 | 用途 | 状态 |
|------|------|------|
| `lib/imgui_markdown` | Agent 回复 / Playbook 预览 | **已 submodule** |
| `lib/imgui-node-editor` | Playbook 节点图（比 imnodes 更适合蓝图式编辑） | **已 submodule** |
| [epezent/implot](https://github.com/epezent/implot) | 迁移进度、指标曲线 | 待加 |
| [Nelarius/imnodes](https://github.com/Nelarius/imnodes) | 轻量节点图（若 node-editor 过重再考虑） | 可选 |

### 7.6 可选：规范化 vendored 代码

| 仓库 | 现状 |
|------|------|
| [AidanSun05/ImGuiTextSelect](https://github.com/AidanSun05/ImGuiTextSelect) | 已拷贝至 `ai/textselect.*`；可改为 submodule 便于更新 |

### 7.7 阶段汇总

| 阶段 | 新 submodule 数量 | 交付物 |
|------|-------------------|--------|
| Phase 0 | **0** | `NedTheme` + 基础 widgets + Settings 面板样板 |
| Phase 1 | 0～1 | `NedStyleScope` 或 imgui_sugar |
| Phase 2 | 0～3 | `NedNotify`、`NedSpinner`、`NedToggle` 封装 |
| Phase 3 | 0～3 | Ops 图表、流程图、Markdown 预览 |

---

## 8. 实施顺序（建议）

```
Step 1  ui/ned_theme.*
        收拢 init.cpp、settings.cpp、ned_embed.cpp、editor 选区色

Step 2  ui/widgets/style_scope + button + card
        最小可复用 API

Step 3  ui/panels/settings_panel.cpp
        第一个「纯组件」样板；util/settings.cpp 只留数据与 Apply

Step 4  文档与 CI 约定
        新 PR 禁止在 panels 外新增 PushStyleColor（逐步 lint/审查）

Step 5  迁移 welcome、agent、file_tree 等高频页面

Step 6  ui/panels/ops_panel.cpp
        与 ops_core MVP 同步（见 product/README 实施顺序）
```

### 8.1 与产品路线图的关系

| 产品步骤 | UI 架构步骤 |
|----------|-------------|
| ops_core MVP | Phase 0 Theme + Ops Panel 壳 |
| NED Ops UI | Phase 3 implot / imnodes |
| 代码 Agent 增强 | Phase 2 notify / spinner；Agent Panel 迁 widgets |

---

## 9. 迁移检查清单（重构 PR 自检）

- [ ] 是否仍有新增 `PushStyleColor` 在 `ui/panels/` 之外？
- [ ] 新颜色是否进入 `NedTheme::Color`？
- [ ] 第三方 ImGui 头文件是否只出现在 `ui/widgets/`？
- [ ] Panel 是否只通过 service/API 访问 ops、MCP、LSP？
- [ ] `init.cpp` / `settings.cpp` 是否仍直接写 `ImGuiStyle`（应只剩对 NedTheme 的调用）？
- [ ] Editor 选区色是否与 `NedTheme::EditorSelection` 一致？

---

## 10. 非目标（本阶段不做）

- 替换自研文本编辑器为 ImGuiColorTextEdit
- 全站声明式 UI（YAML / ImRAD）— widgets 稳定后再评估
- 1:1 复刻 VS Code / Cursor 视觉（Electron + CSS 体系不同）
- 为 UI 规范化一次性引入 Wiki 清单中大量扩展库

---

## 11. 参考资料

- [Dear ImGui Useful Extensions](https://github.com/ocornut/imgui/wiki/Useful-Extensions)
- [Adobe imgui Spectrum](https://github.com/adobe/imgui/blob/master/docs/Spectrum.md)
- 本仓库：[ops-core-architecture.md](./ops-core-architecture.md)
- 本仓库：[agent-platform-vision.md](./agent-platform-vision.md)

---

*文档版本：2026-05-30 · 随 UI 重构进展更新「状态」与各 Phase 完成情况。*
