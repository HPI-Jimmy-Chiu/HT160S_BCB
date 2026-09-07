# SortArm/Loader 定位對齊 HT172 模型 — 作戰計畫

狀態：**計畫待審 / 未動工**（依指示先出計畫）。
產出：workflow `wf_ea1e7a7d-277`（8 偵查員 + 合成 + 完整性審查），已整合審查修正。
前置已完成：tray pitch 單位 bug 已修（`GetTrayXPitch/YPitch *100`，見 sortarm-loader-pitch-unit-bug 記憶）。

---

## ★ P0 盤點結果（已完成 2026-06-24，不改碼）

| 查證項 | 結果 |
|---|---|
| 機台真實 setup.ini（State Record 2026-06-24 17:09, Test20260611）| **XStart=67.95, YStart=55.5**, Pitch=102/102, Div 1×3 |
| repo data/Test20260611/setup.ini | 20/20/50/50/3×3（**與機台 drift**，repo 不可信）|
| repo data/Default/setup.ini | 0/0/1/1（預設）|
| SECS/GUI 能寫 XStart/YStart？ | **能** — host S2F16 + GUI 都寫 EC2758-2763（含 2760/2761）→ `TrayForm.Save` → setup.ini（uHGemHT160.cpp:560/591）|
| BottomCCD 掃描鏈 | **未接線** — `SortArmToBottomCCDFirstX`/`BottomCCDYCapture` 只有 teach/offset，**無任何運動消費者** → 無逐格掃描原點問題（critic 撞機風險**解除**）|
| aAuto1To6 feed | `:518` `MoveAutoY(GetAutoFirstSortY)` 確為第三個共用 `AutoCarFirstSortY` 的點（Row=0 第一格），改 Start 須一致 |
| Y 方向 | HT160 現況遞增（`FirstSortY+Row*pitch`）；本機 YDivision=3 |

### ⚠️ 對策略的決定性衝擊：策略 A 破滅 → 轉向策略 B

**現場 Start 非 0 且為有意義大值（67.95/55.5）**，不是 0。代表操作員是「按 HT172 模型認知」在填（tray 原點偏移），只是 HT160 不消費它 → 定位實際全靠 teach 補償、Start 被忽略。

- **策略 A 不可行**：注入 `GetTrayXStart()` 會立刻把 X 偏 **+67.95mm**、Y 偏 55.5mm，不是等價落地。
- **真正對齊 HT172 = 策略 B 數學換算**：`new_base = old_teach − XStart*100`（X）/ `old_teach ± YStart*100`（Y，符號待真機定）。換算後第一格位置不變（`new_base + Start = old_teach`），且此後操作員調 Start 會生效 ← 符合操作員原意。
- 換算須在 **`TeachBase` 端**（非 effective `Teach.*`，因 cprod.cpp:165/167/173 已折 Offset）+ tech.ini **版本旗標**防重複換算。

**新推薦：策略 B**（P0 證據驅動，取代第 4 節原推薦的 A）。

### 機構基準鏈（user 確認 2026-06-24，①② 已答）

- **校正盤基準點**（teach 點 = HT172 `iPos_Loader_X/Y`）位於 tray **左上尖角往內 10mm**。
- **±1000 (=10mm)** = 基準點 ↔ 左上尖角 的偏移（HT172 `−1000`(X)/`+1000`(Y) = 從基準點退回尖角）→ **`DATUM_BIAS = ±1000`，非 0**。
- **XStart=67.95** = 左上尖角往**右** 67.95mm 到第一顆 IC；**YStart=55.5** = 尖角往**下** 55.5mm 到第一顆 IC。
- 完整鏈：`第一格 = 校正盤基準 − 10mm(到尖角) + Start(尖角→第一格)`。

HT160 現況 teach = **第一格**位置（機台正常運作反推）。策略 B 換算成校正盤基準：

`new_base = old_firstcell + 1000 − XStart×100`（X，1/100mm）

實例（Loader1 X）：`19022 + 1000 − 6795 = 13227`（132.27mm）；驗證第一格不變 `13227 − 1000 + 6795 = 19022` ✓。Auto1~6 同法（同一 XStart）。

**X 已可完整實作**（公式 + 換算值都確定）。**Y 仍差一個方向符號** —— 本機 Y 隨 row 遞增、HT172 遞減，`new_base_Y` 與 `GetSortArmCellY` 的 YStart 正負需真機確認一次（P3 / 或現場量一筆）。

---

## 0. 目標與單位前提

