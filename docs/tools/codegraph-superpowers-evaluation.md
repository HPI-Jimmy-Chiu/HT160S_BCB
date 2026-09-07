# CodeGraph 與 Superpowers — 白話評估 + 操作說明書

> 對象：HT160S_BCB 專案（BCB6 / C++Builder 6 工控程式，單人開發、重度使用 Claude Code、大量從 HT172 移植）
> 日期：2026-06-29
> 一句話結論：**CodeGraph 值得馬上試（低風險、對大型舊碼很有用）；Superpowers 借它的觀念、只挑幾個技能裝，不要整套照搬（TDD / worktree 不適合本專案）。**

---

## 0. 先看這張表（懶人包）

| 工具 | 它是什麼（白話） | 對「現在的你」效益 | 風險 | 建議 |
|---|---|---|---|---|
| **CodeGraph** | 把整個程式碼先建成一張「索引地圖」放在本機，AI 不用一個一個檔翻，一次查詢就知道「誰呼叫誰、改這裡會炸到哪」 | ⭐⭐⭐⭐⭐ | 低 | **先試**（pilot），重點用 `impact` 影響分析 |
| **Superpowers** | 一套「開發方法論 + 技能包」外掛，逼 AI 先想清楚→寫計畫→再動手→做完驗證 | ⭐⭐⭐（挑著用） | 中 | **選擇性裝**，關掉/忽略 TDD 與 worktree |

---

## 1. CodeGraph 是什麼？（白話）

GitHub：`colbymchenry/codegraph`（MIT 授權，TypeScript 寫的）

**問題情境**：現在 Claude 要理解你的程式，做法是「一直開檔、一直 grep、一路追路徑」。在 HT160S 這種有 `main.cpp`、`csystem.cpp`、`aLoader.cpp`、`aColor.cpp`、一堆 form 跟全域變數的大專案，這很花時間、也很花 token（= 花錢）。

**CodeGraph 的做法**：先用 tree-sitter 把整個專案解析一遍，把「函式、類別、誰呼叫誰、誰 include 誰」存進一個**本機的 SQLite 資料庫**（`.codegraph/codegraph.db`）。之後 AI 只要查這張地圖，一兩次查詢就拿到答案，不用翻檔。

**對本專案最關鍵的三件事**：

1. **影響分析（impact / blast radius）** — 這是 grep 做不到的。改一個函式之前，先問「誰會呼叫它？改了會影響哪些地方？」。你最痛的就是改 A 壞 B 的回歸（hang up、silent stop、互鎖），這個功能正中要害。
2. **100% 本機、不外傳** — 全部在你電腦上跑，沒有任何資料送雲端。對公司專屬的工控原始碼（honprec）這點很重要。
3. **支援 C++ 與 Pascal/Delphi** — 涵蓋 `.cpp/.h`，BCB6 的 VCL form（`.dfm`）屬於 Pascal 系，部分也能吃。

**官方/文章宣稱的效益**：tool call 次數大幅減少、速度更快、檔案讀取趨近於零（README 寫約「少 58% tool call、快 22%」，其他文章宣稱更高）。實際數字會因專案而異，但方向是對的：**更省、更快、更準**。

### ⚠️ 給本專案的誠實提醒
- **BCB6 解析品質要先驗**：tree-sitter 的 C++ 文法是針對標準 C++，BCB6 有 `__published`、`__fastcall`、`AnsiString`、VCL 巨集這些非標準東西。函式/類別/成員大多解析得出來，但可能有漏。**裝完先跑 `codegraph status` 看抓到的符號數，再決定信任程度。**
- **只在 `D:\HT160S_BCB` 跑 `init`**，不要去 `D:\HT172`（那是唯讀參考，也沒必要建索引）。
- `.codegraph/` 要加進 `.gitignore`（資料庫不進版控）。
- 它新增的是一個 **MCP server**，提供的是「唯讀的程式碼情報」，不碰機台控制，安全。

---

## 2. Superpowers 是什麼？（白話）

GitHub：`obra/superpowers`（作者 Jesse Vincent / Prime Radiant；Claude Code 官方 marketplace 也收錄）

**它不是工具，是一套「工作習慣」外掛**。安裝後會塞進一堆「技能（skill）」，引導 Claude 不要一上來就寫扣，而是照專業流程走：先腦力激盪釐清需求 → 寫計畫 → 照計畫做 → 做完驗證。

