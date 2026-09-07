# WhiteList 臨時覆蓋模型 + 獨立面板 + Main 徽章 — 設計與修改計畫 (v3)

> 建立日期：2026-07-17　分支：feat/iosetview-172-refactor
> 狀態：**CODE-COMPLETE + 建置(sim+真機 -Full EXIT0) + 對抗式複驗(wf_98d6a8dd，3 發現已修) 完成；UNCOMMITTED（共用 worktree 與 settle-delay 重構糾纏，user 決定先不提交）。剩：提交 + 客戶規格書 + 上機驗證。**
> 對抗式複驗修正：①BLOCKER `Load()→SetDefault()` 清 overlay（開維護頁中途解武裝）→ boot-clear 移到 ctor；②MAJOR SECS 無 pair LOTSTART 不 disarm（洩漏下一 lot）→ 每個接受的 LOTSTART 都設 overlay；③MINOR Load 後徽章 stale → LoadHardwareSettings 尾刷新。
> 前置：Phase 1 (a46b7e9) + Phase 2 (4466cac) 已上線；本計畫**改寫**其模式模型（sticky → 臨時覆蓋）。
> 偵察：wf_720037ff-233（sortmode / panel / badge / lifecycle / secs 五路 + completeness critic，皆 file:line 佐證）。
> 需求來源：2026-07-17 對話。user 要點：
> 1. WhiteList 目前是 rgSortMode 第 4 個 radio，與生產模式平起平坐，不妥。
> 2. WhiteList = 客戶特殊功能、**非常態生產流程**；Lot Start 要**特別命令**才啟動，**Lot End 後還原**成原本生產模式（貴司 = By Lot+PassFail）。
> 3. 新增獨立 panel（比照 pnlSortModeBox 建置），元件命名有意義。
> 4. Main 加「目前使用模式」顯示（比照 pnlFeatureBadge1）。

---

## 0. 決策摘要（2026-07-17 user 拍板）

| # | 決策 | 選擇 |
|---|---|---|
| D1 模式架構 | 移出 rgSortMode，改臨時覆蓋 **base + overlay** | ✅ 採建議 |
| D2 SECS | **一併改寫** Phase 2 SORTMODE pair 成新模型 + 更新客戶規格書 + 重測 | ✅ 採建議 |
| D3 本地啟用介面 | 勾選框 `chkWhiteListActive`（idle 才可改） | ✅ 採建議 |
| D4 還原目標（AI 定，可改） | 還原成**維護頁設定的基礎生產模式**（非寫死 smLotPassFail；貴司 base=LotPassFail 故外觀一致、也更安全） | 預設 |
| D5 重啟續產（AI 定，可改） | overlay 隨工單持久；中途重開機沿用「繼承上次工單？」決策（YES 續 WhiteList / NO 清） | 預設 |

---

## 1. 核心模型：base（基礎生產模式）與 effective（有效模式）分離

```
基礎生產模式 iSortMode ∈ {smNormal, smLotBin, smLotPassFail}   ← rgSortMode 設定、sticky 持久(General.ini)
        + WhiteList 疊加 bWhiteListActive (布林；隨工單走、Lot End 清)
        ↓
有效模式 GetEffectiveSortMode() = bWhiteListActive ? smWhiteList : iSortMode
```

**關鍵不變式**：`iSortMode`（base）**永遠不會是 smWhiteList**。WhiteList 只以 overlay 存在。
→ 開機/當機重啟天然回到基礎生產模式（= 需求「非常態、要重新命令」語意，免費得到）。

### 1.1 helper 改寫（GeneralSetting.h:62-68，唯一集中改點）

現況：
```cpp
bool IsNormalSortMode()      { return iSortMode==smNormal; }
bool IsLotBinSortMode()      { return iSortMode==smLotBin; }
bool IsLotPassFailSortMode() { return iSortMode==smLotPassFail; }
bool IsWhiteListSortMode()   { return iSortMode==smWhiteList; }
bool IsDynamicBindingMode()  { return iSortMode==smLotBin || iSortMode==smLotPassFail; }
```
改為（全部改讀「有效模式」；消費端零改，因大家本就走 helper）：
```cpp
int  GetEffectiveSortMode()  { return bWhiteListActive ? smWhiteList : iSortMode; }
bool IsNormalSortMode()      { return GetEffectiveSortMode()==smNormal; }
bool IsLotBinSortMode()      { return GetEffectiveSortMode()==smLotBin; }
bool IsLotPassFailSortMode() { return GetEffectiveSortMode()==smLotPassFail; }
bool IsWhiteListSortMode()   { return bWhiteListActive; }   // 等價 effective==smWhiteList
bool IsDynamicBindingMode()  { return IsLotBinSortMode() || IsLotPassFailSortMode(); }
```

