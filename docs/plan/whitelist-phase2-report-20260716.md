# WhiteList 分選模式 — Phase 2 實作完成報告

| 項目 | 內容 |
|---|---|
| 日期 | 2026-07-16 |
| 分支 | `feat/iosetview-172-refactor` |
| Commit | `4466cac` — `feat(secs): WhiteList Phase 2 - host SORTMODE switch + trace-1005 + SVID 66032` |
| 前置 | Phase 1 核心 `a46b7e9`（2026-07-15，WebAPI-substitution 模型） |
| 計畫 SSOT | `docs/plan/whitelist-sortmode-plan-20260714.md` §6 / §7 |
| 建置 | sim `-Full` EXIT 0；真機 `SOFT_SIMULATE` off `-Full` EXIT 0（define 已還原）；編碼檢查 163 檔通過 |
| 審查 | 5 視角對抗式複驗 + 獨立 skeptic 驗證：**0 confirmed 缺陷**（2 NIT 皆 refuted） |
| 狀態 | Code-complete、compile-clean；**上機 host round-trip 驗證 pending** |

---

## 0. 一句話總結

Phase 2 讓 **SECS 主機可在 `S2F41 LOTSTART` 電文尾端夾帶一個可選的 `SORTMODE` 選項對來切換分選模式**（NORMAL / WHITELIST），並補齊 Production_Log 的 `NotWhitelisted`（trace 1005）可追溯性、以及 host 回讀模式的 SVID 66032。全部改動為 shared core（無 `#ifdef SOFT_SIMULATE`），sim 與真機組態皆建置乾淨。

---

## 1. 本次交付範圍

### ✅ 已完成（本次核准範圍）

| # | 項目 | 檔案 |
|---|---|---|
| 1 | SECS `LOTSTART` 可選 `SORTMODE` pair 解析（Phase 2 主體）+ 4 個審查修正 | `SecsGem/uHGemHT160.cpp` |
| 2 | SVID 66032 host 回讀目前 SortMode（Q6） | `SecsGem/uHGemHT160.cpp` |
| 3 | Production_Log trace 1005 `NotWhitelisted`（**兩處** trace 站點） | `aSortArm.cpp` + `deviceinfo.cpp` |
| 4 | maintenance UI 選擇器同步（避免 stale 頁存檔覆寫 host 切換） | `maintenance.h` + `maintenance.cpp` |

### ⏳ 刻意延後（非本次核准範圍）

- **StateRecord JSON 具名 SortMode 4-way**：`cStateRecordHT160.cpp` 的 3-way 三元就緊鄰**目前未提交的 `SR_Trace` AV-hunt 除錯碼**（標記 "REMOVE after fix"）。為避免把該暫時除錯碼一起 commit，本次不動此檔。
  - **無功能缺口**：host 已可用新 SVID 66032 讀模式；快照也已 dump 數值 `iSortMode`（可交叉核對）。僅 JSON 字串標籤在 WhiteList 模式仍顯示 `Normal`。
  - **1 行 follow-up**：待除錯碼清掉/提交後，把三元改成 4-way 即可。

---

## 2. SECS 協定規格（給 KYEC EAP）

### 2.1 電文格式

在既有 `S2F41 LOTSTART` 內層 lot 清單中，允許夾帶**至多一個**巢狀 `L[2]{ A"SORTMODE", A值 }` 選項對；值域 `NORMAL` | `WHITELIST`（大小寫不敏感，內部 `Trim().UpperCase()`）。

**切為 WhiteList 模式（同時開一個 lot）**
```
S2F41 W
<L [2]
  <A "LOTSTART">
  <L [2]
    <A "LOT001">
    <L [2] <A "SORTMODE"> <A "WHITELIST">>
  >
>
```

**切回一般模式**
```
S2F41 W
<L [2]
  <A "LOTSTART">
  <L [2]
    <A "LOT002">
    <L [2] <A "SORTMODE"> <A "NORMAL">>
  >
>
```

**不帶 pair（sticky，維持目前模式；與現行電文位元組相容）**
```
S2F41 W
<L [2]
  <A "LOTSTART">
  <L [1] <A "LOT003">>
>
```

### 2.2 HCACK 回覆對照

| HCACK | 意義 | 觸發條件 |
|---|---|---|
| 0 | 接受；模式已切、lot 已註冊 | 清單解析成功且守衛通過 |
| 1 | 電文清單格式錯 | 外層非預期 LIST |
| 2 | 參數錯 | pair 長度非 2 / name 非 `SORTMODE` / 值域外 / **pair 無 lot** / 清單截斷 / lot 讀取失敗 / 超過容量(64) / 型別不符 |
| 4 | 設備忙（整包拒絕） | 外層守衛 `SystemStart \|\| HasICUnderMachine`，**或** pair 存在且 `MachineRun.bRunning==true`（lot 已開始） |

