# HT-160S SECS/GEM 介面規格書 / SECS/GEM Interface Specification

> **機型 Model:** HT-160S (Tray Sorter) &nbsp;|&nbsp; **MDLN:** `HT-160S` &nbsp;|&nbsp; **SOFTREV:** `1.0.0.0`
> **文件版本 Doc rev:** 2026-08-04
> **依據 Based on:** current firmware build (branch `feat/iosetview-172-refactor`)
>
> 本規格反映 HT-160S **目前實作**的 SECS-II / GEM 介面。命令面向 HT-90XX (HT9045) 對齊;
> **CEID 字典自 2026-07-29 起為 HT-90XX 的逐字複本**(號碼 **1–292** 全數註冊,別名逐字相同,見 §3.3);
> **SVID 字典自 2026-08-04 起全部為 HT-90XX 家族編號**(見 §3.1):凡 HT-90XX 有同概念的資料,
> 一律沿用 HT-90XX 的號碼、型別與線上格式;唯一的延伸段是 AMR / AGV 用的 **38xxx** 段。
> 原本承載 sorter 特有資料的 **66xxx 自有段已整段退役**(見下方 ⚠ 段落與 §3.1 的退役墓碑表)。
> This document describes the SECS-II / GEM interface **as currently implemented** on HT-160S.
> The command surface is aligned to HT-90XX. **As of 2026-07-29 the CEID dictionary is a verbatim copy of
> HT-90XX's** (ids **1–292** all registered, aliases character-identical, §3.3). **As of 2026-08-04 every
> published SVID is an HT-90XX family id** (§3.1): a common band that reuses HT-90XX's ids, types and wire
> formats wherever the same concept exists, plus the **38xxx** AMR/AGV band as the only extension. The former
> HT-160S-only **66xxx band has been retired in full** — see the ⚠ note below and the tombstone table in §3.1.
>
> ⚠️ **2026-07-29 為 breaking change**:舊版宣告的 CEID 1–31 語意已作廢,host 端必須依 **§3.3.5 對照表**
> 重新設定。最高風險為 CEID **27** 與 **28** —— 新舊字典都有這兩個號碼但意思不同,未更新會靜默誤讀。
> A host that is not re-provisioned will silently misread CEID **27** and **28**.
>
> **2026-08-03 修訂(本段列出的號碼皆為新增;同日另有三項 breaking change,見下方 ⚠ 段落):** 補齊 16 個
> host 已在引用、但本機先前回空 item 的 SVID —— **3**(GemClock)、**1002**(Machine ID)、**1009**
> (Lot Start Time)、**1103–1105**(Auto1–3 Count)、**1501**(Setup File)、**2758–2763**(Type 1 Tray
> 幾何),以及 **1259–1261**(Auto4–6 Count;本版初版時為 66025–66027,同日改號,見下方 ⚠ 段落)。號碼、型別與線上格式皆比對 HT-90XX 原始碼
> 與京元現場 log 後對齊。同時更正 §6 對 SVID 1006 的錯誤描述。詳見 §3.1。
>
> **同日追加(仍為僅新增):控制狀態改用 HT-90XX 編號** —— 新增 **SVID 4 GemControlState**
> (1=Off-Line / 2=On-Line Local / 3=On-Line Remote,U1)與 **SVID 9 PreviousGemControlState**,
> 並在狀態變更時發 **CEID 141** 與 **91 / 92 / 93**(此四個號碼先前已註冊但無發射點)。既有的
> SVID 66002 保留不變(GEM 標準值域 1/4/5)。詳見 §1、§3.1、§3.3.1。
> 【**2026-08-04 更新**:本段最後一句已不成立 —— **66002 已移除**,控制狀態僅由 **SVID 4 / 9** 公佈。
> 機內部仍以 GEM 標準值域 1/4/5 儲存,但不再對外公佈。】
>
> ⚠️ **同日追加(此項為 breaking change):重複資訊收斂到 HT-90XX 編號** —— 依「相同功能使用同一
> 編號」的原則,四個與 HT-90XX 號碼**同值、同來源**的本機自有號碼已**移除**:
> **66022 / 66023 / 66024**(Auto1–3 Count,與 **1103 / 1104 / 1105** 指向同一組計數器)與
> **66033**(Lot Start Time,與 **1009** 為同一個閂鎖、同一時刻,僅分隔符不同)。
> 這四個號碼自本版起**不再註冊**,S1F3 查詢會得到「未知 SVID」而非數值,S1F11 名稱表也不再列出。
> **若貴端已在 S2F33 報表定義中綁定這四個號碼,請改綁 1103 / 1104 / 1105 / 1009。**
> 詳見 §3.1。
>
> ⚠️ **同日追加(此項亦為 breaking change):Auto4–6 Count 改用 HT-90XX 家族編號** ——
> **66025 / 66026 / 66027** 原本以「HT-90XX 無對應號碼」為理由留在本機自有段。該理由**已被推翻**:
> HT-90XX 家族中的 **HT-9011UC V3.33.899** 本身就是**六站 Auto** 機台,並以
> **1259 / 1260 / 1261** 命名 Auto4–6 Count(其後 1262 起為 Fix7–12)。依「相同功能使用同一編號」
> 的原則,本機自本版起改用 **1259 / 1260 / 1261**,**66025–66027 不再註冊**。
> **若貴端已在 S2F33 報表定義中綁定 66025–66027,請改綁 1259 / 1260 / 1261。**
> 66022–66027 這六個號碼**永不遞補、永不改用他義**。詳見 §3.1。
>
> ⚠️ **同日追加(此項亦為 breaking change):Auto4–6 的 AMR Carrier ID 改用家族編號** ——
> 同一條原則(**Auto1–3 遵循 HT-9046LS V3.32.810、Auto4–6 參照 HT-9011UC V3.33.899**)套用到 AMR 盤號欄位:
> Auto1–3 維持 **38205 / 38206 / 38207**(810 定義,不變),Auto4–6 由本機自創的 38208 / 38209 / 38210
> 改為 899 的 **38199 / 38200 / 38201**(`Output 4/5/6 Tray ID`)。**若貴端已綁定 38208–38210,請改綁
> 38199–38201。** Auto4–6 的 tray / device / bin-setting(**38237–38245**)**維持不變**——兩棵參照樹皆無
> 可對齊的號碼(810 的 AMR 段止於 38236、899 無 AMR SVID 家族)。詳見 §3.1 的 AMR 對照表。
>
> **同日追加:CEID 字典依貴端現場機台補齊至 292(僅新增)** —— 以貴端 HT-9046 現場機台自己存下的
> `EventReport_CEID.def` 逐筆核對:**1–275 與本機 0 筆不符**;該機另有 **276–292** 共 17 個號碼,
> 本版已全部註冊(名稱逐字照抄),`S1F23`/`S1F24` 全查由 `L,275` 變為 **`L,292`**(約 32 KB)。
> 這 17 個**目前只註冊、無發射點**。同時更正 CEID **272–275** 在 `S1F24` 回的名稱字串,改與貴端機台一致
> (`AMR Supplement` 等,原為 `AGVSupplement` 等)——**號碼與行為未變**。詳見 §3.3、§3.3.2.1。
>
> **同日追加(僅新增):SVID / ECID 1007 Operator ID** —— 本機新增操作員身分欄位(主畫面右側設定區,
> 預設 `Operator`,存 `General.ini`,跨電源保存),並以 **SVID 1007** 公佈、同號 **ECID 1007** 註冊。
> ⚠️ **本號接受貴端以 `S2F15` 寫入,且是唯一在運轉中也可寫的 EC**(其餘可寫 EC 限機台 idle)——
> 貴端 2026-06-08 對 HT-90XX 的實際用法即為運轉中寫 `1007 = "AGV"`。寫入成功回 `EAC=0`、
> 主畫面即時同步、寫入設定檔保存、並發 **CEID 48**;空字串回 `EAC=3`。
> ⚠️ **並更正本文件先前的建議**:§6 原稱「1007 無對應,建議 host 自報表定義移除」,該建議**作廢**,
> 請保留 1007。至此貴端 **RPTID 502 的 8 格中有 7 格有值**,僅餘 **1513 Tester On/Off** 無對應。
> 詳見 §3.1、§3.2、§6。
>
> ⚠️ **2026-08-04 修訂(此項為 breaking change):HT-160S 自有 66xxx 段全數退役,只公佈 HT-90XX 家族編號** ——
> 依貴端裁定「HT-160S 只公佈 HT-90XX 家族編號」,自有高位段最後留存的十個 SVID
> **66000 / 66001 / 66002 / 66010 / 66011 / 66020 / 66021 / 66030 / 66031 / 66032** 已**全部不再註冊**
> (2026-08-03 已先移除 66022–66027 與 66033,至此整段為空);同時新增家族編號
> **SVID 1008 Run Mode**(ASCII,值域 `0:Normal; 1:RT; 2:EQC`,本機恆為 `"0"`)取代 66000。
> 註冊 SVID 數由 **76 → 67**。逐號替代方案見 **§3.1 的退役墓碑表**。
> 這十個號碼**永久退役、永不遞補、永不改用他義**,故未更新的 host 綁定一定是「查不到值」而非「查到別的值」:
> `S1F3` 回可偵測的**空 list item `<L[0]>`**(位元 `01 00`),與「已註冊但值為空字串」的 `<A[0]>`(位元 `41 00`)
> 可明確區分,不存在靜默誤讀的風險。**若貴端已在 `S2F33` 報表定義中綁定這十個號碼,請依墓碑表改綁。**
> ⚠ **已知並接受的代價**:分選模式(原 66032)在 HT-90XX 家族中沒有可用編號,host 仍可用
> `S2F41 LOTSTART` 的 `SORTMODE` pair **設定**,但**無法再以 `S1F3` 回讀**。
>
> **本次有兩項客戶端看得見的後果,請務必知會現場:**
>
> 1. **預設 Report 1 縮成 `{1027 System Time}` 一格**(與 HT-90XX 的 `AddReprot` 及貴端 HT-9046 自存的
>    `EventReport_ReportID.def` 逐字相同),原本 13 格的機況快照不再存在;292 個 CEID 中有 288 個掛在
>    Report 1 上,故絕大多數 `S6F11` 現在只帶一個時戳,`S1F24` 的對應 288 列也由 13 個 VID 縮為 1 個。
>    ⚠ **`S2F35` 的事件→報表連結不跨電源保存**,因此**每次復電後這 288 個號碼都會退回 Report 1** ——
>    2026-08-04 之前這個退場報表還帶著 13 個機況 SV,**現在只有時戳**;host 復電後必須重跑
>    `S2F33` + `S2F35` 才能拿回批號與機況。Report 2–7(AMR)**完全未變**。詳見 §3.3.3。
> 2. **SVID 1006 Lot ID 改為回「全部已登錄批號、以半角逗號串接」**(不含空白)。單批時位元與舊版**完全相同**,
>    多批時形如 `PROBE_LOT_A,PROBE_LOT_B,PROBE_LOT_C`。⚠ **1006 是貴端已保存的 `RPTID 502` 第 1 格**,
>    故此值會直接出現在該既有事件報表內(**item 數與 A 型別皆未變**,封包形狀不動);若貴端把第 1 格當
>    單一批號 token 解析,請改為可處理多批的解析。詳見 §3.1、§3.3.3、§6。
>
> As of 2026-08-04 HT-160S publishes **HT-90XX family SVIDs only**: the ten remaining 66xxx ids are
> **unregistered** (new family id **1008 Run Mode**, ASCII, permanently `"0"`, replaces 66000; 76 → 67 SVIDs).
> Two customer-visible consequences: default **Report 1 is now `{1027}`** and the `S2F35` links are session-only,
> so after **every power cycle** all 288 ids fall back to a timestamp-only report until the host re-runs
> `S2F33` + `S2F35`; and **SVID 1006** now answers every registered lot id joined by commas — it is slot 1 of
> your persisted `RPTID 502`, so a host parsing that slot as a single token must be updated.

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
- 連線參數設定於 `system\General.ini` 的 **`[SECS]`** 段(**不在 `ComPort.ini`**;該檔無 SECS 設定)。
  現場實機值(取自 2026-07-31 京元竹南 State Record 的 `MachineConfig\system\General.ini`,以現場資料為準):
  `Address=192.168.8.3`、**`Port=6000`**、**`DeviceID=0`**、**`ActiveMode=0`**(= passive,設備監聽)、
  `ReconnectInterval=60`、`LinktestInterval=10`、`T6Timeout=6`。
  > 2026-08-03 更正:本節前版寫「見 `ComPort.ini`」與「Device ID = 1」,兩者皆與現場設定不符,已依 State Record 更正。
