# 統一模組 Status Enum — 設計提案（待審）

- 日期：2026-07-03　狀態：**設計待使用者核准**（未動工）
- 依據：使用者補充「所有流道、SortArm、TrayArm 等都要和 Loader 一樣有明確 status 狀態」
- 鐵律（Empty 干涉教訓 2026-07-03）：**status 只能由各模組自己的梯形在「物理正確時刻」設定；`RefreshStateFromSensors` 永遠不寫 status**（sensor 只做確認，不做狀態來源）

## 1. 現況盤點

| 模組 | 現況 | 缺口 |
|---|---|---|
| Loader | ✅ `eLoaderStatus LS_*`（範本） | 無 |
| TrayArm | `TAS_IDLE/PICKING/CARRYING/PLACING` 已存在 | **`TAS_PLACING` 從未被設**（死狀態） |
| SortArm | 無成員 status，只有 PickTask/PlaceTask 游標 | 全缺 |
| Empty | 游標(Feed/GoUp/GoDown)＋latch 拼湊 | 全缺 |
| Color | 游標(×5)＋latch 拼湊 | 全缺 |
| Auto | 共用梯形三游標＋每站 bool 袋 | 全缺（需**每站**status） |

## 2. 提案 Enum（最小集，只表達「別的模組需要知道的事」）

### TrayArm（補完既有 enum）
- 補設 `TAS_PLACING`：DoPlace*/DoLowerClampRaise 進入時
- `TAS_CARRYING` = 盤在手+Z上（divert 視窗）——已正確

### Empty `eEmptyStatus ES_*`
`ES_IDLE / ES_DESTACK / ES_FEEDING / ES_REAR_READY / ES_RETURNING`
- 關鍵設定點：`ES_REAR_READY` 在 DoFeedTray **case7000 成功分支**（夾缸已 Pop＋sensor 確認）——即今日 `bRearHasTray=true` 的正確時點
- `ES_FEEDING` 覆蓋 case1000..6000（= 不可夾窗口）

### Color `eColorStatus CS_*`
`CS_IDLE / CS_DESTACK / CS_FEEDING / CS_REAR_READY / CS_RETURNING`
- 鏡射 Empty；`CS_REAR_READY` = 今日 `bTrayReady` 時點（本來就物理正確）
- SortBin 模式凍結在 `CS_IDLE`（同儕不與 Color 交握）

### SortArm `eSortArmStatus SAS_*`
`SAS_IDLE / SAS_PICKING / SAS_PLACING / SAS_RECOVERY`
- `SAS_RECOVERY` = residue 重吸中或吸錯 latch（背景覆蓋態）
- 吸嘴全在上方(`AreAllSuckersHome`) 保持獨立 live 檢查、與 status AND

### Auto **每站** `eAutoStatus AS_*`（`TAutoStationState` 加 `int Status`）
`AS_IDLE / AS_REAR_STAGED / AS_LOADING / AS_SORTING / AS_FULL / AS_DISCHARGING / AS_CLEANOUT_DONE`
- `AS_REAR_STAGED` 設於 `NotifyTrayArmDelivered`（唯一 producer）
- `AS_IDLE` 提案設於 discharge **case6100**（載台退回+FrontRise 完成）——比今日 case1000 旗標清除**晚**（見開放問題 1）

## 3. 佈線（predicate 改讀 status）
- CleanOut idle 複合條件 → `Status!=XX_IDLE`（退役游標拼湊）
- `IsRearReadyForPick` 家族 → `Status==ES/CS_REAR_READY`＋夾缸 out-bit 保底
- Auto `FindFeedAuto`→`AS_REAR_STAGED`、`GetTrayRequest`→`AS_IDLE`、`FindDischargeAuto`→`AS_FULL`
- 每模組加 `StatusName()`（仿 Loader）→ `DescribeState()` 前置 `Status=<名>`；遷移期 **status 與舊 latch 並行**，DescribeState 印不一致警示

## 4. 遷移順序（每步獨立可建置）
1. TrayArm（最小；補 TAS_PLACING + 換讀者 + 記錄）
2. Empty（enum+5 設定點+記錄 → 再翻 IsRearReadyForPick/IsCleanOutFinish 讀者）
3. Color（鏡射；3b=TrayArm Color pick 等待屬行為變更，需上機驗證）
4. SortArm
5. Auto：**5a 只寫不讀**（status 影子跑、FeederDecision 比對）→ 燒機後 **5b 一次翻一個讀者**（每個一 commit）
6. 清理（燒機後才刪冗餘 latch——見開放問題 3）

## 5. 開放問題（待使用者裁決）
1. **Auto 接盤時機**：discharge 尾段（載台退回中）可否接受新的 rear 交付？今日=可（case1000 就清旗標）；提案 `AS_IDLE`=case6100 較晚（防撞更嚴、產能略降）。
2. **SAS_RECOVERY**：吸錯/重吸做成第 4 個狀態，還是留兩個 live predicate 與 SAS_IDLE 並列 AND？
3. **退役深度**：燒機後刪 `bRearDeliveredPending`/`bRearReturnInProgress`/`bTrayReady`，或永久保留當冗餘安全 latch？
4. **UI**：status 是否上 stbMain/MotionView 顯示（建議先只進記錄，UI 後續）？