### 2.3 主機回讀（SVID 66032）

host 以 `S1F3` 查 SVID **66032**（INT_4）→ `S1F4` 回目前 `iSortMode`：`0=Normal / 1=LotBin / 2=LotPassFail / 3=WhiteList`。切模式後可回讀確認。

### 2.4 語意要點（務必寫進客戶規格）

1. **省略即不變（sticky）**：不帶 `SORTMODE` pair 的 LOTSTART 不改模式，且跨重開機持久（`GeneralSetting.Save()`）。
2. **切換與 Lot Start 綁定**：`SORTMODE` pair 必須隨一個 lot 一起送；pair-only（無 lot）回 HCACK=2。模式在**同一筆 LOTSTART**內、於 2D 資料載入決策**之前**套用，故該 lot 即以新模式載入（WhiteList→本地檔、否則→WebAPI）。
3. **原子性**：任何解析失敗或忙碌拒絕 → **不註冊任何 lot、不改模式**（無 ghost lot）。
4. **忙碌保護**：lot 已開始（`bRunning`）時送含 pair 的 LOTSTART → 整包 HCACK=4；host 應先 Lot End 再切模式。
5. **重複 pair** → 取最後一個。
6. **白名單檔規格**（沿用 Phase 1）：`HT160S_WhiteList\WhiteList.json`，`"Maps"` schema，`LotNumber` 須對齊 host 宣告的真實 Lot ID，碼 byte-exact、不得重複。

---

## 3. 逐檔改動

### 3.1 `SecsGem/uHGemHT160.cpp`

**(a) LOTSTART 緩衝式解析 + SORTMODE pair**（`S2F42_Host_Command_Acknowledge`，約 842–966 行）
- 內層逐項先 `GetDataItemLenAndType`（**非消耗 peek**）判型別：
  - `LIST_TYPE` → 當 `SORTMODE` pair：`GetDataItemLenAndTypeAndDelete(pairLen, LIST_TYPE)`（消耗 header），`pairLen!=2` → HCACK=2 break；讀 name/value，非 `SORTMODE` 或值域外 → HCACK=2 break。
  - `ASCII_TYPE` → lot id，緩衝進 `bufLots[HT160_MAX_LOT]`（=64）。
- lot **先緩衝不 commit**；迴圈結束後才在 `if(HCACK==0)` 區塊 `LotRegistry.AddLot()`，再套模式 → **在既有 2D 載入決策之前**。
- 樣式對齊已驗證的 `START_AGV`（同檔 ~920–969）peek/consume 慣例。

**(b) SVID 66032**（`AddSV`，約 124 行）
```cpp
HGemPtr->SetSVDataPointer(66032, HType.INT_4_TYPE, "Sort Mode", "",
    &GeneralSetting.iSortMode, "0=Normal 1=LotBin 2=LotPassFail 3=WhiteList");
```
綁 live config int（全域穩定位址、serialize 時讀），免 `RefreshSVData` mirror。

**(c) include**：新增 `#include "maintenance.h"`（取用 `fMaintenance` + 新方法）。

### 3.2 `maintenance.h` / `maintenance.cpp`
新增 public 方法 `SyncSortModeSelectorFromSetting()`：在 `bLoadingHardwareSettings=true` 括號內把 `rgSortMode->ItemIndex` 設為 `iSortMode`，**抑制 `rgSortModeClick` 的 re-entrant OnClick 與其 modal**（否則會在 HSMS 接收路徑彈窗卡 T3），避免 stale 硬體頁 `SaveHardwareSettings` 靜默把 host 切換改回。

### 3.3 `aSortArm.cpp`（Big5 檔，以 latin1 byte-splice 修改）
**兩處** `iTrace2D` 推導站點（`TransferPickDataFromLoader` ~1491、`RecordAutoSkippedCells` ~1262）：
```cpp
else if(Slot[...].LotIndex<0)
    iTrace2D = GeneralSetting.IsWhiteListSortMode() ? 1005 : 1000;
```
即「碼讀得到但無 owning lot」→ WhiteList 模式=名單外拒收(1005)，其餘=一般 NoMap(1000)。

### 3.4 `deviceinfo.cpp`
`AddTraceInfo` 加 `case 1005: sLabel = "NotWhitelisted";`。與 `SaveRejectRecord` 寫的 `eErrorCode`（如 "AutoSkip"）為不同欄位，三欄（eTraceCode/eErrorType/eErrorCode）並存不衝突。

---

## 4. 關鍵正確性決策

