# HT9045 / HT160S CEID 對照矩陣 (2026-07-29)

## 0. 出處與方法 / Provenance

**建立日期**:2026-07-29

**使用來源(全部唯讀)**

| 用途 | 絕對路徑 |
|---|---|
| 9045 CEID 目錄(韌體傾印) | `D:\backup_version\HT9046\KYEC\20260626\EventReport_CEID.def` |
| 9045 報表定義傾印 | `D:\backup_version\HT9046\KYEC\20260626\EventReport_ReportID.def` |
| 9045 事件 enum / 描述字串 | `D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\SECSGEM\uHGemHT9045.h`、`...\SECSGEM\uHGemHT9045.cpp` |
| 9045 事件發射點 | `D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\` 之 `main.cpp` / `note.cpp` / `csystem.cpp` / `mymessbox.cpp` / `asendic_Loader.cpp` / `acatchtray.cpp` / `AutoClean\AutoClean.cpp` / `BarcodeReader.cpp` / `cSpeed.cpp` / `PowerSavingMode.cpp` / `HS_Function.cpp` / `uLotInfo.cpp` |
| 京元竹南現場 log(2026-06-08 全日,20 檔) | `D:\backup_version\HT9046\KYEC\20260626\2026_06_08\SECSGEM_TextLog_00.txt` … `_19.txt` |
| HT160S CEID 註冊 / 描述 | `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemHT160.cpp`(`AddCEID` / `AddReprot`)、`...\SecsGem\uHGemHT160.h`(`ETypeStruct SECS_EVENT`) |
| HT160S S6F11 送出邏輯 | `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemEquipment.cpp`(`THGem::EventReport`、`THGem::EmitEventReportBody`) |
| HT160S 事件發射點 | `...\main.cpp`、`...\note.cpp`、`...\aAuto1To6.cpp`、`...\aColor.cpp`、`...\SecsGem\uAgvStation.cpp`、`...\csystem.cpp` |
| 客戶規格書(對客戶為權威) | `D:\HT160S_BCB\docs\SECS\HT160S_SECS_Interface_Spec_20260727.md` |
| 前次比對(僅作提示,本文全部重新推導) | `D:\HT160S_BCB\docs\plan\secs-9045kyec-diff-20260728\`、`D:\HT160S_BCB\docs\plan\secs-9045kyec-vs-160-cmd-diff-20260728.md` |
| Path A 三方交叉複驗裁定 | `D:\HT160S_BCB\docs\plan\secs-pathA-3way-crossverify-20260727.md` |

**方法說明(哪些數字來自 log、哪些來自原始碼)**

- 「**HT9045 名稱**」「**HT160S 名稱**」「**語意**」「**本機是否會發射**」= **原始碼推導**(9045 `.def` + `uHGemHT9045.h/.cpp`;HT160S `AddCEID` + `SECS_EVENT` + 全樹 `EventReport(` / `AutoCeid` / `AutoFullCeid` grep)。
- 「**京元當天發射次數**」「**host 訂閱**」「**host 拉取**」= **現場 log 統計**(2026-06-08 全日 20 檔,本文自行解析 SML,未沿用任何既有 JSON)。
- 兩者的**交集**才是真實可觸發的問題;只在原始碼層存在差異、當天 host 完全沒碰的號碼,屬文件面工作。本文每張表都分開列這兩種欄位,不混寫。

**本文自行複算的計數(未沿用任何既有檔案)**

| 項目 | 數值 | 複算方式 |
|---|---|---|
| `EventReport_CEID.def` 資料列 | **292**(CEID 1–292 連續) | 逐列解析 |
| 其中具名(alias 除數字前綴外有文字) | **286** | 未具名者為 CEID 74 / 75 / 79 / 142 / 143 / 144 共 6 個 |
| 9045 原始碼 `EventDescription[]` 指派 | **272** 條(CEID 1–275,缺 142/143/144) | regex 解析 `uHGemHT9045.cpp` |
| 原始碼 ∩ `.def` 名稱不一致 | **0**(272 條全數逐字相符) | 逐號比對 |
| `.def` 有、V3.32.810 原始碼**沒有**的號碼 | **20** 個:142、143、144、276–292 | 集合差 |
| HT160S CEID 號碼總數 | **47** | 已註冊 41 + 未註冊但會上線發射 6 |
| ├ 已註冊(`SetCEIDContent`) | **41** | `AddCEID` 迴圈 1–31(31)+ 272–275(4)+ 35/36/37/148/149/150(6) |
| ├ 已註冊但無任何發射點 | **15** | CEID 1、3、5、13、17、18、19、20、22、23、24、25、26、29、30 |
| └ 未註冊但會實際發射 | **6** | CEID 136/137/138/145/146/147(`aAuto1To6.cpp:788`) |
| 京元當天實際送出的 S6F11 | **645** 筆 / **46** 個不同 CEID | 解析每個 `[S6F11] Event Report Send` 區塊的第 2 個 `U4`(= CEID) |
| 京元當天因 host 停用而放棄送出 | **10987** 筆 / **7** 個 CEID(CEID 80 佔 10962) | 解析 `CEID=N be disabled , abort send !!!` |
| 京元當天 S2F35 Link Event Report | **121** 筆(其中 3 筆為 `L[0]` 清空)/ **122** 條連結 / **34** 個不同 CEID | 逐區塊解析,已扣除外層 `<L[2]>`+`DATAID`(該 host 把 DATAID 設成 CEID 本身,天真 regex 會重複計數) |
| 京元當天 S6F15 Event Report Request(host 主動拉取) | **3** 筆,**全部為 CEID = 1** | 本體為裸 `<U4[1] 1>` |
| 京元當天 S2F37 Enable/Disable | **9** 筆 = **3 輪相同 provisioning**(停全部 → 開全部 → 只停 CEID 80),時間 13:58、14:06、16:47 | 逐筆傾印 |

**覆蓋率自我檢查**:§1(16,一致)+ §2(31,同號不同義)= 47 = HT160S 全部號碼;§3a(26,僅 9045 且當天用過)+ §3b(219,僅 9045 且當天未用)= 245 = 僅 9045 的號碼。47 + 245 = **292 = `.def` 全部列數**。本文對 292 個號碼**逐號有歸屬,無遺漏、無重複計數**;其中 73 個逐條列出,219 個依主題彙總(§3b)。

> **重要出處警告**:`EventReport_CEID.def`(292 列)比手上的 9045 原始碼快照(V3.32.810,enum 止於 275)**更新**。
> `.def` 的 276–292(`Loader_Buffer_NoTray`、`OutputPort1-6BinCode`、`MaterialModeChange`、`PortStateUpdated`、
> `UnloaderTrayIDReadOK/Fail`、`LoaderTrayIDReadFail`、`MaximumOutputPortReport`、`RunCheckRequest`、
> `AGVLDUnLDFinish`、`AGVLdID`、`DoSecsGemIndexFail`)在該原始碼樹**完全查不到識別字**。
> 因此:凡本文引用 276–292,語意僅以 `.def` alias 為據,**不從原始碼佐證**;
> 凡本文引用 1–275,`.def` 與原始碼**已逐號交叉驗證一致**(0 不符)。

**HT160S 現況基準線(含未提交的工作樹變更)**

本文以 `feat/iosetview-172-refactor` 分支 **工作樹現況**(含未 commit 變更)為基準。與 CEID 直接相關的已實作項目:

- **CEID 27 One Cycle Finish 已有發射點**(`main.cpp:2786` `EmitOneCycleOK()`,由 `csystem.cpp:1695` OneCycle-finish 分派呼叫)—— **未提交**。本文一律視為「會發射」,不列為缺口。
- **CEID 145/146/147**(Auto4/5/6 Unloadtray)已完成改號(commit `bf9d048`)。已於原始碼複核:`aAuto1To6.cpp:747` `int AutoCeid[6]={136, 137, 138, 145, 146, 147};` —— **確認生效**。
- S6F15→S6F16、S6F19→S6F20、S10F3/F5、S125F1 已實作(未提交);RCMD `ONE_CYCLE` / `ENERGY_SAVING`(刻意 HCACK=2 拒絕)/ `PP_SIGNALTOWER` / `PP_MUSIC` 已實作;`START_AGV` 已修 Action/NA + LoaderICCount + AMR-off 拒絕(`f7f3939`)。以上不列為缺口。
- `uHGemHT160.cpp:299` 的註解仍寫 `(136-138/140-142, ...)`,**與已改好的程式碼不一致**(純註解陳舊,不影響行為)。

---

## 1. §1 一致(安全)

這一段是**兩邊同號同義**的全部號碼,共 **16 個**,恰好是三個整塊:

1. **AMR / AGV 材料交握 272–275**(4 個)
2. **Auto 滿盤 35/36/37 + 148/149/150**(6 個)
3. **Auto 退盤 Unloadtray 136/137/138 + 145/146/147**(6 個)—— 145/146/147 為 `bf9d048` 改號後才成立,已在原始碼複核

| CEID | HT9045 名稱 | HT160S 名稱 | 語意 | 京元當天發射次數 | 本機是否會發射 | 處置建議 |
|---|---|---|---|---|---|---|
| 35 | Auto 1 Full | Auto1 Full | 一致 | 0 | 是(`uAgvStation.cpp:315`,需 `bUseAMR=1`) | 維持 |
| 36 | Auto 2 Full | Auto2 Full | 一致 | 0 | 是(同上) | 維持 |
| 37 | Auto 3 Full | Auto3 Full | 一致 | 0 | 是(同上) | 維持 |
| 136 | Auto 1 Unloading tray | Auto1 Unloadtray(未註冊) | 一致 | 2 | 是(`aAuto1To6.cpp:788`) | 維持 |
| 137 | Auto 2 Unloading tray | Auto2 Unloadtray(未註冊) | 一致 | 3 | 是(同上) | 維持 |
| 138 | Auto 3 Unloading tray | Auto3 Unloadtray(未註冊) | 一致 | 0 | 是(同上) | 維持 |
| 145 | Auto 4 Unloading tray | Auto4 Unloadtray(未註冊) | 一致 | 0 | 是(同上,`bf9d048` 改號後) | 維持 |
| 146 | Auto 5 Unloading tray | Auto5 Unloadtray(未註冊) | 一致 | 0 | 是(同上) | 維持 |
| 147 | Auto 6 Unloading tray | Auto6 Unloadtray(未註冊) | 一致 | 0 | 是(同上) | 維持 |
| 148 | Auto 4 Full | Auto4 Full | 一致 | 0 | 是(`uAgvStation.cpp:315`,需 `bUseAMR=1`) | 維持 |
| 149 | Auto 5 Full | Auto5 Full | 一致 | 0 | 是(同上) | 維持 |
| 150 | Auto 6 Full | Auto6 Full | 一致 | 0 | 是(同上) | 維持 |
| 272 | AMR Supplement | AGVSupplement | 一致 | 2(host 訂閱 x3) | 是(`uAgvStation.cpp:314`/`341`,需 `bUseAMR=1`) | 維持 |
| 273 | AMR LDUnLD Status | AGVLDUnLDStatus | 一致 | 4(host 訂閱 x3) | 是(`uAgvStation.cpp:396`/`467`,需 `bUseAMR=1`) | 維持 |
| 274 | AMR LDUnLD Finish | AGVLDUnLDFinish | 一致 | 4(host 訂閱 x3) | 是(`uAgvStation.cpp:415`/`476`,需 `bUseAMR=1`) | 維持 |
| 275 | AMR LD ID | AGVLdID | 一致 | 2(host 訂閱 x3) | 是(`uAgvStation.cpp:217`,由 `aColor.cpp:1614` 身分盤 2D 掃描後呼叫;**不受 `bUseAMR` 限制**) | 維持 |

**這一段的三個註記(都影響現場,不影響「維持」的結論)**

1. **136–138 / 145–147 在 HT160S 未註冊**,`THGem::EventReport()`(`uHGemEquipment.cpp:347`)在 `FindCEIDItem()` 回 NULL 時 `reportCount=0`,但仍 `InitLocalHead(6,11,1)` → `SendLocalData()`,**S6F11 確實上線**,只是報表清單為空 `L[0]`。已對現場 log 獨立驗證 9045 也是這樣送的:`SECSGEM_TextLog_18.txt:441-449` 的 CEID 136 本體為 `<L[3] <U4 1> <U4 136> <L[0]> >`,host 回 `S6F12 <B 0x00>` 接受。**「不註冊 → 空報表」是 9045-faithful 行為,不是 bug。**
2. **客戶規格書未收錄 136/137/138/145/146/147**。`docs\SECS\HT160S_SECS_Interface_Spec_20260727.md` §3.3 只列 1–31、Auto Full 35/36/37/148/149/150、AMR 272–275;grep 全文找不到 136/145 等號碼。機台會發、規格書沒寫 → 建議規格書 §3.3 補列(**文件動作,非程式動作**)。
3. **host 對 272–275 綁的報表是 RPTID 502 + 2000 / 2001**(自 log 的 S2F35 逐筆解析),不是 HT160S 預設的 Report 2/3/4/5/6/7。HT160S 的 S2F33/S2F35 容忍策略(規格書 §4)會接受 host 自訂報表,所以 CEID 一致但**報表內容需靠 host provisioning 補齊**——SVID 層細節屬另一份 SVID 對照文件,此處僅提示。

---

## 2. §2 同號不同義(全部 31 個,無省略)

HT160S 的 CEID 1–31 與 HT9045 的 CEID 1–31 **逐一比對後全部不同義,無一例外**(含 22 對 23 這種字面相近者:9045 CEID 22 = Enter Message Page,HT160S CEID 22 = Enter Setup Page;9045 CEID 23 = Enter Debug Page,HT160S CEID 23 = Enter Maintenance Page)。

**危險度判準(明確定義,不憑感覺)**

| 等級 | 條件 |
|---|---|
| **極高** | HT160S **會發射**該號 **且** host 當天對該號有明確 S2F35 訂閱或 S6F15 拉取 → host 必定收到,且必定套 9045 語意 |
| **高** | HT160S **會發射**該號 **且** 9045 當天該號有發射(host 靠 `.def` 預設連結已有既有解讀規則) |
| **中** | HT160S 會發射但當天 host 兩邊皆無動作;或 host 有訂閱但 HT160S 不發射(host 等不到資料) |
| **低** | HT160S **不發射**該號 **且** host 未訂閱 → 純文件面 |

**2-1 極高 / 高(依當天流量排序)**

| CEID | HT9045 名稱 | HT160S 名稱 | 語意 | 京元當天發射次數 | 本機是否會發射 | 危險度 | 處置建議 |
|---|---|---|---|---|---|---|---|
| 27 | Change Machine State | One Cycle Finish | 同號不同義 | **406**(另 3 次因停用放棄);host 訂閱 x3(→RPTID 502) | 是(`main.cpp:2786`,由 `csystem.cpp:1695` 呼叫,20260728 新增) | **極高** | 待雙方確認 |
| 1 | Start Pressed | Handler change status | 同號不同義 | 5;host 訂閱 **x7**(綁 12 張報表 501/502/505-509/512-514/800/801);**host S6F15 主動拉取 x3** | 否(註冊但無發射點);但**會回覆 host 的 S6F15 拉取** | **極高** | 待雙方確認 |
| 2 | Pause Pressed | Recipe Change | 同號不同義 | 24;host 未顯式訂閱 | 是(`main.cpp:1500`) | 高 | 待雙方確認 |
| 14 | Switch StartMode | Press Retry | 同號不同義 | 16(另 12 次因停用放棄);host 訂閱 x3 | 是(`note.cpp:685`) | **極高** | 待雙方確認 |
| 28 | Retry Pressed | Clean Out Finish | 同號不同義 | 9;host 未顯式訂閱 | 是(`main.cpp:2776`,由 `csystem.cpp:1643` 呼叫) | 高 | 待雙方確認 |
| 4 | CleanOut Pressed | Press Start (無 IC) | 同號不同義 | 6;host 未顯式訂閱 | 是(`note.cpp:669`) | 高 | 待雙方確認 |
| 21 | Enter IO Page | Switching User Level | 同號不同義 | 3;host 未顯式訂閱 | 是(`main.cpp:1598`) | 高 | host 端改設定 |
| 6 | Lot Start | Press Pause | 同號不同義 | 2;host 未顯式訂閱 | 是(`main.cpp:1947`、`note.cpp:674`) | 高 | 待雙方確認 |
| 15 | Switch Setup File | Press Skip | 同號不同義 | 1;host 訂閱 x3 | 是(`note.cpp:680`) | **極高** | 待雙方確認 |
| 9 | Switch Real Dummy Mode | Press Clean Out | 同號不同義 | 0;host 訂閱 x3 | 是(`main.cpp:1926`、`note.cpp:700`) | **極高** | 待雙方確認 |
| 10 | Switch Tester Online | Press Tray Feed | 同號不同義 | 0;host 訂閱 x3 | 是(`note.cpp:690`、`note.cpp:695`) | **極高** | 待雙方確認 |

**2-2 中 / 低(依號碼排序)**

| CEID | HT9045 名稱 | HT160S 名稱 | 語意 | 京元當天發射次數 | 本機是否會發射 | 危險度 | 處置建議 |
|---|---|---|---|---|---|---|---|
| 3 | OneCycle Pressed | Press Clear Count button | 同號不同義 | 5;未訂閱 | 否(註冊但無發射點) | 低 | 無需處理 |
| 5 | ClearCount Pressed | Press Start (有 IC) | 同號不同義 | 0;未訂閱 | 否(唯一呼叫已註解:`main.cpp:2080`) | 低 | 無需處理 |
| 7 | Lot | Press Home | 同號不同義 | 0;未訂閱 | 是(`main.cpp:1820`) | 中 | 待雙方確認 |
| 8 | Lot End | Press One Cycle | 同號不同義 | 0;未訂閱 | 是(`main.cpp:1897`,在 `OneCycleCore`) | 中 | 待雙方確認 |
| 11 | Switch Production Mode | Press Lot Start | 同號不同義 | 0;未訂閱 | 是(`main.cpp:2390`) | 中 | 待雙方確認 |
| 12 | Switch Engineer Mode | Press Lot End | 同號不同義 | 0;未訂閱 | 是(`main.cpp:2743`) | 中 | 待雙方確認 |
| 13 | Switch Temperature Mode | Press Exit button | 同號不同義 | 0;host 訂閱 x3(→RPTID 502/509) | 否(註冊但無發射點) | 中 | host 端改設定 |
| 16 | Switch UserLevel | Press Alarm Reset | 同號不同義 | 0;未訂閱 | 是(`main.cpp:2158`、`note.cpp:705`) | 中 | 待雙方確認 |
| 17 | Enter Tool Page | Show Alarm | 同號不同義 | 5;未訂閱 | 否(註冊但無發射點) | 低 | 無需處理 |
| 18 | Enter Maintenance Page | Release Alarm | 同號不同義 | 1;未訂閱 | 否(註冊但無發射點) | 低 | 無需處理 |
| 19 | Enter Offset Page | Show Message | 同號不同義 | 0;host 訂閱 x3(→RPTID 502) | 否(註冊但無發射點) | 中 | host 端改設定 |
| 20 | Enter Speed Page | Release Message | 同號不同義 | 1;未訂閱 | 否(註冊但無發射點) | 低 | 無需處理 |
| 22 | Enter Message Page | Enter Setup Page | 同號不同義 | 2;未訂閱 | 否(註冊但無發射點) | 低 | 無需處理 |
| 23 | Enter Debug Page | Enter Maintenance Page | 同號不同義 | 9;未訂閱 | 否(註冊但無發射點) | 低 | 無需處理 |
| 24 | Exit Pressed | Enter I/O Page | 同號不同義 | 0;未訂閱 | 否(唯一呼叫已註解:`maintenance.cpp:1864`) | 低 | 無需處理 |
| 25 | Home Pressed | Enter Teach Page | 同號不同義 | 4;未訂閱 | 否(註冊但無發射點) | 低 | 無需處理 |
| 26 | Get Test Result | Enter SECS GEM Page | 同號不同義 | 12;host 訂閱 x3(→RPTID 501/502/503/508/513) | 否(註冊但無發射點) | 中 | host 端改設定 |
| 29 | Skip Pressed | Tray Feed Finish | 同號不同義 | 0;未訂閱 | 否(註冊但無發射點) | 低 | 本機補發射(見 §3a CEID 49) |
| 30 | Alarm Reset Pressed | Time Event | 同號不同義 | 0;未訂閱 | 否(註冊但無發射點) | 低 | 無需處理 |
| 31 | Tray End Pressed | Switching Real/Dummy Mode | 同號不同義 | 0;未訂閱 | 是(`main.cpp:1712`) | 中 | 待雙方確認 |

**§2 附帶發現(非 CEID 編號問題,但同一段程式碼)**:`note.cpp:695` 的面板 **TRAY END** 鍵發的是 `SECS_EVENT.PressTrayFeed`(CEID 10),與 `note.cpp:690` 的 **TRAY FEED** 鍵同號。HT160S 沒有專屬的 Tray End 事件(9045 有 CEID 31 Tray End Pressed 與 CEID 209 Tray End Finish)。host 因此無法區分兩個按鍵。列為 **待雙方確認**(是否需要 Tray End 專屬事件)。

---

## 3. §3 只有 HT9045 有

僅 9045 有的號碼 = `.def` 全部 292 − HT160S 的 47 = **245** 個。其中:

| 分類 | 個數 |
|---|---|
| (3a-1) 京元當天**實際送出過** | **23** |
| (3a-2) 京元當天**機台想送但被 host 停用** | **3** |
| (3b) 當天完全沒動作 | **219** |
| 合計 | 245 |

### 3a 京元當天實際用過的(逐條列;這才是有用的部分)

「HT160S 等價事件」= HT160S 是否在**別的號碼**上有同一件事(詳細對照見 §5)。

依當天發射次數遞減排列。

| CEID | HT9045 名稱 | 京元當天發射次數 | host 訂閱 | HT160S 等價事件 | 處置建議 |
|---|---|---|---|---|---|
| 44 | Site On Off | 31 | x3(→502/507) | 無(測試機專屬:test site 開關,sorter 無 site 概念) | 無需處理 |
| 70 | Barcode Reader Enter | 13 | x3(→515) | 無同義 CEID。HT160S 的 2D 讀取分兩路:身分盤 2D 走 CEID 275,Lot/單顆 2D 走 WebAPI + Production_Log,皆不發此事件 | 待雙方確認 |
| 76 | Start Pressed HasIC | 13 | **x7**(綁 12 張報表,與 CEID 1 同組) | **有,但從不發射**:HT160S CEID 5 `Press Start (with IC)`,唯一呼叫已註解(`main.cpp:2080`) | **本機補發射**(理由:host 為此號綁了 12 張報表並訂閱 7 次,當天實際觸發 13 次 —— 這是 host 認定的「帶料開始」錨點) |
| 123 | Safe Door On Off | 9 | 未訂閱 | 無 CEID(HT160S 有安全門 sensor 與警報,但不發事件) | 待雙方確認 |
| 124 | Save Recipe | 8 | 未訂閱 | 部分:HT160S CEID 2 `Recipe Change` 是**切換**,9045 124 是**存檔**,不同動作 | 待雙方確認 |
| 73 | Mymessbox OK | 7 | x3(→502) | 有,但從不發射:HT160S CEID 20 `Release Message`(訊息框關閉),無發射點 | 本機補發射(理由:host 訂閱且當天觸發 7 次;HT160S 的 `TMyMessageBox`/`TfNote` 關閉點與 9045 `mymessbox.cpp:368` 對位明確) |
| 67 | Tray Test Finish | 6 | 未訂閱 | 無(測試機專屬:一盤測完,sorter 無測試步驟) | 無需處理 |
| 53 | UPH Record Start | 5 | 未訂閱 | 無 CEID;HT160S 以 SVID 1021(UPH)供 host 主動 poll | 無需處理 |
| 66 | Load Tray Finish | 5 | 未訂閱 | 無同號同義事件(9045 由 `asendic_Loader.cpp:778/802/830` 在 Loader 供盤完成時發;HT160S 只有 Unloadtray 136-147 卸盤側) | 待雙方確認 |
| 41 | One Cycle Finish | 3 | 未訂閱 | **有**:HT160S CEID **27**(見 §5) | 維持 |
| 42 | Clean Out Finish | 2 | **x7**(→501/502/505-509/513) | **有**:HT160S CEID **28**(見 §5) | 維持 |
| 47 | Change HandlerSpeed | 2 | 未訂閱 | 無 CEID(HT160S 速度設定不發事件) | 無需處理 |
| 58 | Ready for ART | 2 | x7(→502/506) | N/A(測試機專屬:ART = Auto Retest Tray,sorter 無重測流程) | 無需處理 |
| 59 | ART Receive Tray OK | 2 | x3(→600) | N/A(測試機專屬,同上) | 無需處理 |
| 34 | Auto Clean Start | 1 | x3(→502/510/521) | N/A(測試機專屬:9045 `AutoClean\` 清針 / socket 清潔子系統,sorter 無測試接點) | 無需處理 |
| 49 | Tray Feed Finish | 1 | **x7**(→501/502/505-509/512-514/900) | **有,但從不發射**:HT160S CEID **29** `Tray Feed Finish`,無發射點(見 §5) | **本機補發射**(理由:host 為此號綁了 11 張報表並訂閱 7 次,是 host 判定手動補盤結束的信號;HT160S 已有 Tray Feed 模式與結束條件,只差發射點) |
| 50 | Auto Clean Finish | 1 | x3(→502/511) | N/A(測試機專屬,同 CEID 34) | 無需處理 |
| 55 | Initial ART Start | 1(另 1 次因停用放棄) | 未訂閱 | N/A(測試機專屬 ART) | 無需處理 |
| 60 | ART Receive Tray START | 1 | 未訂閱 | N/A(測試機專屬 ART) | 無需處理 |
| 63 | FT Finish | 1 | x3(→601) | N/A(測試機專屬:FT = Final Test 完成) | 無需處理 |
| 212 | Energy Saving Start | 1 | x3(→522) | 無。HT160S **沒有節能子系統**;RCMD `ENERGY_SAVING` 已刻意實作為 HCACK=2 拒絕 | 無需處理(需向 host 說明本機無此子系統) |
| 213 | Energy Saving End | 1 | x3(→523) | 同上 | 無需處理(同上) |
| 250 | START Auto contact height | 1 | 未訂閱 | N/A(測試機專屬:contact height 自動量測,sorter 無壓測接點) | 無需處理 |

> **§3a 的「27」為何不在此表**:CEID 27 兩邊都存在(9045 = Change Machine State、HT160S = One Cycle Finish),屬 §2 同號不同義,不屬「僅 9045 有」。它的 406 次是全日最高流量,已列在 §2-1 首列。

### 3a-2 機台想送、但被 host 以 S2F37 停用而放棄

| CEID | HT9045 名稱 | 當天放棄送出次數 | host 訂閱 | HT160S 等價事件 | 處置建議 |
|---|---|---|---|---|---|
| 80 | Read Now Handler Data | **10962** | x3(→502/509/700) | 有,但從不發射:HT160S CEID **30** `Time Event`(9045 此號為每 ~5 秒的週期資料推送) | **維持不發**(host 當天明確用 `S2F37 CEED=0 L,1{80}` 三度關掉它,總計抑制 10962 次;本機沒有這條週期推送反而與 host 現行設定一致)。若日後 host 要求週期推送,對應號為 HT160S 30 |
| 93 | SECS/GEM Online Remote | 3 | 未訂閱 | 無 CEID;HT160S 的上線狀態走 S1F2 / S1F14 / S1F18 + SVID 66002 Control State | 無需處理 |
| 141 | GEM Control State Change | 3 | 未訂閱 | 無 CEID;HT160S 以 SVID **66002** Control State 供 host poll | 待雙方確認 |

> 三筆放棄送出全部落在 host 那 3 輪 provisioning 的「停全部」空窗(13:58:57–13:59:31、14:06:08–14:06:38、16:47:40–16:48:20,各約 30–40 秒),不是 host 針對它們的長期停用。CEID 80 則從 log 第一秒(00:00:02)就在放棄,代表停用設定跨日留存。

### 3b 其餘 219 個(當天完全沒動作)—— 依主題彙總,只給計數

| 主題 | 個數 | 號碼 | 對 HT160S 的意義 |
|---|---|---|---|
| A 保留位 `Reserved_xx`(韌體佔位,無語意) | **56** | 214–249、252–271 | 無需處理 |
| B `.def` 未具名(韌體 enum 有識別字,alias 為空) | **6** | 74、75、79、142、143、144 | 74=`RemoteProgramClose`、75=`ChangeTesterPrgToEQC`、79=`REVERSED79`、142=`PickerCountWasCleared`、143=`UploadPickerCount`、144=`RequestPickerCount`(識別字出自 `uHGemHT9045.h`;`.def` alias 為空 → 對 host 而言**未具名**)。無需處理 |
| C 測試流程(ART / FT / RT / Site Map / 溫控 / Tester program / 測試結果) | **27** | 46、51、52、56、57、61、62、64、65、69、77、81–83、84–90、119–122、128、129 | N/A(測試機專屬:測試流程與 site/溫度/重測,sorter 無對應機構) |
| D E87 Cassette / Carrier / OHT 載具流程 | **27** | 94–109、116–118、130–135、283、284 | N/A(9045 走 E87 cassette/OHT 載具模型;HT160S 的 AMR 交握走 272–275 自有模型) |
| E 9045 材料流程其他(Clean-Out TrayFeed / Map skip / MR mode / Access mode / SoftwareBin / TrayID / PreLoadTray) | **7** | 110–115、140 | 皆無 HT160S 對應號碼;當天未使用 → 待雙方確認(低優先) |
| F Loader / Empty / Color / Auto 站別細粒度事件 | **58** | 154–211 | 9045 為 ASEKH_K1/K3 專案新增的逐站細粒度事件(缺盤 / 滿盤 / 僅一盤 / 放盤 / 上蓋 / 就緒卸盤 / 完成)。HT160S 目前只做粗粒度(272 要料 + 274 完成 + 136-147 退盤)。當天 host 未用任何一個 → 待雙方確認(若京元要細粒度,這 58 個是移植清單的來源) |
| G 新韌體追加(Loader buffer / OutputPort BinCode / TrayID 讀取 / AGV 第二組) | **15** | 276–282、285–292 | **僅存在於 `.def`,V3.32.810 原始碼查無識別字**(見 §0 出處警告)。語意未能從原始碼佐證 → 待雙方確認 |
| H Fix 固定站滿盤 | **6** | 38、39、40、151、152、153 | N/A(機構差異:HT160S 無 Fix 固定站) |
| I SECS 連線 / 診斷事件 | **3** | 91(SECS/GEM Offline)、92(SECS/GEM Online)、251(SECS GEM consecutive failure) | HT160S 無對應 CEID;上線狀態走 S1F2/F14/F18 + SVID 66002 → 無需處理 |
| J 測試機其他(Arm 開關 / 清針計數 / EESUG offset / OTD) | **6** | 45、68、71、72、125、126 | N/A(測試機專屬) |
| K 有 HT160S 對應但號碼不同 | **1** | 32(Tray Feed Pressed)→ HT160S CEID 10 | 見 §5 |
| L HT160S 無對應且當天未使用 | **7** | 33(Reset Pressed)、43(DownLoadRecipe)、48(Change EC)、54(UPH Record End)、78(Jam Skip IC Count)、127(Back To Normal)、139(Visual sort Lot start,SPIL 專案) | 無需處理(48 Change EC 值得留意:HT160S 有 S2F13/S2F15 EC 讀寫但不發 EC-change 事件) |
| **合計** | **219** | | 已逐號驗證此分類為完全分割,無重複、無遺漏 |

---

## 4. §4 只有 HT160S 有

**依「號碼」計:0 個。** HT9045 的 `.def` 在 1–292 是連續佔滿的,HT160S 全部 47 個號碼都落在這個區間內,所以**沒有任何 HT160S 號碼是 9045 字典裡不存在的**。

有意義的問法是**依「語意」**:HT160S 的哪些事件,在 9045 那 286 個具名 CEID 裡**任何號碼都找不到對應**。逐一比對 HT160S 1–31 後,答案是 4 個:

| CEID | HT160S 名稱 | 語意 | 9045 是否有任何等價號碼 | 已註冊? | 有東西會發射? | 客戶規格書是否收錄 | 處置建議 |
|---|---|---|---|---|---|---|---|
| 17 | Show Alarm | 僅 HT160S | 無。9045 警報一律走 S5F1 ALCD/ALID,CEID 89 `Pre Alarm Message` 是「預警位置」不同語意 | 是(`AddCEID` 迴圈) | 否(無發射點) | 是(§3.3 表列) | 無需處理(HT160S 亦以 S5F1 送警報,此 CEID 為冗餘) |
| 18 | Release Alarm | 僅 HT160S | 無(同上,9045 以 S5F1 ALCD bit7=0 表解除) | 是 | 否(無發射點) | 是(§3.3) | 無需處理(同上) |
| 19 | Show Message | 僅 HT160S | 無「訊息顯示」事件。9045 只有關閉側:CEID 73 `Mymessbox OK`、109 `Die Count Fail Message Close` | 是 | 否(無發射點) | 是(§3.3) | 待雙方確認(若 host 要訊息顯示通知才需補;host 當天訂閱的是 CEID 19 → 9045 語意 `Enter Offset Page`,與此無關) |
| 26 | Enter SECS GEM Page | 僅 HT160S | 無。9045 的 Enter-page 事件為 17/18/19/20/21/22/23,沒有 SECS 頁 | 是 | 否(無發射點) | 是(§3.3) | 無需處理(純操作員 UI 導航) |

**近似而非「僅 HT160S」的四個,已改列 §5**:CEID 2 `Recipe Change`(9045 15 Switch Setup File / 124 Save Recipe)、20 `Release Message`(9045 73)、22 `Enter Setup Page`(9045 17 Enter Tool Page)、25 `Enter Teach Page`(9045 19 Enter Offset Page)、30 `Time Event`(9045 80)。

---

## 5. §5 「同一件事、不同號碼」對照 —— host 整合最需要的一張表

每一列都對兩邊原始資料驗證過。最後兩欄是關鍵:**同一個號碼在對面機台是完全不同的事**,所以「照 9045 字典設定的 host 收到 HT160S 的號」與「照 HT160S 字典設定的 host 收到 9045 的號」**兩個方向都會錯**。

| # | 事件(同一件事) | HT9045 CEID | HT160S CEID | 京元當天 9045 側次數 | HT160S 是否發射 | ⚠ HT160S 這個號在 9045 是什麼 | ⚠ 9045 這個號在 HT160S 是什麼 |
|---|---|---|---|---|---|---|---|
| 1 | **機台狀態變更** | **27** | **1** | **406** | 否(僅回應 S6F15 拉取) | 1 = Start Pressed | 27 = One Cycle Finish |
| 2 | **One Cycle Finish** | **41** | **27** | 3 | 是(`main.cpp:2786`) | 27 = Change Machine State | 41 = 未註冊 / 不存在 |
| 3 | **Clean Out Finish** | **42** | **28** | 2(host 訂閱 x7) | 是(`main.cpp:2776`) | 28 = Retry Pressed | 42 = 未註冊 / 不存在 |
| 4 | **Tray Feed Finish** | **49** | **29** | 1(host 訂閱 x7) | **否(無發射點)** | 29 = Skip Pressed | 49 = 未註冊 / 不存在 |
| 5 | Start 按下(機內無 IC) | 1 | 4 | 5 | 是(`note.cpp:669`) | 4 = CleanOut Pressed | 1 = Handler change status |
| 6 | Start 按下(機內有 IC) | 76 | 5 | 13(host 訂閱 x7) | **否(呼叫已註解)** | 5 = ClearCount Pressed | 76 = 未註冊 / 不存在 |
| 7 | Pause 按下 | 2 | 6 | 24 | 是(`main.cpp:1947`) | 6 = Lot Start | 2 = Recipe Change |
| 8 | One Cycle 按下 | 3 | 8 | 5 | 是(`main.cpp:1897`) | 8 = Lot End | 3 = Press Clear Count |
| 9 | Clean Out 按下 | 4 | 9 | 6 | 是(`main.cpp:1926`) | 9 = Switch Real Dummy Mode | 4 = Press Start (無 IC) |
| 10 | Clear Count 按下 | 5 | 3 | 0 | 否(無發射點) | 3 = OneCycle Pressed | 5 = Press Start (有 IC) |
| 11 | Lot Start | 6 | 11 | 2 | 是(`main.cpp:2390`) | 11 = Switch Production Mode | 6 = Press Pause |
| 12 | Lot End | 8 | 12 | 0 | 是(`main.cpp:2743`) | 12 = Switch Engineer Mode | 8 = Press One Cycle |
| 13 | Tray Feed 按下 | 32 | 10 | 0 | 是(`note.cpp:690`) | 10 = Switch Tester Online | 32 = 未註冊 / 不存在 |
| 14 | Home 按下 | 25 | 7 | 4 | 是(`main.cpp:1820`) | 7 = Lot | 25 = Enter Teach Page |
| 15 | Exit 按下 | 24 | 13 | 0 | 否(無發射點) | 13 = Switch Temperature Mode | 24 = Enter I/O Page |
| 16 | Retry 按下 | 28 | 14 | 9 | 是(`note.cpp:685`) | 14 = Switch StartMode | 28 = Clean Out Finish |
| 17 | Skip 按下 | 29 | 15 | 0 | 是(`note.cpp:680`) | 15 = Switch Setup File | 29 = Tray Feed Finish |
| 18 | Alarm Reset 按下 | 30 | 16 | 0 | 是(`main.cpp:2158`) | 16 = Switch UserLevel | 30 = Time Event |
| 19 | Real / Dummy 切換 | 9 | 31 | 0(host 訂閱 x3) | 是(`main.cpp:1712`) | 31 = Tray End Pressed | 9 = Press Clean Out |
| 20 | User Level 切換 | 16 | 21 | 0 | 是(`main.cpp:1598`) | 21 = Enter IO Page | 16 = Press Alarm Reset |
| 21 | Recipe / 工作檔切換 | 15 | 2 | 1(host 訂閱 x3) | 是(`main.cpp:1500`) | 2 = Pause Pressed | 15 = Press Skip |
| 22 | 進入 Tool / Setup 頁 | 17 | 22 | 5 | 否(無發射點) | 22 = Enter Message Page | 17 = Show Alarm |
| 23 | 進入 Maintenance 頁 | 18 | 23 | 1 | 否(無發射點) | 23 = Enter Debug Page | 18 = Release Alarm |
| 24 | 進入 Offset / Teach 頁 | 19 | 25 | 0(host 訂閱 x3) | 否(無發射點) | 25 = Home Pressed | 19 = Show Message |
| 25 | 進入 I/O 頁 | 21 | 24 | 3 | 否(呼叫已註解) | 24 = Exit Pressed | 21 = Switching User Level |
| 26 | 訊息框關閉 | 73 | 20 | 7(host 訂閱 x3) | 否(無發射點) | 20 = Enter Speed Page | 73 = 未註冊 / 不存在 |
| 27 | 週期資料推送(time event) | 80 | 30 | 0 送出(**10962 次被 host 停用**)(host 訂閱 x3) | 否(無發射點) | 30 = Alarm Reset Pressed | 80 = 未註冊 / 不存在 |

**這張表最該先看的四列**

- **第 1 列(27 ↔ 1)**:當天最高流量事件。9045 用 27 送機台狀態 406 次;HT160S 的 27 是 One Cycle Finish、而且**現在會發**。同時 host 對 CEID 1 綁了 12 張報表、訂閱 7 次、還用 S6F15 拉了 3 次 —— 而 HT160S 的 CEID 1(Handler change status)**從來不主動發**。等於「host 最在意的狀態流」在 HT160S 上既錯號又沒有。
- **第 2、3 列(41→27、42→28)**:HT160S 兩個「完成」事件都已在發,只是號碼落在 9045 的 27/28(Change Machine State / Retry Pressed)上。
- **第 4、6 列(49→29、76→5)**:host 分別綁了 11 張與 12 張報表、各訂閱 7 次,HT160S 的對應號**已註冊但無發射點**。這兩條是 §6 建議「本機補發射」的主體。
- **第 27 列(80 ↔ 30)**:唯一「本機不做反而對」的一列 —— host 當天用 S2F37 三度明確關掉 CEID 80,共抑制 10962 次。

---

## 6. §6 風險與決策

### 6-1 碰撞暴露量(量化)

| 指標 | 數值 | 依據 |
|---|---|---|
| 兩邊都佔用、但語意不同的號碼 | **31**(CEID 1–31 全部) | §2,逐號比對 |
| 其中 HT160S **會實際上線發射**的 | **16**(2、4、6、7、8、9、10、11、12、14、15、16、21、27、28、31) | 全樹 `EventReport(` grep |
| 其中 host 當天**明確訂閱或拉取**的 | **9**(1、9、10、13、14、15、19、26、27) | S2F35 x122 條連結 + S6F15 x3 逐筆解析 |
| **兩者交集 = 保證誤讀** | **6**(9、10、14、15、27,加上以 S6F15 被拉取的 1) | 交集 |
| 當天流量最高的碰撞 | **CEID 27 = 406 次** | S6F11 逐筆計數 |
| host 為單一碰撞號綁的最多報表數 | **CEID 1 = 12 張**(RPTID 501/502/505/506/507/508/509/512/513/514/800/801) | S2F35 逐筆解析 |
| 一致(安全)的號碼 | **16**(272–275、35/36/37/148/149/150、136-138/145-147) | §1 |
| 僅 9045 有、且當天真的用過 | **26**(23 送出 + 3 被停用) | §3a |
| 僅 HT160S 有(依號碼) | **0** | 9045 `.def` 1–292 連續佔滿 |

**Path A 裁定的一項更新**:`secs-pathA-3way-crossverify-20260727.md` 把 CEID 層全部判為 **discovery-gated**,理由是「host 以 S2F35 綁定自己的報表、不吃設備端的 CEID 編號」。本文的現場 log 已把這個 discovery **做完了**:host 確實用 S2F35,但它是**綁到具體的 CEID 號碼**(34 個,含低位 1/9/10/13/14/15/19/26/27)。所以「host 不吃 CEID 編號」這半句需要修正為:**host 吃的正是 CEID 編號**,只是報表內容由它自己定義。碰撞因此從「discovery-gated」升級為「條件成立即發生」。

**唯一還未確定的條件**:這份 S2F35 是 KYEC host 對著一台 **HT9045** 講話、載入 9045 字典時的 provisioning。同一個 host 指向 HT160S 時是否沿用同一份 CEID profile,取決於京元是否 per-machine-type 分開設定 —— 這一點**只有客戶能回答**,無法從 log 推導。若沿用,上表那 6 個「保證誤讀」就會實際發生;若另建 HT160S profile,則全部降為文件面。

### 6-2 S6F15 是 20260728 起的新暴露面(必須讓 host 知道)

- HT160S 已實作 **S6F15 → S6F16**(`uHGemHT160.cpp` `S6F16_EventReportData` + `uHGemEquipment.cpp` `EmitEventReportBody`)。這是 20260728 才有的能力。
- 京元 host 當天對 9045 發過 **3 次 S6F15,全部是 CEID = 1**(本體為裸 `<U4[1] 1>`,非 `L,1` 包裝;三次皆同)。
- 因此:**同一個 host 對 HT160S 拉 CEID 1,會拿到「Handler change status」的資料,而不是它預期的「Start Pressed」**。以前 HT160S 對 S6F15 沒有回覆(host T3 逾時),host 拿不到任何東西;現在會拿到**語意錯誤但格式合法**的回覆。從「沉默」變成「答錯」,對 host 端的可偵測性反而**變差**。
- 一個減輕因素、不是解法:回覆內容取決於該 CEID 當下連結到哪些報表。若 host **還沒**對 HT160S 下 S2F33/S2F35,CEID 1 帶的是 HT160S 預設 Report **1**(13 個 SVID:1001/1003/1021/1027 + 66000–66031),RPTID 與 host 期待的 501/502/… 完全不同 —— 嚴格的 host 會當成格式異常而不是把它當 Start Pressed 吃下去。但若 host **已經**照 9045 profile 對 HT160S 下完 S2F33 + S2F35(HT160S 的容忍策略會接受未知 SVID/CEID,見規格書 §4),那 S6F15 CEID 1 的回覆就會是 **host 自己的 RPTID 外殼 + HT160S 的值(未知 SVID 回空)**,此時「答錯」完全隱形。

### 6-3 重申:重編號是商務決定,不是工程決定

- **本文不建議、也不主張單方面重編 CEID。** 標準決策維持不變:客戶規格書 `docs\SECS\HT160S_SECS_Interface_Spec_20260727.md` §3.3 所宣告的 HT160S 語意為**權威**;§4/§5 已明文寫出「同號不同義」並要求 host 依 §3.3 對應。
- 本文的作用是把風險**量化**(6-1 的 6 個保證誤讀、CEID 27 的 406 次/日、CEID 1 的 12 張報表)與把 host 端該改什麼**具體化**(§5 那 27 對號碼),讓雙方在同一份數字上談。
- 可行的三條路,由客戶選:(a) host 端為 HT160S 另建 CEID profile(§5 即為對照表,**本機零改動**);(b) 雙方協議一份號碼對照,由 HT160S 改號(**需客戶書面確認,且必須整段改、不可孤立改單一號**);(c) 維持現狀並接受 §2 標為「極高/高」的號碼在 host 側被誤讀。

### 6-4 唯一有證據支撐的「本機補發射」清單(共 4 條)

只列 host **當天真的發過或訂閱過**、且 HT160S **已註冊卻無發射點**的號碼。不含「為求完整」的項目。

| HT160S CEID | 事件 | 對應 9045 CEID | 證據 | 建議 |
|---|---|---|---|---|
| **1** | Handler change status | 27 | 9045 側 **406 次/日**(最高流量);host **訂閱 x7 + 綁 12 張報表 + S6F15 拉取 x3**;HT160S 已註冊、零發射點;9045 發射條件明確(`main.cpp:18486`:`palMainStatus->Caption` 變更即發) | **本機補發射**(優先度最高) |
| **5** | Press Start (with IC) | 76 | host **訂閱 x7 + 綁 12 張報表**;9045 側當天 13 次;HT160S 唯一呼叫被註解在 `main.cpp:2080` | **本機補發射** |
| **29** | Tray Feed Finish | 49 | host **訂閱 x7 + 綁 11 張報表**;9045 側當天 1 次;HT160S 已有 Tray Feed 模式與結束條件,只差發射點 | **本機補發射** |
| **20** | Release Message | 73 | host **訂閱 x3**;9045 側當天 7 次,發射點對位明確(`mymessbox.cpp:368`,HT160S 有 `TMyMessageBox` / `TfNote` 對應關閉點) | **本機補發射**(優先度最低,純通知性) |

> 補發射本身**不會**改變任何碰撞:補了 CEID 1 之後,一個照 9045 字典設定的 host 收到 HT160S 的 CEID 1 仍會讀成「Start Pressed」。補發射解決的是**資料缺口**(host 訂了卻永遠收不到),碰撞要靠 6-3 的商務決定解決。兩件事必須分開談,不要混為一個工單。

### 6-5 純文件面的兩項待辦

1. 客戶規格書 §3.3 **補列 CEID 136/137/138/145/146/147**(機台會發、規格書沒寫;見 §1 註記 2)。
2. `uHGemHT160.cpp:299` 的註解仍寫 `136-138/140-142`,與 `bf9d048` 改好的 `145/146/147` 不一致 —— 純註解陳舊,下次動該檔時順手修正即可(本文不改任何原始碼)。
