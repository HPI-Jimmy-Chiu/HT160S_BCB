# Item C 最終深度分析報告 — TrayArm 取 Empty 後盤就緒判定 IsRearReadyForPick() 現場疑似誤判

> **狀態附錄 (2026-07-05)**: §5.1 的兩個 P0 洞已實作修復並通過 sim+real `-Full` 雙組建 EXIT 0 與 4-lens 對抗性審查（未 commit）:
> - **P0-1 post-reset 死結**: `TLoaderModule::InitialFlag` 守衛式保留（`bRearReadyForPick && IsOutputBottomOccupied()` 時保留 latch+RearKind/RearTrayID/RearSourceTray; ctor 先硬歸零）; 不可保留的殘盤（冷啟動/中途中斷）改走新告警 **MES0924**「Loader rear has a leftover tray」(鏡像 MES1426, sensor-aware, 每 episode 一次, 已註冊 registry)。
> - **P0-2 stale-TRUE latch**: `RefreshRearState` sensor 空邊沿清 latch; re-arm 點從 case 2000 提前到 `DoDischargeTray` case 100 commit（同時 arm `bRearDischargeInProgress`）。
> - **審查加修 2 個 MEDIUM 下游洞**: TrayArm `DoTrayArm` case 1000 job-abandon（recovery source 空 + PickTask<1000 → 釋放手臂, 防 CleanOut 無聲卡死至 HOME）; `DoPick` case 4000 重讀 `iDeliverKind`（防釘住的 job 撿到下一片 identity 盤誤送）。
> - **接受殘留**: rear sensor Enable=false 組態全盲（pre-existing）; 單次 sensor flicker 可誤發 MES0924（無 debounce）; §5.2 觀測欄位與 P1 blocked-pick watchdog 仍為部署前建議項。


---

## 1. 一句話結論

**誤判屬實，但誤判的是舊版述詞（commit 8548420，2026-07-02 上午出貨的版本），根因已於 2026-07-03 在 repo 診斷並以四層狀態閘重寫修復（564154c + 1a43387 + 038d5bc + 403e8e3）；現場退回的 magic-70000 編碼器閘「在事故窗口內判定正確」但自身帶有 pos≤70000 完全不擋的碰撞窗口、AMR 路徑全無防護、re-teach 失效、reed 卡死無聲停機等缺陷，不採用；裁決建議：以 repo 現行版覆蓋現場手改版，但須先修 Loader latch 生命週期兩個殘洞並補 Empty 觀測欄位，再依第 6 節清單上機驗證。** 重要前提：現行四層閘全套**零實機時數** — KYEC 機器從未跑過任何一個 07-03 的修復 commit。

---

## 2. 誤判的完整因果鏈（時間軸）

### 2.1 現場當時跑的是哪個版本（git 證據）

