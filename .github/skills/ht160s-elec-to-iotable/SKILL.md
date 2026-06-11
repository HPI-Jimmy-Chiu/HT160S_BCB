---
name: ht160s-elec-to-iotable
description: >-
  Use when converting an electrical-control (電控) cylinder/sensor address sheet
  into HT160S_BCB IO_Table.csv rows, or when adding/auditing a cylinder, sensor,
  or switch that an electrical engineer provided. Documents the fixed mapping
  from a 電控 hardware label (輸出 / On感測 / Off感測 / 中位 / 在席 sensor) to the
  IO_Table.csv row structure, the three files that must change together, and the
  naming convention. Triggers: 電控, electrical sheet, IO_Table.csv, Cylinder_On,
  Cylinder_Off, 中位, 到位感測, 在席, FrontRiseTray, FrontSeparateTray, PushTray,
  LeanOnTray, CartRotate, CartSlide, InitialCylinderName, InitialSensorName,
  add cylinder, add sensor, IOType Cylinder Sensor Switch.
---

# HT160S 電控資料 → IO_Table.csv 轉換法

電控（電氣/配線）工程師提供的是「**實體點位表**」：每個氣缸/感測器的
輸出線號、到位感測線號、中位線號。軟體端的工作是把它**轉成 IO_Table.csv 的
列結構 + 命名 + database 成員**，不是去發明硬體位址。本 Skill 固化這個轉換法，
避免每次重新分析。

## 0. 鐵則（先記這條）

- **電控線號可依本機規則換算成 IP/Port/Bit，但必须用 IO_Table 既有列驗證 + 確認位址未被佔用。**
  本機實證換算規則（2026-06-09 由 Color 既有列反推驗證）：
  - 輸出 `10Wxy` → `IP=32, Port=x, Bit=y`（x/y 為八進位字組編號：W00~W07=Port0 Bit0~7、W10~W17=Port1 Bit0~7）。
    例：10W07→`32,0,7`、10W11→`32,1,1`、10W12→`32,1,2`、10W13→`32,1,3`、10W10→`32,1,0`。
  - 感測 `116xy` → `IP=6, Port=x, Bit=y`（同八進位）。
    例：11612→`6,1,2`、11613→`6,1,3`、11621→`6,2,1`、11622→`6,2,2`、11625→`6,2,5`。
  - **這是本機 IP=32 輸出卡 / IP=6 感測卡的經驗規則，不是全機通用**。換算後一定要：
    ①用 grep 查 IO_Table 確認該位址未被占；②用同卡已知可用列反向驗證規則套得上。
  - 若某卡的線號規則與本表不符（不同輸出卡/不同機型），以 IO_Table 既有實測為準，勿硬套。
- 一個氣缸 = **1 主列 + 2 到位列**（On/Off），中位/在席視硬體另加 Sensor 列。

## 1. IO_Table.csv 欄位定義

第一行 header：

```
IOType,Alias,Lane,ModuleType,IP,Port,Bit,InType,ISABase,Enable,OnAlarmTime,OffAlarmTime,OnDelayTime,OffDelayTime,Note
```

| 欄 | 意義 |
|----|------|
| `IOType` | `Sensor` / `Cylinder` / `Cylinder_On` / `Cylinder_Off` / `Switch` |
| `Alias` | 程式成員名（= database.h 結構成員、InitialXxxName 對應）|
| `Lane,ModuleType,IP,Port,Bit` | 實體位址（**電控決定**）|
| `InType` | 接點型態（0/1）|
| `ISABase` | ISA 卡基底（一般 0）|
| `Enable` | 1=啟用、0=停用（位址留空時填 0）|
| `OnAlarmTime,OffAlarmTime` | 到位逾時報警（ms 倍率，氣缸常見 50）|
| `OnDelayTime,OffDelayTime` | 動作延遲（常見 5）|
| `Note` | 備註（例如 `COMM_PAD`）|

## 2. 電控標籤 → IO_Table 列 的對照

電控一個「氣缸」通常給：**輸出 / On感測 / Off感測 / 中位**。轉成：

| 電控欄 | 產生的 IO_Table 列 | 範例（C_Color_FrontRiseTray）|
|--------|-------------------|------------------------------|
| 輸出 | `Cylinder,<名>` 主列（放輸出位址）| `Cylinder,C_Color_FrontRiseTray,0,0,32,0,7,1,0,1,50,50,5,5,` |
| On感測 | `Cylinder_On,<名>_On`（放 On 感測位址）| `Cylinder_On,C_Color_FrontRiseTray_On,0,0,6,1,2,1,0,1,,,5,5,` |
| Off感測 | `Cylinder_Off,<名>_Off`（放 Off 感測位址）| `Cylinder_Off,C_Color_FrontRiseTray_Off,0,0,6,1,3,1,0,1,,,5,5,` |
| 中位 | 另開一條 `Sensor,Sn<...>Mid`（若程式要讀中位）| 視需求 |
| 在席（托盤/料）| `Sensor,Sn<Module>_<Pos>HasTray` | `Sensor,SnColor_OutputBottomHasTray,...` |

