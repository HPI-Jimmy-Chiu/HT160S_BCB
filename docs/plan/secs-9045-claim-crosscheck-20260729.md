# HT9045 / 京元現場 log 事實主張複驗 (2026-07-29)

- 受檢對象：工作樹中未提交的 `AI(secs-msggap)` + `AI(secs-kyec-rcmd4)` 兩組 SECS 工作
- 方法：4 個檢查者逐條把程式碼註解裡對「HT9045 原始碼」與「京元 2026-06-08 現場 log」的事實主張回查原始證據
  （次數一律重新計算、body 一律重新引用），再由一名裁判獨立複核關鍵數字。
- 一級來源：
  - `D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\SECSGEM\`
  - `D:\backup_version\HT9046\KYEC\20260626\2026_06_08\SECSGEM_TextLog_00..19.txt`
  - `D:\backup_version\HT9046\KYEC\20260626\EventReport_CEID.def`
- Workflow run id：`wf_2b8b7983-5a4`

> **引用注意**：本文件中 HT160S 端的行號是複驗當時（2026-07-29 上午）工作樹的行號，
> 期間另有並發 session 在改同一批檔案，行號已漂移；任何要送客戶的内容請先 pin 到 commit hash 重新取行號。
> 另外 §5 已指出 AREA 4 的 log 行號系統性偏低 3 行、且有一筆檔名/行號誤植，該批行號需重新推導。

---

# Judge verdict — HT160S uncommitted SECS work vs HT9045 source + KYEC 2026-06-08 log

**Tally: 61 claims. 46 TRUE, 13 PARTLY-TRUE, 1 FALSE, 1 UNVERIFIABLE.** No claim is wrong in a way that makes shipped HT160S code wrong. Every correction below lands on comment/justification text or on the customer-facing spec — except item A1, where the *underlying behaviour* was genuinely broken and is being fixed as this review ran.

I independently re-verified the load-bearing counts and the single FALSE verdict against the raw logs (results inline). Two checkers' citation hygiene is bad enough to matter — see **Evidence-quality callouts**.

---

## 1. CORRECTIONS

### Tier A — the error hides or misstates a real behaviour delta (fix code-comment AND spec doc)

**A1. `SVIDs[64]` cap — comment is stale and it papered over a live field defect.**
Claimed at `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemEquipment.cpp:426-427` and `:476`. Verdict PARTLY-TRUE, and I confirm both halves myself:
- Working tree now reads `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemEquipment.h:97` `#define GEM_MAX_SVID_PER_REPORT 192`, `:102` `unsigned SVIDs[GEM_MAX_SVID_PER_REPORT];`, with the gate at `uHGemEquipment.cpp:1009` and clamp at `:1217`. The comments at `:426-427` and `:476` still say `SVIDs[64]` — they now contradict their own file.
- The 64 cap was **not** merely a safety property. I verified the chain: `D:\backup_version\HT9046\KYEC\20260626\2026_06_08\SECSGEM_TextLog_13.txt` S2F35 @13:59:21.081 links CEID 1 to `<L[12] {501,513,502,505,506,507,508,509,514,512,800,801}>`; RPTID 505 is defined `<L[179]>` at `SECSGEM_TextLog_13.txt:3336-3337` and RPTID 800 `<L[103]>` at `:2987-2988`. Under the old cap HT160 would have answered `DRACK=0x01` to those definitions, left 505/800 undefined, and emitted `S6F16 = L,3{1,1,L[10]}` where the host demonstrably accepted `L[12]`.
- `ReportIDs[32]` half is correct (`uHGemEquipment.h:109`, clamp at `:1198`); max RPTIDs per CEID on the wire is 12.
**Action:** rewrite the two comments to the 192 macro, and state that the old cap was a defect on real KYEC traffic, not a hypothetical bound. Confirm the 192 change is committed — it arrived from a concurrent session mid-review.