| 時間 (2026) | 事件 | 證據 |
|---|---|---|
| 07-02 09:18:30 | **8548420** `fix(cleanout): full drain cascade` 出貨，內含 Empty 述詞初版 | `git show 8548420:...aEmpty.cpp` |
| 07-02 ≤16:19:24 | KYEC 快照定格（最新檔 `aLoader.cpp` 16:19:24） | `D:\HT160S_Program_BCB_V1.0.0.0_kyec\` 檔案時戳 |
| 07-02/03 | **現場發生 TrayArm 撞入干涉**；現場工程師退回 emptypos>70000 閘、註解掉述詞呼叫 | kyec `aTrayArm.cpp:413-421` |
| 07-03 11:26:46 | **564154c** Empty 述詞重寫（state-based，實機干涉修復）— 注意 `git log -S "IsRearReadyForPick"` **抓不到**此 commit（只改函式本體，未增刪含該字串的行），已用 `merge-base --is-ancestor` 驗證為 HEAD 祖先 | 564154c diff |
| 07-03 11:55:24 | **e7c0966** Loader 三層版（同類修復），commit message 明確**否決**現場 LS_ToRear 閘 | e7c0966 |
| 07-03 16:06-16:07 | **403e8e3**（AMR 路徑補第三閘；3dcf777 為 shared-worktree 重複雜湊，非 HEAD 祖先）+ **038d5bc**（Loader 改 published latch，修 e7c0966 自己的 ≤4000 off-by-one 死結） | 403e8e3 / 038d5bc |

三重獨立證據確認機器跑的是舊版：(a) 所有重寫 commit 晚於快照最新檔 19+ 小時；(b) kyec `aEmpty.cpp:939-943` 逐字等於 8548420 本體；(c) kyec `aTrayArm.cpp:420-421` 該述詞呼叫被註解 — 快照時機器在 EMPTYTRAY_TO_AUTO 路徑上**連舊述詞都沒在呼叫**了。

### 2.2 舊版誤判的精確機制

舊述詞（8548420 = kyec `aEmpty.cpp:939-943`）：

```cpp
bool TEmptyModule::IsRearReadyForPick()
{
    RefreshStateFromSensors();
    return (bRearHasTray && bRearReturnInProgress==false);
}
```

物理時序（repo `aEmpty.cpp` DoFeedTray 階梯）：

1. **case 4000**（`aEmpty.cpp:429-435`）：載送車**夾著盤**移到後方 discharge Y（tech.ini 教點 836.00mm=83600）。**盤一落位，`SnEmpty_OutputBottomHasTray` 原始 sensor 即亮**。
2. `RefreshStateFromSensors()`（`aEmpty.cpp:182`）在實機上**無條件把 raw sensor 回寫進 `bRearHasTray`** → latch 提前變 true。
3. 但夾缸要到 **case 5000**（PushTray Pop，`:437-440`）、**case 6000**（LeanOnTray Pop，`:442-445`）才依序放開；正確的 latch 點在 **case 7000**（`:465`）。
4. `bRearReturnInProgress` 只涵蓋回程（DoGoUpTray）窗口，不涵蓋**送達**窗口 → 述詞在「車已到位、夾缸仍夾持」時回報 ready。
5. TrayArm DoPick case 1/10 閘通過 → Z-down 衝入 → **與仍夾著盤的載送車干涉**。這就是 Item C 事故。

### 2.3 現場工程師的退回邏輯與語意

kyec `aTrayArm.cpp:413-419`：

```cpp
int emptypos=HSys.Mot.MEmptyY->ReadEncoderPos();
if(emptypos>70000 &&
   (HSys.Cyn.C_Empty_LeanOnTray.IsOn() ||
    HSys.Cyn.C_Empty_PushTray.IsOn()))
{
    break;
}
```

語意：「車在後段（>700.00mm，接近 discharge 836mm）**且**任一運輸夾缸 reed sensor 回報伸出 → 擋」。`IsOn()` 是**物理 reed 回授**（kyec `mycylin.cpp:173-176`），非命令位。這確實精準命中了事故窗口（車在高位＋夾缸夾持 → 擋；夾缸放開後即使車還停在下方 → 放行，物理上安全 — 車床面在盤下方，手臂從上方抓）。現場另外在 Loader-recovery 分支加了 LS_ToRear 閘（kyec `aTrayArm.cpp:433-437`），並在 Empty/Color 加了手工 `int status` busy 旗標 — 這正是 repo 後來正式化成 `ES_FEEDING/ES_RETURNING` Status enum 的雛形，但 kyec 版是 ctor-only 初始化、且在 NULL check 之前解參考（kyec `aTrayArm.cpp:672` vs `:677`）。

### 2.4 為何 sim 沒抓到

- `RefreshStateFromSensors()` 在 SOFT_SIMULATE/DUMMY **early-out**（`aEmpty.cpp:156-157`），raw re-latch 那行根本不執行；sim 的 latch 是在**正確步驟**（case 7000）由階梯設定的 — 所以 sim 裡述詞行為是對的，bug 只在實機路徑存在。
- 即便誤判發生，sim 也撞不出來：`MoveEmptyY` 防撞閘 `#ifndef SOFT_SIMULATE`（`aEmpty.cpp:196-205`）、`IsZUpAtPosition` 強制 true（`aTrayArm.cpp:121-125`）、`TMyCylinder::Push/Pop` 直接 return true（`mycylin.cpp:199-200, 301-302`）。sim 對「false-ALLOW（碰撞類）」回歸**結構性全盲**（詳第 4 節 lens 4）。