### 1.2 新增成員 / 方法（GeneralSetting.h/.cpp）

```cpp
// overlay：本 lot 是否套 WhiteList（隨工單走、非 General.ini sticky）
bool bWhiteListActive;
// 給 SVID 66032 指標回讀用的「有效模式」鏡像（helper 用計算式即可，SVID 需要實體 int）
int  iEffectiveSortMode;
void RecomputeEffectiveSortMode() { iEffectiveSortMode = GetEffectiveSortMode(); }
void SetWhiteListActive(bool b)   { bWhiteListActive=b; RecomputeEffectiveSortMode(); }
```
- ctor/SetDefault（GeneralSetting.cpp:40 附近）：`bWhiteListActive=false;` 後 `RecomputeEffectiveSortMode();`。
- **凡改動 `iSortMode` 之處**（Load、rgSortModeClick、SaveHardwareSettings）尾端呼叫 `RecomputeEffectiveSortMode()`，讓鏡像同步。

### 1.3 base clamp 收回 + back-compat 遷移（GeneralSetting.cpp:125-127）

- clamp 上界從 `smWhiteList` **收回 `smLotPassFail`**（base 不再收 3）。
- **遷移**：Load 若讀到 `Mode==smWhiteList(3)`（Phase 2 sticky 殘留）→ 視為 `iSortMode=smNormal`（WhiteList 路由本就等同 Normal）、`bWhiteListActive=false`。
  - 部署註記：跨此升級、原本卡在 sticky WhiteList 的機台，開機會落在 Normal（base），需重新命令 WhiteList。屬工程升級可接受。
- legacy `UseLotBinMode` 寫入（:218）不變（WhiteList overlay 與它無關；base 仍只 0/1/2）。

### 1.4 SVID 66032 改指鏡像（uHGemHT160.cpp:127）

`SetSVDataPointer(66032, ..., &GeneralSetting.iSortMode, ...)` → 改指 `&GeneralSetting.iEffectiveSortMode`。
→ host 讀回**有效模式**：lot 中=WHITELIST、lot 間=base，前後一致（化解 Phase 2 的 host/機台分歧雷）。

---

## 2. 持久化與生命週期（overlay 跟著工單）

| 事件 | 動作 | 掛鉤點 |
|---|---|---|
| 本地啟用 | `chkWhiteListActive` 勾選 → `SetWhiteListActive(true)`（HasICUnderMachine 守衛） | 新 handler（§4） |
| SECS 啟用 | `SORTMODE=WHITELIST` pair → `SetWhiteListActive(true)` | uHGemHT160.cpp（§6） |
| Lot Start | overlay 已由上兩者設好；SaveWorkOrder 一併存 overlay | main.cpp:2234 / uHGemHT160.cpp:956 |
| Lot End（還原）| `SetWhiteListActive(false)` → 有效模式回 base | btnLotEndClick 早段（§5） |
| CleanOut 自動收尾 | 若日後成真 work-order-close，同步 disarm | csystem.cpp:1414（條件式，見 §5.2） |
| 重啟續產 | RestoreLastWorkOrder 沿用工單 → 還原 overlay；fresh(NO) → 清 overlay | main.cpp RestoreLastWorkOrder |

**持久化位置**：`bWhiteListActive` 存進**工單狀態**（WorkOrder.json / lastdata，per-lot tier），**不進 General.ini**（那是 sticky 機台組態 tier，放 per-lot 狀態違反 config-tiers）。
- 實作前確認：WorkOrder 序列化欄位（新增一個 bool 欄）與 RestoreLastWorkOrder 的 YES/NO 分支讀回點。
- 洗盤原則對齊既有 LotBinBinding：Lot Start / Lot End 清、Machine Start(resume) 不清。

---

## 3. UI-A：rgSortMode 瘦身回 3 項

- **maintenance.dfm:2153-2157**：`Items.Strings` 移除 `'By WhiteList'`，回 3 項 `('Normal','By Lot+Bin','By Lot+PassFail')`。
  - `rgSortMode` Height 98→~74、父容器 `pnlSortModeBox` Height 104→~80（Phase 1 為第 4 項加高的還原）。**手工編輯 DFM，禁 designer**（[[bcb-designer-save-strips-components]]）。
  - `lblLotBinModeHint` 文案移除 WhiteList 段（WhiteList 說明移到新 panel 的 hint）。
