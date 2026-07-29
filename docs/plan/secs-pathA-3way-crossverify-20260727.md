# HT160S SECS Path A 三方交叉驗證 - 定稿 (2026-07-27)

> Path A 目標:將 HT160 的 SECS 字典/行為對齊 HT9045,使現場京元竹南(KYEC)CJ_EAP host 無需改動即可運作。本文為 finder 產出、對抗式 verifier 逐條複驗後的定稿。

---

## (a) 摘要與覆蓋率

finder 共提出 **221** 條缺口候選,已由對抗式 verifier **逐條** 複驗。

| 項目 | 數量 |
|---|---|
| 已驗證總數 | **221 / 221 (覆蓋率 100%)** |
| 真實缺口 gapReal = yes | 23 |
| 部分缺口 gapReal = partial | 21 |
| 非缺口 gapReal = no | 177 |
| finder 偽陽性(複驗推翻) | 9 |
| 確認的 Path A 缺口(yes + partial) | **44** |

**未完成批次 caveat:無。** 221 條全數完成複驗,無待驗、無截斷批次。

**定稿核心結論:** 現場擋 host 的直接原因並非任何單一「缺訊息」列,而是 **S2F33 對現場 RPTID 502/503/504 所引用之未知 SVID 是否以 DRACK = 0x04 拒絕** 的行為問題(詳見 (e))。S2F33 的訊息處理本身在本次複驗判定為已對齊(gapReal = no)。CEID 層雖有大量編號碰撞,但因 host 以 S2F35 綁定自己的報表、不吃 HT160 的 CEID 編號,故全部 discovery-gated,非直接擋 host。

---

## (b) 各層統計表

| 層 (layer) | 總數 | 真實缺口 (yes) | 部分缺口 (partial) | 偽陽性 (wrong) |
|---|---|---|---|---|
| message | 104 | 18 | 7 | 1 |
| rcmd | 27 | 0 | 0 | 0 |
| ceid | 48 | 5 | 10 | 8 |
| svid | 42 | 0 | 4 | 0 |
| **合計** | **221** | **23** | **21** | **9** |

觀察:
- **message 層** 是唯一有硬缺口(yes)的訊息層,18 條全為 stub / routed-stub / missing 的 handler,會導致 host 對該交易 T3 逾時。
- **rcmd 層** 零缺口 — 現場 CJ_EAP 用到的 START/PAUSE/STOP/LOTSTART/ONLINE_* 皆已實作;其餘皆 tester-only 無 sorter 對應。
- **ceid 層** 的 5 條 yes + 10 條 partial 幾乎全是「編號碰撞」而非「事件不存在」,語意事件多已在 HT160 別的 CEID 上發送;動作皆 discovery-gated。
- **svid 層** 零硬缺口;4 條 partial 皆為 EC-change 稽核族(20001-3)與 Real/Dummy(1518),值多已存在僅未 SECS 曝露。

---

## (c) 確認的 Path A 缺口清單 (by layer)

僅列 gapReal = yes 或 partial 的 44 條。file:line 依 verdict 所載。

### C-1 message 層 (25 條)