規則重點：
- **無到位感測的氣缸**（純計時型，例如「分盤缸 FrontSeparateTray_1」）：
  仍要建 `Cylinder_On` / `Cylinder_Off` 兩列，但位址欄**全部留空、Enable=0**。
  （Empty/Color 的 `FrontSeparateTray_1` 即如此 → 程式靠 `GoDownDelay` 計時，不讀到位。）
- **電控是雙缸**（例如「前頂Tray氣缸」標 `10W07/10W10` 兩個輸出 + 中位）：
  → 程式要建 `_1` 與 `_2` 兩個成員（`C_xxx_FrontRiseTray_1` / `_2`），
  不可只建一個。逐片分盤（destack）機構需要兩段升降缸交替，缺一顆無法分盤。
- **在席 sensor 與堆疊 sensor 是兩個角色**，不可共用一顆。命名要分清：
  `SnXxx_InputHasTray`（堆疊/投料）vs 分盤後待命位在席（若有，命名如 `SnXxx_FrontHasTray`）。

## 3. 命名慣例（比照 Empty）

- 氣缸：`C_<Module>_<Function>`，托盤類動作加 `Tray` 字尾。
  - 升降：`FrontRiseTray_1/_2`、`RearRiseTray`
  - 分盤：`FrontSeparateTray_1`、`RearSeparateTray_1`
  - 推/靠：`PushTray`、`LeanOnTray`
  - 台車（Color 專有，輔助定位）：`CartRotate`、`CartSlide`
- 感測：`Sn<Module>_<Pos>`，例如 `SnColor_InputHasTray`、`SnColor_OutputBottomHasTray`。
- `<Module>` = `Empty` / `Color` / `Loader` / `Auto1`...。
- Loader 的前頂/分盤缸是 **Loader1/2 共用**（無 1/2 後綴：`C_Loader_FrontRiseTray_1/_2`），
  Push/LeanOn 才分邊（`C_Loader1_PushTray`）；Empty/Color 是獨立單道。

## 4. 新增一個氣缸/感測，必須同步改的 3 處

1. **database.h** — 在對應 `CYLINDER_MODULAR` / `SENSOR_MODULAR` 區塊加成員宣告。
2. **IO_Table.csv** — 加主列 + On/Off 列（氣缸）或 Sensor 列；位址由電控填。
3. **database.cpp `InitialCylinderName()` / `InitialSensorName()`** — 註冊中文/別名，
   讓 IO 載入與畫面顯示對得上。

> 漏掉第 3 步 → 程式可編譯但 IO 載入時找不到名稱對應；漏第 2 步 → Enable=0 形同無此點。

## 5. 自檢清單

- [ ] 電控雙輸出 → 是否建了 `_1`/`_2` 兩成員？
- [ ] 純計時缸的 On/Off 列位址留空且 Enable=0？
- [ ] 在席 sensor 與堆疊 sensor 沒有共用同一顆？
- [ ] database.h / IO_Table.csv / database.cpp 三處都改了？
- [ ] 命名沿用 Empty 慣例（`C_<Module>_<Func>Tray` / `Sn<Module>_<Pos>`）？
- [ ] 編譯驗證：`scripts\ops\build-ht160s.ps1 -Clean`。

## 6. 已知對照（Color，2026-06-09 以 IO_Table 實測校正）

> ⚠ 警告：舊版 section 6 曾把 10W11/10W12 誤標為台車缸；經 IO_Table 反推修正，
> 10W11=PushTray、10W12=LeanOnTray。下表以 IO_Table 實際位址為準。

| 電控 Color 點 | 程式成員 | IO_Table 位址 | 狀態 |
|---|---|---|---|
| 前頂Tray氣缸 輸出1 10W07 | `C_Color_FrontRiseTray_1` | `32,0,7` | ✅ |
| 前頂Tray氣缸 輸出2 10W10 | `C_Color_FrontRiseTray_2` | `32,1,0` | ✅ 雙缸第二顆 |
| 前頂Tray On/Off 11612/11613 | `_1` 到位 On/Off | `6,1,2` / `6,1,3` | ✅ |
| 前頂Tray 中位 11625 | `_2` 到位 On | `6,2,5` | ✅（_2 Off 留空計時）|
| PushTray 10W11, On 11615/Off 11614 | `C_Color_PushTray` | `32,1,1` | ✅ |
| LeanOnTray 10W12, On 11621/Off 11622 | `C_Color_LeanOnTray` | `32,1,2` | ✅ |
| 前分Tray氣缸_1 10W13 | `C_Color_FrontSeparateTray_1` | `32,1,3` | ✅（純計時 On/Off 留空）|
| （電控無此點）| `C_Color_RearRiseTray` | 無 | 殘留，待清 |

> 台車旋轉/滑台缸（A 案輔助定位，本輪未建模）的實際輸出線號需重新向電控確認，
> 不可沿用舊表的 10W11/10W12。