**A2. FALSE — "ALL of KYEC's observed PP_SIGNALTOWER packets carry 2 on every colour."**
Claimed at `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\csystem.cpp:843`. It is **5 of 6**. My own dump of all 15 PP_SIGNALTOWER records (6 SET / 9 CLEAR):

| file:line (`<A[14]>`) | body |
|---|---|
| `SECSGEM_TextLog_14.txt:5946` | 2/2/2 |
| `SECSGEM_TextLog_14.txt:6366` | 2/2/2 |
| `SECSGEM_TextLog_15.txt:95` | 2/2/2 |
| `SECSGEM_TextLog_15.txt:390` | 2/2/2 |
| `SECSGEM_TextLog_15.txt:463` | 2/2/2 |
| `SECSGEM_TextLog_19.txt:1951` (19:07:50.526) | **RED=2, GREEN=0, YELLOW=0** |

Code is fine — `SecsTowerLampOn()` maps 2→true/0→false so 2/0/0 renders red-only. Two consequences the false "ALL" hides and the spec must disclose: (a) HT160 cannot express **blink** at all, so on the five 2/2/2 packets HT9045 shows a synchronised triple blink and HT160 shows three solid lamps — a visually different annunciator to the operator; (b) the host does use non-uniform per-colour states, so "they always send the same thing" is not a safe simplifying assumption.
**Action:** correct `csystem.cpp:843` to "5 of 6"; add the blink→steady deviation to `D:\HT160S_BCB\docs\SECS\HT160S_SECS_Interface_Spec_20260727.md` as an explicit customer-visible deviation (it currently rests only on the internal user decision at `maintenance.cpp:284-285`).

**A3. ONE_CYCLE accept/swallow split is 3/8, not 2/9 — and one CEID 41 was a local press.**
Claimed at `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\csystem.cpp:1677-1678`. The third CEID 41 (18:51:58.301) follows two local `CEID 3 OneCycle Pressed` 83 ms earlier, with the nearest host ONE_CYCLE 9 min 43 s back. And ONE_CYCLE #3 (19:00:19.281) **did** arm — CEID 3 at 19:00:19.306 — on a machine that went HALT 0.85 s later and then latched. Only #4..#11 (eight) were silently swallowed.
This makes the guard's case **stronger**, not weaker: the field log contains one real stale-arm event, and the checker independently nailed the mechanism — 9045's `BtnOneCycle->Down` latch is cleared at exactly one site, `D:\HT9045\...\csystem.cpp:11962`, inside the finish handler that never ran.
**Action:** rewrite the `csystem.cpp` narrative to "3 accepted / 8 swallowed, one accept immediately stranded by a HALT," and drop the claim that all three CEID 41s follow host commands.

**A4. "The host demonstrably tolerates refusals and keeps its own retry cadence regardless" is not established for ONE_CYCLE.**
Claimed at `uHGemHT160.cpp:1254-1255`, carried over from the ENERGY_SAVING evidence at `:1296-1298`. The 23/23 HCACK=2 is real, but that host **did** change cadence: I verified the ENERGY_SAVING heartbeat runs 22 beats from 00:03:24.754 to 10:33:54.163 (28.9–30.8 min spacing) and then stops for 4 h 59.6 min, with a single orphan at 15:33:32.458 and nothing for the last 3 h 49 min. And HCACK≠0 for ONE_CYCLE was **never** on the wire (11/11 got 0). HT160's new HCACK=4/2 answers to the 60 s retry storm are untested wire behaviour.
**Action:** delete the "keeps its cadence regardless" inference from the ONE_CYCLE comment; keep the narrow, true statement (HCACK=2 to a well-formed command did not make this host escalate — zero S9Fx all day). Add an on-machine watch item; do not present HCACK=4 as field-proven in the spec.