| 代號 | 名稱 | 類別 | HT160 現況 | 需實作動作 |
|---|---|---|---|---|
| S1F15 | Request OFF-LINE | MISSING | S=1 switch 僅 F=1/3/11/13/17 (uHGemClass.cpp:76-80),無 F=15 → 落空 | 加 Dispatch case S1F15 → HT160Gem::S1F16 override 回 `<B OFLACK=0>` 並設 iControlState=OFF-LINE(1),仿 S1F17/F18 |
| S1F16 | OFF-LINE Acknowledge | STUB | 基底 HTGem::S1F16 @uHGemClass.cpp:158 = SendUnsupported;無 override | 與 S1F15 同一 handler 的回覆半,回 `<B OFLACK=0>` |
| S2F17 | Date and Time Request | ROUTED_STUB | case 17 存在(uHGemClass.cpp:86)→ 基底 S2F18 @:165 = SendUnsupported,只 log,無回覆 → host T3 逾時 | override S2F18 回 16-char ASCII TIME(仿 9045 uHGemClass.cpp:791:GetTimeInfo + InitLocalHead(2,18,0)) |
| S2F18 | Date and Time Data | STUB | 基底 uHGemClass.cpp:165 SendUnsupported,只經 case 17 可達,未 override | 與 S2F17 同 handler,無獨立動作 |
| S2F23 | Trace Initialize Send | MISSING | S2 switch(:83-96)無 case 23 → 落到 log-only S9F3 → host T3 逾時 | CJ_EAP 用 event report 取代 trace;若需支援,加 case S2F23 → S2F24 TIAACK(ack-only 即防逾時)。低優先 |
| S2F24 | Trace Initialize Acknowledge | STUB | 基底 uHGemClass.cpp:166 SendUnsupported,無 case 23 從不 dispatch | 與 S2F23 同 handler,ack-only TIAACK 即可。低優先 |
| S2F25 | Loopback Diagnostic Request | ROUTED_STUB | case 25 存在(:87)→ 基底 S2F26 @:167 = SendUnsupported,無 echo → host T3 逾時 | override S2F26 讀 inbound ABS 原封 echo(仿 9045 uHGemClass.cpp:1008)。標準鏈路診斷 |
| S2F26 | Loopback Diagnostic Data | STUB | 基底 uHGemClass.cpp:167 SendUnsupported,只經 case 25 可達 | 與 S2F25 同 handler,echo ABS |
| S2F29 | EC Namelist Request | MISSING | S2 switch 無 case 29 → log-only S9F3 → host T3 逾時 | 加 case S2F29 → override S2F30 由既有 EC 表(S2F14 所用)組 ECID/ECNAME/ECMIN/MAX/DEF/UNITS namelist |
| S2F30 | EC Namelist | STUB | 基底 uHGemClass.cpp:168 SendUnsupported,無 case 29 | 與 S2F29 同 handler,由 EC 表(AddEC :209 / S2F14 :380)組回覆 |
| S2F31 | Date and Time Send | OPTIONAL | 已 dispatch(:88)→ S2F32 @uHGemHT160.cpp:1262-1276 回 TIACK=0(不逾時),但刻意不寫 OS clock | host 通訊無需動作(已回 TIACK=0)。若合約要求時間同步才擴充寫 clock,需產品簽核 + 上機驗 |
| S2F32 | Date and Time Acknowledge | OPTIONAL | HT160Gem::S2F32 @uHGemHT160.cpp:1262-1276 回 TIACK=0,回覆本身完整 | 回覆無需動作;寫 clock 是遞延/選配決策 |
| S5F3 | Enable/Disable Alarm Send | OPTIONAL | case 3(:100)→ S5F4 @uHGemHT160.cpp:1278 回 ACKC5=0,但未讀存 ALED+ALID mask | Path A 無需(已回 ACKC5=0,CJ_EAP 用 event report 非 alarm mask)。選配深度對齊:解析 ALED 存 per-ALID enable |
| S6F15 | Event Report Request | MISSING | Dispatch(:71-125)無 case 6 → log-only S9F3,無回覆 → host T3 逾時 | 加 case S6F15 → 新 S6F16_EventReportData:讀 CEID,重用 EventReport 序列化(FindCEIDItem/FindReportItem + DataItemOutSVValue @uHGemEquipment.cpp:378-400) |
| S6F16 | Event Report Data | STUB | 基底 HTGem::S6F16 @uHGemClass.cpp:184 = SendUnsupported,從不 dispatch | 與 S6F15 同 handler 的回覆半;InitLocalHead(6,16,0) + report body |
| S6F19 | Individual Report Request | MISSING | 無 case 6 → log-only S9F3,host RPTID 資料請求靜默 → T3 逾時 | 加 case S6F19 → 新 S6F20_IndividualReportData:讀 RPTID list,由 S2F33/S6F11 report-def 表序列化 |
| S6F20 | Individual Report Data | STUB | 基底 HTGem::S6F20 @uHGemClass.cpp:186 = SendUnsupported,不可達 | 與 S6F19 同 handler 的回覆半 |
| S7F19 | Current EPPD Request | ROUTED_STUB | case 19(:112)→ HTGem::S7F20 @uHGemClass.cpp:199 = SendUnsupported,無 SECS 回覆 | recipe 清單非 CJ_EAP 的 RPTID event-report 集,非 Path-A-critical;若有 host polls,回空 PPID list。確認是否 intentional |
| S7F20 | Current EPPD Data | STUB | HTGem::S7F20 @uHGemClass.cpp:199 = SendUnsupported,只經 case 19 可達 | 與 S7F19 配對:若 recipe inventory 在範圍內回 L,0;否則保留 stub 並確認 intentional |
| S10F3 | Terminal Display, Single | ROUTED_STUB | case S10F3(:118)→ 基底 S10F4 @uHGemClass.cpp:214 = SendUnsupported,無 ACK → host T3 逾時 | override S10F4:至少 W-bit 時 LocalAcknowledge(10,4,0) 防逾時;選配解析 LIST(2){BINARY,text} 顯示 |
| S10F4 | Terminal Display Single Ack | ROUTED_STUB | uHGemClass.cpp:214 stub(即 S10F3 case 到達的回覆 builder),只 SendUnsupported | 與 S10F3 同 override 建 ACK10=0 |
| S10F5 | Terminal Display, Multi-block | ROUTED_STUB | case S10F5(:119)→ 基底 S10F6 @uHGemClass.cpp:215 = SendUnsupported,無 ACK → host T3 逾時 | override S10F6:至少 W-bit 時 LocalAcknowledge(10,6,0);選配解析 LIST(2){flag, ASCII lines} 顯示(9045 idiom) |
| S10F6 | Terminal Display Multi-block Ack | ROUTED_STUB | uHGemClass.cpp:215 stub(S10F5 case 到達的回覆 builder),只 SendUnsupported | 與 S10F5 同 override 建 ACK10=0 |
| S125F1 | Enable/Disable EC Change Report | MISSING | Dispatch(:71-125)無 S125 case;基底 S125F2 @uHGemClass.cpp:234 stub,從不 dispatch | 加 case S125F1 → 解析 ALED+SVID list 回 S125F2 ack(LocalAcknowledge(125,2,0/1))滿足 spec-V 握手;完整報表僅在 host 要求時做 |
| S125F2 | Enable/Disable EC Change Report Ack | MISSING | 不可達 base stub @uHGemClass.cpp:234;無 S125F1 handler 產生此 ack | 與 S125F1 同 handler 發出 |