- 型號 / 版本 由 S1F2 與 S1F14 回報:`MDLN = HT-160S`,`SOFTREV = 1.0.0.0`。
- **GEM 控制狀態 Control State** —— 以 **SVID 4 GemControlState**(1=Off-Line / 2=On-Line Local / 3=On-Line Remote,HT-90XX 值域)與 **SVID 9 PreviousGemControlState** 公佈。**自 2026-08-04 起這是唯一公佈的控制狀態編號**(原並存的 SVID 66002 已移除;機內部仍以 GEM 標準值域 1/4/5 儲存,但不再對外公佈)。變更時送 **CEID 141**,緊接著送 **91 / 92 / 93** 中對應新狀態的一個(與 HT-90XX 同順序)。
- ⚠ **控制狀態目前不作為命令閘門**:本機在 Off-Line / On-Line Local 狀態下仍會受理並執行 S2F41 遠端命令。
  **與 HT-90XX 的對照(2026-08-03 逐段查證後更正前版敘述)**:HT-90XX 的 S2F41 處理器同樣**沒有**任何控制狀態檢查
  (其 `S2F42_Host_Command_Acknowledge` 全函式查無 `bOnLine` / `GemControlState`),這一點兩機一致;但 HT-90XX
  **另有兩項本機沒有的控制狀態機制**——(1) 對 host 的 S1F17 有**操作員否決權**:已在線回 `ONLACK=2`、
  操作員勾選 `AcceptHostOnlineRequest` 才回 `0`、未勾選回 **`1` 拒絕**(本機一律硬回 `0`);
  (2) **離線時不主動發送**事件與警報。前版寫「其控制狀態閘僅對另一家客戶代碼生效」只對「S/F 收訊閘門」成立,
  對上述兩項並不成立,特此更正。
  若需依 GEM 規範在 Off-Line 拒絕命令,尚待貴端確認三件事:要拒絕哪些命令(全部,或保留 `ONLINE_*` 以免無法上線)、
  拒絕時回哪個 HCACK、On-Line Local 是否也拒絕。
- 事件 / 警報推播(S6F11 / S5F1)僅在 **HSMS SELECTED** 時送出。
- 標準上線序列(host):`S1F13 → S1F17 → S2F37(disable all) → S5F3 → S2F33(define) → S2F35(link) → S2F37(enable)`,全數支援(見 §2、§4)。

**控制狀態對照 / Control state map:**

| SVID 4 值 | 狀態 | 進入方式 | 變更時發射 |
|---|---|---|---|
| 1 | Off-Line | S1F15/F16,或 RCMD `ONLINE_LOCAL` 前 | CEID 141 + 91 |
| 2 | On-Line Local | RCMD `ONLINE_LOCAL` | CEID 141 + 92 |
| 3 | On-Line Remote | S1F17/F18,或 RCMD `ONLINE_REMOTE`/`ONLINE` | CEID 141 + 93 |

> **2026-08-04**:本表原有的「SVID 66002 值」欄(GEM 標準值域 1/4/5,開機且尚無 host 交握時為 0)已移除
> —— **66002 不再註冊**,控制狀態只由 **SVID 4 / 9** 公佈。機內部儲存值域不變,只是不再對外公佈。
> The former "SVID 66002 value" column is gone: 66002 is unregistered and SVID 4 / 9 are now the only
> published control state.

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
| S1F18 | ON-LINE Acknowledge | E→H | — | `B ONLACK` | ONLACK=0,控制狀態轉 On-Line Remote(**SVID 4 = 3**;2026-08-04 前本欄寫「→5」,那是已退役 66002 的 GEM 標準值域,機內部仍存 5 但不再對外公佈) |
| S1F23 | Collection Event Namelist Request | H→E | S1F24 | `L,n{ CEID }`(n=0=全部) | **本次新增 (2026-07-30)** |
| S1F24 | Collection Event Namelist Reply | E→H | — | `L,n{ L,3{ U4 CEID, A CENAME, L,m{ U4 VID } } }` | n=0 回全部 **292** 個 CEID(約 32 KB);未註冊者回 `{CEID,"",L,0}` |

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
| S2F29 | Equipment Constant Namelist Request | H→E | S2F30 | `L,n{ ECID }`(n=0=全部) | **本次新增 (2026-07-30)** |
| S2F30 | Equipment Constant Namelist Reply | E→H | — | `L,n{ L,6{ U4 ECID, A ECNAME, ECMIN, ECMAX, ECDEF, A ECUNITS } }` | 三個界限以該 EC 自身型別編碼;未宣告者回零長度項目 |
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
> (2) 清單中含**本機未註冊的 ECID**。本機只註冊 8 個 EC(§3.2:1007、1501、2758–2763),
> 若對未註冊的 ECID 回 0,等於承諾「該 EC 變更會回報」——本機沒有 EC 變更事件,永遠不會發生,故據實回 1
> (與 HT-90XX 對未註冊 ECID 回 1 的行為一致)。SECS log 會列出未註冊的 ECID 清單供雙方比對。

---

## 3. 資料字典 / Data Dictionary

> ⚠️ **CEID 編號(§3.3)自 2026-07-29 起與 HT-90XX 完全相同**,host 可直接沿用 HT-90XX 的 CEID 設定,
> 但須注意本機只有其中 53 個號碼有發射點(§3.3.1，其中 CEID 78 為條件發射、預設關閉),其餘 222 個雖已註冊卻永遠不會送出(§3.3.2)。
>
> ⚠️ **SVID 編號(§3.1)自 2026-08-04 起全部為 HT-90XX 家族編號** —— 本機自有的 66xxx 段已整段退役,
> 剩下的是共同段(3 / 4 / 9 / 1xxx / 2758–2763 / 37010)與 AMR / AGV 的 38xxx 段,共 **67 個**。
> host 可直接沿用 HT-90XX 的 SVID 設定;HT-90XX 有而本機無對應的號碼(tester 專屬,見 §6)以及已退役的
> 66xxx,HT-160S 的 S2F33 / S2F35 會**容忍**(見 §4),但那些欄位會回空 item。

### 3.1 狀態變數 / Status Variables (SVID) — S1F3/F4, S6F11

**共同段(對齊 HT-90XX)/ Common band (HT-90XX aligned):**

| SVID | 名稱 Name | 型別 | 說明 |
|---|---|---|---|
| 4 | GemControlState | U1 | GEM 控制狀態,**HT-90XX 值域**:1=Off-Line / 2=On-Line Local / 3=On-Line Remote。**2026-08-04 起為唯一公佈的控制狀態編號**——原並存的 **66002 已移除**;機內部仍以 GEM 標準值域 1/4/5 儲存,但不再對外公佈。變更時會送 CEID 141 + 91/92/93 |
| 9 | PreviousGemControlState | U1 | 上一次變更前的控制狀態,值域同 SVID 4 |
| 3 | GemClock | A | 設備當前時間,**SEMI E5 16 字元 `YYYYMMDDhhmmsscc`**(與 HT-90XX 的 `iTimeFormat=1` 同格式,也與本機 S2F18 回覆同格式)。與 1027 是**同一時刻的兩種格式**,不可互換;百分秒欄固定 `00` |
| 1001 | Machine Model | A | 機型名 = HT-160S |
| 1002 | Machine ID | A | 機台識別碼,取自 `General.ini [MachineIdentity] HandlerID`(維護頁的 **Handler ID** 欄)。HT-90XX 只有一個識別字串,HT-160S 有三個(機型 / Handler ID / 序號);**本號回 Handler ID,此為 2026-08-03 定案**(機型已由 1001 提供,序號另議)。⚠ **出廠預設為空字串**——這是現場輸入的欄位,未輸入前本值為空 `A[0]`,不是故障;請於機台交機時填入 |
| 1003 | Software Version | A | 軟體版本 |
| 1006 | Lot ID | A | **全部已登錄批號,以半角逗號 `,` 串接(不含空白)**。⚠ **2026-08-04 語意擴充**:本號原本只回單一作用中批號,現改為回登錄表中的**所有**批號,順序 = 登錄順序。單批時位元與舊版**完全相同**(例:`PROBE_LOT_A`),故只掛一批時把本值當單一 token 解析的 host 不受影響;三批時為 `PROBE_LOT_A,PROBE_LOT_B,PROBE_LOT_C`。`RemoveLot` 釋放出的空槽會被跳過,**不會出現開頭逗號或連續逗號**。登錄表為空時回主畫面批號欄的文字(既有 fallback,不變),結批(`CLEAR_LOT_INFO`)後回空字串。**已退役的 66030 / 66031 由本號導出**:首批 = 第一個逗號之前的字串;批數 = **1006 為空字串則 0,否則逗號數 + 1**(⚠ 必須先判空 —— 登錄表為空時本號回主畫面批號欄的文字,那是 **0 批**,直接算「逗號數 + 1」會誤報成 1 批)。⚠ **本號是貴端已保存的 `RPTID 502` 第 1 格**,故此值會直接出現在該既有事件報表內(item 數與 A 型別皆未變);若貴端把第 1 格當單一批號 token 解析,多批生產時請改為可處理多批的解析 |
| 1007 | Operator ID | A | 本班作業人員身分。主畫面右側設定區的「Operator ID」欄位,預設 `Operator`,存於 `system\General.ini` `[MachineIdentity] OperatorID`,**跨電源保存**。**同號的 ECID 1007 指向同一份資料且可寫**——貴端可用 `S2F15` 直接設定(見 §3.2),寫入後主畫面欄位即時同步顯示。**與本機的 HMI 角色登入無關**:那是本地權限控制(Operation / Supervisor / Engineer / Honprec),會因技術員中途登入而改變,語意不同,刻意不與本號綁定 |
| 1008 | Run Mode | A | 運轉模式,**HT-90XX 家族編號、家族值域**:`"0"`=Normal / `"1"`=RT / `"2"`=EQC。HT-160S 為 tray sorter,無 RT / EQC(重測)流程,故本值**恆為 `"0"`**。**2026-08-04 新增,取代已退役的 66000。** ⚠ **本號不是機台的 TASK 模式** —— Home / One Cycle / Clean Out / Tray Feed 這類任務模式自 2026-08-04 起**不再有任何 SV 公佈**;要追蹤任務請讀 **SVID 1011 Machine State** 的狀態文字,或訂閱對應的任務 CEID(§3.3:25 Home / 3 One Cycle / 4 Clean Out / 32 Tray Feed) |
| 1009 | Lot Start Time | A | 開批時刻,**HT-90XX 線上格式 `yyyy-mm-dd hh:nn:ss`(破折號)**;批與批之間為空字串。Lot Start 當下閂鎖(操作面板 Lot Start 按鈕與 SECS `LOTSTART` 兩條路徑皆寫入)、Lot End 清空,貨批進行中可隨時以 S1F3 回查,**不會隨系統時間變動**。未加入預設 Report 1;需在事件內帶出起測時間請以 S2F33 / S2F35 綁進貴端自訂報表。**跨電源保存**:工單隨機台重開並由操作員選擇「繼承」時,本值會以**原始開批時刻**還原(不是復電時刻);未繼承、工單建立於本功能之前、或中繼檔讀寫失敗則維持空字串——空字串是「批與批之間」的正式值,寧可空也不給一個看似合理但錯誤的時間。**2026-08-03 起本號為唯一的開批時刻**(原本同值的 66033 已移除) |
| 1011 | Machine State | A | 主畫面狀態文字 |
| 1021 | UPH | I4 | 每小時產出 |
| 1027 | System Time | A | 系統時間,`yyyy/mm/dd hh:nn:ss`(另見 SVID 3) |
| 1101 | Loader Count | I4 | 自 Loader 盤取出的 IC 數 |
| 1102 | Output Total Count | I4 | 已分選入 Bin 的 IC 數(**2026-08-04 起為唯一編號**,原同值的 66021 Total Sorted 已移除——兩號本來就綁同一個計數器 `MachineRun.iTotalSorted`,純屬去重) |
| 1103 | Auto1 Count | I4 | 放入 Auto1 出料盤的 IC 數(**2026-08-03 起為唯一編號**,原同值的 66022 已移除) |
| 1104 | Auto2 Count | I4 | 放入 Auto2 出料盤的 IC 數(原同值的 66023 已移除) |
| 1105 | Auto3 Count | I4 | 放入 Auto3 出料盤的 IC 數(原同值的 66024 已移除) |
| 1259 | Auto4 Count | I4 | 放入 Auto4 出料盤的 IC 數。⚠ **2026-08-03 起由本機自有段 66025 改為本號**——HT-90XX 家族中的 **HT-9011UC V3.33.899** 本身就是**六站 Auto** 機台,以 1259 / 1260 / 1261 命名 Auto4–6(其後 1262 起為 Fix7–12),故本機不再自創號碼,改與家族一致 |
| 1260 | Auto5 Count | I4 | 放入 Auto5 出料盤的 IC 數(原 66026) |
| 1261 | Auto6 Count | I4 | 放入 Auto6 出料盤的 IC 數(原 66027) |
| 1501 | Setup File | A | 目前 recipe / setup 檔名。**同號的 ECID 1501 指向同一份資料**(見 §3.2 說明) |
| 1517 | Start Mode | I4 | 0=Continuous Start / 1=Initial Start(HT-90XX 編號) |
| 1518 | Real/Dummy | I4 | 0=Dummy / 1=Tray Only / 2=Real |
| 2758 | Type 1 Tray Pitch X | F8 | 盤格 X 間距,**mm**。**同號的 ECID 2758 指向同一份資料** |
| 2759 | Type 1 Tray Pitch Y | F8 | 盤格 Y 間距,mm |
| 2760 | Type 1 Tray Start Position X | F8 | 盤格 X 起點,mm |
| 2761 | Type 1 Tray Start Position Y | F8 | 盤格 Y 起點,mm |
| 2762 | Type 1 Tray Division X | I4 | 盤格 X 數(無單位) |
| 2763 | Type 1 Tray Division Y | I4 | 盤格 Y 數(無單位) |
| 37010 | Enter Skip IC Count | I4 | 操作員在上一次 SKIP 時輸入的取出 IC 數 |

