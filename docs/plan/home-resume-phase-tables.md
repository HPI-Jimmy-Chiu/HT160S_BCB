# HOME 續產 — Action 相位表（範本期 v1，2026-07-10）

目的：把「生產中回 HOME 後接續生產」拆到 action 顆粒。本文件先用兩個代表性
action 定出**相位表格式與分類標準**（範本期），owner 審核格式後再批量掃描全機
action（批量期）。上游分析見記憶 `home-resume-production-analysis` 與
P0 修復 commit `d63d33a`（TrayArm 回收握手重發 + place 看門狗 MES1723）。

## 0. 方法與格式定義

核心原則：**case = 執行的步；相位（phase）= 恢復的單位**。HOME 後 Task 一律歸 1
（現行 `InitialAllTask` 行為），重入靠「case 1 相位判定：讀保留的意圖 + 即時感測器
→ 跳相位起點」，永不跳段中步、永不持久化 case 游標。

### 相位表欄位

| 欄位 | 意義 |
|---|---|
| 相位 | 恢復單位代號（P0/PA1/...） |
| cases | 該相位涵蓋的 Task case 值 |
| 動作 | 下的致動器/軸命令 |
| 確認源 | 完成判定的實體來源（缸 On/Off reed、編碼器到位、sensor） |
| commit | 該相位提交的資料/閂鎖變更（理想＝單掃描原子） |
| 重入類別 | 見下 |

### 重入類別

- **相位起點**：無運動純判定，讀 sensor/意圖後派工——合法重入點。
- **冪等段**：重跑只是重新確認（`PushCylinder`/`PopCylinder`/`Move*` 對已達成狀態
  幾個 scan 內回 true，無實體動作）。
- **危險段**：段中重入會做錯事（例：Z 在錯位下降）；只能經前綴相位重建前置條件。
- **原子 commit**：資料交接在單一 scan 完成，中斷後世界只有「做了/沒做」兩態。

### 判定分級

- **綠**＝現行碼已可安全重入。
- **黃**＝差一顆 retain latch 或小修（可枚舉、局部）。
- **紅**＝需機構確認或 owner 政策決策才能定修法。

---

## 1. 範本一：`TEmptyModule::DoGoUpTray`（aEmpty.cpp:673-864）— 純汽缸＋載車混合

角色：相位 A（100-600）把 front 盤送回疊倉；相位 B（1000-7000）把 rear 盤拖回
front。觸發：`bReturnTray` 回收握手（TrayArm 還盤前清 rear）與 CleanOut drain。
進入時設 `bRearReturnInProgress`（aEmpty.cpp:677，擋 TrayArm 撿取），完成後在
DoEmpty 收貨梯清除（aEmpty.cpp:358）。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 |
|---|---|---|---|---|---|
| P0 進入判定 | 1,10 | 無運動；`RefreshStateFromSensors` | SnEmpty_InputHasTray | — | 相位起點（**已內建感測快轉**：front 無盤→直跳 1000，aEmpty.cpp:702-707） |
| PA1 升一段 | 100,110 | Rise_1 Push | 缸 On reed | — | 冪等（上升單調段） |
| PA2 插分離爪 | 200,210,300 | Loader 互鎖等待→Separate Push→settle | 缸 On reed＋互鎖 | — | 冪等 |
| PA3 升二段（頂點） | 310 | Rise_2 Push | 缸 On reed | — | 冪等 |
| PA4 退爪 | 400,410,500 | Separate Pop→settle | 缸 Off reed | — | **反向段開始（過頂點）** |
| PA5 降二段 | 510 | Rise_2 Pop | 缸 Off reed | — | 反向段 |
| PA6 降一段＋commit | 600 | Rise_1 Pop | 缸 Off reed | `bFrontHasTray=false`＋`FrontSourceTray.Clear()`（單掃描，aEmpty.cpp:784-785） | 原子 commit |
| PB0 rear 判定 | 1000 | 無運動；讀 rear sensor | SnEmpty_OutputBottomHasTray | — | 相位起點（感測快轉：無 rear→9000 收尾） |
| PB1 移到 rear | 2000 | MoveEmptyY(discharge) | 編碼器到位 | — | 冪等（重下令＝重定位） |
| PB2 夾盤 | 3000,4000 | LeanOn Push→Push Push | 缸 On reed | — | 冪等 |
| PB3 拖回 front | 5000 | MoveEmptyY(feed) | 編碼器到位 | — | 帶料移動段 |
| PB4 放夾＋commit | 6000,7000 | Push Pop→LeanOn Pop | 缸 Off reed | front=true/rear=false＋grid `CopyFrom`+`ClearTray`（單掃描，aEmpty.cpp:838-844） | 原子 commit |
| 終端 | 8000-10000 | 無 | — | GoUpTask=1 | — |