### C-2 RCMD 層

**無確認缺口。** 現場所需的 START / PAUSE / STOP / LOTSTART / ONLINE_LOCAL / ONLINE_REMOTE 皆已實作(uHGemHT160.cpp);其餘 27 項候選中的 tester/handler-only 命令(SWITCH_TO_FT/RT、AUTO_RETEST、CASSETTE_* 等)無 sorter 對應,可回 N/A。

### C-3 CEID 層 (15 條)

| 代號 | 名稱 | 類別 | HT160 現況 | 需實作動作 |
|---|---|---|---|---|
| CEID 1 | Press Start w/o IC | MAP_SVID | 碰撞:CEID 1 = HandlerStatus(uHGemHT160.h:11; cpp:50)無 fire site;PressStartWithoutIC 在 HT160 CEID 4(note.cpp:664) | 重編號 PressStartWithoutIC 4→1,HandlerStatus 移出 CEID 1。**CAVEAT:影響 conditional,discovery-gated** |
| CEID 2 | Press Pause | MAP_SVID | 碰撞:CEID 2 = RecipeChange(uHGemHT160.h:12; :51)fired main.cpp:1500;Pause 在 HT160 CEID 6 | 重編號 PressPause 6→2,RecipeChange 移出 CEID 2。同 discovery-gated caveat |
| CEID 3 | Press One Cycle | MAP_SVID | 碰撞:CEID 3 = ClearCount(uHGemHT160.h:13; :52)register-only 無 fire site | 重編號 PressOneCycle 8→3,ClearCount 移出 CEID 3。同 discovery-gated caveat |
| CEID 4 | Press Clean Out | MAP_SVID | 碰撞:CEID 4 = PressStartWithoutIC(uHGemHT160.h:14; :53)fired note.cpp:664;Clean Out 在 HT160 CEID 9 | 重編號 PressCleanOut 9→4;CEID 4 現由 PressStartWithoutIC 佔(應移到 CEID 1)。同 caveat |
| CEID 5 | Press Clear Count | MAP_SVID | 碰撞:CEID 5 = PressStartWithIC(uHGemHT160.h:15; :54);fire site COMMENTED OUT @main.cpp:2032 | 重編號 ClearCount 3→5 **且補 fire site**(現從未發送);PressStartWithIC 亦佔 CEID 5(fire site 已註解)。同 caveat |
| CEID 6 | Press Lot Start | OTHER | CEID 6 = PressPause(uHGemHT160.h:16; :55)fired main.cpp:1899;Lot Start 已存在於 HT160 CEID 11 並發送 | **不重編號。** HT160 已發 Lot Start(自有 CEID 11)。先擷取現場 CJ_EAP S2F35 綁定再定 |
| CEID 8 | Press Lot End | OTHER | CEID 8 = PressOneCycle(uHGemHT160.h:18; :57)fired main.cpp:1868;Lot End 已存在於 HT160 CEID 12 並發送 | **不重編號。** HT160 已發 Lot End(自有 CEID 12)。僅在 host 證明綁 CEID 8 為 Lot-End 時 remap |
| CEID 9 | Switching Real/Dummy Mode | OTHER | CEID 9 = PressCleanOut(uHGemHT160.h:19; :58)fired main.cpp:1878;Real/Dummy 已存在於 HT160 CEID 31 並發送 | **不重編號。** HT160 已發 Real/Dummy(自有 CEID 31)。僅在 host 證明綁 CEID 9 時 remap |
| CEID 30 | Press Alarm Reset | MAP_SVID | CEID 30 = TimeEvent,register-only 從未發送(uHGemHT160.h:40; cpp:79);alarm-reset 語意在別處 | discovery 先行:確認 CJ_EAP 是否綁 CEID 30 為 Alarm-Reset。若是,對齊 — 但不可孤立 16→30(CEID 30 已被 register-only TimeEvent 佔) |
| CEID 31 | Press Tray End | MISSING | CEID 31 = RealDummy switch,fired main.cpp:1712;HT160 無專屬 Tray-End CEID | 除非 discovery 顯示 CJ_EAP 綁 CEID 31 為 Tray-End 否則不動。若要:新增專屬 Tray-End 事件 — 但 CEID 31 現為 RealDummy,9045-configured host 讀 31 會誤判 |
| CEID 32 | Press Tray Feed | MAP_SVID | CEID 32 未註冊(AddCEID loop cpp:268-271 僅 1-31 + 高位 35/36/37/148-150/272-275);tray-feed 語意在 HT160 CEID 10 | discovery:確認 CJ_EAP 需 CEID 32 為 Tray-Feed。若需,以連貫低頻段 remap 對齊(不可孤立 10→32,須處理整段分歧) |
| CEID 41 | One Cycle Finish | MAP_SVID | 語意事件登記為 CEID 27(OneCycleOK,uHGemHT160.h:37; :76)但無 fire site → 從未發送 | 重編號 OneCycleOK 27→41 **且補 fire site**。僅在 CJ_EAP 確實訂閱 one-cycle(工程/試跑事件)時才動;discovery-gated |
| CEID 42 | Clean Out Finish | MAP_SVID | 以 CEID 28 發送(CleanOutOK,uHGemHT160.h:38; :77)via EmitCleanOutOK(main.cpp:2675-2677);僅編號分歧 | 重編號 CleanOutOK 28→42 對齊 9045 字典(事件已發,僅號不同)。**注意:HT160 刻意讓 host 看 CEID 28 再 PressLotEnd 12,重編號前先驗序列假設** |
| CEID 49 | Tray Feed Finish | MAP_SVID | 登記為 CEID 29(TrayFeedOK,uHGemHT160.h:39; :78);CEID 49 未註冊,無 fire site → register-only | 重編號 TrayFeedOK 29→49 **且補 fire site**。Tray Feed 為維護/手動功能;host 訂閱才動 |
| CEID 76 | HasIC Press Start | MAP_SVID | 語意事件為 CEID 5(PressStartWithIC,uHGemHT160.h:15; :54);唯一 emit site COMMENTED OUT @main.cpp:2032 | 重編號 PressStartWithIC 5→76 **且取消註解 emit site**(main.cpp:2032)。niche 操作事件(spec V 僅 9045/9046);對 production EAP 低價值 |