> **關於 1103–1105 / 1259–1261 的兩點必讀 / Two notes on the six Auto counts**
> 1. **HT-160S 有六個 Auto 站,六站全部使用 HT-90XX 家族編號**:Auto1–3 = **1103 / 1104 / 1105**(來源 HT-9046LS V3.32.810),Auto4–6 = **1259 / 1260 / 1261**(來源 HT-9011UC V3.33.899,該機同為六站 Auto 機台)。**六站完整分布 = 1103 / 1104 / 1105 + 1259 / 1260 / 1261。** 另請注意這與 **Fix1–3(1106–1108)** 是兩件不同的事:HT-9046LS 把第 4–6 個出料站叫 Fix1–3,要不要把 HT-160S 的 Auto4–6 對映到 Fix1–3 仍是**雙方商務決定**,本機**不單方面實作**,因此 host 報表中的 1106–1108 仍會維持空 item。若日後貴端確認 Fix1–3 即為第 4/5/6 輸出站,本機是**改用** 1106–1108 並同時下架 1259–1261,**不會兩組並存**(維持「一個資料一個編號」)。(2026-08-03 前 Auto1–3 另有一組同值的 66022–66024、Auto4–6 位於本機自有段 66025–66027,均已收斂到上述家族編號。)
> 2. **1102 與六站計數(1103–1105 / 1259–1261)的總和不保證相等。** ⚠ 並更正本文件先前的敘述:HT-90XX 的 1102 **並不是** 1103–1108 的和 —— 追查 HT-9046LS V3.32.810 原始碼,1102 綁 `RunInfo.iUnloadCount`,其唯一賦值是**整個 bin 陣列**的總和(`eTrayCount`=24 格:Auto1–3、Fix1–6、bulk box、Mag1–14),所以即使在 9045 上,RPTID 501 的八格也對不回 1102。HT-160S 的 1102 在**掃碼/配 Bin 階段**累加,1103–1105 / 1259–1261 在**放料階段**累加,且 1102 為 RAM 值(復電歸零)而 1101 / 1103–1105 / 1259–1261 **隨機台保存**(六站同一個 epoch,Lot Start 一併歸零)。請勿以「總和相符」作為對帳條件。

**HT-160S 自有高位段 66xxx —— 已於 2026-08-04 全數退役 / HT-160S-specific 66xxx band — fully retired 2026-08-04:**

> ⚠️ **依貴端裁定「HT-160S 只公佈 HT-90XX 家族編號」,本段十個 SVID 已全部不再註冊。**
> 下表**僅供 host 端比對舊設定用,不是可查詢的號碼**。這十個號碼**永久退役、永不遞補、永不改用他義**,
> 因此未更新的 host 綁定一定是「查不到值」而不是「查到別的值」——`S1F3` 回**空 list item `<L[0]>`**
> (位元 `01 00`),與「已註冊但值為空字串」的 `<A[0]>`(位元 `41 00`)可明確區分,故**不存在靜默誤讀的風險**
> (2026-08-04 實測:`66031` → `L,0`,`1006` → `A,0`,兩者並列可辨)。`S1F11` 名稱表也不再列出這十個號碼。
>
> Per the customer's ruling that HT-160S publishes HT-90XX family ids only, all ten ids below are **no longer
> registered** — the table is a migration aid, not a queryable list. They are **permanently retired and will
> never be reused**, so a stale host binding always surfaces as a detectable blank: `S1F3` answers an **empty
> list item `<L[0]>`** (bytes `01 00`), distinguishable from a registered-but-empty ASCII SV `<A[0]>`
> (bytes `41 00`). They are also gone from the `S1F11` namelist.

| 已退役 SVID / Retired | 原名稱 Former name | 替代方案 / Replacement |
|---|---|---|
| 66000 | Run Mode | 改用家族編號 **1008 Run Mode**(A,值域 `0:Normal; 1:RT; 2:EQC`,本機恆為 `"0"`)。⚠ 機台的 **TASK** 模式(Home / One Cycle / Clean Out / Tray Feed)**已無任何 SV 公佈**——請讀 **1011 Machine State** 的狀態文字,或訂閱對應的任務 CEID(§3.3) |
| 66001 | System Running | **無替代**。運轉 / 停止請由 **1011 Machine State** 的狀態文字判讀,或訂閱 Start / Pause 事件。⚠ **Start 有兩個號碼,兩個都要訂**:**CEID 1** Start Pressed 只在**機內無 IC** 時發;**CEID 76** Start Pressed HasIC 才是**機內有 IC** 時的 Start(暫停後續跑、警報解除後續跑都走 76 —— 產線上這才是常態)。停止側為 **CEID 2** Pause Pressed。只訂 CEID 1 會漏掉絕大多數的重新起動。HT-90XX 亦無此類 SV |
| 66002 | Control State | 改用 **SVID 4 GemControlState**(搭配 **9 PreviousGemControlState**),值域為 HT-90XX 的 1=Off-Line / 2=On-Line Local / 3=On-Line Remote(§1)。機內部仍以 GEM 標準值域 1/4/5 儲存,但不再對外公佈 |
| 66010 | Alarm Active | **無替代**。警報一律走 **S5F1** 串流,目錄查 **S5F5/F6**,與 HT-90XX 完全一致(§3.5) |
| 66011 | Alarm Code | **無替代**,同上——警報碼即 S5F1 的 **ALID** |
| 66020 | Total IC | 改讀 **1101 Loader Count**(自 Loader 盤取出的 IC 數)與 / 或 **1102 Output Total Count**(已放入 Bin 的 IC 數);兩者語意見 §3.1 的計數說明 |
| 66021 | Total Sorted | 改讀 **1102 Output Total Count**——兩號本來就綁**同一個**計數器(`MachineRun.iTotalSorted`),純屬去重,值不會有任何差異 |
| 66030 | Active Lot Count | 由新語意的 **1006 Lot ID** 導出,但**必須先判空**:**1006 為空字串 → 批數 0;否則批數 = 逗號數 + 1**。⚠ 登錄表為空時 1006 回主畫面批號欄的文字(通常是空字串),機上其實是 **0 批** —— 直接套「逗號數 + 1」會把 0 批誤報成 1 批(已退役的 66030 在該狀態回 0)。⚠ 另請注意:批號欄若有操作員打的字但尚未開批,1006 會回那串文字,這同樣是 **0 批**,不能只靠 1006 判定機上有批 —— 需要嚴格的批數請以`LOTSTART` / `CLEAR_LOT_INFO` 的事件流追蹤 |
| 66031 | Current Lot ID | 由新語意的 **1006 Lot ID** 導出:**首批 = 第一個逗號之前的字串** |
| 66032 | Sort Mode | **無替代,且這是本次已知並接受的代價。** HT-90XX 家族沒有 sort-mode SVID(唯一近似的 **35530**「[I27] Manual sort mode」是 HT-9045/46 的 BOOLEAN 選項,語意不符,不可挪用)。host 仍可用 **`S2F41 LOTSTART` 的 `SORTMODE` pair 設定**分選模式(§3.4),但**無法再以 `S1F3` 回讀**。若貴端需要回讀,請指定一個家族編號給我方掛上 |

> ⚠️ **本段 2026-08-03 已先移除的號碼 / Retired earlier, on 2026-08-03**
> **66022 / 66023 / 66024**(Auto1–3 Count)與 **66033**(Lot Start Time)已**不再註冊**。這四個號碼
> 與 HT-90XX 的 **1103 / 1104 / 1105** 及 **1009** 是同一份資料的第二種編號,依「相同功能使用同一
> 編號」的原則收斂到 HT-90XX 的號碼。**請改綁 1103 / 1104 / 1105 / 1009。**
> 同日 **66025 / 66026 / 66027**(Auto4–6 Count)改用 HT-9011UC V3.33.899 的 **1259 / 1260 / 1261**。
> 這些號碼**保留空號、不再挪作他用**——若貴端設定未更新,S1F3 會回「未知 SVID」(空 item),
> 不會回到別的資料上,故不存在靜默誤讀的風險。
> 【**2026-08-04**:加上上表的十個號碼,**66xxx 段至此整段為空,HT-160S 不再公佈任何 66xxx SVID。**】

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
| P7 Auto4 | **38199** | 38237 | 38240 | 38243 |
| P8 Auto5 | **38200** | 38238 | 38241 | 38244 |
| P9 Auto6 | **38201** | 38239 | 38242 | 38245 |

Carrier ID = A;Tray/Device Count = I4;Bin Setting = A。

> ⚠ **2026-08-03 變更(breaking change):Auto4–6 的 Carrier ID 由 38208 / 38209 / 38210 改為 38199 / 38200 / 38201。**
> 依「Auto1–3 遵循 HT-9046LS V3.32.810、Auto4–6 參照 HT-9011UC V3.33.899」的原則:810 只定義三個出料口盤號
> (38205 / 38206 / 38207,故 P4–P6 不變),而六站機台 HT-9011UC V3.33.899 把另外三個命名為
> `Output 4/5/6 Tray ID` = **38199 / 38200 / 38201**,因此 P7–P9 改用該組家族編號,不再使用本機自創的
> 38208–38210。**若貴端已綁定 38208–38210,請改綁 38199–38201**;這三個舊號碼即日起為空號,永不改用他義。
> 註:HT-9011UC 把整個盤號家族(含其自身的 38205–38207)宣告在 **ECID** 命名空間;本機**維持 SVID**,
> 因為 Auto1–3 是照 810 的 SVID 定義,六個口必須在同一個命名空間回答。
> Tray / Device / Bin Setting 三欄的 Auto4–6(38237–38245)**維持本機延伸號**:810 的 AMR 段止於 38236、
> HT-9011UC 整段沒有 AMR SVID 家族,兩棵樹皆無可對齊的號碼(2026-08-03 逐檔查證)。

### 3.2 設備常數 / Equipment Constants (ECID) — S2F13/F14, S2F15/F16

| ECID | 名稱 Name | 型別 | 單位 | 可寫? | 說明 |
|---|---|---|---|---|---|
| 1007 | Operator ID | A | — | **可寫(運轉中亦可)** | 本班作業人員身分。**同號的 SVID 1007 指向同一份資料**(見 §3.1)。空字串一律拒絕(EAC=3),確保本欄永不回空 item |
| 1501 | Recipe Name | A | — | 唯讀 | 目前配方(Setup File)名 |
| 2758 | Tray X Pitch | FT8 | mm | 可寫(idle) | 對齊 9045 Type1 Pitch X |
| 2759 | Tray Y Pitch | FT8 | mm | 可寫(idle) | 9045 Type1 Pitch Y |
| 2760 | Tray X Start | FT8 | mm | 可寫(idle) | 9045 Type1 Start X |
| 2761 | Tray Y Start | FT8 | mm | 可寫(idle) | 9045 Type1 Start Y |
| 2762 | Tray X Division | I4 | — | 可寫(idle) | 每列格數 X |
| 2763 | Tray Y Division | I4 | — | 可寫(idle) | 每欄格數 Y |

