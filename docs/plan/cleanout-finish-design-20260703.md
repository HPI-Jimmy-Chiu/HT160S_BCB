# CleanOut 完成判定 — 物料流模型與收斂設計

- 日期：2026-07-03　依據：使用者現場設計規格（on-site 版本仍有 bug、非正解參照）
- 來源：6 模組平行建模 + Tray-feed 稽核 workflow（`wf_a7c01dfc`）+ 人工交叉核對
- 適用分支：`feat/iosetview-172-refactor`

---

## 1. 物料流 input/output 模型

**搬運分工**：**IC 由 SortArm 搬**（Loader 盤 → Auto 盤）；**盤由 TrayArm 搬**（Loader/Empty/Color 後方 ↔ Auto 後方 / Empty/Color 回收）。各模組另有自己的堆疊缸做 GoUp/GoDown（在模組內部升降）。

| 物件 | INPUT（進料） | OUTPUT（出料） |
|---|---|---|
| **Loader** | 盤：供料車堆疊區 `SnLoader_Inputend`（人工/AMR 補）→ 前分張 | **IC**：掃描後盤在 sort 位，被 **SortArm** 吸走；**盤**：後方 `OutputBottom` 空盤，被 **TrayArm** 取走 |
| **SortArm** | **IC**：Loader 盤（4 nozzle 逐格吸） | **IC**：放入 **Auto** 盤格（SortArm 自己放） |
| **Auto** | 盤：後方 staging（**TrayArm** 送來）＋ **IC**：working 盤（**SortArm** 放） | 盤：整盤（含 IC）GoUp 到**輸出堆疊車** `Car[]`（人工/AMR 取）。**Auto 不取 IC** |
| **TrayArm** | 盤：Empty 後方 / Color 後方 / Loader 後方 | 盤：Auto1-6 後方 staging / Empty 後方回收 / Color 後方回收 |
| **Empty** | 盤：前供料車堆疊區 → 前分張 | 盤：後方 pick 位（**TrayArm** 取）／GoUp 堆回供料車 |
| **Color** | 盤：前供料車（identity 盤）＋ 後方（**TrayArm** 送回） | 盤：後方 pick 位（**TrayArm** 取）／GoUp 堆回前車 |

**待確認的模型要點**（見 §4 提問）：
- Auto 是**收集/輸出端**——IC 只進不出（SortArm 放進來），最後**整盤含 IC 出到堆疊車**。故通則「所有 IC 被吸走」**不適用 Auto**；Auto 的「清空自己」＝**軌道上所有盤都 GoUp 出到車**。
- Empty/Color 的「GoUp 到堆疊區」＝堆回**前方供料車**（來源即堆疊區）。

---

## 2. CleanOut 完成串接（✅ 使用者已確認 2026-07-03 — 未來開發以此為基準）

```
1. Loader (L+R 兩側：源 Inputend 乾 + IC 全被吸走 + 無虛擬盤
           + 前/後 sensor 熄 + 後盤已被 TrayArm 收 + 兩側 Idle)
  └► 2. SortArm   (Loader 完成 + 手上無 IC + Pick/Place 停格
                   + 無重吸/吸錯待處理 + ★吸嘴全部在上方 Home)
        └► 3. Auto ×6 (SortArm 完成 + 後方殘盤自收進 working
                       + 軌道盤全 GoUp 出堆疊車 + Idle) ⚠Full 閘
              └► 4. TrayArm (Loader＋Auto 完成 + 手上盤送達有效目的地
                             + Z 回上方 + 無在途工作 + Idle)
                    └► 5. Empty / Color (TrayArm 完成 + 盤全 GoUp 回供料車
                                         + 前/後皆空 + 前缸回 Home + Idle) ⚠Full 閘
```

**通則**：完成 ＝ ①需求/上游已完成 ＋ ②自己物料清空到定位 ＋ ③自己 Idle（手上工作做完、非停半途）。全部**即時運算**——盤中途再出現會自動取消完成再收一次。
**★ 使用者補充 (2026-07-03)**：SortArm 的 Idle 明確包含「所有啟用吸嘴 Z 在上方 Home sensor」（`AreAllSuckersHome()`，sim-true）——已實作進 `IsCleanOutFinish()`。
**⚠ Full 閘**：Auto 輸出車×6 / Empty 供料車 / Color 供料車（Loader 依裁決不加）；Full 亮→暫停 GoUp+modal 通知清空，熄才續跑/完成。

