# 現場 (KYEC) 版本 vs repo 差異對照與整頓報告

- 日期：2026-07-03
- repo 端：`D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\`（分支 `feat/iosetview-172-refactor`，HEAD `918e149`）
- 現場端：`D:\HT160S_Program_BCB_V1.0.0.0_kyec\`（非 git，純快照；.cpp 時間戳 2026-07-02 15:53–16:19）
- 分析法：`diff -u` 逐 hunk + repo `git log -S/-G` 考古 + 6 個平行分析員交叉驗證

---

## 0. 方向結論（已用 git 確認）

現場快照是**從「已含我最新 cleanout 重構」的狀態複製出去**的：現場檔已包含 `8548420`（full drain cascade）、`321dae9`（adversarial-review edges）、`8bb7e63`（in-flight divert）這些 2026-07-02 上午的 commit，而現場檔時間戳是同日 15:53–16:19（在那些 commit **之後**）。

> 因此：**每一處差異都是現場工程師在最新 repo 之上「刻意改的」，不是現場落後。** 換句話說——現場的改動＝**針對我的 cleanout 重構在實機上遇到的問題所做的修正/退回**。這些改動是寶貴的實機 bug 線索。

差異範圍：僅 7 個原始碼檔（其餘全數位元相同，含 `aSortArm`/pick-retry/messbox/GeneralSetting）。

| 檔案 | 變更行(<>) | 性質 |
|---|---|---|
| `MachineType.h` | 2 | 預期差異（`SOFT_SIMULATE` 實機關），**不整併** |
| `aColor.cpp` / `aColor.h` | 45 / 3 | cleanout 完成判定改寫（旗標鎖存）＋ `status` 互鎖 |
| `aEmpty.cpp` / `aEmpty.h` | 46 / 3 | 同上（Empty） |
| `aLoader.cpp` | 30 | 關 MES0922、finish 加 `bLoaderhastray`、缺料改走 9500 |
| `aTrayArm.cpp` | 28 | rear-pick 退回 magic-70000、`status`/`LS_ToRear` 互鎖、init 改 false |

---

## 1. 決策總表（依「議題」跨檔分組）

| # | 議題 | 現場改法 | repo 現況 | 建議 | 風險 | 待你確認的實機症狀 |
|---|---|---|---|---|---|---|
| A | **Empty/Color CleanOut 完成判定**：即時運算 vs 旗標鎖存 | `IsCleanOutFinish()` 整段註解、改 `return bCleanOutFinish`（在 DoEmpty/DoColor 內鎖存） | 每次即時運算：TrayArm 完成 + 感測器無盤 + AMR 交接就緒 | **合併**（保留感測器/交接把關，OR 進 TrayArm-finish 鎖存）；不盲抄不盲棄 | 高 | 實機 CleanOut 是否曾「跑不完/收不了尾」？（懷疑 `RefreshStateFromSensors`/`IsReadyForAmrHandoff` 在 KYEC 機恆為 false） |
| B | **TrayArm↔進料防撞** (`status` 互鎖) | 進料中(`status==1`)時 TrayArm place case500 先 `break` 等待 | 無此互鎖 | **採用（需整清）**：這是真的防「盤疊盤」新硬化 | 中 | 實機是否出現過 TrayArm 在 Empty/Color 抬盤/進料時就下手放盤而撞盤？ |
| C | **TrayArm 取 Empty 後盤就緒判定**：退回 magic-70000 | 改回 `emptypos>70000 && (LeanOn||Push).IsOn()`，`IsRearReadyForPick()` 註解 | `IsRearReadyForPick()`（producer 擁有的就緒述詞，`8548420` 導入） | **已結案（2026-07-05，root-caused + superseded）**：誤判屬實，但誤判的是舊 `8548420` 版（`RefreshStateFromSensors` raw re-latch，車落位夾缸未放即回報 ready）；已由 `564154c`/`e7c0966`/`038d5bc`/`403e8e3`/`eec9ca9` 以狀態閘+latch 生命週期重寫修復。現場 magic-70000 手改**不採**（回程 pos≤70000 完全不擋、AMR 路徑無閘、re-teach 失效、reed 卡死無聲停機）。詳 `docs/plan/isrearready-itemc-deep-analysis-20260705.md` | 高→已收斂 | 上機驗證（分析報告 §6 八步清單） |
| D | **TrayArm 取 Loader 後盤**：加 `LS_ToRear` 閘 | Loader 側正移往後(`LS_ToRear`)時 TrayArm 先 `break` | `IsRearReadyForPick()`（`038d5bc` 起讀發佈式 `bRearReadyForPick` latch） | **否決（2026-07-05 於 HEAD 再驗證，維持 e7c0966 結論）**：`ReleaseSortOwner` 在 discharge 未 armed 前即設 `LS_ToRear`，排空側會停在 `LS_ToRear` 等 TrayArm 清後方——現場閘擋掉的正是唯一解鎖者→互等死結；其防撞意圖已由 latch（case100 re-arm→case4000 發佈）完整承接。**勿再移植此閘** | — | （結案，勿採） |
| E | **Loader CleanOut 收尾/缺料 + MES0922** | ① 關掉 MES0922「前殘料」告警 ② finish 加 `bLoaderhastray==false` ③ CleanOut 缺料改走 `FeedTask=9500` | `321dae9`/`8bb7e63` 的殘料告警 + phase-aware finish | **需你裁決**：MES0922 疑似實機誤報；②③可能才是正解 | 高 | 實機 MES0922 是否在沒有殘料時誤跳、擋住 CleanOut？ |
| F | **TrayArm `bCleanOutFinish` 初值** true→false | 初值改 false，但**全檔沒有任何地方再設回 true**（只有註解 TODO） | 初值 true | **暫留 repo**（現場此項未完成） | 中 | 實機開機是否曾出現「一開機就報 cleanout 已完成」的假象？ |
| — | `MachineType.h` `SOFT_SIMULATE` | 實機關閉 | 開發開啟 | **不整併**（刻意差異） | — | — |

---

## 2. 逐議題詳述

### 議題 A — CleanOut 完成判定：即時運算 vs 旗標鎖存（`aEmpty`/`aColor`）
- **現場做了什麼**
  - `aEmpty.h`/`aColor.h`：新增 `int status; bool bCleanOutFinish; bool bTrayFeedFinish;`（建構子初值 0/false/false）。
  - `DoEmpty`/`DoColor`：在 drain 分支把 `bCleanOutFinish=true` **鎖存**（Empty：`bLotFinish && 無前後盤` 時；Color：`RunMode==CleanOut && TrayArm->IsCleanOutFinish()` 時）。
  - `IsCleanOutFinish()`：**整段 repo 邏輯註解掉**，只剩 `return bCleanOutFinish;`。
- **repo 現況**：`IsCleanOutFinish()` 每次即時運算——`TrayArm 完成 + RefreshStateFromSensors() 無前後盤 + IsReadyForAmrHandoff()`（Color 另有 SortBin 模式「視為已完成」把關，來自 `321dae9`）。
- **差異影響**：現場鎖存版**丟掉了** `321dae9` 特意加的三道保護（RunMode 短路、前後盤感測、AMR 交接就緒），且鎖存一旦 true 不再回檢——盤又出現時不會 un-finish。
- **半成品跡象**：`bTrayFeedFinish` 宣告後全程未用（死碼）；Color 留有 `//if(tray arm clean out finish){...}` 偽碼殘跡。
- **建議**：`merge-both`。若確認實機有「即時述詞恆不為 true → CleanOut 收不了尾」的真實 hang，正解是**保留 repo 的感測/交接把關，再 OR 進 TrayArm-finish 鎖存當額外通路**；丟棄 `bTrayFeedFinish`。

