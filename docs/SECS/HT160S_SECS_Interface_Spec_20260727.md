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
| S6F15 | Event Report Request (pull) | H→E | S6F16 | `U4 CEID` 或 `L,1{ U4 CEID }` | host 主動拉取單一事件 |
| S6F16 | Event Report Data | E→H | — | 同 S6F11 本體,DATAID=1 | 未定義 CEID 回 `L,3{ DATAID, CEID, L,0 }`;解析失敗一律回 CEID=0,**不會不回覆** |
| S6F19 | Individual Report Request | H→E | S6F20 | `U4 RPTID` 或 `L,1{ U4 RPTID }` | host 主動拉取單一報表 |
| S6F20 | Individual Report Data | E→H | — | `L,n{ <SV values> }`(扁平,不含 RPTID) | 由**即時**登錄表取值,故 host 以 S2F33 重定義該 RPTID 後長度會跟著變;未定義回 `L,0` |
| S9F3 | Unrecognized Stream/Function | (未上線) | — | — | **現況:本機收到未實作的 primary 時僅記錄於本機 SECS log,不送出 S9F3**。故 host 對未實作的 primary 會等到 T3 逾時而非收到 S9F3。如客戶流程需要真正送出,可另行評估補上 |

### Stream 10 — 終端訊息 / Terminal Display

| S/F | 名稱 Name | 方向 | 回覆 | 本體 Body (SECS-II) | 說明 Notes |
|---|---|---|---|---|---|
| S10F3 | Terminal Display, Single | H→E | S10F4 | `L,2{ B TID, A TEXT }` | host 送單段文字 |
| S10F4 | Terminal Display Acknowledge | E→H | — | `B ACKC10` | ACKC10=0 已接受 |
| S10F5 | Terminal Display, Multi-block | H→E | S10F6 | `L,2{ B TID, L,n{ A TEXT } }` | host 送多段文字 |
| S10F6 | Terminal Display Multi Acknowledge | E→H | — | `B ACKC10` | ACKC10=0 已接受 |

> **顯示行為**:HT-160S 收到後寫入 SECS log 與 EventLog,**不彈出強制視窗**。原因:handler 執行在 HSMS 接收執行緒上,本機所有訊息視窗皆為 ShowModal 且會暫停 MainProc,於此處彈窗會使 SECS 通訊本身停擺;且本機訊息視窗機制會停止所有馬達。文字內容不由韌體判讀嚴重性(現場語料證實以關鍵字或 TID 判別皆不可靠)。

### Stream 125 — EC 變更回報 / EC Change Report (HT-90XX 自訂)

| S/F | 名稱 Name | 方向 | 回覆 | 本體 Body (SECS-II) | 說明 Notes |
|---|---|---|---|---|---|
| S125F1 | Enable/Disable EC Data | H→E | S125F2 | `L,2{ B[1] ALED, L,n{ U4 ECID } }` | host 指定要回報變更的 EC。`ALED` 為 **bit-7 旗標**:`0x80`=啟用 / 未設 bit7(如 `0x01`)=停用。**外層必須是 L,2**,缺 `ALED` 會被判格式錯 |
| S125F2 | Enable/Disable EC Data Acknowledge | E→H | — | `B` 0=接受 / 1=拒絕 | **每個 request 只回一次**(HT-90XX 為每個 ECID 各回一次,本機不沿用) |

> **本機回 `ACK=1` 的兩種情況,請勿誤判為故障**:
> (1) 本體格式不符上表;
> (2) 清單中含**本機未註冊的 ECID**。本機只註冊 7 個 EC(§3.2:1501、2758–2763),
> 若對未註冊的 ECID 回 0,等於承諾「該 EC 變更會回報」——本機沒有 EC 變更事件,永遠不會發生,故據實回 1
> (與 HT-90XX 對未註冊 ECID 回 1 的行為一致)。SECS log 會列出未註冊的 ECID 清單供雙方比對。

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

**Auto 滿盤事件 / Auto Full:** 35=Auto1, 36=Auto2, 37=Auto3, 148=Auto4, 149=Auto5, 150=Auto6。(全數對齊 HT-90XX 同號同義)

**Auto 出盤事件 / Auto Unloading tray:** 136=Auto1, 137=Auto2, 138=Auto3, 145=Auto4, 146=Auto5, 147=Auto6。(全數對齊 HT-90XX 同號同義)

