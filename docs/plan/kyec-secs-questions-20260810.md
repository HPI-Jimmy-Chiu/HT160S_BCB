# 京元 EAP 2026-08-10 SECS 四問 — 分析與待確認事項

> 來源：KYEC `.EAP_Group` 溫浚諺(CYWen1) 來信（四點）。
> **狀態（2026-08-10 二次更新）**：**Q1 與 Q2 都已改 code 並上線**
> （Q1 = `b51e2b2` 嚴格報表驗證，見 §1.6；Q2 = `2bfdb3c` 採用家族 Record Auto Tray Count，見 §2.2/§2.4）。
> Q3 / Q4 仍只做分析，未改 code。§5 尚未關閉的只剩 Q1'、Q2-d、Q2-e、Q2-g、Q4-a、Q4-b、Q4-c。
>
> ⚠ **本文 §2 已於 2026-08-10 因取得新版 9045 而大幅改寫**：先前「11 個版本查無 38237-38239」的
> 結論**已作廢**。若貴端手上是舊版本文，請以本版為準。
> 所有結論都附 code 位置或現場 log 佐證；查不到的地方一律列成問題，不猜。
> 參照樹（唯讀）：`D:\HT9045\` 下 11 個 HT-90XX 版本，本文主要引用
> `HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP`（下稱 **810_B01**）、
> `HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422`（**899**）、
> `HT9011UC_Code_V3.33.906.0_20260618`（**906**）。

---

## 摘要（一句話結論）

| # | 客戶提問 | 結論 |
|---|---|---|
| 1 | S2F33/S2F35 不應有容忍行為 | **已照辦、已上線**（2026-08-10，commit `b51e2b2`）：未定義的 SVID / CEID / RPTID 一律**整封拒絕**。原本擔心的「切嚴格會死掉 29/33 張報表」在 2026-08-07 現場 log 鑑識後**不成立**——貴端 host 當天**一筆 S6F11 都沒收到過**，嚴格模式只是把既有的靜默失敗變成看得見的拒絕。見 §1.5 / §1.6。 |
| 2 | 請調整 SVID 38237~38239 | **已照辦、已上線**（2026-08-10，commit `2bfdb3c`）。我方取得新版 9045 後確認 **HT9046LS 810_B01 已定義**這三個號為 `Record Auto 1/2/3 Tray Count`（廠內 2026-07-10 新增），我方照家族定義實作。原本佔用的 AMR Auto4-6 Tray Count 已讓位到 **38246-38248**。尚待貴端兩點：RPTID 524 綁在永不發射的 CEID 67（§2.5）、多批機台的清除時機（§5 Q2-e）。 |
| 3 | 66030 與 66031 都對應到 1006？ | **是**，兩者都由 1006 推導但取法不同（一個取逗號數、一個取第一段）。範例見 §3。 |
| 4 | 每項 SVID 語意都與 HT-90XX 相同？ | **67 個號碼中 48 個完全同名同義**；其餘分三類差異，逐項列於 §4。另外貴端 1517 表格的 `Max=8` 與 `Description 0..11` 互相矛盾——**矛盾源頭在 HT-90XX 韌體本身**，說明見 §4.3。 |

---

## 1. S2F33 / S2F35 的「容忍行為」

### 1.1 事實：兩邊現在的行為

**HT-160S（2026-07-27 ~ 2026-08-10 的舊行為，現已改掉 — 見 §1.6）** — 刻意容忍：

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

> **§1.4 已由 §1.6 取代。** 下面 §1.5 的現場鑑識推翻了「切嚴格會造成損失」這個前提，
> 因此我方沒有等 A/B/C 的裁決就直接採 B（嚴格，不補假字典）。

### 1.5 現場實證：2026-08-07 貴端機台 log 鑑識

證據：`D:\HT160S_StateRecord\2026-08-07 17_31_22\SecsLog\2026_08_07\SECSGEM_TextLog_14~17.txt`（全天 5,083 行）。
以下每個數字都是對該 log 實際掃描所得，非推估。

當天 15:09 ~ 17:31 共 **8 個 HSMS session**，每個 session 貴端 host 的動作**完全相同**：

| 訊息 | 筆數 | 我方回覆 | 實際內容 |
|---|---|---|---|
| S2F33 Define Report | **64**（8 session × 8 筆） | S2F34 **DRACK=0x00 全接受** | 每 session 1 筆 `L[0]`（刪除全部）＋ **7 張報表定義** |
| S2F35 Link Event Report | **8** | S2F36 **LRACK=0x00** | **8 筆全是 `<L[2] <U4 1> <L[0]>>` ＝ UNLINK ALL** |
| S2F37 Enable/Disable Event | **8** | S2F38 **ERACK=0x00** | **8 筆全是 `<L[2] <Boolean 0x00> <L[0]>>` ＝ DISABLE ALL** |

**全天 80 個 ACK body 全部是 `<B[1] 0x00>`，零拒絕。** 由此得到兩個彼此獨立的事實：

**(1) 貴端引用的 54 個 SVID，HT-160S 100% 未定義 —— 而且我方靜默吞掉了。**
log 中有 **432 行** `accept unknown SVID`（＝ 54 個號 × 8 個 session）。舊的容忍邏輯只寫一行 log、
照樣回 DRACK=0x00，**所以貴端 host 端完全看不到任何錯誤**。
「HT-160S 不支援」的真相是「**訊息支援、字典沒號、而且刻意靜默接受**」——這正是本次修掉的行為（§1.6）。

**(2) 但當天沒有資料上傳，主因不是 SVID。**
貴端 host 定義完 7 張報表後，**從未 link 任何 CEID**（8 筆 S2F35 全是 UNLINK-ALL）、
**也從未 enable 任何 CEID**（8 筆 S2F37 全是 CEED=FALSE ＋ DISABLE-ALL）。結果：

| S6F11 | 筆數 |
|---|---|
| HT-160S 嘗試發出 | **567** |
| 被擋：`suppressed (CEID disabled by host)` | 508 |
| 被擋：`skipped (not selected)` | 59 |
| **實際送上線** | **0** |

（真正送出的訊息會記 `len=.. (sent)` 與 `body:` 兩行，例如 S1F14；S6F11 全天一筆都沒有。）

> ⚠ **關鍵推論：就算 HT-160S 把這 54 個號全部補進字典，2026-08-07 當天一樣不會有任何資料上傳。**
> 貴端 EAP 缺的是 **S2F35 link ＋ S2F37 enable(CEED=TRUE)** 這兩步。
> 這也代表 §1.3 估的「切嚴格會死掉 29/33 張報表」**在實務上不是損失**——那些報表本來就沒在送。

#### 1.5.1 那 54 個 SVID 逐號清單

名稱一律回源 `HT9046LS 810_B01` 的 `uHGemHT9045_SV.cpp` / `uHGemHT9045_EC.cpp`，
54 個**全部**查得到且逐字相符。判定：`—`＝本機無此硬體、`○`＝可實作、`△`＝已有他號可對應。

**RPTID 804 — ATC 換座系統 ＋ 溫控帶**（12 個）

| SVID | 9046LS 810_B01 名稱 | HT-160S |
|---|---|---|
| 1040 | `ATC SYSTEM` | —（本機無此硬體） |
| 1043 | `ATC State` | —（本機無此硬體） |
| 4880 | `Temp Low OffSet` | —（本機無此硬體） |
| 4881 | `Temp Mid OffSet` | —（本機無此硬體） |
| 4882 | `Temp High OffSet` | —（本機無此硬體） |
| 4883 | `Temp User OffSet` | —（本機無此硬體） |
| 4884 | `Temp Single Limit` | —（本機無此硬體） |
| 4885 | `Temp HotLow OffSet` | —（本機無此硬體） |
| 4886 | `Temp HotMid OffSet` | —（本機無此硬體） |
| 4887 | `Temp Init OffSet` | —（本機無此硬體） |
| 4888 | `Temp EOT OffSet` | —（本機無此硬體） |
| 38835 | `Base-point` | —（本機無此硬體） |

**RPTID 994 — 各子模組軟體版本**（8 個）

| SVID | 9046LS 810_B01 名稱 | HT-160S |
|---|---|---|
| 37202 | `ATC Software Version` | —（本機無此硬體） |
| 37203 | `GPIB Software Version` | —（本機無此硬體） |
| 37228 | `ESD Software Version` | —（本機無此硬體） |
| 37529 | `OCR Software Version` | —（本機無此硬體） |
| 37530 | `2D Barcode Software Version` | ○ 可實作（讀碼器存在，但目前未存版本字串） |
| 37531 | `RTC Software Version` | —（本機無此硬體） |
| 37532 | `AOA Software Version` | —（本機無此硬體） |
| 37533 | `Handler Software Minor Version` | △ SVID 1003 部分覆蓋（本機只有一組合併版本字串，無獨立 minor 欄） |

**RPTID 995 — OCR 模組**（2 個）

| SVID | 9046LS 810_B01 名稱 | HT-160S |
|---|---|---|
| 38806 | `Has OCR Module` | —（本機無此硬體） |
| 38834 | `OCR Function Enable` | —（本機無此硬體） |

**RPTID 996 — Auto Clean 探針清潔套件**（15 個）

| SVID | 9046LS 810_B01 名稱 | HT-160S |
|---|---|---|
| 9501 | `Auto Clean Function` | —（本機無此硬體） |
| 9517 | `Auto Clean Interval Contact Count` | —（本機無此硬體） |
| 9518 | `Auto Clean Mode` | —（本機無此硬體） |
| 9521 | `Auto Clean Device Number of pices` | —（本機無此硬體） |
| 9523 | `Auto Clean Alarm Count` | —（本機無此硬體） |
| 9524 | `Auto Clean Pad Deviation` | —（本機無此硬體） |
| 9532 | `Auto Clean Contact Time` | —（本機無此硬體） |
| 9533 | `Auto Clean Contact Count` | —（本機無此硬體） |
| 9552 | `Auto Clean Form X Pitch` | —（本機無此硬體） |
| 9553 | `Auto Clean Form Y Pitch` | —（本機無此硬體） |
| 9554 | `Auto Clean Form X Start` | —（本機無此硬體） |
| 9555 | `Auto Clean Form Y Start` | —（本機無此硬體） |
| 9556 | `Auto Clean Form X Division` | —（本機無此硬體） |
| 9557 | `Auto Clean Form Y Division` | —（本機無此硬體） |
| 9558 | `Auto Clean Kit Mode for HT9045` | —（本機無此硬體） |

**RPTID 997 — 2D 讀碼策略 / 連續失敗 / 讀碼良率**（9 個）

| SVID | 9046LS 810_B01 名稱 | HT-160S |
|---|---|---|
| 38821 | `Handling without 2DID` | ○ 可實作（行為已存在＝虛擬 2D 守衛） |
| 38825 | `Auto skip and set unread device 2DID to ERROR` | ○ 可實作（見下方假朋友警告 3） |
| 38826 | `Check duplicate code by lot` | ○ 可實作（匯入時已檢查，未開 SVID） |
| 38827 | `Consecutive Failure` | —（本機無此硬體） |
| 38828 | `Consecutive Failure Count` | —（本機無此硬體） |
| 38829 | `Set close site 2DID to empty` | —（本機無此硬體） |
| 38830 | `Alarm when 2DID yield is less then setting` | ○ 可實作（目前無對應功能） |
| 38831 | `2DID yield setting Count` | ○ 可實作（目前無對應功能） |
| 38832 | `2DID yield setting Percentage` | ○ 可實作（目前無對應功能） |

**RPTID 998 — Magazine 模組 ＋ Bin-Tray 對應**（5 個）

| SVID | 9046LS 810_B01 名稱 | HT-160S |
|---|---|---|
| 10698 | `Bin Tray Linked` | △ 已有他號（38234-38236 / 38243-38245，**但方向相反**，見下） |
| 11098 | `Bin Tray Linked` | —（本機無此硬體） |
| 11298 | `Bin Tray Linked` | —（本機無此硬體） |
| 11398 | `Bin Tray Linked` | —（本機無此硬體） |
| 38833 | `HAS MAGAZINE Module` | —（本機無此硬體，見假朋友警告 2） |

**RPTID 999 — 測試座接觸警報計數**（3 個）

| SVID | 9046LS 810_B01 名稱 | HT-160S |
|---|---|---|
| 2015 | `Contact Alarm Count Setting Grop 1` | —（本機無此硬體） |
| 2016 | `Contact Alarm Count Setting Grop 2` | —（本機無此硬體） |
| 2017 | `Contact Alarm Count Setting Grop 3` | —（本機無此硬體） |

#### 1.5.2 統計與建議

| 分類 | 個數 | 說明 |
|---|---|---|
| **本機無此硬體**（不適用） | **45** | 測試座 / 溫控槽 / ATC 換座 / OCR / 探針清潔套件 / Magazine 管裝 —— HT-160S 是 tray sorter，這些機構一個都沒有 |
| **可實作**（硬體或行為存在，只是沒開 SVID） | **7** | 37530、38821、38825、38826、38830、38831、38832（**全部集中在 2D 讀碼這一塊**） |
| **已有他號可對應** | **2** | 10698（→ 38234-38236 / 38243-38245，方向相反）、37533（→ 1003，部分覆蓋） |

> 我方建議：**不要為了讓報表通過而替這 45 個號製造假值。** 誠實的答案是告訴 host
> 「這些量在本機型不存在」。真正值得談的只有 2D 那 7 個號，以及 10698 要不要改成
> 貴端解析器慣用的 bin-indexed 方向。

**⚠ 三個「假朋友」（同名不同義，硬對映會給出錯誤資料）**

1. **`Auto Clean`（9501-9558）** — 9045 指的是把清潔片推進**測試座**清接觸針。
   HT-160S 唯一含 `AutoClean` 字樣的是 `GeneralSetting.iAutoCleanOutRiseDwellMs`
   （`GeneralSetting.cpp:269`），那是 **Clean Out 排空機台**的停留時間，兩者毫無關係。
2. **`HAS MAGAZINE Module`（38833）** — 9045 指管裝（tube）媒體模組。
   HT-160S 原始碼裡的 "magazine" 指的是 **AMR 載盤車**（`aLoader.h:46`、`GeneralSetting.h:39/279`）。
   把後者報在 38833 上，等於告訴貴端這台 sorter 有管裝機構——**是假的**。
3. **`Auto skip ...`（38825）** — 9045 的觸發條件是 **2D 讀碼失敗**。
   HT-160S 的 `bSortArmAutoSkipOnPickFail`（`GeneralSetting.cpp:235`）觸發條件是 **吸取真空失敗**。
   字面相同、觸發源不同，不可互相對映。

### 1.6 裁決與實作（2026-08-10 已上線）

我方裁決採 **§1.4 的 B 案**：**嚴格，且不替本機沒有的量補假字典**。
理由就是 §1.5——容忍模式讓一整天的失敗完全隱形，這比報表被拒更危險。

已實作並上線（commit `b51e2b2`）：

| 情境 | 新回覆 | 位置 |
|---|---|---|
| S2F33 含任一本機未定義的 SVID | **DRACK=0x04**，整封拒絕、**一張報表都不建立** | `SecsGem/uHGemEquipment.cpp` `ProcessDefineReport_S2F33` |
| S2F35 連到本機沒有的 CEID | **LRACK=0x04**，整封拒絕、**一條連結都不建立** | 同檔 `ProcessLinkEventReport_S2F35` |
| S2F35 連到不存在的 RPTID | **LRACK=0x05**，同上 | 同檔 |

三個實作要點：

1. **原子且延後拒絕** — 違規號碼先記下來，**整封訊息仍然解析到底**（收訊路徑不會失步），
   拒絕在解析迴圈之後、commit 迴圈之前送出。所以「一個號錯 → 整封不生效」，與 HT-90XX 同形
   （`uHGemEquipment.cpp:7937-7940`、`uHGemClass.cpp:1300-1312` 只在 `:1314` commit）。
2. **開機還原也套同一規則** — HT-160S 會把上次 host 定義的報表存進 `EventReportDef.ini` 並於開機重建。
   若不一起改，跑過容忍模式的機台**每次開機都會把嚴格模式剛拒絕掉的報表原封不動重建回來**。
   現在改成**整張報表丟棄**（絕不只丟其中幾個 SVID —— 那會讓 S6F11 的欄位位移）。
3. **現場逃生口** — `system\General.ini` 的 `[SECS] StrictReportValidation=0` 可**不重編**退回舊的容忍行為。
   這是給試機/量產不能等的情況用的開關，預設為 `1`。

**建置驗證**：模擬版與真機版（關閉 `SOFT_SIMULATE`）全建置皆 exit 0。

> **對貴端的影響（請務必先看）**：此版上線後，貴端 host 若沿用 2026-08-07 那 7 張報表，
> **S2F33 會直接收到 DRACK=0x04，7 張全部建立不起來**。
> 請依 §4.1 的 67 個號重下 S2F33，並補上 **S2F35 link ＋ S2F37 CEED=TRUE** 這兩步。

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

### 2.2 家族出處：已找到（2026-08-10 更新，前一版結論已作廢）

**本節前一版寫「11 個版本全文搜尋為零、請貴端提供資料」。取得新版 9045 後，該結論已作廢。**

新版參照樹 **HT9046LS 810_B01** 確實定義了這三個號：

```
D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\SECSGEM\uHGemHT9045_SV.cpp:861-863
  38237  INT_4  "Record Auto 1 Tray Count"  -> LastSet.RecodeTrayCount[0]   //Frank 20260710 ADD
  38238  INT_4  "Record Auto 2 Tray Count"  -> LastSet.RecodeTrayCount[1]
  38239  INT_4  "Record Auto 3 Tray Count"  -> LastSet.RecodeTrayCount[2]
