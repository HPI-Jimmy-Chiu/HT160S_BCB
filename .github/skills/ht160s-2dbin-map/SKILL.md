---
name: ht160s-2dbin-map
description: >-
  Use when working on the HT160S_BCB 2D-code to Bin lookup ("dui zhang ben"):
  THT160Bin2DMap (CosFunction.cpp/.h), cJSON-based JSON parsing, the
  HT160S_LotInfo\yyyymm\dd\*.json delivery format, Top CCD bin assignment
  (ReadTopCcdBin / DoCcdCheck), TMyTray::iBin sorting-bin grid, and how this
  differs from the HT172 BarcodeSorter design. Triggers: THT160Bin2DMap,
  Bin2DMap, bUse2DBinMap, 2D Bin, dui zhang ben, ReadTopCcdBin, iBin, cJSON,
  HT160S_LotInfo, Code2D, LotNumber, BarcodeSorter.
---

# HT160S 2D Bin 對照本 (2D-code -> Bin Lookup)

## 1. 目的

客戶在 Top CCD 拍照取得每顆 IC 的 2D code 後，需要查一本「對照本」把
`(LotNumber, 2D code)` 對應到 **Bin 別**，再交給 `BinAreaMap` 路由到實體出料區。
若查無對應 → 跳 Alarm 並丟到 Error 區 (Bin 1001)。

資料來源演進：
- 現階段：**手動**建立 JSON 對照本檔案。
- 未來：客戶**雲端 / SECS** 每次提供新檔（檔名含 `HHmmss`，每次新增一個檔，不覆蓋）。

## 2. 資料雙層語意 (重要)

`TMyTray` (MotorAndIO/MyMotor.h) 有兩張 X-major 格點：

| 欄位 | 意義 | 值域 |
|------|------|------|
| `Data[x][y]` | IC 狀態 | `EMPTY_IC=0` / `UNCHECK_IC=1` / `HAS_OK_IC=2` (cmydef.cpp) |
| `iBin[x][y]` | 分選 Bin 別 (查 2D 對照本得到) | `0`=未指派 / `1..999`=正常 / `1000`=2D 掃描失敗 / `1001`=查無對照 |

特殊 Bin 常數定義在 CosFunction.h：
- `HT160_BIN_ERROR_2D_SCAN_FAIL = 1000`
- `HT160_BIN_ERROR_NO_BIN_SETTING = 1001`

### iBin 配套 API (MyMotor)
- `TMyTray`: `ClearBin()`, `SetAllBin(int)`, `SetBin(x,y,bin)`, `GetBin(x,y)`；
  建構式 / `Clear()` 會把 `iBin` 歸 0；`InitNewTray()` / `SetTray()` 會 `ClearBin()`。
- `TTrayMotor`: `SetTrayBin(x,y,bin)`, `GetTrayBin(x,y)` (無顯示副作用)。
- 注意：既有的 `TTrayMotor::SetPTrayData(x,y,iBin)` **其實寫的是 `Data`**（參數名誤導），
  不要把它當 iBin setter。要寫 Bin 用 `SetTrayBin()`。

## 3. JSON 對照本格式 (多 Lot)

- 路徑：`<HSys.CurrentDir>\HT160S_LotInfo\yyyymm\dd\<name>_HHmmss.json`
- 一個檔案可含**多個 Lot**，每個 Lot 多筆 `Code2D -> Bin`。
- `LoadLatest()` 掃今天資料夾，取**最新（檔案時間最大）**的 `*.json` 載入。

```json
{
  "Maps": [
    {
      "LotNumber": "LOT123",
      "Items": [
        { "Code2D": "ABC123", "Bin": 1 },
        { "Code2D": "ABD456", "Bin": 3 }
      ]
    }
  ]
}
```

## 4. 程式落點