> **本組事件預設本體為空報表**:HT-160S 於各 Auto 站出盤時送出 S6F11,但**預設不攜帶任何報表**,本體為
> `L,3{ U4 DATAID, U4 CEID, L,0 }`。這與 HT-90XX 對同一組 CEID 的行為一致(京元 2026-06-08 log 之 CEID 136 即為空報表)。
>
> **但 host 可自行加掛報表**:本組六個 CEID 在韌體中雖未預先綁定報表,**出盤時確實有發射點**,
> 故 host 以 `S2F33` 定義報表後再以 `S2F35` 連結到 136/137/138/145/146/147,**下一次出盤起即會攜帶該報表的 SV 值**
> (本機於發送時才向登錄表取值)。這一點與「本機完全沒有的 CEID」不同 —— 後者雖同樣被 `S2F35` 接受,
> 但因無發射點而永遠不會送出。若不想加掛報表,亦可改訂閱 `274 AGVLDUnLDFinish`(已帶 Report 4 + Report 6)
> 或以 `S1F3` 主動查詢。

**AMR / AGV 材料交握事件 / AMR material handoff:**（全部 DataID=1,對齊 HT9045 / all DataID=1, aligned to HT9045)

| CEID | 名稱 | 攜帶報表 | 說明 |
|---|---|---|---|
| 272 | AGVSupplement | Report 2(SVID 38219 bitmap)**+ Report 6** | 要料;**同時帶各站 Tray Count + Device Count** |
| 273 | AGVLDUnLDStatus | Report 3(SVID 38220 bitmap) | 交握中(尚未計數,不帶 count) |
| 274 | AGVLDUnLDFinish | Report 4(SVID 38221 bitmap)**+ Report 6** | 上/下料完成;**帶收尾 Tray Count + Device Count** |
| 275 | AGVLdID | Report 7(SVID 38204) | Color 身分 Tray 2D(見下) |

> P-bitmap 站別對應:P1=Loader, P2=Empty, P3=Color, P4-P9=Auto1-6。

**下料 Tray/Device Count 上傳(對齊 HT9045 iAMRTrayCount / iAMRDeviceCount)/ Unload count upload:**

- CEID **272(要車)與 274(完成)** 皆附 **Report 6**;Report 6 = 9 站 **Tray Count**(SVID 38222–38227 / 38237–38239)接著 9 站 **Device Count**(SVID 38228–38233 / 38240–38242),與 HT9045 的 `iAMRTrayCount[]` / `iAMRDeviceCount[]` **同編號同語意**。
- 下料(Auto 站)時兩值皆填真值:`TrayCount = 該車盤數 (Car->iTrayCount)`;`Device Count = 該 Auto 車的 IC 累計`(卸料時逐盤 `+= Tray.CountIC()` 累加)。
- 上料方向(Loader/Empty/Color)僅交換盤數,Device Count 保持 0(與上下料契約一致:上料交換盤數、下料才給 IC 件數 — HT9045/HT160 皆然)。

**Color 身分 Tray 2D 上傳(CEID 275 AGVLdID,對齊 HT9045)/ Color identity-tray 2D upload:**

- 由 Color CCD 於 Loader 回收進料點掃描身分 Tray 2D,掃描完成即以 S6F11 **CEID 275** 上傳。
- 值寫入 **SVID 38204**(Color P3 carrier ID),經 Report 7 送出;DataID=1(對齊 HT9045)。
- 對齊 HT9045 AGVLdID(load id);HT9045 的 report 內容為 host 動態定義,HT-160S 現以 Report 7 單一身分 SVID 為預設,host 亦可經 S2F35 改綁自己的報表。

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
| `ONE_CYCLE` | 執行單一循環後停機(Clean Out 進行中為例外,見下方註) | 無 (空 list) | 0=已啟動 / **2=機台未運轉**(不可執行)/ 4=已在 OneCycle 中 / 2=模式或 Lot 資料不符。**與 HT9045 不同**:9045 一律回 0,HT-160S 據實回報。註:機台未運轉一律回 2 而非 4——SEMI E5 的 HCACK=4 語意是「已受理,稍後以事件通知完成」,而停機時本機不會發出 CEID 27 完成事件,回 4 會讓 host 無限等待 |
| `ENERGY_SAVING` | (不支援) 省電模式 | `STATE` = 0/1 | **固定回 HCACK=2**。HT-160S 無加熱器/ATC/省電子系統,語法正確亦拒絕,不謊報成功。與京元現行 HT9045 回覆一致 |
| `PP_SIGNALTOWER` | 主機強制指定三色燈 | `RED`/`GREEN`/`YELLOW` = 0關/1亮/2閃 | 空 list = 解除 (可重複)。**請每次同時指定三色**(京元現行 host 即如此):未列出的顏色沿用「本次覆寫期間」的既有值,而解除會把三色記憶歸零,故解除後只指定一色的 SET 會使另兩色為關。未知 CP 或值超出 0..2 則整包拒絕 (HCACK=2) 且不套用。警報 Note 顯示中、訊息視窗顯示中、或機台自身處於安全異常狀態(EMG/安全門/安全鎖/斷氣/離子風扇,即 RunState=LED_ErrJam)時暫停覆寫,避免遮蔽機台紅燈。值 2(閃)在本機依既有慣例呈現為**恆亮**(HT-160S 塔燈不閃) |
| `PP_MUSIC` | 主機強制指定蜂鳴器音效 | 單一 pair,CP 名稱為空字串 `A[0]`,值 = 1..4 | 空 list = 解除 (可重複)。CP 名稱讀取後即丟棄不比對 (京元送空字串);值超出 1..4 回 HCACK=2 |

