# 参考场景：Kudu 集群迁移 Playbook

本文档将讨论中的 **Kudu→Kudu 数据迁移** 固化为 Playbook 需求示例，供 `ops_core` 与 `ned-runner` 实现时对照。

---

## 1. 业务背景

- **源**：旧 Kudu 集群  
- **目标**：新 Kudu 集群（表结构与旧集群不完全一致）  
- **范围**：1–10 个分组，每组 N 个证券代码  
- **工具**：自研 C++ `migrate_cli`（名称待定，下文统称 migrate_cli）  

### 五阶段流程

| 阶段 | 说明 | 是否落盘 |
|------|------|----------|
| 1 | 旧集群数据落盘 | 是 |
| 2 | 删除新集群将要导入的代码的全部数据 | — |
| 3 | 导入新集群 | 是 |
| 4 | 处理错误包 20488、28660，重新导入 | 是 |
| 5 | 导入 2026 年最新数据 | **否**（直接导入） |

---

## 2. 运营门禁（Policy）

| 规则 | 说明 |
|------|------|
| **导入窗口** | 每个交易日 **09:00–16:00** 可执行阶段 3、5 |
| **16:00 后** | 有新数据写入，**禁止 import** |
| **落盘窗口** | 非导入窗口优先阶段 1（及不冲突的维护操作） |
| **内存** | 内存占用超阈值（如 48GB）则 **暂停** 当前 group，避免 OOM |
| **时区** | `Asia/Shanghai` |

以上写入 Playbook `gates`，由 `ops_core` 强制执行。

---

## 3. Playbook 草稿（YAML）

```yaml
name: kudu_cluster_migration
version: "1.0"
timezone: Asia/Shanghai

schedule:
  import_window:
    start: "09:00"
    end: "16:00"
  # 仅 phase 3、5 受 import_window 约束

tools:
  migrate: "${MIGRATE_CLI}"
  mem_check: "${MEM_CHECK_CMD:-ned-runner mem-check}"

groups: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

phases:
  - id: dump_old
    phase_num: 1
    description: 旧集群数据落盘
    allowed: outside_import_window   # 16:00 后也可落盘
    tool: migrate
    args: { phase: 1 }

  - id: purge_new
    phase_num: 2
    needs: [dump_old]
    tool: migrate
    args: { phase: 2 }

  - id: import_new
    phase_num: 3
    needs: [purge_new]
    tool: migrate
    args: { phase: 3 }
    gates:
      - time_in_window: import_window
      - mem_check: { max_used_gb: 48 }

  - id: retry_packet_errors
    phase_num: 4
    tool: migrate
    args: { phase: 4 }
    when:
      last_errors: [20488, 28660]
    retry:
      max_attempts: 3

  - id: import_2026_latest
    phase_num: 5
    description: 2026 最新数据，不落盘直接导入
    tool: migrate
    args: { phase: 5, direct: true }
    gates:
      - time_in_window: import_window
      - mem_check: { max_used_gb: 48 }
```

> 实际字段名需与 `migrate_cli` 命令行对齐；实现 `ops_core` 时以此为准做映射。

---

## 4. migrate_cli 接口约定（建议）

Agent 与 Runner **只解析 JSON**，不 parse 人类日志。

```bash
migrate_cli --phase 3 --group 5 --json
```

```json
{
  "phase": 3,
  "group": 5,
  "codes_done": 120,
  "codes_total": 450,
  "status": "running",
  "errors": []
}
```

错误包示例：

```json
{
  "errors": [{ "code": 20488, "message": "...", "symbol": "600000.SH" }]
}
```

全局进度：

```bash
migrate_cli --status --json
ned-runner status --run-id 20260530-001
```

---

## 5. 错误码处理

| 错误码 | Playbook 动作 |
|--------|---------------|
| 20488 | 进入 `retry_packet_errors`（phase 4） |
| 28660 | 同上 |
| 其他 | 记录 SQLite；可选通知 Agent / 人工 |

---

## 6. 进度与人工跟进（现状痛点 → 目标）

| 现状（手工） | 目标（Runner + DB） |
|--------------|---------------------|
| 每天看日志拼命令 | Playbook 自动选 phase |
| 心里记各 group 进度 | `run_steps` 表：`group × phase × status` |
| 盯内存 | 每步 `mem_check` gate |
| 4 点后误 import | `import_window` 硬拦截 |

### SQLite 表（规划）

```
runs(run_id, playbook, started_at, status)
run_steps(run_id, group, phase, status, started, finished, error_codes_json)
progress(run_id, group, codes_done, codes_total)
```

NED Ops 面板读取同一 DB 或通过 `ned-runner status --json` 展示。

---

## 7. Skill 示例（Agent 用，可选）

```yaml
name: kudu_cluster_migration
playbook: playbook.yaml
tools: [migrate, mem_check, progress_status]
notes_for_agent:
  - "16:00 后禁止 phase 3/5，只能 phase 1 落盘"
  - "20488/28660 必须走 phase 4"
  - "查进度用 ned-runner status，不要猜"
  - "不要跳过 purge（phase 2）"
```

Skill **不重复** Playbook 步骤，只帮助 LLM 理解场景。

---

## 8. Windows 编制 / Linux 执行（本场景）

| 步骤 | 位置 |
|------|------|
| 编辑 `playbook.yaml`、分组证券列表 | Windows `ned.exe` |
| `ned-runner validate` / `plan` | Windows 或 Linux |
| Pack `kudu_migration.ops.bundle` | Windows |
| 传输 bundle | scp / Git 制品库 |
| `ned-runner run` + 生产 `migrate_cli` | **Linux 跳板机**（推荐） |
| 进度查看 | Windows NED 读远程 status 或 DB 同步 |

Windows 也可 `ned-runner run` 连接测试集群做联调。

---

## 9. 与代码索引的协作

- **migrate_cli** 源码在仓库内时：代码 Agent 用 `grepProject` / `searchSymbols` 查 phase 实现  
- **运维 Agent** 用 Playbook + `query_progress`，二者共用 NED Agent 面板，**Skill 不同**  

---

## 10. 验收清单（本场景）

- [ ] 16:00 后 runner 拒绝启动 phase 3/5  
- [ ] 内存超 48GB 暂停并写入 status  
- [ ] 10 个 group 可并行或串行（Playbook 配置）  
- [ ] 20488/28660 自动进入 phase 4，最多重试 3 次  
- [ ] Windows 编制 bundle，Linux 执行，进度可在 Windows GUI 查看  
- [ ] 全流程 audit 写入 SQLite（谁、何时、哪步、结果）