### C-4 SVID 層 (4 條)

| 代號 | 名稱 | 類別 | HT160 現況 | 需實作動作 |
|---|---|---|---|---|
| 1518 | Real/Dummy For HT9045 (0:Dummy 1:Tray Only 2:Real) | MISSING | 未做 SVID/ECID,但三態值以 iRealDummy(DUMMY/HAS_TRAY/REALLY)真實存在於內部,只是未 SECS 曝露 | 將 iRealDummy 曝露為 SVID 1518(INT_4 0/1/2)使現場 RPTID 502 帶真值;否則 S2F33 至少須容忍。**真 partial 缺口:值存在,未曝露** |
| 20001 | EC Change ID | OPTIONAL | 無 EC-change 稽核 SVID;但 HT160 有 EC 目錄(AddEC 1501/2758-2763 @uHGemHT160.cpp:230-242)+ 規劃中的 S2F15 寫入路徑,稽核 SVID 可行 | 選配:S2F15 EC 寫入時記變更 ECID 到 ASCII SVID 20001。**Path A 最低要求 = S2F33 容忍它(host RPTID 504 / CEID 48)**。低優先 |
| 20002 | EC Change Original Value | OPTIONAL | 同 20001,同 EC-change 稽核族 | 選配:記 EC 變更前值到 ASCII SVID 20002。Path A 最低 = S2F33 容忍。低優先 |
| 20003 | EC Change New Value | OPTIONAL | 同 20001,同 EC-change 稽核族 | 選配:記 EC 變更後值到 ASCII SVID 20003。Path A 最低 = S2F33 容忍。低優先 |