> HCACK: 0=成功 / 1=命令無效 / 2=參數錯,或**認得命令但本機不執行**(如 `ENERGY_SAVING`、`START_AGV` 未知 CP / AMR 關閉)/ 4=busy(運轉中、機內有 IC、或已在 OneCycle)。未列命令仍回 HCACK=1。
> **`ONE_CYCLE` 於 Clean Out(排料)進行中不會停機**:本機允許在 Clean Out 中受理 `ONE_CYCLE`(回 0),
> 循環結束時會**回到 Clean Out 繼續排料而不停機**,但仍會送出 CEID 27「One Cycle Finish」。
> 也就是說在 Clean Out 期間,CEID 27 代表「該循環結束」而非「機台已停」。若 host 需要「必定停機」的語意,
> 請在非 Clean Out 狀態下發送。

> **`ONE_CYCLE` 為此對照表的例外**,請逕依 §3.4 該列:對 `ONE_CYCLE` 而言「運轉中」是回 **0**(正常受理)的狀態,「已在 OneCycle」只在運轉中才回 4,「機台未運轉」回 **2**。原因是 HCACK=4 依 SEMI E5 為肯定回覆(稍後以事件通知完成),而唯有運轉中的循環才會真的送出 CEID 27 完成事件。

> `PP_SIGNALTOWER` / `PP_MUSIC` 為**閂鎖式覆寫**,不會自動逾時解除。操作員解除方式:面板 ALARM RESET 鍵、警報/訊息視窗的 OFF BUZZER、或**維修畫面 SECS/GEM 分頁的「Release Host Override」按鈕**(該分頁同時顯示目前是否處於覆寫中)。此外,HSMS 連線中斷、host 送 S1F15 轉為 OFF-LINE、或操作員開啟**維修畫面的 IO Set View(手動 IO 測試)**時會**自動解除**——避免 host 消失後閂鎖無人可解,也避免手動 IO 測試期間主迴圈暫停、覆寫輸出被凍結在「蜂鳴器持續作響且無解除途徑」的狀態。

> **塔燈「閃」在本機呈現為恆亮——這是客戶端看得見的差異,請一併知會現場**。
> `PP_SIGNALTOWER` 的值域為 0=關 / 1=亮 / 2=閃,本機接受 2 但依既有慣例(HT-160S 塔燈全機不閃)**呈現為恆亮**。
> 因此同一封 `RED=2 GREEN=2 YELLOW=2` 的封包,在 HT-90XX 是三燈同步閃爍,在 HT-160S 是三燈同時恆亮。
> 附帶說明:host 確實會送**各色不同值**的封包(京元 2026-06-08 六筆 SET 中有一筆為 `RED=2 GREEN=0 YELLOW=0`,
> 其餘五筆為 `2/2/2`),故本機逐色處理而非整組同值處理。

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

- **CEID 編號語意差異(重要)**:HT-160S 的 CEID 1–31 與 HT-90XX **同號不同義**(例:CEID 1 在 HT-160S 為 `Handler change status`,在 HT-90XX 為 `Start Pressed`;CEID 27 在 HT-160S 為 `One Cycle Finish`,在 HT-90XX 為 `Change Machine State`)。自 20260728 起本機會回覆 `S6F15`,亦即 host 拉取 CEID 1 時會取得**本機語意**的資料,而非 HT-90XX 語意。host 端請一律以本規格 §3 的 CEID 對照為準;若 host 沿用 HT-90XX 字典,請於雙方確認後調整 host 設定或另行協議編號對照。