| 物件 / 函式 | 檔案 | 角色 |
|-------------|------|------|
| `THT160Bin2DMap` / 全域 `Bin2DMap` | CosFunction.h/.cpp | 對照本載入 + 查詢 |
| `CosFunction.bUse2DBinMap` | CosFunction.cpp `InitialCosFunction()` | 功能旗標 (預設 true) |
| `cJSON` | `Include\cJSON.c/.h` | JSON parser（由 HT172 0420 移植） |
| `TLoaderModule::ReadTopCcdBin()` | aLoader.cpp | Top CCD -> IC 在位狀態（HAS_OK_IC/EMPTY_IC） |
| `TLoaderModule::ReadTopCcd2DCode()` | aLoader.cpp | **單一硬體接縫**：每格 2D 字串。stub 回 `bOk=false`，待硬體補 |
| `TLoaderModule::CurrentLotNumber` / `SetCurrentLotNumber()` | aLoader.cpp | 當前 Lot 號（HT160 原本無 Lot 字串，新增）。待 LotStart 設定 |
| `DoCcdCheck` case 5000 | aLoader.cpp | 已接 lookup（旗標+`HAS_OK_IC` 保護）；`ReadTopCcd2DCode` 未就緒前不寫 iBin |

### THT160Bin2DMap 介面
```cpp
bool Lookup(AnsiString LotNumber, AnsiString Code2D, int &Bin); // 查無回 false
bool LoadLatest();                 // 載入今天最新對照本
bool LoadFromFile(AnsiString f);   // 載入指定檔
int  GetEntryCount();
```
內部用 `TStringList`（`Sorted=true`），key = `LOT \x01 CODE2D`，`Objects[i]=(TObject*)Bin`。

## 5. 使用流程 (P3 已接線，inert 等待硬體)

P3 已接在 `DoCcdCheck` case 5000（旗標 `bUse2DBinMap` + `BinData==HAS_OK_IC` 保護）。
目前 **inert**：`ReadTopCcd2DCode` stub 回 `bOk=false` → 不寫 iBin，行為等同現狀。

啟用前兩個接縫待補：
1. `ReadTopCcd2DCode(LoaderNo,CellX,CellY,bOk)` 填入真正每格 Top CCD 取碼。
2. LotStart 流程呼叫 `SetCurrentLotNumber()` 設定當前 Lot 號。

接線後行為：
1. Top CCD 對每個 pocket 取得 2D code（`ReadTopCcd2DCode`）。
2. `if(CosFunction.bUse2DBinMap && BinData==HAS_OK_IC)`：用 `(CurrentLotNumber, code2D)` 呼叫 `Bin2DMap.Lookup()`。
3. 命中 → `TrayMotor->SetTrayBin(x,y,bin)`。
4. 查無 → `SetTrayBin(x,y,1001)`（目前僅寫 iBin，未報警/未導 Error 區；啟用時再決定 K_SKIP/Alarm 行為）。
5. SortArm 端讀 `GetTrayBin()` → 餵 `BinAreaMap.GetAreaByBin()` 決定實體出料區 (P4)。

## 6. 與 HT172 BarcodeSorter 的差異

| 面向 | HT172 BarcodeSorter | HT160S 2D Bin 對照本 |
|------|---------------------|----------------------|
| 粒度 | 每**盤** (ClipID, tray-level) | 每**顆 IC** (Code2D, cell-level) |
| 傳輸 | FTP + `D:\BarcodeSorter\batch.txt` | 本機 JSON 檔（未來 SECS / 雲端） |
| 結構 | `iBinCode[][]` / `DeviceInfo[][]` + TrayMap 欄位 | `Data[][]`(狀態) + `iBin[][]`(Bin) 雙層 |
| 解析 | cJSON | cJSON（同一套，已移植） |

## 7. BCB6 / 工程約束

- 純程序式，**不可** FSM；無 `Sleep()`；非阻塞。
- 原始檔註解一律 **ASCII English**（Big5 區避免 VS Code 編輯器破壞）。
- 新增 .c/.cpp 必須同步更新 `ht160s.bpr` 的 `OBJFILES` + `FILELIST`
  （`.mak` 由 `bpr2mak` 自動產生，build 腳本會處理）。
- 驗證：`powershell -ExecutionPolicy Bypass -File scripts\ops\build-ht160s.ps1 -Clean`。
- cJSON 編譯設定比照 HT172（`Include\cJSON.c`，CONTAINERID `CCompiler`，預設 `dllexport __stdcall`，已驗證可 link）。