---

## (d) finder 偽陽性修正表 (9 條)

複驗推翻的 finder 過度宣稱。

| 代號 | 修正事實 |
|---|---|
| S6F1 | finder 稱 S6F1 在 HT9045 absent,但 **9045 確有 equipment-initiated S6F1 Trace Data Send**:THGem::DoTraceDataResponse @uHGemEquipment.cpp:4288 做 InitLocalHead(6,1,0) + SendLocalData,由 S2F23 trace-init 驅動。對 9045 參照的「absent」宣稱錯誤;對 HT160 本身 S6F1 確為正確 absent,非 Path A 項。 |
| CEID 14 | 碰撞事實正確,但「Path A gap: Yes(add+renumber)」過度宣稱。Start-mode 切換是操作員設定動作,production fab EAP(CJ_EAP)不訂閱。host 以 S2F35 綁自己報表到 CEID;HT160 內部 CEID 號無關,除非 discovery 證明綁定。非 Path A 缺口。 |
| CEID 15 | 碰撞事實正確,但「renumber RecipeChange(2)→15」過度宣稱。HT160 **已發送** setup/recipe-change 事件(RecipeChange CEID 2, main.cpp:1500),事件存在僅號不同。整套 CEID enum 皆分歧,孤立重編一員不安全且對 report-driven host 無必要。非 Path A 缺口。 |
| CEID 16 | 碰撞事實正確,但「renumber ChangeUser 21→16」過度宣稱。user-level 事件 **已存在並發送**(ChangeUser CEID 21, main.cpp:1598),僅號不同。host 以 S2F35 綁 CEID;無綁定證明前號非 host-facing。非 Path A 缺口。 |
| CEID 17 | 碰撞事實正確,但「Path A gap」過度宣稱。「Enter Tool Page」為純操作員 UI 導航事件,production EAP 不訂閱;即便 9045 也是診斷 screen-trace。HT160 自有等價集在 22-26(register-only)。非 Path A 缺口。 |
| CEID 18 | 碰撞事實正確,但「renumber 23→18 + 補 fire site」過度宣稱。純 UI 導航事件,production EAP 不訂閱;HT160 已有 EnterMaintenPage(23) register-only。host 以 S2F35 綁報表,號非 host-facing。非 Path A 缺口。 |
| CEID 19 | 碰撞事實正確,但「renumber EnterTeach(25)→19」過度宣稱。純 UI 導航事件;HT160 EnterTeach(25) register-only。host 以 S2F35 綁自己報表,非設備 CEID 號。非 Path A 缺口。 |
| CEID 20 | 碰撞事實正確,但「add+renumber」過度宣稱。純 UI 導航事件,production EAP 不訂閱;即便 9045 也是診斷 screen-trace。host 以 S2F35 綁報表。非 Path A 缺口。 |
| CEID 21 | 碰撞事實正確(CEID 21=ChangeUser 有發送;EnterIOPage=24 fire site 已註解),但「renumber 24→21 + 取消註解」過度宣稱。「Enter I/O Page」為純 UI 導航事件,production EAP 不訂閱。host 以 S2F35 綁報表,號非 host-facing。非 Path A 缺口。 |