> 換言之:host 可完整完成上線與報表定義流程;未對應到 HT-160S 實際資料的欄位會回空值,待雙方確認後再由 HT-160S 補實作或由 host 改用本規格 §3 的 SVID/CEID。

#### 4.x「同一件事、不同號碼」對照(host 必讀)

下列三個事件**兩邊都有,但編號不同,而且對方的號碼在本機另有他義**。這是最容易誤讀的一組:

| 事件 | HT-90XX CEID | HT-160S CEID | 該號碼在對方機台的意義 |
|---|---|---|---|
| One Cycle 完成 | 41 | **27** | 27 在 HT-90XX = `Change Machine State` |
| Clean Out 完成 | 42 | **28** | 28 在 HT-90XX = `Retry Pressed` |
| Tray Feed 完成 | 49 | **29** | 29 在 HT-90XX = `Skip Pressed` |

> **CEID 27 需特別注意**:自本版起 HT-160S 於 One Cycle 完成時**確實會發射 CEID 27**(先前該號碼已註冊但無任何發射點)。
> 而 `Change Machine State` 是京元現場 HT-90XX **當天發射量最大的事件**(2026-06-08 約 406 次 S6F11)。
> 若 host 沿用 HT-90XX 字典,會把本機的「單循環完成」解讀為「機台狀態改變」。請以本規格 §3.3 為準。

> **HT-160S 未提供對應 S5F1**:HT-90XX 於 One Cycle 完成時另發一筆 S5F1(ALID `316001640`, ALTX `One cycle finish`)。
> HT-160S 只發 S6F11 事件,不發此告警。若 host 是以該 ALID 判斷循環結束,請改為訂閱 CEID 27,或與我方確認後補上。

---

## 5. 尚未支援 / Not Yet Supported

以下訊息目前**未實作**(收到僅記錄於本機 SECS log,**不回覆任何訊息**,故 host 端會是 T3 逾時;不影響上線流程)。如客戶流程需要,可依需求評估補上:

> 20260728 更新:`S6F15/F16`、`S6F19/F20`、`S10F3–F6`、`S125F1/F2` 已實作完成並移入 §2,不再屬於本節。此四組是京元現場 log (2026-06-08) 證實 host 會主動送出、而本機先前不回覆造成 T3 逾時的訊息。

| 訊息 | 名稱 | 備註 |
|---|---|---|
| S2F23 / F24 | Trace Initialize | 以事件報表(S2F33/35/37 + S6F11)取代 |
| S2F29 / F30 | EC Namelist | 可由現有 EC 表補回覆 |
| S7F1–F26 | Process Program(配方上下傳) | recipe 走本地/FTP;非 event-report 集 |
| S14F1 / F2 | GetAttr(物件屬性) | |

---

## 6. 對齊 HT-90XX 註記 / Alignment Notes vs HT-90XX

- **命令面 / Command surface**:S1/S2/S5/S6 的基礎 GEM 訊息與 HT-90XX 對齊(見 §2)。
- **字典面 / Dictionary**:HT-160S 為 tray sorter,無 tester 站位/測試結果/接觸次數等概念,故:
  - HT-90XX 的 tester 專屬 SVID(如 1420 32-site 測試結果、6001/6002 Arm 接觸次數、1006/1007 Socket ID)**在 HT-160S 無對應**,以容忍機制回空值或請 host 移除。
  - HT-160S 的機台狀態/產出/AMR 資料集中於 66xxx 與 38xxx 段(§3.1)。
- **建議**:host 端請依本規格 §3 對應 HT-160S 實際 SVID/CEID;無法對應者(tester 專屬)由雙方確認後移除或以容忍空值處理。

---

## 7. HT-160S 專屬項目與對齊狀態 / HT-160S-Specific Items & Alignment Status

> 依客戶要求:凡 HT-160S 使用到的 SECS 命令皆對齊 HT-90XX (HT9045)。§7.1 列已對齊項;§7.2 列 HT-160S 專屬(9045 無對應或同號不同義)項目及其存在理由。
> Per customer requirement, every SECS command HT-160S uses is aligned to HT9045. §7.1 = aligned; §7.2 = HT-160S-specific with rationale.

