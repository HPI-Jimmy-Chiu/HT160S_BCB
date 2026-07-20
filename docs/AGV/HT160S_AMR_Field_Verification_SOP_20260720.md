# HT160S AMR 現場驗證 SOP（真機 / 非 SOFT_SIMULATE）

文件狀態：操作手冊（v1）｜日期：2026/07/20｜適用版本：`HT160S_Program_BCB_V1.0.0.0`
範圍：在**真機（`SOFT_SIMULATE` 關閉）**現場，驗證 E87/AGV SECS 握手（叫車 CEID272 → START_AGV → Ready CEID273 → Finish CEID274 → START）。
姊妹文件：`HT160S_AMR_ManualInject_Test_SOP_20260720.md`（**sim / 筆電**單動測試）、`HT160S_E87_AGV_Operation_Manual.md`、`HT160S_E87_AGV_Communication_Draft_20260527.md`。

---

## 0. 先回答你的三個問題

| 問 | 答 |
|---|---|
| **1. 那個「不斷跑」的循環只有 SOFT_SIMULATE 才會發生？** | **是。** 循環的成因是 sim 下 273/274 的判斷式「硬回 true」，短路掉注入 latch 的清除。真機的判斷式是**讀真實 sensor**、不是硬 true，所以不會這樣循環。（且已修 `40a343d`，兩邊都穩。）|
| **2. 現場真機測試（不啟用 SOFT_SIMULATE）也會有問題嗎？** | **不會有那個 sim 循環。** 真機是 sensor 驅動：缺料/滿盤/取車都要 sensor 真的變化，握手才前進。若料一直沒補，機台會「停在等補料」而非 1 秒狂叫。若你在真機用注入測、且有按 Finish/Take，latch 會被正常消費清掉；`40a343d` 的修正讓混用（注入+真sensor）也不會殘留。|
| **3. 現場流程一樣嗎？要碰 sensor 嗎？** | **流程骨架一樣（272→273→274→START），但驅動來源不同**。真機有 **3 種驗證途徑**（§2）：跑真料/真AMR、**手動觸發 sensor**、或**用注入不碰 sensor**。要不要碰 sensor 由你選的途徑決定（見 §2、§7）。|

---

## 1. 真機 vs sim 的關鍵差異（最重要，先讀）

同一套程式，`SOFT_SIMULATE` 開/關時，AMR 判斷式行為完全不同：

| 邊緣（事件） | sim（SOFT_SIMULATE ON） | **真機（SOFT_SIMULATE OFF）** | 真機讀哪個 sensor |
|---|---|---|---|
| **272 叫車**（Auto 滿盤） | 比模擬盤數 `iSimAmrMaxTray` | 讀 **`SnAutoX_InputFullTray`**（Enable&&IsOn） | 滿盤 sensor，**ON=滿** |
| **272 叫車**（Loader/Empty/Color 缺料） | `iSimInfeedCount<=0` | 讀 **`SnX_Inputend`**（Enable&&IsOff） | 進料 sensor，**OFF=空(缺料)** |
| **273 Ready**（Auto 排空） | **硬回 true** | 真排空狀態：無 working/rear/pending 盤 + 殘料清 + 前缸 home | （非單一 sensor，機構狀態） |
| **273 Ready**（Loader 就緒） | **硬回 true** | 前堆疊汽缸 home | （汽缸 out-bit） |
| **274 Finish**（Auto 取車） | **硬回 true** | 讀 **`SnAutoX_InputEnd`**（Enable&&IsOff） | 取車 sensor，**ON=有盤；取走=OFF** |
| **274 Finish**（Loader 補料完成） | **硬回 true** | 讀 **`SnLoader_Inputend`**（Enable&&IsOn） | 同進料 sensor，**ON=有盤(補到了)** |

> **一句話**：sim 下 273/274 自動完成（所以只按第一顆就自跑，也因此會循環）；**真機下 273/274 必須「真的發生」**——不是機構真的排空/取車，就是你用注入去 override。

**這就是為什麼 sim 的循環在真機不會出現**：真機 274 之後，缺料 sensor 已變「有盤(ON)」→ 缺料解除 → 不再叫車。除非料真的又被抽走。

---

## 2. 現場三種驗證途徑（依你手上有什麼選）

