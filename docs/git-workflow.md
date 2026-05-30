# NED 本地定制开发 & 上游合并指南

本文档说明如何在 fork 仓库中持续开发自己的改动，并在原作者（[nealmick/ned](https://github.com/nealmick/ned)）更新时合并上游代码。

---

## 仓库结构

| 远程 | 地址 | 用途 |
|------|------|------|
| `origin` | `https://github.com/qfotcom/ned.git` | 你的 fork，**push 到这里** |
| `upstream` | `https://github.com/nealmick/ned.git` | 原作者仓库，**只 fetch，用来合并更新** |

| 分支 | 说明 |
|------|------|
| `main` | 尽量与 `upstream/main` 保持一致，用于对比或测试纯上游版本 |
| `dev` | 你的定制开发分支，所有本地改动都在这里 |

```
upstream/main  ──fetch──►  本地 main（可选）
                              │
                              └── merge ──►  dev（你的定制分支）
                                                │
                                                └── push ──►  origin/dev（你的 fork）
```

---

## 前置步骤（已完成）

若你尚未完成，请先执行：

1. 在 GitHub 上 Fork `nealmick/ned`（建议勾选 **Copy the main branch only**）
2. 配置远程、创建 `dev` 分支并提交首次改动：

```powershell
cd D:\Win.d\Github\imgui-Agent\ned

git remote add upstream https://github.com/nealmick/ned.git
git remote set-url origin https://github.com/qfotcom/ned.git

git switch -c dev
git add .
git commit -m "Windows 本地化与 UI 改进：UTF-8、CJK 字体、滚动条等"
git push -u origin dev
```

---

## 第三步：日常开发

在 `dev` 分支上改代码、提交、推送到你的 fork：

```powershell
cd D:\Win.d\Github\imgui-Agent\ned

# 切换到开发分支
git switch dev

# ... 修改代码 ...

# 提交改动
git add .
git commit -m "描述你的改动"

# 推送到你的 fork
git push origin dev
```

### 提交建议

尽量**按主题拆分多个 commit**，例如：

- `Windows 编译与 vcpkg 路径`
- `合并 CJK 字体支持`
- `编辑区与文件树滚动条`
- `修复最小化时 framebuffer 报错`

拆得越细，以后与上游合并、解决冲突时越容易定位和处理。

### 常用命令

```powershell
# 查看当前状态
git status

# 查看未提交的改动
git diff

# 查看提交历史
git log --oneline -10

# 查看与上游的差异
git fetch upstream
git log dev..upstream/main --oneline   # 上游有哪些新提交
git log upstream/main..dev --oneline   # 你有哪些独有提交
```

---

## 原作者更新时怎么合并

当 [nealmick/ned](https://github.com/nealmick/ned) 有新提交时：

```powershell
cd D:\Win.d\Github\imgui-Agent\ned

# 1. 拉取上游最新代码
git fetch upstream

# 2. 在你的开发分支上合并（推荐 merge）
git switch dev
git merge upstream/main

# 若有冲突：
#   - 打开冲突文件，手动解决
#   - git add .
#   - git commit

# 3. 子模块可能也变了，需要同步
git submodule update --init --recursive

# 4. 合并后建议清空 build 重配（CMake 路径/选项可能变化）
#    删除 build 目录后重新运行 build-win.bat

# 5. 推送到你的 fork
git push origin dev
```

### 合并后编译

```powershell
# 若编译异常，清空 build 后重配
Remove-Item -Recurse -Force build
.\build-win.bat
```

---

## Merge vs Rebase

| 方式 | 命令 | 特点 | 适用场景 |
|------|------|------|----------|
| **Merge** | `git merge upstream/main` | 保留完整历史，操作简单 | **推荐**：个人 fork + 长期定制 |
| **Rebase** | `git rebase upstream/main` | 历史更线性、更整洁 | 单人分支、且尚未 push 共享时 |

对你这种「个人 fork + 长期定制」场景，**优先使用 merge**，更省心。

若使用 rebase：

```powershell
git fetch upstream
git switch dev
git rebase upstream/main
# 解决冲突后：git add . && git rebase --continue
git push origin dev   # 若之前已 push 过 dev，可能需要 --force-with-lease（谨慎使用）
```

> **注意**：已经 push 给别人协作的分支，不要随意 rebase 或 force push。

---

## 注意事项

1. **不要在 `main` 上直接开发** — 保持 `main` 与 upstream 对齐，改动都放在 `dev`。
2. **不要 push 到 `upstream`** — 你没有原作者仓库的写权限；若要贡献代码，从你的 fork 提 Pull Request。
3. **`ned.aps` 等 VS 临时文件** — 不要提交，可加入 `.gitignore`。
4. **`build/` 目录** — 一般不纳入版本控制；合并上游后若编译失败，删 `build/` 重配。
5. **子模块** — 每次 fetch/merge 上游后，执行 `git submodule update --init --recursive`。
6. **向原作者贡献** — 若某改动希望被上游采纳，从 fork 的 `dev`（或单独 feature 分支）向 `nealmick/ned` 提 PR。

---

## 可选：同步本地 main

若希望本地 `main` 始终跟踪上游（不混入你的定制改动）：

```powershell
git fetch upstream
git switch main
git merge upstream/main
git push origin main   # 可选：让 fork 的 main 也与上游同步
git switch dev
```

---

## 快速参考

```powershell
# 日常：改代码 → 提交 → 推送
git switch dev && git add . && git commit -m "..." && git push origin dev

# 上游有更新：拉取 → 合并 → 同步子模块 → 推送
git fetch upstream && git switch dev && git merge upstream/main && git submodule update --init --recursive && git push origin dev
```