- **maintenance.cpp:1181-1184** SaveHardwareSettings clamp：`idx<=smLotPassFail`（不再收 smWhiteList）+ 尾呼 `RecomputeEffectiveSortMode()`。
- **maintenance.cpp:1107-1108** LoadHardwareSettings：`rgSortMode->ItemIndex=iSortMode`（base 保證 0/1/2，恆合法，不再 out-of-range）。
- **maintenance.cpp:1924-1953** rgSortModeClick：clamp 收回 + 尾呼 `RecomputeEffectiveSortMode()` + `UpdateSortModeFeatureBadge()`（§5.3）。HasICUnderMachine 守衛保留。

---

## 4. UI-B：新獨立面板 pnlWhiteListModeBox（比照 pnlSortModeBox）

**放置**：同 tsLotInfo（maintenance.dfm:2108，pnlSortModeBox 的相鄰 alTop panel）。沿用該區樣式 `pnl<Feature>Box 容器 + lbl<Feature>Hint 深藍換行 + 單一控制項`。

**DFM 塊（手工加，maintenance.dfm，緊鄰 pnlSortModeBox 之後）**：
```
object pnlWhiteListModeBox: TPanel
  Left=0 Top=104 Width=929 Height=~64 Align=alTop BevelInner=bvLowered TabOrder=1
  object chkWhiteListActive: TCheckBox   // Left=8 Top=8
    Caption='Activate By WhiteList for this lot (customer special; reverts at Lot End)'
    OnClick=chkWhiteListActiveClick
  object lblWhiteListModeHint: TLabel     // clNavy WordWrap AutoSize=False
    Caption='By WhiteList: only 2D codes in HT160S_WhiteList\WhiteList.json are sorted; '
            'others -> Error. Special mode, NOT normal production. Takes effect at next Lot '
            'Start and auto-reverts to the base sort mode at Lot End. Idle only.'
end
```

**maintenance.h（__published，與 tab/panel 姊妹相鄰，:292-295 附近）**：
```cpp
TPanel    *pnlWhiteListModeBox;
TCheckBox *chkWhiteListActive;
TLabel    *lblWhiteListModeHint;
void __fastcall chkWhiteListActiveClick(TObject *Sender);   // :377-386 __published 區
```

**maintenance.cpp — chkWhiteListActiveClick**（比照 rgSortModeClick 骨架）：
```cpp
void __fastcall TfMaintenance::chkWhiteListActiveClick(TObject *Sender)
{
    if(bLoadingHardwareSettings) return;                 // 程式化設值抑制
    if(fMain!=NULL && fMain->HasICUnderMachine()) {      // 有在製品：擋、還原、警告
        bLoadingHardwareSettings=true;
        chkWhiteListActive->Checked = GeneralSetting.bWhiteListActive;
        bLoadingHardwareSettings=false;
        ShowMyMessage("Cannot change WhiteList while material is under the machine.");
        return;
    }
    GeneralSetting.SetWhiteListActive(chkWhiteListActive->Checked);   // arm/disarm overlay
    RefreshHardwareSettingsStatus();
    if(fMain!=NULL) fMain->UpdateSortModeFeatureBadge();              // 徽章即時更新
    ShowMyMessage("WhiteList override takes effect at the next Lot Start.");
}
```

**Load 同步**（maintenance.cpp:1107 附近，bLoadingHardwareSettings=true 窗內）：
`chkWhiteListActive->Checked = GeneralSetting.bWhiteListActive;`

**EditLock**（maintenance.cpp:1258-1259 附近）：
`if(chkWhiteListActive!=NULL) chkWhiteListActive->Enabled=bEnable;`（比照 rgSortMode->Enabled 分開鎖）。

**SECS 外部同步**（maintenance.cpp:1960-1968 SyncSortModeSelectorFromSetting）：擴充成同時同步
`rgSortMode->ItemIndex=iSortMode`（base）與 `chkWhiteListActive->Checked=bWhiteListActive`（overlay），仍在 bLoadingHardwareSettings 括號內。

---

## 5. UI-C 生命週期掛鉤 + Main 徽章

### 5.1 Lot End 還原（disarm）— 早段放置避開競態

`btnLotEndClick`（main.cpp:2474）於 `MachineRun.bRunning=false`（:2484）**之後、任何檔案 I/O / modal / CEID 之前**插入：
```cpp
GeneralSetting.SetWhiteListActive(false);            // 有效模式回 base
GeneralSetting.RecomputeEffectiveSortMode();         // (SetWhiteListActive 已含，保險)
if(fMaintenance!=NULL) fMaintenance->SyncSortModeSelectorFromSetting();
UpdateSortModeFeatureBadge();
```
- **競態防禦**：disarm 在 handler 早段、無 ProcessMessages 打斷 → 即使 host 對下一 lot 的 LOTSTART(WHITELIST) 之後才被派送，arm(N+1) 也在 disarm(N) 之後、正確勝出。**動工後對抗式複驗須確認插入點到 handler return 間無 modal pump。**

