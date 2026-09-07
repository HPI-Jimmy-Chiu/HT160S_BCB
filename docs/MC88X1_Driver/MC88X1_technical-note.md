# MC88X1 / MCD451 運動控制卡技術筆記

> 本文件由 `docs/MC88X1_Driver/` 內原廠手冊整理而成，作為 HT160S_BCB 馬達控制
> （`MotorAndIO/myMC88X1motor.cpp`）的開發參考依據。所有暫存器、公式、參數範圍
> 均出自原廠文件，並對照本專案實際程式碼標註已知問題與調機建議。

## 0. 來源與適用範圍

| 文件 | 內容 |
|------|------|
| `MC88系列使用手冊v35.pdf` | MC88X1P DLL API（本專案直接呼叫的層）|
| `MCD451基本功能篇.pdf` | 底層 MCD451 ASIC 暫存器與速度模型（DLL 之下的硬體層）|
| `MCD451補間功能篇.pdf` | 補間（直線/圓弧），本機未使用 |
| `BCB載入DLL方式.pdf` | C++Builder 載入 DLL 的方式 |

- 本機使用的卡別在 `Mot_Table.csv` 標 `CardModel=MC88X1`，每張卡 8 軸（X1Y1Z1U1 / X2Y2Z2U2）。
- 基準時鐘 FCLK 推薦 26.2144 MHz，最大 40 MHz。

---

## 1. 速度模型（最重要，決定調機行為）

MCD451 的 PG 回路用三個資料控制脈波輸出頻率：**RANGE DATA、LOW/HIGH SPEED DATA**。

```
FUNIT (pps/LSB)  = FCLK / (RANGE_DATA × 262144)      // 頻率設定單位
F_實際 (pps)      = FUNIT × SPEED_DATA                 // 實際輸出頻率
```

| 暫存器 | 範圍 | 意義 |
|--------|------|------|
| `RANGE_DATA` | **1 ~ 4,095** (0x001~0xFFF) | 輸出倍率；越大 FUNIT 越小、解析越細、jitter 越小 |
| `LOW SPEED DATA` | **1 ~ 65,535** | 低速（起始速）SPEED_DATA |
| `HIGH SPEED DATA` | **1 ~ 65,535** | 高速（驅動速）SPEED_DATA |
| `RATE-A DATA`（加速度）| **1 ~ 4,095** (0x001~0xFFF) | 加速段最大斜率；越大加速越陡 |
| `RATE-B DATA`（減速度）| 1 ~ 4,095 | 減速段 |
| `SCW-A/B`（S 形比例）| 1 ~ 32,767 | S 形加減速區間 |

加速時間（直線段）：
```
TUNIT (sec) = RATE_A_DATA × 4 / FCLK
TUD  (sec)  = TUNIT × (HIGH_SPEED_DATA - LOW_SPEED_DATA)
```

> **關鍵結論**：`SPEED_DATA` 上限 65535，但**加速度暫存器(RATE-A)上限只有 4095**。
> SPEED_DATA 與 RATE 的合法範圍會隨「最大速度(Max Speed / MDV)」推導出來的
> RANGE_DATA 而變動。任一參數超出範圍，DLL 會回傳錯誤值且**參數不生效**（見 §2）。

範例（手冊 18-1，FCLK=26.2144MHz）：RANGE=100 → FUNIT=1.0pps；LOW=1000 → 1000pps；
HIGH=20000 → 20000pps；RATE-A=256 → 加速時間 0.742s。

---

## 2. `MC88X1PMotAxisParaSet`（本專案速度設定入口）

```c
LRESULT MC88X1PMotAxisParaSet(
    BYTE  byBoardID,    // 0~15
    BYTE  bySetAxis,    // bit 指定軸（1<<port）
    BYTE  byTS,         // 0=T 形加速, 1=S 形加速
    DWORD dwSV,         // 起始速度 SPEED_DATA
    DWORD dwDV,         // 驅動速度(運行速) SPEED_DATA  <- 連續移動/jog 實際跑這個
    DWORD dwMDV,        // 最大速度 SPEED_DATA（速度/加速度合法範圍的參考基準）
    DWORD dwAC,         // 加速度（DLL 內部換算成 RATE-A，1~4095）
    DWORD dwAK );       // 加加速度(jerk)，S 形用
```

`MC88X1PMotAxisParaSet2` 與上者相同，但 `dwAC` 改用**秒**指定（SV→DV 花費時間）。

### 超出範圍的回傳碼（手冊表 9/表 10）