---

## 3. 現行 repo 版 vs 現場退回版 — 逐狀態不一致矩陣

現行 repo 述詞（`aEmpty.cpp:988-1005`）四層閘：(a) `Status==ES_FEEDING||ES_RETURNING` 擋；(b) `bRearHasTray==false || bRearReturnInProgress` 擋；(c) `FeedTask` 非 1/13000 擋；(d) `C_Empty_LeanOnTray/PushTray.GetOutBit()`（**命令位**）任一 true 擋。

| # | 狀態 | 現場閘 | repo 四層閘 | 物理正確方 |
|---|---|---|---|---|
| 3 | **事故窗口**：車落位 discharge（83600）、夾缸夾持、FeedTask 4000/5000 | 擋 | 擋 | **一致**（只有舊述詞在此放行）|
| 5 | 正常交接：車停 discharge、夾缸已放、FeedTask 13000/1 | 放行 | 放行 | **一致**，安全（夾缸放開才是真正安全條件）|
| 6 | 回程在 discharge 重夾（CleanOut/return）| 擋 | 擋 | 一致 |
| 7 | **回程運送中（<70000）**，夾缸夾著盤往前運 | **放行（pos<70000 整個條件為假，完全不擋！）** | 擋（`bRearReturnInProgress` + ES_RETURNING）| **repo** — 現場閘唯一真實安全破洞：已派工的 pick job 可 Z-down 到正被抽走的後座 |
| 4 | 夾缸已放、case 7000 底部確認未完成 | 放行（早開約 1 步，與 MES1021 缺盤檢查競速）| 擋 | repo |
| 8 | 車停 discharge、reed **卡死 ON**（故障）| **永久擋 → 無聲停機無告警** | 放行（GetOutBit 命令位=off）| repo（可用性；違反 silent-stop-must-notify 的是現場版）|
| 9 | discharge 教點改到 <700mm 後車落位夾持 | **放行（碰撞回歸！）** | 擋（狀態式、與教點無關）| repo |
| 10 | SOFT_SIMULATE | 永久擋或恆放行（InType 接線決定）| 正確（命令位在 sim 有效）| repo |
| — | AMR 模式 cover/normal 從 Empty 後方取盤 | **kyec 完全無閘**（只分支 EMPTYTRAY_TO_AUTO / LOADER_RECOVERY）| 有閘（403e8e3，`aTrayArm.cpp:407-421`）| repo |

另兩處現場版缺陷：`HSys.Mot.MEmptyY` 無 NULL check 直接解參考（kyec `aTrayArm.cpp:413`）；硬編碼 70000 與 `Teach.EmptyCarDischargeTrayYPosition` 脫鉤。

現場閘唯一「理論上比 repo 嚴」的狀態：夾缸在 OffSensor 確認放開**之後**因閥/氣路雙重故障自發再伸出 — reed 會擋、命令位擋不到。此為 speculative 級硬體雙故障；階梯層 Pop() 在實機本就等 OffSensor 確認才推進（單故障模型下 repo 的放行有據）。

**結論：repo 版在所有可達狀態嚴格更安全，且在事故定義的兩個狀態（擋夾持落位、放行已釋放停車）與現場閘判定一致 — 現場退回是有效的點修，但不是可留用的方案。**

---

## 4. 對抗性驗證結果（四個 lens）

### Lens 1 — FALSE-READY（提前取盤 → 物理碰撞，Item C 同類）

**真 hole（confirmed-in-code）：**

