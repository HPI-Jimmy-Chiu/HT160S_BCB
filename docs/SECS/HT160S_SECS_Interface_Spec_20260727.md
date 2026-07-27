# HT-160S SECS/GEM 介面規格書 / SECS/GEM Interface Specification

> **機型 Model:** HT-160S (Tray Sorter) &nbsp;|&nbsp; **MDLN:** `HT-160S` &nbsp;|&nbsp; **SOFTREV:** `1.0.0.0`
> **文件版本 Doc rev:** 2026-07-27
> **依據 Based on:** current firmware build (branch `feat/iosetview-172-refactor`)
>
> 本規格反映 HT-160S **目前實作**的 SECS-II / GEM 介面。命令面向 HT-90XX (HT9045) 對齊;
> 但 **SVID / CEID 字典為 HT-160S 自有**(sorter 語意,見 §3),與 HT-90XX (tester) 不完全相同。
> This document describes the SECS-II / GEM interface **as currently implemented** on HT-160S.
> The command surface is aligned to HT-90XX; the SVID/CEID dictionary is HT-160S-specific (§3).

---

## 0. 閱讀慣例 / Conventions

| 記號 | 意義 |
|---|---|
| 方向 Direction | `H→E` = host 送設備 (host→equipment);&nbsp; `E→H` = 設備送 host |
| `L,n{...}` | List of n items |
| `A` / `B` / `Bool` | ASCII / Binary(1 byte) / Boolean |
| `U1 U2 U4` / `I1 I2 I4` | unsigned / signed integer (1/2/4 byte) |
| `FT8` | 8-byte float (F8) |
| W-bit | primary message expects a reply |

---

## 1. 連線與傳輸 / Connection & Transport

- **HSMS-SS**,設備端為 **passive**(host 主動撥入 / equipment is passive, host connects in)。
- Port / Device(Session) ID:見 `system\ComPort.ini`(現場觀察 on-site observed: **port 6000, Device ID = 1**)。
- 型號 / 版本 由 S1F2 與 S1F14 回報:`MDLN = HT-160S`,`SOFTREV = 1.0.0.0`。
- **GEM 控制狀態 Control State**(鏡像於 SVID 66002 / mirrored in SVID 66002):
  | 值 | 狀態 | 進入方式 |
  |---|---|---|
  | 1 | Off-Line | S1F15/F16,或 RCMD `ONLINE_LOCAL` 前 |
  | 4 | On-Line Local | RCMD `ONLINE_LOCAL` |
  | 5 | On-Line Remote | S1F17/F18,或 RCMD `ONLINE_REMOTE`/`ONLINE` |
- 事件 / 警報推播(S6F11 / S5F1)僅在 **HSMS SELECTED** 時送出。
- 標準上線序列(host):`S1F13 → S1F17 → S2F37(disable all) → S5F3 → S2F33(define) → S2F35(link) → S2F37(enable)`,全數支援(見 §2、§4)。

---

## 2. 支援訊息總表 / Supported Message List

> 以下為 HT-160S **目前已實作**的 SECS-II 訊息。未列於此、或列於 §5 者為尚未支援。

### Stream 1 — 設備狀態 / Equipment Status

| S/F | 名稱 Name | 方向 | 回覆 | 本體 Body (SECS-II) | 說明 Notes |
|---|---|---|---|---|---|
| S1F1 | Are You There Request | H→E | S1F2 | (header only) | 存活探測 |
| S1F2 | On-Line Data | E→H | — | `L,2{ A MDLN, A SOFTREV }` | = `{"HT-160S","1.0.0.0"}` |
| S1F3 | Selected Equipment Status Request | H→E | S1F4 | `L,n{ SVID }`(n=0=全部) | 讀 SVID |
| S1F4 | Selected Equipment Status Data | E→H | — | `L,n{ <SV value> }` | 未知 SVID 回空 item |
| S1F11 | Status Variable Namelist Request | H→E | S1F12 | `L,n{ SVID }` | |
| S1F12 | Status Variable Namelist Reply | E→H | — | `L,n{ L,3{ U4 SVID, A NAME, A UNITS } }` | |
| S1F13 | Establish Communications Request | H→E | S1F14 | `L,2{ A MDLN, A SOFTREV }` | body 不解析 |
| S1F14 | Establish Comm. Acknowledge | E→H | — | `L,2{ B COMMACK, L,2{ A MDLN, A SOFTREV } }` | COMMACK=0 恆接受 |
| S1F15 | Request OFF-LINE | H→E | S1F16 | (header only) | **本次新增 (a146d14 前)** |
| S1F16 | OFF-LINE Acknowledge | E→H | — | `B OFLACK` | OFLACK=0,控制狀態→1 |
| S1F17 | Request ON-LINE | H→E | S1F18 | (header only) | |
| S1F18 | ON-LINE Acknowledge | E→H | — | `B ONLACK` | ONLACK=0,控制狀態→5 |

