# NED 文档中心

本目录存放 **fork 定制版 NED** 的说明文档，按用途分子目录，避免 `doc/` / `docs/` 混用。

---

## 目录结构

```
docs/
├── README.md                 ← 你在这里（总索引）
├── development/              ← 仓库协作、编译、日常开发
│   └── git-workflow.md
└── product/                  ← 产品规划、Agent、ops_core、参考场景
    ├── README.md
    ├── agent-platform-vision.md
    ├── codebase-indexing.md
    ├── ops-core-architecture.md
    ├── ui-architecture.md
    └── kudu-migration-scenario.md
```

---

## 该看哪一份？

| 你想… | 文档 |
|--------|------|
| 配置 fork、合并 upstream、日常 `git push` | [development/git-workflow.md](./development/git-workflow.md) |
| 了解 Agent / Cursor / Skill vs Playbook | [product/agent-platform-vision.md](./product/agent-platform-vision.md) |
| 规划 Cursor 同款代码索引 | [product/codebase-indexing.md](./product/codebase-indexing.md) |
| 规划 `ops_core` + `ned-runner` 架构 | [product/ops-core-architecture.md](./product/ops-core-architecture.md) |
| 规划 ImGui UI 组件化 / Design System 重构 | [product/ui-architecture.md](./product/ui-architecture.md) |
| Kudu 迁移 Playbook 参考场景 | [product/kudu-migration-scenario.md](./product/kudu-migration-scenario.md) |
| 产品规划总览与实施顺序 | [product/README.md](./product/README.md) |

---

## 分类约定（以后新增文档放哪）

| 子目录 | 放什么 | 不放什么 |
|--------|--------|----------|
| **`development/`** | Git、构建、环境、调试、贡献流程 | 产品愿景、未实现的功能设计 |
| **`product/`** | 需求、架构设计、Playbook 示例、路线图 | 逐步过期的操作命令（应放 development） |

新增文档时：

1. 先判断属于 **开发协作** 还是 **产品规划**
2. 在该子目录下新增 `.md`，并在 **本子目录 README**（若有）和 **本文件索引表** 各加一行链接
3. 文件名用 **kebab-case**，如 `codebase-indexing.md`

---

## 与上游 README 的关系

- 项目通用介绍、编译入门：仓库根目录 [`README.md`](../README.md)
- 本 `docs/` 目录：**qfotcom fork 的定制开发与扩展规划**，上游可能没有这些文件
