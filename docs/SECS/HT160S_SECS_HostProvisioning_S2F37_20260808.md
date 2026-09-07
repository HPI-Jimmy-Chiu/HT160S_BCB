# HT-160S SECS/GEM Host 端設定缺口報告：S2F37 未啟用事件回報 / Host Provisioning Gap: Event Reports Never Enabled

> **機型 Model:** HT-160S (Tray Sorter) &nbsp;|&nbsp; **MDLN:** `HT-160S` &nbsp;|&nbsp; **SOFTREV:** `1.0.0.0`
> **文件版本 Doc rev:** 2026-08-08
> **證據來源 Evidence:** 京元竹南現場機台 2026-08-07 之 SECS/GEM wire log(14–17 時,5083 行),
> 隨 State Record 快照 `2026-08-07 17_31_22.zip` 之 `SecsLog\2026_08_07\` 一併封存。
>
> **結論一句話**:2026-08-07 當天,機台**一筆事件回報(S6F11)都沒有送達 host**。
> 原因不是連線或機台故障 —— 而是 host 在每一次連線的初始化中送出「停用全部事件」,
> **卻始終沒有補上最後一步「啟用」**。機台依 SEMI E5 忠實照辦,因此保持靜默。
>
> **In one line:** on 2026-08-07 not a single S6F11 event report reached the host. This was not a link
> fault or an equipment fault — the host's per-session provisioning disables every event and never
> sends the matching enable, and the equipment honours that request exactly as SEMI E5 requires.

---

## 1. 觀察到的現象 / What was observed

當天機台共啟動 8 次,每一次連線都完整走完 HSMS Select 與 GEM 初始化,**傳輸層全程零錯誤**:
沒有 S9Fx、沒有 T3/T6 逾時、沒有斷線、沒有協定錯誤。

Across the 8 sessions that day the HSMS transport was flawless — no S9Fx, no T3/T6 timeout, no
disconnect, no protocol error. Nevertheless:

| 項目 Item | 數量 Count |
|---|---|
| 機台嘗試發出的 S6F11 事件回報 / S6F11 reports the tool tried to send | **567** |
| 其中被「host 已停用該 CEID」閘門擋下 / suppressed by the host-disabled gate | **508** |
| 其中發生在 HSMS SELECTED 之前(開機瞬間)/ attempted before Select | **59** |
| **實際送達 host / actually put on the wire** | **0** |
| 涉及的相異 CEID / distinct CEIDs involved | **31** |

被擋下最多的事件(依次數)/ most-suppressed events:

| CEID | 次數 Count |
|---|---|
| 53 | 86 |
| 66 | 85 |
| 54 | 82 |
| 27 | 61 |
| 123 | 36 |
| 136 | 27 |
| 1 | 24 |
| 76 | 19 |

---

## 2. 根本原因 / Root cause

Host 在**每一個** session 都送出同一個封包,共 8 次,時間點如下:

The host sent the identical packet in **every** session — 8 times, at:

```
2026/08/07 15:09:21.662      2026/08/07 17:16:26.974
2026/08/07 15:49:38.115      2026/08/07 17:18:33.881
2026/08/07 16:10:55.082      2026/08/07 17:20:20.338
2026/08/07 16:23:50.005      2026/08/07 17:31:14.052
```

封包內容(逐字取自 wire log)/ verbatim from the wire log:

```
[SECS][RX] S2F37 W=1 body:
<L[2]
  <Boolean[1] 0x00>
  <L[0]
  >