**A5. TID is a dismissal-policy flag, not "not a severity discriminator."**
Claimed at `uHGemHT160.cpp:1830-1832`. The two literal sub-facts hold (`[OMS1] Criteria Check Alarm` at TID=0x00 in `SECSGEM_TextLog_18.txt:6056-6058` and TID=0x01 in `:12367-12369`; 5/5 MIScheduleSetAlarm at TID=0x01). But 9045 consumes TID as `iSECSMessageCanCloseByOperator` (`D:\HT9045\...\SECSGEM\uHGemClass.cpp:2295`, `:2386`) and `D:\HT9045\...\mymessbox.cpp:430-446` uses it to pick no-gate / password / employee-ID-check. Read that way the corpus is mostly coherent, with one off-pattern text.
**Action:** reword to "TID is a dismissal-policy flag (0=normal flow, 1=close directly, 2=employee-ID check), not a severity level, and this host's use of it is not fully self-consistent." Harmless today (HT160 pops nothing and logs TID verbatim at `uHGemHT160.cpp:1846`), but the current wording would actively mislead whoever implements phase 2.

**A6. The `char str[1024]` characterisation is right for S10F4 and dangerously wrong for S10F6.**
Claimed at `uHGemHT160.cpp:1873-1874`. S10F4 matches (`D:\HT9045\...\uHGemClass.cpp:2282`, `:2296`). S10F6 does **not**: `:2395-2396` peeks the request's own length and hands it straight to `DataItemIn(iDataLen, Type, str)` with no 1024 clamp. A verbatim port of the S10F6 form is an unbounded stack smash, not a 1-byte overrun.
**Action:** split the comment per-function. HT160's AnsiString-overload path is safe either way (`uHGemEquipment.cpp:2794-2798`, `new char[Len+4]`), so no code change — but a future maintainer reading only this comment would "fix" S10F6 by capping at 1024, which is not even the bug.

**A7. The omitted enable-gate is safe because the SEND is gated, not because the RECEIVE is.**
Claimed at `uHGemEquipment.cpp:420`. `ProcessReceiveBuffer` hands every well-framed HSMS DATA message to `HandleDataMessage`→`Dispatch` with no `iHsmsState` test, so the stated premise ("you cannot receive an S6F15 unless SELECTED") is false. Conclusion survives via `SendLocalData` (`uHGemEquipment.cpp:1517-1533`, `if(ActiveSocket!=NULL && iHsmsState==HSMS_STATE_SELECTED)`).
**Action:** fix the stated reason. No code change.

**A8. The four 9045 RCMD clears are TOWER-ONLY, and the pair-clear surface is 4 sites, not 2.**
Claimed at `csystem.cpp:889-897`. `uHGemHT9045.cpp:1543`, `:1661`, `:2110`, `:2589` clear only the tower flag; the music flag is untouched. Pair-clears are `main.cpp:2134-2135`, `note.cpp:2506-2507`, `mymessbox.cpp:519-520` **and** `mymessbox.cpp:1252-1253` (`pnlAlarmResetClick`) — 8 clear sites in total, not 6.
**Action:** correct the count and the tower-only qualifier. The decision to omit the four RCMD artifacts is sound and should stay (KYEC really does send START_AGV; porting them would let an unrelated command cancel a tower override). The 4-site fact is *why* HT160 correctly needs both a key-scan and a button handler on each dialog.

### Tier B — narrative/precision only, no behaviour implication

**B1. RPTID 506's L[5]/L[6] story is inverted and the real requirement is stronger.** (`uHGemEquipment.cpp:469-470`, `uHGemHT160.cpp:1787-1788`) L[6] is chronologically **first** (file 17 @17:51), L[5] later (file 19 @19:07); **both** sessions delete-then-redefine, so the delete is not the discriminator; the SVID sets are disjoint (3800-3806 vs 3616-3677) — a different report, not a grown one; and `SECSGEM_TextLog_16.txt:1581`→`:2571`→`:4531` shows 506 going 5→6 **inside one session via a plain overwrite with no delete**. Reading `Rp->SVCount` live (`uHGemEquipment.cpp:487-489`) is required more broadly than the comment argues. Rewrite the narrative to the stronger form.

