---
name: ht160s-bindisplay
description: >-
  Use when working on the HT160S_BCB Bin Display subsystem (TMyBinDispCtrl /
  TMyBinDispHT9046 in HT160S_Program_BCB_V1.0.0.0/MyBinDisp.cpp/.h): the
  HT9046 LED display COM/SPComm protocol, the Modbus-ASCII frame formats
  (WriteBin / WriteColor / ReadVersion), the iVersion (old=1 / new=2) module
  detection, the reply-checkword validation in DoStartSetBin / DoStartSetColor /
  DoStartGetStatus, the Addr+32/+38 single-high-slave addressing, the
  iBinDispCtrlTask state machine pumped by Spin(), and integration via
  HSys.BinDisCtrl + TfComPort. The AUTHORITATIVE protocol reference is the HT9045
  source (HT9011UC). Triggers: BinDisplay, MyBinDisp, TMyBinDispCtrl,
  TMyBinDispHT9046, HT9046, BinDisCtrl, WriteTargetBin, WriteBin, WriteColor,
  ReadVersion, DoStartSetBin, DoStartSetColor, DoStartGetStatus, A_Create_LCR,
  iVersion, LED bin display, General.ini [BinDisplay], ConfigureBinDisplay.
---

# HT160S Bin Display (HT9046 LED, TMyBinDispCtrl)

## 1. 目的與現況

HT160S 的 Bin 顯示器子系統位於 `HT160S_Program_BCB_V1.0.0.0/MyBinDisp.cpp/.h`，
由參考源移植而來，**只保留 HT9046 LED 路徑**（單一 COM / SPComm），單一實例
`HSys.BinDisCtrl`（`TMyBinDispHT9046`）。整套移植與整合決策見 repo memory
`bindisplay-port-plan.md`（P0–P6 全部完成）。

- 硬體經 COM port 連線（9600-8-N-1，baud 可在 General.ini 設定），不是 TCP。
  舊 160 的 `D:\MCU\MCU.exe` 只是外掛 TCP→COM 橋接，已整個移除。
- 位址圖（9 unit，`BIN_DISP_UNIT_COUNT=9`）：
  `0=Empty 1=Loader 2..7=Auto1..6 8=Color`。
- 行為 = 開機 / 套用時推一次靜態 per-unit 標籤（舊 160 同等行為），分選過程
  **不做** per-IC live update（HT172 才有 live `WriteTargetBin`；HT160 已確認不需要）。

