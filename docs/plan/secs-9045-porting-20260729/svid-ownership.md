# SVID 分責表 - 京元 host 引用 vs HT160S 供應 (2026-07-29)

## 0. 來源與方法 (Provenance)

**建立日期**:2026-07-29

**主要來源(全部唯讀)**

| 用途 | 絕對路徑 |
|---|---|
| 京元現場 log(2026-06-08 全日,20 檔,雙向完整 SML body) | `D:\backup_version\HT9046\KYEC\20260626\2026_06_08\SECSGEM_TextLog_00..19.txt` |
| HT9045 SVID 登錄表 | `D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\SECSGEM\uHGemHT9045_SV.cpp` |
| HT9045 ECID 登錄表 | `D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\SECSGEM\uHGemHT9045_EC.cpp` |
| HT9045 GEM 標準 SV / EC→SV 雙註冊框架 | `D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\SECSGEM\uHGemEquipment.cpp`、`...\SECSGEM.cpp` |
| HT160S SV / EC 註冊 | `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemHT160.cpp`(`AddSV` / `AddEC` / `AddReprot`) |
| HT160S S1F4 / S2F16 / S2F33 行為 | `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemEquipment.cpp`、`...\uHGemHT160.cpp` |
| HT160S AGV 站台表(SVID 38202-38245) | `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uAgvStation.cpp`(表在 line 102-113)、`...\uAgvStation.h` |
| HT160S 客戶規格書(對客戶為權威) | `D:\HT160S_BCB\docs\SECS\HT160S_SECS_Interface_Spec_20260727.md` |
| 前次比對(僅作提示,本文全部重算) | `D:\HT160S_BCB\docs\plan\secs-9045kyec-diff-20260728\`、`D:\HT160S_BCB\docs\plan\secs-9045kyec-vs-160-cmd-diff-20260728.md` |
| Path-A 三方交叉裁定 | `D:\HT160S_BCB\docs\plan\secs-pathA-3way-crossverify-20260727.md` |

**方法註**:「host 需求集」100% 來自 **field log**(逐筆解析 S2F33 報表定義與 S1F3 輪詢的 SML body,不採用 9045 登錄表推測);「號碼的名稱與意義」來自 **9045 原始碼登錄表**;「HT160S 現況與可供應性」來自 **HT160S 原始碼**(每一列 B 類都附實際變數與 `檔案:行號`)。本文所有數字均由本次重新解析產生,未沿用前次報告。

**已納入未提交工作樹**:`git diff` 已讀。S6F15/F16、S6F19/F20、S10F3/F4、S10F5/F6、S125F1/F2、RCMD `ONE_CYCLE`/`ENERGY_SAVING`(刻意 HCACK=2 拒絕)/`PP_SIGNALTOWER`/`PP_MUSIC`、CEID 145/146/147 renumber、`START_AGV` Action/NA + LoaderICCount 均**已實作**,本文不列為缺口。`AddSV` / `AddEC` 在未提交差異中**沒有**新增任何 SV/EC,故供應集仍為 51 SV + 7 EC。

---

## 1. 重算後的量體 (Recount)

### 1.1 host 需求集(來自 log)

| 項目 | 本次重算 | 說明 |
|---|---|---|
| log 交易區塊總數 | 17,067 | 含 Linktest 等非 SxFy 區塊 |
| 收到 S2F33 Define Report | 122 筆 | 其中 4 筆為 `L,0` 全刪 |
| S2F33 定義出的相異 RPTID | **37** | 501-523、600、601、700、800、801、804、900、994-999、2000、2001 |
| S2F33 引用的相異 SVID | **437** | (前次報告的 445 實為聯集,見下) |
| 收到 S1F3 Selected Status Request | 74 筆 | |
| S1F3 直接輪詢的相異 SVID | **50** | |
| 兩路徑重疊 | 42 | |
| **host 需求集(聯集)** | **445** | 本表主表列數 |
| 全日 RPTID 槽位總數(取各 RPTID 最後一次定義) | **580** | 同一 SVID 出現在多個報表會重複計 |

### 1.2 HT160S 供應集(來自原始碼)

`HT160Gem::AddSV()`(`uHGemHT160.cpp:88-165`)共註冊 **51 個 SV**:

- 靜態 18 個:1001、1003、1021、1027、1518、38219、38220、38221、66000、66001、66002、66010、66011、66020、66021、66030、66031、66032
- 站台迴圈 27 個(9 站 × Carrier ID / Tray Count / Device Count):38202-38210、38222-38233、38237-38242
- 站台迴圈 Bin Setting 6 個(僅 `SvidBinSet!=0 && AutoIndex>=0`):38234-38236、38243-38245

`HT160Gem::AddEC()`(`uHGemHT160.cpp:221-263`)共註冊 **7 個 EC**:1501、2758、2759、2760、2761、2762、2763。

### 1.3 為什麼 445 個號碼「全部查得名稱」(E 類 = 0)

HT9045 的 `THGem::SetECDataPointer()` 在函式尾端**再呼叫一次** `SetSVDataPointer(ECID, ...)`(`uHGemEquipment.cpp:5951`、`5990`、`6165`、`6207`),因此 **每一個 ECID 同時佔用同號的 SVID**。重算結果:

| 登錄來源 | 首次註冊的號碼數 |
|---|---|
| `uHGemHT9045_SV.cpp`(866 個呼叫行,去註解/重號後) | 848 |
| `uHGemHT9045_EC.cpp`(1,649 個相異 ECID,經 EC→SV 雙註冊) | 1,647 |
| `SECSGEM.cpp` | 196 |
| `uHGemEquipment.cpp`(GEM 標準 SV) | 22 |
| **HT9045 SV 命名空間可讀號碼總數** | **2,713** |

**445/445 在 9045 側都查到名稱**,所以本表 **E 類(未具名/未知)為 0**。唯一無法查名的號碼是 ECID **8506**(只出現在 S125F1 清單,兩份登錄表都沒有),列在 §ext-2。

> **關鍵不對稱**:HT160S 的 `THGem::SetECDataPointer()`(`uHGemEquipment.cpp:807-827`)**不會**回頭註冊 SV。這造成 §2 中一整批「資料在 HT160S 已存在、只是註冊在 EC 命名空間」的 B 類缺口(1501、2758-2763)。

### 1.4 覆蓋率(現況)

| 量測基準 | 現況 | 補完 B 類 19 個後 |
|---|---|---|
| host 需求集號碼覆蓋 | 26 / 445 = **5.8%** | 45 / 445 = **10.1%** |
| RPTID 槽位覆蓋(580 槽) | 25 / 580 = **4.3%** | 55 / 580 = **9.5%** |
| **依實際上線流量加權的槽位覆蓋**(每 RPTID 槽數 × 當天被 S6F11 攜帶次數) | 657 / 12,010 = **5.5%** | 3,211 / 12,010 = **26.7%** |

第三列是唯一有現場意義的數字:host 定義了 179 個 SVID 的 RPTID 505,但它一天只被搬 21 次;只有 8 個 SVID 的 RPTID 502 被搬 **510 次**(佔全日 645 次 S6F11 的 79.1%)。

---

## 2. 主表 - 逐 SVID 分責

**分類定義**

| 分類 | 含意 |
|---|---|
| **A 已供應** | HT160S 已 `AddSV` 註冊且回真值 |
| **B 可供應(資料已存在)** | 資料在 HT160S 確實存在(本表每列附變數 + `檔案:行號`),只是沒註冊成 SV。**這是工作清單** |
| **C 無對應** | HT160S 無資料源。絕大多數為**測試機專屬**(溫度/site/測試結果/ART/MRT/yield/清針/接觸力);少數(節能、MTBF/平均UPH、周邊模組)**非測試流程但 HT160S 未建置**,已在 §2.4 逐主題註明理由,不混為一談 |
| **D 語意需確認** | 兩邊都有「相近」的東西但語意不同,直接回值會被 host 誤讀 |
| **E 未知** | 名稱不在任何主要來源。**本表為 0**(理由見 §1.3) |

**分類統計**:A 26 / B 19 / C 375 / D 25 / E 0 = **445**

---

### 2.1 B 類 - 工作清單(19 項,責任在 HT160S)

| SVID | 9045 名稱/意義 | 所屬 RPTID | host 取用方式 | HT160S 現況 | 分類 | 責任方 | 說明 |
|---|---|---|---|---|---|---|---|
| **1011** | Machine State(機台狀態文字) | **502** | S2F33 **+ S1F3 ×25(全日最高頻)** | 未註冊。`fMain->palMainStatus`(`main.h:141`)、`MachineRun.szRunStatus`(`cprod.h:202`)、`MachineRun.iSystemStatus`(`cprod.h:195`)皆有 | B | HT160S | **本表第一優先**。它同時是全日最常被輪詢的單一 SVID(25 次)且落在流量佔 79.1% 的 RPTID 502。現況 host 拿到空 item,等於整天看不到機台狀態 |
| **3** | GemClock(GEM 標準:設備當前時間) | **502** | S2F33 | 未註冊 SVID 3;**同一份資料已以 SVID 1027 供應**(`sSystemTime`,`uHGemHT160.h:50`,註冊於 `uHGemHT160.cpp:111`) | B | HT160S | 零新資料源:多綁一個號指向同一 `sSystemTime` 即可。GEM 標準號,host 端幾乎不會改 |
| **1501** | Setup File(recipe 名) | **502**、518、519 | S2F33 **+ S1F3 ×1 + S125F1 啟用** | **已註冊,但註冊在 EC 命名空間**:`SetECDataPointer(1501, ..., &ecRecipeName)`(`uHGemHT160.cpp:244`;`ecRecipeName` 宣告於 `uHGemHT160.h:68`) | B | HT160S | HT160S 的 EC 不兼作 SV(§1.3),所以 S1F3/S6F11 讀 1501 回空。補一行 `SetSVDataPointer(1501, ..., &ecRecipeName)` |
| **1006** | 9045 內部**雙重註冊**:SV=`Site Ag Socket ID`(`SECSGEM.cpp:725`)/ EC=`Lot ID`(`uHGemHT9045_EC.cpp:57`) | **502** | S2F33(讀)**+ S2F15 ×3 寫入 `"LQ50SIJAG2"`** | 未註冊。批號資料存在:`svCurrentLot`(SVID 66031,`uHGemHT160.cpp:137`)、`LotRegistry`(`CosFunction.h:244`) | B | **雙方確認** | host 當天寫進去的值是批號 → 實務上採 **EC 的 Lot ID 語意**。但 9045 的 `SetSVDataPointer` 對重號直接拒絕(`uHGemEquipment.cpp:5833-5839`),誰先註冊誰得到 SV 槽,無法由原始碼靜態斷定。**必須先與京元確認 HT160S 的 1006 要回批號**再實作 |
| **1002** | Machine ID | 518、519 | S2F33 + S1F3 ×3 | 未註冊。`GeneralSetting.sHandlerID`(`GeneralSetting.h:208`)已有;另有 `sSerialNo`(`:209`)、`sMachineModel`(`:207`) | B | HT160S | 註:HT160S 的 SVID 1001 目前綁 `HandlerPath`,而它是建構時傳入的**常數字串** `"HT160S"`(`ht160s.cpp:269` → `HT160Gem::HT160Gem(Path, ...)` `uHGemHT160.cpp:28` → `HTGem(Path,...)` `uHGemClass.cpp:26-30`),不是 `GeneralSetting.sMachineModel`(`GeneralSetting.h:207`)。補 1002 時建議一併檢視 1001 的資料源 |
| **1009** | Lot Start Time | 508 | S2F33 | 未註冊。`tRunData.StartTime`(`cprod.h:11`);per-lot 另有 `TLotRunInfo.dtFirstSeen`(`CosFunction.h:209`) | B | HT160S | HT160S 一台機同時掛多個 Lot,需先決定 1009 回「機台本次 run 起始」還是「第 0 個 Lot 起始」。建議回 `tRunData.StartTime`(機台級),與 66030/66031 的既有語意一致 |
| **1101** | Loader Count | 501 | S2F33 | 未註冊。`tRunData.LoaderIC`(`cprod.h:18`) | B | HT160S | RPTID 501 掛在 CEID 1/26/42/49/76,全日被搬 33 次 |
| **1102** | Output Total Count | 501 | S2F33 | **同一份資料已以 SVID 66021 供應**:`MachineRun.iTotalSorted`(`cprod.h:198`,註冊於 `uHGemHT160.cpp:134`) | B | HT160S | 純換號別名。若不補,host 的 RPTID 501 產出欄永遠空,而 HT160S 自有的 66021 host 沒問 |
| **1103** | Auto1 Count | 501 | S2F33 | 未註冊。`MachineRun.iAreaCount[eAuto1]`(`cprod.h:201`;enum `cmydef.h:71`) | B | HT160S | |
| **1104** | Auto2 Count | 501 | S2F33 | 未註冊。`MachineRun.iAreaCount[eAuto2]`(`cprod.h:201`;`cmydef.h:72`) | B | HT160S | |
| **1105** | Auto3 Count | 501 | S2F33 | 未註冊。`MachineRun.iAreaCount[eAuto3]`(`cprod.h:201`;`cmydef.h:73`) | B | HT160S | HT160S 另有 Auto4-6(`cmydef.h:74-76`),9045 那三個號叫 Fix1-3,見 §2.2 |
| **2762** | Type 1 Tray Division X | 800、505 | S2F33 **+ S1F3 ×23(每 5 秒輪詢)** | 註冊在 EC 命名空間:`SetECDataPointer(2762, ..., &TrayForm.XDivision)`(`uHGemHT160.cpp:255`;`TrayForm.XDivision` 在 `CosFunction.h:61`) | B | HT160S | **第二熱路徑**。host 以 5 秒週期輪詢 2734/2762/2763/2775/2776/2788/2789 這組,全日各 23 次;現況 7 個全回空 item |
| **2763** | Type 1 Tray Division Y | 800、505 | S2F33 **+ S1F3 ×23** | `SetECDataPointer(2763, ..., &TrayForm.YDivision)`(`uHGemHT160.cpp:256`;`CosFunction.h:62`) | B | HT160S | 同上 |
| **2758** | Type 1 Tray Pitch X | 800、505 | S2F33 | `SetECDataPointer(2758, ..., &TrayForm.XPitch)`(`uHGemHT160.cpp:252`;`CosFunction.h:58`) | B | HT160S | |
| **2759** | Type 1 Tray Pitch Y | 800、505 | S2F33 | `SetECDataPointer(2759, ..., &TrayForm.YPitch)`(`uHGemHT160.cpp:254`;`CosFunction.h:60`) | B | HT160S | |
| **2760** | Type 1 Tray Start Position X | 800、505 | S2F33 | `SetECDataPointer(2760, ..., &TrayForm.XStart)`(`uHGemHT160.cpp:251`;`CosFunction.h:57`) | B | HT160S | |
| **2761** | Type 1 Tray Start Position Y | 800、505 | S2F33 | `SetECDataPointer(2761, ..., &TrayForm.YStart)`(`uHGemHT160.cpp:253`;`CosFunction.h:59`) | B | HT160S | |
| **3616** | Error Bin Tray Select | **506**、800 | S2F33 + S125F1 啟用 | 未註冊。`THT160BinAreaMap::ErrorBinArea`(`CosFunction.h:92`),讀取器 `GetErrorBinArea()`(`CosFunction.h:118`) | B | HT160S | RPTID 506 全日被搬 23 次,且被 host 中途重定義(見 §ext-1) |
| **3617** | Bin 0-255 Tray Select | **506**、800 | S2F33 | 未註冊。`THT160BinAreaMap::BinToArea[]`(`CosFunction.h:89`),讀取器 `GetAreaByBin(int Bin)`(`CosFunction.h:107`) | B | HT160S | 9045 是「bin→tray select」字串;HT160S 是「bin→Auto 區」整數表。序列化格式需與京元對齊字串編碼(逐 bin 以逗號或定寬),但**資料本身已存在** |

**B 類實作代價**:19 個號碼中,**8 個(1102、3、1501、2758-2763)不需要新資料源**,只是把既有變數再綁一個號;其餘 11 個綁的是已存在的機台變數。沒有任何一列需要新增機構量測或新子系統。

---

### 2.2 D 類 - 語意需確認(25 項,不可單方面實作)

| SVID | 9045 名稱/意義 | 所屬 RPTID | host 取用方式 | HT160S 現況 | 分類 | 責任方 | 說明 |
|---|---|---|---|---|---|---|---|
| **1517** | Start Mode For HT9045(`&LastSet.iRunStartMode`) | 800、**502** | S2F33 + S1F3 ×3 | HT160S 有 `HSys.Sys.RunMode`,已以 SVID **66000** 供應(0=Normal 1=Home 2=OneCycle 3=CleanOut 4=TrayFeed,`uHGemHT160.cpp:120`) | D | 雙方確認 | 兩邊都叫「模式」但值域完全不同(當天 host 讀回 `I4 1`)。若直接把 66000 的值回在 1517,host 會把 `1` 解讀成 9045 的啟動模式 1。這是落在 RPTID 502 的第 6 個欄位,**確認後即可補**,價值高 |
| **35816** | [N07-2] Enable Host Control Start(`&IniConfig.bRCMDStart`) | — | **S1F3 ×3(每次建線都問)** | 無對應開關:HT160S 在 REMOTE 下對 RCMD `START` 一律受理(`uHGemHT160.cpp` S2F42 鏈) | D | 雙方確認 | host 拿這個旗標決定「能不能遠端下 START」。回空 item 有可能讓 host 判定不可遠端啟動。**行為上等效於常數 1**,但要不要用常數回答、以及 host 收到空值時的實際行為,需向京元求證(log 內看不到 host 因此放棄 START) |
| **1106** | Fix1 Count | 501 | S2F33 | HT160S **無 Fix 站**(只有 Auto1-6 + Color,`cmydef.h:68-79`)。最接近的資料是 `MachineRun.iAreaCount[eAuto4]`(`cprod.h:201`;`cmydef.h:74`) | D | 雙方確認 | 把 9045 的 Fix1-3 對映到 HT160S 的 Auto4-6 是**商務決定**,不是工程決定。工程上兩者都是「第 4/5/6 個輸出站」,語意上 Fix 在 9045 是固定 bin |
| **1107** | Fix2 Count | 501 | S2F33 | `MachineRun.iAreaCount[eAuto5]`(`cmydef.h:75`) | D | 雙方確認 | 同上 |
| **1108** | Fix3 Count | 501 | S2F33 | `MachineRun.iAreaCount[eAuto6]`(`cmydef.h:76`) | D | 雙方確認 | 同上 |
| **2734** | Loader Tray Type Index(`&TrayForm.Loader.iTrayType`) | — | **S1F3 ×23** | HT160S 只有單一 tray 幾何(`THT160TrayForm`,`CosFunction.h:54-70`),**沒有** tray 型別 index 欄位 | D | 雙方確認 | 5 秒熱路徑但回不出東西。要嘛 host 停問,要嘛 HT160S 約定固定回 0/1 代表「唯一型別」 |
| **2775** | Type 2 Tray Division X | — | **S1F3 ×23** | 未註冊。`uHGemHT160.cpp:257-262` 已明文把 2771-2776 / 2784-2789 標為**保留、尚無資料源** | D | 雙方確認 | HT160S 只有 Type 1。決策點:(a) Type2/3 回 Type1 同值,(b) host 只問 2762/2763。選 (a) 會讓 host 以為機台支援三種 tray |
| **2776** | Type 2 Tray Division Y | — | **S1F3 ×23** | 同上(`uHGemHT160.cpp:257-262`) | D | 雙方確認 | 同上 |
| **2788** | Type 3 Tray Division X | — | **S1F3 ×23** | 同上 | D | 雙方確認 | 同上 |
| **2789** | Type 3 Tray Division Y | — | **S1F3 ×23** | 同上 | D | 雙方確認 | 同上 |
| **2701** | Tray Type(`&TrayForm.LodareType`) | 800、505 | S2F33 | HT160S 無 tray 型別欄位(同 2734) | D | 雙方確認 | |
| **3656** | Bin 0-255 Type | **506**、800 | S2F33 | HT160S 只有單一 `PassBin`(`THT160BinAreaMap::PassBin`,`CosFunction.h:93`)+ 分類器 `GetPassFailClass(int Bin)`(`CosFunction.h:130`),**沒有** per-bin type 字串 | D | 雙方確認 | 若把 `GetPassFailClass()` 逐 bin 展開成字串,語意是 PASS/FAIL/none 三態,不等於 9045 的 bin type 值域 |
| **3677** | Tray for Fail Bin | **506**、800 | S2F33 | 無 per-bin tray 型別(同 2701) | D | 雙方確認 | |
| **37007** | LoaderTotalTray(`&LastSet.iLoaderTotalTray`) | 600 | S2F33 **+ S2F15 ×1 寫入 `U4 1`** | 同概念資料已存在且已供應:`AgvCoord.TrayCount[0]` = SVID **38222**(`uAgvStation.h:56-61`,由 `START_AGV` 的 `LoaderTrayCount` 帶入) | D | 雙方確認 | 一個概念兩個號。要不要開 37007 當 38222 的別名(以及 37007 是否該可寫)需京元裁定 |
| **37006** | Input Loader Count | 600 | S2F33 | `tRunData.LoaderIC`(`cprod.h:18`)與 1101 同源 | D | 雙方確認 | 9045 的 1101 與 37006 是不同語意(累計 vs 本批投入),HT160S 只有一個計數,需釐清要回哪個 |
| **37010** | Enter Skip IC Count(`&iJamSkipIC`) | 517 | S2F33 | `tRunData.iAutoSkipCount`(`cprod.h:24`)= SortArm 取料失敗自動跳格數 | D | 雙方確認 | 9045 是**操作員 jam 後手動輸入**的跳過數;HT160S 是**自動**跳格數。觸發者不同,回上去會誤導 |
| **38821** | Handling without 2DID(`&TestIF_File.iNoCodeDeviceToErr`) | 997、800 | S2F33 | HT160S 行為**固定**:2D 讀不到/查不到 → Error Bin,計入 `MachineRun.iUnknown2D`(`cprod.h:199`);**無可調旗標** | D | 雙方確認 | host 問的是「策略設定」,HT160S 只有「行為結果」。回常數需先講清 HT160S 沒有別的選項 |
| **38825** | Auto skip and set unread device 2DID to ERROR(`&TestIF_File.bNoCodeDeviceAutoSkip`) | 997、800 | S2F33 | HT160S 的 auto-skip 是 `GeneralSetting.bSortArmAutoSkipOnPickFail`(`GeneralSetting.h:161`),**觸發條件是 SortArm 取料失敗,不是 2D 讀不到** | D | 雙方確認 | 名字很像、觸發源不同。誤綁會讓 host 以為關掉它就不會跳格 |
| **38826** | Check duplicate code by lot(`&TestIF_File.bCheckCodeByLot`) | 997、800 | S2F33 | HT160S 重複 2D 檢查**恆為 on**(`THT160LotRegistry::m_LastDupCode` / `GetLastDuplicateCode()`,`CosFunction.h:251`、`:268`),無開關 | D | 雙方確認 | |
| **38800** | Has RTC Module(`&REAL_TIME_CCD`) | 505 | S2F33 | `CosFunction.bUseTopCcd`(`CosFunction.h:148`)= Top CCD **啟用**旗標 | D | 雙方確認 | 「有無 real-time CCD 硬體模組」≠「Top CCD 是否啟用」。兩者在 HT160S 是不同層 |
| **3454** | Enable Reat Time CCD(`&COM2->bCCDDummyRum`) | 505 | S2F33 | 同 38800:`CosFunction.bUseTopCcd`(`CosFunction.h:148`) | D | 雙方確認 | 9045 綁的是 CCD dummy-run 旗標,語意再偏一層 |
| **35916** | Enable Bar Code Function(`&TestIF_File.bEnableBarCode`) | 800、505 | S2F33 | HT160S 的等價開關是 `CosFunction.bUseTopCcd`(`CosFunction.h:148`)/ 2D map 使用旗標(`CosFunction.h:149` 起) | D | 雙方確認 | HT160S 沒有「barcode reader」這個獨立子系統,2D 讀取即 Top CCD |
| **37200** | SIMCO Ion fan Data(`fLotInfo->lbESDReportData`) | 700 | S2F33 | HT160S 有離子風扇 sensor:`SnIonFan_Balance` / `SnIonFan_Power`(`database.h:273-274`)+ `IsIonFanAlarm()`(`csystem.h:46`),但**沒有** 9045 的 ESD 報表字串 | D | 雙方確認 | 資料形態不同(字串報表 vs 兩顆 sensor 狀態)。RPTID 700 當天只被 S6F19 主動拉、沒被 S6F11 搬過 |
| **1023** | Index Time | 508 | S2F33 | `TLatchCycleTime lctLoader`(`cprod.h:81-90`)量測搬送 cycle | D | 雙方確認 | 9045 的 index 是測試臂 index;sorter 的 cycle 概念不同軸 |
| **1024** | Index Cycle Time | 508 | S2F33 | 同 1023(`cprod.h:81-90`) | D | 雙方確認 | 同上 |

---

### 2.3 A 類 - 已供應(26 項,確認用)

| SVID | 9045 名稱/意義 | 所屬 RPTID | host 取用方式 | HT160S 註冊處 |
|---|---|---|---|---|
| 1001 | Machine Model | (不在任何報表) | S1F3 ×3 | `uHGemHT160.cpp:108`(綁 `HandlerPath`) |
| 1003 | Software Version | (不在任何報表) | S1F3 ×3 | `uHGemHT160.cpp:109` |
| 1021 | UPH | 508、516 | S2F33 | `uHGemHT160.cpp:110` |
| 1518 | Real/Dummy | **502** | S2F33 | `uHGemHT160.cpp:117`(直接綁 `HSys.LastSet.iRealDummy`) |
| 38202 | Load Port Carrier ID | 2000 | S2F33 | `AddSV` 站台迴圈(`uHGemHT160.cpp:151`,表 `uAgvStation.cpp:104`) |
| 38205 / 38206 / 38207 | Auto1/2/3 carrier ID | 2000 | S2F33 | 同上(`uAgvStation.cpp:107-109`) |
| 38219 / 38220 / 38221 | Supplement Bin / LD UnLD Check AGV / LD UnLD Finish AGV(P1-P9 bitmap) | 2000 | S2F33 | `uHGemHT160.cpp:143-145` |
| 38222-38227 | AMR Loader / Empty / Color / Auto1-3 Tray Count | 2001 | S2F33 | `AddSV` 站台迴圈(`uAgvStation.cpp:104-109`) |
| 38228-38233 | AMR Loader / Empty / Color / Auto1-3 Device Count | 2001 | S2F33 | 同上 |
| 38234 / 38235 / 38236 | AMR Auto1/2/3 Bin Setting | 2001 | S2F33 | `AddSV` 站台迴圈 Bin Setting 分支(`uHGemHT160.cpp:158-164`) |

**唯一 100% 對齊的區塊是 AMR**:RPTID 2000(7 個 SV)與 2001(15 個 SV)全數對得上,號碼與語意都與 9045 一致。26 個 A 類中有 **22 個屬於 AMR 帶**,剩 4 個是 1001/1003/1021/1518。

HT160S 已註冊但 **host 當天完全沒問** 的 25 個 SV:1027、38203、38204、38208-38210、38237-38245、66000、66001、66002、66010、66011、66020、66021、66030、66031、66032。其中 66000-66032 是 HT160S 自有的 sorter 語意帶(客戶規格書 §3.1 宣告為權威),host 需依 HT160S 字典重新定義報表才會取到。

---

### 2.4 C 類 - 無對應(375 項,壓縮呈現)

C 類**不會有任何實作動作**,因此僅列號碼與主題。責任方一律為 **京元 host**(從報表定義中移除,或接受回空 item),唯 C8/C11/C14 三組另註。

| 主題 | 數量 | SVID |
|---|---|---|
| **C1 溫度 / soak / ATC 熱控** — sorter 無加熱、無 soak、無 ATC 冷卻頭 | 102 | 1040, 1043, 1044, 1045, 1046, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1077, 1078, 1079, 1080, 1081, 1082, 1351, 1352, 1355, 1356, 1359, 1360, 1363, 1364, 1519, 1520, 1601, 1602, 1603, 1604, 4751, 4752, 4753, 4754, 4755, 4756, 4757, 4758, 4759, 4760, 4761, 4762, 4763, 4764, 4765, 4766, 4767, 4770, 4771, 4772, 4773, 4788, 4789, 4790, 4801, 4802, 4803, 4804, 4805, 4806, 4807, 4808, 4809, 4810, 4811, 4812, 4813, 4814, 4815, 4816, 4880, 4881, 4882, 4883, 4884, 4885, 4886, 4887, 4888, 4900, 37202, 38835 |
| **C7 測試機構 offset / shuttle / arm / 速度** — In/Index/Out Arm、Shuttle、Hot Plate、Rotater 都是測試機軸,HT160S 無此機構 | 66 | 2811, 2812, 2813, 2814, 2815, 2816, 3461, 3462, 4201, 4202, 4203, 4204, 4205, 4208, 4251, 4252, 4253, 4254, 4255, 4256, 4257, 4258, 4259, 4301, 4302, 4303, 4308, 4351, 4352, 4359, 4402, 4403, 4404, 4405, 4408, 4453, 4454, 4455, 4456, 4457, 4458, 4459, 4490, 4491, 4492, 4661, 4662, 4664, 4665, 4666, 4671, 4672, 4674, 4675, 4676, 4682, 4683, 4684, 4685, 4686, 4687, 8501, 8502, 8503, 8504, 8505 |
| **C5 Auto Clean(清針)** — 清針是針對 tester socket 的動作,sorter 沒有 socket | 47 | 3547, 3548, 4206, 4406, 9001, 9003, 9004, 9501, 9502, 9511, 9513, 9514, 9515, 9516, 9517, 9518, 9521, 9523, 9524, 9531, 9532, 9533, 9534, 9535, 9536, 9552, 9553, 9554, 9555, 9556, 9557, 9558, 9560, 9567, 9568, 9569, 9570, 9571, 9572, 9573, 9574, 9601, 9602, 9671, 9672, 9689, 9690 |
| **C2 ART / MRT / 自動重測** — 重測流程本身不存在於 sorter | 45 | 1112, 1113, 1114, 1115, 1116, 1117, 1118, 1119, 1120, 1121, 1122, 1123, 1124, 1125, 1126, 1127, 1128, 1129, 1130, 1131, 1132, 1133, 1134, 1135, 1136, 1137, 1138, 3800, 3801, 3802, 3803, 3804, 3805, 3806, 9512, 10000, 10001, 35011, 37000, 37001, 37002, 37003, 37004, 37005, 37009 |
| **C3 site / socket(測試座)** — site map / socket ID / per-site 良率都源自測試座 | 25 | 1007, 1420, 1530, 1531, 3451, 3452, 3540, 3543, 3544, 3545, 3546, 16000, 16001, 16010, 16011, 16012, 16296, 16297, 16298, 16299, 16404, 16405, 16406, 16407, 38829 |
| **C6 接觸力 / device form** — 接觸力、pin 數、die force、kit 直徑皆為 contact 測試參數 | 25 | 2001, 2003, 2004, 2005, 2007, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2022, 2101, 2102, 2616, 2621, 2622, 3636, 4663, 4673, 6001, 6002 |
| **C4 yield / 連續 fail / 2DID 良率判定** — 良率與連續 fail 都要有測試結果才能算 | 17 | 1151, 1152, 1153, 1154, 1155, 1156, 3676, 16002, 16003, 16007, 16008, 16009, 38827, 38828, 38830, 38831, 38832 |
| **C13 測試機 bin/tray 型別與方向組合** — 9045 的 Loader/Auto/Fix 各站 tray 型別 + 方向 + FT/ART/MRT 四套 Bin-Tray link;HT160S 是單一 tray 幾何 + 單一 Bin→Auto 表 | 15 | 2702, 2703, 2704, 2705, 2711, 2721, 2722, 2723, 2731, 2732, 2733, 10698, 11098, 11298, 11398 |
| **C10 周邊模組存在旗標 / 子系統版本** — OCR / Magazine / RTC / ESD / AOA / GPIB 子系統版本;**非測試流程專屬,但 HT160S 無該模組,也無版本字串可回** | 10 | 37008, 37228, 37529, 37530, 37531, 37532, 37533, 38806, 38833, 38834 |
| **C9 tester 介面 / 測試流程時序** | 9 | 1025, 1513, 3400, 3401, 3405, 3450, 3521, 3523, 37203 |
| **C8 節能** — **非測試流程專屬**:22520/22521/22523 綁 ATC/加熱器/tester purge kit(測試機子系統),22522 Motor / 22524 Dry Air 在 sorter 上有硬體但 **HT160S 未建置節能子系統**(既有決策:RCMD `ENERGY_SAVING` 刻意回 HCACK=2) | 7 | 22505, 22506, 22520, 22521, 22522, 22523, 22524 |
| **C11 EC 變更稽核** — **非測試流程專屬**:HT160S 的 S2F16 只受理 2758-2763 且不留變更前/後值(`uHGemHT160.cpp:592-660`),無稽核三元組 | 3 | 20001, 20002, 20003 |
| **C14 其他** — **非測試流程專屬但 HT160S 無資料源**:1026 MTBF / 1028 Avg UPH(HT160S 只有瞬時 UPH,`GetCalculateUPH`,`csystem.cpp:1795`,無平均值變數);1041 EP Penconder / 1047 Read KG 為測試臂壓力量測 | 4 | 1026, 1028, 1041, 1047 |
| **小計** | **375** | |

> **RPTID 504 = {20001, 20002, 20003}** 全為 C11。這是 Path-A 交叉驗證裡提到的案例:整張報表的 SVID 在 HT160S 全屬未知,`ProcessDefineReport_S2F33` 的容忍設計必須讓它以 DRACK=0x00 被接受並完整落盤(`uHGemEquipment.cpp:877-887` 已修:先前會在下次開機整張消失)。

### 2.5 E 類 - 未具名/未知(0 項)

**SVID 側為 0**。445 個 host 引用號碼在 HT9045 的 SV 命名空間(含 EC→SV 雙註冊,共 2,713 個號碼)全部查得名稱。唯一查不到名稱的號碼是 **ECID 8506**,見 §ext-2。

---

## ext-1. RPTID 覆蓋率表

「SV 數」= host 在該 RPTID **最後一次** S2F33 定義的槽數。「被 S6F11 攜帶」= 當天實際搬運次數(由 S2F35 link × 各 CEID 的 S6F11 發送次數重算,全日 645 次 S6F11)。「補完 B 後」= 若 §2.1 的 19 項全部補上。

| RPTID | SV 數 | HT160S 可供應數 | 覆蓋率 | 補完 B 後 | 被 S6F11 攜帶 | 最關鍵缺項 |
|---|---|---|---|---|---|---|
| **502** | 8 | **1** | **12.5%** | 5 (62.5%) | **510** | **1011 Machine State**、3 GemClock、1501 Setup File(全部 B 類)。補完後只剩 1007 Operator ID(C)、1517 Start Mode(D)、1513 Tester On/Off(C) |
| 507 | 4 | 0 | 0% | 0 | 52 | 1530/1531 Site 狀態、3540 Site Map(全 C3)、3450 Test Mode(C9)。**整張無解** |
| 501 | 12 | 0 | 0% | 5 (41.7%) | 33 | 1101 Loader Count、1102 Output Total、1103-1105 Auto1-3(全 B);剩 1106-1108 Fix(D)、16296-16299 per-site 計數(C3) |
| 508 | 6 | **1** | **16.7%** | 2 (33.3%) | 33 | 1009 Lot Start Time(B);1023/1024 Index Time(D)、1025 Test Time / 1028 Avg UPH(C) |
| 513 | 6 | 0 | 0% | 0 | 33 | 1151-1156 per-station Yield(全 C4)。**整張無解** |
| **506** | **5**(中途改為 6 又改回 5,見下) | 0 | 0% | 2 (40.0%) | 23 | 3616 Error Bin Tray Select、3617 Bin 0-255 Tray Select(B);剩 3656 Bin Type、3677 Tray for Fail Bin(D)、3676 連續 fail(C4) |
| 505 | **179** | 0 | 0% | 6 (3.4%) | 21 | 全表 179 槽中 168 槽是溫度/清針/接觸力/測試臂 offset(C);可補的只有 2758-2763(B) |
| 509 | 38 | 0 | 0% | 0 | 21 | 1051-1082 全部溫度點(C1)。**整張無解** |
| 512 | 2 | 0 | 0% | 0 | 19 | 3461/3462 Shuttle Mode/Select(C7)。**整張無解** |
| 514 | 4 | 0 | 0% | 0 | 19 | 16404-16407 per-site 歷史計數(C3)。**整張無解** |
| 800 | **103** | 0 | 0% | 8 (7.8%) | 18 | 可補 2758-2763、3616、3617(B);其餘 95 槽為溫度/清針/site/2DID 策略(C/D) |
| 801 | 1 | 0 | 0% | 0 | 18 | 3636 Bin 0-255 Contact(C6,FT)/ 3802(ART_FT)。**整張無解** |
| 515 | 1 | 0 | 0% | 0 | 13 | 37008 Enter Barcode Reader(C10)。掛 CEID 70,當天發 13 次 |
| 503 | 3 | 0 | 0% | 0 | 12 | 1420 Site 1-32 Test Result(C3)、6001/6002 Arm Contact Count(C6)。**整張無解** |
| **2000** | 7 | **7** | **100%** | 7 | 12 | 無缺項 |
| **2001** | 15 | **15** | **100%** | 15 | 2 | 無缺項 |
| 600 | 12 | 0 | 0% | 0 | 2(另由 **S6F19 主動拉取 ×3**) | ART 相關 37000-37009 + 10000/10001(C2);37007 為 D |
| 510 | 20 | 0 | 0% | 0 | 1 | Auto Clean 參數(C5)。**整張無解** |
| 511 | 1 | 0 | 0% | 0 | 1 | 9001 Auto Clean Cleaning Count(C5) |
| 521 | 1 | 0 | 0% | 0 | 1 | 9502 Auto Clean Trigger Condition(C5) |
| 522 | 6 | 0 | 0% | 0 | 1 | 22505/22520-22524 節能狀態(C8) |
| 523 | 6 | 0 | 0% | 0 | 1 | 22506/22520-22524 節能狀態(C8) |
| 601 | 27 | 0 | 0% | 0 | 1 | 1112-1138 ART bin 計數(C2)。**整張無解** |
| 900 | 24 | 0 | 0% | 0 | 1 | 1112-1138 ART bin 計數(C2)。**整張無解** |
| **700** | 21 | 0 | 0% | 0 | **0**(僅由 **S6F19 主動拉取** ×3) | 1040-1047 ATC(C1)、1351-1364 ATC head 溫度(C1)、37200 Ion fan(D)、2003/2004/2621/2622 接觸力(C6) |
| 516 | 2 | **1** | **50%** | 1 | 0 | 1021 UPH 已供應;1028 Avg UPH 為 C14 |
| 518 | 4 | 0 | 0% | 2 (50.0%) | 0 | 1002 Machine ID、1501 Setup File(B);4490/4492 EESUG(C7) |
| 519 | 4 | 0 | 0% | 2 (50.0%) | 0 | 同 518;4490/4491 EESUG(C7) |
| 504 | 3 | 0 | 0% | 0 | 0 | 20001-20003 EC Change 稽核(C11)。**整張無解**,但必須被容忍接受(見 §2.4 註) |
| 517 | 1 | 0 | 0% | 0 | 0 | 37010 Enter Skip IC Count(D) |
| **804 / 994 / 995 / 996 / 997 / 998 / 999** | 12 / 8 / 2 / 15 / 9 / 5 / 3 = **54** | 0 | 0% | 0 | **0** | 七張**當天以 S2F33 定義了,但從未被 S2F35 連結到任何 CEID、也未被 S6F19 拉取**。全日 37 張報表裡有 30 張被連結,這 7 張是純浪費 |
| **合計** | **580** | **25** | **4.3%** | **55 (9.5%)** | 12,010 槽·次 | 流量加權覆蓋 **5.5% → 26.7%** |

### RPTID 506 / 800 / 801 的中途重定義(必須注意)

host 在同一天對這三張報表做了 **FT ↔ ART_FT 換頭**,而且 **槽數會變**:

| 時間 | 檔案 | 動作 |
|---|---|---|
| 13:58:58 | `..._13.txt` | S2F33 `L,0` — 全部刪除 |
| 13:59:10 | `..._13.txt` | 506 = **5 槽** {3616, 3617, 3656, 3677, 3676}(FT 帶) |
| 13:59:11 | `..._13.txt` | 801 = 1 槽 {3636}(FT) |
| 14:06:08 / 14:06:20 / 14:06:21 | `..._14.txt` | 全刪後重建,同上(506=5) |
| 16:47:41 | `..._16.txt` | 全刪 |
| 16:47:53 | `..._16.txt` | 506 = **6 槽** {3800, 3801, 3803, 3805, 3804, 3806}(**ART_FT 帶**) |
| 16:47:54 | `..._16.txt` | 801 = 1 槽 {3802}(ART_FT) |
| 17:51:59 | `..._17.txt` | 對 506 / 800 / 801 **逐張** 送 `L,0`(單張刪除,非全刪) |
| 17:51:59 → 17:52:00 | `..._17.txt` | 立刻重定義為 ART_FT 版(506=6) |
| 17:52:03 | `..._17.txt` | **S6F19 主動拉 RPTID 506** → S6F20 回 `L,6` |
| 19:07:42 → 19:07:43 | `..._19.txt` | 再次逐張刪除,重定義回 **FT 版(506=5)** |
| 19:07:46 | `..._19.txt` | **S6F19 再拉 RPTID 506** → S6F20 回 `L,5` |

**工程含意**(HT160S 已處理,`uHGemEquipment.cpp:456-469` 已註明並實作):
1. S6F20 的 list 長度必須跟著 host 最後一次定義走,**不能寫死**(同一天同一 RPTID 出現 `L,5` 與 `L,6`)。
2. S2F33 的 `L,0` 有兩種語意:**外層** `L,0` = 刪全部;**內層某 RPTID 的 SV list 為 `L,0`** = 只刪那一張。HT160S 的 `ProcessDefineReport_S2F33` 已把後者映到 `DeleteHostReport`。
3. 因為 506 的兩套內容(FT 3616/3617/... vs ART_FT 3800/3801/...)中,HT160S 只在 FT 版有 B 類可補(3616/3617),**ART_FT 版對 HT160S 100% 無解**。這點要向京元說明:切到 ART_FT 帶時 HT160S 的 506 會全空,不是故障。

---

## ext-2. ECID 分責

### ext-2.1 供需總量(重算)

| 項目 | 數量 | 明細 |
|---|---|---|
| HT160S `AddEC` 註冊的 EC | **7** | 1501、2758、2759、2760、2761、2762、2763(`uHGemHT160.cpp:244-256`) |
| host 以 **S2F15 實際寫入**的 ECID | **8** | 1006、1007、16000、16002、16023、16026、35011、37007(24 筆 S2F15 交易) |
| host 以 **S125F1 啟用**的 ECID | **45** | 6 筆 S125F1(3 次空清單前導 + 3 次同一份 45 個 ECID)。旁證:9045 全日回了 **138 筆 S125F2**(= 3×45 + 3),因為 9045 在解析迴圈內**逐 ECID 各回一個 ack**;HT160S 刻意改為單一 S125F2 ack(未提交工作樹已註明) |
| 兩者聯集 | **50** | 重疊 3 個:16000、16002、35011 |
| **HT160S 7 個 EC 與 host 寫入集(S2F15 8 個)的交集** | **0** | 完全零交集 — 京元當天寫的每一個 EC 在 HT160S 都會被 `S2F16` 回 **EAC=1**(`uHGemHT160.cpp:635-640`:只有 2758-2763 可寫) |
| **HT160S 7 個 EC 與 host 啟用集(S125F1 45 個)的交集** | **1** | 只有 **1501**(Setup File / Recipe Name)。2758-2763 一個都不在 host 的啟用清單裡 |

> **修正前次說法**:「零交集」只對 **S2F15 寫入集** 成立。對 S125F1 啟用集,交集為 1(ECID 1501)。

### ext-2.2 host 以 S2F15 寫入的 8 個 ECID(逐列)

| ECID | 9045 名稱/意義 | host 寫入次數 / 值 | HT160S 現況 | 分類 | 責任方 | 說明 |
|---|---|---|---|---|---|---|
| **1006** | EC=`Lot ID`(`uHGemHT9045_EC.cpp:57`,綁 `fLotInfo->edtSysLotID`);同號另註冊為 SV `Site Ag Socket ID`(`SECSGEM.cpp:725`) | **3 次**,值 `"LQ50SIJAG2"` | 未註冊 EC 1006 → `S2F16` 回 **EAC=1**。批號在 HT160S 走 RCMD `SET_LOT_INFO`(`uHGemHT160.cpp:687-694`)+ `LotRegistry` | **B** | 雙方確認 | 這是 8 個裡**唯一 sorter 有實質對應**的一個。決策點:要不要讓 host 用 S2F15 ECID 1006 設批號(現在只能用 RCMD)。註意 9045 同號雙註冊,語意須明文寫進規格書 |
| **1007** | EC=`Operator ID`(`uHGemHT9045_EC.cpp:58`);同號 SV 為 `Site Ah Socket ID` | **4 次**,值 `"AGV"` | 未註冊 → EAC=1。HT160S **沒有操作員身分資料源**(全庫查無 operator/user id 變數) | **C** | 雙方確認 | 非測試機專屬,但 HT160S 未建置操作員身分。京元寫的值是 `"AGV"`(表示無人線),語意上等同「本班由 AMR 操作」;若京元需要,這是一個新欄位而非既有資料 |
| **16000** | Consecutive Failure Alarm by Socket | **18 次**,`Boolean 0x00` | 未註冊 → EAC=1 | **C** | 京元 host | 連續 fail 判定需測試結果 + socket,sorter 兩者皆無 |
| **16002** | Consecutive Failure Alarm by Head | **18 次**,`Boolean 0x00` | 未註冊 → EAC=1 | **C** | 京元 host | 同上(head = 測試頭) |
| **16023** | By Site Compare Yield Enable | **18 次**,`Boolean 0x00` | 未註冊 → EAC=1 | **C** | 京元 host | per-site 良率比較 |
| **16026** | Low Yields%(By Total) Enable | **18 次**,`Boolean 0x00` | 未註冊 → EAC=1 | **C** | 京元 host | 良率警報 |
| **35011** | [A10] Auto Retest | **2 次**,值 `"True"` | 未註冊 → EAC=1 | **C** | 京元 host | ART 開關;sorter 無重測流程 |
| **37007** | LoaderTotalTray | **1 次**,`U4 1` | 未註冊為 EC。同概念已以 **SVID 38222** 供應(`AgvCoord.TrayCount[0]`,由 `START_AGV LoaderTrayCount` 帶入,`uAgvStation.h:56-61`) | **D** | 雙方確認 | 若京元希望「用 S2F15 設車載總盤數」而非只靠 `START_AGV`,這是一條需求;但要先確認 HT160S 的 TripQueue 模型允許 host 覆寫 |

**現場結論**:京元 host 全日 24 筆 S2F15 若原封不動打到 HT160S,**24 筆全部拿 EAC=1**。這在功能上沒有阻斷(它們寫的都是測試機參數),但 host 端會看到連續的 EC 寫入失敗,運維上需事先向京元說明,否則會被當成 HT160S 的 SECS 故障。

### ext-2.3 host 以 S125F1 啟用的 45 個 ECID(壓縮呈現)

S125F1 為 KYEC 私有 stream(Enable/Disable EC Data Send)。HT160S 已實作 S125F1 → 單一 `S125F2 <B ACK>`(未提交工作樹,`uHGemHT160.cpp`),**刻意不建 EC 啟用表**,僅記錄被引用的 ECID。分責如下:

下表每個 ECID 只出現一次,數量欄相加 = 45。

| 主題 | 數量 | ECID | 分類 | 責任方 |
|---|---|---|---|---|
| **HT160S 已註冊為 EC(唯一交集)** | 1 | **1501** | **B** — 已存在為 EC(`uHGemHT160.cpp:244`),但 SVID 側未註冊;見 §2.1 的 1501 列 | HT160S |
| **Bin 設定(HT160S 有對應資料)** | 1 | **3616** | **B** — `BinAreaMap::ErrorBinArea`(`CosFunction.h:92`);見 §2.1 的 3616 列 | HT160S |
| **host 遠端啟動旗標** | 1 | **35816** | **D** — HT160S 無此開關但行為等效常數 1;見 §2.2 | 雙方確認 |
| ONE_CYCLE / 警報後啟動檢查(9045 [N07] 系列) | 2 | 35817, 35818 | D — HT160S 的 ONE_CYCLE **行為已實作**(未提交工作樹),但沒有對應 EC 旗標可讀寫 | 雙方確認 |
| 測試臂真空 / 破真空等待 | 8 | 2501, 2502, 2503, 2504, 2511, 2512, 2513, 2514 | C — In/Index/Out/Tray Arm 為測試機軸 | 京元 host |
| site map / site 連續 fail / site 良率 | 7 | 3540, 16000, 16001, 16002, 16003, 16010, 16011 | C — 測試座專屬 | 京元 host |
| 測試臂 / shuttle 速度 | 5 | 8501, 8502, 8503, 8504, 8505 | C — 測試機軸 | 京元 host |
| 接觸力 / device form | 5 | 2004, 2101, 2102, 2621, 2622 | C — contact 測試參數 | 京元 host |
| 溫度 / soak / index heat | 4 | 1514, 1519, 1520, 4914 | C — sorter 無加熱 | 京元 host |
| Auto Clean(清針) | 4 | 9501, 9517, 9535, 9536 | C — 針對 tester socket | 京元 host |
| tester 介面 / 啟動延遲 / ART_FT bin | 3 | 3400, 3523, 3801 | C — 3801 為 ART_FT Bin Tray Select | 京元 host |
| 低良率警報 | 2 | 16007, 16008 | C — 需測試結果 | 京元 host |
| ART 開關 | 1 | 35011 | C — sorter 無重測流程 | 京元 host |
| **未具名號碼** | 1 | **8506** | **E** — 只出現在 S125F1 清單(`..._13.txt` 13:59:33 / `..._14.txt` 14:06:41 / `..._16.txt` 16:48:22),在 `uHGemHT9045_SV.cpp` 與 `uHGemHT9045_EC.cpp` **都查不到**。依 8501-8505 的排列推測應為第 6 個機構速度,但**主要來源無此號,不予命名** | 雙方確認 |
| **合計** | **45** | | | |

### ext-2.4 SVID / ECID 命名空間重疊:2762 / 2763

**事實**

- HT160S 把 2762 / 2763 註冊在 **EC 命名空間**(`SetECDataPointer`,`uHGemHT160.cpp:255-256`),綁 `TrayForm.XDivision` / `TrayForm.YDivision`(`CosFunction.h:61-62`),並允許 host 以 S2F15 寫入(`uHGemHT160.cpp:635-640`,唯一開放的 2758-2763 帶)。
- 京元 host **把 2762 / 2763 當 SVID 用**:S1F3 直接輪詢(全日各 **23 次**,約每 5 秒一組),同時放進 RPTID 800 與 505。
- HT9045 沒有這個問題,因為它的 `SetECDataPointer` 會回頭呼叫 `SetSVDataPointer`(§1.3),EC 自動兼作 SV。

**為什麼不是衝突**

SECS-II 的 SVID 與 ECID 是**兩個獨立命名空間**。同一個數字在 SV 表與 EC 表各有一筆是合法的,而且 HT160S 的兩者指向**完全相同的底層資料**(`TrayForm.XDivision`/`YDivision`),沒有值不一致的風險。HT160S 也不會因此回錯值:未知 SVID 走 `DataItemOutSVItem(NULL)` 回一個空 LIST item(`uHGemEquipment.cpp:645-648`,與 `DataItemOutECItem` 的 `:666-670` 同款),結構對齊、E5 合法,不會壞掉這一筆交易。

**為什麼也不是對齊**

host 走的**讀取路徑**(S1F3 / S6F11 讀 SVID)碰不到 HT160S 的 EC 表。所以現況是:host 每 5 秒問一次盤格數,HT160S 每 5 秒回一個空值,而**答案其實就在隔壁的 EC 表裡**。這不是設計缺陷也不是協議衝突,純粹是**框架層的註冊不對稱**。

**兩種修法(擇一,建議 (a))**

- **(a) 點狀修**:在 `AddSV()` 對 1501、2758-2763 各補一行 `SetSVDataPointer(同號, 同型別, 同指標)`。7 行,零新資料源,不動框架。已列為 §2.1 的 B 類項目。
- **(b) 框架修**:讓 HT160S 的 `THGem::SetECDataPointer()`(`uHGemEquipment.cpp:807-827`)在尾端也註冊 SV,完全比照 9045。一行改動就自動覆蓋現有與未來所有 EC,但會改變 `GetSVCount()` / `S1F3 n<=0`(回報全部 SV)/ `S1F11` namelist 的輸出,屬於框架行為變更,需回歸驗證。

---

## ext-3. 一句話結論

**HT160S 目前能滿足京元 host 需求的比例:以號碼計 5.8%(26/445),以實際上線流量加權計 5.5%;補上 §2.1 那 19 個 B 類(全部是既有變數多綁一個號、無新資料源、無新機構)後,流量加權覆蓋跳到 26.7%,而剩下的 375 個 C 類本質上是測試機資料,無論做多少工都不會有值。**

最短可用路徑,依投報比排序:

1. **先補 RPTID 502 的四個號:1011、3、1501、1006** — 502 一張報表就吃掉當天 79.1% 的 S6F11(510/645 次),覆蓋率從 1/8 拉到 5/8。其中 1011 同時是全日最高頻的 S1F3 輪詢對象(25 次)。**這一步就是 26.7% 裡的絕大部分。**
2. **補 2762/2763(+2758-2761)** — 解掉 host 每 5 秒一次的 7 號輪詢裡最有解的兩個,做法就是 §ext-2.4 的 (a)。
3. **補 RPTID 501 的五個號:1101、1102、1103、1104、1105** — sorter 最該給 host 的東西(投入量 + 產出量 + 各 Auto 站計數),資料全在 `tRunData` / `MachineRun`。
4. **補 3616、3617** — 讓 RPTID 506 的 FT 版有 2/5;但要同時告知京元:切到 ART_FT 帶(3800-3806)時 506 會全空,那是設計結果不是故障。
5. **與京元開一次 D 類會議(25 項)** — 優先議程只有三個:**1517 Start Mode**(卡在 502 的最後一格)、**35816 Enable Host Control Start**(host 用它決定要不要下 START)、**1106-1108 Fix1-3 是否對映 Auto4-6**(商務決定)。其餘 22 項可批次處理。
6. **正式告知京元 host 端要移除的東西**:S2F15 那 8 個 ECID 有 7 個在 HT160S 必然 EAC=1;RPTID 503/507/509/510/511/512/513/514/521/522/523/601/900 十三張報表在 HT160S **整張無解**,建議 host 端直接不定義,否則每次事件都在搬空 list。另外 804/994/995/996/997/998/999 七張當天定義了但從未連結任何 CEID 也沒被 S6F19 拉過,共 54 槽純浪費。

**不做的事**:不單方面 renumber CEID 1-31(既有決策:那是與客戶的商務決定,客戶規格書 `docs\SECS\HT160S_SECS_Interface_Spec_20260727.md` §3 宣告 HT160S 語意為權威);不為了「補齊」去發明溫度、site、測試結果、ART/MRT、yield 的假 SVID。