把 HT160 的 SortArm 取放 / Loader CCD 定位，從現行 **teach-first-cell** 模型，改成對齊 HT172 的 **「工位參考點 + XStart/YStart 偏移 + 格間距」** 模型，讓 setup 畫面的 X-Start/Y-Start 真正生效。

單位鐵律：馬達/teach 座標全程 **1/100mm（條）**。`TrayForm.XStart/YStart/XPitch/YPitch` 在 `setup.ini` 存 **mm**（`ReadFloat`），**只在 getter 端 ×100**，`TrayForm` 本體與 SECS EC 對外維持 mm 不變。

---

## 1. HT172 vs HT160 模型對照

| 項目 | HT172（目標） | HT160（現況） |
|---|---|---|
| **base 工位點** | `iPos_Loader_X` / `iPos_Unload_X[][]` = 工位**參考點**，需再加偏移才到 cell(0,0)（aSortArm.cpp:1690-1700） | `Teach.SortArmToLoaderX` / `*FirstSortY` = **第一格絕對座標**（XStart 已折入） |
| **XStart/YStart** | `atof(edXStart)*100`，X 加 `+XStart`、Y 減 `-YStart`（:210-211） | 已載入但**定位完全不消費**、未 ×100（死參數） |
| **格間距** | `iTrayXPitch/YPitch *100`，只當格與格間距 | `GetTrayXPitch/YPitch *100`（已修），**一值兩用**兼任格距+吸嘴補償 |
| **吸嘴實體間距** | 獨立量 `iColPitch/iRowPitch`（硬體窗 3000~6000 / 6000~12000） | 無獨立量，用 tray pitch 兼任 |
| **機構偏移** | X `-1000`、Y `+1000`（=10.00mm 固定角偏置） | **無此常數**（疑已折入 teach，照搬會偏 10mm） |
| **方向** | Y 隨 row **遞減**、X 吸嘴補償**減** | Y 隨 row **遞增**、X 補償**加**（本機座標方向與 172 相反） |

**關鍵結論**：本機**只啟用 suck2**（=`iBaseSuckX`，`General.ini SuckerEnabled1=1`），單吸嘴下 HT172 模型退化成 `base ±1000 ±Start + index*格距`，與 HT160 現況的差別只剩 **(a) XStart/YStart 偏移** 與 **(b) ±1000 常數** 兩項。吸嘴間距分離可延到多吸嘴啟用前。切入點極乾淨。

---

## 2. 核心設計（最小且精準的切口）

新增**偏移 getter** + 把偏移注入**唯二的定位入口**：

- X：所有 X 定位都過 `GetSortArmCellX`（aSortArm）/ `MoveToCcdCell`（aLoader）→ 改一處覆蓋全部。
- Y：目前散在 **5 處 inline**（見下），先抽成 `GetSortArmCellY` 集中，再注入偏移。

偏移與方向用**具名常數**參數化，**預設 0**（落地時行為與現況等價，可 sim 逐位元驗證），方向/常數值留待真機確認後才賦值——避免在未驗證前押錯方向。

---

## 3. 改造點清單（已併入審查補漏）

### A. aSortArm — X 入口
- **`GetSortArmCellX` (:291-295)**：`X = base + SORT_ARM_X_DATUM_BIAS + SORT_ARM_X_START_SIGN*GetTrayXStart() + (ColMinusSlot+iBaseSuckX)*GetTrayXPitch()`。常數/符號預設 0/+。
  - ⚠️ 審查修正：**不可**宣稱「單吸嘴下吸嘴補償退化為 0」——`(ColMinusSlot+iBaseSuckX)` 在 X **並非恆 0**（`MoveToLoaderPick` 的 `BaseX=PickX-FirstSlot` 可達負值）。XStart 加在最外層不影響此項，但理由要寫對：本機把「格步進+吸嘴基準」合一在單一項。

### B. aSortArm — Y 入口（**5 處**，非 4 處）
- 新增 `GetSortArmCellY(BaseSortY, Row)`，取代 inline Y：**:596 / :605 / :1398 / :1468 / :1120**。
  - ⚠️ 審查補漏：第 5 處 = **`ShowPlaceDebugInfo` (:1120)**。漏改會讓校位診斷值與實際運動漂移 → 操作員拿錯數字校位、把對的機台校歪。
  - 公式：`RoundPosition(BaseSortY + SORT_ARM_Y_DATUM_BIAS + SORT_ARM_Y_START_SIGN*GetTrayYStart() + Row*GetTrayYPitch())`。
  - ⚠️ 審查修正：`SORT_ARM_Y_START_SIGN` **預設 0**（不寫死 `-GetTrayYStart()`）。本機 Y 遞增、HT172 Y 遞減，符號方向**延到 P3 真機 + 機構圖確認**才定。