| 途徑 | 需要 | 碰 sensor？ | 驗證到什麼 | 何時用 |
|---|---|---|---|---|
| **R. 真 sensor / 真料驅動** | 料盤/料匣，手動搬動 | **要**（手動放/取料匣、放/取滿車） | 真實 sensor 接線 + 極性 + 完整協定 | 想連 sensor 一起驗（建議至少做一輪）|
| **I. 注入驅動（AMR 測試模式）** | 只要面板 | **不用**（按鈕 bypass sensor） | SECS 協定 + host round-trip（**不**驗 sensor） | 沒真料/沒 AMR，或只想驗協定 |
| **A. 真 AMR 車** | 真實 AMR + host | 不用（AMR 動作 + sensor 自然變化） | 端到端全部（含車） | 最終整合驗證 |

> **完整現場驗收 = R（至少每站一輪，驗 sensor）＋ I（驗協定不受 sensor 影響）＋ 有車時做 A。**

### 2.1 這三種途徑用哪些設定切換？（都是現成功能，無需新開發）

**沒有單一「R/I/A 模式鍵」**。行為由下列**現成開關組合**決定：

| 層 | 開關 | 位置 | 作用 |
|---|---|---|---|
| ① 編譯 | `SOFT_SIMULATE` | `MachineType.h`（build 時） | 有 define＝**恆 sim**（筆電 dev）；註解掉＝真機 build，才啟用第 ② 層 |
| ② 執行期 Real/Dummy | 主畫面 **Real/Dummy 面板**（點一下循環 Dummy→HasTray→Real） | 主畫面 `pnRealDummy`；存 `lastset.ini [System] RealDummy` | **Real**＝真 sensor 驅動；**Dummy**＝sim 行為（273/274 自動完成）。**只在真機 build 有效**（筆電 build 恆 sim，切了沒用） |
| ③ 注入 | **Enable AMR manual-inject test mode** 勾選 | 維護 → AMR 頁 `chkAmrTestMode` | 勾＝注入按鈕生效（bypass sensor）；不勾＝靠 sensor / 實際 |
| ④ AMR 總開關 | `UseAMR=1` | `General.ini [HardwareInstall]` / 維護 | 沒開整個握手不啟動 |
| ⑤ Host | Auto-AGV / Automation | 模擬器連線列 | 自動回 `START_AGV`/`START`、自動 ACK |

途徑 → 設定對應：

| 途徑 | ① 編譯 | ② Real/Dummy | ③ 測試模式勾選 | 碰 sensor |
|---|---|---|---|---|
| **I 注入** | 任意 | 任意 | **勾 ON** | 不碰 |
| **R 真 sensor** | 真機 build（off） | **Real** | 不勾 | 要碰 |
| **A 真 AMR** | 真機 build（off） | Real | 不勾 | 不碰（車動） |
| （附）真機上跑 sim 行為 | 真機 build（off） | **Dummy** | 任意 | 不碰（273/274 自動） |

**重點**：
- **全部是現成功能**，不用新開發。
- 你**現在筆電跑的是 ① 有 define 的 dev build → 恆 sim**；主畫面把 Real/Dummy 切成 Real **也沒用**（`IsSoftSimulate()` 恆 true）。要驗真 sensor，必須用**真機 build（`SOFT_SIMULATE` 註解掉）**，再把主畫面切 **Real**。
- ②③ 獨立可組合：真機+Real+不勾＝純真 sensor（R）；真機+Real+勾＝可用注入 override 真機當下補不到的邊緣（例如沒車時的 Take）。

---

## 3. 前置條件（真機版，缺一不可）

| # | 條件 | 現場如何確認 |
|---|---|---|
| 1 | SECS/GEM 已啟用 | 主畫面出現 **SECS** badge |
| 2 | AMR 模式開啟（`bUseAMR=1`） | 主畫面 **AMR** badge 綠 ON；AMR 面板 `bUseAMR=1` |
| 3 | HSMS **SELECTED** | AMR 面板 `Selected=1`；host/模擬器 ● selected |
| 4 | **機台在 Normal 模式** | 送 272 的 `PollAndCall` 要 `RunMode==Run_Normal` |
| 5 | **★ 機台已 HOME 完成**（`fAllMotorHome==true`） | 非 HOMING；送 273/274 的 `ServiceHandshake` 閘門需 homed（與 sim 同）|
| 6 | **★ 確認相關 sensor 已 Enable 且極性正確** | **在維護 → IO Monitor / IOsetview 逐一確認**（見 §4）。**Enable 狀態是「每台機」config，勿假設；一定要在現場那台確認** |