---

## 3. 各模組完成判定：現況 vs 應改（標出缺的 code hook）

圖例：✅已有　➕需新增（hook 已存在，只是判定沒查）　⚠️無 hook 要建

### Loader `IsAllCleanOutFinish()` (`aLoader.cpp:685`)
- ✅ 兩側 `Side[0].bCleanOutFinish && Side[1].bCleanOutFinish`（L＋R 都要，符合你的補充1）
- ✅ (REAL) `SnLoader_InputHasTray/OutputBottomHasTray/Inputend` 皆 OFF
- ✅ `bRearHasTray==false`（TrayArm 已收後盤）
- ➕ 兩側 `TrayMotor->fHasTray==false`（無虛擬盤，目前只在 DoLoader guard 查、predicate 自身沒查）
- ➕ 兩側 IC 全吸走 `ActiveTrayAllData(No,EMPTY_IC)` / `HasPickableIC()==false`（目前無明確斷言）
- ➕ 兩側 `Side[].Status==LS_IDLE` + `iYOwner==NONE` + `bAmrLocked==false` + `bRearDischargeInProgress==false`（防停半途/仍持 sort-Y）
- 註：sim/DUMMY 無 IO 卡會跳過 sensor 區，須靠軟體 latch（fHasTray/EMPTY_IC/Status）判定，務必補齊。

### SortArm `IsCleanOutFinish()` (`aSortArm.cpp:1654`)
- 現況：只 `return bCleanOutFinish`（一次性 latch，於 DoSortArm case1 設定），**可能 stale-true**（設 true 後又被晚到的 IC / residue 重吸拉回工作）
- 應改為**即時運算**：`Loader->IsAllCleanOutFinish() && !HasHoldingIC() && PickTask==1 && PlaceTask==1 && !IsResidueCheckBusy() && !HasPickSuckError()`（所有 hook 皆存在）
- 註：SortArm 無 tray、通則的盤/GoUp/Z 條款不適用。

### Auto `IsAllCleanOutFinish()` (`aAuto1To6.cpp:877`)
- 現況：只檢查每站 `State[].bCleanOutFinish`，而該 flag 在 case7000 **無條件設 true**（只是「drain ladder 跑完」latch，未驗證殘料）
- 應改為每站即時驗證：➕上游 `SortArm->IsCleanOutFinish()`（或直接 `Loader->IsCleanOutFinish()`，見提問）＋ ➕`GetAutoVMotor(i)->fHasTray==false`（軌道無盤）＋ `State[].bRearHasTray==false && bRearDeliveredPending[i]==false` ＋ (REAL) rear sensor OFF ＋ `GetFrontRise(i)->GetOutBit()==false`（前缸回 home）＋ ➕idle `FeedTask==1 && DischargeTask==1 && CleanOutTask==1`
- 註：sim 早退（RefreshAutoState / IsDrainedForAmr）要保留，否則 laptop 永不完成。

### TrayArm `IsCleanOutFinish()` (`aTrayArm.cpp:80`)
- ✅ **現況已幾乎完全符合你的 (b)**：`Loader完成 && Auto完成 && HasTray()==false && Job==TAJOB_NONE && IsZUpAtPosition()`（即時運算）
- 維持不變；**不採現場**的 `bCleanOutFinish` 初值 true→false（那是半成品）
- 「停在一半」的真因不在此 predicate（它本來就不會在持盤時報完成），而在**下游 Empty/Color 提早完成 / 沒接手** → 見下

### Empty `IsCleanOutFinish()` (`aEmpty.cpp:898`)
- ✅ `TrayArm->IsCleanOutFinish()`（上游）
- ✅ `bFrontHasTray==false && bRearHasTray==false`
- ✅ `IsReadyForAmrHandoff()`（前缸 home；sim 恆 true）
- ➕ **idle gate（缺，關鍵）**：`FeedTask==1 && GoDownTask==1 && GoUpTask==1 && bReturnTray==false && bRearReturnInProgress==false` — 否則 GoUp/Feed 還在跑時 latch 一 false 就誤報完成 → **這是 TrayArm 停半途的主因**
- ➕ `MMEmptyY->fHasTray==false`（虛擬盤）

