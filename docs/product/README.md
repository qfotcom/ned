# 产品扩展规划

基于 NED fork 的后续方向：**Cursor 级代码索引**、**通用 Agent / Playbook 运维平台（`ops_core`）**、以及 **Kudu 集群迁移** 参考场景。

开发协作（Git、upstream 合并）见 [development/git-workflow.md](../development/git-workflow.md)。

---

## 文档索引

| 文档 | 内容 |
|------|------|
| [agent-platform-vision.md](./agent-platform-vision.md) | 通用 Agent 形态、与 Cursor 对比、Skill vs Playbook、数字分身演进 |
| [codebase-indexing.md](./codebase-indexing.md) | Cursor 同款全项目索引：现状、目标架构、分阶段落地 |
| [ops-core-architecture.md](./ops-core-architecture.md) | `ops_core` + 双 target（`ned` / `ned-runner`）、bundle、跨平台 |
| [ui-architecture.md](./ui-architecture.md) | ImGui 组件化、NedTheme、widgets/panels 分层、外部库分阶段引入 |
| [kudu-migration-scenario.md](./kudu-migration-scenario.md) | Kudu→Kudu 五阶段迁移：Playbook 示例、门禁、与 C++ 工具对接 |

---

## 架构速览

```
┌─────────────────────────────────────────────────────────────┐
│  NED（Windows / Linux GUI）— 编制、编辑、AI、进度可视化          │
│  · 代码 Agent：索引 + MCP grep + 现有 Morph editFile          │
│  · 运维 Agent：Playbook 编辑、Pack bundle、远程 Runner 状态    │
└───────────────────────────┬─────────────────────────────────┘
                            │  .ops.bundle（YAML，跨平台）
                            ▼
┌─────────────────────────────────────────────────────────────┐
│  ned-runner（Windows / Linux headless）— 执行 Playbook        │
│  · ops_core：状态机、门禁、SQLite 进度                         │
│  · 调用外部 migrate_cli（Kudu 迁移 C++ 工具）                 │
└─────────────────────────────────────────────────────────────┘
```

- **一套 `ops_core` 源码**，**两个 CMake target**（`ned` + `ned-runner`），**一个 Git 仓库**。
- **Playbook** = 可执行流程真相；**Skill** = Agent 读 Playbook 的适配层。
- **LLM** 负责推理与汇报；**时间窗、门禁、阶段** 由 Playbook + Runner 确定性执行。

---

## 建议实施顺序

1. **代码索引 MVP** — MCP `grepProject` + Tree-sitter 符号表  
2. **ops_core MVP** — YAML Playbook + mock CLI + SQLite + `ned-runner`  
3. **UI 规范层 Phase 0** — `NedTheme` + widgets + Settings 面板样板（见 [ui-architecture.md](./ui-architecture.md)）  
4. **NED Ops UI** — Playbook 编辑、Validate、Pack、进度面板  
5. **接入真实 Kudu migrate_cli** + Linux 生产部署  
6. **语义向量索引**（可选）

各步细节见上表对应文档。