**B2. "nine times in nine minutes against a HALTed machine"** (`uHGemHT160.cpp:1250-1251`, `main.cpp:1860`) → verified span 19:00:19.281 → 19:08:19.586 = **8 min 00.3 s** at exact 60.0 s cadence; 7 of 9 unambiguously in HALT, one on the RT1 ART→HALT boundary, the last in Alarm. Substance intact, wording loose.

**B3. "~30-minute heartbeat"** (`uHGemHT160.cpp:1290`) → true for 10.5 h then abandoned. See A4 for the numbers. Say "22 beats 00:03→10:33, then abandoned; one orphan at 15:33."

**B4. "23/23 HCACK=2 from its 'Function Disable' rung"** (`uHGemHT160.cpp:1296-1297`) → the HCACK=2 is directly verified; the *rung* is not observable in the SECS log. It is narrowed to `uHGemHT9045.cpp:2882-2886` or `:2999-3003` by elimination (the IC / Contact / ATC-online gates are time-varying; HCACK was 2 across 15.5 h and states SLEEP/Alarm/HALT/Running). Say "a config-level Function Disable rung (one of two)". No HT160 branch depends on it.

**B5. "right after an S10F5 'tester is IDLE, priority lot waiting'... the use case is 'come load a lot', NOT an alarm."** (`uHGemHT160.cpp:1338-1340`, `csystem.cpp:827-829`) → only 3 of 6 SET bursts follow that text. The others follow `has no schedule.`, `MES_Status Changed toSetUp`, and an **S10F3** (not F5) `[ART]User manually abort ART process.` It is a generic host attention annunciator. The 0.3 s pairing (0.256–0.361 s, PP_MUSIC always first) is solid. This matters because the "NOT an alarm" reading is cited as background for the alarm-suppression deviation at `csystem.cpp:925-936` and the buzzer gate at `:1000-1006` — those gates remain defensible on safety grounds (safety-derived red must win), but the comment must not claim the host never uses this channel around alarm-ish conditions. See NEW GAP G8.

**B6. "longest observed armed span 1723.8 s"** (`csystem.cpp:829`) → correct as a *continuous armed interval* (15:02:41.838 → 15:31:25.593) but it contains three SETs and one release. Longest single SET→CLEAR pair is 1131.9 s. Say "longest continuous armed interval 1723.8 s (three consecutive SETs, one release)". Either figure supports the operator-escape argument equally.

---

## 2. CONFIRMED — the defensible numbers

Independently recounted by me over all 20 files unless noted.

**Message surface (my recount, exact):** S6F15 = **3** (files 13/14/16, one each); S6F19 = **8** (13:2, 14:2, 16:2, 17:1, 19:1); S6F17/F18/F23/F24 = **0**; S9Fx = **0** all day; S10F3 = **1**, S10F5 = **7**; S125F1 = **6**, S125F2 = **138** (= 3 × (1+45), partition 46/46/46); ONE_CYCLE = **11**; ENERGY_SAVING = **23**; PP_SIGNALTOWER = **15** (6 SET / 9 CLEAR); PP_MUSIC = **15** (6 SET / 9 CLEAR); CLOSE_ONECYCLE = **0**.