### 7.1 已對齊 HT9045 / Aligned to HT9045
- **訊息 Messages**:S1 / S2 / S5 / S6 基礎 GEM 全數對齊(§2)。
- **RCMD**:`PAUSE` `START` `STOP` `HOME` `LOTSTART` `ONLINE_REMOTE` `ONLINE_LOCAL` `START_AGV` `ONE_CYCLE` `ENERGY_SAVING` `PP_SIGNALTOWER` `PP_MUSIC`。
  - 三項宣告差異:`ONE_CYCLE` 據實回 HCACK(9045 一律 0);`ENERGY_SAVING` 固定回 2(本機無省電子系統,與京元 9045 現行回覆相同);`PP_SIGNALTOWER`/`PP_MUSIC` 在警報 Note 顯示期間、訊息視窗顯示期間、以及機台自身安全異常(RunState=LED_ErrJam)時暫停覆寫(9045 無此例外),以免遮蔽機台自身紅燈與警報音;值 2(閃)呈現為恆亮。
- **SVID 共同段**:1001 / 1003 / 1021 / 1027 / 1518。
- **ECID**:1501,2758–2763(Type1 tray 幾何)。
- **CEID**:AMR 272 / 273 / 274 / 275;Auto-Full 35 / 36 / 37 / **148 / 149 / 150**;Auto-Unloadtray **136 / 137 / 138 / 145 / 146 / 147**。
  - 上列 Auto-Full 六號與 Auto-Unloadtray 六號**皆與 HT-90XX 同號同義**(依 HT-90XX 韌體 CEID 目錄 `EventReport_CEID.def`:148/149/150 = `Auto 4/5/6 Full`,136–138/145–147 = `Auto 1–6 Unloading tray`)。
  - 註:HT-160S 有 6 個 Auto 輸出站,故六號全部會用到。京元 2026-06-08 當天的 HT-90XX log 中,本家族只觀察到 `136`(2 次)與 `137`(3 次)實際發射(兩者皆為空報表),其餘號碼當天未出現 —— 但**編號與語意皆為 9045 韌體既有定義**,並非 HT-160S 自創,故列為「已對齊」而非「HT-160S 專屬」。
- **AMR 資料**:Tray/Device Count(SVID 38222+ / 38228+)、Color 身分 2D(CEID 275 / SVID 38204)、事件 DataID=1 —— 皆對齊 HT9045(見 §3.3)。

### 7.2 HT-160S 專屬(9045 無對應)+ 為何特殊 / HT-160S-only + rationale

| 項目 Item | 類別 | 用途 Purpose | 為何 HT-160S 專屬 Why HT-160S-specific |
|---|---|---|---|
| `SET_LOT_INFO` | RCMD | 一次**覆蓋式**登記整批 Lot(清空後重登) | 9045 用 `START_LOT`(keyed)+ `LOTSTART` 按鈕;HT-160S 的 lot 模型支援批次多-Lot 一次載入,故自有此命令 |
| `ONLINE`(裸) | RCMD | = `ONLINE_REMOTE` 別名 | 便利別名;9045 僅有 `ONLINE_REMOTE` / `ONLINE_LOCAL` |
| `CLEARCOUNT` | RCMD | host 遠端清除生產計數 | 9045 將 clear-count 僅作操作員事件(CEID 5),無對應 RCMD(9045 另有 `CLEAN_AUTO_SORT_COUNT`,語意不同) |
| SVID **66000–66032** | SVID | 機台狀態/產出/Lot/分選模式:RunMode(66000)、SystemRunning(66001)、ControlState(66002)、AlarmActive(66010)、AlarmCode(66011)、TotalIC(66020)、TotalSorted(66021)、ActiveLotCount(66030)、CurrentLotID(66031)、SortMode(66032) | sorter 特有資料,9045(tester)無對應;刻意置於 **66000+ 高位段**以絕不與 9045 的 SVID 段碰撞 |
| SVID **38237–38245** | SVID | Auto4/5/6 的 carrier / tray / device / bin-setting | HT-160S 有 **6 個 Auto 輸出站**,9045 僅 3(SVID 到 38236);為第 4–6 站延伸 |
| CEID **1–31** 編號 | CEID | HT-160S 自有操作 / UI / 機台狀態事件集(見 §3.3) | **同號不同義**:HT-160S 的操作事件集與 9045 的 CEID 1–37 語意不同。host 以 S2F35 綁定報表到所需 CEID 時,**請依本規格 §3.3 的意義**,勿套用 9045 的 CEID 語意。AMR(272–275)與 Auto-Full(35/36/37)則同號同義。 |

> **無 HT-160S 專屬的 SxFy 訊息,亦無專屬 ECID** —— HT-160S 未自創任何 SECS 訊息(§2 皆標準 GEM),ECID 全數對齊 9045。

---

*本規格依現行 firmware 實作產出,供 KYEC / EAP 整合對接使用。行為之最終確認以現場 host round-trip 為準。*
*This spec reflects the shipping firmware; final behavior is confirmed by on-site host round-trip testing.*
