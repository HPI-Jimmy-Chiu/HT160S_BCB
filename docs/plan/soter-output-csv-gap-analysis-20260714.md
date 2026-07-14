# HT160S Soter 逐 IC 輸出 CSV — 決策級可行性綜整

> 產出日期:2026-07-14　來源:4-domain 平行程式碼調查 + 綜整(workflow soter-csv-gap-analysis)
> 專案根目錄:`D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\`
> 權威規格:客戶 Excel `D:\backup_version\HT160S\Document\Soter檔案格式.xlsx`(per-die,每顆 IC 一列)
> 狀態:**計畫待審查 — Phase 0 客戶確認為前置阻塞,尚無程式碼**

---

## 0. 檔案規格摘要(來自客戶 Excel)

**檔名**
```
{Date}_{Time}_KYEC-LFT_{ProductCode}_{CustomerLotNo}_{KYECLotNo}_BI_{Substage}_{SorterID}_{Qty}.csv
20260317_165040_KYEC-LFT_MT3781Q-ZAHJA32-ETTTT-H_A5921.RCS.TEST99_NQ4000NAA1_BI_BI1_XX-01_30.csv
```
`KYEC-LFT` 與 `BI` 為固定字面;Date=YYYYMMDD;Time=HHMMSS;Qty=總 unit 數。

**內容** 15 欄,逐顆記錄(見第 2 節)。

---

## 1. 可行性總判 (verdict)

**分類:(a) 大致就緒 — 資料模型已是 per-die,主要缺的是「寫檔器 + 少數非量測欄位」,而非 per-die 追蹤能力本身。**

最關鍵的結論:**HT160S 今天就是逐顆 IC 在記錄的,不是只有 Bin 統計。** per-die 鏈結「已經存在」,這是本案最強的正面證據:

- **已有逐 IC CSV 記錄器且線上運作**:`TDeviceInfo` / `g_DeviceInfo`(`deviceinfo.cpp/.h`),每顆 IC 寫一列到 `D:\HT160S_Log\Production_Log\{yyyymm}\{Lot}_{start}.csv`,21 欄表頭 `deviceinfo.cpp:70-76`,在 place 完成時 flush(`AddOutputInfo` `deviceinfo.cpp:246-247`,由 `aSortArm.cpp:1518` 呼叫),reject 走 `SaveRejectRecord`(`deviceinfo.cpp:251`,`aSortArm.cpp:1263`)。
- **物理 die ↔ 2D-map 列的 join key 已逐顆持久化**:每顆 IC 的全域唯一 2D 碼在 Top CCD 掃描時解析並蓋章到料盤格(`aLoader.cpp:1765-1806`、`SetTrayCode2D` `MyMotor.cpp:211`),隨 SortArm pick 帶到 `Slot.Code2D`(`aSortArm.cpp:910`),最後寫入 Production_Log `e2DCode`(`deviceinfo.cpp:165`)。
- **靠 2D 碼即可補齊 6 個 2D-Mapping 欄位**:`LotRegistry.FindIcInfo(Code2D)`(`CosFunction.cpp:1249`)→ HBin/SBin/DiePass/RetestCode;`GetLot(LotIndex)`→ ProductCode/Substage/LotID。
- **共用 CSV 基礎設施已存在**:`cCsvDailyLog`(資料夾輪替、首寫表頭、thread-safe AppendLine、CsvQuote、保留清理),Soter writer 應直接繼承它,而非重寫 fopen。

因此本功能不需要「新建 per-die 感知子系統」。真正需要新做的是:一支 Soter 專屬 writer,加上少數本機不存在的欄位(KYEC lot 第二身分、SorterID、cover tray 持久化)。這幾個是局部的 (c) 級缺口,不影響整體 (a) 判定。

---

## 2. 逐欄位來源表 (15 欄)

| # | 欄位 | 來源 | 今天可得 | 位置 (file:line / struct) | 缺什麼 |
|---|------|------|:---:|------|------|
| 1 | No. (serial) | 自產 | no | 最近似 `tRunData.TotalIC` (`cprod.h:19`, 累加於 `aSortArm.cpp:1492`) | 需 writer 內每列遞增計數器;TotalIC 只算 placed、排除 reject,不能當序號 |
| 2 | StartTime | 自產 | yes | `m_sStartTimeStr` (`deviceinfo.cpp:46`, 由 `main.cpp:1978` OnLotStart 播種);per-die pick 時間 `eLoadTime` (`deviceinfo.cpp:149`) | 格式 `yyyy-mm-dd_hhnnss` → Soter 要 `yyyy-mm-dd hh:nn:ss`,writer 重排 |
| 3 | FinishTime | 自產 | partial | per-die place 時間 `eUnloadTime` (`deviceinfo.cpp:244`);lot 級 `tRunData.LotEndTime` (`cprod.h:12`, `main.cpp:2424`) | 若指 per-die place 完成 = yes;若指 Lot 結束才知 → 逼出「LotEnd 批次產檔」或回填 |
| 4 | ProductCode | 2D Mapping | partial | `TLotRunInfo.sProductCode` (`CosFunction.h:218`, 載於 `CosFunction.cpp:1436-1452`);經 `GetLot(Slot.LotIndex)` | 記憶體內存在(lot 級),但未寫進 Production_Log,需拉進 Soter 列 |
| 5 | Substage | (spec) E87 / (實作) 2D Mapping | partial | `TLotRunInfo.sSubstage` (`CosFunction.h:217`, 設於 `CosFunction.cpp:1452`) | **來源不符**:規格說 E87,實作只從 2DIDHistory JSON 取。SECS host 載入時不送 substage |
| 6 | Cust lot | 2D Mapping | partial | `TLotRunInfo.sLotID` (`CosFunction.h:204`);已寫 Production_Log 'Lot' 欄 (`deviceinfo.cpp` AddIcIdentity) | 本機只有「單一」lot 身分;需確認 2D-map LOTID 就是 Cust lot |
| 7 | Kyec Lot | E87 | no | 無獨立欄位;host-push lot 存於同一個 `sLotID` (`uHGemHT160.cpp:730/838`) | 無 KYEC-vs-Cust 區分;col 6/7 今天會塌成同值,需新增 `sKyecLotID` + 來源路徑 |
| 8 | Load Cover Tray ID | E87 / AMR 身分盤 | partial | Color CCD 讀身分盤 2D `sTrayID2D` (`aColor.cpp:1249`);外傳 S6F11 CEID275/SVID38204 (`aColor.cpp:1622`, `uAgvStation.cpp:184-194`) | 方向相反(本機是「讀出+上傳」);只存最新值(每次讀被覆寫),未按 lot/unit 持久化 |
| 9 | Unload Cover Tray ID | 自產 | no | `eOutTrayID` 欄存在 (`deviceinfo.h:20`) 但恆為 ""(`aSortArm.cpp:1518` sOutputTray="") | 機台不指派輸出盤身分;客戶範例也空。留白或需自建產生器 |
| 10 | SorterID | 自產 | partial | `GeneralSetting.sHandlerID` / `sSerialNo` (`GeneralSetting.h:171-172`);`General.ini [MachineIdentity]` (:114-117,皆空) | 設定管線在但未命名 SorterID、值為空;需擇一並逐機填入(例 'XX-01') |
| 11 | 2D ID | 2D Mapping | yes | Production_Log `e2DCode` (`deviceinfo.cpp:165`);來源 `Slot.Code2D` (`aSortArm.cpp:910`) | 無缺口。主 join key,pick/place 皆有 |
| 12 | RetestCode | 2D Mapping 透傳 | partial | `TLotIcInfo.sRetestCode` (`CosFunction.h:234`, JSON 載入 `CosFunction.cpp:1470-1492`);`FindIcInfo` | 純上游透傳值(如 R0),本機無 retest/rework run 概念;未寫進 Production_Log |
| 13 | Hbin | 2D Mapping | partial | `TLotIcInfo.iHBin` (`CosFunction.h:232`);`FindIcInfo` (`CosFunction.cpp:1254`) | Production_Log `eBin` 是「routing bin」非 HBin,必須改用 FindIcInfo |
| 14 | Sbin | 2D Mapping | partial | `TLotIcInfo.iSBin` (`CosFunction.h:233`);`FindIcInfo` (`CosFunction.cpp:1255`) | 同上,經 FindIcInfo,非既有 Bin 欄 |
| 15 | DiePass | 2D Mapping | partial | `TLotIcInfo.sDiePass` (`CosFunction.h:235`);`FindIcInfo` (`CosFunction.cpp:1257`) | 與既有 `ePassFail`(PASS/FAIL vs PassBin 分類器)不同,需經 FindIcInfo |

**檔名 token**:Date(YYYYMMDD)、Time(HHMMSS)= 自產,`FormatDateTime` trivial(`deviceinfo.cpp:132/134`);Qty = 總 unit 數,`tRunData.TotalIC` 但排除 reject,需真實列數;固定字面 `KYEC-LFT` / `BI` 原碼不存在,writer 內硬編。CustomerLotNo/KYECLotNo/ProductCode/Substage/SorterID token 與對應欄位同源。

---

## 3. 關鍵落差 (gaps) — 最阻塞優先

1. **[非阻塞,但為主體工作] Soter 專屬 writer 尚不存在**。錨點(per-die、place-flush)齊備,只需在既有 flush 點加一支繼承 `cCsvDailyLog` 的 `cSoterOutput`。**注意:per-die 鏈結不是缺口 — 它已存在**,這是與「純 Bin 統計機」的本質差異。
2. **Customer lot vs KYEC lot 塌成單一 `sLotID`**(col 6/7)。`TLotRunInfo` 只有一個 lot 身分,且同時當作 2D-map 比對 key。若兩欄需不同值,必須新增第二欄位並決定哪個來源(SECS lot ID vs 2D JSON LOTID)對應哪欄。
3. **SorterID 未命名、未設值**(col 10 + 檔名)。HandlerID/SerialNo 皆空,需擇一並逐機填。
4. **Load Cover Tray ID 只存最新、未持久化**(col 8)。`sTrayID2D` / `CarrierID[2]` 每次進料被覆寫,需按 lot/batch/unit 綁定其涵蓋的 IC。非 AMR 或 Color CCD 關閉時為 `COLOR2D_` placeholder 且刻意不上傳(`aColor.cpp:1194/1621`),此時可能真的沒有值。
5. **Unload Cover Tray ID 從不指派**(col 9)。欄位插槽在但恆空,需決定留白或自建產生器。
6. **Substage 來源不符**(col 5)。規格標 E87,唯一可運作來源是 2D JSON。需客戶裁決;若堅持 E87,得擴充 SET_LOT_INFO/LOTSTART 或新增 SVID/CP 承載。
7. **離線 'Maps' lot 會使 col 12-15 退化**。離線路徑(`AddItem` `CosFunction.cpp:1062`)把 routing Bin 鏡射為 HBin/SBin,RetestCode/DiePass 留空 — 這些欄位在非 2DIDHistory 建立的 lot 會空白。
8. **無每列序號 + Qty 分母錯誤**(col 1、檔名 Qty)。`TotalIC` 只算 placed OK、跳過 reject,不是每列單調計數,也非真實輸出列數。
9. **FinishTime lot-end vs per-die 分歧**(col 3)。若採 lot 級,無法在 per-unit 串流即時填,逼出 LotEnd 批次或回填;檔名帶總 Qty 也指向 LotEnd 定稿。
10. **LotRegistry 為記憶體、換 2D-map 即清**。join 必須在 place 時(lot 仍載入)執行,或在 pick/scan 時把 2D-map 欄位快照到 Slot/TDeviceInfo 記錄上,否則 lot 卸載後 HBin/SBin/DiePass/ProductCode 遺失,無法事後產檔。

---

## 4. 待客戶/使用者確認的問題 (open questions)

1. **Cust lot vs Kyec Lot 對映**:2D-map LOTID 是 Cust lot(col 6)嗎?SECS-push 的 lot ID(SET_LOT_INFO/LOTSTART → `sLotID`/`edLotNo`)是 KYEC lot(col 7)嗎?還是兩者同一實體 lot?此決定是否需新增欄位。檔名 CustomerLotNo/KYECLotNo 與這兩欄同源。
2. **Substage 真源**:接受 2D-Mapping 來源的 `sSubstage`,還是必須來自真正的 host E87 載入訊息?本機目前只有前者。客戶是否真的跑 host E87 上貨交易?(HT160S 今天完全沒有 inbound E87 載入訊息,只有 outbound S6F11。)
3. **Unload Cover Tray ID 來源**(必列):螢幕截圖說 E87,現行 Excel 說 self-generated。到底留白(符合客戶範例)、還是機台要自產一個輸出盤身分?若自產,格式為何?
4. **Load Cover Tray ID 綁定粒度**:身分盤 2D 該按 lot、AMR batch、還是 per-unit 存,key 為何,writer 才能為每列取回正確值?此欄是否只在 AMR 模式(bUseAMR)有意義?
5. **檔案何時產出、寫在哪**:per-unit 邊跑邊寫、還是 LotEnd 一次批次匯出?FinishTime(lot 級)與檔名帶總 Qty 都指向「LotEnd 定稿」。是「即時 append + LotEnd 改名」還是「緩衝到 LotEnd 一次寫」?輸出目錄?(建議比照 `D:\HT160S_Log\` 下自有子資料夾。)
6. **SorterID 取哪個身分**:`sHandlerID` 還是 `sSerialNo`(`General.ini [MachineIdentity]`)?或用 SECS/GEM equipment id?
7. **No. 序號規則**:每檔/每 lot 從 1 重置?是否含 reject 每列都編號?
8. **Hbin/Sbin 輸出語意**:兩欄都輸出 2D-map 原始值(經 FindIcInfo),與哪一個驅動 routing(`iSortBinSource`)無關?
9. **RetestCode 語意**:確認純為上游 2D-map 透傳(R0/R1…),機台永不自行計算 normal-vs-retest。
10. **Reject/AutoSkip IC**:是否也產 Soter 列?其 DiePass/Hbin/Sbin 填什麼?

---

## 5. 分階段實作計畫 (phased plan)

原則:HT160S 慣用法 — 無 FSM、無 C++11(不用 auto/nullptr/lambda/range-for)、保留 Big5 legacy 檔編碼、新註解 ASCII-only。每階段獨立可驗證。Build gate(每次 C++/DFM 改動):刪對應 `.obj` 後編譯;動到 `SOFT_SIMULATE`-guarded 或共用核心(aSortArm、deviceinfo)須同時驗 sim 與 real(切 `MachineType.h` 的 `#define SOFT_SIMULATE`,`-Full` exit 0 後還原);跑 `check-ht160s-source-encoding.ps1`。優先 `build-ht160s.ps1 -Clean`。