> S2F15 寫入原則上僅在機台 idle(非運轉、機內無 IC)時接受(EAC=2 否則);tray-geometry 段適用此閘,Recipe(1501)唯讀。

> **2026-08-03:ECID 1007 Operator ID 為 idle 閘的唯一例外 / The one exception to the idle gate.**
> 1007 **在運轉中也接受寫入**,不會回 EAC=2。理由:idle 閘存在的目的是防止運轉中變更 tray 幾何而位移取放
> 座標;1007 只是識別字串,無任何機構後果,而貴端 host 正是在**運轉當下**寫它——2026-06-08 現場 log 的
> 24 筆 S2F15 中,有 4 筆寫 `ECID 1007 = "AGV"`。若比照設閘,那 4 筆會全部被拒。
> 寫入成功後:回 `EAC=0`、值即時顯示於主畫面欄位、寫入 `General.ini` 保存、並發 **CEID 48 Change EC**。
> 空字串一律回 `EAC=3` 不予寫入。

> **2026-08-03:同號的 SVID 已可讀 / Same ids are now readable as SVIDs too.** SECS-II 的 SVID 與 ECID
> 是兩個獨立命名空間。本機先前只把 1501 與 2758–2763 註冊在 **EC** 命名空間,而 host 是以 **SVID**
> 路徑(S1F3 / S6F11 報表)讀它們,因此那些槽位一直回空 item。本版已在 SV 命名空間補註冊**同號、同型別、
> 指向同一份資料**的七個號碼(見 §3.1),兩條路徑的值恆等,不會出現不一致。

### 3.3 事件 / Collection Events (CEID) — S6F11

> **2026-07-29 重大變更 / Breaking change**
>
> HT-160S 的 CEID 字典自本版起為 **HT-90XX 字典的逐字複本**:號碼 1–292 全數註冊,別名(alias)與 HT-90XX
> 逐字相同。**舊版(2026-07-27 及之前)的 CEID 1–31 語意已全部作廢**,請 host 端依本節重新設定。
> 遷移對照見本節末「§3.3.5 舊字典 → 新字典對照」。
>
> As of this revision the HT-160S CEID dictionary is a **verbatim copy of the HT-90XX dictionary**: ids
> 1–292 are all registered and their aliases match HT-90XX character for character. **The CEID 1–31
> meanings published in the 2026-07-27 and earlier revisions are withdrawn.** Hosts must re-provision
> per this section; see §3.3.5 for the old-to-new mapping.

> **2026-08-03 補充:字典已對貴端現場機台逐筆核對過 / Dictionary verified against your own equipment**
>
> 本機的 CEID 字典原先移植自 HT-90XX 原始碼(HT9046LS V3.32.810),涵蓋 **1–275**。2026-08-03 取得
> 貴端 **HT-9046 現場機台自己存下來的 `EventReport_CEID.def`**(2026-06-25)後逐筆比對,結果:
>
> - **1–275 與本機完全相同,0 筆別名不符** —— 證實本機的 CEID 編號與語意就是貴端 host 當初設定所依據的那一套。
> - 貴端機台另有 **276–292 共 17 個號碼**是本機先前沒有的。本版已全部補上註冊(名稱逐字照抄該檔),
>   使 `S1F23` / `S1F24` 的字典與貴端機台完全一致。詳見 §3.3.3。
> - 併此更正:CEID **272 / 273 / 274 / 275** 先前在 `S1F24` 回的別名是本機自用的
>   `AGVSupplement` / `AGVLDUnLDStatus` / `AGVLDUnLDFinish` / `AGVLdID`,與貴端機台的
>   `AMR Supplement` / `AMR LDUnLD Status` / `AMR LDUnLD Finish` / `AMR LD ID` 不符;
>   本版已改為與貴端機台一致。**號碼與行為完全未變,只有 `S1F24` 回覆的名稱字串改了。**
>   若貴端是以 CEID 號碼設定(一般作法)則不受影響;以名稱比對者請以本版為準。
>
> ⚠ **給雙方工程人員的提醒**:HT-90XX 另有一條 **HT9011UC(V3.33.9xx)** 分支,它把
> **CEID 217–271 整段重新編號**(該分支的 217 是 `LoadPortStatusChanged`,貴端機台與本機是 `Reserved_06`),
> 並把上述 sorter 事件放在 275–288 而非 276–292。**貴端機台用的不是那一套**。日後任何一方要更新字典,
> 請以貴端機台的 `.def` 或本節為準,不要從 HT9011UC 分支同步,否則 55 個號碼會靜默變義。

#### 3.3.1 本機實際會發射的事件 / Events this equipment actually sends

以下 **56** 個號碼是 HT-160S 有發射點的事件。號碼與名稱皆為 HT-90XX 原文。
其中 **CEID 78 為條件發射**，出廠預設關閉，其餘 55 個不受設定影響。
(2026-08-03 由 53 增為 57:新增控制狀態變更事件 **141** 與 **91 / 92 / 93**,見表末。
2026-08-04 由 57 減為 56:**CEID 17 Enter Tool Page** 的發射點是主畫面工具列的 Tools 鍵,
該鍵只會開出一片空白視窗,已連同按鈕一併移除,17 改列 §3.3.2。)

The following **56** ids have a real emit site on HT-160S. Ids and names are HT-90XX's own.
**CEID 78 is conditional** (off by default); the other 55 are not gated by any setting.
(53 -> 57 on 2026-08-03: control-state change events **141** and **91 / 92 / 93** were added.
57 -> 56 on 2026-08-04: the only emit site for **CEID 17 Enter Tool Page** was the main-screen
Tools button, which opened an empty form and has been removed with it; 17 moved to §3.3.2.)

| CEID | 名稱 / Name | HT-160S 發射時機 / When it is sent |
|---|---|---|
| 1 | Start Pressed | Start 且機內**無** IC;警報畫面的面板 START 鍵 |
| 2 | Pause Pressed | Pause(畫面鍵或面板鍵) |
| 3 | OneCycle Pressed | One Cycle 受理 |
| 4 | CleanOut Pressed | Clean Out 受理 |
| 5 | ClearCount Pressed | Clear Count,**操作員確認對話框之後** |
| 6 | Lot Start | 手動 Lot Start |
| 8 | Lot End | 手動 Lot End / Clean Out 完成後自動 Lot End |
| 9 | Switch Real Dummy Mode | Real / Dummy 切換 |
| 14 | Switch StartMode | Start Mode(Initial / Continue)切換 |
| 15 | Switch Setup File | 工作檔(recipe)切換 |
| 16 | Switch UserLevel | 使用者權限層級切換 |
| 18 | Enter Maintenance Page | 進入 Maintenance 頁 |
| 19 | Enter Offset Page | 進入 Offset 頁 |
| 20 | Enter Speed Page | 進入 Speed 頁 |
| 21 | Enter IO Page | 進入 I/O 頁,**權限檢查通過後** |
| 22 | Enter Message Page | 進入 Message 頁 |
| 24 | Exit Pressed | 按下 Exit(於連線拆除前送出) |
| 25 | Home Pressed | Home 受理 |
| 27 | Change Machine State | **機台狀態文字變更**(RUNNING / PAUSE / EMG / LOCK / SAFE DOOR / AIR / MOTOR OFF / Clean Out / Tray Feed / One Cycle 等) |
| 28 | Retry Pressed | 面板或畫面 Retry |
| 29 | Skip Pressed | 面板或畫面 Skip |
| 30 | Alarm Reset Pressed | 面板或畫面 Alarm Reset |
| 31 | Tray End Pressed | 面板 **TRAY END** 鍵 |
| 32 | Tray Feed Pressed | 面板 **TRAY FEED** 鍵 |
| 35 / 36 / 37 | Auto1 / Auto2 / Auto3 Full | Auto 車滿(需 `bUseAMR=1`) |
| 41 | One Cycle Finish | One Cycle 走完 |
| 42 | Clean Out Finish | Clean Out 排空完成(送在 Lot End 之前) |
| 47 | Change Handler Speed | 離開 Speed 頁**且百分比確實有變更** |
| 48 | Change EC | S2F15/F16 **實際寫入成功**(EAC=0 且有值變更) |
| 53 | UPH Record Start | 某 Auto 站進入 SORTING,一筆 UPH 記錄視窗開啟 |
| 54 | UPH Record End | 該盤 UPH 記錄完成並寫入報表清單 |
| 66 | Load Tray Finish | Loader 盤到位確認 + 盤身分產生 |
| 73 | Mymessbox OK | 訊息框關閉(Yes / No / Pause / Esc 任一路徑) |
| 76 | Start Pressed HasIC | Start 且機內**有** IC |
| 78 | Jam Skip IC Count | 條件發射：以 SKIP 解除警報且操作員輸入取出顆數（需 General.ini `[SECS] AskSkipICCount=1`，預設關閉） |
| 123 | Safe Door On Off | 安全門開↔關**任一方向**邊緣(運轉中與停機中皆送) |
| 124 | Save Recipe | Setup 頁 Save 或 Save As 成功 |
| 136 / 137 / 138 | Auto1 / Auto2 / Auto3 Unloading tray | Auto 出盤 |
| 145 / 146 / 147 | Auto4 / Auto5 / Auto6 Unloading tray | Auto 出盤 |
| 148 / 149 / 150 | Auto4 / Auto5 / Auto6 Full | Auto 車滿(需 `bUseAMR=1`) |
| **141** | GEM Control State Change | **控制狀態每次變更即送**(2026-08-03 新增)。狀態值見 SVID 4 / 9 |
| **91 / 92 / 93** | SECS/GEM Offline / Online / Online Remote | **同一次變更會在 141 之後再送這三者中對應新狀態的一個**(2026-08-03 新增)。與 HT-90XX 同順序 |
| 272 / 273 / 274 / 275 | AMR Supplement / LDUnLD Status / LDUnLD Finish / LD ID | 見 §3.3.4 |

**頻率提醒 / Frequency notes**

- **CEID 27** 為本機流量最高的事件(機台狀態每次變更即送),與 HT-90XX 同一性質。
- **CEID 53 / 54**:HT-160S 有 **6 個 Auto 站**,故每站各自開關 UPH 視窗,頻率為 HT-90XX(單一 Loader)的數倍。
- **CEID 73** 於**每一次**訊息框關閉皆送(HT-90XX 僅對 SECS 來源的警報框送)。

#### 3.3.2 已註冊但本機不會發射的事件 / Registered but never sent

號碼 1–292 之中,上表以外的 **236** 個號碼**皆已註冊**(有別名、可被 `S2F35` 連結、可被 `S2F37` 啟用/停用),
但 HT-160S **沒有對應機構,永遠不會送出**。這是為了讓 host 的字典與 HT-90XX 完全一致而刻意保留的。

Every id in 1–292 not listed above — **236** of them — **is registered** (it has an alias, can be linked by
`S2F35`, can be enabled/disabled by `S2F37`) but HT-160S **has no such mechanism and will never send it**.
They are kept registered on purpose so the host dictionary matches HT-90XX exactly.

主要類別 / Main categories:

| 類別 / Category | 號碼 / Ids | 為何本機沒有 / Why not on HT-160S |
|---|---|---|
| 測試機專屬(tester / ART / site / 溫控 / 清針 / OTD) | 10, 13, 26, 34, 44, 45, 46, 50, 51, 52, 55–63, 67–69, 71, 72, 81–88, 90, 111, 119–122, 125, 126, 128, 129, 250, 251 | HT-160S 為 sorter,無測試接點、無 site、無溫控、無 ART 重測流程 |
| E87 Cassette / OHT 載具流程 | 94–109, 116–118, 131–135 | HT-160S 的 AMR 交握走 272–275 自有模型,不走 E87 cassette |
| Fix 固定站滿盤 | 38, 39, 40, 151, 152, 153 | HT-160S 無 Fix 固定站 |
| 站別細粒度事件(Loader / Empty / Color / Auto 逐站缺盤、滿盤、剩一盤、放盤、上蓋、就緒卸盤、完成) | 154–165, 166–211 | HT-90XX 為 ASEKH_K1/K3 專案新增,**HT-90XX 韌體自身亦僅宣告未發射**;HT-160S 以粗粒度的 272 / 274 / 136–147 表達同一資訊 |
| 節能子系統 | 212, 213 | HT-160S 無節能子系統(`ENERGY_SAVING` 遠端命令刻意回 HCACK=2) |
| 保留位 / Reserved | 214–249, 252–271 | HT-90XX 韌體佔位,無語意 |
| **貴端機台的延伸段(2026-08-03 新增)** | **276–292** | 見下方「§3.3.2.1 CEID 276–292」。其中數項 HT-160S 機構上做得到,但目前**尚未接上發射點**,需雙方確認酬載內容後另行排入 |
| HT-90XX 亦未發射(僅宣告) | 43, 91, 92, 93, 114, 115, 130, 141, 209 | HT-90XX 韌體同樣只宣告不發射,無可對照的觸發時機 |
| 觸發條件本機不存在 | 7, 11, 12, 17, 23, 33, 49, 64, 65, 77, 78, 89, 110, 112, 113, 127, 139, 140, 142, 143, 144, 74, 75, 79 | 詳見下表 |