| 決策 | 說明 |
|---|---|
| **緩衝式原子提交** | lot 先進 `bufLots[]`，僅在全清單解析成功且守衛通過後才 `AddLot`。任何拒絕路徑不留 ghost lot、不改模式。 |
| **`bRunning` busy 守衛** | 既有 832 行守衛只擋 `SystemStart\|\|HasICUnderMachine`，漏掉「lot 已 Start、機台未 Start、無在製 IC」窗口。pair 存在 + `bRunning` → HCACK=4。 |
| **套模式在 load 決策之前** | `iSortMode` 於 `if(HCACK==0)` 區塊套用，位於 post-block 的 WhiteList-vs-WebAPI 載入判斷之前，確保該 lot 以新模式載入。 |
| **UI 同源守衛** | SECS 切換後同步 `rgSortMode` 選擇器，與 UI 端 `rgSortModeClick` 的 `HasICUnderMachine` 守衛對稱；避免 stale 頁覆寫。 |
| **VCL 主執行緒安全** | S2F42 跑在 VCL 主執行緒（stNonBlocking OnClientRead）；同步用 `bLoadingHardwareSettings` 抑制不彈 modal，無 T3 風險；`Save()` 為短檔寫。 |
| **trace 兩站點一致** | 同一拒收在正常揀料與 AutoSkip 兩路徑都記 1005，Production_Log 分析不會前後不一。 |

---

## 5. 建置與編碼閘門

| 閘門 | 結果 |
|---|---|
| sim `-Full`（`SOFT_SIMULATE` on，開發 active define） | EXIT 0；`EXE/ht160s.exe` 新產出 |
| 真機 `-Full`（`SOFT_SIMULATE` off） | EXIT 0（驗證 shared core 於真機組態亦編譯乾淨） |
| 還原 define + sim 重編 | EXIT 0；`MachineType.h` 無 diff（define 已還原） |
| 編碼檢查 `check-ht160s-source-encoding.ps1` | 163 檔通過（無 `EF BF BD`、無 UTF-8 BOM） |
| Big5 檔改法 | `aSortArm.cpp` 以 latin1 byte-splice；git diff 恰 2 行、Big5 bytes(7) 保留、無行尾 churn |

---

## 6. 對抗式審查結果

5 視角（SECS 解析 / HCACK 原子性 / 回歸相容 / 併發-T3 / 領域-trace-SVID）獨立審查，每個發現再由獨立 skeptic 讀實際碼嘗試反駁。

- **Confirmed 缺陷：0**
- **提出並 REFUTED 的 NIT：2**（皆屬「相對舊碼的刻意行為改變」，非缺陷；見 §7）

> 這代表本次改動通過對抗式複驗，未發現真實邏輯/協定/回歸缺陷。（compile-clean ≠ works：仍需上機驗證，見 §8。）

---

## 7. 與舊行為的差異（透明列出，皆為改善）

1. **超過 64 個 lot id 的單筆 LOTSTART**：舊碼靜默截斷到 64 並回 HCACK=0；新碼回 HCACK=2（整包拒絕）。新行為對 host 誠實，且 `bufLots[64]` 固定陣列需此上限保記憶體安全。實務上 LOTSTART 通常僅 1 個 lot。
2. **中途解析失敗的 partial-commit**：舊碼會把失敗點前的 lot 永久留在 registry（HCACK=2 但殘留 ghost lot）；新碼原子化（拒絕即不留）。無合規 host 會依賴舊的殘留行為。

---

## 8. 上機驗證清單（host round-trip）

建議用 `HT160S_SECS_Simulator` 實跑（勿偽造 log）：

- [ ] LOTSTART + `SORTMODE WHITELIST` → HCACK=0；SVID 66032 回讀=3；maintenance 頁 radio 顯示 By WhiteList。
- [ ] LOTSTART + `SORTMODE NORMAL` → HCACK=0；SVID 66032 回讀=0。
- [ ] 純 lot（無 pair）→ 模式不變（sticky）。
- [ ] pair-only（無 lot）→ HCACK=2。
- [ ] `SORTMODE FOO`（值域外）/ pairLen≠2 → HCACK=2。
- [ ] lot 已 Start（`bRunning`）時送含 pair → HCACK=4；模式未變、lot 未動。
- [ ] WhiteList 模式跑一輪：名單外 IC → Production_Log `eTraceCode=1005 / eErrorType=NotWhitelisted`。
- [ ] AutoSkip 開啟時，名單外且揀料失敗 IC → 1005/NotWhitelisted + eErrorCode=AutoSkip 並存。
- [ ] host 切 WHITELIST 後，開 maintenance 硬體頁再存檔 → 模式不被改回。

---

## 9. 仍待辦

- **StateRecord 4-way 具名標籤**（§1 延後）：待 `cStateRecordHT160.cpp` 的 SR_Trace 除錯碼清除/提交後，1 行三元擴充。
- **客戶規格書**（計畫 §6.6）：HCACK 表 / 值域大小寫 / 省略語意 / 白名單檔 schema / 範例交易 log（SECS_Simulator 實跑）。
- **上機 host round-trip 驗證**（§8）。