### Stream 2 — 設備控制與診斷 / Equipment Control & Diagnostics

| S/F | 名稱 Name | 方向 | 回覆 | 本體 Body (SECS-II) | 說明 Notes |
|---|---|---|---|---|---|
| S2F13 | Equipment Constant Request | H→E | S2F14 | `L,n{ ECID }`(n=0=全部) | 讀 EC |
| S2F14 | Equipment Constant Data | E→H | — | `L,n{ <EC value> }` | |
| S2F15 | New Equipment Constant Send | H→E | S2F16 | `L,n{ L,2{ ECID, ECV } }` | 寫 EC(限 idle,見 §3.2) |
| S2F16 | New EC Acknowledge | E→H | — | `B EAC` | 0=ok 1=不可寫 2=busy 3=範圍 |
| S2F17 | Date and Time Request | H→E | S2F18 | (header only) | **本次新增** |
| S2F18 | Date and Time Data | E→H | — | `A[16] TIME` | `YYYYMMDDhhmmsscc`,唯讀 |
| S2F25 | Loopback Diagnostic Request | H→E | S2F26 | `B ABS` | **本次新增** |
| S2F26 | Loopback Diagnostic Data | E→H | — | `B ABS`(echo) | 原封回送 |
| S2F31 | Date and Time Send | H→E | S2F32 | `A[..] TIME` | |
| S2F32 | Date and Time Acknowledge | E→H | — | `B TIACK` | TIACK=0;**刻意不寫設備時鐘** |
| S2F33 | Define Report | H→E | S2F34 | `L,2{ U4 DATAID, L,a{ L,2{ U4 RPTID, L,b{ U4 SVID } } } }` | 見 §4;**容忍未知 SVID** |
| S2F34 | Define Report Acknowledge | E→H | — | `B DRACK` | 0=ok 1=space 2=fmt 3=firmware-dup |
| S2F35 | Link Event Report | H→E | S2F36 | `L,2{ U4 DATAID, L,a{ L,2{ U4 CEID, L,b{ U4 RPTID } } } }` | 見 §4;**容忍未知 CEID/RPTID** |
| S2F36 | Link Event Report Acknowledge | E→H | — | `B LRACK` | 0=ok 2=fmt |
| S2F37 | Enable/Disable Event Report | H→E | S2F38 | `L,2{ Bool CEED, L,n{ U4 CEID } }` | n=0=全部 |
| S2F38 | Enable/Disable Event Ack | E→H | — | `B ERACK` | 0=ok 1=CEID不存在 |
| S2F41 | Host Command Send | H→E | S2F42 | `L,2{ A RCMD, L,n{ L,2{ A CPNAME, A CPVAL } } }` | 見 §3.4 |
| S2F42 | Host Command Acknowledge | E→H | — | `L,2{ B HCACK, L,0 }` | 0=ok 1=無效 2=不可執行 4=busy |

### Stream 5 — 警報 / Alarm

| S/F | 名稱 Name | 方向 | 回覆 | 本體 Body (SECS-II) | 說明 Notes |
|---|---|---|---|---|---|
| S5F1 | Alarm Report Send | E→H | (S5F2) | `L,3{ B ALCD, U4 ALID, A ALTX }` | ALCD bit7:0x80=set/0x00=clear |
| S5F3 | Enable/Disable Alarm Send | H→E | S5F4 | `L,2{ ALED, ALID }` | |
| S5F4 | Enable/Disable Alarm Ack | E→H | — | `B ACKC5` | ACKC5=0 |
| S5F5 | List Alarm Request | H→E | S5F6 | `L,n{ ALID }`(n=0=全部) | |
| S5F6 | List Alarm Data | E→H | — | `L,n{ L,3{ B ALCD, U4 ALID, A ALTX } }` | 目錄由 AlarmList.csv/SSOT 即時產生 |
| S5F7 | List Enabled Alarm Request | H→E | S5F8 | `L,0` | |
| S5F8 | List Enabled Alarm Data | E→H | — | `L,n{ L,3{ B ALCD, U4 ALID, A ALTX } }` | |

