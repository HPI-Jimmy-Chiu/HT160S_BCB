# 前站 InputHasTray 讀值有效性（rise1 假亮）— Empty / Color 補閘計畫

- 日期：2026-08-02
- 分支：`feat/iosetview-172-refactor`
- 狀態：**計畫（未實作）**——使用者指示「先想清楚並制定計畫」
- 前情：rear 側同族問題已修（Empty `92e8bc3`、Color `b6da402`）；本文處理 front 側

---

## 1. 問題定義

機構 owner 口述（存檔於 `docs/plan/home-posture-manifest-plan-20260720.md:4,:55`）：

> **rise1 假亮 InputHasTray**；**InputHasTray 有效 = rise1 在下**

前站的 rise-1 缸伸出時會把盤堆抬過 sensor 光束，讓 `Sn*_InputHasTray` 讀到「有盤」，
不論前停位實際有沒有盤。rise1 什麼時候在上：destack（GoDown）與 restack（GoUp cases 100-600）
的編排過程中，以及**卡缸／HOME 打斷編排後停在上面**的異常姿態。

## 2. 現況盤點（本輪逐點開檔驗證）

### 2.1 Loader：**已處理**（不動）

`anti-ghost-d`（2026-07-20，owner ruling Q3「never mint, never blind-act」）：

- 述詞 `IsInputHasTrayTrustworthy()`（`aLoader.cpp`）：
  `rise1 Off-reed 確認縮回才信`；sim 恆信；**Off-reed 未安裝（Enable=false）視為可信**
  （「disabled point never blocks」慣用法，避免永久卡供料）。
- 消費點防護：`DoFeedTray` case 10 在讀值不可信時 **WAIT**（`bRise1Waiting` + `Rise1WaitTimer`），
  逾時發**具名警報 MES0925**，絕不沉默。
- 為什麼 Loader 需要這麼重的防護：它的 case 10 會 **mint**（把幽靈盤寫進身分資料，
  經 9500 鑄造 → 後續空吸），錯一次就污染資料鏈。

### 2.2 Empty：**未處理**（本計畫 E1）

- `RefreshStateFromSensors()`（`aEmpty.cpp:226-227`）**無條件** `bFrontHasTray = SnEmpty_InputHasTray.IsOn()`。
  這個函式被 `DoEmpty` case 100 每個 pass、GoUp case 10/1000、`IsRearReadyForPick` 反覆呼叫——
  **destack/restack 進行中（rise1 在上）每一次呼叫都把 `bFrontHasTray` 污染成 true**。
- `DoGoDownTray` case 700 的 MES1024 檢查（`:818-823`）：**天生有效**——它在 rise1 已 Pop
  （case 500）＋ settle（case 600）之後才讀，該時點 rise1 保證在下。**不需要動。**

### 2.3 Color：**未處理**（本計畫 E2）

- `RefreshStateFromSensors()`（`aColor.cpp:263-270`）同樣無條件讀 `SnColor_InputHasTray`
  （註解自述「mirroring TEmptyModule」——連洞一起鏡像）。
- 另有 `if(bInputFullTray) bFrontHasTray=true` 的 fallback（`SnColor_InputFullTray` 是另一顆
  sensor，quirk profile **未記載**）→ 開放問題 §6-2。
- Color 的 GoDown 前盤確認（`:710-716`，鏡像 Empty case 700）：預期同樣天生有效，
  實作時需比對 Color GoDown 的 rise pop 順序確認（驗證項 §5-V1）。

### 2.4 閘的素材（已驗證）

三台的 rise1 **都有 Off reed 且 Enable=1**（`system/IO_Table.csv`）：

| 缸 | Off reed | 行 |
|---|---|---|
| `C_Empty_FrontRiseTray_1` | `..._Off` Lane0 IP1 P0 B1 | :87 |
| `C_Loader_FrontRiseTray_1` | `..._Off` Lane0 IP1 P1 B7 | :99 |
| `C_Color_FrontRiseTray_1` | `..._Off` Lane0 IP6 P1 B3 | :192 |

## 3. 失效模式分析（為什麼 Empty/Color 用輕量修法就夠）

rise1 卡在上（持續假亮）時，Empty 端的下游鏈：

| 幽靈 `bFrontHasTray=true` 的後果 | 既有皮帶 | 淨風險 |
|---|---|---|
| case 100 跳過 destack 分支（「前面已有盤」）| 無 | **供料餓死**（不撞機、不污染資料）|
| rear 空時 feed 分支啟動 → 夾空氣搬到 rear | `DoClampTray` SettleTicks>0 有 OnSensor confirm＋Pop-on-miss（`PushTray_On reed=夾盤真值`）| 夾空氣會 confirm 失敗 → 警報，不會靜默鑄造幽靈 rear 盤（**實作時驗證 Empty 的 FeedClampSub 走 SettleTicks>0 路徑**，§5-V2）|
| GoUp case 10 誤判「有前盤」→ 多跑 restack | 無 | 多餘動作（rise1 卡上時 restack 的 Push 立即成立，等於空轉）|