### 議題 B — TrayArm↔進料防撞 `status` 互鎖（`aTrayArm` + `aEmpty`/`aColor`）
- **現場做了什麼**：`DoPlaceToEmpty`/`DoPlaceToColor` case 500 起始加 `if(EmptyModule->status==1) break;` / `if(ColorModule->status==1) break;`；`status` 於 `DoEmpty`/`DoColor` 進料開始設 1、完成設 0。
- **意義**：進料模組正在抬盤/進料時，TrayArm 不要下降放盤 → 防「盤疊盤」對撞。**這是 repo 沒有的、真的防撞硬化。**
- **問題點**：
  1. **跨檔相依**：要採用就必須連 `aEmpty.h/.cpp`、`aColor.h/.cpp` 的 `status` 一起移植，TrayArm 單獨拉無法編譯。
  2. **latent NULL-deref bug**：`ColorModule->status` / `EmptyModule->status` 在同段稍後的 `==NULL` 檢查**之前**就被解參考（實務上 module 不會是 NULL，但仍應把 NULL 檢查前置）。
  3. Color 端 `status` 幾乎只寫不讀（一致性待整）。
- **建議**：`採用但整清`——把 `status` 語意統一為「進料進行中」旗標、修 NULL 檢查順序、Empty/Color/TrayArm 一起改。