### Stream 6 / 9 — 事件與錯誤 / Event & Error

| S/F | 名稱 Name | 方向 | 回覆 | 本體 Body (SECS-II) | 說明 Notes |
|---|---|---|---|---|---|
| S6F11 | Event Report Send | E→H | (S6F12) | `L,3{ U4 DATAID, U4 CEID, L,r{ L,2{ U4 RPTID, L,{ <SV values> } } } }` | 事件觸發時送;DATAID=1 |
| S9F3 | Unrecognized Stream/Function | E→H | — | `B[10] MHEAD` | 收到未實作的 primary 時送 |

---

## 3. 資料字典 / Data Dictionary

> ⚠️ HT-160S 的 SVID/CEID 編號為 **本機自有**,與 HT-90XX (tester) 不同。若 host 依 HT-90XX 字典設定報表,
> 請依本節重新對應;HT-160S 的 S2F33/S2F35 會**容忍**未知的 SVID/CEID(見 §4),但那些欄位會回空值。

### 3.1 狀態變數 / Status Variables (SVID) — S1F3/F4, S6F11

**共同段(對齊 HT-90XX)/ Common band (HT-90XX aligned):**

| SVID | 名稱 Name | 型別 | 說明 |
|---|---|---|---|
| 1001 | Machine Model | A | 機型名 = HT-160S |
| 1003 | Software Version | A | 軟體版本 |
| 1021 | UPH | I4 | 每小時產出 |
| 1027 | System Time | A | 系統時間 |
| 1518 | Real/Dummy | I4 | 0=Dummy / 1=Tray Only / 2=Real |

**HT-160S 自有高位段 / HT-160S-specific high band (66000+):**

| SVID | 名稱 Name | 型別 | 說明 |
|---|---|---|---|
| 66000 | Run Mode | I4 | 0=Normal 1=Home 2=OneCycle 3=CleanOut 4=TrayFeed |
| 66001 | System Running | I4 | 1=運轉中 0=停止 |
| 66002 | Control State | I4 | 4=Local 5=Remote 1=Off-Line(GEM 控制狀態鏡像) |
| 66010 | Alarm Active | I4 | 1=有警報顯示中 |
| 66011 | Alarm Code | I4 | 目前警報碼(0=無) |
| 66020 | Total IC | I4 | 本批/本輪處理 IC 數 |
| 66021 | Total Sorted | I4 | 已分選 IC 數 |
| 66030 | Active Lot Count | I4 | 目前載入 Lot 數 |
| 66031 | Current Lot ID | A | 首個 Lot ID |
| 66032 | Sort Mode | I4 | 0=Normal 1=LotBin 2=LotPassFail 3=WhiteList |

**AMR / AGV 段 / AMR band (38xxx):**

| SVID | 名稱 Name | 型別 | 說明 |
|---|---|---|---|
| 38219 | Supplement Bitmap | A | 要料事件站別 `P1:x,...,P9:x`(CEID 272) |
| 38220 | LD/UnLD Status Bitmap | A | 交握中站別(CEID 273) |
| 38221 | LD/UnLD Finish Bitmap | A | 完成站別(CEID 274) |

**AMR 各站 carrier/count SVID / per-station SVIDs**(P1=Loader … P9=Auto6):

| 站 Station | Carrier ID | Tray Count | Device Count | Bin Setting |
|---|---|---|---|---|
| P1 Loader | 38202 | 38222 | 38228 | — |
| P2 Empty | 38203 | 38223 | 38229 | — |
| P3 Color | 38204 | 38224 | 38230 | — |
| P4 Auto1 | 38205 | 38225 | 38231 | 38234 |
| P5 Auto2 | 38206 | 38226 | 38232 | 38235 |
| P6 Auto3 | 38207 | 38227 | 38233 | 38236 |
| P7 Auto4 | 38208 | 38237 | 38240 | 38243 |
| P8 Auto5 | 38209 | 38238 | 38241 | 38244 |
| P9 Auto6 | 38210 | 38239 | 38242 | 38245 |

Carrier ID = A;Tray/Device Count = I4;Bin Setting = A。

### 3.2 設備常數 / Equipment Constants (ECID) — S2F13/F14, S2F15/F16