### C. 偏移 getter
- aSortArm：`double GetTrayXStart(){return TrayForm.XStart*100.0;}`、`GetTrayYStart()` 同。
- aLoader：自有同名 getter（**不可跨讀 SortArm**），讀同一 `TrayForm`。

### D. aLoader — CCD 入口
- **`MoveToCcdCell` (:299-306)**：注入 `LOADER_*_DATUM_BIAS + Start`。CCD 掃描格心必須與取放格心**同一原點**，否則錯位。

### E. 待盤點（審查補漏，先查證再決定改不改）
- **SortArm→BottomCCD**：`SortArmToBottomCCDFirstXPosition` / `BottomCCDYCapturePosition`（uteach.cpp:272/328）。**若**底部 2D 是逐格複判 → 必須共用同一 Start 原點，否則複判抓相鄰格 → 誤判/誤踢料。grep 在 aSortArm 找不到其運動消費者，需追到實際掃描碼確認。
- **aAuto1To6**：`TAutoModule::GetAutoFirstSortY`(:160) + `MoveAutoY`(:518) 是**第三個**共用 tray 幾何的模組。判定 feed 對位（Row=0）是否需注入 YStart，確保三模組原點一致。
- **CalculatePitchPosition (:301)** 仍消費 `GetTrayXPitch`（MPitchX 內插輸入）——P4 拆吸嘴間距時這個耦合點要一起顧。

### F. 標頭與 SECS
- aSortArm.h / aLoader.h 加宣告 → 改 .h **必須 `-Full` 全量重編**。
- setup UI **不需改**（edXStart/edYStart 本來就可編輯；避免動 DFM 掉元件）。
- **SECS 單位不變性（硬驗證項，非 open question）**：注入後跑 host 讀 EC2760/2761 確認回傳仍是 mm 原值（`TrayForm.XStart` 未被 ×100 污染）。

---

## 4. teach 值遷移策略

現有 tech.ini 是 first-cell 語意（已折入 XStart 與隱含 ±1000）。讓 Start 進公式後若改語意，舊值會「重複位移」錯位。

| 策略 | 作法 | 評價 |
|---|---|---|
| **A（推薦）** | teach 維持第一格語意；常數/Start **預設 0**，公式形狀對齊 HT172。行為等價、tech.ini 不需重教 | 零遷移風險；**但前提**=現場 Start 真的是 0（見 ⚠️） |
| B | teach 改工位參考點 + 對 tech.ini 一次性換算 `new=old-Start*100-BIAS` | 高風險：符號/單位逐欄易錯；**換算須在 base 端**（`cprod.cpp:165/167/173` 已把 Offset 折入 `Teach.*`，誤在 effective 端會連帶縮放 offset）；需版本旗標防重複換算 |
| C | 上線重教 16+ 工位 + 重填 Start | 最乾淨、成本最高 |

⚠️ 審查修正：策略 A「Start 多半=0」前提**被 SECS/GUI 寫入路徑推翻**——`uHGemHT160.cpp:560-574`(S2F16 host 寫) / `:591-609`(GUI 寫 EC) 都能寫 EC2760/2761 並 `TrayForm.Save` 存進 setup.ini。**凡曾連上位機或用過 GUI EC 編輯器的機台，Start 可能已非 0**，P2 一上線即套偏移 → 整盤撞機。

---

## 5. 分階段（每階段可獨立編譯 + 驗證）

### P0 前置盤點與基線（不改碼）
- 掃描**所有** recipe 的 `data\<recipe>\setup.ini` 記錄 XStart/YStart/Pitch 實際值。
- ⚠️ 擴大：(a) 確認本機是否曾連 SECS host 或用 GUI EC 寫過 EC2760/2761；(b) 比對 State Record 內 `MachineConfig\system` 的 setup.ini（機台真實值，repo 與機台會 drift，見 [[io-table-compare-use-staterecord]]）。「host 曾寫非 0」列為策略 A 的**阻斷條件**。
- 跑 SOFT_SIMULATE selftest 記錄 SortArm pick/place + Loader CCD 的 ExpectX/Y 當回歸基線。
- 向機構/HT172 原作者查證 ±1000 (10mm) 意義 + 本機 Y 正方向。
- **驗收**：基線座標表 + 全 recipe Start 清單 + host 寫入確認。