> **注意**：本 repo 的 `system/IO_Table.csv` 目前把 `SnAutoX_InputEnd`、`SnX_Inputend`、`SnX_InputFullTray` 都設 **Enable=1** 並給了實體位址。但現場機台的 `IO_Table.csv` 可能不同——**務必在現場那台用 IO Monitor 實測**，不要照抄本文件。

---

## 4. Sensor 對照表（真機驗證的核心）

| 站/邊緣 | Sensor（Alias） | 觸發 predicate | 極性（真機） | 現場怎麼讓它成立 |
|---|---|---|---|---|
| Auto 滿盤（272） | `SnAuto1..6_InputFullTray` | `IsOutputCarFullForAmr` | **ON=滿** | 把該 Auto 輸出車裝滿 / 遮住滿盤 sensor |
| Auto 取車（274） | `SnAuto1..6_InputEnd` | `IsAmrTaken` | **ON=有盤；OFF=取走** | 把滿車移走（sensor 由 ON→OFF）|
| Loader 缺料（272） | `SnLoader_Inputend` | `IsInputShortageForAmr` | **OFF=空(缺料)** | 抽走 Loader 進料匣（sensor→OFF）|
| Loader 補料完成（274） | `SnLoader_Inputend` | `IsInputHandoffFinishedForAmr` | **ON=有盤** | 放回/放上料匣（sensor→ON）|
| Empty 缺料/補料 | `SnEmpty_InputEnd` | 同 Loader | 同上 | 同上 |
| Color 缺料/補料 | `SnColor_InputEnd` | 同 Loader | 同上 | 同上 |

- Auto 的 **273（排空）不是單一 sensor**：要該 Auto 真的把盤都堆進車、殘料清、前缸 home。真機若該 Auto 本來就空，273 會很快成立。
- Loader/Empty/Color 的 **273（就緒）** 看前堆疊汽缸 home（不是 sensor）。
- **同一顆 `SnLoader_Inputend` 同時扮演「缺料(OFF)」與「補料完成(ON)」**——這是 Loader 真機驗證能用「抽走→放回」單顆 sensor 走完全程的關鍵。

---

## 5. 驗證 R-1 — Loader 上料（真 sensor 驅動）｜泳道

前置：§3 全滿足；**不要**勾測試模式（這是真 sensor 途徑）。host/模擬器建議勾 Auto-AGV（自動回 START_AGV/START）。

```
 操作員(手動)        機台 HT160S                         Host / 模擬器
 ───────────        ─────────────────────────           ─────────────────
 抽走 Loader 料匣 ─► SnLoader_Inputend: ON→OFF(空)
                    IsInputShortageForAmr=true
                    ├─► TX S6F11 CEID272 (P1) ──────────► 收叫車
                    │                          ◄────────── TX S2F41 START_AGV(Loader)
                    └─◄ RX, TX S2F42 HCACK=0 ───────────► 收 ACK
                    hs=PREP；前缸 home
                    ├─► TX S6F11 CEID273 (P1 Ready) ─────► 收 Ready
 放上新料匣 ───────► SnLoader_Inputend: OFF→ON(有盤)
                    IsInputHandoffFinishedForAmr=true
                    ├─► TX S6F11 CEID274 (P1 Finish) ────► 收 Finish
                    │   解鎖 + 恢復進料              ◄──── TX S2F41 START
                    └─◄ RX START → HCACK=0(有Lot)/2(無Lot)
```

| 步 | 操作員做 | 機台 SECS log 應出現 | 判讀 |
|---:|---|---|---|
| 1 | **抽走** Loader 料匣（sensor→OFF） | `TX S6F11 CEID=272` bitmap `P1:1` | 缺料叫車 |
| 2 | （host 自動）| `RX S2F41 START_AGV(Loader)` → `TX S2F42 HCACK=0` | 派車受理 |
| 3 | （前缸 home）| `TX S6F11 CEID=273`（P1） | Ready |
| 4 | **放上**料匣（sensor→ON） | `TX S6F11 CEID=274`（P1） | 補料完成、解鎖 |

**通過準則**：抽走→272；放回→274；bitmap 全 P1；不再重覆叫（因為 sensor 已 ON）。Empty/Color 換對應 sensor（P2/P3）。

> **這一輪就驗證了：真實 sensor 接線 + 極性 + 完整 SECS 協定。** 要碰 sensor（手動抽/放料匣）。

---