>
```

`CEED = 0x00` (FALSE) = **停用 / disable**;`<L[0]>` = 空清單,依 E5 表示**全部 CEID**。
所以這個封包的意思精確地是「**停用本機所有事件回報**」。機台回覆 `S2F38 ERACK = 0`(受理)。

`CEED = FALSE` with an **empty** CEID list means "all CEIDs" under E5, so this packet says exactly
"disable every event report on this tool". The equipment answers `S2F38 ERACK = 0` (accepted).

**問題在於:全天 8 筆 S2F37,`CEED` 全部都是 `0x00`,一筆 `CEED = 0x01`(啟用)都沒有。**
**Across all 8 S2F37 messages that day, `CEED` was `0x00` every time. There was never a `CEED = 0x01`.**

### 2.1 Host 目前的初始化順序 / The host's current sequence

從 wire log 還原出的實際順序(以 15:09 那次為例):

```
1. HSMS Select                                    -> Select.rsp (SELECTED)
2. S1F13  Establish Communications                -> S1F14 COMMACK=0
3. S1F17  Request ON-LINE                         -> S1F18 ONLACK=2 (On-Line Remote)
4. S2F37  CEED=FALSE, <L[0]>   ← 停用全部事件      -> S2F38 ERACK=0
5. S5F3   警報啟用/停用                            -> S5F4 ACKC5=0
6. S2F33  <L[2] <U4 1> <L[0]>>  ← 刪除全部報表定義  -> S2F34 DRACK=0
7. S2F35  <L[2] <U4 1> <L[0]>>  ← 解除全部事件連結  -> S2F36 LRACK=0
8. S2F33  ×N  ← 重新定義報表(23/41/20/38/50/59… 項) -> S2F34 DRACK=0
9. S2F35  ×N  ← 重新連結事件與報表                  -> S2F36 LRACK=0
   ...
   ✗ 缺少 / MISSING:  S2F37  CEED=TRUE
```

第 4 步到第 7 步是標準且正確的「清除舊設定」動作,我們完全認同。
問題是在第 8、9 步重新佈建完成之後,**沒有把事件重新開啟**。

Steps 4–7 are a textbook provisioning reset and we have no objection to them. The gap is that after
re-defining and re-linking in steps 8–9, nothing re-enables the events.

### 2.2 為什麼機台不「自己想通」 / Why the equipment does not "figure it out"

依 SEMI E5 / E30,`S2F35`(Link Event Report)只建立「事件 → 報表」的連結,**不具備啟用效果**;
啟用是 `S2F37 CEED=TRUE` 的專責。因此機台無法把「host 定義了報表」推論成「host 想要收事件」——
一台合規的設備必須等 host 明確說「啟用」。

Under E5/E30, `S2F35` only links reports to events; it does **not** enable them. Enabling is
exclusively `S2F37 CEED=TRUE`. A conformant tool therefore cannot infer intent from S2F33/S2F35 —
it must wait to be told.

---

## 3. 請 Host 端補上的動作 / What the host needs to add

在現有 `S2F33` / `S2F35` 佈建**之後**,補送一筆 `S2F37 CEED=TRUE`。兩種寫法都可以:

Add one `S2F37 CEED=TRUE` **after** the existing S2F33/S2F35 provisioning. Either form works:

**(A) 啟用全部事件 / enable everything** — 空清單 = 全部:

```
S2F37 W
<L[2]
  <Boolean[1] 0x01>
  <L[0]
  >
>
```

**(B) 只啟用指定的事件(建議)/ enable only the CEIDs you want (recommended)**:

```
S2F37 W
<L[2]
  <Boolean[1] 0x01>
  <L[n]
    <U4[1] 53>
    <U4[1] 66>
    <U4[1] 54>
    ...
  >