**Body shapes, invariant:**
- S6F15 body = bare `<U4[1] 1>`, 3/3 — no L,1 wrapper. S6F19 body = bare `<U4[1] RPTID>`, 8/8 (700×3, 600×3, 506×2).
- S6F16 reply = `L,3{DATAID=1, CEID=1, L,12{L,2{RPTID,L,b}}}`, 3/3. DATAID hardcoded 1 in 9045 (`uHGemClass.cpp:1965`, `:1977`), matching HT160's glue `UsecegemMainFrom.cpp:137-142`.
- S6F20 reply = **flat** `L,n` of SV values, 8/8 — no RPTID echo, no per-SV wrapper. 700→L[21] (3/3), 600→L[12] (3/3), 506→L[6] then L[5].
- ONE_CYCLE = `L[2]{A[9]"ONE_CYCLE", L[0]}`, 11/11 empty param list; HCACK=**0**, 11/11.
- ENERGY_SAVING = `L[2]{A[13], L[1]{L[2]{A[5]"STATE", U4 0}}}`, 23/23, STATE always 0; HCACK=**2**, 23/23; zero escalation (0 S9Fx, no alarm, no ONLINE/OFFLINE churn).
- PP_MUSIC SET = `L[1]{L[2]{A[0] "", U4 1}}` — **the CP name is a zero-length ASCII item**, 6/6, class always 1. 9045 reads it and discards without comparison (`uHGemHT9045.cpp:1819-1829`, `str` never referenced again) — a name check here would reject every real KYEC packet.
- PP_SIGNALTOWER SET names are RED/GREEN/YELLOW in that order 6/6; values ∈ {0,2} only, never 1. Domain 0=off/1=on/2=blink from `D:\HT9045\...\ckernel.cpp:801-820`.
- S10F3/F5 body = `L,2{B[1] TID, L,n{A TEXT}}`, n ∈ 1..3 across 7 captures; ACKC10 = **0**, 8/8. CRLF join matches 9045 (`uHGemClass.cpp:2398` vs `uHGemHT160.cpp:1946`), trailing CRLF and all.
- S125F1 = ALED 0x01+L[0] then ALED 0x80+L[45], 3 pairs, ~0.3 s apart; the three 45-ECID lists are **byte-identical** across sessions. ALED is bit-7 (`D:\HT9045\...\uHGemEquipment.cpp:9421`, `:9438`), so 0x01 = disable.

**Pre-patch failure mode, confirmed from git:** `HEAD` had `case 3:`/`case 5:` for stream 10 landing on `SendUnsupported` (log only, `uHGemClass.cpp:158-162`), no `case 125:` at all, and no S6 F15/F19 entries — all four routes produced **zero bytes** and could only T3 the host. `S9F3_Unrecognized_Stream_Function_Type` (`uHGemClass.cpp:265-269`) is a single `StringOut`. HT160 has no S9F7 sender anywhere.

**Source facts settling design choices:** 9045's unknown-CEID S6F16 is a bare zero-length U4 (`uHGemClass.cpp:1983-1988`); its `IsEnableEvent` gate is literally commented out at `:1973` and the wire proves it would have changed behaviour (all 3 S6F15s arrived while the host had globally disabled every event, and 9045 answered in full); 9045's unknown-RPTID path falls out of the loop to `S9F7_IllegalData` at `:2080` with no S6F20 built; `iReturnCode` is monotone (`uHGemEquipment.cpp:2930-2937`); `DataItemIn(AnsiString&)` has no BINARY/BOOLEAN/U8/F8 branch and rejects `Len!=1` (`:2861-2928`); `SetReportIDContent` overwrites in place and an empty SV list maps to `DeleteHostReport` (`:1204-1222`, `:1032-1035`).

**ONE_CYCLE deviation, correctly identified:** 9045's branch is three lines (`uHGemHT9045.cpp:1101-1107`) that call `BtnOneCycleClick` unchanged and set HCACK=0 outside every gate; `main.cpp:3757-3807` has five bare `return;` gates and **no** SystemStart/RunMode test. CEID 27 was declared and registered but sent from nowhere before this diff (`git grep OneCycleOK HEAD` = 2 hits, no call site). CLOSE_ONECYCLE exists only to dismiss 9045's blocking MES1640 note; HT160's finish block raises no modal, so nothing equivalent is needed. RCMD AnsiPos chain audited — no prefix can swallow ONE_CYCLE / ENERGY_SAVING / PP_SIGNALTOWER / PP_MUSIC.

