# CodeGraph + 移植版 Superpowers 技能 — 安裝後操作手冊（白話詳細版）

> 日期：2026-06-29　對象：HT160S_BCB + 你未來的其他專案
> 這份是「已經幫你裝好之後，要怎麼用」的說明。評估與取捨請看同資料夾的 `codegraph-superpowers-evaluation.md`。

---

## 0. 這次到底幫你裝了什麼？（現況）

| 項目 | 狀態 | 位置 | 範圍 |
|---|---|---|---|
| CodeGraph CLI（`codegraph` 指令） | ✅ 已裝 v1.1.2 | 全機（npm 全域） | 所有專案都能用這個指令 |
| CodeGraph MCP 接線 | ✅ 已寫入 | `D:\HT160S_BCB\.mcp.json` | **只有 160**（其他專案要各自加，見第 5 節） |
| CodeGraph 索引（程式碼地圖） | ✅ 已建好 | `D:\HT160S_BCB\.codegraph\` | **只有 160**（每個專案各自 `init`） |
| `.gitignore` 排除 `.codegraph/` | ✅ 已加 | `D:\HT160S_BCB\.gitignore` | 160 |
| 移植版 Superpowers 技能 ×3 | ✅ 已建 | `C:\Users\jimmychiu\.claude\skills\` | **全機所有專案通用** |

**160 索引實測**：267 個檔、11,455 個符號、24,977 條關係；C++ 210 檔、.dfm 表單 33 檔都正確解析（抽出 4,596 methods / 909 functions / 244 classes）。**結論：CodeGraph 對 BCB6 解析品質良好，可放心用。**

### ⚠️ 兩件「要重開 Claude Code 才生效」的事
1. **CodeGraph 的 MCP 工具** —— Claude Code 在啟動時才載入 MCP。重開後會問你是否信任 `.mcp.json` 的 codegraph server，**按同意**即可。
2. **3 個全域技能** —— 同樣重開後才會被 Claude Code 發現。

> 在重開之前也不是不能用：`codegraph` 指令在終端機**現在就能用**，我（Claude）也能直接在終端機幫你跑（下面第 2 節就是這樣示範的）。

---

## 1. CodeGraph 是做什麼的（一句話複習）
把整個專案先建成「誰呼叫誰」的地圖存在本機。**改程式前先問它「動這裡會影響哪些地方」**，不用自己一個一個檔翻。100% 本機、不外傳。

---

## 2. 在 160 怎麼用 CodeGraph（含真實範例）

### 用法 A：你自己在終端機下指令（PowerShell）
所有指令都先 `cd D:\HT160S_BCB`（或加 `--path "D:\HT160S_BCB"`）。

**最常用：改某個函式前，先看會波及哪裡**
```powershell
codegraph impact DoFeedTray
```
這是我剛剛實際在 160 跑出來的結果（節錄）：
```
Impact of changing "DoFeedTray" — 13 affected symbols:
  aAuto1To6.cpp   DoFeedTray:444   DoAuto:1288
  database.cpp    actAuto1to6Execute / actColorExecute / actEmptyExecute / actLoader1Execute / actLoader2Execute
  aColor.cpp      DoFeedTray:698   DoColor:240
  aEmpty.cpp      DoFeedTray:317   DoEmpty:197
  aLoader.cpp     DoFeedTray:1050  DoLoader:909