- **[HIGH] Loader stale-TRUE latch**：discharge 完成後（`aLoader.cpp:1811` 發布 true），若操作員**手動移走後方盤**（sensor OFF → `RefreshRearState` 清 `bRearHasTray`，但唯一消費端清除點 `NotifyTrayArmPickRearTray`（`:725-735`）沒跑），`bRearReadyForPick` 殘留 true。下一次 discharge case 1000 盤一落位（夾缸仍夾），`IsRearOccupied()&&bRearReadyForPick`（`:608`）即為 true — case 2000 的 re-arm-false（`:1788`）還沒執行 → **重演 Item C 同類碰撞窗**（窗口窄但每次 discharge 落位都重播）。
- **[MEDIUM] Empty case-10 短路停格**：DoFeedTray case 10 見 `bRearHasTray` 已 true 直接 `return true` 不改 FeedTask（`aEmpty.cpp:399-404`）→ FeedTask 停 10、Status 停 ES_FEEDING → 述詞永久 false（此洞屬 false-BLOCK，詳 lens 2）。

**Plausible-needs-machine-check：** TrayArm `fHasTray` 晚一整段階梯才 latch（`aTrayArm.cpp:477-479`），夾持後 Z-up 中報警 → HOME 走無盤分支開夾（`uHome.cpp:481-484`）→ 盤掉落；DUMMY 模式下 DoPlaceToEmpty 放盤閘被 `IsSoftSimulate()` 短路（`aTrayArm.cpp:703-718`）。

**Speculative：** 程式 crash-restart 後 OutValue 重建為 false 而 MN200 輸出可能保持激磁（`aEmpty.cpp:1002-1003` 閘 (d) 失義）— 有 HOME 前置緩解。

**明確排除（clean）：** Empty 四層閘在 REALLY 機上每個物理不安全狀態至少被兩層擋住，含 alarm-abort（out-bit 不被 InitialFlag 清）與 post-HOME 復原；`IsPickFromColor`/`GetPickSourceX` 用同一述詞分支，無 job 可未閘抵達 Empty X；PickTask=1000 提交後無第二 actor 可回收後座；Color `bTrayReady` 純階梯 latch 乾淨。

### Lens 2 — FALSE-NOT-READY（卡 false 死結 — 現場失去信任的方向）

- **[CRITICAL, confirmed-in-code] Loader post-reset 死結**：盤停在 Loader 後方待取時發生 HOME / 程式重啟 / OneCycle 結束（`csystem.cpp:1028/1346/1424`；`CheckOneCycleFinish` 只看 SortArm idle，`:1486-1493`）→ `InitialFlag` 清 `bRearReadyForPick=false` 並抹掉 RearKind/RearTrayID（`aLoader.cpp:63, 70-72`）→ 重啟後 sensor re-latch `bRearHasTray=true`，但**唯一 set-true 點 case 4000 不可達**（case 10/100 都等後方淨空，`:1743-1763`）→ priority-1 TAJOB_LOADER_RECOVERY 把整支 TrayArm 釘死在 DoPick case 1/10（`aTrayArm.cpp:430-431`），全機供料鏈餓死、**零告警**。這正是 038d5bc 聲稱要消滅的「終態不可達-ready」類 — 修復把死結從「每次 discharge 完成後」移到了「任何帶後盤的 reset 後」。Empty 側的負向 Status 閘設計理由（`aEmpty.cpp:991-995`）沒有移植到 Loader latch。uHome 持盤提示不涵蓋此盤（只查車上 fHasTray，`uHome.cpp:642-652`）；Color 有 MES1426 殘盤告警（`aColor.cpp:804-814`），Loader 沒有。
- **[HIGH, confirmed-in-code] Empty case-10 停格**（同 lens 1 第二項的 false-BLOCK 面）：後方 sensor 在派工~case-10 的 ~2 tick 窗口內 OFF→ON 抖動（髒污/振動 reed，或人工塞盤）→ FeedTask 停 10 + Status 停 ES_FEEDING，閘 (a)(c) 永久 false，唯一解法是沒人知道要做的 HOME。修法方向：case-10 早退應停在 FeedTask=13000（+Status=ES_REAR_READY）或比照 Color 告警。
- **[MEDIUM] 所有 rear-ready 等待無界且無聲**：DoPick 三閘與 Loader discharge hold 皆無 timeout/Note/EventLog（違反 silent-stop-must-notify）— 操作員看到的是「看起來健康卻不生產的機器」，**正是導致現場斷定新閘壞掉而手動退回的感知環境**。
- **[INFO]** SOFT_SIMULATE 下閘 (d) 兩個方向都不可測（Push/Pop bypass）。