**「觸發條件本機不存在」逐條說明 / Triggers that do not exist on HT-160S**

| CEID | HT-90XX 名稱 | 本機為何無法送 |
|---|---|---|
| 7 | Lot | HT-90XX 語意未定義,無對應動作 |
| 11 / 12 | Switch Production Mode / Switch Engineer Mode | HT-160S 無「生產/調機」與「一般/工程」模式切換(僅有 User Level = CEID 16) |
| 17 | Enter Tool Page | HT-160S 無 Tool 頁。工具列原有一顆 Tools 鍵,但它開出的是一片空白視窗(HT-90XX 該頁的權限設定與警報碼產生功能在本機分屬他處),已於 2026-08-04 連同按鈕移除 |
| 23 | Enter Debug Page | HT-160S 無 Debug 頁 |
| 33 | Reset Pressed | 面板僅有 ALARM RESET(= CEID 30),無獨立 Reset 鍵 |
| 49 | Tray Feed Finish | HT-160S 尚未實作 Tray Feed 結束判定 |
| 64 / 65 | DownLoad Recipe by FTP OK / NG | HT-160S 的 FTP 為**生產資料上傳**,不下載 recipe |
| 74 / 75 / 79 / 142 / 143 / 144 | (HT-90XX 未具名) | HT-90XX 字典本身即無別名 |
| 77 | Read Current ESD Data | HT-160S 無 ESD 子系統 |
| 78 | Jam Skip IC Count | HT-90XX 於操作員**輸入 Skip IC 數量**後送出;HT-160S 的 SKIP 無數量輸入流程 |
| 89 | Pre Alarm Message | HT-90XX 為特定客戶碼 + 8 unloader 的良率監控掛勾,HT-160S 無此機制 |
| 110 | Clean Out Tray Feed Finish | HT-160S 的 Clean Out 與 Tray Feed 為獨立模式,無合併事件 |
| 112 / 113 | MR Run Mode Change / Access Mode Change | HT-160S 無 MR mode、無 access mode |
| 127 | Back To Normal | HT-90XX 語意未定義 |
| 139 | Visual sort Lot start | HT-90XX 為特定客戶視覺分選專案 |
| 140 | Prepare Load Tray | HT-90XX 為 Tray Map Throw IC 功能前置,HT-160S 無此功能 |

> **重要**:`S2F35` 連結一個「本機不會發射」的 CEID **會被接受(DRACK=0x00)**,但該事件永遠不會送出。
> 這與「已註冊且會發射」的號碼不同。請以 §3.3.1 的表為訂閱依據。
>
> Linking a never-sent CEID via `S2F35` **is accepted (DRACK=0x00)** but no event will ever arrive.
> Subscribe according to the §3.3.1 table.

##### 3.3.2.1 CEID 276–292:貴端機台的延伸段 / The 276–292 block from your equipment

這 17 個號碼**不是**來自 HT-90XX 原始碼,而是逐字取自貴端 **HT-9046 現場機台自己存下的
`EventReport_CEID.def`**(2026-06-25)。同一份檔案的 1–275 與本機**逐筆相同、0 筆不符**,
因此本機於 2026-08-03 把這 17 個一併註冊,使兩機的字典完全一致。

These 17 ids do **not** come from the HT-90XX source tree. They were copied verbatim from the
`EventReport_CEID.def` your own HT-9046 had persisted (2026-06-25); the 1–275 block of that same
file matches HT-160S with **zero** mismatches, so the 17 were registered on 2026-08-03 to make the
two dictionaries identical.

| CEID | 名稱 / Name | HT-160S 目前狀態 / Status on HT-160S |
|---|---|---|
| 276 | `Loader_Buffer_NoTray` | 已註冊,未發射。機構上對應「Loader 緩衝區無盤」,本機有此感測 |
| 277–282 | `OutputPort1BinCode` … `OutputPort6BinCode` | 已註冊,未發射。**對應本機六個 Auto 出料站的 Bin code**,機構上做得到 |
| 283 | `MaterialModeChange` | 已註冊,未發射 |
| 284 | `PortStateUpdated` | 已註冊,未發射 |
| 285 | `UnloaderTrayIDReadOK` | 已註冊,未發射。本機有出料盤 2D 讀取 |
| 286 | `UnloaderTrayIDReadFail` | 已註冊,未發射 |
| 287 | `LoaderTrayIDReadFail` | 已註冊,未發射。本機有 Loader 側身分盤 2D 讀取 |
| 288 | `MaximumOutputPortReport` | 已註冊,未發射 |
| 289 | `RunCheckRequest` | 已註冊,未發射。本機無「測前查檢」流程(見 §5 功能表) |
| 290 | `AGVLDUnLDFinish` | 已註冊,未發射。⚠ 與 **274** 在貴端機台上是同一個名稱,本機的 AMR 交握走 **272–275**(見 §3.3.4) |
| 291 | `AGVLdID` | 已註冊,未發射。⚠ 與 **275** 同名,同上 |
| 292 | `DoSecsGemIndexFail` | 已註冊,未發射。⚠ 與 **251** 同概念(SECS/GEM 連續失敗) |

> **註**:上表中「機構上做得到」的幾項(277–282 出料站 Bin code、285/286/287 Tray ID 讀取結果、
> 276 Loader 緩衝無盤),本機**目前只有註冊、沒有發射點**。要真的送出這些事件,需先與貴端確認
> **每個事件要攜帶哪些 SVID**(例如 Bin code 事件要不要帶站別、盤號、Bin 值),再排入實作。
> 若貴端需要,請提供貴端 HT-9046 上這幾個事件的實際 S6F11 範例,本機即可比照。
>
> The items above marked as mechanically available are **registered only** — there is no emit site yet.
> Turning them on needs an agreed payload (which SVIDs each event carries) first. Sending us a real
> S6F11 sample from your HT-9046 for those ids is the fastest way to settle it.

#### 3.3.3 報表連結預設值 / Default report linkage

**自 2026-08-04 起,HT-160S 的預設 Report 1 = `{1027 System Time}` 一格,與 HT-90XX 逐字相同**——
兩機的出廠預設自此**一致**,不再有差異需要 host 特別處理(此為與前版**相反**的結論):

- HT-90XX 的 `AddReprot` 定義 `ReportIDContent[] = {1027}`;貴端 HT-9046 現場機台自存的
  `EventReport_ReportID.def` 也**整份只有一列**:`RPTID 1, Type 1, SVID 1027`。**HT-160S 的 Report 1
  現在與這兩者逐字相同。** 前版的 Report 1 帶 13 個 SV(1001 / 1003 / 1021 / 1027 + 九個已退役的 66xxx),
  該快照**不再存在**。
- **292 個 CEID 中有 288 個預設連結 Report 1**,因此絕大多數 `S6F11` 的本體現在**只有一個 item**(時戳):
  `L,3{ DATAID, CEID, L,1{ L,2{ RPTID 1, L,1{ A 19 字元時戳 } } } }`。
  2026-08-04 實測 CEID 27 / 141 / 93 / 6 / 8 皆為此形狀。
- **例外只有四個**:CEID **272 → Report 2 + 6**、**273 → Report 3**、**274 → Report 4 + 6**、
  **275 → Report 7**(AMR 交握,見 §3.3.4)。**Report 2–7(AMR 位圖、各站 tray / device count、身分盤 2D)
  完全未變。**
- `S1F23` / `S1F24` Event Namelist 走同一條 CEID→RPTID→SVID 鏈,故那 288 列的 VID 清單由 **13 個縮為 1 個**。
- 一旦 host 下了 `S2F33` + `S2F35`,連結即被覆寫,事件即攜帶 host 自訂的內容。

As of 2026-08-04 HT-160S's default **Report 1 is exactly `{1027 System Time}`** — byte-identical to HT-90XX
(whose `AddReprot` defines `ReportIDContent[]={1027}`) and to your own HT-9046's persisted
`EventReport_ReportID.def`. The two machines' factory defaults are now **identical**, which **inverts** the
comparison this section used to make: the previous 13-SV snapshot (1001 / 1003 / 1021 / 1027 plus nine retired
66xxx ids) is gone. **288 of the 292 CEIDs** link to Report 1, so almost every `S6F11` now carries **one item**
— `L,3{ DATAID, CEID, L,1{ L,2{ RPTID 1, L,1{ A 19-char timestamp } } } }` — and the matching 288 `S1F24`
namelist rows shrink from 13 VIDs to 1. The only exceptions are CEID **272 → reports 2 + 6**, **273 → 3**,
**274 → 4 + 6** and **275 → 7**; **reports 2–7 are completely unchanged**. Once the host issues
`S2F33` + `S2F35` the linkage is overwritten.

> ⚠️ **本次唯一的真實退化,請務必知會現場 / The one real regression — state this plainly**
> **`S2F35` 的事件→報表連結只在 session 內有效,不跨電源保存**,因此**每一次復電後,那 288 個 CEID 都會
> 退回預設的 Report 1**。2026-08-04 之前,這個退場報表還帶著 13 個機況 SV(批號、機況、UPH 等);
> **現在它只有一個時戳**。也就是說:復電後在 host 重跑 `S2F33` + `S2F35` 之前,所有事件都只有時間欄位,
> 拿不到批號與機況。**請把 `S2F33` + `S2F35` 固定放進 host 的每一次連線 / 復線流程**
> (§4 的標準上線序列本來就包含這兩步)。報表**定義**本身會寫入 `system\EventReportDef.ini`,
> 但**連結不會**——兩者請分開看待。
>
> The `S2F35` event→report **links are session-only and are never persisted**, so after **every power cycle**
> all 288 ids fall back to Report 1. Before this change that fallback window still carried 13 machine-context
> SVs; **now it carries only the timestamp**. Re-run `S2F33` + `S2F35` on every (re)connect — report
> *definitions* are written to `system\EventReportDef.ini`, the *links* are not.

> ⚠️ **另一項請一併知會:`SVID 1006` 是貴端已保存的 `RPTID 502` 第 1 格。**
> 1006 自 2026-08-04 起回「**全部**已登錄批號、以半角逗號串接」(§3.1),故**該既有事件報表第 1 格的值語意
> 隨之改變**。**item 數與 A 型別皆未變**,封包形狀不動;但多批生產時該格會是 `LOT_A,LOT_B` 這樣的字串,
> 若貴端把第 1 格當單一批號 token 解析,請改為可處理多批的解析。單批時位元與舊版完全相同。
>
> `SVID 1006` is **slot 1 of your persisted `RPTID 502`**, so the comma-joined value now appears **inside that
> existing event report**. Item count and A-type are unchanged; a host that parses slot 1 as a single lot token
> must be updated for the multi-lot case.

> **Auto Full(35/36/37/148/149/150)與 Auto Unloading tray(136–138/145–147)連結 Report 1**,與其他事件一致。
> 更早的版本這六個出盤事件為**空報表** `L,3{ DATAID, CEID, L,0 }`;現已與其他事件一致註冊在 Report 1 上。
> 【**2026-08-04 更正形狀**:上面「連結 Report 1」仍然成立,**但 Report 1 已縮為一格**,故這些事件現在的本體是
> `L,3{ DATAID, CEID, L,1{ L,2{ RPTID 1, L,1{ A 時戳 } } } }` —— **既不是空報表,也不是舊版的 13 格快照,
> 而是單一時戳**。Host 若已針對空報表**或** 13 格快照撰寫解析,請一併更新。】
>
> The Auto Full and Auto Unloading-tray ids are linked to Report 1 — they no longer ship an empty report.
> **[Corrected 2026-08-04: the linkage claim still holds, but Report 1 now holds a single item, so their body is
> one timestamp — neither an empty list nor the old 13-SV snapshot.]**

