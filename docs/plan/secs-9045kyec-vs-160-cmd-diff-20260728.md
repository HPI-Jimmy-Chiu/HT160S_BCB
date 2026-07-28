# HT9045(京元現場) vs HT160S — SECS 指令面完整比對

- 建立日期：2026-07-28
- 9045 現場 log：`D:\backup_version\HT9046\KYEC\20260626\2026_06_08\SECSGEM_TextLog_00..19.txt`（20 檔，2026-06-08 全日，含完整 SML body dump）
- 9045 CEID 目錄：`D:\backup_version\HT9046\KYEC\20260626\EventReport_CEID.def`（292 個編號、286 個具名）
- 9045 原始碼（唯讀對照）：`D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\SECSGEM\`
- HT160S 實作（本專案）：`D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SECSGEM\uHGemClass.cpp`（dispatch）、
  `uHGemHT160.cpp`（SV/EC/CEID/Report/RCMD）、`SecsGem\uAgvStation.cpp`（AMR）
- 解析工具：`scratchpad\parse9045.py`（遞迴 SML parser）＋ `scratchpad\compare.py`

> 方法說明：log 只能證明「host 當天真的送了什麼、機台真的回了什麼」。
> 因此本報告把**「兩邊實作能力」**（比原始碼）與**「京元 host 當天實際使用」**（比 log）分開列，
> 兩者交集才是真正會踩到的缺口（以下標記 **HOT**）。

---

## 0. 一句話結論

三層指令（Message / RCMD / CEID）**共通的部分很少，而且 CEID 1–31 是「同號不同義」**——
這是比「缺指令」更危險的問題。以京元 host 當天實際送出的內容為準，HT160S 會出事的有 **13 項**：
5 個 message 不回覆（T3 timeout）、7 個 RCMD 沒分支、外加 `START_AGV` 參數處理的 2 個實作缺陷。

---

## 1. Message 層（SxFy primary）

| 分類 | 內容 |
|---|---|
| **兩邊都有（都會真的回覆）18 項** | S1F1, S1F3, S1F11, S1F13, S1F15, S1F17, S2F13, S2F15, S2F17, S2F25, S2F31, S2F33, S2F35, S2F37, S2F41, S5F3, S5F5, S5F7 |
| **只有 9045（HT160 完全沒 dispatch → 只記 log 不回，host T3 timeout）22 項** | S1F23, S2F23, S2F29, S2F43, S6F15, S6F17, S6F19, S6F23, S14F3, S100F3, S101F1/F3/F5/F7/F11, S103F11, S110F2/F6/F8, S120F2, S125F1, S125F3 |
| **9045 有、HT160 有 dispatch 但是空殼（不回覆）7 項** | S7F1, S7F3, S7F5, S7F17, S7F19, S10F3, S10F5 |
| **只有 HT160** | S14F1（GetAttr；9045 走的是 S14F3 Get2DID_BinCode，兩邊 S14 不相容） |

京元 host 當天實際送出：`S1F1 S1F3 S1F13 S1F17 S2F15 S2F33 S2F35 S2F37 S2F41 S5F3 S6F15 S6F19 S10F3 S10F5 S125F1`
（機台側送出 `S5F1 S6F11`）

> **HOT — host 真的送了、HT160 不會回：`S6F15`、`S6F19`、`S10F3`、`S10F5`、`S125F1`**
> 對應計次：S6F15×3、S6F19×8、S10F3×1、S10F5×7、S125F1×6。
> 這 5 項與 [[secs-pathA-ht9045-align]] 之前列的 deferred 清單一致，但**現在有現場證據證明 host 會送**，
> 不再只是 discovery-gated 的推測 —— 建議至少補「最小 ack」。
> 另注意 9045 側 S125F2 送了 138 次而 S125F1 只收到 6 次 → S125F2 在 9045 是**機台主動**送的 EC 變更回報。

---

## 2. Remote Command 層（S2F41 RCMD）

| 分類 | 數量 | 內容 |
|---|---|---|
| **兩邊都有** | 8 | `LOTSTART` `ONLINE_LOCAL` `ONLINE_REMOTE` `PAUSE` `SET_LOT_INFO` `START` `START_AGV` `STOP` |
| **只有 HT160** | 3 | `CLEARCOUNT` `HOME` `ONLINE`（`ONLINE_REMOTE` 的裸別名） |
| **只有 9045** | 39 | `AUTHORITY_CHECK` `AUTOSITEMAP` `AUTO_CLEAN` `AUTO_RETEST` `CLEAN_AUTO_SORT_COUNT` `CLEAN_OUT` `CLEAR_LOT_INFO` `CLOSE_ONECYCLE` `CONTINUE_RETEST_ART` `CONTINUE_START_ART` `CONTINUE_START_MRT` `DEVTEMPOFFSETADJUST` `DOWNLOAD_RECIPE_BY_FTP` `EESUG_OFFSET` `ENERGY_SAVING` `HALT` `INITIAL_START` `INITIAL_START_ART` `INITIAL_START_MRT` `LOTORDER` `ONE_CYCLE` `PP_MUSIC` `PP_PASSWORD` `PP_SELECT` `PP_SIGNALTOWER` `REMOTE_SAVE` `REMOTE_START` `REMOTE_UPDATE_PROGRAM` `RESET` `RETEST_MRT` `START_AQL` `START_LOT` `STOP_LOT` `SWITCH_TO_FT` `SWITCH_TO_RT` `TESTTEMPSETTING` `TRAY_FEED` `TRAY_MAP` `YIELD_FAIL` |

京元 host 當天實際送出 10 個：
`CLEAN_AUTO_SORT_COUNT`(2) `ENERGY_SAVING`(23) `INITIAL_START_ART`(1) `LOTSTART`(3) `ONE_CYCLE`(11)
`PP_MUSIC`(15) `PP_SIGNALTOWER`(15) `START`(7) `START_AGV`(4) `SWITCH_TO_FT`(1)

> **HOT — host 真的送了、HT160 沒有分支（會回 HCACK≠0）7 項：**
> | RCMD | 次數 | HT160 是否該做 | 說明 |
> |---|---|---|---|
> | `ENERGY_SAVING` | 23 | **建議做** | 參數 `STATE=0/1`；純節能狀態切換，Sorter 適用 |
> | `PP_SIGNALTOWER` | 15 | **建議做** | 參數 `RED/YELLOW/GREEN`；塔燈遠端控制，Sorter 適用 |
> | `PP_MUSIC` | 15 | **建議做** | 參數為單值（範例 `=1`）；蜂鳴器控制，Sorter 適用 |
> | `ONE_CYCLE` | 11 | **建議做** | HT160 有 One Cycle 功能，只是沒開 RCMD |
> | `CLEAN_AUTO_SORT_COUNT` | 2 | 可評估 | 清 Auto 分選計數；HT160 已有 `CLEARCOUNT`，可考慮加別名 |
> | `INITIAL_START_ART` | 1 | **N/A** | ART＝測試機專屬流程，Sorter 無對應 |
> | `SWITCH_TO_FT` | 1 | **N/A** | FT/RT 測試程式切換，Sorter 無對應 |

### 2-1. `START_AGV` 參數比對（**發現 2 個 HT160 實作缺陷**）

京元 host 實際送出的 payload（`SECSGEM_TextLog_15.txt:504`，9045 回 HCACK=0x00）：

```
START_AGV  L[10]
  Loader=Action  Empty=NA  Color=NA
  AUTO1=NA  AUTO2=NA  AUTO3=NA  AUTO4=NA  AUTO5=NA
  LoaderTrayCount=1   LoaderICCount=3
