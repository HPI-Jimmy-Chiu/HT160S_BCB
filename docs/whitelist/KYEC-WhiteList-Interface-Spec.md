# HT160S 白名單分選功能 — 客戶介接規格書

| 項目 | 內容 |
|---|---|
| 設備 | HT160S IC 分類機（Sorter） |
| 對象 | 客戶 EAP / IT 介接端 |
| 版本 | v1.1（2026-07-27） |

---

## 1. 功能概述

白名單分選（By WhiteList）為特殊分選模式。啟用後，機台僅處理白名單檔案內列出的 2D 碼：

- **名單內的 IC**：依檔案指定的 Bin 分選至對應料盤。
- **名單外的 IC**（讀得到 2D 碼但不在名單內）：判定為拒收，送入 Error 區，並記錄於生產報表；不彈出操作員視窗、不中斷生產。

白名單為臨時覆蓋（overlay）模式，隨單一 Lot 生效：

- 於 Lot Start 啟用，於 Lot End 自動還原為機台原本的基礎分選模式。
- 每一個需要白名單的 Lot 皆須於啟動時明確指定；未指定者以基礎模式生產。

**By WhiteList 不是機台「基礎分選模式」的第四個選項。** 機台維護畫面的 Sort Mode 選擇器固定為三選一
（Normal／By Lot+Bin／By Lot+PassFail，此為基礎模式、開機沿用）；白名單是疊加在其上的臨時覆蓋，
啟用時取代分選行為，Lot End 後基礎模式原封不動地回來。

---

## 2. 模式切換（SECS）

### 2.1 指令

主機以 `S2F41 Host Command`、指令名 `LOTSTART`，在 Lot 清單中夾帶一組 `SORTMODE` 選項對切換：

| 選項對值 | 效果 |
|---|---|
| `WHITELIST` | 本 Lot 套用白名單覆蓋 |
| `NORMAL` | 本 Lot **不**套用白名單覆蓋，維持機台維護畫面 Sort Mode 選擇器的原設定（可能是 Normal、By Lot+Bin 或 By Lot+PassFail） |

- 值不分大小寫。
- `NORMAL` 指的是「不加白名單」，**不代表把基礎模式設成 Normal**；機台的基礎模式由現場人員在維護畫面設定，主機不會、也無法用這組選項對去改它。
- 省略 `SORTMODE` 選項對時，等同 `NORMAL`（不套用白名單，使用基礎模式）。
- **每個白名單 Lot 都必須帶 `SORTMODE = WHITELIST`。**
- 選項對隨 Lot 一併送出；一次 `LOTSTART` 至多一組 `SORTMODE`，重複時以最後一組為準。

### 2.2 電文範例

啟用白名單並開 Lot：

```
S2F41 W
<L [2]
  <A "LOTSTART">
  <L [2]
    <A "CUSTLOT0001">
    <L [2] <A "SORTMODE"> <A "WHITELIST">>
  >
>
```

使用基礎模式並開 Lot：

```
S2F41 W
<L [2]
  <A "LOTSTART">
  <L [2]
    <A "CUSTLOT0002">
    <L [2] <A "SORTMODE"> <A "NORMAL">>
  >
>
```

### 2.3 回覆碼（HCACK）

| HCACK | 意義 | 條件 |
|---|---|---|
| 0 | 接受；模式已切換、Lot 已建立 | 清單解析成功且設備閒置 |
| 1 | 電文結構錯誤 | 外層非預期清單 |
| 2 | 參數錯誤 | 選項對長度非 2；名稱非 `SORTMODE`；值非 `NORMAL` 或 `WHITELIST`；有選項對但無 Lot；清單截斷；Lot 數超過 64 |
| 4 | 設備忙碌（整包拒絕） | 設備生產中、機台內有在製 IC、或該 Lot 已啟動 |

任何錯誤或忙碌拒絕時，不建立任何 Lot、不改變模式。

### 2.4 模式回讀（SVID 66032）

主機以 `S1F3` 查詢 SVID `66032`（INT4），回覆目前**有效**分選模式（基礎模式 + 白名單覆蓋合併後的結果）：

