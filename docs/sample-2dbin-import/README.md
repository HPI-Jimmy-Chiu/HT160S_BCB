# 2D→Bin 手動匯入範例檔

本資料夾的範例檔提供給 **主畫面 → WorkOrder → 「2D/Bin 手動編輯」分頁** 的
**Import（匯入）** 按鈕使用，內容完全比照程式內建的「載入模擬資料」
（`btnLoadSimuData`）。

對應程式碼：

- 匯入流程：`TfMain::btn2DImportClick`（main.cpp）
- 解析規則：`Split2DBinLine`（main.cpp）
- 模擬資料來源：`TfMain::btnLoadSimuDataClick`（main.cpp）

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

格式範例（每行 `Code,Bin`）：

```
2D_Simu_1,1
2D_Simu_2,2
2D_Simu_3,3
```

---

## 二、模擬資料規則（btnLoadSimuData）

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

## 三、檔案清單

| 檔名 | 內容 | 用途 |
|------|------|------|
| `2DBin_Import_Sample.csv` | 全部 100 筆（`2D_Simu_1`~`100`） | 一次匯入完整模擬資料 |
| `2DBin_Import_SIMU_LOT_A.csv` | LOT_A 的 20 筆 | 對應單一 Lot 匯入 |
| `2DBin_Import_SIMU_LOT_B.csv` | LOT_B 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_C.csv` | LOT_C 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_D.csv` | LOT_D 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_E.csv` | LOT_E 的 20 筆 | 同上 |
| `2DBin_Import_SIMU_LOT_A.tab.txt` | LOT_A 的 20 筆（**Tab 分隔**） | 驗證 Tab 分隔分支也能匯入 |

---

## 四、使用步驟

1. 主畫面切到 WorkOrder 的「2D/Bin 手動編輯」分頁。
2. （建議）先在 Lot 清單選好 / 輸入要匯入的 Target Lot。
3. 按 **Import**，於檔案對話框選擇本資料夾任一 `.csv` / `.txt`。
4. 表格會附加匯入的列；確認無誤後按 **Commit** 寫入該 Lot。

> 匯入是「追加」到目前表格，不會清空既有列；重複按會重複附加。

> 機台運轉中（`Is2DEditLocked`）禁止編輯/匯入，請先停機。