**內含的技能（skill）清單**：

| 分類 | 技能 | 白話 |
|---|---|---|
| 協作 | `brainstorming` | 動手前先問清楚、探索其他做法、產出設計文件 |
| 協作 | `writing-plans` / `executing-plans` | 把工作切成 2–5 分鐘的小塊、寫成計畫再照著做 |
| 除錯 | `systematic-debugging` | 4 階段找根因，不靠猜 |
| 驗證 | `verification-before-completion` | 做完一定要拿證據驗證才算完成 |
| 測試 | `test-driven-development` | 強制 RED→GREEN→REFACTOR（先寫失敗測試再寫扣） |
| 協作 | `requesting/receiving-code-review` | 任務間自動 code review，嚴重問題擋下不准過 |
| 並行 | `dispatching-parallel-agents` / `subagent-driven-development` | 派多個子代理平行做、兩段式審查 |
| 並行 | `using-git-worktrees` / `finishing-a-development-branch` | 用 git worktree 隔離平行工作、收尾合併 |
| 後設 | `writing-skills` | 教 AI 怎麼寫新的 skill |

**安裝後技能會自動觸發**，你只要描述要做什麼，它就會把 Claude 拉進正確流程。

### ⚠️ 給本專案的誠實提醒（重要）
你**已經自己建了一套很完整的治理系統**（`.github/instructions`、十幾個 domain skill、agents、hooks、70+ 條 memory），Superpowers 是「通用版」，整套照搬會跟你的專案硬規則打架：

- **`test-driven-development` 不適合 ❌** — BCB6 工控程式沒有單元測試框架，驗證是「上機台跑」。這個 skill 會強迫「沒測試就刪扣重來」，直接跟你的 build-gate + 上機驗證流程衝突。**務必忽略/關掉。**
- **`using-git-worktrees` / 平行子代理 風險高 ⚠️** — BCB6 是單機編譯、有 `.obj` 產物、還有「designer 存檔會吃掉元件」的陷阱（見 memory `bcb-designer-save-strips-components`）。多個 worktree 平行改 `.dfm/.h` 很危險。本專案維持「一次一條線」比較安全。
- **跟既有 skill 重疊** — 你已經有 `/code-review`、`docs/plan/` 計畫流程、State Record 除錯慣例。Superpowers 的對應技能是補強，不是取代。

**結論**：別把它當「裝了就照單全收」的東西，而是當成**一份高品質的觀念清單**。真正值得吸收的是 4 個：`brainstorming`、`writing-plans`、`systematic-debugging`、`verification-before-completion`——而且這些觀念其實可以直接寫進你自己的 `.github/skills`，不一定要裝整套外掛。

---

## 3. 哪些功能對「現在的你」效益最大？（排序）

| 排名 | 功能 | 來自 | 為什麼對你有用 |
|---|---|---|---|
| 🥇 1 | **影響分析 `codegraph impact <符號>`** | CodeGraph | 移植/改函式前先看「誰呼叫它、會炸到哪」。直接降低你最痛的回歸風險（hang up、互鎖、silent stop） |
| 🥈 2 | **快速理解架構 `codegraph explore`** | CodeGraph | 大檔（main/csystem/aLoader/aColor）一次拿到入口點＋相關符號＋片段，省 token 省時間 |
| 🥉 3 | **系統化除錯 `systematic-debugging`** | Superpowers | 完美貼合你「hang up → 分析 State Record」的流程，逼出根因不靠猜 |
| 4 | **動手前腦力激盪 `brainstorming`** | Superpowers | 移植任務常需要先釐清需求/取捨，先想清楚再寫 |
| 5 | **寫計畫/照計畫 `writing-plans`** | Superpowers | 把你已經在做的 `docs/plan/` 流程正規化 |
| 6 | **完成前驗證 `verification-before-completion`** | Superpowers | 強化你的「build EXIT 0 + 上機驗證」紀律 |
| 7 | **省 token / 省錢** | CodeGraph | 你重度使用 Claude，少翻檔 = 少花錢 |

**不建議用**（對本專案）：CodeGraph 的 `.dfm` 純表單描述索引（價值低）、Superpowers 的 `test-driven-development`、`using-git-worktrees`、平行子代理。

