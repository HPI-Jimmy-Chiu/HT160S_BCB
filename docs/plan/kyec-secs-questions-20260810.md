# 京元 EAP 2026-08-10 SECS 四問 — 分析與待確認事項

> 來源：KYEC `.EAP_Group` 溫浚諺(CYWen1) 來信（四點）。
> 本文只做「分析 + 列出需要貴端提供的資料」，**尚未改任何 code**。
> 所有結論都附 code 位置或現場 log 佐證；查不到的地方一律列成問題，不猜。
> 參照樹（唯讀）：`D:\HT9045\` 下 11 個 HT-90XX 版本，本文主要引用
> `HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP`（下稱 **810_B01**）、
> `HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422`（**899**）、
> `HT9011UC_Code_V3.33.906.0_20260618`（**906**）。

---

## 摘要（一句話結論）

| # | 客戶提問 | 結論 |
|---|---|---|
| 1 | S2F33/S2F35 不應有容忍行為 | **要求合理**，HT-90XX 確實是嚴格拒絕。但直接切嚴格會讓貴端現有 33 張報表**當場死掉 29 張**、31 條連結死掉 25 條。需先決定補字典還是縮報表。 |
| 2 | 請調整 SVID 38237~38239 | **直接撞號**。這三個號在 HT-160S 已是 AMR Auto4/5/6 Tray Count。且我方手上 **11 個 HT-90XX 版本都沒有定義 38237~38239**；家族既有的同義號是 **37003/37004/37005**。需貴端拍板。 |
| 3 | 66030 與 66031 都對應到 1006？ | **是**，兩者都由 1006 推導但取法不同（一個取逗號數、一個取第一段）。範例見 §3。 |
| 4 | 每項 SVID 語意都與 HT-90XX 相同？ | **67 個號碼中 48 個完全同名同義**；其餘分三類差異，逐項列於 §4。另外貴端 1517 表格的 `Max=8` 與 `Description 0..11` 互相矛盾——**矛盾源頭在 HT-90XX 韌體本身**，說明見 §4.3。 |

---

## 1. S2F33 / S2F35 的「容忍行為」

### 1.1 事實：兩邊現在的行為

**HT-160S（現況，2026-07-27 起）** — 刻意容忍：

| 情境 | 現在的回覆 | 位置 |
|---|---|---|
| S2F33 內含本機未定義的 SVID | **DRACK=0x00 接受**，只寫一行 log `[SECS][S2F33] accept unknown SVID=...` | `HT160S_Program_BCB_V1.0.0.0/SecsGem/uHGemEquipment.cpp:1247-1252` |
| S2F35 連到本機沒有的 CEID | **LRACK=0x00**，並幫 host 建一個永遠不會發射的 CEID | 同檔 `:1303-1308` |
| S2F35 連到不存在的 RPTID | **LRACK=0x00**，該 RPTID 被略過 | 同檔 `:1322-1328` |
| 報表 SVID 數 > 192 | DRACK=0x01（這一項本來就是硬拒） | 同檔 `:1235` |

當初導入容忍的原因記錄在 code 註解：貴端 CJ_EAP host 是照 HT9045 的號碼表出廠的，硬拒會讓它整個 S2F33/S2F35 佈建階段停住（例如 `RPTID 504={20001,20002,20003}`）。

**HT-90XX（810_B01 / 899 / 906 三版一致）** — 嚴格，且**整封拒絕**：

| 情境 | 回覆 | 位置 |
|---|---|---|
| S2F33 任一 SVID 不在字典 | `DefineReportAcknowledgeInvalidSVID()` → **DRACK=0x04**，整封不生效 | `SECSGEM/uHGemClass.cpp:1323-1335` |
| S2F33 RPTID 已定義 | DRACK=0x03 | 同檔 `:1312-1321` |
| S2F35 CEID 不存在 | **LRACK=0x04** | `SECSGEM/uHGemEquipment.cpp:8004-8008` |
| S2F35 RPTID 不存在 | **LRACK=0x05** | 同檔 `:8017-8025` |

所以**貴端要求的行為就是 HT-90XX 的行為，這點沒有爭議**。

### 1.2 為什麼 9045 上不會被拒、160S 上會

因為字典大小差兩個數量級：

| 機種 | SV 註冊數 | EC 註冊數 | **有效 SV 字典** |
|---|---|---|---|
| 9046LS 810_B01 | 870 | 1689 | **2558** |
| 9011UC 899 | 785 | 1672 | 2456 |
| 9011UC 906 | 890 | 1741 | 2629 |
| **HT-160S** | 67 | 8 | **67** |

關鍵機制：HT-90XX 的 `SetECDataPointer` **四個多載最後都會再呼叫一次 `SetSVDataPointer` 登記同一個號碼**
（`810_B01/SECSGEM/uHGemEquipment.cpp:5951 / 5990 / 6165 / 6207`），所以 9045 上「每個 EC 同時也是 SV」。
HT-160S 的 `SetECDataPointer` **沒有**這個鏡射（`uHGemEquipment.cpp:1035-1055`），SV / EC 各自登記。

### 1.3 影響量化（用貴端 2026-07-31 現場 log 實算）

資料來源：`docs/plan/onsite-0731-kyec-secs/host_reports_S2F33.csv`（33 張報表）、
`host_links_S2F35.csv`（31 條連結）、`host_svids.txt`（389 個相異 SVID）。

- 貴端引用 **389** 個相異 SVID，HT-160S 目前定義其中 **47** 個 → **342 個未定義**。
- 若改成 HT-90XX 的嚴格模式（DRACK=0x04）：

| 項目 | 結果 |
|---|---|
| 含至少 1 個未定義 SVID 的報表 | **29 / 33 張** → 全部被拒 |
| 能存活的報表 | 只有 **RPTID 517、524、2000、2001** 四張 |
| 接著 S2F35：所有 RPTID 都不存在的連結 | **25 / 31 條** → LRACK=0x05 |

被拒最嚴重的幾張：`505`（179 個 SVID，本機只認 6 個）、`509`（38 個全不認）、
`900`（24 個全不認）、`700`（21 個全不認）、`510`（20 個全不認）。

**也就是說：只要一切成嚴格模式，貴端的報表佈建等於全滅。**

### 1.4 需要貴端裁決（§5 Q1）

我方可以做，但要先確定貴端要的是哪一種「不容忍」：

- **A. 完全照 9045 嚴格** — 需要 HT-160S 先把貴端引用的 389 個號碼補到字典。
  但 HT-160S 是 sorter，其中大量是測試機專屬量（site / socket / 溫控 / 針測），
  補進來也只能回固定值或空值，等於製造 342 個假資料，我方**不建議**。
- **B. 嚴格，但貴端把報表縮到 HT-160S 實際支援的號碼** — 最乾淨。
  我方可提供「HT-160S 支援清單」（本文 §4 那 67 個），貴端據此重下 S2F33。
- **C. 維持現行容忍，但改成「可稽核」** — 現在已有 log，可再加上
  S1F3 讀未註冊號碼時回**空 list `<L[0]>`**（與「已註冊但值為空」的 `<A[0]>` 在線上可區分，
  2026-08-04 已實測），讓貴端 EAP 自己判斷哪些號沒生效。

---

## 2. SVID 38237~38239（Record Auto 1/2/3 Tray Count）

### 2.1 直接撞號

`38237/38238/38239` 在 HT-160S 目前是 **AMR Auto4 / Auto5 / Auto6 Tray Count**：

```
HT160S_Program_BCB_V1.0.0.0/SecsGem/uAgvStation.cpp:125-127
    /*P7*/ { 7, ASK_AUTO, 3, 38199, 38237, 38240, 38243, "AUTO4" },
    /*P8*/ { 8, ASK_AUTO, 4, 38200, 38238, 38241, 38244, "AUTO5" },
    /*P9*/ { 9, ASK_AUTO, 5, 38201, 38239, 38242, 38245, "AUTO6" },
                                  ↑TrayCount
