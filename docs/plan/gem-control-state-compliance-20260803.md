# GEM 控制狀態合規化實作計畫 (2026-08-03)

**決策**：使用者拍板「全做」——依 SEMI E30 把控制狀態做成規範行為，客戶端若不符規範由文件說明。
**前置研究**：`docs/plan/` 無前案；可行性評估來自 2026-08-03 的 4 角度 × 攻防 workflow（wf_dc9bd082-b69）。
**現場權威資料**：`D:\HT160S_StateRecord\2026-07-31 16_38_37\`（`General.ini [SECS]`：`ActiveMode=0`
`Port=6000` `DeviceID=0`；SecsLog 17 檔 = 當天完整雙向 SML）。

---

## 0. 不可違反的約束（每一階段都要重新確認）

| # | 約束 | 為什麼 |
|---|---|---|
| C1 | **階段 1 必須先落地** | E30 只允許從 HOST OFF-LINE 上線。本機**零操作員上線控制**，且 7/31 唯一讓機台上線的是 host 的 S1F17 打在 `iControlState==0`。先做閘門＝機台永久鎖死＝合規工作自己造成停機 |
| C2 | **v1 斷線不重設控制狀態** | 7/31 有 76/97 筆 primary（含全部 5 筆 S2F41 與 3 次 START_AGV）落在一條**沒有 S1F13、沒有 S1F17** 的連線上。照規範重設＝那 76 筆全被拒＝AMR 交握停死。客戶 EAP 在自己送 Separate 後確實不重新交握 |
| C3 | **SVID 66002 值域與 report 1 形狀不得變** | 66002 是凍結的 13 格 report 1 第 7 格，全天 76 筆 S6F11 都帶它，值域已書面承諾 1/4/5。子狀態另尋住所 |
| C4 | **SVID 4/9 不得動** | `MapGemControlState9045()` 已把非 4/5 折成 1，兩號自動保持正確 |
| C5 | **CEID 141/91/92/93 必須豁免 outbound 閘** | 它們在狀態已變成 1 之後才發，會被自己的閘門消音——那是昨天才對客戶承諾的事件 |
| C6 | **`ONLINE_*` 類 RCMD 永遠放行** | 否則一旦離線就再也上不了線 |
| C7 | 新註解 ASCII English、無 C++11、元件定義進 DFM、DFM 事件須 `__published` | 專案規則 |

---

## 階段 1 — 讓合規「可上線」的地基（線上僅 ONLACK 可見）

- [x] **1.1 `THGem::SendAbort(int Stream)`** — SxF0 送出。`InitLocalHead` 把偶數 Function 當 reply、
      自動沿用被拒交易的 SystemBytes；F=0 可編、產生 14-byte header-only frame。
- [x] **1.2 `LogSmlBody` 對 F==0 抑制 dump** — 否則每次拒絕都印 `[SML parse error rc=-1]`，工程師會誤判故障。
- [x] **1.3 子狀態 member + 單一 setter** — `iControlSubstate`（1=EQUIPMENT OFF-LINE / 2=ATTEMPT ON-LINE /
      3=HOST OFF-LINE / 0=線上不適用）+ `SetControlState()`；五個 `iControlState` 寫入點全部改走它。
      66002 與 SVID 4/9 **byte 不變**（C3/C4）。
- [x] **1.4 操作員控制 UI** — `maintenance.dfm` 的 `tsMaintSECS` 頁籤（**已存在**，且兩個 DFM 皆純 ASCII）
      新增一個 `pnlSecsControlState` 面板：三個按鈕（Off-Line / On-Line Local / On-Line Remote）+
      `chkAcceptHostOnline` + 狀態標籤。樣式沿用同檔 `pnlSecsOverride` / `chkSortArmAutoSkip` 先例。
- [x] **1.5 `[SECS]` 兩個新設定鍵** — `InitialControlState`(預設 5 = On-Line Remote) 與
      `AcceptHostOnlineRequest`(預設 1)，走 `GeneralSetting` 的 `bAskSkipICCount` 先例。
- [x] **1.6 開機不再是 0** — `PollGemControlState()` 第一 tick 一次性套用 `InitialControlState`。
      殺掉「66002 送出 0（不在自己公佈值域內）」這個已出貨的文件違反。**開機預設必須是 ON-LINE**。
- [x] **1.7 `S1F18` ONLACK 0/1/2** — 對齊 9045：已在線回 2、操作員允許回 0、不允許回 **1 拒絕**。
      本機原本一律硬回 0。
- [x] **1.8 `S1F16` 落在 HOST OFF-LINE** — 使 host 自己要求的離線可由 host 自己收回（E30）。
- [~] 建置閘：lint + compile + link exit 0；headless 實測確認狀態機與 ONLACK；**141+91/92/93 的伴隨事件驗證未完成**（共用工作樹另一 session 同時建置，PCH 競爭 + exe 被取代，需在樹靜止時重驗）

## 階段 2 — 收訊閘門與 HCACK 值域（**線上可見**）

- [ ] **2.1 HCACK 值域改為 E5** — 參數錯誤 `2 -> 3`（~20 處）、`1` 保留給命令不存在、`2` 專用於
      「現在不能執行」。**這會改變客戶 EAP 現在收到的位元**，工作簿與模擬器須同版重出。
- [ ] **2.2 S2F41 依狀態設閘** — OFF-LINE → `S2F0`；ON-LINE LOCAL → A 類（運動/製程）回 `HCACK=2`；
      C 類（`ONLINE_*`）永遠放行（C6）。B 類（資料/設定）在 LOCAL 放行並記錄理由。
- [ ] **2.3 明文不重設** — `OnCommunicationLost()` 不動控制狀態，並在程式碼與規格書寫明是 v1 的
      刻意偏離（C2）。
- [ ] 建置閘 ×3 + 實測 + commit

## 階段 3 — 完整 OFF-LINE 閘門、spooling、機台主動 S1F13

- [ ] **3.1 COMMUNICATIONS 狀態模型** — 本機**完全沒有**（無 `bCommEstablished`、無 T5、
      `S1F13` sender 只是 log-only stub）。E30 的 OFF-LINE 規則與 ATTEMPT ON-LINE 都靠它。
- [ ] **3.2 機台主動 S1F13** — Select 完成即送；收 `S1F14 COMMACK=0` → ATTEMPT ON-LINE → 預設狀態。
      這是讓 82.5% 拒絕率塌縮到 ~2 筆的槓桿。**預設關閉**（若 CJ_EAP 不回 S1F14，合規機台會永遠停在
      ATTEMPT ON-LINE = 自己造成的鎖死）。
- [ ] **3.3 完整收訊閘** — 除 S1F13/F14、S1F17（且僅 HOST OFF-LINE）外全部 `SxF0`。這就是 80/97 懸崖。
- [ ] **3.4 outbound 壓制 + CEID 141/91/92/93 豁免**（C5）
- [ ] **3.5 spooling** — S2F43/S6F23/S6F24 + SPOOLCNT/SPOOLFULL + 持久佇列。沒有它，壓制＝**永久丟失**
      （7/31 會吃掉 17 筆 S6F11 + **當天唯一的警報** set/clear 兩筆）。
- [ ] **3.6 回線警報再同步** — set 與 clear 跨狀態變更會讓 host 永遠卡在舊警報。
- [ ] 建置閘 ×3 + 實測 + commit

## 階段 4 — 文件

- [ ] 規格書 §1/§3.1/§3.3/§4 + 新增控制狀態專節
- [ ] 客戶工作簿重出（SVID/CEID/功能/修訂說明 四表 + xlsx）
- [ ] 共用 SECS 模擬器同步（`D:\AI_Area\Tool\HT160S_SECS_Simulator`）

---

## 已知偏離規範之處（必須寫進客戶文件）

1. **斷線不重設控制狀態**（C2）——待京元確認 CJ_EAP 是否每連線重送 S1F13/S1F17 後才改。
2. **機台主動 S1F13 預設關閉**（3.2）——待確認 CJ_EAP 會回 S1F14。
3. 兩個 OFF-LINE 子狀態之間切換**不發 CEID**（141/91/92/93 對折疊值觸發，C4 的代價）。