---

## 4. 白話操作說明書

### 4-A. CodeGraph 安裝與使用（Windows）

**步驟 1：裝 CLI**（PowerShell，任選一種）
```powershell
# 方法 A：官方安裝腳本
irm https://raw.githubusercontent.com/colbymchenry/codegraph/main/install.ps1 | iex

# 方法 B：用 npm（若已有 Node）
npm i -g @colbymchenry/codegraph
```
> CLI / MCP server 是自帶 runtime 的，本身**不需要先裝 Node**（方法 B 的 npm 才需要）。

**步驟 2：把 MCP 接進 Claude Code**
```powershell
codegraph install
```
> 這會自動偵測 Claude Code、把 MCP server 設定寫進去，並加上使用指引。

**步驟 3：在本專案建索引**（一定要在對的資料夾）
```powershell
cd D:\HT160S_BCB
codegraph init
codegraph status      # 看抓到多少符號 → 判斷 BCB6 解析品質
```

**步驟 4：加進 .gitignore**
```
.codegraph/
```

**常用指令速查**
| 指令 | 用途 |
|---|---|
| `codegraph explore <問題>` | 拿到相關符號＋呼叫流程＋片段 |
| `codegraph query <名稱>` | 用名字搜尋符號 |
| `codegraph impact <符號>` | ⭐ 影響分析：誰呼叫、改了影響哪 |
| `codegraph affected [檔案]` | 找受影響的測試/檔案 |
| `codegraph status` | 看索引統計 |
| `codegraph sync` | 手動增量更新（平常會自動偵測檔案變更同步） |
| `codegraph upgrade` | 原地升級 |

**裝完後怎麼用**：你不用每次下指令，Claude（透過 MCP）會自己去查這張地圖。你只要在請我做事時，正常描述任務即可——我會優先用它做影響分析再動手。

---

### 4-B. Superpowers 安裝與使用

**步驟 1：在 Claude Code 裡裝外掛**
```
/plugin install superpowers@claude-plugins-official
```
> 或用作者的 marketplace：
> ```
> /plugin marketplace add obra/superpowers-marketplace
> /plugin install superpowers@superpowers-marketplace
> ```

**步驟 2：（重要）先決定要不要全開**
裝完技能會「自動觸發」。對本專案建議的用法：
- ✅ 想用 → 直接描述任務，讓 `brainstorming` / `writing-plans` / `systematic-debugging` / `verification-before-completion` 自然介入。
- ❌ 不想被 TDD 綁架 → 明確告訴我「這專案不做 TDD、不用 worktree、照 HT160S build-gate 驗證」，或乾脆**不裝外掛，改把這幾個觀念寫進你自己的 `.github/skills`**（推薦，最不會跟既有規則打架）。

**替代方案（最適合你）**：不裝外掛，請我把 Superpowers 的 `systematic-debugging`、`brainstorming`、`writing-plans` 三個流程，**改寫成符合 HT160S 規則的版本**放進 `.github/skills/`。好處：保留觀念精華、完全相容你的 Big5 / build-gate / 上機驗證 / write-boundary。

---

## 5. 給本專案的最終建議

1. **CodeGraph：先 pilot 一週。** 裝好後跑 `codegraph status` 驗證 BCB6 解析品質；若符號抓得夠多，就把「改碼前先 impact」變成習慣。風險低、可隨時移除。
2. **Superpowers：不要整套裝。** 真正的價值是 4 個觀念（brainstorm / plan / 系統化除錯 / 完成前驗證）。最佳做法是把這幾個觀念**內化進你自己的 `.github/skills`**，而不是引入一個會推 TDD/worktree、跟你硬規則衝突的通用外掛。
3. **兩者都不碰機台控制、都可逆**——CodeGraph 是唯讀情報、Superpowers 是流程引導。可以放心試，不滿意就移除。

---

### 參考來源
- CodeGraph — https://github.com/colbymchenry/codegraph
- CodeGraph 官網 — https://colbymchenry.github.io/codegraph/
- Superpowers — https://github.com/obra/superpowers
- Superpowers marketplace — https://github.com/obra/superpowers-marketplace
- 作者文章（Superpowers 設計理念）— https://blog.fsck.com/2025/10/09/superpowers/