**明確排除：** Empty post-HOME 復原真的可行（InitialFlag → FeedTask=1/ES_IDLE + sensor re-latch + 負向 Status 閘接受 ES_IDLE）；`bRearReturnInProgress` 階梯自有自清；正常交接 parked 狀態四閘全過（`case 13000: return true` 逐行驗證，不會重演 Loader 式等待死結）。

### Lens 3 — 與現場閘比較 + reconciliation 完整性

- **[HIGH, confirmed] 現場閘自身的 unsafe-open**：第 3 節第 7/9 列（回程 <70000 全開、re-teach 失效）+ AMR 全無閘 + NULL deref + reed 卡死永擋。布林邏輯已逐字驗證：`emptypos<=70000` 時整個條件為假，**必不 break**。
- **[MEDIUM, plausible] Color identity pick 無抵達時重查**：DecideJob 只在派工時查 `IsTrayReady()`（`aTrayArm.cpp:317-321`），DoPick 刻意排除 Color（`:407,416-417`）；若手臂橫移途中 lot-finish/CleanOut 觸發 Color DoGoUpTray 回收重夾（`aColor.cpp:333-334, 581+`）→ Z-down 撞上被重夾的盤 — 三條 pick 路徑中唯一的不對稱，一行對稱修法：`if(IsPickFromColor() && ColorModule->IsTrayReady()==false) break;`。
- **[MEDIUM, confirmed→待機驗] 中途 abort + HOME 後 Empty 夾缸命令位殘留 ON**：uHome 放 Loader 夾缸（`uHome.cpp:531-545`）、關 TrayArm 夾缸（`:483-484`），**唯獨不碰 C_Empty_Lean/Push**（全 repo grep 確認寫入者只在 aEmpty.cpp 內）→ 閘 (d) 永久擋、無聲。若 HOME/斷氣實際上會物理放開則是純 false-block；若不會則是正確擋但仍需通知＋復原路。現場閘也有等價停格，非回歸，但屬共同殘留。
- **[LOW, confirmed] reconciliation 文件 C/D 兩列過期**（詳第 7 節）。
- **[SPECULATIVE]** reed vs 命令位的雙故障差異（第 3 節末）；放盤側回程 sensor-off 幾何餘裕（plausible，上機確認）。

### Lens 4 — Sim/實機分歧 + 觀測性