### Phase 0 — 客戶規格確認(阻塞,先於任何程式碼)
把第 4 節 open questions 收斂為決策,尤其:col 6/7 是否兩值、Substage 來源、Unload Cover Tray ID 來源、檔案產出時機/位置、SorterID 取值。這些決定後續是否需 Phase 4/5。**無程式碼。**

### Phase 1 — 在 pick 時把 2D-map 欄位快照進 per-die 記錄(解記憶體清除 + linkage 持久化)
- 觸點:`TSortArmModule::TransferPickDataFromLoader`(`aSortArm.cpp:1449-1467`)。以 `LotRegistry.FindIcInfo(Slot.Code2D)`(`CosFunction.cpp:1249`)取 HBin/SBin/DiePass/RetestCode,以 `GetLot(Slot.LotIndex)` 取 ProductCode/Substage/LotID,快照到 `Slot`(比照現有 `Slot.Code2D`/`Slot.PassClass`)並/或帶入 `TDeviceInfo` 內部記錄。
- 效益:避免 place-time 每顆 FindIcInfo(仍可接受,但 pick 快照更穩),且 lot 卸載後值不遺失。
- 驗證:sim 跑一批,debug dump 或暫時性欄位確認 Slot/record 帶到正確 HBin/SBin/DiePass/ProductCode。

