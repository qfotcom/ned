# ops_core 架构：Playbook、双 Target、跨平台

## 1. 产品定位

在 NED 基础上扩展 **运维 / 数据迁移 Playbook 平台**：

- **Windows（及 Linux）**：NED GUI 编制 Playbook、校验、Pack、AI 辅助、查看进度  
- **Windows / Linux**：`ned-runner` 无头执行 bundle  
- **生产迁移** 常部署在 Linux 跳板机（离 Kudu 近），但 **core 与 runner 均跨平台**  

与现有 NED 关系：**同一 Git 仓库**，不是两个仓库。

---

## 2. 一套 ops_core，两个 Target

类比现有 CMake 产物：

| 现有 | 规划 |
|------|------|
| `ned` 可执行文件 | `ned` / `ned_ops`（GUI，含编辑器 + AI + Ops 面板） |
| `ned_embed` 静态库 | **`ops_core` 静态库**（Playbook、状态机、DB，无 ImGui） |
| — | **`ned-runner` 可执行文件**（CLI，链接 `ops_core`） |

```
ned/                          # 一个仓库
├── ops/
│   ├── playbook_parser.cpp   ┐
│   ├── runner_engine.cpp     ├── add_library(ops_core STATIC ...)
│   ├── sqlite_store.cpp      │
│   ├── process_spawn.cpp     │  跨平台：Win/Linux 进程启动
│   └── mem_check.cpp         ┘
├── main.cpp                  → target: ned
├── runner_main.cpp           → target: ned-runner
└── CMakeLists.txt
```

**不是两个仓库：** 同一份源码，`git pull` 后在各平台 `cmake --build`。

---

## 3. 编制 → 传输 → 执行

```
Windows: ned.exe
  编辑 playbook.yaml
  Validate / Plan (dry-run)
  Pack → kudu_migration.ops.bundle
  scp / git → Linux

Linux (或 Windows 测试机):
  ned-runner run /opt/bundles/kudu_migration/
  → ops_core 读 Playbook
  → 调 migrate_cli --phase N --group G --json
  → 写 SQLite 进度
```

### Bundle 内容（跨平台文本，非 exe）

```
kudu_migration.ops.bundle/
├── manifest.json       # 版本、min_cli_version
├── playbook.yaml       # 五阶段、门禁、错误码分支
├── skill.yaml          # Agent 要点（可选，引用 playbook）
└── config.schema.json  # 参数校验
```

**敏感信息**（集群地址、账号）不打包进 bundle，使用本机 `config.local.yaml` 或环境变量。

---

## 4. Playbook 运作机制

### 4.1 编制

- 用 **YAML** 描述流程（非 Visio）  
- `playbook validate`：检查阶段依赖、gate、工具名  
- Git 版本管理  

### 4.2 运行（状态机）

对每个 `phase` × `group`：

1. 检查 **gates**（时间窗、内存等）  
2. 调用 **tool**（如 `migrate_cli`）  
3. 解析 **JSON** 输出，写入 **SQLite**  
4. 按 **when** 规则分支（如 20488 → phase 4）  
5. 失败重试 / 等待 / 通知 Agent 或人  

**16:00 门禁、内存上限** 写在 Playbook 的 `gates` 里，不由 LLM 记忆。

### 4.3 Agent 介入（可选）

- LLM **不替代** Runner 调度  
- 用于：异常建议、日报、自然语言查进度  
- **Skill** 仅声明：`playbook: ...`、`tools: [...]`  

---

## 5. 跨平台设计

| 组件 | 跨平台 |
|------|--------|
| `ops_core` 逻辑 | ✅ 必须 |
| `playbook.yaml` / bundle | ✅ 纯文本 |
| `ned-runner` | ✅ Win + Linux 各编译 |
| `ned` GUI | ✅ 已有 |
| 外部 `migrate_cli` | 各平台分别编译；路径由 config 指定 |

### 平台差异封装

| 模块 | Windows | Linux |
|------|---------|-------|
| `process_spawn` | `CreateProcess` | `fork`/`exec` |
| `mem_check` | `GlobalMemoryStatusEx` | `/proc/meminfo` |
| 路径 | `%MIGRATE_CLI%` | `/opt/bin/migrate` |

Playbook 内使用逻辑名与环境变量，不写死 `C:\` 或 `/opt/`。

### Windows 上运行的价值

- 编制、校验、Pack  
- `ned-runner plan` / dry-run  
- 连接测试环境 Kudu  
- 生产 import 仍可只部署 Linux runner（**部署策略**，非代码限制）  

---

## 6. 建议目录与 CMake（规划）

```cmake
# 规划片段，尚未实现
add_library(ops_core STATIC
  ops/playbook_parser.cpp
  ops/runner_engine.cpp
  ops/sqlite_store.cpp
  ops/process_spawn.cpp
  ops/mem_check.cpp
)
target_link_libraries(ops_core PUBLIC yaml-cpp sqlite3)

add_executable(ned-runner runner_main.cpp)
target_link_libraries(ned-runner PRIVATE ops_core)

# ned 现有 target 增加：
target_link_libraries(ned PRIVATE ops_core)
```

依赖建议：

- **yaml-cpp**：Playbook 解析  
- **SQLite**：Run / Step 进度（与 NED 无冲突的轻量方案）  

---

## 7. ned-runner CLI 接口（规划）

```bash
ned-runner validate  ./bundle/          # 校验 Playbook
ned-runner plan      ./bundle/ --group 3  # 将执行的步骤（不跑）
ned-runner run       ./bundle/ [--group N] [--from-phase id]
ned-runner status    [--run-id xxx]       # 读 SQLite / JSON
```

外部工具约定：

```bash
migrate_cli --phase 1 --group 3 --json
# → { "phase", "group", "codes_done", "codes_total", "errors": [...] }
```

---

## 8. NED GUI 扩展（Ops 面板，规划）

| 功能 | 说明 |
|------|------|
| 打开 / 编辑 Playbook | YAML 语法高亮（可加 tree-sitter-yaml） |
| Validate / Pack | 调 `ops_core` API |
| 进度表格 | 读 SQLite 或 runner HTTP |
| AI Agent | 现有面板 + Skill 指向当前 Playbook |
| Terminal | ssh / 本地 dry-run |

---

## 9. MVP 实施顺序

1. `ops_core` + mock CLI（echo JSON）+ SQLite  
2. `ned-runner run` 在 **Windows** 跑通五阶段状态机  
3. NED Ops 面板：Validate + 显示进度  
4. 对接真实 **migrate_cli**  
5. Linux 生产部署 + bundle scp 流程  
6. MCP：`run_phase` / `query_progress`（可选 SSH）  

---

## 10. 与非 NED 方案对比

| | 基于 NED（C++） | Python + Temporal |
|---|-----------------|-------------------|
| 与 migrate_cli | 同生态 | 子进程包 CLI |
| 编制 UI | ImGui + AI 一体 | 多为 Web |
| 首版速度 | 中 | 快 |
| 长期 | 桌面「运维 IDE」 | 偏后台服务 |

当前选择：**基于 NED 延伸**，复用 MCP、Agent、Terminal、跨平台构建。

---

## 11. 相关文档

- GUI 壳层与 Ops 面板的 ImGui 组件化规划：[ui-architecture.md](./ui-architecture.md)