### Color `IsCleanOutFinish()` (`aColor.cpp:1146`)
- ✅ `IsInstalled()` / `IsTraySupplyMode()` 短路、`TrayArm->IsCleanOutFinish()`、`bFrontHasTray/bRearHasTray==false`、`IsReadyForAmrHandoff()`
- ➕ **idle gate（缺）**：`FeedTask==1 && GoDownTask==1 && GoUpTask==1 && bReturnTray==false`
- ➕ `MMColorY->fHasTray==false`（虛擬 identity 盤）

**共同修法原則**：全部**保留即時運算**、補「idle + 虛擬盤 fHasTray」條款；**丟棄**現場的一次性 latch 與死碼（`bTrayFeedFinish`、cleanout 用途的 `bCleanOutFinish`、Color 的 `status`——`status` 屬議題 B 防撞，另處理）。

---

## 4. Tray Feed 入口稽核（此機無 Tray Feed 功能）

- **Run mode**：`Run_TrayFeed=4`（`database.h:254`）。**唯一進入點** `ChangeRunMode(Run_TrayFeed)`（`csystem.cpp:1409`）**已整段註解** → 任何 live 路徑都進不去。
- **消費端全數不可達**：`csystem.cpp:1438`（`CheckAllTrayFeedFinish()` 恆 false stub）、`csystem.cpp:953`（狀態燈顯示）、`database.cpp` ×7（速度/enable gating）、log helper。
- **`bTrayFeedFinish` / `IsTrayFeedFinish()`**：`aTrayArm` 內宣告+寫+讀，但 `IsTrayFeedFinish()` **零呼叫者**（全樹只有定義）。
- **按鍵/面板 plumbing**：`K_TRAY_FEED=0x0004`、`BtnTrayFeed`、`SwFK/RKTrayFeed`、Pad `PAD_TrayFeed`、SECS `PressTrayFeed(10)/TrayFeedOK(29)`、`svRunMode '4=TrayFeed'` — 都連著但**無 live KeyCode 會提供給 operator**。
- **處置建議**：全屬死碼但無害。建議**保留** enum、log helper、IOsetview 診斷 LED、SECS SV 對映（維持穩定）；可選擇**清掉** `bTrayFeedFinish`/`IsTrayFeedFinish()` 這對純死碼（見提問）。

---

## 4b. 使用者補充待辦（2026-07-03，排在 part2 之後）

1. **統一 Status enum**：所有模組（Auto 流道、SortArm、TrayArm、Empty、Color）比照 Loader `LS_*` 建立明確 status 狀態；完成判定與互鎖改讀 status，取代「sub-task 全==1」的拼湊 idle。
2. **TrayArm↔Loader 夾盤干涉 bug（實機回報）**：TrayArm 於 Loader 後方夾盤時，Loader 載台（MotorY）前後夾缸尚未釋放 → 干涉。需加互鎖：夾缸確認釋放後 TrayArm 才可夾走（查 C_Loader* clamp out-bit；可能落點 IsRearReadyForPick 或 DoPick TAJOB_LOADER_RECOVERY 閘）。
3. **TrayArm 順路補 Auto（效率）**：TrayArm 夾空盤往 Empty 途中、放到 Empty rear 之前，若 Auto 有盤需求 → 直接轉送 Auto，不放下重夾。實作形狀鏡射 aTrayArm.cpp:505 的 drain-boundary divert（方向相反 Empty→Auto），僅限 Auto 可收的盤種。

## 5. 待使用者確認（(a) 精修提問）
1. **Auto 上游依賴**：Auto 真正要等的是 **SortArm 放完 IC**（SortArm 已依賴 Loader），比「等 Loader」精確。採「Auto ← SortArm ← Loader」？
2. **Auto 物料語意**：Auto 不吸 IC、是收集端，「清空自己」＝**整盤(含IC) GoUp 出到堆疊車**。確認？
3. **殘留 IC**：Loader/Auto 盤若殘留 IC（吸取失敗/operator skip），finish 要**卡住到 IC 全清**，還是允許「operator 手動移除」後才算？
4. **Tray Feed 死碼**：只清 `bTrayFeedFinish`/`IsTrayFeedFinish()` 這對純死碼、其餘無害保留？還是全部先不動？