### Phase 2 — 新增 `cSoterOutput` writer(繼承 `cCsvDailyLog`),掛在既有 flush 點
- 新檔 `cSoterOutput.cpp/.h`,15 欄,重用 `cCsvDailyLog` 的表頭/quote/thread-safe append。
- emit 點:placed 於 `aSortArm.cpp:1518`(或 `TDeviceInfo::AddOutputInfo` `deviceinfo.cpp:230` 內);reject 於 `aSortArm.cpp:1263`(或 `SaveRejectRecord` `deviceinfo.cpp:251`)。與 Production_Log 平行輸出、零新增機構管線。
- 自產欄位:每列序號(writer 內計數器)、StartTime/FinishTime(重排 `NowTimeStr`)、SorterID(暫讀 GeneralSetting)。
- 2D-Mapping 欄位:取自 Phase 1 快照。
- 驗證:sim 跑,檢視 CSV 每列 15 欄、reject 行為、序號連續。

### Phase 3 — 檔名 + Lot 生命週期定稿 + SorterID 設定
- 依 Phase 0 決策實作 `{Date}_{Time}_KYEC-LFT_{ProductCode}_{CustLot}_{KyecLot}_BI_{Substage}_{SorterID}_{Qty}.csv`,硬編 `KYEC-LFT`/`BI`。
- Qty 用真實輸出列數(非 `TotalIC`)。Lot 生命週期:LotEnd 定稿/改名或緩衝寫出,對接 `main.cpp:2424` btnLotEndClick 與 `g_DeviceInfo.OnLotStart`(`main.cpp:1978`)。
- SorterID:在 `General.ini [MachineIdentity]` 明確化(擇 HandlerID 或 SerialNo),maintenance UI 已有欄位(`maintenance.cpp:1145-1147/1212-1214`),逐機填值。
- 驗證:LotStart→跑→LotEnd,檔名 token 全正確,Qty 對得上列數。

