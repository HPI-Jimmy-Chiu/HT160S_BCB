# 2D→Bin 手動匯入範例檔

本資料夾的範例檔提供給 **主畫面 → WorkOrder → 「2D/Bin 手動編輯」分頁** 的
**Import（匯入）** 按鈕使用，內容完全比照程式內建的「載入模擬資料」
（`btnLoadSimuData`）。

對應程式碼：

- 匯入流程：`TfMain::btn2DImportClick`（main.cpp）
- CSV 解析規則：`Split2DBinLine`（main.cpp）
- JSON 解析：`THT160LotRegistry::LoadFromJsonString` / `LoadFromJsonFile`（CosFunction.cpp，底層 cJSON）
- 模擬資料來源：`TfMain::btnLoadSimuDataClick`（main.cpp）

---

## 零、格式總覽（為什麼有 CSV 也有 JSON）

問題：真實 2D 碼可能含 **逗號 / 空格 / 引號**，而舊的 CSV 解析（`Split2DBinLine`
「先逗號、再 Tab」切兩欄）遇到含逗號的碼就會被切錯。決策採 **hybrid**：

| 格式家族 | 用途 | 對含特殊字元的碼 |
|----------|------|------------------|
| **JSON（Maps schema）** — 本資料夾 `2DBin_Import_*.json` | 正式 / 機台 / 多-Lot / 與 `WorkOrder.json` round-trip | **完全安全**（cJSON 自動處理逗號/空格/引號/Tab） |
| **CSV（RFC-4180 引號版）** — 本資料夾 `*.csv` | 操作員用 Excel 手編、單 Lot 進 grid | 含特殊字元的欄位須用 `"…"` 包住才安全 |

> **例外：`WhiteList.sample.json` 不是 Maps schema。** 白名單走客戶格式（根 `QRCodeIDHis` →
> 每 Lot `LOTID` / `KYECLotID` / `Substage` / `ProductCode` / `ICInfo[{QRCodeID, HBin}]`），
> 且 `LOTID`、`KYECLotID` 皆為必填——機台在 Lot Start 會逐 Lot 檢查，缺一即整份檔案退回。
> 用 Maps schema 寫白名單一定會被拒（該 schema 沒有 `KYECLotID` 欄）。
> 規格見 `D:\HT160S_BCB\docs\whitelist\KYEC-WhiteList-Interface-Spec.md`，
> 佈署路徑 `D:\HT160S_BCB\HT160S_WhiteList\WhiteList.json`。

> **狀態（重要）**：本資料夾的 **JSON 範例檔已就緒**，但主畫面 Import 按鈕
> 目前**只解析 CSV**（走 `Split2DBinLine`）。要讓 `.json` 能被匯入，需在
> `btn2DImportClick` 加一段依副檔名分流、對 `.json` 直接呼叫
> `LotRegistry.LoadFromJsonFile(...)` 的分支（約 15 行）；CSV 的引號解析
> 則需新增讀取端 tokenizer。**兩者皆為尚待實作的程式接線**，需經編譯閘與上機驗證。
> 在接線完成前，這些 `.json` 僅作為格式規格與測試 fixture。

---

## 一、匯入檔格式（Import 能成功讀取的規則）

`btn2DImportClick` 會用 `TStringList::LoadFromFile` 逐行讀檔，每一行交給
`Split2DBinLine` 解析，成功的行會被「附加」到編輯表格 `sg2DBinEdit`（兩欄：
`2D Code` / `Bin`），操作員確認後再 Commit 到目前選取的 Target Lot。

解析規則：

| 規則 | 說明 |
|------|------|
| 欄位分隔 | 先找 **逗號 `,`**，找不到才找 **Tab `\t`** |
| 第 1 欄 | 2D Code（會 Trim 去頭尾空白） |
| 第 2 欄 | Bin（會 Trim） |
| 空行 | 直接略過 |
| 無分隔符號的行 | 整行當作 Code，Bin 留空 |
| **表頭列** | **不要加**。像 `2D Code,Bin` 這種表頭會被當成一筆資料匯入成垃圾列 |

> 檔案編碼用 ASCII / ANSI 即可（範例 Code 皆為 `2D_Simu_N`，純 ASCII）。
> 換行 CRLF 或 LF 都可（`TStringList` 兩者皆可解析），本範例使用 Windows 慣用的 CRLF。

格式範例（每行 `Code,Bin`，純資料無特殊字元時免引號）：

```
2D_Simu_1,1
2D_Simu_2,2
2D_Simu_3,3
```

### RFC-4180 引號規則（碼含逗號/空格/引號時）

若 2D 碼含 **逗號、雙引號、換行**，該欄位須以雙引號 `"…"` 包住，內部雙引號
用兩個 `""` 跳脫（＝ Excel 預設行為）。純資料列不含特殊字元時**免引號**，
所以既有 `*.csv` 範例維持原樣即可。

範例（第 3 列的碼實際為 `AB,12 "X9"`，Bin 為 `1000`）：

```
2D_Simu_1,1
2D_Simu_2,2
"AB,12 ""X9""",1000
```

> ⚠️ 引號解析為**尚待實作**的讀取端 tokenizer（現行 `Split2DBinLine` 只做
> 「逗號/Tab 切兩欄」，尚不支援引號；未實作前，含逗號的碼用純 CSV 匯入仍會被切錯）。
> **切勿**改用「從最後一個逗號切」的偷吃步——會把 code-only 列與引號欄位靜默切到錯 Bin。

---

## 二、JSON（Maps）匯入格式