### 中斷視窗表（HOME 落在該相位時，今日行為）

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| P0～PA3（上升段） | InitialFlag 清 GoUpTask/bReturnTray/bRearReturnInProgress；uHome 不碰 C_Empty_* 缸→實體姿態凍結保留。重發回收請求（P0 fix `d63d33a`）後從 case 1 重入，front sensor 決定走向 | **紅-M1**：front 盤被 Rise 升起時 `SnEmpty_InputHasTray` 是否仍讀到？讀不到→誤跳 rear 段，缸留伸出姿態＋盤懸空，後續 GoDown/Feed 有碰撞疑慮。**紅-M2**：若重跑，case 210 在 Rise_2 已伸時重插分離爪，幾何是否安全（同 TestGoUpTray 歧義） |
| PA4～PA6（反向段） | 重入 case 10 讀 front sensor，但缸姿態 (R1=On,S=Off,R2=Off) 與上升初期**不可區分** | **黃-L1**：補一顆 retain latch `bGoUpPassedApex`（進 400 設、600 清、keep-material 保留），或改「前滾收斂」（HOME 凍結前讓純汽缸序列跑完）——修法選型待 owner |
| PB1～PB3（拖運段） | HOME 歸 EmptyY，盤被 LeanOn/Push 夾著隨車走（uHome 不碰 Empty 夾爪）；閂鎖被清後靠 rear sensor 重推導——但盤此刻**在車上不在底位**，sensor 可能讀空→重生失敗、軟體失明 | **黃-L2**：重入時讀 LeanOn/Push On reed 判「拖運中」相位→直接跳 PB1 重定位續拖（或 InitialFlag 收養為 car-held），比照 TrayArm 殘料收養樣式 |
| PA6 / PB4 邊界 | 單掃描 commit，重入只見「已做/未做」兩態 | 綠 |

**整體判定：黃**。綠項＝P0/PB0 內建感測快轉、兩個原子 commit、冪等段。
黃項＝L1（過頂點 latch）、L2（拖運中相位）。紅項＝M1、M2（機構確認）。

---

## 2. 範本二：`TSortArmModule::DoPlaceToAuto`（aSortArm.cpp:1763-1901）— 馬達＋真空混合

角色：把吸嘴上的 IC 放進目的 Auto 工作盤。HOME 後由 DoSortArm case 1 的
`HasHoldingIC()` 派工重入（aSortArm.cpp:2012-2019）；Slot 酬載由
`InitialFlag(bKeepMaterial=true)` 保留＋真空重掛（aSortArm.cpp:176-201）。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 |
|---|---|---|---|---|---|
| P0 目的地決策 | 1 | `SelectPlaceAuto()`（純軟體，依 Slot 酬載重解） | — | — | 相位起點 |
| P1 定位前綴 | 10,20,30,35 | Z 安全高→Pitch→X＋AutoY 到放料位→位置核對（ShowPlaceDebugInfo） | 各軸到位；falldown 監視覆蓋 10-35 | — | 冪等重定位 |
| P2 危險段 | 40,45 | Z 下＋settle | MovePlaceZDown 完成 | — | 危險段：只能經 P1 抵達（結構保證） |
| P3 原子交接 | 50 | Destroy（真空關）＋blow 重掛 | — | `TransferPlaceDataToAuto`：Auto grid＋計數＋ClearSlot（單掃描） | 原子 commit |
| P4 收尾 | 55,60,70 | blow dwell→Z 升→blow off＋`bResidueArmed` | Z 到位 | 殘留驗證武裝 | 中斷＝IC 已交接，不重放 |

### 中斷視窗表

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| P0～P2 | bHasIC＋酬載保留、真空不斷（uHome 全程不碰 Sucker）；HOME case 100 先歸 SuckZ（Z 帶料升頂）→重入 P0 重解目的地→P1 重新定位→P2 再下 Z | 綠（「不能記住 case 40」的結構性解答：位置由前綴重建，非記憶） |
| P3 邊界 | 單掃描 commit：重入只見「還吸著/已放完」，無「放到一半」邏輯態 | 綠 |
| P4 | IC 已交接不重放；blow 遺留由 InitialFlag 實體關閉（aSortArm.cpp:165-170）。但 InitialFlag 清殘留驗證狀態＋Auto `bResidueClear=true`→驗證被靜默放行 | **黃-S1**：殘留驗證中斷收斂（既列 P3 清單） |

註：吸取衝程視窗（bCanPick 已開真空、bHasIC 未立→ClearSlot 不關真空的無主 IC）
屬 `DoPickFromLoader` 的表，批量期處理（既列 P3 清單）。

**整體判定：綠**（僅 S1 黃項）。本 action 是「相位＝恢復單位」的標準示範，
批量期以此為健康形狀的對照組。

---

## 3. 批量期名單與待決事項

### 批量掃描候選（每 action 一張同格式表）

Empty `DoFeedTray`/`DoGoDownTray`；Color `DoFeedTray`/`DoGoUpTray`/2D 讀取梯；
Loader `DoFeedTray`（含 CCD 掃描梯）/`DoDischargeTray`/`DoFrontDestackDown`/destack 梯；
TrayArm `DoPick`/`DoPlace`（Auto 路）；SortArm `DoPickFromLoader`／殘留驗證梯；
Auto `DoFeedTray`/`DoDischargeTray`/CleanOut 梯。
另做一張**跨模組 setter 清單表**（RequestReturnTray／NotifyTrayXToEmptyFinish／
StageRearGrid／SetRearHasTray*／SetPlaceResidueClear／ChangeActiveTrayData…），
每個 setter 問三題：HOME 後發訊方記得嗎？收訊方記得嗎？誰負責重同步？

### 待 owner 決策／機構確認

| 代號 | 事項 | 類型 |
|---|---|---|
| M1 | front 盤升起時 SnEmpty_InputHasTray 可視性 | 機構確認 |
| M2 | Rise_2 伸出時重插 Separate 爪的幾何安全 | 機構確認 |
| L1 | 過頂點歧義修法：retain latch vs 前滾收斂 | owner 選型 |
| （掛） | ArmMotorHome 無條件清全軸 bHomeFlag（Loader 保盤前置） | owner 政策 |