> **2026-08-03 佐證:上述「HT-90XX 出廠預設幾乎不帶資料」已由貴端機台的檔案證實。**
> 貴端 HT-9046 存下的 `EventReport_ReportID.def` **整份只有一列**:`RPTID 1, Type 1, SVID 1027`;
> 同機的 `EventReport_CEID.def` 則把 **292 個 CEID 全部 Enable、且 1:1 連到同號的 RPTID**。
> 也就是說該機除了 CEID 1 之外,**每個事件預設都送空清單**,要有資料一律得靠 host 自己下 `S2F33` / `S2F35`。
> 這正是貴端 2026-08-03 來信第 2 點「CEID 27 有發、但批號與機況沒資料」的同一個成因 ——
> 當時的差別在於 HT-160S 預設就掛 Report 1(13 個機況 SV),所以未設定的 host 在本機仍讀得到基本資料。
> 【**2026-08-04 更新:上一句所述的差別已消失。** 本機 Report 1 已縮為 `{1027}`,兩機在「host 尚未設定」
> 狀態下的行為自此**完全相同** —— 未設定的 host 在本機同樣只拿到一個時戳。】
>
> Confirmed from your own equipment on 2026-08-03: its `EventReport_ReportID.def` contains a single line
> (`RPTID 1, Type 1, SVID 1027`) while its `EventReport_CEID.def` enables all 292 ids linked 1:1 to a
> same-numbered report — so on the HT-9046 every event but CEID 1 ships an empty list until the host
> provisions. At the time HT-160S attached a 13-SV Report 1 by default, so an un-provisioned host still got
> data. **[Updated 2026-08-04: that difference is gone — HT-160S's Report 1 is now `{1027}` too, so an
> un-provisioned host gets a bare timestamp on either machine.]**

#### 3.3.4 AMR / AGV 材料交握事件 / AMR material handoff

（全部 DataID=1,對齊 HT9045 / all DataID=1, aligned to HT9045)

| CEID | 名稱 | 攜帶報表 | 說明 |
|---|---|---|---|
| 272 | AMR Supplement | Report 2(SVID 38219 bitmap)**+ Report 6** | 要料;**同時帶各站 Tray Count + Device Count** |
| 273 | AMR LDUnLD Status | Report 3(SVID 38220 bitmap) | 交握中(尚未計數,不帶 count) |
| 274 | AMR LDUnLD Finish | Report 4(SVID 38221 bitmap)**+ Report 6** | 上/下料完成;**帶收尾 Tray Count + Device Count** |
| 275 | AMR LD ID | Report 7(SVID 38204) | Color 身分 Tray 2D(見下) |

> **名稱更正(2026-08-03)**:上表四列的名稱是 `S1F24` 會回的 **CENAME**。本機先前回的是自用字串
> `AGVSupplement` / `AGVLDUnLDStatus` / `AGVLDUnLDFinish` / `AGVLdID`,與貴端機台的
> `AMR Supplement` / `AMR LDUnLD Status` / `AMR LDUnLD Finish` / `AMR LD ID` 不一致;本版已對齊。
> **CEID 號碼、攜帶報表、發射時機全部未變**,只有 `S1F24` 回覆的名稱字串改了。

> P-bitmap 站別對應:P1=Loader, P2=Empty, P3=Color, P4-P9=Auto1-6。

**下料 Tray/Device Count 上傳(對齊 HT9045 iAMRTrayCount / iAMRDeviceCount)/ Unload count upload:**

- CEID **272(要車)與 274(完成)** 皆附 **Report 6**;Report 6 = 9 站 **Tray Count**(SVID 38222–38227 / 38237–38239)接著 9 站 **Device Count**(SVID 38228–38233 / 38240–38242),與 HT9045 的 `iAMRTrayCount[]` / `iAMRDeviceCount[]` **同編號同語意**。
- 下料(Auto 站)時兩值皆填真值:`TrayCount = 該車盤數 (Car->iTrayCount)`;`Device Count = 該 Auto 車的 IC 累計`(卸料時逐盤 `+= Tray.CountIC()` 累加)。
- 上料方向(Loader/Empty/Color)僅交換盤數,Device Count 保持 0(與上下料契約一致:上料交換盤數、下料才給 IC 件數 — HT9045/HT160 皆然)。

**Color 身分 Tray 2D 上傳(CEID 275 AMR LD ID,對齊 HT9045)/ Color identity-tray 2D upload:**

- 由 Color CCD 於 Loader 回收進料點掃描身分 Tray 2D,掃描完成即以 S6F11 **CEID 275** 上傳。
- 值寫入 **SVID 38204**(Color P3 carrier ID),經 Report 7 送出;DataID=1(對齊 HT9045)。
- 對齊 HT9045 AGVLdID(load id);HT9045 的 report 內容為 host 動態定義,HT-160S 現以 Report 7 單一身分 SVID 為預設,host 亦可經 S2F35 改綁自己的報表。

#### 3.3.5 舊字典 → 新字典對照 / Old-to-new CEID mapping

**host 端必須依此表重新設定。** 左欄為 2026-07-27 及之前版本 HT-160S 所宣告的號碼,右欄為本版(= HT-90XX)號碼。

**Hosts must re-provision using this table.** Left column = the id published by HT-160S up to 2026-07-27;
right column = this revision's id (= HT-90XX's).

| 事件 / Event | 舊 CEID | **新 CEID** | 備註 |
|---|---|---|---|
| Handler / 機台狀態變更 | 1 | **27** | 舊版無發射點,本版起會送 |
| 工作檔(recipe)切換 | 2 | **15** | |
| Clear Count 按下 | 3 | **5** | 舊版無發射點,本版起會送 |
| Start 按下(機內無 IC) | 4 | **1** | |
| Start 按下(機內有 IC) | 5 | **76** | 舊版無發射點,本版起會送 |
| Pause 按下 | 6 | **2** | |
| Home 按下 | 7 | **25** | |
| One Cycle 按下 | 8 | **3** | |
| Clean Out 按下 | 9 | **4** | |
| Tray Feed 按下 | 10 | **32** | |
| Lot Start | 11 | **6** | |
| Lot End | 12 | **8** | |
| Exit 按下 | 13 | **24** | 舊版無發射點,本版起會送 |
| Retry 按下 | 14 | **28** | |
| Skip 按下 | 15 | **29** | |
| Alarm Reset 按下 | 16 | **30** | |
| Show Alarm | 17 | **(取消)** | HT-90XX 無此事件;警報一律走 S5F1 |
| Release Alarm | 18 | **(取消)** | 同上(S5F1 ALCD bit7=0 表解除) |
| Show Message | 19 | **(取消)** | HT-90XX 無「訊息顯示」事件 |
| 訊息框關閉 | 20 | **73** | 舊版無發射點,本版起會送 |
| User Level 切換 | 21 | **16** | |
| 進入 Setup / Tool 頁 | 22 | **17** | 舊版無發射點,本版起會送 |
| 進入 Maintenance 頁 | 23 | **18** | 舊版無發射點,本版起會送 |
| 進入 I/O 頁 | 24 | **21** | 舊版無發射點,本版起會送 |
| 進入 Teach / Offset 頁 | 25 | **19** | 舊版無發射點,本版起會送 |
| 進入 SECS GEM 頁 | 26 | **(取消)** | HT-90XX 無此事件 |
| One Cycle 完成 | 27 | **41** | |
| Clean Out 完成 | 28 | **42** | |
| Tray Feed 完成 | 29 | **49** | 兩版皆無發射點(本機尚未實作結束判定) |
| 週期資料推送 | 30 | **80** | 兩版皆無發射點 |
| Real / Dummy 切換 | 31 | **9** | |
| Auto Full / Unloading tray / AMR | 35–37, 136–138, 145–150, 272–275 | **不變 / unchanged** | 舊版即已對齊 HT-90XX |

> **最高風險的兩列 / The two riskiest rows**
>
> **27** 與 **28**:這兩個號碼在新舊字典中**都存在但意思不同**。舊版 27 = One Cycle Finish、28 = Clean Out
> Finish;本版 27 = Change Machine State、28 = Retry Pressed。若 host 未更新設定,這兩個號碼會被**靜默誤讀**
> (格式合法、語意錯誤)。請務必確認 host 已切換到本節字典。
>
> **27** and **28** exist in both dictionaries with different meanings. A host that is not re-provisioned
> will **silently misread** them — well-formed message, wrong semantics. Confirm the host has switched.

### 3.4 遠端命令 / Remote Commands (RCMD) — S2F41/F42

| RCMD | 動作 | 參數 CP | 說明 |
|---|---|---|---|
| `SET_LOT_INFO` | **唯一**的 Lot 資訊設定命令(**疊加式**) | `L,n{ A custLot }` 或 `L,n{ L,2{ A custLot, A kyecLot } }` | **20260730 起改為疊加**:不再清空既有工單。同一 Lot id 只保留一筆(沿用原 slot、index 與計數)。同一 Lot 若改帶**不同的 KYEC 批號**,會退掉該 Lot 舊的 2D/Bin 明細(此情形機內尚有 IC 時回 **4**)。登錄表會超過 64 筆回 **2**(不再靜默丟批)。不啟動運轉 |
| `LOTSTART` | 開批 / 補拉 2D-Bin(**不帶 Lot id**) | 無(空 list;京元 HT9045 現場實測即為 `L[0]`) | **20260730 起對齊 HT9045**(`uHGemHT9045.cpp:2081`):一律回 **HCACK=0**,不帶、也不設定任何 Lot 身分(Lot 資訊一律由 `SET_LOT_INFO` 設定;清單內若仍帶 lot id 會被忽略並記 log),可重複下達。**批未開**→執行全套開批初始化(per-lot 計數 / UPH 資料夾 / Soter buffer / 產品資訊 / (Lot,Bin)→Auto 綁定 / 1009 開批時刻 / CEID 6)+ 拉 2D/Bin;**批已開**→只拉 2D/Bin,不做任何初始化。唯二回非 0 的情況都來自 HT-160S 專屬的 `SORTMODE` pair:格式或值錯誤回 2;批已開時要換 sort mode、或 WhiteList 模式要重載檔案回 4。不啟動運轉(啟動仍為 operator-gated / `START`) |
| `CLEAR_LOT_INFO` | 結批(host 版 Lot End) | — | 對齊 HT9045(`uHGemHT9045.cpp:2431`)。走與面板 Lot End **同一條** `DoLotEndProcess`:記錄 UPH、發 CEID 8、歸檔 LotStory、清空工單 / (Lot,Bin) 綁定 / WhiteList 覆蓋、取消在途 WebAPI pull。運轉中回 **1**、機內仍有 IC 回 **2**(皆為 9045 語意)。**`SET_LOT_INFO` 改疊加之後,這是唯一會退掉 Lot 的命令** |
| `START` | 遠端啟動運轉 | — | 需 RUN CHECK / idle 條件 |
| `STOP` | 停機(收尾) | — | |
| `PAUSE` | 暫停 | — | = SystemStart=false, SoftStop=true |
| `HOME` | 回原點 | — | |
| `CLEARCOUNT` | 清除**全部**生產計數 | — | HT-160S 專屬。清 TotalIC / UPH / LoaderIC / JamCount / scanned / sorted **與**各 Bin/Auto 分選數,並重啟 UPH 計時。運轉中回 4 |
| `CLEAN_AUTO_SORT_COUNT` | 只清**各 Auto / Bin 分選數** | — | 對齊 HT9045。**與 `CLEARCOUNT` 語意不同,不可互換**:本命令保留機台層總數(TotalIC/UPH/…),只歸零 per-Bin / per-Auto 分選數。機內仍有 IC 時回 2。清除前會把原值記入 Production log |
| `CLEAN_OUT` | 遠端啟動排料(Clean Out) | — | 對齊 HT9045。僅能自 **Normal 運轉模式**進入;非 Normal 回 2(不謊報已啟動,9045 於此處一律回 0) |
| `HALT` | 遠端軟停(減速停止) | — | 對齊 HT9045(9045 為清除 SoftStart 閂鎖的軟停,非硬停)。機台未運轉回 2。與 `STOP` 的差別:`STOP` 為硬停(立即停馬達),`HALT` 走與 `PAUSE` 同一減速停止路徑 |
| `ONLINE_REMOTE` / `ONLINE` | 控制狀態→On-Line Remote | — | 公佈於 **SVID 4 = 3**(前一狀態記入 SVID 9),並發 CEID 141 + 93。**2026-08-04 起 SVID 4 / 9 是唯一的公佈編號**(原並存的 66002 已移除);機內部值為 GEM 標準的 5,不對外公佈 |
| `ONLINE_LOCAL` | 控制狀態→On-Line Local | — | 公佈於 **SVID 4 = 2**(前一狀態記入 SVID 9),並發 CEID 141 + 92。機內部值為 GEM 標準的 4,不對外公佈 |
| `START_AGV` | AMR 派車 prep + lock | station / `LoaderTrayCount` … | 記 intent+盤數;實際 motion 由機構/START 驅動 |
| `ONE_CYCLE` | 執行單一循環後停機(Clean Out 進行中為例外,見下方註) | 無 (空 list) | 0=已啟動 / **2=機台未運轉**(不可執行)/ 4=已在 OneCycle 中 / 2=模式或 Lot 資料不符。**與 HT9045 不同**:9045 一律回 0,HT-160S 據實回報。註:機台未運轉一律回 2 而非 4——SEMI E5 的 HCACK=4 語意是「已受理,稍後以事件通知完成」,而停機時本機不會發出 CEID 41 完成事件,回 4 會讓 host 無限等待 |
| `ENERGY_SAVING` | (不支援) 省電模式 | `STATE` = 0/1 | **固定回 HCACK=2**。HT-160S 無加熱器/ATC/省電子系統,語法正確亦拒絕,不謊報成功。與京元現行 HT9045 回覆一致 |
| `PP_SIGNALTOWER` | 主機強制指定三色燈 | `RED`/`GREEN`/`YELLOW` = 0關/1亮/2閃 | 空 list = 解除 (可重複)。**請每次同時指定三色**(京元現行 host 即如此):未列出的顏色沿用「本次覆寫期間」的既有值,而解除會把三色記憶歸零,故解除後只指定一色的 SET 會使另兩色為關。未知 CP 或值超出 0..2 則整包拒絕 (HCACK=2) 且不套用。警報 Note 顯示中、訊息視窗顯示中、或機台自身處於安全異常狀態(EMG/安全門/安全鎖/斷氣/離子風扇,即 RunState=LED_ErrJam)時暫停覆寫,避免遮蔽機台紅燈。值 2(閃)在本機依既有慣例呈現為**恆亮**(HT-160S 塔燈不閃) |
| `PP_MUSIC` | 主機強制指定蜂鳴器音效 | 單一 pair,CP 名稱為空字串 `A[0]`,值 = 1..4 | 空 list = 解除 (可重複)。CP 名稱讀取後即丟棄不比對 (京元送空字串);值超出 1..4 回 HCACK=2 |