| 回值 | 意義 | 來源 |
|---|---|---|
| 0 | Normal | 基礎模式（維護畫面 Sort Mode 選擇器） |
| 1 | By Lot + Bin | 基礎模式（維護畫面 Sort Mode 選擇器） |
| 2 | By Lot + PassFail | 基礎模式（維護畫面 Sort Mode 選擇器） |
| 3 | By WhiteList 覆蓋生效中 | 臨時覆蓋（不屬於 Sort Mode 選擇器） |

- `0 / 1 / 2` 是機台三選一的**基礎模式**；`3` 不是選擇器的第四個選項，而是「白名單覆蓋已生效」這件事，此時基礎模式仍在背後保留著。
- 白名單 Lot 進行中回報 `3`；Lot End 自動還原後，回報值回到該機台的基礎模式（`0`、`1` 或 `2`）。
- **`SORTMODE = NORMAL` 的 Lot 不保證回報 `0`。** `NORMAL` 只代表不套白名單，回讀值取決於現場 Sort Mode 選擇器的原設定，可能是 `0`、`1` 或 `2`。主機若要確認「白名單有沒有生效」，判斷準則是 `= 3` 或 `≠ 3`，不要用 `= 0` 當作 NORMAL 的檢查條件。

---

## 3. 白名單檔案

### 3.1 路徑與規則

| 項目 | 值 |
|---|---|
| 資料夾（絕對路徑） | `D:\HT160S_BCB\HT160S_WhiteList\` |
| 完整檔案路徑 | `D:\HT160S_BCB\HT160S_WhiteList\WhiteList.json` |
| 路徑由來 | 白名單資料夾恆為 `<程式根目錄>\HT160S_WhiteList\`；機台軟體安裝根目錄為 `D:\HT160S_BCB`（含 `EXE\`、`system\`，由安裝程式固定佈署），故絕對路徑如上 |
| 檔名 | 固定 `WhiteList.json`（Windows 檔名不分大小寫；不可改名） |
| 編碼 | UTF-8 純文字 |
| 存取 | 機台只讀不寫；資料夾於第一次執行白名單 Lot Start 時自動建立（機台若從未跑過白名單，此資料夾可能尚未存在，手動建立即可） |
| 載入時機 | 每次 Lot Start 載入一次，整個 Lot 常駐 |

檔案為權威名單。Lot Start 時若檔案不存在、無法解析或不符 §3.2 規則，機台**不載入任何 2D 資料**，
並在按下生產 **Start**（或 One Cycle）時擋下、顯示明確原因。

> 注意：被擋下的是**生產啟動**，不是 Lot Start 這個動作本身。Lot Start 仍會建立工單並進入生產待命，
> 只是名單是空的；請以 Start 的提示訊息（或機台 Process 記錄中的 `WhiteList: ...` 行）判斷檔案是否被接受。

### 3.2 檔案格式

根物件含一個 `QRCodeIDHis` 陣列，每個元素代表一個 Lot，內含 Lot 資訊與 `ICInfo` 陣列（逐顆 IC）。

| 層級 | 欄位 | 內容 | 型別 | 必填 |
|---|---|---|---|---|
| Lot | `LOTID` | 客戶批號 | 字串（不可空） | 是 |
| Lot | `Substage` | 站點 | 字串 | 是 |
| Lot | `ProductCode` | 產品型號 | 字串 | 是 |
| Lot | `KYECLotID` | 京元內部批號 | 字串（不可空） | 是 |
| Lot | `ICInfo` | 該 Lot 的 IC 陣列 | 陣列 | 是 |
| IC | `QRCodeID` | IC 2D 碼 | 字串 | 是 |
| IC | `HBin` | 分選 Bin | 字串 | 是 |

格式規則：

- `HBin` 須為**字串**（例：`"1"`），其編號對應機台已設定之 Bin。
- `QRCodeID` 以完全一致（含大小寫）方式比對 CCD 讀值。
- 同一檔案內 `QRCodeID` 不可重複。
- `LOTID` 不可為空。
- **`KYECLotID` 為必填、不可為空**：它是該批白名單所屬的京元批號，機台於載入時記錄。
  只要檔案中**任何一個** Lot 缺少 `KYECLotID`（欄位不存在、空字串或只有空白），整份檔案視為無效 → 機台不載入任何資料。
- 欄位名稱不分大小寫；`QRCodeID` 之值分大小寫。

**機台強制檢查項目**（不符即整份檔案退回，不載入任何資料，並於生產 Start 時顯示對應訊息）：

| 檢查 | 訊息 |
|---|---|
| 檔案存在 | `WhiteList file not found : <路徑>` |
| 可解析為 JSON | `WhiteList file is not valid JSON : <路徑>` |
| 根物件有 `QRCodeIDHis`（或 `2DIDHistory`）陣列 | `WhiteList file has no QRCodeIDHis lot list (customer format required)` |
| 至少一個 Lot | `WhiteList file contains no lot data` |
| 每個 Lot 有非空 `LOTID` | `WhiteList rejected : lot #<序號> has no LOTID (mandatory) !` |
| 每個 Lot 有非空 `KYECLotID` | `WhiteList rejected : lot <LOTID> has no KYECLotID (mandatory) !` |