呼叫成功回 `ERROR_SUCCESS`（不代表執行成功）；任一速度/加速度參數超範圍回 `0x1000~0x10E9`：

| 偏移（X1 基底 0x1000，其餘軸 +0x20/軸）| 原因 |
|--|--|
| `+0H / +1H` | SV 低於 / 高於範圍 |
| `+2H / +3H` | **DV 低於 / 高於範圍** |
| `+4H / +5H` | MDV 低於 / 高於範圍 |
| `+6H / +7H` | **AC 低於 / 高於範圍** |
| `+8H / +9H` | SCW 低於 / 高於範圍 |

> Y1 基底 0x1020、Z1 0x1040、U1 0x1060、X2 0x1080、Y2 0x10A0、Z2 0x10C0、U2 0x10E0。

### 本專案對應（`SetMC88X1MotPara()` @ `myMC88X1motor.cpp`）

```
dwSV  = InitSpeed   × Range      // Range = Mot_Table 的 Range 欄（軟體倍率，非晶片 RANGE_DATA）
dwDV  = iSpeed      × Range      // iSpeed：回 HOME 尋原點時被設為 HomeHighSpeed
dwMDV = JogHighSpeed× Range
dwAC  = (iSpeed×Range - InitSpeed×Range) / Acc     // Acc = Mot_Table 的 Acc 欄(秒)，再夾到 [iAccMin,iAccMax]
```

✅ **已修（20260617）：接回傳碼診斷。** `SetMC88X1MotPara()` 現在把 AxisParaSet 回傳碼
存到 `LastParaError`（`GetLastParaError()` 可取）；另有 `VerifyHomeParaRange()` 乾跑
HOME-seek 參數組並回傳卡片判定碼。Motor Parameter 畫面 Save 後會對每顆軸檢查 run/home
兩個 profile，把卡片拒收（哪個參數 over/under range）用對話框報給操作員（`DecodeAxisParaError`
解碼手冊表 9 的 0x1000~0x10E9）。

> 原始問題：早期 `SetMC88X1MotPara()` 直接呼叫 AxisParaSet **不檢查回傳碼**，算出的 `dwDV`/
> `dwAC` 超出（由 `dwMDV` 推導的）合法範圍時，卡片拒絕整組參數、軸維持上一組（多半是慢速
> 預設值），程式卻以為設定成功——這就是「200 反而慢」會靜默發生的原因。

⚠ **已知風險 2：加速度與速度、Range 綁死。** `dwAC ∝ Range × (HomeHighSpeed - InitSpeed)`。
`Range` 欄同時是速度與加速度的總倍率，不是單純速度倍率。

---

## 3. 200/50 比 10/1 還慢、扭力過載之謎（實測現象的解釋）

實測（M12 `MTopCCDX` / M20 `MTopCCDX_Color`，Range=60、Acc=0.1、JogHighSpeed=20000）：

| 設定 | dwDV = Speed×Range | dwAC ≈ (DV-SV)/Acc | 合法性 | 實測 |
|------|----|----|----|----|
| HighHome=200 | 12000 | ≈119,400 | **AC 遠超上限** | 被拒 → 走慢速預設 → **慢** |
| HighHome=10  | 600   | ≈5,400   | AC 仍偏大但較接近 | 被接受/部分接受 → **快、過載** |
| HighHome=10, **Range=5** | 50 | ≈450→夾到下限 | 都在範圍內 | 接受 → **慢且安全** |

**結論**：`dwAC` 是物理加速度，DLL 要把它換算成 RATE-A（上限 4095）。HighHome 越大，
`dwAC` 反而越大、越容易超出範圍被卡片拒絕，於是軸退回慢速預設——這就是「200 反而慢」
的反直覺現象。降 `Range` 會同時縮小 DV 與 AC，把兩者拉回合法且溫和的區間，所以
「Range 降到 5 就沒事」。

**正確調機旋鈕**：
- 想要「速度快但加速溫和、不過載」→ 調大 `Acc` 欄（秒，分母，越大越緩），不要只壓 Range。
- `Range` 請維持讓 `Speed×Range ≤ 65535` 且 `dwAC` 落在合法範圍的數量級。
- 中長期建議：在 `SetMC88X1MotPara()` 檢查 AxisParaSet 回傳碼並記 log，超範圍時夾到
  合法上限而非靜默失敗。

---

## 4. 原點復歸（Home）

### 4.1 卡片原生模式一覽（`HomeType` 暫存器，值 0~8）