> HCACK: 0=成功 / 1=命令無效 / 2=參數錯,或**認得命令但本機不執行**(如 `ENERGY_SAVING`、`START_AGV` 未知 CP / AMR 關閉)/ 4=busy(運轉中、機內有 IC、或已在 OneCycle)。未列命令仍回 HCACK=1。
>
> **HT9045 的測試機專屬命令回 HCACK=2,不回 1**。下列 16 個命令確實存在於 HT9045,但描述的是 HT-160S(sorter)沒有的機構(測試/重測流程 ART / MRT / AQL、FT-RT 程式切換、清針、site mapping、器件溫度與 EESUG offset、yield-fail):
> `AUTO_RETEST` `CONTINUE_RETEST_ART` `CONTINUE_START_ART` `CONTINUE_START_MRT` `INITIAL_START_ART` `INITIAL_START_MRT` `RETEST_MRT` `SWITCH_TO_FT` `SWITCH_TO_RT` `START_AQL` `DEVTEMPOFFSETADJUST` `TESTTEMPSETTING` `EESUG_OFFSET` `AUTOSITEMAP` `AUTO_CLEAN` `YIELD_FAIL`
> 回 2 而非 1 的理由:對 host 稽核而言 1 讀作「本設備沒聽過這個命令」(像打錯字或版本不符),2 才是實話「認得,但本機沒有這個功能」。**一律不回 4** —— HCACK=4 依 SEMI E5 是肯定回覆(承諾稍後以事件通知完成),而這些命令永遠不會產生完成事件。
>
> **尚未實作的 HT9045 命令**:`TRAY_FEED`(本機無 Tray Feed 功能 —— `Run_TrayFeed` 無 live 進入點,結束判定亦未實作)、`RESET`(HT9045 的 `Reset()` 是測試機收料回復流程:未測件送 error bin、OutArm/Shuttle/RotateKit 復位、GPIB 重置,HT-160S 無對應機構;本機最接近的遠端復位是 `HOME`)。此兩者目前落在未列命令,回 HCACK=1。
> **`ONE_CYCLE` 於 Clean Out(排料)進行中不會停機**:本機允許在 Clean Out 中受理 `ONE_CYCLE`(回 0),
> 循環結束時會**回到 Clean Out 繼續排料而不停機**,但仍會送出 CEID 41「One Cycle Finish」。
> 也就是說在 Clean Out 期間,CEID 41 代表「該循環結束」而非「機台已停」。若 host 需要「必定停機」的語意,
> 請在非 Clean Out 狀態下發送。

> **`ONE_CYCLE` 為此對照表的例外**,請逕依 §3.4 該列:對 `ONE_CYCLE` 而言「運轉中」是回 **0**(正常受理)的狀態,「已在 OneCycle」只在運轉中才回 4,「機台未運轉」回 **2**。原因是 HCACK=4 依 SEMI E5 為肯定回覆(稍後以事件通知完成),而唯有運轉中的循環才會真的送出 CEID 41 完成事件。

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

事件觸發時,S6F11 依 CEID→RPTID→SVID 連結序列化當前 SV 值。報表**定義**(`S2F33` 的 RPTID→SVID 內容)**跨重開機保留**,存於 `system\EventReportDef.ini`;⚠ 但**事件→報表連結(`S2F35` 的 CEID→RPTID)只在 session 內有效、不跨電源保存**,復電後 288 個 CEID 全部退回機台預設的 Report 1(= `{1027 System Time}` 一格)。因此 host 每次(重)連線都必須重跑 `S2F33` + `S2F35`。詳見 §3.3.3。

### 容忍行為(重要)/ Tolerance behavior (important)

為讓依 **HT-90XX 字典**設定的 host(其引用的 SVID/CEID 未必存在於 HT-160S)能順利完成報表定義,HT-160S 採**容忍**策略,而非整批拒絕:

- **S2F33 未知 SVID**:接受定義(不回 DRACK=0x04),該 SVID 於 S6F11 回**空 item**。
- **S2F35 未知 CEID**:接受連結(自動建立為 host CEID,無對應機台事件則不觸發)。
- **S2F35 未知 RPTID**:略過該連結(不影響其餘)。
- **保護**:若對「已存在的機台事件(如 AMR 272-275)」連結到全部未知的 RPTID,**不覆寫**其既有綁定(避免清空)。

- **CEID 編號語意差異 —— 已於 2026-07-29 解除**:先前 HT-160S 的 CEID 1–31 與 HT-90XX 同號不同義,現已整份改為 HT-90XX 字典的逐字複本(§3.3),**同號同義**。host 可直接沿用 HT-90XX 的 CEID 設定;`S6F15` 拉取任一號碼取得的語意亦與 HT-90XX 相同。唯一需注意的是本機只有 53 個號碼有發射點(§3.3.1)。**由舊版升級的 host 必須依 §3.3.5 重新設定**,尤其 CEID 27 / 28。

> 換言之:host 可完整完成上線與報表定義流程;未對應到 HT-160S 實際資料的欄位會回空值,待雙方確認後再由 HT-160S 補實作或由 host 改用本規格 §3 的 SVID/CEID。

#### 4.x「同一件事、不同號碼」對照(host 必讀)

下列三個事件**兩邊都有,但編號不同,而且對方的號碼在本機另有他義**。這是最容易誤讀的一組:

| 事件 | HT-90XX CEID | HT-160S CEID | 該號碼在對方機台的意義 |
|---|---|---|---|
| One Cycle 完成 | 41 | **27** | 27 在 HT-90XX = `Change Machine State` |
| Clean Out 完成 | 42 | **28** | 28 在 HT-90XX = `Retry Pressed` |
| Tray Feed 完成 | 49 | **29** | 29 在 HT-90XX = `Skip Pressed` |

> **CEID 41 需特別注意**:HT-160S 於 One Cycle 完成時**確實會發射 CEID 41**(2026-07-29 前此事件掛在本機自有的 CEID 27 上)。
> 而 `Change Machine State` 是京元現場 HT-90XX **當天發射量最大的事件**(2026-06-08 約 406 次 S6F11)。
> 若 host 沿用 HT-90XX 字典,會把本機的「單循環完成」解讀為「機台狀態改變」。請以本規格 §3.3 為準。

> **HT-160S 未提供對應 S5F1**:HT-90XX 於 One Cycle 完成時另發一筆 S5F1(ALID `316001640`, ALTX `One cycle finish`)。
> HT-160S 只發 S6F11 事件,不發此告警。若 host 是以該 ALID 判斷循環結束,請改為訂閱 CEID 41,或與我方確認後補上。

---

## 5. 尚未支援 / Not Yet Supported

以下訊息目前**未實作**(收到僅記錄於本機 SECS log,**不回覆任何訊息**,故 host 端會是 T3 逾時;不影響上線流程)。如客戶流程需要,可依需求評估補上:

> 20260728 更新:`S6F15/F16`、`S6F19/F20`、`S10F3–F6`、`S125F1/F2` 已實作完成並移入 §2,不再屬於本節。此四組是京元現場 log (2026-06-08) 證實 host 會主動送出、而本機先前不回覆造成 T3 逾時的訊息。

> 20260730 更新:`S1F23/F24`(Collection Event Namelist)與 `S2F29/F30`(EC Namelist)已實作完成並移入 §2,不再屬於本節。兩者皆為唯讀的字典查詢:S1F23 不影響事件訂閱(訂閱仍由 S2F33/F35/F37 決定),S2F29 不放寬 EC 可寫範圍(寫入仍走 S2F15 的閒置閘與 tray 幾何白名單)。

| 訊息 | 名稱 | 備註 |
|---|---|---|
| S2F23 / F24 | Trace Initialize | 以事件報表(S2F33/35/37 + S6F11)取代 |
| S7F1–F26 | Process Program(配方上下傳) | recipe 走本地/FTP;非 event-report 集 |
| S14F1 / F2 | GetAttr(物件屬性) | |

---

## 6. 對齊 HT-90XX 註記 / Alignment Notes vs HT-90XX

- **命令面 / Command surface**:S1/S2/S5/S6 的基礎 GEM 訊息與 HT-90XX 對齊(見 §2)。
- **字典面 / Dictionary**:HT-160S 為 tray sorter,無 tester 站位/測試結果/接觸次數等概念,故:
  - HT-90XX 的 tester 專屬 SVID(如 1420 32-site 測試結果、6001/6002 Arm 接觸次數、1007 Operator ID、1513 Tester On/Off、1151–1156 各站良率、16296–16299 各 site 累計)**在 HT-160S 無對應**,以容忍機制回空值或請 host 移除。
  - **更正(2026-08-03)**:本條先前把 **1006** 一併列為「tester 專屬、無對應」並建議 host 移除,那是錯的——1006 在 HT-90XX 是 **Lot ID**(不是 Socket ID),HT-160S **自 2026-08-01 起已實作**並回目前作用中的批號(§3.1)。請**不要**從報表定義中移除 1006。同理 1007 是 **Operator ID**,不是 Socket ID。
  - **再更正(2026-08-03,同日稍後)**:上一句原本接著寫「1007 確實無對應,因為 HT-160S 沒有操作員身分欄位可回」——該敘述**已不再成立**。HT-160S 已於本日新增操作員身分欄位並以 **SVID / ECID 1007** 公佈(§3.1 / §3.2),且**接受貴端以 `S2F15` 寫入**(貴端 2026-06-08 對 HT-90XX 寫的 `"AGV"` 用法在本機同樣受理)。請**不要**從報表定義中移除 1007。本條剩下真正無對應的號碼是:1420、6001/6002、1513、1151–1156、16296–16299。
  - **更正(2026-08-04)**:本條先前寫「HT-160S 的機台狀態/產出/AMR 資料集中於 **66xxx** 與 38xxx 段」——**66xxx 段已於 2026-08-04 依貴端裁定全數退役**(最後留存的十個號碼 66000 / 66001 / 66002 / 66010 / 66011 / 66020 / 66021 / 66030 / 66031 / 66032 不再註冊,逐號替代方案見 §3.1 的退役墓碑表;新增家族編號 **1008 Run Mode** 取代 66000)。機台狀態與產出資料現在**一律使用 HT-90XX 家族的共同段編號**(1008 Run Mode、1011 Machine State、1006 Lot ID、1101 / 1102 / 1103–1105 / 1259–1261 各項計數);**只有 AMR / AGV 資料仍在 38xxx 段**。註冊 SVID 總數 **76 → 67**。
  - **已知並接受的代價(2026-08-04)**:分選模式(原 **66032 Sort Mode**)在 HT-90XX 家族中沒有可用編號(唯一近似的 35530「[I27] Manual sort mode」是 HT-9045/46 的 BOOLEAN 選項),host 仍可用 `S2F41 LOTSTART` 的 `SORTMODE` pair **設定**,但**無法再以 `S1F3` 回讀**。
  - **1006 語意擴充(2026-08-04)**:SVID 1006 改為「全部已登錄批號、以半角逗號串接」。⚠ **1006 是貴端已保存的 `RPTID 502` 第 1 格**,故該既有事件報表第 1 格的值會在多批生產時變成 `LOT_A,LOT_B` 形式(item 數與 A 型別不變);把該格當單一批號 token 解析的 host 需更新。單批時位元與舊版完全相同。詳見 §3.1、§3.3.3。
