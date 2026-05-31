# 通用 Agent 平台愿景与 Cursor 对比

## 1. 为什么编程 Agent（Cursor）特别「好用」

编程 Agent 在结构上比其他领域更容易做出「变革性」体验，因为同时满足：

| 条件 | 编程场景 |
|------|----------|
| 上下文可索引 | 文件树、符号、git、LSP |
| 可执行 | 改文件、跑命令、跑测试 |
| 可快速验证 | 编译、测试、diff |
| 用户是专家 | 程序员能判断 AI 改得对不对 |
| 错误可回滚 | Git、Undo |

**Cursor 的核心分工：**

- **强模型（规划）**：理解需求、读代码、生成修改 sketch  
- **专用 Apply 模型（落地）**：Fast Apply / Morph 把片段合并进完整文件  
- **索引 + grep**：全项目语义/关键词检索，自动注入 context  

NED 现状：Agent + MCP（`readFile` / `editFile` + Morph）+ Tab 补全；**缺索引与项目级 grep**，体感弱于 Cursor。

---

## 2. 其他领域的「通用 Agent」形态

适合做成「Cursor 级」能力的领域（结构化输入 + 可执行 + 可验证）：

| 类型 | 示例 | 成熟度 |
|------|------|--------|
| 诊断型 | 日志 / 指标 / trace 分析 + 图表 | 中（Datadog Bits 等） |
| 查询型 | 自然语言 → SQL → 表/图 | 中高 |
| 检索型 | 组织知识库 + 与真实系统交叉验证 | 中 |
| 执行型 | 跨 Jira/GitHub/Slack 工作流 | 中 |
| 审查型 | diff / 合规 / 合同标注 | 中 |

**未来形态（判断）：** 不是「一个 AGI 助理」，而是 **多个 Worker + 统一编排层**：

```
Observe → Reason → Propose → (Human) Act → Verify
```

- 编程：**Cursor** = 执行型 + 检索型（代码域）  
- 运维：**日志/指标 Agent** = 诊断型 + 查询型  

---

## 3. Skill vs Playbook：选哪个？

| | **Playbook** | **Skill** |
|---|--------------|-----------|
| 给谁用 | 系统 / Workflow / 人 | LLM Agent |
| 内容 | 阶段、门禁、重试、工具调用 | 场景说明、工具清单、错误码、汇报要点 |
| 是否可执行 | **是**（绑 Runner） | 单独写则**虚**；必须引用 Playbook + Tools |
| 类比 | SRE Runbook、Ansible、Temporal Workflow | Agent「岗位培训包」 |

**结论：先 Playbook，后 Skill。Skill 指向 Playbook，不重复写步骤。**

```
Playbook（唯一真相） → Workflow 引擎执行
                    ↘ Skill（Agent 加载入口）
```

---

## 4. 分层架构（NED 代码 Agent + 运维 Agent 共用）

```
Digital Twin 层（远期）
  偏好、记忆、审批、对话 / 通知
        ↓
Agent Runtime（薄）
  LangGraph 或自研 loop；异常、日报、建议
        ↓
┌─────────────┬─────────────┬─────────────┐
│ Skill       │ Playbook    │ Policy      │
│ 场景知识     │ 流程定义     │ 时间/内存/权限 │
└──────┬──────┴──────┬──────┴─────────────┘
       ↓             ↓
   MCP Tools    ops_core Runner
       ↓             ↓
   代码库 / CLI   migrate_cli / 监控 API
```

- **确定性部分**（4 点 cutoff、五阶段、内存上限）→ Playbook + `ops_core`  
- **LLM** → 异常分支、拼命令建议、进度解释、数字分身对话  

---

## 5. 数字分身演进路线

| 阶段 | 能力 |
|------|------|
| A | Playbook + Runner 自动跑，零 LLM |
| B | Agent 读日志/DB，异常建议 + 日报，人 Approve |
| C | 多场景 Skill 库，同一 Agent Runtime |
| D | 持久记忆 + 偏好 + IM/语音入口 + 审计 |

「分身」= **Platform + Memory + 你的历史决策**，不是单个 Skill 文件名。

---

## 6. 与 NED 现有 AI 的关系

| 模块 | 路径 | 作用 |
|------|------|------|
| AI Agent 面板 | `ai/ai_agent.cpp` | OpenRouter 对话 + tool_calls |
| MCP | `ai/mcp/` | `readFile` / `editFile` / `executeCommand` 等 |
| Morph Fast Apply | `ai/mcp/mcp_file_system.cpp` | `editFile` 合并代码 |
| Tab 补全 | `ai/ai_tab.cpp` | 编辑器内 ghost text |

扩展方向：

- **代码域**：索引 + `grepProject` MCP + 可选自动 context 注入  
- **运维域**：`ops_core` + Playbook bundle + `ned-runner`  

两套场景 **共用 MCP 与 Agent UI 壳**，不各写一套 Agent 服务。