```

對照兩邊實作：

| 行為 | HT9045（`uHGemHT9045.cpp:1655`） | HT160S（`uHGemHT160.cpp:1094`） |
|---|---|---|
| 未知 CP 名稱 | 靜默忽略，`HCACK=0` | `BeginPrep()` 查無站名 → **`HCACK=2`（整包拒絕）** |
| `LoaderICCount` | 無此分支＝被忽略 | 無此分支 → 落入上一列 → **整包 HCACK=2** |
| CP 值判斷 | **只在 `值=="Action"` 時才動作**，`NA` 不動作 | **完全不看值**，任何站名都直接 `BeginPrep()` |
| Auto 站數 | AUTO1–3 | AUTO1–6（HT160 為超集，OK） |

> **缺陷 1（會整包被拒）**：host 每次 `START_AGV` 都帶 `LoaderICCount`，HT160 會回 `HCACK=2`，AMR 上料握手直接失敗。
> **缺陷 2（會誤動作）**：host 用 `NA` 表示「這站不要動」，HT160 會把 `Empty / Color / AUTO1~5` **全部**設成 `AGV_PREP` 並上鎖
> （`BeginPrep()` 內含 `InfeedSetLock()` / `SetAmrLock()`），等於 host 只叫 Loader，HT160 卻鎖了 8 個站。
>
> 這兩點在 SECS 模擬器上不會出現（模擬器只送要動的站、且不帶 `LoaderICCount`），
> 只有拿現場 log 對才看得到。修法：`START_AGV` 迴圈改成「未知 CP 名稱忽略不報錯」＋「只有 `值=="Action"` 才 `BeginPrep()`」。
> 這也修正了記憶中 [[amr-updown-ic-count-contract]]「上料只交換盤數不給 IC 數」的敘述——
> host **確實**在上料時一併送了 `LoaderICCount`（9045 是忽略它，不是沒收到）。

---

## 3. Event 層（CEID）

- 9045 韌體目錄：**286 個具名 CEID**（編號到 292）
- HT160S：**41 個已註冊**（1–31、35–37、148–150、272–275）＋ **6 個有發射但未註冊**（136–138、140–142，`aAuto1To6.cpp:742`）＝ 47
- 號碼在兩邊都存在：46 個 → 其中**語意一致只有 13 個，衝突 33 個**

### 3-1. 兩邊都有且語意一致（安全）13 個

| CEID | HT9045 | HT160S |
|---|---|---|
| 35 / 36 / 37 | Auto1 / 2 / 3 Full | Auto1 / 2 / 3 Full |
| 136 / 137 / 138 | Auto 1 / 2 / 3 Unloading tray | Auto1 / 2 / 3 Unloadtray |
| 148 / 149 / 150 | Auto 4 / 5 / 6 Full | Auto4 / 5 / 6 Full |
| 272 | AMR Supplement | AGVSupplement |
| 273 | AMR LDUnLD Status | AGVLDUnLDStatus |
| 274 | AMR LDUnLD Finish | AGVLDUnLDFinish |
| 275 | AMR LD ID | AGVLdID |

> 也就是說：**AMR 那一段（272–275）與 Auto Full/Unload 那一段是唯一真正對齊的區塊**，其餘全部各走各的。

### 3-2. 同號不同義（host 會誤讀）33 個

`1–31` 整段撞號。挑幾個殺傷力最大的：

| CEID | HT9045 意義 | 當天發射次數 | HT160S 同號的意義 |
|---|---|---|---|
| 27 | Change Machine State | **406（最高）** | One Cycle Finish |
| 2 | Pause Pressed | 24 | Recipe Change |
| 14 | Switch StartMode | 16 | Press Retry button |
| 26 | Get Test Result | 12 | Enter SECS GEM Page |
| 28 | Retry Pressed | 9 | Clean Out Finish |
| 1 | Start Pressed | 5 | Handler change status |
| 6 | Lot Start | 2 | Press Pause button |
| **140** | **Prepare Load Tray** | 0 | **Auto4 Unloadtray** |
| **141** | **GEM Control State Change** | 0（被 disable 擋下 3 次） | **Auto5 Unloadtray** |

（完整 33 筆見 `scratchpad\compare_tables.md` C-2 表）

> **額外發現（編號 bug）**：9045 的 Auto4/5/6 Unloading tray 是 **145 / 146 / 147**，
> 但 HT160 的 `AutoCeid[6]={136,137,138,140,141,142}` 把 Auto4/5/6 放在 **140/141/142**，
> 正好撞到 9045 的 140（Prepare Load Tray）與 141（GEM Control State Change）。Auto1–3（136/137/138）是對的。
>
> **更正（20260728 複查）**：初版此處寫「未註冊所以發了也不會送出」是**錯的**。
> `THGem::EventReport()`（`uHGemEquipment.cpp`）在 `FindCEIDItem()` 回 NULL 時 `reportCount=0`，
> 但仍照常 `InitLocalHead(6,11,1)` → `SendLocalData()`，**S6F11 會實際送上線**，只是報表清單為空 `L[0]`。
> 也就是說撞號是**現正發生**、不是潛在風險。
> 附帶驗證：9045 送 CEID 136 時同樣是 `L[0]` 空報表（`SECSGEM_TextLog_18.txt:445`），
> 所以 HT160「不註冊 → 空報表」的設計本身是 9045-faithful，**唯一要修的就是編號**。
>
> **已修**：`aAuto1To6.cpp` `AutoCeid[6]` 改為 `{136,137,138,145,146,147}`；sim `-Clean`、
> 真機 `-Full`（關 `SOFT_SIMULATE`）、還原後 `-Full` 三次建置皆 exit 0。

### 3-3. 只有 9045 有（240 個號碼）

其中當天**實際發射過**、HT160 完全沒有對應事件的有 22 個，這是 host 端已在訂閱的：

| CEID | 意義 | 次數 | | CEID | 意義 | 次數 |
|---|---|---|---|---|---|---|
| 80 | Read Now Handler Data | 10962（被 disable 攔） | | 66 | Load Tray Finish | 5 |
| 44 | Site On Off | 31＋3 | | 53 | UPH Record Start | 5 |
| 70 | Barcode Reader Enter | 13 | | 25 | Home Pressed | 4 |
| 76 | Start Pressed HasIC | 13 | | 41 | One Cycle Finish | 3 |
| 123 | Safe Door On Off | 9 | | 42 | Clean Out Finish | 2 |
| 124 | Save Recipe | 8 | | 47 | Change HandlerSpeed | 2 |
| 73 | Mymessbox OK | 7 | | 212 / 213 | Energy Saving Start / End | 1 / 1 |
| 67 | Tray Test Finish | 6 | | 250 | START Auto contact height | 1 |
| 93 | SECS/GEM Online Remote | 3（被 disable 攔） | | 34/49/50/55/58/59/60/63 | Auto Clean、ART 流程等 | 各 1–2 |

> 值得注意：`41 One Cycle Finish` / `42 Clean Out Finish` / `49 Tray Feed Finish` 在 9045 是這三個號碼，
> HT160 卻用 `27 / 28 / 29`。**同一件事、不同號碼，而且那三個號碼在 9045 另有他義**。

### 3-4. 只有 HT160 有

`142`（Auto6 Unloadtray）—— 9045 該號碼保留未具名。

---

## 4. SVID / ECID / Alarm 概況（附帶）

| 項目 | HT9045 | HT160S | 交集 |
|---|---|---|---|
| SVID 註冊數 | 866 | 51 | 26 |
| ECID 註冊數 | 1689 | 7 | **0** |
| 京元 host 在 S2F33 引用的 SVID | 445 個 | HT160 只有其中 26 個 | 缺 419 |
| 京元 host 用 S1F3 直接輪詢的 SVID | 50 個 | HT160 只有 `1001` `1003` | 缺 48 |
| 京元 host 用 S2F15 寫入的 ECID | `1006 1007 16000 16002 16023 16026 35011 37007` | 全部沒有 | **0** |
| S5F1 告警 | ALID `2`/`3`，ALTX 為純數字碼（`209000962`／`316001640`） | ALID＝alarm code 的 31-bit hash，ALTX＝code+訊息 | 編碼規則不同 |

> ECID 交集為 0：HT160 的 EC（1501 Recipe、2758–2763 Tray 幾何）雖然「對齊 9045 編號」，
> 但京元 host 當天一個都沒碰；host 實際寫的 8 個 ECID HT160 全無。
> 另外 host 用 **S1F3 輪詢 SVID 2762/2763**，而 HT160 把 2762/2763 拿去當 **ECID**（Tray X/Y Division）——
> SV 與 EC 是不同命名空間所以不會衝突，但也代表這個「對齊」對 host 沒有實際效果。

---

## 5. 建議處理順序

**P0 — 有現場證據、成本低、直接影響 AMR 上料**
1. `START_AGV`：未知 CP 名稱改為忽略（勿回 HCACK=2）；加上 `值=="Action"` 才動作的判斷。（缺陷 1＋2）
2. CEID `AutoCeid[]` Auto4–6 由 `140/141/142` 改為 `145/146/147`（目前未註冊，改動代價最低）。

**P1 — host 真的會送但 HT160 不回（T3 timeout）**
3. 補最小 ack：`S6F15/F16`、`S6F19/F20`、`S10F3/F4`、`S10F5/F6`、`S125F1/F2`。

**P2 — host 真的會送的 RCMD，且 Sorter 適用**
4. `ENERGY_SAVING`、`PP_SIGNALTOWER`、`PP_MUSIC`、`ONE_CYCLE`（`CLEAN_AUTO_SORT_COUNT` 可評估做成 `CLEARCOUNT` 別名）。

**P3 — 需要與客戶確認，不要自己改**
5. CEID 1–31 撞號：HT160 已在規格書 §7.2 聲明「同號不同義，host 須以本規格書為準」。
   要嘛維持現況（host 端設定），要嘛整段 renumber。**這是商務決策，不是工程決策** ——
   維持 [[secs-pathA-ht9045-align]] 既有結論：renumber 屬 LOW / discovery-gated，不主動動。

---

## 6. 產出檔案

全部存放於 `D:\HT160S_BCB\docs\plan\secs-9045kyec-diff-20260728\`：

- `kyec9045_surface.json` — 9045 現場指令面（機器可讀：messages / CEID / RCMD＋參數 / RPTID-SVID / ECID / ALID）
- `compare_result.json` — 三集合比對結果（機器可讀）
- `compare_tables.md` — 完整 C-1 / C-2 / C-3 表
- `parse9045.py` — log 遞迴 SML parser（可重跑其他日期的 log）
- `compare.py` — 比對與表格產生器