### 議題 C — TrayArm 取 Empty 就緒判定退回 magic-70000（`aTrayArm`）

> **狀態更新 2026-07-05：已結案。** 深度分析（`docs/plan/isrearready-itemc-deep-analysis-20260705.md`）以 git 三重證據確認：現場撞機時跑的是舊 `8548420` 版述詞（`RefreshStateFromSensors` raw re-latch），KYEC 機**從未跑過** 07-03 之後的狀態閘重寫版；現行 repo 版在所有可達狀態嚴格優於現場 magic-70000 閘（該閘在 pos≤70000 回程運送段完全不擋）。P0 兩洞（Loader post-reset 死結、stale-TRUE latch）已於 `eec9ca9` 修復。以下原始分析保留作歷史紀錄。
- **現場做了什麼**：`DoPick` case1/10、`Job==TAJOB_EMPTYTRAY_TO_AUTO` 分支，把 `if(EmptyModule->IsRearReadyForPick()==false) break;` **註解掉**，改回 `if(emptypos>70000 && (C_Empty_LeanOnTray.IsOn()||C_Empty_PushTray.IsOn())) break;`。
- **意義**：這是**直接退回 `8548420`**。現場八成發現 `IsRearReadyForPick()` 在實機誤判（TrayArm 太早取 / 不等），退回已知可動的具體 encoder+缸況判斷。
- **注意**：`IsRearReadyForPick()` 現在是跨模組的就緒「契約」（Loader 側 recovery 也用它），只在 Empty 這條退回會造成兩套判定並存不一致。
- **建議**：`需裁決（優先）`。請先釐清 `IsRearReadyForPick()` 在實機的實際誤判情形——若屬實，應把述詞本身修對（或在 Empty 這條採現場判斷並同步檢視 Loader 側）。

### 議題 D — TrayArm 取 Loader 加 `LS_ToRear` 閘（`aTrayArm`）

> **狀態更新 2026-07-05：否決（勿採）。** e7c0966 的否決理由在 HEAD 重新驗證仍成立：`ReleaseSortOwner` 於任何批次釋放時設 `LS_ToRear`（aLoader.cpp:681 附近），`DoLoader` case 3000 讓排空側在後方被佔時停在 `LS_ToRear` 等 TrayArm 清後方——現場加的閘擋掉的正是唯一能解鎖它的 actor，形成互等死結。其防撞意圖已由 `bRearReadyForPick` latch（case 100 re-arm → case 4000 發佈，`038d5bc`/`eec9ca9`）完整承接。下方原「建議採用」文字作廢，保留作歷史紀錄。
- **現場做了什麼**：`Job==TAJOB_LOADER_RECOVERY` 分支，在 `IsRearReadyForPick()` 之後**再加** `if(LoaderModule->GetLoaderStatus(0)==LS_ToRear || GetLoaderStatus(1)==LS_ToRear) break;`。
- **意義**：加成式互鎖——Loader 某側正移往後時，TrayArm 不要去取 Loader 後盤，防對撞/早取。`GetLoaderStatus`/`LS_ToRear` 於現行 repo `aLoader.cpp` 已存在，**可直接編譯**。
- **建議**：`採用`（低-中風險，合理硬化）。