| ECID | 名稱 Name | 型別 | 單位 | 可寫? | 說明 |
|---|---|---|---|---|---|
| 1501 | Recipe Name | A | — | 唯讀 | 目前配方(Setup File)名 |
| 2758 | Tray X Pitch | FT8 | mm | 可寫(idle) | 對齊 9045 Type1 Pitch X |
| 2759 | Tray Y Pitch | FT8 | mm | 可寫(idle) | 9045 Type1 Pitch Y |
| 2760 | Tray X Start | FT8 | mm | 可寫(idle) | 9045 Type1 Start X |
| 2761 | Tray Y Start | FT8 | mm | 可寫(idle) | 9045 Type1 Start Y |
| 2762 | Tray X Division | I4 | — | 可寫(idle) | 每列格數 X |
| 2763 | Tray Y Division | I4 | — | 可寫(idle) | 每欄格數 Y |

> S2F15 寫入僅在機台 idle(非運轉、機內無 IC)時接受(EAC=2 否則);目前僅 tray-geometry 段可寫,Recipe(1501)唯讀。

### 3.3 事件 / Collection Events (CEID) — S6F11

**操作 / 機台狀態事件 1–31**(每個攜帶 Report 1 = 13-SVID 狀態快照):

| CEID | 事件 | CEID | 事件 |
|---|---|---|---|
| 1 | Handler change status | 17 | Show Alarm |
| 2 | Recipe Change | 18 | Release Alarm |
| 3 | Press Clear Count | 19 | Show Message |
| 4 | Press Start (no IC) | 20 | Release Message |
| 5 | Press Start (with IC) | 21 | Change User |
| 6 | Press Pause | 22 | Enter Setup Page |
| 7 | Press Home | 23 | Enter Maintenance Page |
| 8 | Press One Cycle | 24 | Enter I/O Page |
| 9 | Press Clean Out | 25 | Enter Teach Page |
| 10 | Press Tray Feed | 26 | Enter SECS GEM Page |
| 11 | **Press Lot Start** | 27 | One Cycle Finish |
| 12 | **Press Lot End** | 28 | Clean Out Finish |
| 13 | Press Exit | 29 | Tray Feed Finish |
| 14 | Press Retry | 30 | Time Event |
| 15 | Press Skip | 31 | Switch Real/Dummy |
| 16 | Press Alarm Reset | | |

**Auto 滿盤事件 / Auto Full:** 35=Auto1, 36=Auto2, 37=Auto3, 148=Auto4, 149=Auto5, 150=Auto6。

**AMR / AGV 材料交握事件 / AMR material handoff:**

| CEID | 名稱 | 攜帶報表 | 說明 |
|---|---|---|---|
| 272 | AGVSupplement | Report(SVID 38219) | 要料(哪個站以 P-bitmap 表示) |
| 273 | AGVLDUnLDStatus | Report(SVID 38220) | 交握中 |
| 274 | AGVLDUnLDFinish | Report(SVID 38221) | 上/下料完成 |
| 275 | AGVLdID | Report(carrier-ID SVID) | 載具/Tray ID |

> P-bitmap 站別對應:P1=Loader, P2=Empty, P3=Color, P4-P9=Auto1-6。

### 3.4 遠端命令 / Remote Commands (RCMD) — S2F41/F42

| RCMD | 動作 | 參數 CP | 說明 |
|---|---|---|---|
| `SET_LOT_INFO` | 覆蓋式登記 Lot | Lot id list | 清空後重登;不啟動運轉 |
| `LOTSTART` | 累加式登記 Lot + 預取 2D/Bin | Lot id list | 不啟動運轉(啟動仍為 operator-gated) |
| `START` | 遠端啟動運轉 | — | 需 RUN CHECK / idle 條件 |
| `STOP` | 停機(收尾) | — | |
| `PAUSE` | 暫停 | — | = SystemStart=false, SoftStop=true |
| `HOME` | 回原點 | — | |
| `CLEARCOUNT` | 清除計數 | — | |
| `ONLINE_REMOTE` / `ONLINE` | 控制狀態→Remote(5) | — | 鏡像 66002 |
| `ONLINE_LOCAL` | 控制狀態→Local(4) | — | 鏡像 66002 |
| `START_AGV` | AMR 派車 prep + lock | station / `LoaderTrayCount` … | 記 intent+盤數;實際 motion 由機構/START 驅動 |

> HCACK: 0=成功 / 1=命令無效 / 2=不可執行(如機內有 IC)/ 4=busy。未列命令回 HCACK=1。

### 3.5 警報 / Alarms (ALID) — S5F1, S5F5/F6