## 6. 驗證 R-2 — Auto 下料（真料/真 AMR 驅動）｜泳道

Auto 下料要「先滿車、再取走」，比 Loader 多一個「排空」條件。真機要嘛跑真料填滿，要嘛用注入（§7）。

```
 操作員/生產         機台 HT160S                          Host / 模擬器
 ───────────        ─────────────────────────            ────────────────
 (Auto 輸出車裝滿) ─► SnAutoX_InputFullTray: OFF→ON(滿)
                    IsOutputCarFullForAmr=true
                    ├─► TX S6F11 CEID272 (P4) + CEID35 ──► 收叫車(+離散滿盤碼)
                    │                            ◄──────── TX S2F41 START_AGV(AUTO1)
                    └─◄ HCACK=0；鎖 Auto，TrayArm 停送
                    (該 Auto 盤全堆進車、殘料清、前缸home)
                    ├─► TX S6F11 CEID273 (P4 Ready) ─────► 收 Ready
 移走滿車 ─────────► SnAutoX_InputEnd: ON→OFF(取走)
                    IsAmrTaken=true
                    ├─► TX S6F11 CEID274 (P4 Finish) ────► 收 Finish
                    │   清車 + 解鎖 + 恢復收盤        ◄─── TX S2F41 START
                    └─◄ RX START → HCACK
```

| 步 | 動作 | 機台 log | 判讀 |
|---:|---|---|---|
| 1 | Auto1 輸出車滿（`SnAuto1_InputFullTray` ON） | `TX CEID=272` `P4:1` + `CEID=35` | 滿盤叫車 |
| 2 | （host）| `RX START_AGV(AUTO1)` → `HCACK=0` | 鎖車、派車 |
| 3 | 該 Auto 排空（盤堆進車） | `TX CEID=273`（P4） | Ready |
| 4 | **移走滿車**（`SnAuto1_InputEnd` ON→OFF） | `TX CEID=274`（P4） | 取車完成、清車解鎖 |

**通過準則**：滿→272；排空→273；車移走→274；bitmap 全 P4。Auto2..6＝P5..P9。

---

## 7. 驗證 I — 注入驅動（真機，不碰 sensor）｜泳道

沒真料/沒 AMR，只想驗協定時用。**真機的注入要按「每一個」邊緣**（不像 sim 會自動完成 273/274）。順序鐵律同 sim：**先 HOME → 再勾測試模式 → 再注入**。

```
 面板注入            機台 HT160S                          Host / 模擬器
 ────────           ─────────────────────────            ────────────────
 按 Loader Short ─► InputShort 注入=true
                   ├─► TX CEID272 (P1) ─────────────────► 收叫車
                   └─◄ RX START_AGV(Loader) → HCACK=0 ◄── (自動)
                   hs=PREP
 按 Loader Ready ─► InputReady 注入=true(或前缸本就home)
                   ├─► TX CEID273 (P1) ─────────────────► 收 Ready
 按 Loader Finish► InputFinish 注入(one-shot)
                   ├─► TX CEID274 (P1) ─────────────────► 收 Finish
                   │   ClearInputCycle 清 latch(40a343d) ◄─ RX START
                   └─◄ 按一次跑一輪就停（不循環）
```

- **真機注入必須三顆都按**（Short→Ready→Finish；Auto 為 Full→Drain→Take），因為真機 273/274 不會自動成立。
- Auto 的 **Take** 若現場 `SnAutoX_InputEnd` 已 Enable，可改用「真的移走車」讓 274 由真 sensor 成立（R-2）；沒車就用注入 Take。
- **不碰任何 sensor**，純驗 SECS 協定 + host 回覆。

---

## 8. 「現場到底要不要碰 sensor？」明確結論

- **要驗 sensor 接線/極性** → 用途徑 **R**（§5/§6）：**要**手動搬料匣/移車去觸發真 sensor。至少每站跑一輪。
- **只驗 SECS 協定、不想碰硬體** → 用途徑 **I**（§7）：**不用**碰 sensor，面板注入 bypass。但這樣**不會驗到 sensor**。
- **最終整合** → 途徑 **A**：真 AMR + 真 sensor，全自動。
- 不論哪種，**HOME 一定要先做**（HOME 是 motion 閘門，不是 sensor；§3 第 5 條）。

---

## 9. 驗收檢查表（真機）