### P1 Y 公式集中化（純重構，行為不變）
- 新增 `GetSortArmCellY`（暫不含 Start/常數），替換 **5 處** inline（含 :1120 ShowPlaceDebugInfo）。
- aSortArm.h 加宣告 → `-Full`。
- **驗收**：`-Full` EXIT=0；selftest 座標與 P0 基線**逐位元相同**；`CanMoveSuckerToCell`/`MoveSuckerToCell` 軟極限預檢路徑一併改到。

### P2 新增 getter + 注入偏移（常數/符號預設 0，等價落地）
- 新增 X/Y Start getter（aSortArm + aLoader）。
- `GetSortArmCellX` / `GetSortArmCellY` / `MoveToCcdCell` 注入 `BIAS + SIGN*Start`（全預設 0/+，Y 符號預設 0）。
- **驗收**：`-Full` EXIT=0；Start=0 時座標仍等於 P0 基線；手動把某 recipe XStart 設 5.00mm → 確認 X 整體 +500（1/100mm）、sim 不卡不踩軟極限；**SECS host 讀 EC2760/2761 仍回 mm 原值**。

### P3 真機驗證 +（條件性）語意遷移
- 交付 P2 由使用者真機驗證吸嘴對準格心、決定 Y 符號方向與 ±1000 是否賦值。
- 若業務確認要工位參考點語意 → 依策略 B/C 執行（高風險，需版本旗標 + base 端換算）。
- **驗收**：真機吸嘴落格心無 10mm 系統偏移。（sim 瞬時到位 + seeded CCD 不反映真實 tray 幾何，**sim 過 ≠ 對準**，須真機驗，見 [[confirm-compile-user-verifies-machine]]）

### P4 多吸嘴間距分離（延後 / 獨立里程碑）
- 啟用 suck2 以外吸嘴時才做：引入獨立吸嘴間距（移植 `AutoCalculateSortArmClosePitch*` + `IN_OUT_ARM_*_PITCH_MIN/MAX`），把 `GetSortArmCellX` 吸嘴補償從 `GetTrayXPitch` 拆出，同時確認 `CalculatePitchPosition` 仍讀對的量。
- 本機長期單吸嘴則可永久延後。

---

## 6. 風險清單

- **±1000 意義未證實**：照搬會偏 10mm → 具名常數預設 0，真機/機構圖確認後才賦值。
- **現場 Start 可能非 0（SECS/GUI 寫入）**：P2 上線即撞機 → P0 必須先全掃描 + 確認 host 未寫。
- **Y 符號盲抄**：本機 Y 遞增、172 遞減 → 符號常數預設 0，方向延 P3 真機定。
- **BottomCCD 原點不同步**：2D 複判抓相鄰格 → 誤判/誤踢料 → P0 先查證是否逐格掃描。
- **teach 換算誤在 effective 端**：`cprod.cpp` 已折 Offset 進 `Teach.*`，誤算會縮放 offset → 策略 B 換算只在 base 端 + 版本旗標。
- **:1120 漏改**：校位診斷漂移 → 把對的機台校歪 → P1 納入 5 處一起改。
- **sim 假陽性**：sim recipe 與現場 setup.ini 不同則 sim 過≠現場等價 → 真機驗。
- **Big5 源碼**：新註解 ASCII；用 `scripts/ops/bcb6-bytesafe-edit.ps1`（Edit 工具會 mojibake）。
- **改 .h**：必 `-Full`，否則 stale obj 連結錯。

---

## 7. 待你/機構決策的 Open Questions

1. **±1000 (10mm) 是什麼？** 工位參考點到 cell(0,0) 的固定機構偏置，還是 half-pitch/邊框？本機 teach 點是否已含此 10mm？（未答前 BIAS=0）
2. **是否要真的改 teach 語意（策略 B/C），還是只做公式對齊+保留可調 Start（策略 A）？** 屬業務/可調性需求。
3. **本機未來會啟用多吸嘴嗎？** 決定 P4 是否必要。
4. **本機 SortArm 確為單列吸嘴（comb 沿 X）？** 確認後才能安全省略 HT172 的 `iBaseSuckY/iRowPitch/3` 列補償。

P0 的盤點（現場 setup.ini 值、host 是否寫過、BottomCCD 是否逐格掃描、本機 Y 正方向）我可以直接動手查證——這些我能查的，先做不需等你。