### 5.2 CleanOut 自動收尾（第二出口）

[csystem.cpp:1407-1434](../../HT160S_Program_BCB_V1.0.0.0/csystem.cpp) 目前是「drained not closed」（不清 registry、不發 CEID12）→ **lot 尚未關**，overlay 應**維持**。
→ **本計畫不在此 disarm**。僅當 [[cleanout-amr-unload-lotend]] 讓 CleanOut 變成真 work-order-close 時，才把 §5.1 的 disarm 一併掛到 :1414。文件標注此依賴。

### 5.3 Main 徽章 pnlFeatureBadge4（比照 pnlFeatureBadge1；徽章格下排整排空、容量 6）

- **main.h:55-65**：`eMainFeatureReserve2` → `eMainFeatureSortMode`。COUNT/COLS/ROWS/CAPACITY 不動（4 個仍在 3×2 內）。
- **main.dfm:2493-2534**：複製 pnlFeatureBadge3 塊為 `pnlFeatureBadge4`（子 `lblFeatureName4` Caption `'SORT'` + `lblFeatureValue4`），置於 pnlFeatureStatus。Left/Top 由 LayoutFeatureBadges 覆寫，Width=132 Height=30。DFM 為 ASCII，Edit 安全。
- **main.cpp BuildFeatureStatusBadges()（:423-475）**：綁 `FeatureStatusPanels/NameLabels/ValueLabels[eMainFeatureSortMode]=pnlFeatureBadge4/...4`；初次 paint 呼 `UpdateSortModeFeatureBadge()`（比照 AmrBadge :470）。
- **新 `TfMain::UpdateSortModeFeatureBadge()`**（比照 UpdateSecs/AmrFeatureBadge :537-596，edge-trigger 選配）：
  ```cpp
  int m = GeneralSetting.GetEffectiveSortMode();
  switch(m){
    case smNormal:      SetFeatureStatusBadge(eMainFeatureSortMode,"NORMAL",   clGray);  break;
    case smLotBin:      SetFeatureStatusBadge(eMainFeatureSortMode,"LOT+BIN",  clGreen); break;
    case smLotPassFail: SetFeatureStatusBadge(eMainFeatureSortMode,"PASS/FAIL",clGreen); break;
    case smWhiteList:   SetFeatureStatusBadge(eMainFeatureSortMode,"WHITELIST",clRed);   break;  // 醒目：非常態
  }
  ```
- **呼叫點**（critic 4a：漏一個徽章就 stale）：rgSortModeClick、chkWhiteListActiveClick、SECS SORTMODE 套用後、Lot End disarm、BuildFeatureStatusBadges 初次。
- 徽章「顏色」= lblFeatureValueN 的 Font.Color（既有慣例）。

---

## 6. SECS 改寫（D2；動到已上線 Phase 2）

**uHGemHT160.cpp LOTSTART handler（:822-973）SORTMODE pair 套用段（:931-937）改寫**：
- 現況：`GeneralSetting.iSortMode = (WHITELIST?smWhiteList:smNormal); Save();`
- 改為：
  - `SORTMODE=WHITELIST` → `GeneralSetting.SetWhiteListActive(true);`
  - `SORTMODE=NORMAL`   → `GeneralSetting.SetWhiteListActive(false);`（語意：用**基礎生產模式**，非強制 smNormal）
  - **不再寫 base `iSortMode`、不再 `GeneralSetting.Save()`**（overlay 隨工單存，見 §2）。
  - 保留：`if(fMaintenance!=NULL) fMaintenance->SyncSortModeSelectorFromSetting();` + `fMain->UpdateSortModeFeatureBadge();`
- **bRunning 守衛（:919-920）保留**（arm 仍禁在跑時切）。
- **值域**：pair 值仍限 `NORMAL|WHITELIST`（host 不需、也無法命令 base 生產模式；base 是本地維護頁的事）。
- **WhiteList 檔載入判斷（:962-965）**：`IsWhiteListSortMode()`（已讀 effective）不變。

**客戶規格書更新點（§6.6 of 20260714 plan）**：
- `SORTMODE=WHITELIST` = 本 lot 套白名單覆蓋；`SORTMODE=NORMAL` = 用設備基礎生產模式（**不再是 sticky 全域切換**）。
- **省略 pair 語意改變**：不帶 pair = 用**基礎生產模式**（overlay 預設 off），**不再是「沿用上次 sticky 模式」**。→ host 每個白名單 lot **都要**帶 `SORTMODE=WHITELIST`。
- SVID 66032 回報有效模式（lot 中 3、lot 間 = base 0/1/2）。
- 重跑 `HT160S_SECS_Simulator` round-trip，log 實跑不偽造（[[secs-comm-examples-doc-maintenance]]）。