**關鍵差異**：Empty/Color 的 refresh 只寫 latch，**不 mint 身分資料**——Loader 那種
「錯一次污染整條資料鏈」的風險不存在。所以不需要照搬 Loader 的 WAIT+警報重裝甲，
**refresh 端 hold-last-state（92e8bc3 同型）即可**。卡缸本身已有具名警報
（`PushCylinder`/`PopCylinder` 逾時 → 40xxx），不會沉默。

另一個方向的失效（假亮遮蔽「真的沒盤」）：MES1024/Color 鏡像的讀取點天生有效（§2.2），
不受影響。

## 4. 修法設計

### E1（Empty）— `RefreshStateFromSensors()` 前站讀取加 rise1 閘

```cpp
//讀值有效 = rise1 確認在下（Off-reed 亮）。不可信時保持 latch 現值。
//Enable-gate 照抄 Loader IsInputHasTrayTrustworthy：Off-reed 未裝 → 視為可信（永不卡）。
bool bFrontReadValid = (HSys.Cyn.C_Empty_FrontRiseTray_1.OffSensor.Enable==false) ||
                       HSys.Cyn.C_Empty_FrontRiseTray_1.OffSensor.IsOn();
if(HSys.Sen.SnEmpty_InputHasTray.Enable==true && bFrontReadValid)
    bFrontHasTray=HSys.Sen.SnEmpty_InputHasTray.IsOn();
```

- **hold-last-state**：與 rear 側 `92e8bc3` 完全同型，語意一致、好審。
- 不加 WAIT／不加新警報（§3 論證：Empty 不 mint；卡缸已有 40xxx 具名警報）。
- GoDown/GoUp 編排期間 latch 凍結在編排起點的正確值；編排的內部判斷
  （case 700 確認）用的是天生有效的讀取點，不受 gate 影響。

### E2（Color）— 鏡像 E1

- 同樣的閘放在 `aColor.cpp` refresh 的 `SnColor_InputHasTray` 讀取前。
- `SnColor_InputFullTray` fallback **維持不動**（quirk 未記載，§6-2 先問）。

### 明確不做

- **不動 Loader**（已有 owner-ruled 防護，重複加閘反而兩層語意打架）。
- **不動 `InputEnd`／`InputFullTray`**（不同 sensor，quirk profile 未記載——
  `InputEnd` 是 source-dry 判斷，錯誤方向的後果完全不同，沒有證據不要碰）。
- **不動 rise2**：manifest 只記 rise1，Loader 的述詞也只看 rise1。照抄已驗證的範圍。

## 5. 驗證計畫

sim **驗不了**（兩台的 refresh 都在 sim early-return）。

- **V1（實作時、案頭）**：比對 Color GoDown 的 rise pop 順序，確認 `:710-716` 讀取點
  在 rise1 落下之後（預期是，鏡像 Empty）。
- **V2（實作時、案頭）**：確認 Empty `DoFeedTray` 的 `FeedClampSub` 走 `DoClampTray`
  SettleTicks>0 路徑（夾空氣的皮帶存在性）。
- **V3（上機）**：手動把 rise1 伸出（Teach Advanced 單缸測試）、前停位淨空 →
  `FeederDecision [Empty] bFrontHasTray` 應保持 0（舊碼會變 1）。
- **V4（上機）**：正常 destack 一輪 → 行為與改前相同（gate 只在 rise1 非在下時介入）。
- **V5（上機）**：Color 重複 V3。

## 6. 開放問題（實作前不需答案，但要記錄）

1. **rise2 是否也假亮？** manifest 未記載。若日後觀察到 rise2-only 姿態下的假亮，
   把 gate 的條件擴成 rise1 AND rise2 皆在下（一行改動）。
2. **`SnColor_InputFullTray` 的 quirk profile**——下次現場順帶問機構 owner：
   rise 缸伸出時它會不會也被抬過光束？
3. **Empty/Color 是否需要 Loader 式的 rise1-卡缸具名警報？** 目前判斷不需要
   （40xxx 已涵蓋），若上機發現卡缸時操作員找不到原因再補。

## 7. 建置與提交

- 兩檔皆 Big5（各 7 個 non-ASCII byte）→ **byte-safe 編修**，non-ASCII 數驗證不變。
- 建置閘：`-Clean` → `-Full`（關 SOFT_SIMULATE）→ 還原重建 → 編碼檢查。
- 兩個 scoped commit：`fix(empty)` 與 `fix(color)`，訊息引用本計畫。