- [ ] SECS badge、AMR badge = ON；`Selected=1`、`fAllMotorHome=true`、RunMode=Normal。
- [ ] （R）逐站確認 sensor Enable + 極性（IO Monitor）：`SnX_Inputend` OFF=空、`SnAutoX_InputFullTray` ON=滿、`SnAutoX_InputEnd` ON=有盤。
- [ ] （R）抽料匣→272；放料匣→274；移滿車→274；bitmap 站點正確。
- [ ] （I）注入三顆邊緣，每按一輪 272→273→274 完成後**自動停**（不循環）；面板 log 見 `... cycle cleared (274)`。
- [ ] START 回 HCACK=0（有載 Lot）或 HCACK=2（bench 無 Lot，正常）。
- [ ] host 端收到的 CEID/bitmap/SVID 與規格一致。

---

## 10. 疑難排解（真機）

| 症狀 | 原因 / 處置 |
|---|---|
| 抽走料匣沒送 272 | sensor 未 Enable 或極性反（IO Monitor 確認 `SnX_Inputend`）；或非 Normal / 未 SELECTED / 未 HOME |
| 送了 272，但放回料匣不出 274 | 補料 sensor 沒到 ON（`SnLoader_Inputend`）；或未先收 START_AGV（未進 PREP）；或未 HOME（273/274 閘門）|
| Auto 滿了不送 272 | `SnAutoX_InputFullTray` 未 Enable/未 ON；或非 Normal |
| Auto 到 273 卡住不出 274 | 車還沒真的移走（`SnAutoX_InputEnd` 仍 ON）；或該 sensor 未 Enable → 需改用注入 Take |
| 注入按了沒反應 | 沒勾測試模式，或勾了之後又 HOME/Start 被清（先 HOME 再勾）|
| 真機也一直重覆叫車 | **不是 sim 那個 bug**：多半是料真的一直缺（sensor 一直 OFF）→ 補料即停；或注入 Short 開著沒關（再按一次關掉）|
| `START` HCACK=2 | bench 沒載 Lot（正常）；要 HCACK=0 需先 `SET_LOT_INFO`/`LOTSTART`+WebAPI 載入 |

---

## 11. 附錄

### 11.1 站點 P mapping
P1 Loader / P2 Empty / P3 Color / P4-P9 = Auto1-6。

### 11.2 CEID
272 叫車(SVID38219) / 273 Ready(38220) / 274 Finish(38221) / 275 CarrierID。Auto Full 附帶離散碼 35/36/37、148/149/150。

### 11.3 Sensor 位址（本 repo config，現場請以實機為準）
| Sensor | IP | Port | Bit | Enable |
|---|---|---|---|---|
| SnAuto1..6_InputEnd | 2 | 1 | 0..5 | 1 |
| SnLoader_Inputend | 2 | 0 | 3 | 1 |
| SnEmpty_InputEnd | 2 | 0 | 2 | 1 |
| SnColor_InputEnd | 6 | 2 | 4 | 1 |
| SnAuto1..6_InputFullTray | 3-6 | 0-2 | — | 1 |

### 11.4 Predicate 程式位置
| 邊緣 | 函式 | 檔案 |
|---|---|---|
| Auto 272 | `IsOutputCarFullForAmr` | aAuto1To6.cpp:1331 |
| Auto 273 | `IsDrainedForAmr` | aAuto1To6.cpp:1375 |
| Auto 274 | `IsAmrTaken` | aAuto1To6.cpp:1403 |
| Loader 272 | `IsInputShortageForAmr` | aLoader.cpp:539 |
| Loader 273 | `IsReadyForAmrHandoff` | aLoader.cpp:525 |
| Loader 274 | `IsInputHandoffFinishedForAmr` | aLoader.cpp:548 |
| 握手/閘門/latch清 | `ServiceHandshake`（gate:324、清latch:360/405）| SecsGem/uAgvStation.cpp |

### 11.5 與 sim SOP 的關係
sim 版（`HT160S_AMR_ManualInject_Test_SOP_20260720.md`）用於筆電無硬體時驗協定；本現場版用於真機。兩者協定相同，差別在 273/274 由真 sensor（真機）還是硬回 true（sim）驅動。sim 的無限循環（注入 level latch 不自清）已於 `40a343d` 修正，真機本無此問題。

---

*本 SOP 對齊 IO_Table.csv（Enable 欄）與程式碼（aAuto1To6.cpp / aLoader.cpp / uAgvStation.cpp）撰寫；行為若與程式衝突以程式為準，Enable 狀態以現場實機為準。*