> **權威協定參考（read-only）：** HT9045 原始碼
> `D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\BinDisplay\`
> （`MyBinDisp.cpp/.h`、`BinDisp.cpp/.h` 測試 form、`BinDispTester.bpr`）。
> 通訊命令以 9045 為對齊基準。HT172 的同檔 **不可**當權威 — 它用了錯誤的
> `Addr+1` 低位址（見第 4 節），9045 才是對的。

## 2. 類別架構

- `TMyBinDispCtrl`（抽象基底）：狀態機資料、COM 包裝、`ProcessStopStart`、
  `SetComPort/SetComParity/SetDelayTime`、`InstalledUnit`、`WriteTargetBin`、
  `Spin()`（HT160）/ `Timer1Timer`（9045）泵動；純虛擬 `WriteBin/WriteColor/
  ReadVersion/DoStartSetBin/DoStartSetColor/DoStartGetStatus` 由子類實作。
- `TMyBinDispHT9046`（LED 子類）：實作上述 HT9046 Modbus-ASCII 命令。
- 9045 額外有：`CommBin2`/雙鏈、Magazine（HTA18/BT008）、TFT（`*_TFT`、
  `command_TFT_*`、`iAddArrayTFT`、`NUMBER_PANEL_TYPE==4`）—— **HT160 目前只移植 LED**。
  Magazine 不移植；TFT 之擴充設計（兄弟子類 + `ProcessTick()` 介面切點，**不採用**
  9045 的 `NUMBER_PANEL_TYPE` 散落分支）見第 10 節。

## 3. HT9046 通訊命令（Modbus-ASCII over COM）

Frame：`:` + 12 個資料字元 + 2 字元 LRC + `CR(0x0D) LF(0x0A)`。
LRC = `A_Create_LCR(&SendBuffer[1], 12)`，再用 `T_HEX2ASCII_Mac` 拆成
高/低 nibble 放到 `SendBuffer[13]/[14]`。`Bin_CR=13`、`Bin_LF=10`。

**從機位址 = 單一高位址**（見第 4 節）：`wAddr = (Addr>=10)? Addr+38 : Addr+32`，
以 `%02X` 印出。9045 與 HT160 三個送出命令（格式逐字相同）：

| 命令 | 格式字串 | 暫存器 / 意義 |
|------|----------|----------------|
| WriteBin   | `:%02X06008%d00%02d00` | 功能碼 06；register `008X`：`Command=0`→0080 數字、`Command=1`→0081 字母；value `%02d` |
| WriteColor | `:%02X06008200%02d00`  | register 0082；value = 顏色碼（1=R,2=G,3=R+G） |
| ReadVersion| `:%02X030080000100`    | 功能碼 03；讀 register 0080 一個字，用來偵測模組版本 |

送字編碼（`WriteTargetBin` → `iSetBin`）：`-1`=空白（顯示 X，用字母 123→`WriteBin(.,1,23)`）、
`0..99`=數字、`100..125`=字母 A..Z（`value-100`）。

9045 命令對照表（檔內註解 line 560-565）：
```
//0 Read Status : 2,X,0,3,0,0,8,0,0,0,0,1  XX,XX \R\N
//1 Write Num   : 2,X,0,6,0,0,8,0,0,0,X,X  XX,XX \R\N  0~99
//2 Write Eng   : 2,X,0,6,0,0,8,1,0,0,X,X  XX,XX \R\N  A~Z = 0~26
//3 R LIGHT     : 2,X,0,6,0,0,8,2,0,0,0,1
//4 G LIGHT     : 2,X,0,6,0,0,8,2,0,0,0,2
//5 R+G LIGHT   : 2,X,0,6,0,0,8,2,0,0,0,3
```

## 4. 關鍵：HT9046 是單一高位址從機（位址不可用 Addr+1）

真實 HT160 / HT9045 機台上，HT9046 板是 **一個** Modbus 從機，位址落在
**高（顏色）區段**；bin / version / color **只靠 register 區分（0080/0081/0082）**，
不是靠不同位址。

- 9045 `WriteBin` / `WriteColor` / `ReadVersion` **全部**用 `Addr+32`（或
  `Addr+38`，當 `Addr>=10`）+ `%02X`。
- HT172 的舊碼對 `WriteBin` / `ReadVersion` 誤用 `Addr+1`（`%02d`，低位址 01-09），
  只有 `WriteColor` 用高位址 —— 在此硬體上低位址無人應答，造成 `iVersion=0`、
  進而所有 bin/color reply 都被當成 NoReply 丟棄。
- HT160 已修正成與 9045 一致（`MyBinDisp.cpp` WriteBin ~513 / ReadVersion ~552 /
  各 checkword）。**新增/修改命令時務必沿用 `Addr+32/+38`，勿退回 `Addr+1`。**

## 5. iVersion 模組版本（舊=1 / 新=2）—— reply checkword 依版本不同

`DoStartGetStatus` 先送 `ReadVersion`，再依回覆判定版本：
- reply `Pos(":%02X03020001")==1` → `iVersion=1`（舊模組）
- reply `Pos(":%02X03020002")==1` → `iVersion=2`（新模組）
- 都不符 → `iVersion=0`（視為無回應）

之後 `DoStartSetBin` / `DoStartSetColor` 的 reply 驗證 checkword 依版本分支：

```
// Bin reply (DoStartSetBin):
iVersion==1: ":%02X06020010"
iVersion==2: 空白 -> ":%02X060201%02d"(23)
             數字 -> ":%02X060200%02d"(value)
             字母 -> ":%02X060201%02d"(value-100)