`.json` 走 `THT160LotRegistry::LoadFromJsonString`（底層 cJSON），對逗號 / 空格 /
引號 / Tab **完全免煩惱**（JSON 字串規則自動處理）。本資料夾採 **Maps 精簡 schema**：

```json
{
  "Maps": [
    {
      "LotNumber": "SIMU_LOT_A",
      "Items": [
        { "Code2D": "2D_Simu_1", "Bin": 1 },
        { "Code2D": "2D_Simu_2", "Bin": 2 }
      ]
    }
  ]
}
```

規則（易踩雷）：

| 規則 | 說明 |
|------|------|
| 根鍵 | `"Maps"`，值為陣列，每元素一個 Lot |
| Lot 欄位 | `"LotNumber"`（字串）+ `"Items"`（陣列） |
| `"Code2D"` | 2D 碼（字串；含逗號/空格皆可，內部雙引號寫成 `\"`） |
| `"Bin"` | **必須是數字** `1`；**不可**寫成字串 `"1"`（cJSON 判為非數字會**整筆略過**） |
| Lot 名稱 | 由檔案自帶（不像 CSV 靠 UI 的 Target Lot） |
| trailing comma | **不可**（cJSON 嚴格；多一個逗號整檔解析失敗） |
| 多 Lot | 一檔可放多個 Lot 物件（見 `2DBin_Import_Sample.json`） |

> 另有 `2DIDHistory` 完整 schema（＝ `WorkOrder.json`，`HBin`/`SBin` 為**字串**），
> 同一個 `LoadFromJsonString` 也能解析；本資料夾選用較好手編的 Maps 版。

---

## 三、模擬資料規則（btnLoadSimuData）

- 共 **5 個 Lot**：`SIMU_LOT_A` ~ `SIMU_LOT_E`
- 每個 Lot **20 筆**，全機共 **100 筆**，序號 `CodeSeq = 1..100`
- Code = `"2D_Simu_" + CodeSeq` → `2D_Simu_1` ~ `2D_Simu_100`（不重複）
- Bin = `((CodeSeq-1) % 6) + 1` → 依序循環 `1,2,3,4,5,6,1,2,...`

各 Lot 起始序號與起始 Bin：

| Lot | 序號範圍 | 起始 Bin | 該段 Bin 序列 |
|-----|----------|----------|----------------|
| SIMU_LOT_A | 2D_Simu_1 ~ 20 | 1 | 1,2,3,4,5,6,1,2,3,4,5,6,1,2,3,4,5,6,1,2 |
| SIMU_LOT_B | 2D_Simu_21 ~ 40 | 3 | 3,4,5,6,1,2,... |
| SIMU_LOT_C | 2D_Simu_41 ~ 60 | 5 | 5,6,1,2,3,4,... |
| SIMU_LOT_D | 2D_Simu_61 ~ 80 | 1 | 1,2,3,4,5,6,... |
| SIMU_LOT_E | 2D_Simu_81 ~ 100 | 3 | 3,4,5,6,1,2,... |

> 注意：Bin 是「全機連續循環」而非「每個 Lot 重新從 1 開始」，所以 B/C/E
> 的起始 Bin 不是 1（承接前一段的循環）。

---

## 四、檔案清單

**CSV（現行可匯入；RFC-4180 引號版待接線）**

| 檔名 | 內容 | 用途 |
|------|------|------|
| `2DBin_Import_Sample.csv` | 全部 100 筆（`2D_Simu_1`~`100`） | 一次匯入完整模擬資料 |
| `2DBin_Import_SIMU_LOT_A.csv` | LOT_A 的 20 筆 | 對應單一 Lot 匯入 |
| `2DBin_Import_SIMU_LOT_B.csv` | LOT_B 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_C.csv` | LOT_C 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_D.csv` | LOT_D 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_E.csv` | LOT_E 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_A.tab.txt` | LOT_A 的 20 筆（**Tab 分隔**） | 驗證 Tab 分隔分支也能匯入 |

**JSON（Maps schema；code/bin 與對應 CSV 逐筆相同）**

| 檔名 | 內容 | 用途 |
|------|------|------|
| `2DBin_Import_Sample.json` | 5 個 Lot × 20 筆 = 100 筆（多 Lot 單檔） | JSON 多-Lot 匯入示範 |
| `2DBin_Import_SIMU_LOT_A.json` | LOT_A 的 20 筆（單 Lot） | 對應單一 Lot 匯入 |
| `2DBin_Import_SIMU_LOT_B.json` | LOT_B 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_C.json` | LOT_C 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_D.json` | LOT_D 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_E.json` | LOT_E 的 20 筆 | 同上 |
| `2DBin_Import_HostileCode.json` | 5 筆，碼含**逗號/空格/雙引號** | 對抗性 fixture：驗證特殊字元 round-trip |

> `2DBin_Import_SIMU_LOT_F.csv.bak-20260630-spurious-dupE` 是與 LOT_E 逐位元相同的
> 誤複製檔，已隔離，非正式範例。

---

## 五、使用步驟

1. 主畫面切到 WorkOrder 的「2D/Bin 手動編輯」分頁。
2. （建議）先在 Lot 清單選好 / 輸入要匯入的 Target Lot（**JSON 檔自帶 Lot 名稱**）。
3. 按 **Import**，於檔案對話框選擇本資料夾任一 `.csv` / `.txt`（JSON 匯入待接線後才支援 `.json`）。
4. 表格會附加匯入的列；確認無誤後按 **Commit** 寫入該 Lot。

> 匯入是「追加」到目前表格，不會清空既有列；重複按會重複附加。

> 機台運轉中（`Is2DEditLocked`）禁止編輯/匯入，請先停機。