>
```

> 建議用 (B):只收真正要用的事件,可避免不必要的流量。
> 本機 CEID 字典共 **292** 個號碼(1–292,自 2026-07-29 起為 HT-90XX 的逐字複本),
> 完整清單見 `HT160S_SECS_Interface_Spec_20260727.md` §3.3。
> **注意 CEID 上限**:單筆 S2F37 的 CEID 清單本機可接受至 512 個,列舉整本字典不會被拒。
>
> (B) is recommended so only the events you consume are sent. The dictionary holds **292** ids
> (1–292, a verbatim copy of HT-90XX's since 2026-07-29) — see §3.3 of the interface spec. A single
> S2F37 accepts up to 512 ids, so enumerating the whole dictionary is safe.

### 3.1 驗證方式 / How to confirm

補上之後,機台會在自己的 EventLog 記下一行,可直接用來驗收:

After the change the tool writes one line to its own EventLog, which you can use as the acceptance
check:

```
SECS host event reporting enabled : <N> of 292 CEIDs
```

而在 SECS wire log 會看到:

```
[SECS][S2F37] CEED=1 n=0 -> 292 of 292 CEIDs enabled
```

---

## 4. 機台端已做的改善 / What has been changed on the equipment side

這次事件當天完全沒有留下任何**明顯**的線索 —— 唯一的痕跡是 SECS wire log 裡 508 條
「suppressed」逐筆訊息,在那個數量下根本無法閱讀,而隨機台出貨的 EventLog 則完全沒有提到。
這一點是我們的問題,已修正(**行為未改變**):

The condition left no *findable* trace: the only evidence was 508 per-event "suppressed" lines
buried in the wire log, and the shipped EventLog said nothing at all. That part was on us and has
been fixed (**with no change in behaviour**):

1. **S2F37 現在會在 wire log 陳述結果政策**,一行取代 508 條:
   `[SECS][S2F37] CEED=0 n=0 -> 0 of 292 CEIDs enabled`
2. **EventLog 會在「完全發不出去」狀態的進入/離開時各記一行**,
   例如 `SECS host DISABLED every event report (S2F37, all 292 CEIDs) - no S6F11 reaches the host until it enables some`。
   只在狀態轉換時記錄,不會洗版。
3. 新增可查詢的內部狀態(已啟用 CEID 數 / 總數 / host 是否接管報表),供維護畫面顯示。

### 4.1 過渡期選項 / Interim option

為避免 host 端修改完成前無法取得任何資料,新增一個**預設關閉**的設定:

So that no data is lost while the host change is scheduled, a **default-off** setting was added:

```ini
[SECS]
IgnoreHostSilenceAll=0      ; 0 = 預設,嚴格遵循 E5(現行行為)
                            ; 1 = 過渡期用:當 S2F37 導致零個 CEID 啟用時,不接受被完全靜音
```

* `0`(預設)= 完全遵循 E5:host 要求靜音就靜音。行為與現行版本**完全相同**。
* `1` = 過渡期:當一筆 S2F37 造成「零個 CEID 啟用」時,機台釋放 host 報表接管,事件照常送出。
  host 之後只要送出**指定清單**的 `S2F37 CEED=TRUE`,即刻恢復為完全依 host 指定的行為,
  不會多送 host 沒有指定的事件。

> **這是暫時的擋板,不是解法。** 正式解法仍是 host 端補上 `S2F37 CEED=TRUE`。
> This is a stopgap, not the fix. The correct resolution remains the host-side enable step.

---

## 附錄:同一次佈建中另一個可一併處理的項目 / Appendix: one related provisioning item

**與本案不同、但屬於同一份 host 佈建設定**,建議一併確認。

Host 在 `S2F33` 中引用了 **54 個本機未實作的 SVID**。機台採取寬容策略(接受定義、該欄回空值),
所以不會報錯,但這些欄位會**永遠是空的**:

The host's S2F33 references **54 SVIDs this tool does not implement**. The tool tolerates them
(accepts the definition, answers a zero-length item), so nothing errors — but those cells will be
empty forever:

```
1040, 1043, 2015, 2016, 2017, 4880, 4881, 4882, 4883, 4884, 4885, 4886, 4887, 4888,
9501, 9517, 9518, 9521, 9523, 9524, 9532, 9533, 9552, 9553, 9554, 9555, 9556, 9557, 9558,
10698, 11098, 11298, 11398, 37202, 37203, 37228, 37529, 37530, 37531, 37532, 37533,
38806, 38821, 38825, 38826, 38827, 38828, 38829, 38830, 38831, 38832, 38833, 38834, 38835
```

請協助確認這些號碼是否為必要資料。若為必要,我們會依 HT-90XX 家族編號規則評估實作;
若非必要,建議自 host 的報表定義中移除,以免報表出現永久空欄。

Please confirm whether these are required. If they are, we will evaluate implementing them under the
HT-90XX numbering rules; if not, we suggest removing them from the host's report definitions so the
reports do not carry permanently empty cells.

---

## 附件 / Attachments

| 檔案 File | 內容 Content |
|---|---|
| `2026-08-07 17_31_22.zip` → `SecsLog\2026_08_07\SECSGEM_TextLog_14..17.txt` | 當天完整 SECS wire log(5083 行),含本文引用的全部封包 |
| `2026-08-07 17_31_22.zip` → `MachineConfig\system\EventReportDef.ini` | 機台當時持有的報表定義 |
| `HT160S_SECS_Interface_Spec_20260727.md` §3.1 / §3.3 | SVID / CEID 完整字典 |