- ALID 目錄由機台警報 SSOT(`system\AlarmList.csv` / `mapAlarmCodeList`)**即時**產生,約 480+ 碼。
- 欄位:`AlarmCode, AlarmType, E_ErrMessage, C_ErrMessage, E_Description, C_Description`(中英雙語)。
- S5F1 ALTX = `<code> <English message>`;ALCD bit7 = 0x80(set)/0x00(clear)。
- **完整警報清單請見隨附的 `system\AlarmList.csv`**(過長,不在本規格內展開)。

---

## 4. 動態報表定義與容忍行為 / Dynamic Event Reports & Tolerance

HT-160S 支援標準 GEM 動態報表定義流程:

```
S2F37 (Bool=0, L,0)  → 停用全部事件 (disable all)
S2F33 (RPTID→SVIDs)  → 定義報表      (define reports)   → S2F34 DRACK
S2F35 (CEID→RPTIDs)  → 連結事件↔報表 (link)             → S2F36 LRACK
S2F37 (Bool=1, CEIDs)→ 啟用指定事件  (enable)           → S2F38 ERACK
```

事件觸發時,S6F11 依 CEID→RPTID→SVID 連結序列化當前 SV 值。報表定義**跨重開機保留**(存於 `system\EventReportDef.ini`)。

### 容忍行為(重要)/ Tolerance behavior (important)

為讓依 **HT-90XX 字典**設定的 host(其引用的 SVID/CEID 未必存在於 HT-160S)能順利完成報表定義,HT-160S 採**容忍**策略,而非整批拒絕:

- **S2F33 未知 SVID**:接受定義(不回 DRACK=0x04),該 SVID 於 S6F11 回**空 item**。
- **S2F35 未知 CEID**:接受連結(自動建立為 host CEID,無對應機台事件則不觸發)。
- **S2F35 未知 RPTID**:略過該連結(不影響其餘)。
- **保護**:若對「已存在的機台事件(如 AMR 272-275)」連結到全部未知的 RPTID,**不覆寫**其既有綁定(避免清空)。

> 換言之:host 可完整完成上線與報表定義流程;未對應到 HT-160S 實際資料的欄位會回空值,待雙方確認後再由 HT-160S 補實作或由 host 改用本規格 §3 的 SVID/CEID。

---

## 5. 尚未支援 / Not Yet Supported

以下訊息目前**未實作**(收到會回 S9F3 或僅記錄,不影響上線)。如客戶流程需要,可依需求評估補上:

| 訊息 | 名稱 | 備註 |
|---|---|---|
| S2F23 / F24 | Trace Initialize | 以事件報表(S2F33/35/37 + S6F11)取代 |
| S2F29 / F30 | EC Namelist | 可由現有 EC 表補回覆 |
| S6F15 / F16 | Event Report Request(pull) | host 若改用 pull 模式再補 |
| S6F19 / F20 | Individual Report Request | 同上 |
| S7F1–F26 | Process Program(配方上下傳) | recipe 走本地/FTP;非 event-report 集 |
| S10F3–F6 | Terminal Display | 終端訊息顯示 |
| S14F1 / F2 | GetAttr(物件屬性) | |
| S125F1 / F2 | KYEC 自訂 EC-change report | HT-90XX 自訂 stream |

---

## 6. 對齊 HT-90XX 註記 / Alignment Notes vs HT-90XX

- **命令面 / Command surface**:S1/S2/S5/S6 的基礎 GEM 訊息與 HT-90XX 對齊(見 §2)。
- **字典面 / Dictionary**:HT-160S 為 tray sorter,無 tester 站位/測試結果/接觸次數等概念,故:
  - HT-90XX 的 tester 專屬 SVID(如 1420 32-site 測試結果、6001/6002 Arm 接觸次數、1006/1007 Socket ID)**在 HT-160S 無對應**,以容忍機制回空值或請 host 移除。
  - HT-160S 的機台狀態/產出/AMR 資料集中於 66xxx 與 38xxx 段(§3.1)。
- **建議**:host 端請依本規格 §3 對應 HT-160S 實際 SVID/CEID;無法對應者(tester 專屬)由雙方確認後移除或以容忍空值處理。

---

*本規格依現行 firmware 實作產出,供 KYEC / EAP 整合對接使用。行為之最終確認以現場 host round-trip 為準。*
*This spec reflects the shipping firmware; final behavior is confirmed by on-site host round-trip testing.*