---

## 7. 診斷：StateRecord 4-way（順手收 TODO）

- **cStateRecordHT160.cpp:583-584**：3-way → 4-way，且改依 `GetEffectiveSortMode()` 具名（否則白名單 run 記成 Normal，與 Main 徽章矛盾）。額外 dump `bWhiteListActive` + base `iSortMode` 兩欄，狀態全透明。
- **cStateRecordHT160.cpp:781** raw dump：改 dump `iEffectiveSortMode`（或 base+overlay 兩者）。
- trace 1005（aSortArm.cpp:1262/1491）、deviceinfo.cpp:244 `NotWhitelisted`：不變（讀 IsWhiteListSortMode()=effective）。

---

## 8. 修改清單總表

| 層 | 檔案 | 內容 | 風險 |
|---|---|---|---|
| A 模型 | GeneralSetting.h/.cpp | overlay 成員+helper 改讀 effective+GetEffective/Recompute/SetWhiteListActive+clamp 收回+Mode3 遷移 | **中；.h 改→full rebuild** |
| B UI-A | maintenance.dfm/.cpp | rgSortMode 回 3 項+容器高度還原+clamp×2+Recompute | 中；DFM 手工 |
| C UI-B | maintenance.dfm/.h/.cpp | 新 pnlWhiteListModeBox+chkWhiteListActive+handler+Load 同步+EditLock+Sync 擴充 | 中；DFM 手工 |
| D 徽章 | main.dfm/.h/.cpp | eMainFeatureSortMode+pnlFeatureBadge4+UpdateSortModeFeatureBadge+多點呼叫 | 低-中 |
| E 生命週期 | main.cpp | Lot End disarm 早段+RestoreLastWorkOrder overlay 續產/清+SaveWorkOrder 存 overlay | 中 |
| F SECS | SecsGem/uHGemHT160.cpp | SORTMODE pair→arm/disarm overlay+66032 指鏡像+Sync/Badge 呼叫 | 中（配 Simulator 測） |
| G 診斷 | cStateRecordHT160.cpp | 4-way effective 名+base/overlay dump | 低 |
| H 文件 | docs/plan+SECS Comm Examples+客戶規格 | 模型改變+SORTMODE 語意+省略語意 | 低 |
| — 免改 | aSortArm/aLoader routing、GetMappedAutoIndex、綁定表、LoadWhiteListFile、error-bin、bin display | 全走 helper，effective 生效 | — |

---

## 9. Build gate（CLAUDE.md）

- 改 `.obj` 刪後重編；`GeneralSetting.h`/`main.h`/`maintenance.h` 變更 → **full rebuild**（`scripts/ops/build-ht160s.ps1 -Clean`）。
- 真機組態驗證：註解掉 `MachineType.h` 的 `#define SOFT_SIMULATE`，`-Full` EXIT0，再**還原 define** 重編。
- 編碼：`check-ht160s-source-encoding.ps1` 過關；新註解 ASCII 英文；aSortArm.cpp/aLoader.cpp 檔首 Big5，非 ASCII 區段走 python byte-splice（[[edit-tool-corrupts-big5-source]]）—— 本計畫這兩檔僅 trace 註解區，多為 ASCII，仍逐檔確認。

---

## 10. 待驗證風險（動工後對抗式複驗，[[adversarial-review-after-feature]]）

1. **Lot End disarm 競態**：disarm 到 handler return 間確無 modal/ProcessMessages 打斷（否則 host LOTSTART(N+1) 可能被 disarm 蓋回）。
2. **overlay 持久化正確性**：SaveWorkOrder 存到、RestoreLastWorkOrder YES 續 / NO 清、Machine Start(resume) 不清；三態如 LotBinBinding。
3. **Mode==3 遷移**：升級後原 sticky WhiteList 機台落 base=Normal，不誤判為武裝。
4. **66032 鏡像同步**：凡改 base/overlay 皆 Recompute，host S1F3 讀值即時正確。
5. **EditLock 涵蓋**：跑動中 chkWhiteListActive 與 rgSortMode 同鎖；HasICUnderMachine 兩處守衛等價。
6. **徽章不 stale**：五個呼叫點齊全。
7. **rgSortMode out-of-range**：base 恆 0/1/2，ItemIndex 不再落 -1。