其餘欄位（`Substage`、`ProductCode`）與 `QRCodeID` 唯一性屬**資料品質要求**：機台不會因此退回檔案，
但缺漏會直接反映在生產報表（欄位空白）；重複的 `QRCodeID` 只保留第一筆、其餘忽略（僅記錄於機台 Process 記錄）。
`HBin` 若誤寫成數字（`1` 而非 `"1"`）機台讀不到，該顆會被視為 Bin 0，請務必用字串。

### 3.3 範例檔

```json
{
  "QRCodeIDHis": [
    {
      "LOTID": "CUSTLOT0001",
      "KYECLotID": "KYEC-INTERNAL-9001",
      "Substage": "FT1",
      "ProductCode": "TESTDEV-A123",
      "ICInfo": [
        { "QRCodeID": "WLTEST2D0001", "HBin": "1" },
        { "QRCodeID": "WLTEST2D0002", "HBin": "2" },
        { "QRCodeID": "WLTEST2D0003", "HBin": "3" }
      ]
    }
  ]
}
```

---

## 4. 名單內／外處理

| 情況 | 處理 |
|---|---|
| 2D 讀取成功且在名單內 | 依 `HBin` 分選至對應料盤 |
| 2D 讀取成功但不在名單內 | 判定拒收 → Error 區，報表記為 NotWhitelisted；不彈窗、不中斷 |
| 2D 讀取失敗（CCD 誤讀） | 走機台既有重讀／人工流程，與白名單無關 |

---

## 5. 客戶需提供之資料

- 每顆 IC 的 `QRCodeID`，須與 CCD 讀值完全一致。
- 每顆 IC 的 `HBin`，其編號須對應機台已設定之 Bin。
- Lot 層之 `LOTID`、`KYECLotID`、`Substage`、`ProductCode`（四項皆為必填）。

---

## 6. 使用流程

1. 依 §3.2 格式製作 `WhiteList.json`（每個 Lot 都要有 `KYECLotID`）。
2. 將檔案放入機台 `D:\HT160S_BCB\HT160S_WhiteList\` 資料夾（完整路徑 `D:\HT160S_BCB\HT160S_WhiteList\WhiteList.json`）。
3. 主機以 `LOTSTART` + `SORTMODE = WHITELIST` 開 Lot。
4. （選用）以 `S1F3` 查詢 SVID 66032，確認回報 `3`。
5. Lot End 後，機台自動還原為基礎分選模式（回讀值變回 `0`／`1`／`2`，視現場 Sort Mode 設定）。

> **66032 = 3 只證明「白名單模式已切換成功」，不代表白名單檔案已被接受。** 兩者是分開的：
> 模式由 `LOTSTART` 的 `SORTMODE` 決定（HCACK=0 即成功），檔案則在 Lot Start 當下才讀取與檢查。
> 檔案若不合格，66032 仍回 `3`，但機台名單為空、生產 Start 會被擋下。檔案是否被接受，
> 請看機台端的提示訊息或 Process 記錄中的 `WhiteList: ...` 行。

操作員亦可於機台維護畫面的白名單面板勾選啟用，效果與 SECS 相同：於下一次 Lot Start 生效、Lot End 自動還原；機台內有在製 IC 時不可切換。