### Phase 4 — Cover Tray ID(依 Phase 0)
- Load Cover Tray ID:把 `sTrayID2D`(`aColor.cpp:1249`)按決定粒度(lot/batch)持久化並綁到各 die 列。尊重 `IsTrayID2DGenuine` gate(非 AMR/CCD 關閉時留空)。
- Unload Cover Tray ID:留白(符合範例)或自建產生器 → 經 `AddOutputInfo` 的 `sOutputTray` 傳入(目前恆 "")。
- 驗證:AMR 模式 sim 確認 col 8 帶入正確身分盤 2D,col 9 依決策。

### Phase 5 —(條件性)KYEC lot 第二身分 / E87 Substage
- 僅當 Phase 0 確認 col 6/7 需兩不同值:於 `TLotRunInfo` 新增 `sKyecLotID`,決定填入路徑(SECS push vs 2D JSON)。
- 僅當 Substage 必須來自 host E87:擴充 SET_LOT_INFO/LOTSTART(`uHGemHT160.cpp:730/838`)或新增 SVID/CP 承載,並建 inbound 儲存。此為最大工程量,列最後且視需求開。
- 驗證:SECS simulator(`D:\AI_Area\Tool\HT160S_SECS_Simulator`)推 lot,確認第二身分/Substage 正確落檔。

---

**建議路徑**:Phase 0 決策 → Phase 1+2+3 即可交付「機台自有 + 2D-Mapping」13 欄的完整可用 Soter 檔;Phase 4/5 視客戶對 cover tray 與 KYEC/E87 的實際需求再開。整體風險低,因 per-die 骨幹已在線上運作。