| HomeType | Phase0 | Phase1 | Phase2 | Phase3 |
|--|--|--|--|--|
| 0 | 找Home | 出Home | 找Z相 | Offset |
| 1 | 找Home | - | - | Offset |
| 2 | 找Home | 出Home | - | Offset |
| 3 | - | - | 找Z相 | Offset |
| 4 | 找Limit | - | 找Z相 | Offset |
| 5 | 找Limit | 出Limit | 找Z相 | Offset |
| 6 | 找Limit | 出Limit | - | Offset |
| **7** | 找Home | 出Home | **再次入Home 瞬間停止** | Offset |
| 8 | 找Home | 出Home | 移動到Home位置 | Offset |

> **本機沒有 Type 90。** 晶片合法值只有 0~8。程式裡的 `iHomeType==90` 是**軟體自訂的
> 手動尋原點流程**（`HomeType90()`，連續離開/慢速再接近）。當 `iHomeType==90` 時，
> `InitMotor()` 會把卡片 `HomeType` 暫存器寫成 8，由軟體自己跑 seek。

> **架構決定（20260617）：全面採用 type 7（卡片原生）。** `database.cpp` 對**所有軸**
> `SetHomeType(7)`，`HomeType90()` 軟體 seek 已退役（程式保留為 dormant fallback）。
> 原因：type 7 的「找到 Home → 瞬間停止」由卡片硬體閘斷 IN3 完成，**home 重複性不受 PC
> 主迴圈延遲/負載影響**；多軸同時 home 各卡片硬體並行、互不干擾。type 90 軟體 seek 的
> sensor→Stop 在主迴圈裡，程式越大/軸越多誤差越大。若某軸機構日後真的需要手動反向逼近，
> 才在 `database.cpp` 把該軸切回 90。

> **type 7 的限制 + 已加的保護（20260617）：卡片原生 home 不會自己脫離極限。**
> `MC88X1PMotHome` 只會照 `HomeP0_Dir` 一個方向找 Home；若軸正卡在「該方向上的硬體極限」，
> 它會**繼續往極限撞**（實測：MAutoY_6 在 CW 極限上按 HOME，反而往 CW 撞）。原本的軟體
> `HomeType90` seek 有脫限（case 11 檢查 cw/ccw）。現已把脫限移植進 `MC88X1MotHome`
> （cases 2-4）：home 前先掃 limit，亮哪個極限就往脫離方向 Jog，逾時（`HOME_PHASEB_TIMEOUT_MS`）失敗不卡死。
>
> ⚠⚠ **極限有兩層，務必分清（20260618 實機+手冊修正；中途試過 Direction-aware 擋鎖，錯誤已還原）：**
>
> **(A) 操作層 — 極限開關是「空間慣例」接線，全機一致、與 Direction 無關。** 實測 M20（Direction=1）：
> Jog− → 實體左 → 亮 CCW(`iCcwLed`)；Jog+ → 實體右 → 回 HOME、無 CW 燈。所以 **Jog+ 端 = CW 極限、
> Jog− 端 = CCW 極限** 對每顆軸都成立。正確的 jog 擋鎖是 **Direction-BLIND 原版**：`iCwLed` 亮擋 Jog+、
> `iCcwLed` 亮擋 Jog−（`uMotorTest`/`uteach` `StartJog`、`MC88X1MotHome` case 2 皆已還原成這個）。
> 中途那版「Direction-aware 擋鎖」在 Direction=1 軸是**反的**（會叫操作員往極限裡撞），已撤掉。
> 使用者認知「Jog+→右→CW 極限、Jog−→左→CCW 極限」**正確且通用**，不要把擋鎖改成吃 Direction。
>
> **(B) 卡片層 —「脫不了極限」是卡片擋的，這才是真正待解的問題。** MCD451基本功能篇 p.37：
> **+LM 只在「正轉/CW」驅動時觸發停止、−LM 只在「逆轉/CCW」驅動時觸發**（依脈波方向、escape-aware）。
> `Cmove` dir（p.92）0=CW/1=CCW；`JogP/JogN` 吃 `Direction` 翻轉（`Direction?Cmove(0):Cmove(bit)`）、
> OT 位元不吃。於是 **Direction=1 軸 Jog+=CCW 脈波**：卡在 −OT(CCW) 時要 Jog+ 脫離=CCW 脈波，正好被
> −LM 擋（脫不掉）；反向 Jog−=CW 脈波往裡撞卻**不被卡片擋**（所以 (A) 的軟體擋鎖才是唯一保護）。
> 也就是 **Direction=1 軸，卡片的 allow/block 跟空間脫離方向是相反的**。原本 `JogP/JogN` 的
> `Stop()+MotIpReset()` 清不掉這個（MotIpReset≠放鬆硬極限），已移除；現為純 Cmove + busy 時 ClearAxisAlarm，
> 與 HT172 MN200 一致（其 `JogP/JogN` 純粹、表單也無極限擋鎖）。
> 另：撞限**不一定**觸發 servo alarm（實測 `alarm=0` 仍卡），別預設是 alarm。
>
> ✅ **(B) 已解（治本，在驅動器，M20 實機驗證 20260618）：不要在程式裡加放鬆硬極限的 hack。**
> 每顆 **Direction=1** 軸做「驅動器正規化」，讓軸卡回到原生 Direction=0：**同時翻 A6 `Pr0.06`
> (指令脈衝旋轉方向 0↔1；EEPROM 寫入後斷電重開) 和 `Mot_Table Direction`(1→0)**（通常還要翻
> `HomeDirectior`，用 home 驗證）。兩個一起翻是**物理淨中性**（Jog+/MoveTo/NowPosition/教導點不變），
> 只是把指令脈波方向對齊（空間接線的）極限開關，使 CW 脈波=實體右=+OT 端，於是卡片**原生允許脫離、
> 撞進去也原生自動停**。M20 驗證 OK。操作員認知「Jog+→右→CW 極限、Jog−→左→CCW 極限」自然成立。
> 其餘 15 顆 Direction=1 軸（M01-M04、M06-M11、M14-M17、M19）比照辦理（M05/M12/M13/M18 本就 Direction=0）。
> 面板按鈕 SOP 與調機文件索引：`docs/panasonicA6/A6_front-panel-parameter-SOP.md`(+.html)。
>
> ⚠ 若某軸的 home 方向正指向它脫離的那個極限、且 home sensor 緊貼極限，單純脫限可能仍不足
> ——該軸應保留 type 90（用其 adaptive seek）。type 7 vs 90 因此**可能是逐軸決定**，不是全域。