- **[HIGH, confirmed] sim 對 false-ALLOW 回歸結構性全盲**（§2.4 已述）— 「下次會不會抓到」的答案：sim **NO**（碰撞方向）、現場 **PARTIALLY**。
- **[MEDIUM, confirmed] Empty DescribeState 違反 log-computed-verdict 慣例**：FeederDecision.txt 的 [Empty] 區（`aEmpty.cpp:1066-1095`）印了 Status/bRearHasTray/FeedTask，但**缺 `bRearReturnInProgress`、缺兩個後方夾缸 GetOutBit、缺計算後的 RearPickReady verdict**（印的是前方 destack out-bits）。Loader 側反而合規（`aLoader.cpp:1985-1991`）— 實際撞機的模組是儀器最少的那個。
- **[MEDIUM, confirmed] TrayArm 在 State Record 完全隱形**：無 DescribeState；CurrentTasks.txt 只記頂層 Task（整段 pick 都停在 1000，`aTrayArm.cpp:853-866`）→ 無法區分「閘前等待（PickTask 1/10）」vs「已提交（≥1000）」，Job 種類也沒記。SortArm 有 PickTask 匯出（`cStateRecordHT160.cpp:589,623`），TrayArm 沒有。
- **[MEDIUM, confirmed]** 本機無 07-02/03 事故證據：`D:\HT160S_StateRecord` 只有 `2026-07-01 10_46_08\` 資料夾（事故前本機 sim run）；`D:\HT160S_Log` 07-02 零檔案。事故記錄在 KYEC 機上，且該機記錄是**舊 schema**（無 07-03 新增欄位）。
- **[LOW]** 閘開啟瞬間無 breadcrumb（快照是停機後狀態，非 PickTask 10→1000 那一刻）；wrong-BLOCK 只剩 StuckMs 可見。
- **[INFO]** 564154c 的 commit message 未明確聲稱 real-machine 組建（其餘關鍵 commit 都有）— 已被 29 分鐘後 e7c0966 的 real-gate build 傳遞性補齊。

---

## 5. 殘餘風險與待補事項

### 5.1 需修碼（上機部署前）

| 優先 | 項目 | 修法方向 |
|---|---|---|
| P0 | **Loader post-reset stale-FALSE 死結**（lens 2 CRITICAL）| 比照 Color 加 MES142x 級「Loader 後方殘盤請移除」告警，或有守衛的 post-reset re-arm（sensor 確認佔用＋discharge 非進行中＋夾缸命令位 off 才准重發布）；同時 InitialFlag 不應在後盤實存時抹 RearKind/RearTrayID（否則救回的 cover/identity 盤會被當 Normal 誤送）|
| P0 | **Loader stale-TRUE latch**（lens 1 HIGH）| 在 `RefreshRearState` sensor true→false 邊沿同步清 `bRearReadyForPick`，或把 re-arm-false 提前到 DoDischargeTray case 1000 進入點 |
| P1 | **Empty case-10 早退停格**（lens 2 HIGH）| `aEmpty.cpp:399-404` 早退改停 FeedTask=13000 + Status=ES_REAR_READY（或告警）|
| P1 | **blocked-pick watchdog**：DoPick case 1/10 持續 N 秒被擋且 raw presence latch 為 true → Note 指名模組＋觸發 DescribeState dump（silent-stop-must-notify 合規；同時是把「現場再次誤解」轉為「可診斷事件」的信任修復手段）|
| P2 | Color identity pick 加抵達時對稱閘（一行，lens 3 MEDIUM）|
| P2 | uHome 補放 C_Empty_LeanOnTray/PushTray（或至少告警），消除 abort+HOME 殘留命令位停格 |

### 5.2 觀測性缺口（低風險改動，建議與 P0/P1 同車）

- Empty DescribeState 補印：`bRearReturnInProgress`、`C_Empty_LeanOnTray/PushTray.GetOutBit()`、計算後 `RearPickReady`（比照 Loader `aLoader.cpp:1985-1991`，同樣 inline 不呼叫述詞以免 dump 路徑刷 sensor）。
- TrayArm 補 DescribeState / 匯出 `PickTask`+`Job` 到 State Record（比照 SortArm）。
- （選配）DoPick 通過閘那一刻寫一行 EventLog breadcrumb（比照 mapAlarmContext 模式）。

### 5.3 只需上機驗證（不改碼）

- 四層閘全套零實機時數 — 第 6 節全清單。
- HOME/斷氣是否物理釋放 Empty 夾缸（決定 5.1 P2 uHome 項是 false-block 修復還是通知需求）。
- 放盤側回程 carriage 幾何餘裕（lens 3 LOW plausible）。

---

## 6. 下次上機驗證清單

**前置：** 先從 KYEC 機自己的 `D:\HT160S_StateRecord` / `D:\HT160S_Log` 拉 07-02 事故記錄（本機沒有），確認舊述詞干涉簽章；注意該記錄是舊 schema。然後部署 ≥403e8e3（理想含 5.1 修復），**確認機上 EXE 已取代手改的 aTrayArm.cpp 版本**。

每步以 State Record 的 FeederDecision.txt / CurrentTasks.txt 為判據：

1. **正常交接（AMR=0）**：Empty 送盤中 → [Empty] 行必須 `Status=FEEDING`、FeedTask∈{2000,4000,5000,6000,7000}，TrayArm 停 PickTask 1/10、Z-UP 不下；呈盤後 `Status=REAR_READY FeedTask=13000` 且兩後夾缸 out-bit=0，**下一 poll 內必須起取**（不起取＝重演 Loader 式 stale-block，判「仍誤判」方向 B）。
2. **Loader 完整 discharge**：完成後 `bRearHasTray=1 bRearDischargeInProgress=0 bRearReadyForPick=1 RearPickReady=1 Disc=4000` 且 TAJOB_LOADER_RECOVERY 成功取盤；discharge 中（Disc=2000/3000）`bRearReadyForPick=0` 且手臂 hold。
3. **AMR=1 cover/normal 供給**：Empty 送盤中派 AMR supply → 手臂必須 Z-UP hold（403e8e3 路徑，從未實機跑過）。
4. **lot-finish 回程 + 待決 pick job**：手臂必須 hold 過**整段**回程，含 <700mm 運送段（`bRearReturnInProgress=1` / ES_RETURNING）— 這是現場閘留下的破洞窗，新述詞在此擋住＝嚴格優於現場版的直接證明。
5. **斷電/HOME + Empty 後方實盤**：重啟後必須正常取走（ES_IDLE+FeedTask=1+sensor re-latch，不得 strand）。
6. **HOME + Loader 後方實盤**（P0 死結重現測試）：若未修 5.1，預期整機無聲卡死於 TAJOB_LOADER_RECOVERY — 請在受控條件下驗證並記錄，作為 P0 修復的 before/after 對照。
7. **夾缸夾持~釋放間 alarm-abort → HOME**：觀察 Empty 夾缸是否物理釋放、手臂是否無聲停格（決定 uHome 修法）。
8. **判定準則**：「新述詞正確」＝步驟 1-5 全過且第 4 步 hold；「仍誤判」＝步驟 1 中 FeedTask≤6000 時手臂 Z-down（方向 A，碰撞）或步驟 1 呈盤後 >數秒不取（方向 B，stale-block）。若 5.2 觀測欄位已補，記錄本身即可一翻兩瞪眼。

---

## 7. 對 reconciliation 文件的建議更新

`D:\HT160S_BCB\docs\plan\onsite-kyec-reconciliation-20260703.md`：

- **Item C（:34, :63-67）**：現行「需你裁決…若述詞在實機誤判就採現場」已過期 — 該行寫於 564154c（07-03 11:26）之前。應改為：**「已結案（root-caused + superseded）：誤判屬實，肇因 8548420 版 RefreshStateFromSensors raw re-latch；已由 564154c/1a43387/038d5bc/403e8e3 以四層狀態閘重寫；現場 emptypos>70000 手改不採（回程窗全開、AMR 無閘、re-teach 失效、reed 卡死無聲停機）；上機驗證 pending（附驗證清單）＋部署前先修 Loader latch P0 兩洞。」**
- **Item D（LS_ToRear，:35, :69-72）**：現行「建議採用」**必須改為「否決，維持 e7c0966 結論」** — 否決在 HEAD 重新驗證仍成立：`ReleaseSortOwner` 在任何批次釋放時設 LS_ToRear（`aLoader.cpp:681-682`），DoLoader case 3000 會讓排空的 side 在後方被佔用時**停在 LS_ToRear 等 TrayArm 清後方**（`:1108-1111`）— kyec 閘擋的正是唯一能解鎖它的 actor → 互鎖死結；其正當意圖（discharge 運動中擋取）已由 `bRearReadyForPick` latch 完整承接（case 2000 re-arm false `:1788`、case 4000 才發布 `:1811`）。若此列留著「採用」，未來 session 有把死結移植回來的實際風險。

---

**證據誠實度聲明**：本報告所有 confirmed-in-code 項均有 file:line 或 commit hash 支撐；plausible 項（fHasTray 晚 latch 掉盤、DUMMY 放盤閘短路、Color 抵達競速、回程幾何餘裕）需上機確認物理後果；speculative 項（crash-restart IO 保持、夾缸雙故障再伸出）僅列為殘餘風險註記。四個驗證 lens 對「repo 版關閉 Item C 誤判、嚴格優於現場退回版」**結論一致**；分歧僅在殘餘風險排序 — lens 2 將 Loader post-reset 死結列 CRITICAL 而 lens 1 列 HIGH（同一洞、兩個觸發面），本報告採較嚴評級。最大的單一 caveat 重申：**現行閘零實機時數，「嚴格更安全」是 code-level 判定，以第 6 節清單的上機結果為最終仲裁。**