**偽陽性型態總結:** 9 條中 8 條是 CEID 層的同一錯誤模式 —— finder 把「HT160 CEID 編號與 9045 字典不同」誤判為「Path A 缺口」,忽略了 (1) 事件語意多已在 HT160 別的 CEID 上存在並發送,(2) host 透過 S2F35 綁定自己的報表到 CEID、不依賴設備端的 CEID 編號常數。剩 1 條(S6F1)是對 9045 參照事實的誤述。

---

## (e) 與現場的關聯:CJ_EAP RPTID 504 = {20001, 20002, 20003}

**這三個 EC-change SVID 本身不是 confirmed 硬缺口。** 複驗判定 20001/20002/20003 為 **gapReal = partial、類別 OPTIONAL**;HT160 尚無 EC-change 稽核 SVID,但有 EC 目錄與規劃中的 S2F15 寫入路徑,可日後補上。三者的建議動作一致寫著:「**Path A 最低要求 = S2F33 容忍它**」。

**現場擋 host 的直接原因是 S2F33 的未知-SVID 容忍行為,而非缺這三個 SVID:**

- 現場 CJ_EAP 以 S2F33 定義報表,RPTID 502 / 503 / 504 內含 HT160 目前沒有的 SVID(502→1517/1518…、503→6001/6002…、504→20001/20002/20003)。
- 若 HT160 的 S2F33 對未知 SVID 回 **DRACK = 0x04(拒絕)**,host 建報表即失敗 —— 這就是現場直接被擋的機轉。
- 若 S2F33 對未知 SVID **容忍**(接受定義、之後以空值/placeholder 回報),host 即可完成報表定義並繼續運作,縱使那些 SVID 暫時回空。