// Color reply (DoStartSetColor):
iVersion==1: ":%02X06020010"
iVersion==2: ":%02X060202%02d"(iSetColor)
```

驗證方式：`sReadBuffer.Pos(sCheckWord)==1`（回覆 ASCII 字串前綴比對）。
`%02X` 的位址同樣用 `wAddr=(Addr>=10)?Addr+38:Addr+32`。

> 9045 在 checkword 用 `iAddrFix`（針對其 Fix1-12 / Auto4-6 的 bin 佈局做位址
> 重映射）。HT160 是單純 0-8 佈局，`iAddrFix==Addr`，故直接用 `Addr` 即正確。

## 6. 狀態機與接收

- 泵動：HT160 由 `TfComPort::Spin()` 呼叫 `HSys.BinDisCtrl->Spin()`；`Spin()`
  在 `InitialOK==true && bStopProcess==true` 才跑（`ProcessStopStart(true)` 設
  `bStopProcess`，且只在 `bBinDisplayInstalled` 勾選時被呼叫）。
  task 1 用 `GetCOMPortStatus()` 獨佔開埠；開不成會卡在 task 1，零位元組送出。
- 流程：GetStatus（讀版本）→ SetColor → SetBin，各自 `switch(Task)` case
  1/100/200/1000。case 200 等 `BinDispRecv`（由 `CommBinReceiveData` 設）或
  `BinDisDelay.Off()` 逾時。
- 接收：LED 路徑 `sReadBuffer` = 原始 ASCII 回覆字串（如 `:2606020203CD`）。
- **HT160 強化（與 9045 不同）：** 任一 unit 連續 >2 次無回應時 **記 log + 標記
  該 unit 錯誤 + 跳下一位址**（`BinNoReply`/`ColorNoReply`/`VerNoReply`），不像
  9045 會卡住重試整條匯流排。

## 7. 整合點（HT160S_BCB）

- 物件：`database.cpp` `HSys.BinDisCtrl = new TMyBinDispHT9046;`
  （`database.h` SYSTEM_MODULAR 成員 + 前置宣告）。
- COM 設定：`ComPort.cpp` `ConfigureBinDisplay()`（設 baud-8-N-1、wire
  `CommBin`、`SetUsedBinNumber(9)`、`InstalledUnit(0..8)`、`ApplyBinDisplayConfig()`
  推 9 unit 標籤、`InitialOK=true`，僅在 installed 時 `ProcessStopStart(true)`）。
  由 `TfComPort` ctor 呼叫；`StopAllCom()` 停 `BinComm`。
- 設定檔：`General.ini [BinDisplay]`：`bBinDisplayInstalled`、`sBinDispComPort`
  (def COM5)、`iBinDispBaud`(def 9600)、`iBinDispDelaySec`(def 5)、
  `Text0..8` / `Color0..8`（per-unit 標籤與顏色碼，預設 Empty=E Loader=L
  Auto1-6=1..6 Color=C，color=3）。在 `GeneralSetting.cpp` 載入/存檔。
- 維護頁：repurpose 自舊 TCP「MCU Display」頁（`tsMaintMCUDisplay`，handler 名稱
  仍含 "MCUDisplay" 但驅動 `HSys.BinDisCtrl`）：COM/Baud/Delay/Installed 設定 +
  SendDisplay/SendCode/SendLight 手動測試鈕。
- log：`slBinDispLog` → `BinDisplayLog.txt`（`LogBinDisplay`）。維護頁螢幕 memo
  只記 UI 按鍵，**不是** COM TX/RX —— memo 沒看到 Recv 不代表裝置沒收到。

## 8. 獨立串列測試工具（不需編譯）

`D:\AI_Area\Tool\HT160S_SECS_Simulator\BinDisplayTester\Test-BinDisplay.ps1`：
直接開 COM 送 1:1 移植的 HT9046 frame（WriteBin/WriteColor/ReadVersion +
`A_Create_LCR`），`d) DIAGNOSE` 掃 u0..8 並判定 PASS / garbage=baud /
no-answer=wiring。用來把硬體/接線/協定問題從 HT160 Spin 架構中隔離。

## 9. 規則提醒

- 遵守 `bcb6-coding-style`：無 C++11、無 FSM（`switch(Task)` 程序式 OK）、
  每次改 .cpp/.dfm 後刪 .obj 再編譯。
- 9045 / HT172 為 read-only 參考；任何修改只落在 `D:\HT160S_BCB`。
- 新註解用 ASCII 英文（BCB6 source 為 Big5，勿引入 UTF-8 mojibake）。
- 改通訊命令前，先回讀 9045 對應 frame 確認格式，再對齊 HT160。

## 10. LED/TFT 介面層擴充計畫（設計，未實作）

完整設計草案：[`docs/plan/bindisplay-led-tft-interface-design.md`](../../../docs/plan/bindisplay-led-tft-interface-design.md)。

要點：
- LED（HT9046 Modbus-ASCII）與 TFT（HT9011 20-byte 繪圖框）是**兩套協定**，不能共用 frame builder。
- HT160 已有抽象基底 `TMyBinDispCtrl`，但 `Spin()` 狀態機與那組純虛擬是「LED 形狀」，TFT 套不進。
- 方案：把整個 tick 變成唯一跨協定虛擬 `virtual void ProcessTick()=0`；基底 `Spin()` 只做
  run-gate 後委派。新增**兄弟子類** `TMyBinDispTFT`，與 `TMyBinDispHT9046` 並列；
  LED 形狀的純虛擬（`WriteBin/.../DoStartGetStatus`）**下放** LED 子類，基底回歸協定無關。
- 選型：`General.ini [BinDisplay] PanelType`（0=LED 預設 / 1=TFT）+ `database.cpp` 工廠
  `new TMyBinDispTFT / TMyBinDispHT9046`；`ComPort/maintenance/TfComPort` 只認 `HSys.BinDisCtrl`
  基底指標，切換零分支。
- **不採用** 9045 的單一肥子類 + `NUMBER_PANEL_TYPE==4` 散落分支模型。
- 分階段：T1 純重構（抽 `ProcessTick`、LED 方法下放，行為不變，可獨立交付）→ T2 TFT 空骨架+工廠+
  PanelType key → T3 移植 TFT frame builder（`command_TFT_Input/Font`、`A_Create_LRC`、
  `WriteBin_TFT` 家族、`iAddArrayTFT`）→ T4 TFT `ProcessTick`（DoOnce/DoCycle）→ T5 維護頁 →
  T6 上機。T1/T2 不依賴 TFT 硬體可先做；T3 起待硬體需求確認（見草案 §8 開放決策）。