**PP_* deviations, correctly identified:** 9045 has no timeout, no persistence, and no link-drop release for either override flag (exhaustive grep: 2 defs, 2 arms, 2 in-command clears, 4 tower-only RCMD clears, 4 pair clears — nothing else). 9045's `SW[SwMusic1+CLASS-1].On()` has no bound check (`ckernel.cpp:747-752`); with `SwMusic1=37` and `SwHeaterRelay=36`, CLASS=0 energises the **heater relay** — and the upper side is unbounded too (CLASS=5→SwTestPassLed, ≥7 walks into Start/Clear switches). HT160's 1..4 gate covers both directions. 9045's tower branch partially applies and latches unconditionally (`uHGemHT9045.cpp:1848-1873`), re-executes `HCACK=0` per iteration, and range-checks nothing. 9045 bypasses the whole per-state light table while armed (`ckernel.cpp:798` vs the `else` at `:822-866`), with a dead commented-out gate at `main.cpp:2509-2517`. Both HT160 internal cross-references check out (`database.h:453-456`, `maintenance.cpp:284-285`).

**Strong corroborating find (AREA 4, keep it):** the KYEC host itself clears the pair after every reconnect — 3 reconnects on 2026-06-08, each followed 32–41 s later by `PP_SIGNALTOWER L[0]` + `PP_MUSIC L[0]` as part of its cold-init sweep. The host does not assume the latch survived a link drop, so HT160's release-on-link-loss (`uHGemHT160.cpp:1574-1582`) cannot desynchronise it and closes a real 32–41 s window. And 5 of 9 CLEARs arrive with nothing armed — the CLEAR path must be a pure no-op, which HT160's is.

---

## 3. UNVERIFIABLE — and exactly what would settle each

1. **"E5-clean": that the KYEC host tolerates HT160's `L,3{DATAID,CEID,L,0}` for an unknown CEID** (`uHGemEquipment.cpp:424-425`). All 3 S6F15s asked for CEID 1, which was defined, so the branch never fired. **Settle by:** a KYEC trace containing S6F15 for an undefined CEID, or the host-side parser spec, or a bench run against `D:\AI_Area\Tool\HT160S_SECS_Simulator` scripted to pull an unknown CEID. Standards argument ≠ field observation. Unreachable on today's traffic.
2. **Whether the host tolerates HCACK=4/2 on ONE_CYCLE.** Zero field instances (11/11 got 0). **Settle by:** simulator run reproducing the exact 60 s-cadence retry storm with HT160 refusing, then on-machine at KYEC. Until then this is the single largest untested behaviour in the RCMD work.
3. **Which of 9045's two Function-Disable rungs produced the 23 HCACK=2s.** **Settle by:** KYEC's machine-side `RecordProcess` log for 2026-06-08, or that machine's `IniConfig` values for `bC05_PowerSaveATC` / `bC05_PowerSaveTemp` / `bC05_PowerSaveMotor`. No HT160 consequence.
4. **Whether the KYEC host keys One-Cycle-Finish off the CEID or off S5F1 ALID 316001640.** **Settle by:** the host provisioning/recipe doc, or asking the KYEC MES team. Determines whether G3 and G4 below are real problems or non-issues.
5. **The 18:51:58 double `CEID 3` 41 ms apart** — should be impossible given 9045's `BtnOneCycle->Down` latch. **Settle by:** the 9045 machine log for that minute. Looks like a 9045 defect; not load-bearing for any HT160 claim.
6. **"HT9045's 45-same-SystemByte burst is a protocol defect"** — TRUE as source analysis, but note the field consequence did **not** occur: the host proceeded to its next primary 1.75 s after the 46th ack, with no T3, no S9Fx, no S125F1 retry. HT160's single-reply is still correct and strictly safer; do not cite host breakage as the motive.

---

## 4. NEW GAPS — primary sources show these; the implementation does not account for them