所有模式的 Home Sensor 都是 **IN3**。

### 4.2 Home 參數埠位址（`MC88X1PMotWrReg`，wPort）

| Port Name | 位址 | 意義 |
|--|--|--|
| HomeOffset | 0x309 | 機械原點與程式原點偏移 |
| HomeMode | 0x30A | 保留 |
| HomeType | 0x30B | 原點搜尋模式 0~8 |
| HomeP0_Dir | 0x30C | 第0階段方向 |
| Home_P0_Speed | 0x30D | 第0階段速度 |
| HomeP1_Dir | 0x30E | 第1階段方向 |
| HomeP1_Speed | 0x30F | 第1階段速度 |
| HomeP2_Dir | 0x310 | 第2階段方向 |
| HomeOffset_Speed | 0x311 | 移至程式原點速度 |

本專案 `InitMotor()` 寫入：`HomeP0_Speed = HomeHighSpeed×Range`、
`HomeP1_Speed = HomeP2/Offset_Speed = HomeLowSpeed×Range`。

### 4.3 相關函式

- `MC88X1PMotHome(board, axis)`：啟動原生原點搜尋。
- `MC88X1PMotHomeStatus(board, axis)`：回 `AxisHomeBusy`（搜尋中）/`AxisHomeErr`（失敗）/
  `ERROR_SUCCESS`（完成）/`AxisDrvBusy`/`AxisIpBusy`。
- `MC88X1PMotHomeReset(board, axis)`：被外部錯誤截停、但 Home 程式仍在跑時，用它停止。
- `MC88X1PSetHomeLogic(board, axis, value)`：設定 IN3 作動位準，**0=低位準致動，1=高位準致動**。

---

## 5. 數位輸入 `MC88X1PMotDI`（回傳 byte 的位元定義）

| bit | 訊號 | 說明 |
|--|--|--|
| 0 (0x01) | IN0 | 一般用（本機 Z 相邏輯掛 IN0）|
| 1 (0x02) | IN1 | |
| 2 (0x04) | IN2 | |
| **3 (0x08)** | **IN3** | **原點感測器（Home Sensor）** |
| 4 (0x10) | EXPP | 正方向外部驅動輸入 |
| 5 (0x20) | EXPM | 負方向外部驅動輸入 |
| 6 (0x40) | InPos | 到位 |
| 7 (0x80) | Alarm | 伺服 Alarm |