```

而**家族已經有 Auto1-3 的 AMR Tray Count = 38225 / 38226 / 38227**
（810_B01 與 906 同名 `AMR Auto1/2/3 Tray Count`），HT-160S 也是照抄的。
所以貴端要的「Auto1/2/3 盤數」在 38225-38227 上已經存在一份。

### 2.2 我方查不到 38237~38239 的家族出處

我方把手上 **11 個 HT-90XX 原始碼版本**（9011UC .874 / .896×2 / .899 / .903 / .905 / .906；
9046LS .715×2 / .745 / .810 / .810_B01 / .812×3）全文搜過：

> **沒有任何一版出現過 `38237`、`38238`、`38239` 這三個字串**（不只是 SVID 註冊，是全樹全文）。

另外，`BU5` 在這些原始碼裡是 **`USE_BU5_Function` 這個 KYEC 專用功能開關**
（810_B01 有 80 處、906 有 55 處），**不是一個版本號**。所以「BU5 版本」我方無法對應到具體韌體版本。

→ **這是第一個必須請貴端提供資料的地方（§5 Q2-a）。**

### 2.3 家族其實已經有貴端要的那個量：37003-37005 / 37013-37015

貴端描述的語意是「**該 Auto 軌道本批已產出的盤數，Lot End 清除**」。
HT-90XX 家族已經有一模一樣的量：

| SVID | 名稱 | 版本 | 位置 |
|---|---|---|---|
| 37003 / 37004 / 37005 | `UnloaderTrayCount_ART_Auto1/2/3` | 810_B01、899、906 | `SECSGEM/uHGemHT9045_SV.cpp:691-693` |
| 37013 / 37014 / 37015 | `UnloaderTrayCount_ART_Auto4/5/6` | 899、906 | 同檔 `:701-703` |

- 遞增點：出盤時 `LastSet.iUnloaderTrayCount_ART[Pos]++`（`asendic_Auto.cpp:534`）
- 清零點：Initial Start / ReTest Start（`csystem.cpp:12192-12206`），也就是**每批開始時歸零**
- 主畫面同一個值顯示在 `lblAuto1TrayCnt` / LotInfo `edAuto1Cnt`

**而貴端 host 目前完全沒有引用 37003-37005**（`host_svids.txt` 沒有這三個號）。

### 2.4 語意差異（就算改了號，值也不一樣）

| | 貴端要的 | HT-160S 38237-38239 現值 |
|---|---|---|
| 對象 | Auto**1-3** 軌道 | Auto**4-6** 站 |
| 量 | 本批**累計已產出盤數** | AMR **車上目前**盤數快照 |
| 歸零 | Lot End | **AMR 車一離站就歸 0**（`uAgvStation.cpp:435`） |
| 來源 | 出盤事件累加 | `Car->iTrayCount`（`uAgvStation.cpp:321`） |

**HT-160S 目前沒有任何一個「每軌道本批已產出盤數」的計數器**，
`Car[].iTrayCount` 只是車上現有盤數（`aAuto1To6.cpp:845-854`、`MyMotor.h:119`）。
所以這是**新功能**，不是改個號碼就好。

### 2.5 現場已經在錯位（且這正是我方 8/2 問過的那題）

貴端 host 已經把 **RPTID 524 = {38237, 38238, 38239} 綁在 CEID 67 上**
（`host_links_S2F35.csv:19`、`host_reports_S2F33.csv:23`）。

- **CEID 67 = `Tray Test Finish`**，是測試機專屬事件；
  HT-160S 是 sorter，沒有測試步驟，**永遠不會發射這個事件**
  （`docs/plan/secs-9045-porting-20260729/ceid-matrix.md:186`）。
- 所以這張報表**目前一次也沒送出過**，且就算送出，內容會是 Auto4-6 的車上盤數快照，不是貴端要的值。

我方已於 **2026-08-02** 在《SECS_GEM功能_Handler_20260802_修訂說明》第 19 列問過貴端：
「請貴端確認 9045 對這三個號碼的定義是否與本機一致，若不一致則需另行編號。此為本項唯一尚未關閉的部分。」
**本次來信即是該題的回覆**，方向確定了，但還缺 §5 的幾個細節才能動工。

---

## 3. 66030 / 66031 是否都對應到 1006

**是，兩個都對應到 1006，但取法不同。**

`66000` 整段（10 個號）已於 2026-08-04 依貴端裁示全數下架，號碼**永久保留不再挪用**
（tombstone：`uHGemHT160.cpp:650-691`）。新的 `SVID 1006 Lot ID` 是**把機上所有批號用逗號串起來**
（實作：`uHGemHT160.cpp:790-806`，分隔符為**無空格的逗號**，2026-08-04 貴端指定）。

### 3.1 換算規則

| 舊號 | 舊語意 | 由 1006 換算 |
|---|---|---|
| `66030` | Active Lot Count | **先判空**：1006 為空字串 → **0 批**；否則 = 逗號數 + 1 |
| `66031` | Current Lot ID | 第一個逗號**之前**的字串（只有一批時就是整串） |

> ⚠ 「先判空」不能省：未開批時 1006 會回主畫面 lot 欄位的文字（通常是空的），
> 直接算「逗號數 + 1」會把 0 批誤報成 1 批（已退役的 66030 在該狀態是回 0）。

### 3.2 範例

| 機上狀態 | S1F3 讀 1006 回覆 | 換算 66030 | 換算 66031 |
|---|---|---|---|
| 未開批 | `<A [0] "">` | `0` | `""` |
| 單批 `TW2601A` | `<A [7] "TW2601A">` | `1`（0 個逗號 + 1） | `TW2601A` |
| 三批同時在機上 | `<A [23] "TW2601A,TW2601B,TW2601C">` | `3`（2 個逗號 + 1） | `TW2601A` |

### 3.3 附帶：已下架號碼在線上是可偵測的

- 讀**未註冊**的號碼（例如 66031）→ 回**空 list** `<L [0]>`（位元組 `01 00`）
- 讀**已註冊但值為空**的號碼（例如未開批的 1006）→ 回 `<A [0]>`（位元組 `41 00`）

兩者在線上可區分，2026-08-04 已並排實測：`66031 → L,0`、`1006 → A,0`。
建議貴端 EAP 用這個特徵做「號碼是否還有效」的自我檢查。

（同樣寫在 `docs/SECS/HT160S_SECS_Comm_Examples.md:433-437`、
`docs/SECS/HT160S_SECS_Interface_Spec_20260727.md:330-331`。）

---

## 4. 每項 SVID 的資訊是否與 HT-90XX 相同

### 4.1 完整對照表（HT-160S 全部 67 個 SVID）

`SV:` / `EC:` 表示該號在該版 HT-90XX 是以哪個名稱空間登記；
HT-90XX 的 EC 會自動鏡射成同號 SV，所以 `EC:` 的號碼在 9045 上 **S1F3 也讀得到**。

| SVID | HT-160S 名稱 | 9046LS 810_B01 | 9011UC 899 | 9011UC 906 |
|---|---|---|---|---|
| 3 | GemClock | SV:GemClock | SV:GemClock | SV:GemClock |
| 4 | GemControlState | SV:GemControlState | SV:GemControlState | SV:GemControlState |
| 9 | PreviousGemControlState | SV:PreviousGemControlState | SV:PreviousGemControlState | SV:PreviousGemControlState |
| 1001 | Machine Model | SV:Machine Model | SV:Machine Model | SV:Machine Model |
| 1002 | Machine ID | SV:Machine ID | SV:Machine ID | SV:Machine ID |
| 1003 | Software Version | SV:Software Version | SV:Software Version | SV:Software Version |
| 1006 | Lot ID | **EC**:Lot ID | **EC**:Lot ID | **EC**:Lot ID |
| 1007 | Operator ID | **EC**:Operator ID | **EC**:Operator ID | **EC**:Operator ID |
| 1008 | Run Mode | — | — | — |
| 1009 | Lot Start Time | SV:Lot Start Time | SV:Lot Start Time | SV:Lot Start Time |
| 1011 | Machine State | SV:Machine State | SV:Machine State | SV:Machine State |
| 1021 | UPH | SV:UPH | SV:UPH | SV:UPH |
| 1027 | System Time | SV:System Time | SV:System Time | SV:System Time |
| 1101 | Loader Count | SV:Loader Count | SV:Loader Count | SV:Loader Count |
| 1102 | Output Total Count | SV:Output Total Count | SV:Output Total Count | SV:Output Total Count |
| 1103 | Auto1 Count | SV:Auto1 Count | SV:Auto1 Count | SV:Auto1 Count |
| 1104 | Auto2 Count | SV:Auto2 Count | SV:Auto2 Count | SV:Auto2 Count |
| 1105 | Auto3 Count | SV:Auto3 Count | SV:Auto3 Count | SV:Auto3 Count |
| 1259 | Auto4 Count | — | SV:Auto 4 Count | SV:Auto 4 Count |
| 1260 | Auto5 Count | — | SV:Auto 5 Count | SV:Auto 5 Count |
| 1261 | Auto6 Count | — | SV:Auto 6 Count | SV:Auto 6 Count |
| 1501 | Setup File | **EC**:Setup File | **EC**:Setup File | **EC**:Setup File |
| 1517 | Start Mode | **EC**:Start Mode For HT9045 | **EC**:同左 | **EC**:同左 |
| 1518 | Real/Dummy | **EC**:Real/Dummy For HT9045 | **EC**:同左 | **EC**:同左 |
| 2758 | Type 1 Tray Pitch X | **EC**:Type 1 Tray Pitch X | **EC**:同左 | **EC**:同左 |
| 2759 | Type 1 Tray Pitch Y | **EC**:Type 1 Tray Pitch Y | **EC**:同左 | **EC**:同左 |
| 2760 | Type 1 Tray Start Position X | **EC**:同名 | **EC**:同名 | **EC**:同名 |
| 2761 | Type 1 Tray Start Position Y | **EC**:同名 | **EC**:同名 | **EC**:同名 |
| 2762 | Type 1 Tray Division X | **EC**:同名 | **EC**:同名 | **EC**:同名 |
| 2763 | Type 1 Tray Division Y | **EC**:同名 | **EC**:同名 | **EC**:同名 |
| 37010 | Enter Skip IC Count | SV:Enter Skip IC Count | SV:同名 | SV:同名 |
| 38199 | Auto4 Carrier ID | — | **EC**:Output 4 Tray ID | **EC**:Output 4 Tray ID |
| 38200 | Auto5 Carrier ID | — | **EC**:Output 5 Tray ID | **EC**:Output 5 Tray ID |
| 38201 | Auto6 Carrier ID | — | **EC**:Output 6 Tray ID | **EC**:Output 6 Tray ID |
| 38202 | Loader Carrier ID | SV:Load Port Carrier ID | — | — |
| 38203 | Empty Carrier ID | — | — | — |
| 38204 | Color Carrier ID | — | — | — |
| 38205 | Auto1 Carrier ID | SV:Auto1 carrier ID | **EC**:Output 1 Tray ID | **EC**:Output 1 Tray ID |
| 38206 | Auto2 Carrier ID | SV:Auto2 carrier ID | **EC**:Output 2 Tray ID | **EC**:Output 2 Tray ID |
| 38207 | Auto3 Carrier ID | SV:Auto3 carrier ID | **EC**:Output 3 Tray ID | **EC**:Output 3 Tray ID |
| 38219 | Supplement Bin | SV:Supplement Bin | — | — |
| 38220 | LD UnLD Check AGV | SV:LD UnLD Check AGV | — | — |
| 38221 | LD UnLD Finish AGV | SV:LD UnLD Finish AGV | — | — |
| 38222 | AMR Loader Tray Count | SV:同名 | — | SV:同名 |
| 38223 | AMR Empty Tray Count | SV:同名 | — | SV:同名 |
| 38224 | AMR Color Tray Count | SV:同名 | — | SV:同名 |
| 38225 | AMR Auto1 Tray Count | SV:同名 | — | SV:同名 |
| 38226 | AMR Auto2 Tray Count | SV:同名 | — | SV:同名 |
| 38227 | AMR Auto3 Tray Count | SV:同名 | — | SV:同名 |
| 38228 | AMR Loader Device Count | SV:同名 | — | SV:同名 |
| 38229 | AMR Empty Device Count | SV:同名 | — | SV:同名 |
| 38230 | AMR Color Device Count | SV:同名 | — | SV:同名 |
| 38231 | AMR Auto1 Device Count | SV:同名 | — | SV:同名 |
| 38232 | AMR Auto2 Device Count | SV:同名 | — | SV:同名 |
| 38233 | AMR Auto3 Device Count | SV:同名 | — | SV:同名 |
| 38234 | AMR Auto1 Bin Setting | SV:同名 | — | — |
| 38235 | AMR Auto2 Bin Setting | SV:同名 | — | — |
| 38236 | AMR Auto3 Bin Setting | SV:同名 | — | — |
| **38237** | **AMR Auto4 Tray Count** | — | — | — |
| **38238** | **AMR Auto5 Tray Count** | — | — | — |
| **38239** | **AMR Auto6 Tray Count** | — | — | — |
| 38240 | AMR Auto4 Device Count | — | — | — |
| 38241 | AMR Auto5 Device Count | — | — | — |
| 38242 | AMR Auto6 Device Count | — | — | — |
| 38243 | AMR Auto4 Bin Setting | — | — | — |
| 38244 | AMR Auto5 Bin Setting | — | — | — |
| 38245 | AMR Auto6 Bin Setting | — | — | — |

### 4.2 差異歸類

**(a) 名稱寫法不同、語意相同（可直接視為相同）**

| SVID | HT-160S | HT-90XX |
|---|---|---|
| 1259-1261 | `Auto4 Count` | `Auto 4 Count`（多一個空格） |
| 38202 | `Loader Carrier ID` | `Load Port Carrier ID` |
| 38199-38201 | `Auto4-6 Carrier ID` | `Output 4/5/6 Tray ID`（899/906） |
| 38205-38207 | `Auto1-3 Carrier ID` | 810 = `Auto1 carrier ID`(SV)；899/906 = `Output 1 Tray ID`(EC) |

**(b) HT-160S 專屬（三個家族版本都沒有）**

| SVID | 說明 |
|---|---|
| 38203 / 38204 | Empty / Color 站的 Carrier ID。HT-160S 是 sorter，多出這兩個進料站，家族沒有對應機構。 |
| 38237-38245 | Auto4-6 的 tray / device / bin-setting（本次爭議點）。810 的 AMR 帶止於 38236，899 沒有 AMR SVID 家族，故當初沒有家族號可抄。 |
| **1008 Run Mode** | 2026-08-04 下架 66000 時，我方選了家族號段裡的 1008。**但家族三版都沒有定義 1008**，貴端 host 也沒有引用。→ 見 §5 Q4-b。 |

**(c) 名稱空間差異（最容易咬人的一類）**

HT-90XX 的 EC 會自動鏡射成同號 SV；HT-160S **不會**。HT-160S 目前只有 **8 個 ECID**：
`1007、1501、2758、2759、2760、2761、2762、2763`（`uHGemHT160.cpp:1029-1050`）。

| SVID | HT-90XX | HT-160S | 後果 |
|---|---|---|---|
| 1006 Lot ID | EC + SV | **只有 SV** | 貴端無法用 S2F15 寫批號；HT-160S 開批走 S2F41 `LOTSTART` / `SET_LOT_INFO` |
| **1517 Start Mode** | EC + SV | **只有 SV** | **貴端若對 1517 下 S2F15 會失敗（EAC 拒絕）** |
| **1518 Real/Dummy** | EC + SV | **只有 SV** | 同上 |
| 1007 / 1501 / 2758-2763 | EC + SV | SV + EC 都有 | 相同，可讀可寫 |

### 4.3 1517 的 `Max=8` vs `Description 0..11` 矛盾

貴端表格內部矛盾，**但源頭在 HT-90XX 韌體本身，不是貴端筆誤**：

1. HT-90XX 的 `eRunStartMode` enum 實際定義到 13
   （`MachineType.h`，810_B01 / 906 相同）：
   ```
   0 rsmContinuStart      4 rsmAutoSiteMap    8  rsmInitial_ART        12 rsmFIFOMode
   1 rsmInitialStart      5 rsmQAMode         9  rsmContinuStart_ART   13 rsmInitial_MRT...
   2 rsmContinuRetest     6 rsmContinuEQC     10 rsmContinuRetest_ART
   3 rsmCInitialRetest    7 rsmInitialEQC     11 rsmAutoRetest
   ```
   → **貴端表格 Description 的 0..11 是正確的**（與 enum 一致）。

2. 但 HT-90XX 的 EC 宣告本身寫的是 `Max="8"`，而且 legend 停在 `8:Auto Retest`
   （`uHGemHT9045_EC.cpp` 的 1517 那一行，**810_B01 / 899 / 906 三版完全相同**）。
   照 enum，8 是 `rsmInitial_ART` 不是 `Auto Retest`（Auto Retest 是 11）。
   → **HT-90XX 韌體的 EC 宣告 Max 與 legend 都是舊的、沒跟上 enum。**

3. HT-160S 的 1517 **只會回 0 或 1**（sorter 沒有 retest / site-map / QA / EQC / ART），
   而且 HT-160S 內部旗標與 9045 **完全相反**（HT-160S 0=Initial、1=Continue），
   已在 `uHGemHT160.cpp:767-770` 做反轉後才上報，不會回錯值。

→ 請貴端裁決 Max 應該是 8 / 11 / 13（§5 Q4-a）。我方會照裁決同步規格書。

---

## 5. 需要貴端提供 / 確認的事項（不猜）

| # | 題目 | 為什麼卡住 |
|---|---|---|
| **Q1** | S2F33/S2F35 要的是哪一種？<br>(A) 完全嚴格 (B) 嚴格＋貴端縮報表 (C) 容忍但可稽核 | 直接切嚴格會讓貴端 29/33 張報表、25/31 條連結當場失效（§1.3）。這是營運層決定，不是技術層。 |
| **Q2-a** | 38237~38239 是**哪一版** HT-90XX 韌體開始定義的？能否提供該版的 SVID 目錄傾印？ | 我方 11 個版本全文搜尋都沒有這三個號；`BU5` 在原始碼裡是功能開關 `USE_BU5_Function`，不是版本號。無法比對。 |
| **Q2-b** | 家族既有的 **37003/37004/37005**（`UnloaderTrayCount_ART_Auto1/2/3`，Initial Start 歸零）是不是就是貴端要的量？若是，可否直接改用這三個號？ | 這三個號語意完全吻合貴端描述，而且貴端 host 目前沒引用。改用它可**完全避開撞號**。 |
| **Q2-c** | 若堅持用 38237~38239：<br>① HT-160S 現有的 Auto4-6 AMR Tray Count 要改到哪三個號？<br>② Auto4-6 的 Record Tray Count 用哪三個號？ | 號碼必須由貴端指定。上次 38208-38210 → 38199-38201 的改號就是這樣定的，舊號永久作廢不再挪用。 |
| **Q2-d** | RPTID 524 目前綁在 **CEID 67 Tray Test Finish**。HT-160S 是 sorter，此事件永不發射。請指定要綁到哪個 HT-160S 事件。 | 不指定的話，就算號碼改對了，這張報表仍然一次都不會送出。 |
| **Q2-e** | 「Lot End 則會清除」——HT-160S **可同時掛多批**（最多 64 批）。是「該批 Lot End 清該批」還是「全部批次結束才清」？ | 9045 是單批機，這個歧義在 9045 上不存在，在 HT-160S 上必須講清楚。 |
| **Q4-a** | 1517 的 `Max` 應為 **8 / 11 / 13**？ | 貴端表格 Max=8 與 Description 0..11 矛盾；矛盾源頭是 HT-90XX 韌體 EC 宣告本身（§4.3）。 |
| **Q4-b** | 貴端（或 HT-90XX 家族）對 **SVID 1008** 是否已有定義？ | HT-160S 用 1008 當 `Run Mode`（取代下架的 66000），但家族三版都沒定義 1008，怕日後撞號。 |
| **Q4-c** | 貴端是否需要 HT-160S 把 **1517 / 1518 也開成 EC**（可用 S2F15 寫入）？ | 目前 HT-160S 這兩個號只有 SV，貴端若對它們下 S2F15 會失敗。 |

---

## 附：本文事實的驗證方式

| 主張 | 驗證 |
|---|---|
| HT-160S 容忍 / HT-90XX 嚴格 | 逐行讀 `ProcessDefineReport_S2F33` / `ProcessLinkEventReport_S2F35`（160S）與 `S2F34_ProcessHostSendReportID` / `ProcessHostSendReportLinkID`（90XX） |
| 29/33、25/31 的數字 | 以 `host_reports_S2F33.csv` + `host_links_S2F35.csv` 對 HT-160S 的 67 個號碼實算 |
| 90XX 有效 SV 字典 2558 | 解析 `uHGemHT9045_SV.cpp` + `uHGemHT9045_EC.cpp`（legacy `SECSGEM.cpp` 已確認**不在** `HT9045.bpr` 專案內，不計入） |
| 11 版都沒有 38237-38239 | 對 `D:\HT9045\` 下 11 個 `*_Code_*` 目錄全樹全文字串搜尋 |
| 1517 enum 0..13 | `MachineType.h` 的 `eRunStartMode`（810_B01 / 906 一致） |