```
👉 白話：它馬上告訴你「`DoFeedTray` 在 Color/Empty/Loader/Auto 四個模組各有一份，而且改了會牽動 `database.cpp` 裡的 5 個 `act*Execute` 派發器」。這正是你最怕的「改一個壞一片」回歸——**動手前就先看到。**

**其他常用指令**
| 想做的事 | 指令 | 白話 |
|---|---|---|
| 找一個符號在哪 | `codegraph query DoFeedTray` | 用名字搜尋，列出所有定義位置 |
| 看誰呼叫它 | `codegraph callers MoveColorY` | 往上游：哪些函式會叫它 |
| 看它呼叫誰 | `codegraph callees DoColor` | 往下游：它內部又叫了哪些 |
| 改某函式的影響面 | `codegraph impact <符號>` | ⭐ blast radius，最常用 |
| 一次看懂一塊區域 | `codegraph explore "tray feed flow"` | 給一句話，回傳相關符號＋原始碼＋呼叫路徑 |
| 看單一符號的來龍去脈 | `codegraph node DoFeedTray` | 該符號原始碼 + 上下游呼叫鏈 |
| 看索引統計 | `codegraph status` | 檔數/符號數/是否最新 |
| 看檔案結構 | `codegraph files` | 從索引列出專案檔案樹 |

### 用法 B：讓 Claude（我）自動用（重開 Claude Code 後）
重開並同意 MCP 後，你**不用記任何指令**。你正常叫我做事，例如：
- 「我要改 `DoFeedTray`，先幫我看會影響哪些地方再動手」
- 「`DoColor` 的供料流程幫我理一下」

我會自動透過 codegraph MCP 查地圖，而不是一個一個開檔——更快、更省 token、更不會漏掉相依。

### 索引會自動更新嗎？
會。CodeGraph 有背景監看，你（或我）改檔後會自動增量同步。若覺得不同步，手動補一刀：
```powershell
codegraph sync       # 增量更新
codegraph index      # 整個重建（大改/換分支後保險用）
```

---

## 3. 3 個全域技能怎麼用（移植自 Superpowers）

這 3 個是「工作習慣」技能，放在 `C:\Users\jimmychiu\.claude\skills\`，**所有專案都通用**（不只 160）。重開 Claude Code 後生效。它們是通用版，且都寫了「遵守該專案 CLAUDE.md 的建置/驗證規則」，所以**不會跟 160 的 build-gate / 上機驗證 / Big5 打架**。

| 技能 | 什麼時候會自動出動 | 它逼我做什麼 |
|---|---|---|
| `systematic-debugging` | 你說「當機 / hang / 壞掉 / 變慢 / 之前好好的」 | 4 階段找根因（重現→縮小→根因→驗證），先讀 log/State Record，不准用「猜了就補個 try/catch」 |
| `brainstorming` | 要做「比較大或需求不清」的功能前 | 先用一句話複述目標、問關鍵問題、給 2–3 個做法＋建議、列出設計再動手 |
| `writing-plans` | 多檔、跨 session 的大工程（移植、重構） | 切成小步、每步寫清楚「怎麼驗」、存成計畫文件、一步一步勾完 |

**怎麼觸發？** 不用特別指令，描述任務時自然會帶出來。也可以明講：「用 systematic-debugging 的方式查這個 hang」。

> 為什麼不直接裝 Superpowers 外掛？因為它整套含 TDD 和 git-worktree，會在 160 強迫「沒測試就刪扣」「開多個工作目錄平行改」，跟你的工控 build-gate、上機驗證、designer 存檔陷阱衝突。所以只挑了 3 個安全又通用的觀念移植成你的個人技能。細節見 evaluation 那份。

---

## 4. 維護 / 移除（隨時可逆）

```powershell
codegraph upgrade            # 升級 CodeGraph 到最新版
codegraph uninit "D:\HT160S_BCB"   # 只刪 160 的索引（.codegraph/）
codegraph uninstall          # 從 Claude Code 移除 MCP 接線
npm uninstall -g @colbymchenry/codegraph   # 整個移除 CLI
```
移除 3 個全域技能：直接刪 `C:\Users\jimmychiu\.claude\skills\` 底下那 3 個資料夾即可。

---

## 5. 🚀 未來在「其他專案」怎麼用（重點）

記住一句話：**CLI 與全域技能裝一次到處有效；但 CodeGraph 的「索引」是每個專案各自一份。**

### 5-1. 任何新專案，啟用 CodeGraph 只要 2 步
```powershell
cd D:\你的新專案
codegraph init .            # 建立該專案的索引（各自獨立，互不干擾）
```
然後在該專案根目錄放一個 `.mcp.json`（內容跟 160 一模一樣）：
```json
{
  "mcpServers": {
    "codegraph": {
      "type": "stdio",
      "command": "codegraph",
      "args": ["serve", "--mcp"]
    }
  }
}
```
重開 Claude Code、同意一次，就能用。記得把 `.codegraph/` 加進那專案的 `.gitignore`。

### 5-2. 想「一次設定、全部專案都自動有 MCP」？（建議）
不想每個專案都放 `.mcp.json`，可以把 MCP 接成**全機全域**。這次因為 Claude Code 正在執行、鎖住設定檔而失敗，所以改用了專案層級。要全域，請在**完全關閉 Claude Code 後**，開一個一般 PowerShell 視窗跑：
```powershell
codegraph install --target claude --location global --yes
```
成功後，之後任何專案只要 `codegraph init`（建該專案索引）就能用 MCP，不必再放 `.mcp.json`。
> 驗證是否接好：`codegraph install --print-config claude` 會印出它要寫進 `C:\Users\jimmychiu\.claude.json` 的設定。

### 5-3. 3 個全域技能：什麼都不用做
它們已經在 `~/.claude/skills/`，**對你開的每一個專案都自動生效**，不必複製、不必重裝。

---

## 6. 一頁速查（貼牆上）

```
# 改程式前先看影響
codegraph impact <函式名>

# 理解一塊流程
codegraph explore "你想懂的東西，一句話"

# 找符號 / 看上下游
codegraph query <名字>
codegraph callers <名字>      # 誰呼叫它
codegraph callees <名字>      # 它呼叫誰

# 維護
codegraph status              # 看索引狀態
codegraph sync                # 手動同步
codegraph upgrade             # 升級

# 新專案啟用（2 步）
cd <專案>; codegraph init .   # + 放 .mcp.json + 加 .gitignore .codegraph/
```

> 別忘了：**改完 CodeGraph 設定或新增技能後，要重開 Claude Code 才生效。**