**G1. S125F2 tells the host "enabled" for 44 ECIDs HT160 does not have.** 9045 acks **1** for an unregistered ECID (`uHGemClass.cpp:2650-2653`), wire-proven by the 4×`0x01` at list positions 7, 8, 10, 45 = ECIDs 2101, 2102, 2004, 8506 — the exact four that are absent from the whole 9045 tree. HT160's handler (verified: `uHGemHT160.cpp:1993-2021`) sets ACK=1 **only on parse/format failure**; a well-formed unknown ECID gets ACK=0. HT160 owns exactly **1** of the 45 (ECID 1501). Cheap honest fix: `ACK=1` when `FindECItem(ECID)==NULL`. The comment justifies the *storage* decision and never mentions the *acknowledgement-truthfulness* decision.

**G2. S6F20's placeholder for an unknown SVID is an empty LIST where the host expects a scalar.** `uHGemEquipment.cpp:626-632` emits `DataItemOut(0, LIST_TYPE, NULL)`. List length stays aligned (good, and better than 9045, which poisons the whole report with S9F7), but a strict host type-checker could reject `L,0` in an SV slot. Untested on this wire.

**G3. CEID number collision, severe for this event.** HT160 uses CEID 27 for One Cycle Finish (`uHGemHT160.h:37`). In KYEC's 9045 map CEID 27 is "Change Machine State" — the busiest event of the day (~406 S6F11 sends) — while the real One-Cycle-Finish is CEID 41 (3 sends). A host provisioned from KYEC's 9045 CEID list decodes HT160's finish event as a machine-state change. The `csystem.cpp` comment names 9045's equivalent correctly but does not flag the collision. **This belongs in the customer-facing spec.**

**G4. No S5F1 twin for One-Cycle-Finish.** 9045 emits S5F1 ALID 316001640 ALTX "One cycle finish" ALCD 3 alongside CEID 41. HT160's `EmitOneCycleOK` emits only S6F11. A host keying off the ALID sees nothing. Depends on unverifiable #4.

**G5. Host terminal text reaches no operator.** 9045 drains `SecsAlarmMessage` into a modal with a TID-keyed password / employee-ID gate (`UsecegemMainFrom.cpp:1073-1079` + `mymessbox.cpp:426-455`). HT160's queue has **no consumer** — I verified: the only references are the three ctors, the dtor, the header decl, and the two new sink call sites (`uHGemHT160.cpp:1921`, `:1969`). So `"Please issue Malfunction Notice, CALL 4/2 EE do Correlation Check"` is visible only in the SECS log or the EventLog CSV. Disclosed as phase 1 with a sound reason (ShowModal inside the HSMS receive callback would stall MainProc), but it is a real functional delta versus 9045, not a nicety.

**G6. S10F6 can silently truncate and still ack success.** `uHGemHT160.cpp:1960` `if(sText.Length() < 8192)` drops remaining lines while ACKC10 stays 0. Unreachable on this corpus (largest total ~161 bytes; n already capped at 64). Documentation nit, but it is a lie to the host.

**G7. W-bit ignored.** 9045 gates the S10F4/S10F6 reply on `Remote.W_Bit==1` (`uHGemClass.cpp:2335-2336`, `:2411-2412`); HT160 always replies. I confirmed `Remote.W_Bit` is parsed at `uHGemEquipment.cpp:2361` and consulted by **no** handler. All 8 KYEC captures had W=1 so behaviour is identical against this host, but a W=0 S10F3 would draw an unsolicited S10F4. Pre-existing house style, not introduced here — worth one line in the spec.

**G8. The host ties the PP_* annunciator to alarm flow, which undercuts the "NOT an alarm" framing.** Two of the nine CLEARs arrive +0.3 s after the equipment's own `CEID 73 / SVID 502 / "AGV" / "Alarm"` event report (18:20:38 and 18:42:44). Combined with B5 (one SET burst follows an ART-abort S10F3), the honest description is "generic host attention annunciator, used around alarm-ish conditions too". The suppression gates stay — safety-derived red must win — but the *rationale* should not rest on "the host never uses this for alarms".