對照 `ScanMotorStatus()`：`iHomeLed = (MotDI&0x08)`、`iInposLed=(&0x40)`、`iAlarmLed=(&0x80)`。

### 5.1 `MC88X1PGetMotionInput`（極限/異常，`*lpValue` 位元）

| bit | 訊號 | 對應 ScanMotorStatus |
|--|--|--|
| 0 (0x01) | 正軟體極限 | iSoftcwLed |
| 1 (0x02) | 負軟體極限 | iSoftccwLed |
| **2 (0x04)** | **正硬體極限 +OT** | **iCwLed** |
| **3 (0x08)** | **負硬體極限 -OT** | **iCcwLed** |
| 4 (0x10) | 伺服異常 Servo alarm | iServoalarmLed |
| 6 (0x40) | 到位 InPos | — |

> 重點：`iCwLed`=正硬限(+OT)、`iCcwLed`=負硬限(-OT)。卡片層脫限走向（純晶片）：脫 +OT 走 CCW、
> 脫 -OT 走 CW；但 `JogP/JogN` 吃 `Direction` 翻轉，操作層的極限標籤又是空間慣例（Jog+ 端=CW），
> 兩層在 Direction=1 軸會打架——詳見 §4 的 (A)/(B) 兩層說明與待解的卡片放鬆極限問題。

---

## 6. `HomeFlag()` 兩套判定不一致（已修 20260617）

> ✅ 已修：`HomeFlag()` 現在所有 home type 統一用 `MC88X1PMotDI` bit 0x08 (IN3)，
> 移除了下表 type-7 那條讀錯暫存器/極性相反的路徑。以下保留問題分析作為紀錄。


`TMyMC88X1Motor::HomeFlag()` 依 `iHomeType` 走**兩條不同路徑**讀原點 sensor：

| 路徑 | 來源 | 取位元 | bSensorType=1 時回傳 |
|--|--|--|--|
| `iHomeType==90` | `MC88X1PMotDI` | bit `0x08`（IN3）| `(MotDI & 0x08)!=0` |
| 其它（含 7）| `ReadStatus(0x08)` 原始字 | bit `0x0080` | `(word & 0x0080)==0`（**反相**）|

- type-90 路徑與 `ScanMotorStatus` 的 home LED **完全一致**（都是 MotDI IN3）。
- type-7 路徑讀的是另一個讀取埠（原碼註解「PG signal read port」）且 active level 相反。

**實測現象**（2026/06/17 單軸 HOME log）：
- `MTopCCDX`（type 90）：`HomeFlag=1` → `HOME_DONE` 成功。
- `MSuckZ_1/2`、`MSortingArmX`、`MTrayArmX`（type 7）：卡片回完成，但 `HomeFlag=0` →
  `HOME_DONE_TIMEOUT`（在 `MyMotor.cpp` case 200 等 sensor 確認逾時）。

**研判**：type-7 的 `HomeFlag()` 讀錯來源/極性，導致原點明明到位卻判讀為 0 → 誤逾時。
此判定**沿用原始 HT160 生產版**（非移植回歸），但生產版預設 `HomeType=90`，type-7 路徑
平常未被走到；現在預設改成 7（`database.cpp` 依 Alias 設定，`MTopCCDX/MTopCCDX_Color`=90、
其餘=7）才暴露出來。

**建議修法（待確認後再改）**：把 type-7 的 `HomeFlag()` 統一成 type-90 的可靠讀法
（`MC88X1PMotDI` bit 0x08，`bSensorType?Flag:!Flag`），與 `ScanMotorStatus` 一致。
驗證方式：type-7 軸停在原點時，看 Motor Test 畫面 home LED 是否亮；亮但 `HomeFlag()`=0
即可確認是判讀 bug。

---

## 7. 開發備忘

- 速度單位都是 SPEED_DATA（1~65535），實際 pps 還要乘 FUNIT；本專案以 `Speed×Range` 當
  SPEED_DATA 直接餵卡。
- 改 `Mot_Table.csv` 的 `Range`/`Acc`/`HomeHighSpeed`/`HomeLowSpeed` 前，先用本文 §1~§3
  估算 `DV=Speed×Range`、`AC=(DV-SV)/Acc` 是否落在 65535 / 4095 的合法範圍。
- `HomeType` 已不在 CSV（移除 HomeType 欄正確），改由 `database.cpp` 依 Alias 寫死。
- 原點 sensor 一律是 IN3；極性由 `MC88X1PSetHomeLogic` 設定，對應 `bSensorType`。