- **建議**:host 端請依本規格 §3 對應 HT-160S 實際 SVID/CEID;無法對應者(tester 專屬)由雙方確認後移除或以容忍空值處理。

---

## 7. HT-160S 專屬項目與對齊狀態 / HT-160S-Specific Items & Alignment Status

> 依客戶要求:凡 HT-160S 使用到的 SECS 命令皆對齊 HT-90XX (HT9045)。§7.1 列已對齊項;§7.2 列 HT-160S 專屬(9045 無對應或同號不同義)項目及其存在理由。
> Per customer requirement, every SECS command HT-160S uses is aligned to HT9045. §7.1 = aligned; §7.2 = HT-160S-specific with rationale.

### 7.1 已對齊 HT9045 / Aligned to HT9045
- **訊息 Messages**:S1 / S2 / S5 / S6 基礎 GEM 全數對齊(§2)。
- **RCMD**:`PAUSE` `START` `STOP` `LOTSTART` `CLEAR_LOT_INFO` `ONLINE_REMOTE` `ONLINE_LOCAL` `START_AGV` `ONE_CYCLE` `ENERGY_SAVING` `PP_SIGNALTOWER` `PP_MUSIC` `CLEAN_OUT` `HALT` `CLEAN_AUTO_SORT_COUNT`。
  - `CLEAN_OUT` / `HALT` / `CLEAN_AUTO_SORT_COUNT` 為 2026-07-29 新增對齊。
  - `LOTSTART` 於 2026-07-30 改為與 9045 逐字對齊(無參數、無條件 HCACK=0、可重複下達);`CLEAR_LOT_INFO`(host 版 Lot End)為同日新增對齊。兩者合起來構成 host 端完整的開批 / 結批一對。
  - **`HOME` 已自本節移出** —— 經逐字查證 HT9045 的 S2F42 dispatch,其命令集為 `AUTHORITY_CHECK` `AUTOSITEMAP` `AUTO_CLEAN` `AUTO_RETEST` `CLEAN_AUTO_SORT_COUNT` `CLEAN_OUT` `CLEAR_LOT_INFO` `CLOSE_ONECYCLE` `CONTINUE_*` `DEVTEMPOFFSETADJUST` `EESUG_OFFSET` `HALT` `INITIAL_START*` `LOTORDER` `LOTSTART` `ONE_CYCLE` `ONLINE_LOCAL` `ONLINE_REMOTE` `PAUSE` `PP_*` `REMOTE_*` `RESET` `RETEST_MRT` `SET_LOT_INFO` `START` `START_AGV` `START_AQL` `START_LOT` `STOP` `STOP_LOT` `SUBSTRATETYPE` `SWITCH_TO_*` `TESTTEMPSETTING` `TRAY_FEED` `TRAY_MAP` `YIELD_FAIL` —— **其中沒有 `HOME`**。改列 §7.2。
  - 三項宣告差異:`ONE_CYCLE` 據實回 HCACK(9045 一律 0);`ENERGY_SAVING` 固定回 2(本機無省電子系統,與京元 9045 現行回覆相同);`PP_SIGNALTOWER`/`PP_MUSIC` 在警報 Note 顯示期間、訊息視窗顯示期間、以及機台自身安全異常(RunState=LED_ErrJam)時暫停覆寫(9045 無此例外),以免遮蔽機台自身紅燈與警報音;值 2(閃)呈現為恆亮。
- **SVID 共同段**:3 / 4 / 9 / 1001 / 1002 / 1003 / 1006 / 1007 / **1008** / 1009 / 1011 / 1021 / 1027 / 1101 / 1102 / 1103 / 1104 / 1105 / 1259 / 1260 / 1261 / 1501 / 1517 / 1518 / 2758–2763 / 37010(號碼、型別與線上格式皆對齊 HT-90XX)。
  - **1008 Run Mode 為 2026-08-04 新增**(A,值域 `0:Normal; 1:RT; 2:EQC`,本機恆為 `"0"`),取代已退役的 66000;1259–1261 為 2026-08-03 由 66025–66027 改號而來。
  - **自 2026-08-04 起本機註冊的 SVID 共 67 個**(原 76),全部落在此共同段與 AMR 的 38xxx 段,**不再有任何 66xxx 號碼**。
- **ECID**:1501,2758–2763(Type1 tray 幾何)。
- **CEID**:AMR 272 / 273 / 274 / 275;Auto-Full 35 / 36 / 37 / **148 / 149 / 150**;Auto-Unloadtray **136 / 137 / 138 / 145 / 146 / 147**。
  - 上列 Auto-Full 六號與 Auto-Unloadtray 六號**皆與 HT-90XX 同號同義**(依 HT-90XX 韌體 CEID 目錄 `EventReport_CEID.def`:148/149/150 = `Auto 4/5/6 Full`,136–138/145–147 = `Auto 1–6 Unloading tray`)。
  - 註:HT-160S 有 6 個 Auto 輸出站,故六號全部會用到。京元 2026-06-08 當天的 HT-90XX log 中,本家族只觀察到 `136`(2 次)與 `137`(3 次)實際發射(兩者皆為空報表),其餘號碼當天未出現 —— 但**編號與語意皆為 9045 韌體既有定義**,並非 HT-160S 自創,故列為「已對齊」而非「HT-160S 專屬」。
- **AMR 資料**:Tray/Device Count(SVID 38222+ / 38228+)、Color 身分 2D(CEID 275 / SVID 38204)、事件 DataID=1 —— 皆對齊 HT9045(見 §3.3)。

### 7.2 HT-160S 專屬(9045 無對應)+ 為何特殊 / HT-160S-only + rationale

| 項目 Item | 類別 | 用途 Purpose | 為何 HT-160S 專屬 Why HT-160S-specific |
|---|---|---|---|
| `SET_LOT_INFO` | RCMD | **疊加式**登記 Lot,並且是本機**唯一**的 Lot 資訊設定入口(2026-07-30 起) | **同名不同體(不是 9045 沒有)**。HT9045 的 S2F42 確實有 `SET_LOT_INFO`,但兩邊的 lot 模型與參數本體不同:HT-160S 收 `L,n{ A custLot }`(或 `L,2{custLot,kyecLot}` 配對)並**疊加**進登錄表、同 id 留一筆;9045 的版本收 `L,2{("LOT_INFO", XML), ("DISPLAY", str)}`,走它自己的 SPIL XML lot 模型(`uHGemHT9045.cpp:2183`)。**host 不可假設兩機的 `SET_LOT_INFO` 可互換**,需依 §3.4 本機格式。另註:京元 2026-06-08 全日 log 中 host **從未送過** `SET_LOT_INFO`——該線的 Lot 身分是 host 用 `S10F5` 終端訊息告知操作員、由人輸入機台,再由機台回報給 host |
| `HOME` | RCMD | 遠端回原點(等同操作員 Home 鍵) | **9045 整棵 SECSGEM 樹查無 `HOME` 命令**(見 §7.1 的完整命令集)。9045 最接近的是 `RESET`,但那是測試機收料回復流程,語意不同。保留本命令:刪掉會少一個有用的遠端功能而換不到任何對齊 |
| `ONLINE`(裸) | RCMD | = `ONLINE_REMOTE` 別名 | 便利別名;9045 僅有 `ONLINE_REMOTE` / `ONLINE_LOCAL` |
| `CLEARCOUNT` | RCMD | host 遠端清除生產計數 | 9045 將 clear-count 僅作操作員事件(CEID 5,HT-160S 自 2026-07-29 起同號同義且會發射),無對應 RCMD(9045 另有 `CLEAN_AUTO_SORT_COUNT`,語意不同) |
| ~~SVID **66000–66033**~~ | SVID | — | **本列已於 2026-08-04 作廢:66xxx 自有段整段退役,本機不再公佈任何 66xxx SVID。** 依貴端裁定「HT-160S 只公佈 HT-90XX 家族編號」,最後留存的十個號碼 **66000 / 66001 / 66002 / 66010 / 66011 / 66020 / 66021 / 66030 / 66031 / 66032**(RunMode / SystemRunning / ControlState / AlarmActive / AlarmCode / TotalIC / TotalSorted / ActiveLotCount / CurrentLotID / SortMode)已全部不再註冊;2026-08-03 已先移除 66022–66024(與 1103–1105 同值)、66025–66027(改用 1259–1261)、66033(與 1009 同值)。**逐號替代方案見 §3.1 的退役墓碑表**;新增家族編號 **1008 Run Mode** 取代 66000。這些號碼**永久退役、永不遞補、永不改用他義**,故舊綁定只會得到可偵測的空 item `<L[0]>`。⚠ **已知並接受的代價**:分選模式(原 66032)無家族編號可用——host 仍可用 `LOTSTART` 的 `SORTMODE` pair 設定,但無法以 `S1F3` 回讀。本機剩下唯一的延伸號碼是下一列的 38237–38245 |
| SVID **38237–38245** | SVID | Auto4/5/6 的 tray / device / bin-setting,共 **9 個號**(carrier 三個號已於 2026-08-03 改用家族編號 38199–38201,不再是本機延伸,見 §3.1.x 的 AMR 對照表) | HT-160S 有 **6 個 Auto 輸出站**,9045 的 AMR 目錄僅到 38236(3 站);為第 4–6 站延伸。**更正(2026-07-29)**:舊版本節誤寫成「38237–38245 = Auto4/5/6 的 carrier/tray/device/bin-setting」—— carrier 三個號當時實際是 38208/38209/38210,不在 38237–38245 之內。**2026-08-03 已用原始碼查證(不再是「待確認」)**:(a)**38237–38245 確實無 HT-90XX 對應號碼** —— HT-9046LS V3.32.810 的 AMR 段是 38222–38236(Loader/Empty/Color + Auto1–3 的 tray / device count 與 Auto1–3 bin setting)且**止於 38236**,而 HT-9011UC V3.33.899 **整段沒有 AMR SVID**,故本機這 9 個延伸號成立、保留。(b)**38208–38210 已改號(已裁定,不再是爭議)**:HT-9011UC V3.33.899 有 Auto4–6 的盤號欄位 **38199 / 38200 / 38201**(名稱 `Output 4/5/6 Tray ID`)。依「Auto1–3 遵循 810、Auto4–6 參照 899」的原則,Auto4–6 carrier 已改用 38199–38201,Auto1–3 維持 810 的 38205–38207;命名空間統一留在 **SVID**(899 把該家族宣告為 ECID,但 810 對 Auto1–3 是 SVID,六個口必須同一命名空間) |
| ~~CEID **1–31** 編號~~ | CEID | — | **本列已於 2026-07-29 作廢**:CEID 字典整份改為 HT9045 逐字複本(1–292 全數註冊、別名逐字相同),**已無「同號不同義」問題**。host 可直接沿用 HT9045 的 CEID 設定;由舊版升級者請依 **§3.3.5** 對照表重新設定。詳見 §3.3。 |

> **無 HT-160S 專屬的 SxFy 訊息,亦無專屬 ECID** —— HT-160S 未自創任何 SECS 訊息(§2 皆標準 GEM),ECID 全數對齊 9045。

---

*本規格依現行 firmware 實作產出,供 KYEC / EAP 整合對接使用。行為之最終確認以現場 host round-trip 為準。*
*This spec reflects the shipping firmware; final behavior is confirmed by on-site host round-trip testing.*