**定稿裁定:**
1. S2F33 的**訊息層處理**在本次複驗為 **已對齊(gapReal = no)** —— dispatch / ProcessDefineReport / DRACK 回覆齊備。
2. 但「對未知 SVID 是否 DRACK = 0x04」這一 **行為點** 是現場能否成立 RPTID 502/503/504 的前提,散見於 1517/1518/6001/6002/20001-3 各列的建議動作。此點雖未被列為獨立缺口列,卻是 **唯一已確認的現場直接擋 host 原因**,列為最高優先(見 (f) 高)。
3. 20001/20002/20003 的實際 **實作**(記錄 EC 變更稽核)為 **選配、低優先**;現場僅需 S2F33 容忍它們即可不擋 host。

---

## (f) 建議實作優先序

### 高 — 直接擋 host(現場唯一已確認直接原因)

| 項目 | 說明 |
|---|---|
| **S2F33 未知-SVID 容忍** | 確保/驗證 S2F33 對現場 RPTID 502/503/504 引用的未知 SVID **不以 DRACK = 0x04 拒絕**(接受定義並以空值回報)。這是現場 CJ_EAP 建報表被擋的直接機轉。優先於任何 SVID 個別實作。 |

### 中 — host 可能引用,回 placeholder / T3-safe ack 即可

| 項目 | 動作 |
|---|---|
| SVID 1518 (Real/Dummy) | 曝露內部 iRealDummy 為 SVID 1518 (INT_4 0/1/2),使 RPTID 502 帶真值(值已存在) |
| SVID 1517 / 6001 / 6002 / 20001-3 | 提供 placeholder 值(在 S2F33 容忍前提下,可先回空/0);EC-change 稽核(20001-3)實作為選配 |
| S2F17 / S2F18 | Date/Time 回覆(避免 host 對時間查詢 T3 逾時) |
| S2F25 / S2F26 | Loopback echo(標準鏈路診斷) |
| S2F29 / S2F30 | EC Namelist(由既有 EC 表組回覆) |
| S6F15 / S6F16、S6F19 / S6F20 | Event / Individual Report Request 回覆(host 若改用 pull 模式) |
| S10F3 / S10F4、S10F5 / S10F6 | Terminal Display 至少回 ACK10=0(W-bit 時)防逾時 |
| S125F1 / S125F2 | EC-change-report enable 的 spec-V 握手 ack |
| S1F15 / S1F16 | OFF-LINE 握手(回 OFLACK=0 + 設 iControlState) |

### 低 — tester-only 無 sorter 對應(回 N/A)或 discovery-gated

| 項目 | 說明 |
|---|---|
| 全部 CEID 重編號(yes: 1-5;partial: 6/8/9/30/31/32/41/42/49/76) | **全 discovery-gated。** host 以 S2F35 綁自己報表,不吃 HT160 CEID 號;僅在現場 S2F35 capture 證明 host 訂閱該低位 CEID 時才動。語意事件多已在別的 CEID 發送。 |
| S2F23 / S2F24 (Trace) | 被 event report(S2F33/35/37 + S6F11)取代;需要才加 ack-only |
| S5F3 (Alarm mask depth-align) | 現已回 ACKC5=0;深度對齊為選配 |
| S7F19 / S7F20 (Recipe inventory) | 非 CJ_EAP 的 event-report 集;確認 intentional 即可 |
| SVID 6001 / 6002 (Arm contact count) | sorter 無 test-contactor,無對應值 → N/A(僅需 S2F33 容忍) |
| RCMD tester-only(SWITCH_TO_FT/RT、AUTO_RETEST、CASSETTE_* 等) | tray sorter 無對應功能 → 回 N/A |

---

*定稿依據:221/221 對抗式複驗;file:line 引用取自各 verdict 的 ht160 現況欄。本文供 review。*