```

我方把 `D:\HT9045\` 底下 **17 個 HT-90XX 版本樹**重掃一次：
**只有 810_B01 有這三個號，其餘 16 版皆為零**。所以先前的「查無」是版本問題，不是我方漏查。

**語意（9045 內部依 `CUSTOMER_CODE` 分成兩種，這點很重要）**：

| 客戶碼 | 行為 | 位置 |
|---|---|---|
| `CC_TSI` | 累加到 **10 就寫一次 summary log 並歸零**（0~9 滾動） | `asendic_Auto.cpp:628-633` |
| **`CC_KYEC_XILINX`** | **純累加，不歸零** | `asendic_Auto.cpp:640` |
| 清零 | **Lot End**（`RunInfo.bLotStart=false` 之後）與 Counter Clear | `uLotInfo.cpp:1171-1173`、`cCounterClear.cpp:242` |

→ **京元分支的語意 =「該 Auto 軌道本批已產出盤數，Lot End 清除」，與貴端來信描述完全一致。**
HT-160S 採用**京元這一支**的語意，不做 `CC_TSI` 的 10 盤歸零。

附帶：`BU5` 在原始碼裡是 **`USE_BU5_Function` 這個 KYEC 專用功能開關**
（810_B01 有 80 處、906 有 55 處），**不是版本號**——這點不變，但既然號碼已找到，
就不再需要請貴端指認版本（原 §5 Q2-a 撤銷）。

### 2.3 另一組同義號 37003-37005 / 37013-37015（保留備查，**已非採用方案**）

> 本節是 §2.2 結論翻轉前的替代方案。既然 38237-38239 本身就是家族號，
> 我方直接採用該三號，**不再改用 37003-37005**。以下保留供貴端交叉核對語意。

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

### 2.4 HT-160S 的實作（2026-08-10 已上線，commit `2bfdb3c`）

這確實是一個**新功能**——HT-160S 先前沒有任何「每軌道本批已產出盤數」的計數器
（`Car[].iTrayCount` 只是 AMR 車上現有盤數，車一離站就歸 0）。已新增：

| 項目 | 實作 |
|---|---|
| 新計數器 | `tRunData.RecordTrayCnt[]`（`cprod.h`），與既有的 `TrayICCnt[]` 並列 |
| **累加點** | `aAuto1To6.cpp` `DoDischargeTray`——**整盤自 Auto 出料完成的唯一位置**，就在該軌道 Unloadtray CEID（136/137/138/145/146/147）發射的前一行。與 9045 同一個事件點 |
| 清零點 | `ResetPerLotProductionCounters()`（`cprod.cpp`）＝ **Lot Start**，與其他所有 per-lot 計數器一致 |

**號碼配置**

| SVID | 名稱 | 來源 |
|---|---|---|
| 38237 / 38238 / 38239 | `Record Auto 1/2/3 Tray Count` | **家族號**（810_B01） |
| **38249 / 38250 / 38251** | `Record Auto 4/5/6 Tray Count` | **HT-160S 擴充**——家族只編到 Auto3（810 的 AMR 帶止於 38236、899 沒有 AMR 家族） |

**兩處與 9045 刻意不同（都已在程式碼註解記錄）**

1. **不做客戶碼分流**：9045 的同一個計數器在 `CC_TSI` 下會 10 盤歸零；HT-160S 只實作京元語意（純累加）。
2. **清零時機：Lot Start，不是 Lot End。** HT-160S 最多可同時掛 64 批，
   「剛結束的那一批」無法指認一個**軌道**計數器裡的盤是誰的，任一批 Lot End 就清會清掉別批的數。
   改在 Lot Start 清，計數器在整個工單期間都代表「本工單」，且 Lot End 後數值仍讀得到而不是被抹成 0。
   → **這點仍需貴端確認（§5 Q2-e）**；若貴端要求嚴格照 9045 在 Lot End 清，我方可改。

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

### 2.6 與貴端 2026-08-04「只用家族號」裁示的關係：**不衝突**（結論已更新）

> 本節前一版判定 Q2 牴觸貴端 0804 裁示三處。**§2.2 找到家族出處後，三處全部消失。**
> 保留本節是因為其中一項風險已經應驗，值得雙方記錄。

**0804 裁示**（`SecsGem/uHGemHT160.cpp:650-691` tombstone）：HT-160S 只發布**家族號**；
`66000-66039` 整段 HT-160S 專屬號下架，且**永久保留、絕不重用**——因為讀未註冊號碼回空 `L[0]` 是
**可偵測**的，一旦重用就會從「可偵測的空白」變成「**無法偵測的錯值**」。

| 原判定的衝突 | 現況 |
|---|---|
| 38237-38239 是新造號，不是家族號 | **不成立**。810_B01 已定義，是不折不扣的家族號（§2.2） |
| 應改用家族同義號 37003-37005 | **不需要**。38237-38239 本身就是家族號，直接採用 |
| 重新定義已在用號碼的語意，正是 0804 要防的事 | **仍然成立，而且已經發生**（見下） |

**第 3 點請貴端特別留意**：38237-38239 在 HT-160S 原本是 **AMR Auto4-6 Tray Count**，
且已寫進 2026-08-02 交付貴端的規格書。改採家族語意後，
**在 2026-08-10 之前佈建的 host，讀這三個號拿到的不再是 AMR 車上盤數，而是 Record 累計盤數**——
數值型別相同、看起來都合理，**但意義不同且線上無法分辨**。這正是 0804 裁示要防的情況，
所以我方在此明確告知，不讓貴端自己去發現。AMR Auto4-6 Tray Count 已搬到 **38246-38248**。

**教訓（雙方都適用）**：在家族自己會成長的號段裡自創號碼，遲早會被家族追上。
38237-38239 就是實例——我方 2026-08-03 查證「家族此段沒有號可用」後才擴充進去，
七天後家族就補上了。我方已在 `uAgvStation.cpp` 的號碼表旁留下註記，**每次 9045 升版都要重跑這項比對**。

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
| ~~Q1~~ | ~~S2F33/S2F35 要的是哪一種？~~ | **已結案**：我方 2026-08-10 採 B 案並上線（§1.6）。0807 log 證明容忍期間貴端一筆 S6F11 都沒收到（§1.5），故無損失可言。 |
| **Q1'** | 貴端 host 何時可依 §4.1 的 67 個號重下 S2F33？<br>並補上 **S2F35 link ＋ S2F37 CEED=TRUE**？ | 嚴格模式上線後，沿用 0807 那 7 張報表會直接 DRACK=0x04 建不起來。而且**光補報表還不夠**——0807 全天 8 個 session 一次都沒 link、沒 enable，這兩步不補，資料仍然不會上傳。 |
| ~~Q2-f~~ | ~~Q2 與 2026-08-04「只用家族號」裁示牴觸？~~ | **不牴觸**。38237-38239 是家族號，貴端的要求與 0804 裁示一致（§2.6）。 |
| ~~Q2-a~~ | ~~38237~38239 是哪一版開始定義的？~~ | **已自行解決**：新版 9046LS 810_B01 內就有（`uHGemHT9045_SV.cpp:861-863`，2026-07-10 新增）。17 個版本重掃，只有此版有。不需貴端補資料。 |
| ~~Q2-b~~ | ~~改用家族既有的 37003-37005？~~ | **不需要**。38237-38239 本身即家族號，直接採用。 |
| ~~Q2-c~~ | ~~Auto4-6 的號碼要改到哪裡？~~ | **已自行決定並上線**：AMR Auto4-6 Tray Count → **38246-38248**；Record Auto4-6 Tray Count → **38249-38251**。兩段在 810_B01 與 HT-160S 皆為空號（810_B01 在 38239 之上的下一個用號是 38511）。**若貴端要改指定他號，請於出貨前告知**。 |
| **Q2-g（新）** | 貴端是否有 host 在 2026-08-10 之前把 38237-38239 綁為 **AMR Auto4-6 Tray Count**？若有，請一併改綁 38246-38248。 | 這三個號的**語意已改變**（非僅停用），線上無法分辨新舊值——型別同為 INT_4，數值都合理。詳見 §2.6。 |
| **Q2-d** | RPTID 524 目前綁在 **CEID 67 Tray Test Finish**。HT-160S 是 sorter，此事件永不發射。請指定要綁到哪個 HT-160S 事件。 | 不指定的話，就算號碼改對了，這張報表仍然一次都不會送出。 |
| **Q2-e**（仍待確認） | 「Lot End 則會清除」——HT-160S **可同時掛多批**（最多 64 批）。<br>**我方暫定：改在 Lot Start 清**（理由見 §2.4）。請貴端確認可接受，或指定要嚴格照 9045 在 Lot End 清。 | 9045 是單批機，此歧義不存在；HT-160S 上「剛結束那批」無法指認一個**軌道**計數器裡的盤屬於誰。任一批 Lot End 就清會清掉別批的數。 |
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
| ~~11 版都沒有 38237-38239~~ | **已作廢**（2026-08-10 取得新版後重驗，見下一列） |
| 只有 810_B01 定義 38237-38239 | 對 `D:\HT9045\` 下 **17 個** `*_Code_*` / `*_Cpp_*` 目錄全樹全文搜尋 `.cpp/.h`：810_B01 = 3 筆命中，其餘 16 版 = 0 |
| 9045 該三號的語意 | 逐行讀 `asendic_Auto.cpp:626-641`（`CC_TSI` 10 盤歸零 vs `CC_KYEC_XILINX` 純累加）、`uLotInfo.cpp:1164-1174`（Lot End 清零）、`cCounterClear.cpp:242` |
| 38246-38251 兩邊皆空 | 由 810_B01 抽出全部 2539 個有效號（排除註解掉的登記）後求空段：38239 之上的下一個用號是 38511；HT-160S 自身用到 38245 |
| HT-160S 全號對新樹零其他碰撞 | 以 810_B01 字典逐號比對 HT-160S 所有登記：同號同名 28、同號僅名稱差異 2（1517/1518）、HT-160S 專屬 4（1008、1259-1261），**真碰撞僅 38237-38239** |
| 1517 enum 0..13 | `MachineType.h` 的 `eRunStartMode`（810_B01 / 906 一致） |
| 0807 全天零拒絕（80 個 ACK 全 `0x00`） | 掃 `SECSGEM_TextLog_14~17.txt`，逐一比對每個 `S2F34/36/38 body:` 的下一行 |
| 54 個未定義 SVID、432 行 | 同上抓 `accept unknown SVID=(\d+)`，取相異值 → 54；54 × 8 session = 432 |
| 7 張報表的 RPTID→SVID 歸屬 | 直接解析 log 內的 S2F33 SML body dump（縮排層級），非由名稱推測 |
| 54 個名稱 | 對 810_B01 的 `uHGemHT9045_SV.cpp` + `uHGemHT9045_EC.cpp` 逐號正規表示式比對，**54/54 逐字相符、0 查無** |
| S6F11 實際送出 0 筆 | 567 筆 `EventReport DataID=` 的**下一行**全部是 `suppressed`(508) 或 `skipped`(59)，且全檔無任何 S6F11 的 `(sent)` / `len=` / `body:` 行 |
| 8 筆 S2F35 全 UNLINK-ALL、8 筆 S2F37 全 DISABLE-ALL | 逐筆傾印 body，8/8 皆為 `<L[2] <U4 1> <L[0]>>` 與 `<L[2] <Boolean 0x00> <L[0]>>` |
| 三個假朋友 | 在 HT-160S 全樹 grep：`ATC_SYSTEM` / `ContactAlarm` / `TempMode` 各 **0 命中**；大小寫敏感的 `OCR` **0 命中**；`AutoClean` 只命中 Clean Out 排空；`magazine` 只命中 AMR 載盤車 |