### 議題 E — Loader CleanOut 收尾/缺料 + MES0922（`aLoader`）
- **現場做了什麼**（三處，全繞著前進料感測 `SnLoader_InputHasTray`）：
  1. **關掉 MES0922**「Loader front residual tray, please remove」整段操作員告警（`#ifndef SOFT_SIMULATE` 內，`321dae9` 導入）。
  2. phase-aware finish guard 加 `bool bLoaderhastray = (!Enable) || IsOn();`，條件多一項 `&& bLoaderhastray==false`。
  3. `DoFeedTray` case 9000 else（CleanOut 供料車乾）由單純 `break` 改為：sim / 感測 disable / 感測 ON 時 `State->FeedTask=9500; break;`（導向 InputHasTray 確認案例）。
- **意義**：MES0922 疑似在**沒有殘料時誤報**、卡住 CleanOut；②③（真正查 `SnLoader_InputHasTray`）可能才是正確收尾邏輯，讓 MES0922 變得多餘或需改對。
- **建議**：`需裁決`。先確認 MES0922 實機誤報情形；②③大機率可採，MES0922 應改成「用同一顆感測正確判定」而非直接刪。

### 議題 F — TrayArm `bCleanOutFinish` 初值 true→false（`aTrayArm`）
- **現場做了什麼**：建構子與 `InitialFlag()` 把 `bCleanOutFinish` 初值改 false；case100 留 `//bCleanOutFinish=true;` 的 TODO 但**未實作**。
- **意義**：`IsCleanOutFinish()` 在 `RunMode!=Run_CleanOut` 時回傳此旗標；改 false 後、又沒地方設回 true → **此改動未完成**，可能讓非 CleanOut 情境的完成回報錯亂。
- **建議**：`暫留 repo`（true）。除非實機確有「一開機就假性 cleanout-finish」的問題，否則不採此半成品。

---

## 3. 建議整併流程（待你決定後執行）

1. 你先就議題 A/C/E（高風險、疑似實機 bug）補上**實機症狀**：到底哪個畫面/動作卡住、告警是否誤報。這決定「修述詞 vs 採現場」。
2. 低風險先落地：議題 D（`LS_ToRear` 閘，可直接編譯）。
3. 議題 B（`status` 防撞）整清後跨檔一起移植（含 NULL 檢查修正、丟死碼）。
4. 議題 A：以 merge-both 收斂（感測把關 OR TrayArm-finish 鎖存）。
5. 議題 E：以「正確判定 `SnLoader_InputHasTray`」重寫 MES0922，而非刪除。
6. 議題 F：預設不動；視實機決定。
7. 每次改完依規則刪 obj → `build-ht160s.ps1 -Full`（動到 header）→ EXIT=0；動到 `SOFT_SIMULATE` 守護碼則另跑實機建置驗證。
8. `MachineType.h` 全程保持開發 `SOFT_SIMULATE` 開啟，**不要**把現場的關閉版本併回。

---

## 附註：現場半成品/死碼清單（採用時一併清掉）
- `bTrayFeedFinish`（Empty/Color）：宣告+初值，全程未用。
- Color `status`：寫多讀少（`if(status==0)status=1;` 屬自癒 no-op）。
- TrayArm `//bCleanOutFinish=true;`、Color `//if(tray arm clean out finish)`：偽碼殘跡。
- TrayArm place case500：`Module->status` 於 NULL 檢查前解參考（latent bug）。
