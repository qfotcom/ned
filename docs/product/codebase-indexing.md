# Cursor 同款代码索引系统（NED 扩展需求）

## 1. 目标

让 NED 右侧 AI Agent 能像 Cursor `@Codebase` 一样：

- 快速理解项目结构，而非仅依赖「当前打开文件 + 目录名」  
- 支持 **关键词 grep** 与（可选）**语义检索**  
- 减少 Agent 盲目 `listFiles` / `readFile` 轮询  

---

## 2. NED 现状

| 能力 | 现状 | 路径/说明 |
|------|------|-----------|
| Agent 上下文 | system prompt 含 `selectedFolder`、`currentOpenFile` | `ai/ai_agent.cpp` |
| MCP 工具 | `listFiles` / `readFile` / `editFile` / `executeCommand` | `ai/mcp/mcp_manager.cpp` |
| 文件名列表 | 后台递归扫描 | `files/file_finder.cpp` |
| 单文件搜索 | Cmd+F | `files/file_content_search.cpp` |
| 语法/符号 | Tree-sitter 高亮、LSP 跳转 | `editor/`、`lsp/` |
| **全项目 grep 工具** | ❌ 无 | — |
| **语义向量索引** | ❌ 无 | — |

Agent 改代码路径：对话模型 → `editFile` → **Morph** → 写磁盘 → `FileMonitor` 重载编辑器。

---

## 3. Cursor 索引原理（对标）

```
打开项目 → 分块（函数/类/~500 tokens）
         → Embedding → 向量库（仅向量+元数据）
         → 查询时 embed 问题 → 最近邻 → 本地读代码块 → 注入 LLM
```

辅以 **grep** 做精确字符串匹配。Merkle 树用于增量更新变更文件。

---

## 4. NED 目标架构

```
用户消息 / Agent 工具调用
        ↓
ProjectIndex（新建 util/project_index/ 或 index/）
├── FileCatalog      ← 复用 FileFinder 扫描逻辑
├── GrepIndex        ← 阶段 1：ripgrep 或自研递归搜索
├── SymbolIndex      ← 阶段 2：Tree-sitter 抽函数/类/行号
└── SemanticIndex    ← 阶段 4：embedding + 本地向量库（可选）
        ↓
MCP: searchCode / searchSymbols / searchCodebase
        ↓
ai_agent.cpp：可选 sendMessage 前自动 retrieve(top_k) 注入 system prompt
```

---

## 5. 分阶段落地

### 阶段 1：MCP `grepProject`（优先，1–2 天量级）

- 参数：`query`、`path`、`glob`、`max_results`  
- 尊重 `.gitignore`  
- 返回：`file:line:snippet`  
- 注册于 `mcp_manager.cpp`  

**效果：** Agent 可主动搜 `ensureCursorVisible`、`editFile`，接近 Cursor grep。

### 阶段 2：符号索引

- 打开项目 / 保存文件时，Tree-sitter 解析源文件  
- 索引：`symbol → (file, line, kind)`  
- MCP：`searchSymbols(name)`  

NED 已有 Tree-sitter grammar 子模块，复用成本低。

### 阶段 3：增量与持久化

- 文件 content hash（类似 `FileMonitor`）  
- 仅重索引变更文件  
- SQLite 存 catalog / symbols  

### 阶段 4：语义检索（可选）

- 分块 → OpenRouter embedding API 或本地模型  
- 本地：`sqlite-vec` / hnswlib  
- MCP：`searchCodebase(query, top_k)`  
- **源码不出本机**，仅向量可上云（按合规选择）  

---

## 6. MCP 工具建议清单

| 工具名 | 阶段 | 说明 |
|--------|------|------|
| `grepProject` | 1 | 全项目文本搜索 |
| `searchSymbols` | 2 | 符号定义搜索 |
| `searchCodebase` | 4 | 语义相似块 |
| `readFile` | 已有 | 读全文/片段 |
| `editFile` | 已有 | Morph 合并写盘 |

---

## 7. Agent 使用方式

**方式 A — 工具调用（Cursor Agent 模式）**  
模型自行决定何时 `grepProject` / `searchSymbols`。

**方式 B — 自动注入（Cursor @Codebase 模式）**  
`sendMessage` 前：

```cpp
auto chunks = gProjectIndex.retrieve(userMsg, top_k=8);
systemPrompt += formatChunks(chunks);
```

建议 **A + B 同时提供**。

---

## 8. 与 ops_core 的边界

| | 代码索引 | ops_core |
|---|----------|----------|
| 对象 | 源码仓库 | Playbook / 运维 CLI |
| 消费者 | 代码 Agent | ned-runner |
| 存储 | 项目目录下 `.ned/index/` | Run 进度 SQLite |

二者共用 **MCP 注册机制** 与 **Agent UI**，索引实现独立模块。

---

## 9. 验收标准（MVP）

- [ ] Agent 问「ensureCursorVisible 在哪」能 via grep 定位到 `editor_scroll.cpp`  
- [ ] 索引更新：保存文件后 5s 内 grep 结果一致  
- [ ] 不破坏现有 `readFile` / `editFile` 流程  
- [ ] Windows 与 Linux 均可构建（路径、`gitignore` 处理 UTF-8）