**G9. Host cannot read back HT160's tower override state.** 9045 exposes SVID 1014/1015/1016 (Tower Light Red/Yellow/Green) pointing at the three colour ints (`uHGemHT9045_SV.cpp:74-76`). HT160 registers no equivalent SVs. Note also that 9045's would-be EC registration for these (ECID 520-523 in `SECSGEM\SECSGEM.cpp:754-758`) is dead code — that file is not in `HT9045.bpr` — so persistence was never real on 9045 either.

**G10. Minor parity notes, no action needed but do not misdescribe them:** HT160 consumes the ONE_CYCLE parameter list where 9045 does not (`uHGemHT160.cpp:1256` vs nothing in `uHGemHT9045.cpp:1101-1107`) — harmless while the body is `L[0]`, 11/11, but the comment's "consumed and ignored, same as 9045" is backwards. HT160 loops over all PP_MUSIC pairs where 9045 reads only the first — HT160 is the safer one. HT160 sets HCACK=1 on a non-list ENERGY_SAVING body where 9045 falls through silently (`uHGemHT9045.cpp:2859`, no else) — a real improvement that no comment claims. And 9045's own `IndexOf(SVID) > 0` test (`uHGemClass.cpp:2053`) is an off-by-one that mis-rejects whatever SVID sits at index 0; plus `uHGemClass.cpp:2022` answers an S6F17 with an `InitLocalHead(6,16,0)`. Both matter only if anyone ports S6F17/F18.

---

## 5. Evidence-quality callouts

**AREA 4's line numbers are systematically 3 lines low, and its one FALSE-verdict citation is a file/line mixup.** The checker cites the `[Receive]` header line, not the quoted item. Actual `<A[14] "PP_SIGNALTOWER">` lines are 14:5946, 14:6366, 15:95, 15:390, 15:463, 19:1951 against its 5943/6363/92/387/460/1948. Worse, its FALSE verdict cites the 2/0/0 packet as `SECSGEM_TextLog_19.txt:6366-6380` — **file 19 has only 2989 lines**, and 6366 is file 14's line for a 2/2/2 packet. The substance survives on its own claim-2 dump and on my recount (real location `SECSGEM_TextLog_19.txt:1951`), but **that citation must not go into a customer document**. Re-derive all AREA-4 line numbers before publishing.

**Ad-hoc parser aggregates with no file:line spot-check.** "122 S2F33 messages", "S2F34 distribution = {0x00: 122}", "406 S6F11 with CEID 27", "10,962 of 10,987 disabled lines", "17067 log records", "Linktest.req 98/99/103/92 per hour", "135 U4 values diffed pairwise". Individually plausible and mutually consistent, but none is anchored. The two that actually carry weight I verified myself: 505→`L[179]` at `SECSGEM_TextLog_13.txt:3337` and 800→`L[103]` at `:2988`, plus the CEID-1→`L[12]` link. The Linktest-per-hour figure is the thinnest of the set and it is the sole support for "the link stayed up while the heartbeat stopped" — re-derive it with file:line before that argument goes anywhere customer-facing.

**AREA 2's Big5 comment translation is a paraphrase.** `iSECSMessageCanCloseByOperator` semantics (0=normal flow / 1=close alarm directly / 2=employee-ID check) come from a decoded Big5 comment. It is almost certainly right and it drives correction A5, but if this lands in a spec doc, quote the raw bytes and mark the translation as such.

**Line numbers in the HT160S working tree drifted during the review.** A concurrent session edited the same files: AREA 2 cited the `SinkHostTerminalText` sinks at `uHGemHT160.cpp:1906`/`:1954`; they are now `:1921`/`:1969`. AREA 3 flagged its own drift. Pin every HT160S citation to a commit hash before publishing, and re-check A1 against whatever the 192-cap change commits as.

**Nothing else was passed through on thin evidence.** All 9045 source claims I spot-checked were quoted verbatim with correct file:line, and every headline wire count I recounted matched exactly.