# HT160S 警報 / 警示 檢修對應手冊
> 版本日期：2026-07-13 ｜ 來源：機台的 `system/AlarmList.csv` + `system/IO_Table.csv` 與實機行為
> 涵蓋：**576** 則 — 汽缸 55×6=330、馬達 20×9=180、真空 1×6=6、流程字串碼 60

本手冊每一則警報都以「**應該狀態（健康）→ 目前狀態（故障）**」對照呈現，並指出要在哪個畫面（IOsetview / MotionView / State Record）查哪個 IO、汽缸、馬達或真空點。

---

## 目錄
1. [如何使用本手冊](#一如何使用本手冊)
2. [警報分類與編碼](#二警報分類與編碼)
3. [汽缸類 4xxxx](#三汽缸類-4xxxx)
4. [馬達／伺服類 5xxxx](#四馬達伺服類-5xxxx)
5. [真空吸嘴類 6xxxx](#五真空吸嘴類-6xxxx)
6. [系統流程類 JAM／MES／WAR](#六系統流程類-jammeswar)
7. [附錄：快速查碼與位址說明](#七附錄快速查碼與位址說明)

---

## 一、如何使用本手冊
本手冊將 HT160S 全機 576 個警報 (alarm) 與警示 (warning) 對應到「檢修判讀」：每一則警報的本質都是**某個裝置沒有達到機台預期的狀態**。因此每一節都以「**應該是什麼狀態 (健康) vs. 目前是什麼狀態 (故障)**」呈現，讓操作員與維修人員能快速鎖定要查看的 IO、汽缸、馬達或真空點。

### 1.1 警報的三個通道
發生警報時，同一則訊息會走三個通道，可從不同地方查到：

| 通道 | 內容 | 在哪裡看 |
|---|---|---|
| **Code → SECS ALID** | 警報代碼 (如 `40203` / `JAM1030`)，對應 SECS 主機的 alarm 表 | 主機端 / EventLog |
| **Message → 畫面 + EventLog + SECS ALTX** | 一行文字訊息 (可能帶 `[IO=感測器名]`) | 螢幕紅字、EventLog |
| **Detail → Note 備註** | 詳細說明 (含觸發 IO 全文與驅動流程軌跡) | 警報彈窗 (Note) 的備註欄 |

汽缸／馬達的排隊式警報，在**發生當下**會於 EventLog 寫入一行軌跡紀錄，載明「是哪個裝置、由哪個動作、在流程的哪一步」在驅動該裝置。這行紀錄是事後追查原因的關鍵。

### 1.2 三個現場查看工具
本手冊指向三個現場查看工具：

- **IOsetview (IO 監看畫面)** — 查 sensor / switch / 汽缸到位訊號的即時 ON/OFF。以 **Alias (名稱)** 搜尋 (例如 `SnLoader_InputHasTray`、`C_Empty_PushTray_On`)。
  - ⚠️ 綁定成功的點永遠不會變紅；「綠/灰但永遠不變」代表該節點的 MotionNet 讀取失敗 (位址／環號設定問題)，並非顯示問題。
- **MotionView (馬達畫面)** — 查馬達命令位置、編碼器位置、軟／硬體極限、伺服狀態。馬達以 `[M01] MSortingArmX` 之類的編號+別名標示。
- **State Record / EventLog** — 查邏輯狀態、盤流計數、以及警報前的動作軌跡。純邏輯類 (如盤數不符) 無單一 IO，需在此查對應的計數／旗標。

---

## 二、警報分類與編碼
警報代碼分四大族：

| 代碼族 | 範圍 | 類型 | 產生方式 |
|---|---|---|---|
| **汽缸 Cylinder** | `4xxxx` | 到位逾時 | `4` + 三碼汽缸序號 + 一碼錯誤別 (共 55 汽缸 × 6 = 330) |
| **馬達 Motor** | `5xxxx` | 伺服／極限 | `5` + 三碼馬達序號 + 一碼錯誤別 (共 20 馬達 × 9 = 180) |
| **真空 Sucker** | `6xxxx` | 真空／掉件 | `6` + 三碼吸嘴組序號 + 一碼錯誤別 (1 組 × 6 = 6) |
| **流程 JAM/MES/WAR** | 字串碼 | 盤流／視覺／計數 | 對齊 HT9045 客戶既有代碼，機台內以字串登錄 (60 筆) |

編碼規則：代碼由「族別 (1 碼) + 裝置序號 (3 碼) + 錯誤別 (1 碼)」串成一組數字。例：`40203` = 汽缸族(4)、序號 020、錯誤別 3 → 第 20 號汽缸 `C_Auto1_PushTray` 的「伸出無法到位」。

---

## 三、汽缸類 4xxxx
### 3.1 動作與警報機制
每個汽缸都有三個要素：一個**輸出線圈**（電磁閥），一個**伸出到位 sensor**（別名 = 汽缸名 + `_On`），一個**縮回到位 sensor**（別名 = 汽缸名 + `_Off`）。

- **伸出動作**：線圈通電（ON）→ 等伸出到位 sensor 變 ON；若在伸出逾時（預設 5000ms）內仍未到位 → 發**伸出逾時碼 `…3`「can not on」**。
- **縮回動作**：線圈斷電（OFF）→ 等縮回到位 sensor 變 ON；若在縮回逾時內仍未到位 → 發**縮回逾時碼 `…0`「can not off」**。
- 若某 sensor 在 IO_Table 內**停用（Enable=0）**，該行程會直接視為到位、**不做確認也不發到位警報**。因此請先確認該 sensor 是否啟用，再判斷警報。

**只有 `…0`（縮回逾時）與 `…3`（伸出逾時）兩碼由現行伸出／縮回流程觸發**；同一汽缸另外 4 碼（`…1/…2/…4/…5`）已登錄於代碼表但現行流程不觸發（保留）。

### 3.2 六種錯誤子類（`4`＋序號＋錯誤別）
| 錯誤別 | 行程 | 訊息 | 現行觸發？ | 應該狀態 | 目前狀態(故障) |
|---|---|---|---|---|---|
| `…0` | 縮回 | `can not off error` | 是（縮回逾時） | 縮回到位 sensor(_Off) = ON、輸出線圈 = OFF | 縮回到位 sensor(_Off) 仍 = OFF（逾時內未確認） |
| `…3` | 伸出 | `can not on error` | 是（伸出逾時） | 伸出到位 sensor(_On) = ON、輸出線圈 = ON | 伸出到位 sensor(_On) 仍 = OFF（逾時內未確認） |
| `…1` | 縮回 | `can not on error` | 否（保留，現行流程不觸發） | — | — |
| `…2` | 縮回 | `off sensor is on error` | 否（保留） | 同時只應有一個到位 sensor 為 ON | 縮回 sensor 與伸出 sensor 同時 ON（機構不可能）= sensor 卡死/短路/接錯 |
| `…4` | 伸出 | `can not off error` | 否（保留） | — | — |
| `…5` | 伸出 | `on sensor is on error` | 否（保留） | 同時只應有一個到位 sensor 為 ON | 伸出 sensor 與縮回 sensor 同時 ON = sensor 卡死/短路/接錯 |

### 3.3 檢修順序
**伸出無法到位（`…3` can not on）／縮回無法到位（`…0` can not off）檢修順序**：

1. **氣壓** — 總氣壓是否足夠（`SnAirIsEnough` 是否 ON）？該汽缸分歧氣壓/流量閥是否正常？
2. **氣管/電磁閥** — 線圈是否有作動聲/得電（IOsetview 看該汽缸輸出）？氣管是否脫落、折到、漏氣？電磁閥是否卡住？
3. **機構** — 是否被盤/IC/異物卡住？滑軌、連桿是否卡死？行程是否被干涉？
4. **到位 sensor** — 對照本手冊該汽缸的 sensor 位址，在 IOsetview 手動推到位看 sensor 是否會 ON？感測距離/位置是否跑掉？
5. **配線** — sensor→控制卡的配線、接頭是否鬆脫；InType（常開/常閉）是否與實際相符。

### 3.4 各子系統到位確認概況
| 子系統 | 汽缸數 | 伸出到位確認 | 縮回到位確認 |
|---|---|---|---|
| TrayArm 送盤手臂 | 4 | 4/4 | 0/4 |
| Empty 空盤供給 | 7 | 5/7 | 4/7 |
| Loader 進料 | 8 | 7/8 | 6/8 |
| Color 色帶/覆蓋盤供給 | 6 | 4/6 | 3/6 |
| Auto1 出料站 | 5 | 4/5 | 4/5 |
| Auto2 出料站 | 5 | 4/5 | 4/5 |
| Auto3 出料站 | 5 | 4/5 | 4/5 |
| Auto4 出料站 | 5 | 4/5 | 4/5 |
| Auto5 出料站 | 5 | 4/5 | 4/5 |
| Auto6 出料站 | 5 | 4/5 | 4/5 |

> 「伸出／縮回到位確認」= 該 sensor 有配線且啟用，能真正發到位逾時警報。未列入者靠逾時但無 sensor 確認。

本機組態下，以下汽缸**輸出線圈為停用（未安裝/未使用）**，其警報不會出現，若出現代表組態被更動：

- `C_Auto1_FrontSeparateTray_1`
- `C_Auto2_FrontSeparateTray_1`
- `C_Auto3_FrontSeparateTray_1`
- `C_Auto4_FrontSeparateTray_1`
- `C_Auto5_FrontSeparateTray_1`
- `C_Auto6_FrontSeparateTray_1`
- `C_Color_RearRiseTray`

### 3.5 全汽缸參考表
> 欄位：伸出碼(`…3`)／縮回碼(`…0`)、輸出線圈 IO、伸出到位 sensor、縮回到位 sensor。位址格式 `L環號/IP/P埠/b位元`；以 IOsetview 用 **Alias 名稱**搜尋最準。逾時為 On/Off AlarmTime(ms)。

#### TrayArm 送盤手臂
| 汽缸 | 伸出碼 | 縮回碼 | 輸出線圈 | 伸出到位 sensor | 縮回到位 sensor | 逾時ms |
|---|---|---|---|---|---|---|
| `C_TrayArmZ_Up` | 40003 | 40000 | L0/IP0/P2/b5 (啟用) | `C_TrayArmZ_Up_On` L0/IP0/P1/b5 常閉(NC) (啟用) | `C_TrayArmZ_Up_Off` 未配線 — (停用) | 5000 |
| `C_TrayArmZ_Down` | 40013 | 40010 | L0/IP0/P2/b4 (啟用) | `C_TrayArmZ_Down_On` L0/IP0/P1/b4 常閉(NC) (啟用) | `C_TrayArmZ_Down_Off` 未配線 — (停用) | 5000 |
| `C_TrayArm_FrontClamp` | 40023 | 40020 | L0/IP0/P2/b6 (啟用) | `C_TrayArm_FrontClamp_On` L0/IP0/P1/b6 常閉(NC) (啟用) | `C_TrayArm_FrontClamp_Off` 未配線 — (停用) | 5000 |
| `C_TrayArm_RearClamp` | 40033 | 40030 | L0/IP0/P2/b7 (啟用) | `C_TrayArm_RearClamp_On` L0/IP0/P1/b7 常閉(NC) (啟用) | `C_TrayArm_RearClamp_Off` 未配線 — (停用) | 5000 |

#### Empty 空盤供給
| 汽缸 | 伸出碼 | 縮回碼 | 輸出線圈 | 伸出到位 sensor | 縮回到位 sensor | 逾時ms |
|---|---|---|---|---|---|---|
| `C_Empty_FrontRiseTray_1` | 40043 | 40040 | L0/IP8/P0/b0 (啟用) | `C_Empty_FrontRiseTray_1_On` L0/IP2/P0/b0 常閉(NC) (啟用) | `C_Empty_FrontRiseTray_1_Off` L0/IP1/P0/b1 常閉(NC) (啟用) | 5000 |
| `C_Empty_FrontRiseTray_2` | 40053 | 40050 | L0/IP8/P3/b6 (啟用) | `C_Empty_FrontRiseTray_2_On` L0/IP1/P0/b0 常閉(NC) (啟用) | `C_Empty_FrontRiseTray_2_Off` 未配線 — (停用) | 5000 |
| `C_Empty_PushTray` | 40063 | 40060 | L0/IP8/P0/b1 (啟用) | `C_Empty_PushTray_On` L0/IP1/P0/b3 常閉(NC) (啟用) | `C_Empty_PushTray_Off` L0/IP1/P0/b2 常閉(NC) (啟用) | 5000 |
| `C_Empty_LeanOnTray` | 40073 | 40070 | L0/IP8/P0/b2 (啟用) | `C_Empty_LeanOnTray_On` L0/IP1/P1/b0 常閉(NC) (啟用) | `C_Empty_LeanOnTray_Off` L0/IP1/P1/b1 常閉(NC) (啟用) | 5000 |
| `C_Empty_FrontSeparateTray_1` | 40083 | 40080 | L0/IP8/P0/b3 (啟用) | `C_Empty_FrontSeparateTray_1_On` 未配線 — (停用) | `C_Empty_FrontSeparateTray_1_Off` 未配線 — (停用) | 5000 |
| `C_Empty_RearRiseTray` | 40093 | 40090 | L0/IP8/P0/b7 (啟用) | `C_Empty_RearRiseTray_On` L0/IP1/P1/b4 常閉(NC) (啟用) | `C_Empty_RearRiseTray_Off` L0/IP1/P1/b5 常閉(NC) (啟用) | 5000 |
| `C_Empty_RearSeparateTray_1` | 40103 | 40100 | L0/IP8/P1/b0 (啟用) | `C_Empty_RearSeparateTray_1_On` 未配線 — (停用) | `C_Empty_RearSeparateTray_1_Off` 未配線 — (停用) | 5000 |

#### Loader 進料
| 汽缸 | 伸出碼 | 縮回碼 | 輸出線圈 | 伸出到位 sensor | 縮回到位 sensor | 逾時ms |
|---|---|---|---|---|---|---|
| `C_Loader_FrontRiseTray_1` | 40113 | 40110 | L0/IP8/P1/b4 (啟用) | `C_Loader_FrontRiseTray_1_On` L0/IP2/P0/b1 常閉(NC) (啟用) | `C_Loader_FrontRiseTray_1_Off` L0/IP1/P1/b7 常閉(NC) (啟用) | 5000 |
| `C_Loader_FrontRiseTray_2` | 40123 | 40120 | L0/IP8/P3/b7 (啟用) | `C_Loader_FrontRiseTray_2_On` L0/IP1/P1/b6 常閉(NC) (啟用) | `C_Loader_FrontRiseTray_2_Off` 未配線 — (停用) | 5000 |
| `C_Loader1_PushTray` | 40133 | 40130 | L0/IP8/P1/b5 (啟用) | `C_Loader1_PushTray_On` L0/IP1/P2/b1 常閉(NC) (啟用) | `C_Loader1_PushTray_Off` L0/IP1/P2/b0 常閉(NC) (啟用) | 5000 |
| `C_Loader2_PushTray` | 40143 | 40140 | L0/IP8/P1/b6 (啟用) | `C_Loader2_PushTray_On` L0/IP1/P2/b3 常閉(NC) (啟用) | `C_Loader2_PushTray_Off` L0/IP1/P2/b2 常閉(NC) (啟用) | 5000 |
| `C_Loader1_LeanOnTray` | 40153 | 40150 | L0/IP8/P1/b7 (啟用) | `C_Loader1_LeanOnTray_On` L0/IP1/P3/b0 常閉(NC) (啟用) | `C_Loader1_LeanOnTray_Off` L0/IP1/P3/b1 常閉(NC) (啟用) | 5000 |
| `C_Loader2_LeanOnTray` | 40163 | 40160 | L0/IP8/P2/b0 (啟用) | `C_Loader2_LeanOnTray_On` L0/IP1/P3/b2 常閉(NC) (啟用) | `C_Loader2_LeanOnTray_Off` L0/IP1/P3/b3 常閉(NC) (啟用) | 5000 |
| `C_Loader_FrontSeparateTray_1` | 40173 | 40170 | L0/IP8/P2/b1 (啟用) | `C_Loader_FrontSeparateTray_1_On` 未配線 — (停用) | `C_Loader_FrontSeparateTray_1_Off` 未配線 — (停用) | 5000 |
| `C_Loader_RearRiseTray` | 40183 | 40180 | L0/IP8/P2/b5 (啟用) | `C_Loader_RearRiseTray_On` L0/IP1/P3/b6 常閉(NC) (啟用) | `C_Loader_RearRiseTray_Off` L0/IP1/P3/b7 常閉(NC) (啟用) | 5000 |

#### Color 色帶/覆蓋盤供給
| 汽缸 | 伸出碼 | 縮回碼 | 輸出線圈 | 伸出到位 sensor | 縮回到位 sensor | 逾時ms |
|---|---|---|---|---|---|---|
| `C_Color_FrontRiseTray_1` | 40493 | 40490 | L0/IPW/P0/b7 (啟用) | `C_Color_FrontRiseTray_1_On` L0/IP6/P2/b5 常閉(NC) (啟用) | `C_Color_FrontRiseTray_1_Off` L0/IP6/P1/b3 常閉(NC) (啟用) | 5000 |
| `C_Color_FrontRiseTray_2` | 40503 | 40500 | L0/IPW/P1/b0 (啟用) | `C_Color_FrontRiseTray_2_On` L0/IP6/P1/b2 常閉(NC) (啟用) | `C_Color_FrontRiseTray_2_Off` 未配線 — (停用) | 5000 |
| `C_Color_PushTray` | 40513 | 40510 | L0/IPW/P1/b1 (啟用) | `C_Color_PushTray_On` L0/IP6/P1/b5 常閉(NC) (啟用) | `C_Color_PushTray_Off` L0/IP6/P1/b4 常閉(NC) (啟用) | 5000 |
| `C_Color_LeanOnTray` | 40523 | 40520 | L0/IPW/P1/b2 (啟用) | `C_Color_LeanOnTray_On` L0/IP6/P2/b1 常閉(NC) (啟用) | `C_Color_LeanOnTray_Off` L0/IP6/P2/b2 常閉(NC) (啟用) | 5000 |
| `C_Color_RearRiseTray` | 40533 | 40530 | 未配線 (停用) | `C_Color_RearRiseTray_On` 未配線 — (停用) | `C_Color_RearRiseTray_Off` 未配線 — (停用) | 5000 |
| `C_Color_FrontSeparateTray_1` | 40543 | 40540 | L0/IPW/P1/b3 (啟用) | `C_Color_FrontSeparateTray_1_On` 未配線 — (停用) | `C_Color_FrontSeparateTray_1_Off` 未配線 — (停用) | 5000 |

#### Auto1 出料站
| 汽缸 | 伸出碼 | 縮回碼 | 輸出線圈 | 伸出到位 sensor | 縮回到位 sensor | 逾時ms |
|---|---|---|---|---|---|---|
| `C_Auto1_FrontRiseTray` | 40193 | 40190 | L0/IP8/P2/b6 (啟用) | `C_Auto1_FrontRiseTray_On` L0/IP3/P0/b0 常閉(NC) (啟用) | `C_Auto1_FrontRiseTray_Off` L0/IP3/P0/b1 常閉(NC) (啟用) | 5000 |
| `C_Auto1_PushTray` | 40203 | 40200 | L0/IP8/P2/b7 (啟用) | `C_Auto1_PushTray_On` L0/IP3/P0/b3 常閉(NC) (啟用) | `C_Auto1_PushTray_Off` L0/IP3/P0/b2 常閉(NC) (啟用) | 5000 |
| `C_Auto1_LeanOnTray` | 40213 | 40210 | L0/IP8/P3/b0 (啟用) | `C_Auto1_LeanOnTray_On` L0/IP3/P1/b0 常閉(NC) (啟用) | `C_Auto1_LeanOnTray_Off` L0/IP3/P1/b1 常閉(NC) (啟用) | 5000 |
| `C_Auto1_RearRiseTray` | 40223 | 40220 | L0/IP8/P3/b1 (啟用) | `C_Auto1_RearRiseTray_On` L0/IP3/P1/b4 常閉(NC) (啟用) | `C_Auto1_RearRiseTray_Off` L0/IP3/P1/b5 常閉(NC) (啟用) | 5000 |
| `C_Auto1_FrontSeparateTray_1` | 40233 | 40230 | L0/IP8/P3/b2 (停用) | `C_Auto1_FrontSeparateTray_1_On` 未配線 — (停用) | `C_Auto1_FrontSeparateTray_1_Off` 未配線 — (停用) | 5000 |

#### Auto2 出料站
| 汽缸 | 伸出碼 | 縮回碼 | 輸出線圈 | 伸出到位 sensor | 縮回到位 sensor | 逾時ms |
|---|---|---|---|---|---|---|
| `C_Auto2_FrontRiseTray` | 40243 | 40240 | L0/IP8/P2/b5 (啟用) | `C_Auto2_FrontRiseTray_On` L0/IP3/P1/b6 常閉(NC) (啟用) | `C_Auto2_FrontRiseTray_Off` L0/IP3/P1/b7 常閉(NC) (啟用) | 5000 |
| `C_Auto2_PushTray` | 40253 | 40250 | L0/IP8/P3/b1 (啟用) | `C_Auto2_PushTray_On` L0/IP3/P2/b1 常閉(NC) (啟用) | `C_Auto2_PushTray_Off` L0/IP3/P2/b0 常閉(NC) (啟用) | 5000 |
| `C_Auto2_LeanOnTray` | 40263 | 40260 | L0/IP8/P3/b2 (啟用) | `C_Auto2_LeanOnTray_On` L0/IP3/P2/b6 常閉(NC) (啟用) | `C_Auto2_LeanOnTray_Off` L0/IP3/P2/b7 常閉(NC) (啟用) | 5000 |
| `C_Auto2_RearRiseTray` | 40273 | 40270 | L0/IP9/P0/b3 (啟用) | `C_Auto2_RearRiseTray_On` L0/IP3/P3/b2 常閉(NC) (啟用) | `C_Auto2_RearRiseTray_Off` L0/IP3/P3/b3 常閉(NC) (啟用) | 5000 |
| `C_Auto2_FrontSeparateTray_1` | 40283 | 40280 | L0/IP9/P0/b4 (停用) | `C_Auto2_FrontSeparateTray_1_On` 未配線 — (停用) | `C_Auto2_FrontSeparateTray_1_Off` 未配線 — (停用) | 5000 |

#### Auto3 出料站
| 汽缸 | 伸出碼 | 縮回碼 | 輸出線圈 | 伸出到位 sensor | 縮回到位 sensor | 逾時ms |
|---|---|---|---|---|---|---|
| `C_Auto3_FrontRiseTray` | 40293 | 40290 | L0/IP9/P1/b0 (啟用) | `C_Auto3_FrontRiseTray_On` L0/IP3/P3/b4 常閉(NC) (啟用) | `C_Auto3_FrontRiseTray_Off` L0/IP3/P3/b5 常閉(NC) (啟用) | 5000 |
| `C_Auto3_PushTray` | 40303 | 40300 | L0/IP9/P1/b1 (啟用) | `C_Auto3_PushTray_On` L0/IP3/P3/b7 常閉(NC) (啟用) | `C_Auto3_PushTray_Off` L0/IP3/P3/b6 常閉(NC) (啟用) | 5000 |
| `C_Auto3_LeanOnTray` | 40313 | 40310 | L0/IP9/P1/b2 (啟用) | `C_Auto3_LeanOnTray_On` L0/IP4/P0/b4 常閉(NC) (啟用) | `C_Auto3_LeanOnTray_Off` L0/IP4/P0/b5 常閉(NC) (啟用) | 5000 |
| `C_Auto3_RearRiseTray` | 40323 | 40320 | L0/IP9/P1/b3 (啟用) | `C_Auto3_RearRiseTray_On` L0/IP4/P1/b0 常閉(NC) (啟用) | `C_Auto3_RearRiseTray_Off` L0/IP4/P1/b1 常閉(NC) (啟用) | 5000 |
| `C_Auto3_FrontSeparateTray_1` | 40333 | 40330 | L0/IP9/P1/b4 (停用) | `C_Auto3_FrontSeparateTray_1_On` 未配線 — (停用) | `C_Auto3_FrontSeparateTray_1_Off` 未配線 — (停用) | 5000 |

#### Auto4 出料站
| 汽缸 | 伸出碼 | 縮回碼 | 輸出線圈 | 伸出到位 sensor | 縮回到位 sensor | 逾時ms |
|---|---|---|---|---|---|---|
| `C_Auto4_FrontRiseTray` | 40343 | 40340 | L0/IP9/P2/b0 (啟用) | `C_Auto4_FrontRiseTray_On` L0/IP5/P0/b0 常閉(NC) (啟用) | `C_Auto4_FrontRiseTray_Off` L0/IP5/P0/b1 常閉(NC) (啟用) | 5000 |
| `C_Auto4_PushTray` | 40353 | 40350 | L0/IP9/P2/b1 (啟用) | `C_Auto4_PushTray_On` L0/IP5/P0/b3 常閉(NC) (啟用) | `C_Auto4_PushTray_Off` L0/IP5/P0/b2 常閉(NC) (啟用) | 5000 |
| `C_Auto4_LeanOnTray` | 40363 | 40360 | L0/IP9/P2/b2 (啟用) | `C_Auto4_LeanOnTray_On` L0/IP5/P1/b0 常閉(NC) (啟用) | `C_Auto4_LeanOnTray_Off` L0/IP5/P1/b1 常閉(NC) (啟用) | 5000 |
| `C_Auto4_RearRiseTray` | 40373 | 40370 | L0/IP9/P2/b3 (啟用) | `C_Auto4_RearRiseTray_On` L0/IP5/P1/b4 常閉(NC) (啟用) | `C_Auto4_RearRiseTray_Off` L0/IP5/P1/b5 常閉(NC) (啟用) | 5000 |
| `C_Auto4_FrontSeparateTray_1` | 40383 | 40380 | L0/IP9/P2/b4 (停用) | `C_Auto4_FrontSeparateTray_1_On` 未配線 — (停用) | `C_Auto4_FrontSeparateTray_1_Off` 未配線 — (停用) | 5000 |

#### Auto5 出料站
| 汽缸 | 伸出碼 | 縮回碼 | 輸出線圈 | 伸出到位 sensor | 縮回到位 sensor | 逾時ms |
|---|---|---|---|---|---|---|
| `C_Auto5_FrontRiseTray` | 40393 | 40390 | L0/IP9/P3/b0 (啟用) | `C_Auto5_FrontRiseTray_On` L0/IP5/P1/b6 常閉(NC) (啟用) | `C_Auto5_FrontRiseTray_Off` L0/IP5/P1/b7 常閉(NC) (啟用) | 5000 |
| `C_Auto5_PushTray` | 40403 | 40400 | L0/IP9/P3/b1 (啟用) | `C_Auto5_PushTray_On` L0/IP5/P2/b1 常閉(NC) (啟用) | `C_Auto5_PushTray_Off` L0/IP5/P2/b0 常閉(NC) (啟用) | 5000 |
| `C_Auto5_LeanOnTray` | 40413 | 40410 | L0/IP9/P3/b2 (啟用) | `C_Auto5_LeanOnTray_On` L0/IP5/P2/b6 常閉(NC) (啟用) | `C_Auto5_LeanOnTray_Off` L0/IP5/P2/b7 常閉(NC) (啟用) | 5000 |
| `C_Auto5_RearRiseTray` | 40423 | 40420 | L0/IP9/P3/b3 (啟用) | `C_Auto5_RearRiseTray_On` L0/IP5/P3/b2 常閉(NC) (啟用) | `C_Auto5_RearRiseTray_Off` L0/IP5/P3/b3 常閉(NC) (啟用) | 5000 |
| `C_Auto5_FrontSeparateTray_1` | 40433 | 40430 | L0/IP9/P3/b4 (停用) | `C_Auto5_FrontSeparateTray_1_On` 未配線 — (停用) | `C_Auto5_FrontSeparateTray_1_Off` 未配線 — (停用) | 5000 |

#### Auto6 出料站
| 汽缸 | 伸出碼 | 縮回碼 | 輸出線圈 | 伸出到位 sensor | 縮回到位 sensor | 逾時ms |
|---|---|---|---|---|---|---|
| `C_Auto6_FrontRiseTray` | 40443 | 40440 | L0/IPW/P0/b2 (啟用) | `C_Auto6_FrontRiseTray_On` L0/IP5/P3/b4 常閉(NC) (啟用) | `C_Auto6_FrontRiseTray_Off` L0/IP5/P3/b5 常閉(NC) (啟用) | 5000 |
| `C_Auto6_PushTray` | 40453 | 40450 | L0/IPW/P0/b3 (啟用) | `C_Auto6_PushTray_On` L0/IP5/P3/b7 常閉(NC) (啟用) | `C_Auto6_PushTray_Off` L0/IP5/P3/b6 常閉(NC) (啟用) | 5000 |
| `C_Auto6_LeanOnTray` | 40463 | 40460 | L0/IPW/P0/b4 (啟用) | `C_Auto6_LeanOnTray_On` L0/IP6/P0/b4 常閉(NC) (啟用) | `C_Auto6_LeanOnTray_Off` L0/IP6/P0/b5 常閉(NC) (啟用) | 5000 |
| `C_Auto6_RearRiseTray` | 40473 | 40470 | L0/IPW/P0/b5 (啟用) | `C_Auto6_RearRiseTray_On` L0/IP6/P1/b0 常閉(NC) (啟用) | `C_Auto6_RearRiseTray_Off` L0/IP6/P1/b1 常閉(NC) (啟用) | 5000 |
| `C_Auto6_FrontSeparateTray_1` | 40483 | 40480 | L0/IPW/P0/b6 (停用) | `C_Auto6_FrontSeparateTray_1_On` 未配線 — (停用) | `C_Auto6_FrontSeparateTray_1_Off` 未配線 — (停用) | 5000 |

---

## 四、馬達／伺服類 5xxxx

全機 20 顆馬達（以 MC88X1 伺服為主），每顆有 9 種錯誤別。代碼組成方式為：開頭 `5`＋三碼馬達序號＋一碼錯誤別（例如 `50013`）。下表為 9 種錯誤的判讀；再接全馬達對照表。

### 4.1 九種錯誤別判讀

| 錯誤別 | 意義 | 應該狀態 | 目前狀態(故障) | 在哪看 |
|---|---|---|---|---|
| `…0` Power Off | 該軸伺服電源掉電（電源繼電器 SwMotorRelay／電源感測 SnMotorPower 失電），軸在待命時就失去激磁。 | 驅動器已激磁：SwMotorRelay ON、SnMotorPower 有電；馬達狀態格裡該軸的 Alarm／ServoAlarm 燈是暗的。 | 該軸的驅動器警報、卡片伺服警報、到位訊號同時亮起，代表放大器已斷電或激磁在待命中掉落。 | IOsetview（看 SwMotorRelay 輸出／SnMotorPower 輸入）＋主畫面馬達狀態格（該軸 Alarm／ServoAlarm／InPos 燈）。 |
| `…1` Out Of Torque | 該軸在移動中失速、超過扭力上限，編碼器跟不上指令。 | 沒有警報；每次定位動作結束會到位；指令位置與編碼器位置在容許誤差內收斂。 | 警報亮起但還沒到位，代表軸在負載下卡住／移動中超出扭力，編碼器落後指令。 | MotionView（看該軸指令位置與編碼器位置的落差）＋馬達狀態格（Alarm＋ServoAlarm 亮、InPos 暗）。 |
| `…2` CW sensor ON | 該軸壓到或衝過正向（CW／+LM）硬體極限。 | 正向硬體極限訊號 OFF（軸在行程範圍內）；IOsetview 中該極限燈是暗的。 | 正向極限訊號 ON，代表軸實體頂到或衝過正向端點。 | IOsetview（該軸的 CW／+LM 極限輸入燈）＋MotionView（現在位置落在正向極端）。 |
| `…3` CCW sensor ON | 該軸壓到或衝過負向（CCW／-LM）硬體極限。 | 負向硬體極限訊號 OFF（軸在行程範圍內）；IOsetview 中該極限燈是暗的。 | 負向極限訊號 ON，代表軸實體頂到或衝過負向端點。 | IOsetview（該軸的 CCW／-LM 極限輸入燈）＋MotionView（現在位置落在負向極端）。 |
| `…4` Soft P position | 該軸到達或超過正向軟體極限 SoftLimitP。 | 現在位置落在 SoftLimitN～SoftLimitP 之間；正向軟極限旗標沒亮。 | 位置到達或超過 SoftLimitP，卡片正向軟極限被觸發。 | MotionView（現在位置對照該軸的 SoftLimitP 值）。 |
| `…5` Soft N position | 該軸到達或超過負向軟體極限 SoftLimitN。 | 現在位置落在 SoftLimitN～SoftLimitP 之間；負向軟極限旗標沒亮。 | 位置到達或低於 SoftLimitN，卡片負向軟極限被觸發。 | MotionView（現在位置對照該軸的 SoftLimitN 值）。 |
| `…6` position Error（需回 HOME＋重啟） | 該軸位置／跟隨誤差過大，驅動器鎖定位置偏差警報（沒有伴隨極限或卡片伺服警報）。 | 沒有警報；編碼器跟得上指令（跟隨誤差在容許內）；軸能到達並維持每個目標。 | 驅動器因位置偏差過大而鎖定警報，但沒有任何極限或卡片伺服警報同時亮起。 | MotionView（指令位置與編碼器位置的落差）＋馬達狀態格（Alarm 亮、所有極限／伺服警報燈暗）。 |
| `…7` Undefine | 讀到一組無法歸類的狀態，通常是瞬間跳動或未被建模的警報來源，正常運轉不該出現。 | 真正的故障都會落到已定義的錯誤別；此類代碼正常不會出現。 | 狀態快照沒有任何可辨識的警報位，可能是瞬間雜訊，或是系統未涵蓋的警報來源。 | State Record・EventLog（看跳出的 5xxx7 代碼與當下的狀態快照），再回馬達狀態格即時確認是否持續。 |
| `…8` Target will Out Of Limit | 要下達的目標位置超出軟體極限範圍，移動被事先擋下（不是實際撞到極限）。 | 要求的目標落在 SoftLimitN～SoftLimitP 之間，移動照常進行。 | 要求的目標落在軟極限範圍外，移動被擋下；Note 會印出目標值／現在位置／軟極限 N～P（單位 1/100mm）。 | Note 備註列（印出的目標／現在／極限值，前綴為該軸別名）＋MotionView（設定的 SoftLimitN／P 對照 teach 目標）。 |

**處置：**

- `…0` Power Off：恢復伺服／主電源。鎖定的過載或伺服警報無法用軟體清除，請把電源繼電器 SwMotorRelay 關再開做一次斷電重置，然後對該軸重新回 HOME（HOME 會刻意切斷再恢復馬達電源，本來就負責這種復歸）。
- `…1` Out Of Torque：先排除該軸的機構卡死／過載，檢查扭力上限與負載、齒輪；用 SwMotorRelay 斷電重置清除驅動器警報，再重新回 HOME。
- `…2` CW sensor ON：用寸動把該軸往負向（離開 CW 極限）退出，檢查 +LM 極限開關的接線與安裝，並確認有無機構衝過頭。注意：回 HOME 過程中壓到極限屬正常，這個警報只有在運轉中才會停機。
- `…3` CCW sensor ON：用寸動把該軸往正向（離開 CCW 極限）退出，檢查 -LM 極限開關與機構衝過頭。回 HOME 中壓到極限屬正常，只有運轉中才會停機。
- `…4` Soft P position：用寸動把該軸退回軟極限範圍內；確認 teach 目標與該軸的 SoftLimitP 設定；若位置基準跑掉就重新回 HOME。
- `…5` Soft N position：用寸動把該軸退回軟極限範圍內；確認 teach 目標與該軸的 SoftLimitN 設定；若位置基準錯誤就重新回 HOME。
- `…6` position Error（需回 HOME＋重啟）：對該軸重新回 HOME 並重啟。鎖定的放大器警報無法用軟體清除，請先用 SwMotorRelay 斷電重置再回 HOME；並排查偏差原因（編碼器聯軸、增益、機構阻力）。
- `…7` Undefine：重新掃描該軸；若重複發生，視為未歸類的驅動器警報，先做 SwMotorRelay 斷電重置＋重新回 HOME，並回報維修人員檢查狀態訊號或接線異常。
- `…8` Target will Out Of Limit：修正 teach 位置或要下達的目標，使其落在 SoftLimitN～SoftLimitP 範圍內，或修正該軸的 SoftLimitN／SoftLimitP 設定；若軸的位置基準跑掉也要重新回 HOME（基準遺失會讓原本合法的目標被判為超界）。

### 4.2 全馬達對照表

| 編號 | 別名 Alias | 起始碼 | 碼範圍(9碼) |
|---|---|---|---|
| M01 | `MSortingArmX` | `50000` | `50000`~`50008` |
| M02 | `MTrayArmX` | `50010` | `50010`~`50018` |
| M03 | `MEmptyY` | `50020` | `50020`~`50028` |
| M04 | `MLoaderY_1` | `50030` | `50030`~`50038` |
| M05 | `MLoaderY_2` | `50040` | `50040`~`50048` |
| M06 | `MAutoY_1` | `50050` | `50050`~`50058` |
| M07 | `MAutoY_2` | `50060` | `50060`~`50068` |
| M08 | `MAutoY_3` | `50070` | `50070`~`50078` |
| M09 | `MAutoY_4` | `50080` | `50080`~`50088` |
| M10 | `MAutoY_5` | `50090` | `50090`~`50098` |
| M11 | `MAutoY_6` | `50100` | `50100`~`50108` |
| M12 | `MTopCCDX` | `50110` | `50110`~`50118` |
| M13 | `MBottomCCDY` | `50120` | `50120`~`50128` |
| M14 | `MSuckZ_1` | `50130` | `50130`~`50138` |
| M15 | `MSuckZ_2` | `50140` | `50140`~`50148` |
| M16 | `MSuckZ_3` | `50150` | `50150`~`50158` |
| M17 | `MSuckZ_4` | `50160` | `50160`~`50168` |
| M18 | `MPitchX` | `50170` | `50170`~`50178` |
| M19 | `MColorY` | `50180` | `50180`~`50188` |
| M20 | `MTopCCDX_Color` | `50190` | `50190`~`50198` |

> 例：`50013` = 馬達序號 001（`MTrayArmX`，M02）錯誤別 3（CCW 負向極限 ON）。

---

## 五、真空吸嘴類 6xxxx
分類手臂真空吸嘴組 `SortArmSuck`（1 組 4 嘴），共 6 種錯誤別（`6000x`）。四嘴的真空感測器代號為 SuckAa / SuckAb / SuckAc / SuckAd（即 Suck1～Suck4）。

| 代碼 | 意義 | 應該狀態 | 目前狀態(故障) | 查看 | 處置 |
|---|---|---|---|---|---|
| `60000` | 吸取確認失敗：吸嘴下壓、開真空(關吹氣閥、開吸氣閥)後，在警報等待時間內真空感測器一直沒讀到 ON，代表 IC 沒被吸上吸嘴。 | 吸取指令後，該嘴真空感測器（SuckAa～SuckAd）在警報時間內讀到 ON（吸住了）。 | 真空感測器超過警報等待時間仍讀到 OFF（沒吸住）。 | IOsetview — 故障嘴的真空感測器 LED。吸嘴下壓後應立即亮起；故障時始終不亮。 | 先確認來源盤格內是否真的有 IC（空格本來就吸不到，屬正常）。若有 IC 但感測器不亮：檢查氣壓／真空供應壓力、是否漏氣或吸嘴嘴頭堵塞磨損、密封不良（IC 歪斜），並在 IOsetview 輸出確認吸氣電磁閥確實作動、感測器配線正常。常見原因：無 IC／漏氣／吸嘴堵塞／真空不足／感測器失效。排除後按 RETRY 重試，或按 SKIP 跳過。 |
| `60001` | 釋放確認失敗：關真空並吹氣(關吸氣閥、開吹氣閥)後，在警報等待時間內真空感測器仍未落到 OFF，代表 IC 沒從吸嘴放開。 | 釋放指令後真空感測器在警報時間內落到 OFF（IC 已放入目標盤格）。 | 真空感測器超過警報等待時間仍讀到 ON（沒放開）。 | IOsetview — 放料嘴的真空感測器 LED。吹氣時應熄滅；故障時仍亮著。 | 在 IOsetview 輸出與氣路確認吹氣閥確實作動且有正壓空氣。檢查 IC 是否被吸嘴黏住、吸氣電磁閥是否真的關閉。若無真空但感測器仍亮，懷疑感測器卡在 ON 或短路。常見原因：IC 黏住沒放開／電磁閥卡在 ON／無吹氣空氣／感測器卡 ON。排除後按 RETRY 或 SKIP。 |
| `60002` | 持料途中失壓：某支標記為「正持有 IC」的吸嘴，其真空感測器在搬運途中讀到 OFF，屬瞬間或部分失壓的初判。 | 吸嘴持有 IC 期間，真空感測器持續穩定讀到 ON。 | 搬運途中真空感測器讀到 OFF（真空掉了）。此為在確認掉落(60003)之前的早期徵兆。 | IOsetview — 持料嘴的真空感測器 LED。搬運中應恆亮；故障時閃爍或掉 OFF。 | 檢查該嘴是否出現漏氣或密封邊緣不良、負載下氣壓下降、嘴頭部分堵塞、或感測器／接頭接觸不良而間歇。此判定僅在 REALLY 實機模式生效，SOFT_SIMULATE 模式不會觸發。常見原因：漏氣／密封不良／堵塞／感測器間歇。 |
| `60003` | 持料途中 IC 掉落：持料嘴的真空感測器讀到 OFF 並超過掉落去彈跳時間仍為 OFF（手臂正從 Loader 往 Auto 搬運）。機台會減速停下所有動作，該嘴的 IC 記為未放置。 | 持有 IC 期間真空感測器持續 ON，未出現確認的 OFF。 | 真空感測器 OFF 且持續超過掉落去彈跳時間，確認掉料並停機。 | IOsetview — 掉料嘴的真空感測器 LED；並看 Note 備註列，會印出掉落盤格身分（如「SortArm IC Dropped At Pick／In Transit R.. C.. 2D=..」）。 | 從機台內取回掉落的 IC，再找密封完全失效的原因：突發真空流失、吸嘴磨損或破裂、IC 搬運途中勾到、或真空壓力崩潰。與 60002 的差別在於本碼是去彈跳確認且會停機。取料後：於取料端掉落可按 RETRY 復原來源盤格，或按 SKIP 放棄該格。常見原因：掉料（密封破損）／真空驟失。 |
| `60004` | 初始自檢：開機／重置時真空已通電，吸嘴的真空感測器本應確認 ON；若感測器卡 OFF 或真空路徑失效即屬此類。此為僅登錄於警報清單的分類代碼，實機上並無任何流程會實際觸發它；相同「真空開不起來」的情況在實機上會以吸取失敗 60000 呈現。 | （設計語意）初始化通真空時感測器讀到 ON。 | （設計語意）通真空後感測器仍為 OFF；實機無任何流程會實際觸發本碼，它只是警報清單內的分類項。 | IOsetview — 該嘴真空感測器 LED；此情況下通真空時不會亮。 | 視為文件／分類用途。若真的發生，根因為感測器始終未拉 ON：檢查真空產生器是否失效、無氣源、感測器脫落或卡 OFF、配線問題；實務上以 60000 的相同項目診斷。可於 State Record／EventLog 確認本碼只是登錄項而非實際發生事件。 |
| `60005` | 初始自檢：開機／重置時，未指派 IC 的吸嘴本應無真空（感測器 OFF），但真空感測器卻讀到 ON。此為僅登錄於警報清單的分類代碼，實機並不會於執行時實際觸發。 | 閒置／初始化且未指派 IC 時，真空感測器讀到 OFF。 | （設計語意）初始化時空嘴上真空感測器卻讀到 ON；因無實際流程接上，目前不會於執行時觸發。 | IOsetview — 該嘴真空感測器 LED；此情況下開機無持料卻亮著。 | 若真空感測器在初始無持料時亮起：檢查真空電磁閥是否卡在 ON 或誤接（常閉）、感測器是否卡 ON 或短路、或上一循環殘留的 IC 仍黏在嘴上。常見原因：閥卡 ON／感測器卡 ON／殘留 IC。可於 State Record／EventLog 確認 60005 只是登錄項還是實際發生事件。 |

---

## 六、系統流程類 JAM／MES／WAR
這一類是**盤流／視覺／計數**層級的警報，源自各模組的邏輯判斷（非單純到位逾時）。每一則列出這則警報的意義、常見原因，以及要檢查的裝置「應該→目前」狀態與現場處置。

### 6.1 Loader 進料

#### `JAM0913` — Loader 盤子掉在載台上（Tray Lost On Carriage）
- **意義**：前段拆盤完成、盤子理應落在 LoaderY 載台上並被載台有盤感測器 `SnLoader_InputHasTray` 讀到「有盤」，但確認時卻讀到「無盤」，代表這一盤在拆盤／推入過程中掉了或沒到位。此警報只在真機模式（非 SOFT_SIMULATE）且該感測器有啟用時才會判定。
- **常見原因**：
  - 拆下的盤在前拆盤機下降與後續推盤動作中掉落、卡住或分盤失敗，沒有正確落在 LoaderY 載台上；
  - `SnLoader_InputHasTray` 載台有盤感測器故障、偏位或脫線，盤子在上面卻讀不到（這是唯一被判定條件讀取的裝置）；
  - LoaderY 載台沒停在正確的進料 Y 位置，盤子落點偏離感測器；
  - 上游推盤氣缸 `C_Loader1_PushTray / C_Loader2_PushTray` 或靠盤氣缸 `C_Loader1_LeanOnTray / C_Loader2_LeanOnTray` 沒推到定位，盤子還沒送到載台就進了確認步驟。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnLoader_InputHasTray` | sensor | 讀到 ON（載台上有盤，確認通過） | 讀到 OFF（載台空、拆下的盤沒落上去，或感測器故障） | IOsetview（也會印在 Note 備註列） |
  | `C_Loader1_PushTray / C_Loader2_PushTray` | cylinder | 已推出到位，把盤送上載台 | 判定不直接讀它，僅上游嫌疑：若沒推到位，盤就到不了載台，導致 `SnLoader_InputHasTray` 讀 OFF | IOsetview（電磁閥輸出＋到位感測點） |
  | `C_Loader1_LeanOnTray / C_Loader2_LeanOnTray` | cylinder | 已推出到位，搬送中夾住／頂住分開的盤 | 判定不直接讀它，僅上游嫌疑：若沒作動，盤沒被頂住，可能在確認前從載台滑掉 | IOsetview（電磁閥輸出＋到位感測點） |
  | `MMLoaderY_1 / MMLoaderY_2` 的 `fHasTray`(邏輯) | logic | 尚未建立盤身分（確認通過後才登錄盤別與序號） | 這條警報路徑不會登錄任何盤身分，所以 SKIP/RETRY 都不需回滾 | State Record・EventLog |
- **處置**：先打開 Loader 前門，取出任何卡在拆盤機與 LoaderY 載台之間的盤，避免重跑時把第二盤再壓上去。確認載台確實停在進料位、盤能落在感測器上。清好後：按 RETRY 從頭重跑整段進料；或按 SKIP 結束這次進料（不帶盤，因為沒登錄任何盤身分，不需回滾）。若拿掉盤後感測器仍讀不到「有盤」，多半是該感測器或線路故障，請報修。

#### `MES0920` — Loader 進料源已空（Tray Empty）
- **意義**：進料時偵測不到可拆的盤——供料車讀到「已空」（`SnLoader_Inputend` OFF），或推盤氣缸的到位感測器一直沒確認有盤到拆盤處。此警報在 CleanOut 清機模式下不會發出；若有啟用 AMR，會先等一個完整的等料窗（`iAmrFeedWaitSec` 秒）沒補到料才發。
- **常見原因**：
  - 供料匣／進料車真的沒盤了，`SnLoader_Inputend` 讀 OFF（最常見的正常情況）；
  - 啟用 AMR 時，AMR 在 `iAmrFeedWaitSec`(設定) 時限內沒把料補上，等料逾時；
  - 推盤氣缸到位感測器 `C_Loader1_PushTray / C_Loader2_PushTray` 一直沒確認有盤實際到拆盤處；
  - `SnLoader_Inputend` 進料源存量感測器故障或脫線，車上有料卻誤讀為空。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnLoader_Inputend` | sensor | 讀到 ON（供料車還有料） | 讀到 OFF（供料車已空／進料源沒盤，或感測器脫線誤讀為空） | IOsetview（也會印在 Note 備註列） |
  | `C_Loader1_PushTray / C_Loader2_PushTray` 到位確認 | cylinder | 到位感測讀 ON（推盤氣缸已到推出位，代表有盤實際到達拆盤處） | 到位感測讀 OFF（拆盤處沒感測到盤，或氣缸沒到位） | IOsetview（此氣缸感測不印在 Note 列，直接看 LED） |
  | AMR 等料計時／等待旗標(邏輯) | logic | 已清除（等料窗關閉前盤已到，等待被解除）※僅 AMR 啟用時有作用 | 等料窗逾時仍沒補到料，才落到本警報；未啟用 AMR 時為立即發警 | State Record・EventLog（邏輯／計時狀態，無單一 IO 點） |
- **處置**：補滿 Loader 供料匣（或等 AMR／重新叫車補料），然後按 RETRY。RETRY 會從「載台確認」那一步接續，不會重新拆盤，所以不會把已分開的盤再夾／切一次。若要放空整機，改按 CLEAN OUT 進入清機模式把機內剩餘 IC 全數排出。若確定車上有料卻仍報空，請在 IOsetview 確認 `SnLoader_Inputend` 在有料時會亮 ON、推盤氣缸到位感測在盤到拆盤處時會亮 ON，排除感測器誤讀。

#### `MES0921` — Loader 盤數與感測器不符（Tray Count Mismatch）
- **意義**：啟用 AMR 時，SECS 主機宣告的整車盤數（`iCarTrayTotal`(邏輯)）已被進料序號 `iFeedSerial`(邏輯) 全數消耗完，但供料車存量感測器 `SnLoader_Inputend` 卻仍讀到有盤——盤數與硬體不一致。此警報在 CleanOut 模式下不會發出，且不掛特定感測器名稱，Note 上不印 IO 名。
- **常見原因**：
  - SECS 主機下的整車盤數比車上實際裝的盤少（`iCarTrayTotal` 太低）；
  - 車上實際多裝了超出宣告數的盤；
  - `iFeedSerial` 計數失準（某盤重複計或漏計），導致提早看似用完；
  - `SnLoader_Inputend` 卡在 ON／誤觸發，車其實已空卻讀有盤。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `iCarTrayTotal`(邏輯)（SECS 宣告的整車盤數，到車時鎖定） | logic | 等於車上實際盤數（含 IC 盤、cover、身分盤） | 已被消耗完（宣告數扣掉已進數 ≤ 0），但車其實還沒空 | State Record・EventLog |
  | `iFeedSerial`(邏輯)（本車已進盤數） | logic | 小於整車盤數，代表還有盤可進 | 已達或超過整車盤數（計數說車已放空） | State Record・EventLog |
  | `SnLoader_Inputend` | sensor | 讀到 OFF（與「車已放空」一致） | 讀到 ON（仍有盤實際存在，與計數矛盾） | IOsetview |
- **處置**：先實地清點車上還剩幾盤，對照主機宣告數。按 RETRY 會把感測器仍看得到的那盤進掉並繼續生產；或按 CLEAN OUT 進入清機排空模式。若確認是主機盤數給錯，請通知修正 AMR／SECS 端的盤數來源；若是 `SnLoader_Inputend` 卡 ON，在 IOsetview 檢修該感測器。

#### `MES0924` — Loader 後段有殘留盤（Rear Leftover Tray）
- **意義**：Loader 後段出料底部放著一盤，但系統沒有把它標記為「可被 TrayArm 取走」、也沒有任何排盤動作在進行，等於這盤永遠取不走、會讓 TrayArm 空等。只在真機模式（非 DUMMY）下判定，且每次事件只發一次。通常發生在：開機時後段本來就壓著一盤，或排盤中途被中止、盤別／盤號沒登錄，Loader 重置時無法保留後段狀態。健康狀態應為後段淨空（OFF）。
- **常見原因**：
  - 冷開機／上電時後段出料底部本來就實體留著一盤；
  - 排盤中途被中止，留下一盤盤別／盤號不明的盤——系統未登錄它為可取，為避免把 cover／身分盤誤送進正常料流，須由操作員手動移除；
  - `SnLoader_OutputBottomHasTray` 誤讀為 ON（感測器卡住或偏位，後段其實是空的），使系統誤判後段被佔用。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnLoader_OutputBottomHasTray` | sensor | 讀到 OFF（正常流程下後段出料底部淨空） | 讀到 ON（後段被一盤殘留盤佔用；真機下會使後段判定為「有盤」） | IOsetview（也會印在 Note 備註列） |
  | `bRearReadyForPick`(邏輯) | logic | 對「正常排出、TrayArm 可取」的後段盤為 TRUE | FALSE——殘留盤的盤別／盤號不明，從未被標記為可取 | State Record・EventLog |
  | `bRearDischargeInProgress`(邏輯) | logic | 只有在排盤正往後段沉降時為 TRUE | FALSE——沒有排盤在進行，後段卻被佔用（死結情況） | State Record・EventLog |
  | `bRearResidualAlarmed`(邏輯) | logic | 進入時為 FALSE；由本警報鎖為 TRUE（每次事件只發一次） | 發警時被設為 TRUE；只在後段感測器變空的瞬間或重置時重新解除 | State Record・EventLog |
- **處置**：手動把 Loader 後段出料底部的殘留盤取走，然後按 RETRY。取走盤後 `SnLoader_OutputBottomHasTray` 會轉 OFF，系統會在這個變空的瞬間清掉可取標記並重新武裝警報，之後才能對新的殘留事件再次發警。若盤已實體移除、感測器仍讀 ON，請在 IOsetview 檢修／重新校正該感測器。

### 6.2 Empty 空盤供給

#### `JAM1030` — Empty 推盤未確認
- **意義**：Empty 供料時推盤氣缸 `C_Empty_PushTray` 伸出後，其到位感測在整定時間內沒讀到 ON，判定夾盤未成功。
- **常見原因**：
  - 夾盤位置底下根本沒有盤，或行程不足、氣壓不足，推盤沒有頂實
  - 上游供料／分盤未送盤，夾爪關下去沒有盤可夾
  - 推盤到位感測沒對準、髒污或接線／氣路故障，伸出後始終讀不到 ON
  - `MEmptyY` 沒停在教導的供料 Y 位置，盤落在夾爪外側，推盤無法就位
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `C_Empty_PushTray` | cylinder | 推盤伸出並經整定後，到位感測讀到 ON（已夾到盤） | 已下令伸出，但整定時間過後到位感測仍為 OFF，隨即縮回並報警 | IOsetview / Note 備註列 |
  | `C_Empty_LeanOnTray` | cylinder | 靠位氣缸先動作並確認到位，之後才進行推盤 | 不是 JAM1030 的直接觸發裝置；若靠位一直沒確認，流程會停在靠位步驟而不報 JAM1030，屬另一個上游原因 | IOsetview |
  | `MEmptyY` | motor | 停在教導的 `EmptyCarFeedTrayYPosition`，盤剛好在推盤夾爪下方 | 位置偏掉時盤不在夾爪下方，推盤到位感測無法確認 | MotionView |
- **處置**：先確認 Empty 供料位確實有盤，清除任何卡料、確認盤有落到夾爪位置，然後按 RETRY 重新定位 Empty-Y 並重試靠位＋推盤夾盤。若反覆失敗，順手檢查推盤氣缸是否伸滿、到位感測有無對準／髒污、供氣壓力是否足夠，以及靠位氣缸是否先確認到位。

#### `MES1021` — 後端(底部)空盤缺失
- **意義**：Empty 送盤到後端出料底部後，底部到位感測 `SnEmpty_OutputBottomHasTray` 沒讀到盤，判定該位置沒有盤就位。
- **常見原因**：
  - 盤根本沒到後端底部位置，或在鬆夾時掉落／位移
  - `SnEmpty_OutputBottomHasTray` 沒對準、髒污或接線故障，實際有盤卻讀 OFF
  - `MEmptyY` 出料教導位置設錯或行程過／不足，盤停在感測範圍外
  - 上游進料源乾掉，根本沒送盤上來，流程卻仍走到此確認步驟
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnEmpty_OutputBottomHasTray` | sensor | 讀到 ON（盤已就位於 Empty 後端／出料底部）；這是本警報唯一實際讀取的裝置 | 讀到 OFF（底部位置偵測不到盤）；感測也必須為啟用狀態警報才會武裝 | IOsetview / Note 備註列 |
  | `MEmptyY` | motor | 停在教導的 `EmptyCarDischargeTrayYPosition`，盤正好在底部感測下方 | 移動已完成到達目標，但若教導值本身設錯，盤仍停在感測外——要核對教導位置，不能只看移動完成 | MotionView |
  | `C_Empty_PushTray` | cylinder | 已在前一步鬆開釋放 | 正常與故障情況下都已鬆開，不是本警報的辨識點；僅在鬆夾造成盤位移／掉落時才是機械根因 | IOsetview |
  | `C_Empty_LeanOnTray` | cylinder | 已在確認步驟前鬆開釋放 | 正常與故障都已鬆開，不是辨識點；僅懷疑鬆夾把盤帶離底部感測時才追查 | IOsetview |
- **處置**：確認 Empty 後端／出料底部確實有盤。按 RETRY 會從頭重跑整段送盤；按 SKIP 會放棄這一盤並回到待命。若反覆失敗，重點檢查底部感測的對準／清潔／接線（實際有盤卻讀 OFF 時），並在 MotionView 核對 `MEmptyY` 是否停到正確的教導出料位置。鬆夾氣缸只在懷疑機械鬆夾把盤帶偏／帶掉時才檢查。另注意機台若處於 DUMMY 模式會略過此確認，警報不會出現。

#### `MES1022` — Empty 供料倉料盡
- **意義**：Empty 進料源乾掉，進料端感測 `SnEmpty_InputEnd` 讀到 OFF，供料倉沒有盤可取。
- **常見原因**：
  - Empty 供料倉真的用完了，進料端沒有盤
  - AMR 模式下，AGV 未在 `iAmrFeedWaitSec`（AMR 補料等待秒數）內完成補料
  - `SnEmpty_InputEnd` 故障／偏位，實際有盤卻讀 OFF
  - 模擬模式下虛擬供料數 `iSimInfeedCount`（邏輯）歸零（虛擬料車空）
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnEmpty_InputEnd` | sensor | 讀到 ON（進料／供料倉有盤）；感測須為啟用狀態，停用時不會偵測缺料 | 讀到 OFF（進料源乾掉）→ 判定缺料 | IOsetview / Note 備註列 |
  | `bWaitingAmrFeed` | logic (邏輯) | AMR 模式：等待計時中，給 AGV 最多 `iAmrFeedWaitSec` 秒補料再報警 | 計時到期而料源仍空 → 報警；偵測到補料後解除 | State Record・EventLog |
  | `iSimInfeedCount` | logic (邏輯) | 模擬模式：大於 0（虛擬料倉有存量） | 模擬模式下歸零觸發缺料；RETRY 會自動補滿虛擬料車 | State Record・EventLog |
- **處置**：重新裝填 Empty 供料倉（或等 AGV 補料），然後按 RETRY，警報會在有料後自動解除；模擬模式下按 RETRY 會自動補滿虛擬料車。若料倉已裝滿但仍報缺料，檢查 `SnEmpty_InputEnd` 的間隙／對準／接線並確認感測為啟用狀態；AMR 模式下確認 AGV 確實有送到，並檢視 `iAmrFeedWaitSec` 設定。註：CleanOut 模式下不會出現此警報；唯一恢復鍵為 RETRY。

#### `MES1023` — Empty 供料堆疊已滿(感測)
- **意義**：批次結束 CleanOut 排空、要把盤送回供料堆疊時，堆疊滿盤感測 `SnEmpty_InputFullTray` 讀到 ON，堆疊已滿無法回盤。
- **常見原因**：
  - Empty 供料堆疊實際已滿，排空的盤無處可回
  - `SnEmpty_InputFullTray` 卡在 ON／偏位，實際堆疊仍有空間
  - CleanOut 排空過程中，操作員尚未把 Empty 供料倉裡堆疊的盤取走
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnEmpty_InputFullTray` | sensor | 讀到 OFF（堆疊有空間可讓排空的盤送回） | 讀到 ON（堆疊已滿），警報視窗會持續重複顯示直到清空 | IOsetview / Note 備註列 |
  | `bLotFinish` | logic (邏輯) | true——此排空回盤路徑要求批次已結束 | true（屬 CleanOut／批次結束排空情境） | State Record・EventLog |
  | `bFrontHasTray` / `bRearHasTray` | logic (邏輯) | 至少一個為 true——仍有盤在供料機上要排回堆疊 | 至少一個為 true（因此才嘗試排空回盤），而滿盤感測擋住回送 | State Record・EventLog |
  | `bAmrLocked` | logic (邏輯) | false——此排空回盤只在非 AMR 交接時執行 | 觸發本警報時為 false（AMR=0 操作員排空，或 AMR 閒置） | State Record・EventLog |
- **處置**：把 Empty 供料倉／堆疊上堆滿的盤取走。警報視窗會自動重複顯示，直到 `SnEmpty_InputFullTray` 變成 OFF 後排空回盤才會繼續。若堆疊已空或只有部分盤卻仍讀 ON，懷疑滿盤感測卡住或偏位（檢查其啟用狀態與備註列顯示的位址）。此警報只在批次結束／CleanOut 排空且非 AMR 鎖定時出現，模擬模式下不會出現。

#### `MES1024` — 前端空盤缺失
- **意義**：分盤下料完成後，前端有盤感測 `SnEmpty_InputHasTray` 沒讀到盤，判定前端單盤位置沒有盤。
- **常見原因**：
  - 分盤沒把盤降到前端位置（供料倉空，或升降／分離爪沒抓住並放下單盤）
  - 分離過程中盤掉落——第二段升降下降確認太慢／臨界，分離爪在堆疊被夾住前就打開
  - `SnEmpty_InputHasTray` 偏位／髒污／接線故障，前端實際有盤卻讀 OFF
  - 前端分離與 Loader 分盤氣缸 `C_Loader_FrontSeparateTray_1` 的互鎖時序錯亂，放出的盤數不對
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnEmpty_InputHasTray` | sensor | 分盤後讀到 ON（一片空盤就位於前端單盤位置）；這是報警時唯一讀取的裝置 | 讀到 OFF（前端位置偵測不到盤） | IOsetview / Note 備註列 |
  | `C_Empty_FrontRiseTray_1` | cylinder | 上游分盤動作，撐住堆疊後再鬆開，行程確認正常 | 若沒抓住／夾持或鬆開確認卡住，前端就沒有盤降下 | IOsetview |
  | `C_Empty_FrontRiseTray_2` | cylinder | 上游分盤動作，在分離前先下降確認以夾住堆疊 | 下降確認太慢／臨界時，可能在堆疊被夾住前就鬆開而掉盤，前端無盤 | IOsetview |
  | `C_Empty_FrontSeparateTray_1` | cylinder | 上游分盤動作，在 Loader 前端分離互鎖後動作，鬆開時剛好放下一片盤 | 分離時序錯亂時，前端無盤或放出的盤數不對 | IOsetview |
- **處置**：確認確實有一片空盤降到前端單盤位置、且進料倉不是空的，然後按 RETRY 重跑分盤。若反覆失敗，先在 IOsetview 檢查 `SnEmpty_InputHasTray` 的對準／清潔／接線（實際有盤時應讀 ON）；感測正常卻讀 OFF 時，代表上游分盤失敗——檢查三支分盤氣缸的行程與確認感測，並留意分離過程中堆疊掉落的情形。另注意機台若處於 DUMMY 模式會略過此實體確認。

### 6.3 Color 供給

#### `MES1421` — Color 供料盤未就緒

- **意義**：Color 供料流程把已夾持的料盤送到後側取料 Y 位置後，後側取料槽的到位感測器沒有讀到盤，代表這一輪供料實際上沒有把盤送到定位。
- **常見原因**：
  - Color 供料倉已空，實際上沒有盤被送到後側取料槽；
  - 盤在運送到後側取料位途中掉落或未到位（夾持中途鬆脫，或載台行程不足）；
  - `SnColor_OutputBottomHasTray` 感測器歪位、髒污或故障，看不到已送達的盤（若該感測器被關閉 Enable，則整個檢查被略過、不會報此警）；
  - AMR 模式下（`bUseAMR`）：在 `iAmrFeedWaitSec` 設定的等待秒數內，AGV 沒有補送/交付供料盤。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnColor_OutputBottomHasTray` | sensor | 讀到 ON（後側/輸出取料槽有盤） | 讀到 OFF（取料槽沒有盤） | IOsetview（感測器 LED）；此 IO 也會印在 Note 備註列 |
  | `MColorY` | motor | 已到達 `ColorTrayArmPickYPosition`（後側取料 Y），使盤停在感測器上方 | 行程不足會讓盤停在感測器前方（此警不直接讀馬達，僅為上游原因；真正超行程另報馬達警） | MotionView（軸位置 vs 教導位置） |
  | `C_Color_PushTray / C_Color_LeanOnTray` | cylinder | 運送途中夾住盤，到達後側前才退回 | 夾持途中鬆脫會把盤留在原處、送不到後側感測器（此警不直接讀汽缸，僅為上游原因） | IOsetview（汽缸開關 + 到位 LED） |

- **處置**：補滿 Color 供料倉。若有盤卡在運送途中，先清除。AMR 模式下確認 AGV 確實已把供料盤送到 Color 進料源。到 IOsetview 確認 `SnColor_OutputBottomHasTray` 在盤實際停在後側槽時會亮 ON、且其 Enable 為開。若 AMR 等待太短，可調高 system/General.ini 內的 `iAmrFeedWaitSec`。排除後按 RETRY，供料流程會從頭重跑。

#### `MES1422` — Color 推盤未確認（Push Tray Miss）

- **意義**：Color 供料的推盤汽缸 `C_Color_PushTray` 被下令伸出，但在夾持穩定時間內其到位確認感測器沒有讀到 ON，夾持動作判定失敗、推盤退回。
- **常見原因**：
  - `C_Color_PushTray` 被下令伸出，但到位確認感測器在 `iColorFeedClampSettleMs` 時間內沒讀到 ON；
  - 廠務氣壓不足或機構卡滯，推盤到不了確認的伸出位置；
  - `iColorFeedClampSettleMs` 設太短，確認訊號其實剛好在檢查之後才到；
  - 到位確認簧片/接線故障：汽缸實際已伸出，但確認感測器一直沒回報 ON；
  - 前端接料位盤未坐正，推盤到不了確認行程（完全沒有盤通常會先報 MES1424，而非 MES1422）。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `C_Color_PushTray`（其到位感測器） | cylinder | 讀到 ON（推盤完全伸出、到位感測器在穩定時間內確認） | 讀到 OFF（下令推出但到位感測器一直沒到，隨後退回） | Note 備註列（印出該 IO 名與期望 ON/實際狀態）；IOsetview 推盤到位 LED |

- **處置**：確認 Color 前端接料位確實有盤且坐正，清除任何卡料，檢查廠務氣壓，然後按 RETRY（供料會回到重新接近並重新夾持的步驟）。可在 IOsetview 手動點動 `C_Color_PushTray`，觀察其到位 LED 在完整行程時是否確實亮/滅；檢查氣壓與汽缸速度。若只是確認訊號稍慢，調高 system/General.ini [Color] 內的 `iColorFeedClampSettleMs`。同時確認前端確有盤（IOsetview 看 `SnColor_InputHasTray` LED）——完全沒盤通常會在更上游報 MES1424，所以 MES1422 多半代表盤在但推盤行程不足或到位感測器故障。

#### `MES1424` — Color 前端供料盤缺料

- **意義**：Color 前端分盤（destack）流程跑完後，前端輸入/供料緩衝區的到位感測器沒有讀到盤，代表分盤沒有把任何一盤放到前端緩衝區。
- **常見原因**：
  - Color 前端供料疊盤已空，分盤沒有分出任何盤到前端緩衝；
  - 某個前端分盤汽缸沒完成循環（雙頂升 `C_Color_FrontRiseTray_1`/`_2` 撐住疊盤，或分離爪 `C_Color_FrontSeparateTray_1` 分出單一盤），導致沒有盤被放下到前端緩衝；
  - `SnColor_InputHasTray` 卡在 OFF（歪位/髒污/故障）而其 Enable 為開，使已放下的盤被判為缺料（Enable 關閉不會造成此警，只會略過檢查）；
  - `iColorDestackSettleMs` 設太短，流程在分出的盤還沒坐穩前就讀感測器。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnColor_InputHasTray` | sensor | 讀到 ON（分盤後有一盤落在 Color 前端輸入/供料緩衝上） | 讀到 OFF（取樣時前端緩衝沒偵測到盤） | IOsetview（感測器 LED）；此 IO 名會印在 Note 備註列 |
  | `C_Color_FrontRiseTray_1` | cylinder | 先頂升撐住前端疊盤，之後下降把最底盤放到前端緩衝 | 沒完成頂升/下降循環，未把盤放到緩衝 | IOsetview（汽缸 LED） |
  | `C_Color_FrontSeparateTray_1` | cylinder | 分離爪切入分出單一盤，再退出放行剛好一盤 | 未能從疊盤分離/放出一盤 | IOsetview（汽缸 LED） |

- **處置**：補滿 Color 前端供料疊盤，清除前端分盤處任何錯位或卡住的盤，然後按 RETRY（分盤流程重跑）。可在 IOsetview 確認 `SnColor_InputHasTray` 在盤實際放在前端緩衝時會亮 ON（排除感測器卡死/髒污/歪位），並觀察前端分盤汽缸 `C_Color_FrontRiseTray_1`、`C_Color_FrontRiseTray_2` 與分離爪 `C_Color_FrontSeparateTray_1` 是否都有動作。若盤坐穩時間略晚於取樣，調高 system/General.ini 內的 `iColorDestackSettleMs`。此流程與 `MColorY`（Y 運送）無關，不必懷疑該軸。

#### `MES1426` — Color 後側殘留料盤

- **意義**：一輪 Color 供料開始前檢查發現後側交接槽被判定仍有盤（`bRearHasTray`（邏輯）為真），代表有盤殘留在該處，既不能被取走也不能重新上料，必須人工移除。
- **常見原因**：
  - 先前的故障、中止或斷電後，有一盤被留/卡在 Color 後側交接槽；
  - 後側到位感測器（`SnColor_OutputBottomHasTray` 或 `SnColor_TrayPos1`）卡在 ON 或誤讀，使 `bRearHasTray`（邏輯）在沒有實際盤時為真；
  - 接料/回收流程被中斷，`bRearHasTray`（邏輯）在感測器刷新前仍鎖在真。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `bRearHasTray`（邏輯） | logic | 為 false（新一輪供料開始前後側交接槽是空的） | 為 true（後側槽被標為佔用/殘留盤，擋住供料） | State Record・EventLog（此警不會在 Note 印出 IO 名） |
  | `SnColor_OutputBottomHasTray` | sensor | Enable 開時讀到 OFF（後側/輸出槽無盤） | ON（有殘留盤，或感測器卡在 ON）——會使後側判定為佔用 | IOsetview（IO LED） |
  | `SnColor_TrayPos1` | sensor | Enable 開時讀到 OFF（後側位置無盤） | ON（偵測到殘留盤）——並列的後側感測器，同樣使後側判定為佔用 | IOsetview（IO LED） |

- **處置**：把殘留盤從 Color 後側交接槽實際移除，後側到位感測器隨即清除該旗標。按 RETRY 會重新檢查；若已清空，供料繼續進行；若仍佔用，供料會落到結束終點而不出盤。若實際上沒有盤但警報仍在，到 IOsetview 檢查 `SnColor_OutputBottomHasTray` 與 `SnColor_TrayPos1` 是否卡在 ON（並確認各自 Enable 為開），並查 State Record・EventLog 是否有中斷的接料/回收流程把旗標鎖住。

#### `MES1427` — Color 供料疊盤已滿（感測器）

- **意義**：只在 Clean Out 排空階段觸發——Color 把剩餘料盤逐一送回車時，供料/回收疊盤已滿（`SnColor_InputFullTray` 讀到 ON），送回的盤無處可放。
- **常見原因**：
  - Clean Out 排空時，Color 把每個剩餘盤送回車，但供料/回收疊盤滿了（`SnColor_InputFullTray` ON），排出的盤沒地方去；
  - 操作員尚未把排空推回倉裡的疊盤取走；
  - `SnColor_InputFullTray` 卡住/誤讀為 ON（感測器故障），實際上疊盤並未滿。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnColor_InputFullTray` | sensor | 讀到 OFF（Color 供料/回收疊盤仍有空間） | ON（只要仍讀到 ON，就反覆重跳 MES1427） | IOsetview（LED，別名 `SnColor_InputFullTray`）；此 IO 也會印在 Note 備註列 |

- **處置**：把 Color 供料/回收倉裡疊放的盤取走。此警會反覆彈出（RETRY）直到 `SnColor_InputFullTray` 讀到 OFF；按 RETRY 會重新檢查感測器，一旦疊盤實體清空，迴圈退出、排空繼續。若疊盤已取空但警報仍在，到 IOsetview 確認 `SnColor_InputFullTray` 在移除盤時是否轉為 OFF、放滿時轉為 ON（排除卡死感測器把排空困在迴圈裡）。此檢查只在 Clean Out 排空階段生效，且卡在 ON 的感測器也會讓排空永遠無法完成，在 SOFT_SIMULATE 模擬下不會觸發，只能在真機重現。

### 6.4 Auto 出料站 1~6 (六站同模式)

#### `JAM1102~JAM1602` — Auto 推盤未到定位
- **適用**：Auto1~6 各站（序號 11~16）
- **意義**：Auto 站推盤氣缸推出後，在等待時間內沒有讀到「推到定位」的確認訊號，代表盤子沒有被推到最前端的定位。
- **常見原因**：
  - 盤子實體卡住或沒放正，推盤氣缸無法完全推到定位；
  - 推盤氣缸的到位確認 sensor 故障、脫線或位置偏掉，盤子已到定位卻讀不到；
  - 氣壓不足或電磁閥故障，氣缸推到一半就停住；
  - 該站推盤確認的等待時間 `iAutoPushConfirmSettleMs`（General.ini 設定）太短，氣缸還沒推到就先做確認。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `C_Auto1_PushTray`（代表；其餘站為 `C_Auto2..6_PushTray`） | cylinder | 推出並穩定後推到最前定位，到位 sensor 亮（有到位）→ 流程往下進 | 沒到定位：等待時間內到位 sensor 一直沒亮，隨後氣缸縮回才報警 | IOsetview（推盤電磁閥輸出＋氣缸到位確認 sensor 燈）；Note 備註列也會印出這顆到位 sensor 的 IO 名稱 |
  | `C_AutoN_PushTray` 的到位確認 sensor（其名稱會印在 Note 的 [IO=...]） | sensor | 氣缸完全推出、盤子就位時讀到 ON（到位） | 讀到 OFF（未到位）— 在等待時間內始終沒亮 | Note 備註列（印出此 sensor 的 IO 名稱與期望/實際）；再到 IOsetview 現場確認 |
- **處置**：打開該站，取出或重新擺正卡住／沒放正的盤子，然後按 RETRY 重新推盤。若氣缸有完全推出但到位 sensor 燈不亮，多半是 sensor／接線／對位問題；若氣缸推到一半就停，檢查氣壓與機構卡滯。重複發生就停機找維修人員。

#### `JAM1111~JAM1611` — Auto 後段已登記盤資料但後段無盤 sensor
- **適用**：Auto1~6 各站（序號 11~16）
- **意義**：軟體判定 TrayArm 已把盤交到這個 Auto 站的後段暫存位（有交盤鎖存），但要移動前檢查後段底部 sensor 卻讀不到盤。
- **常見原因**：
  - TrayArm 其實從未真正交盤，或交盤鎖存 `bRearDeliveredPending`（邏輯）設立後盤又滑掉／被取走；
  - 後段底部 `SnAutoN_OutputBottomHasTray` sensor 故障、被停用、脫線或對位偏掉，有盤卻讀成無盤；
  - 前次中斷或 HOME 打斷的循環留下殘存／假的交盤鎖存，讓 `bRearHasTray`（邏輯）在 sensor 已 OFF 時仍被鎖成有盤；
  - 盤子有放，但落在後段底部 sensor 的偵測範圍之外。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnAuto1_OutputBottomHasTray`（代表；各站為 `SnAuto2_..SnAuto6_OutputBottomHasTray`） | sensor | 讀到 ON（有盤）— 後段暫存位確實有盤，與軟體交盤鎖存相符 | 讀到 OFF（無盤，或 sensor 未接／被停用）— 軟體認為已交盤但 sensor 讀不到 | IOsetview（後段底部有盤 sensor 燈）；Note 備註列也會印出此 sensor 的 IO 名稱 |
  | `bRearDeliveredPending`（邏輯，交盤鎖存，護住 `bRearHasTray`） | logic | 只有 TrayArm 真正把盤交到此 Auto 後段（Z 上升確認完成）才會設立；用來防止後段有盤鎖存被 OFF sensor 誤清 | 無盤在場卻被設為成立 — 假／滯留的交盤鎖存讓 `bRearHasTray` 一直鎖成有盤，選料仍會選到本站 | State Record・EventLog（後段交盤鎖存與 `bRearHasTray` 狀態） |
- **處置**：先清掉 Auto 後段任何滯留的盤。若確實有盤、sensor 應該要看到它，按 RETRY 重新讀 sensor（有盤讀不到多半是 sensor／接線／對位故障或被停用，需維修處理）。若後段確實沒有盤，按 SKIP 清掉這筆後段登記資料，TrayArm 之後會依需求重新補盤。

#### `MES1120~MES1620` — Auto 出料堆疊實體已滿（sensor）
- **適用**：Auto1~6 各站（序號 11~16）
- **意義**：Auto 站的滿料 sensor `InputFullTray` 讀到 ON，代表該站出料堆疊實體已滿，無法再收盤。此警報只在真機出現。
- **常見原因**：
  - 出料堆疊已裝滿完成盤 — 在 AMR=0／人工顧機的機台上是正常的批次結束狀態，操作員（或 AGV）尚未取走盤子；
  - 完成盤未清空，導致清機上升或出料／退料循環無法進到已滿的堆疊；
  - `SnAutoN_InputFullTray` 滿料 sensor 卡住／誤報 ON（被雜物擋住、對位偏掉或接線異常），實際並未滿 — 此時警報視窗會因為一直讀到 ON 而無法自動關閉。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnAuto1_InputFullTray`（代表；各站為 `SnAuto2_..SnAuto6_InputFullTray`） | sensor | 讀到 OFF（未滿，堆疊還有空間） | 讀到 ON（偵測到滿）— 警報視窗每輪重讀此 sensor，直到讀到 OFF 才會關閉 | IOsetview（此 sensor 的 IO 燈）；Note 備註列也會直接印出此 sensor 名稱、期望 OFF／實際 ON 與其位址 |
- **處置**：把該 Auto 站出料堆疊上的完成盤取走。警報視窗（RETRY）會在每一輪重讀滿料 sensor，一旦 sensor 變 OFF 就自動關閉，系統隨即清空該車計數並重新初始化堆疊 — 不需要手動輸入數量，全由 sensor 決定。若堆疊看起來明明是空的但視窗一直關不掉，到 IOsetview 檢查該站 `SnAutoN_InputFullTray` 是否卡在 ON（光學被擋、滿料撥片偏位或接線短路），sensor 一直讀 ON 視窗就無法自行關閉。

#### `MES1123~MES1623` — Auto 清機排空後仍有殘留盤
- **適用**：Auto1~6 各站（序號 11~16）
- **意義**：在 Clean Out 清機模式下、且所有站的排空鎖存都已完成時，看門檢查發現某 Auto 站的三個 sensor（前段進料、滿料堆疊、後段底部）之中仍有一個讀到有盤，代表排空已判定完成但實體還留著盤。此為診斷紀錄，不會停機、沒有恢復按鈕，機台繼續運轉。只在真機出現。
- **常見原因**：
  - 清機排空完成時，某 Auto 站（前段進料位、滿料／出料堆疊、或後段／出料底部暫存）仍卡著一盤沒清掉；
  - 該站三個 sensor 之一卡在 ON（被擋住／對位偏掉／接線異常），站其實是空的卻被記成有殘留盤；
  - 排空完成是純邏輯鎖存 `bCleanOutFinish`（邏輯，不看 sensor），在盤子還沒實際離開前軟體旗標就先被清掉，導致鎖存提前判完成。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnAuto1_InputHasTray`（各站 `SnAuto2..6_InputHasTray`）— 前段進料位有盤 sensor | sensor | 排空後讀到 OFF（前段進料位無殘留盤） | 讀到 ON — 紀錄的 Where 欄位顯示 front=1 | State Record・EventLog（該筆 MES 紀錄的 Where 欄 front=/full=/rear=）；再到 IOsetview 現場確認 |
  | `SnAuto1_InputFullTray`（各站 `SnAuto2..6_InputFullTray`）— 滿料堆疊有盤 sensor | sensor | 排空後讀到 OFF（堆疊已空） | 讀到 ON — Where 欄顯示 full=1 | State Record・EventLog（Where full= 旗標）；再到 IOsetview 確認 |
  | `SnAuto1_OutputBottomHasTray`（各站 `SnAuto2..6_OutputBottomHasTray`）— 後段／出料底部暫存 sensor | sensor | 排空後讀到 OFF（後段暫存無殘留盤） | 讀到 ON — Where 欄顯示 rear=1 | State Record・EventLog（Where rear= 旗標）；再到 IOsetview 確認 |
  | `bCleanOutFinish`（邏輯，排空完成鎖存）與每次只記一行的記錄鎖存 | logic-state | 該站真正排空（無實體盤）時才鎖存完成 | 排空已鎖存完成，卻仍有實體 sensor 讀到 ON — 兩者不一致就產生此紀錄 | State Record・EventLog（排空完成鎖存與記錄旗標會印在 Auto 狀態追蹤中） |
- **處置**：這是診斷紀錄，不是彈窗 — 機台不會停、也沒有恢復按鈕。清機結束後，到 D:\HT160S_Log 的 EventLog 找到該站的 MES1123..MES1623 那一行，讀它的 Where 欄（front=／full=／rear=）判斷是哪個位置有盤，再到現場檢查並取走殘留盤：front＝前段進料位、full＝滿料／出料堆疊、rear＝後段／出料底部暫存。若該位置實體是空的、sensor 卻仍讀 ON，代表該 sensor 被擋住／對位偏掉／接線異常，需修 sensor（也可能是排空提前鎖存完成所致）。

#### `MES1125~MES1625` — Auto 出料車依盤數計數判定已滿
- **適用**：Auto1~6 各站（序號 11~16）
- **意義**：在 Normal 正常生產模式下，此 Auto 站出料車的軟體盤數計數 `iTrayCount`（邏輯）已達每車上限（100 盤），而且實體滿料 sensor 沒有觸發（OFF／停用／未接），因此改由盤數計數判定滿車。此為純計數警報，不對應任何 IO。
- **常見原因**：
  - 正常換車時機：自上次清空以來已餵入 100 盤到此 Auto 出料車 — 在未啟用實體滿料 sensor 的機台上，這是標準換車觸發；
  - 實體 `SnAuto{N}_InputFullTray` sensor 讀 OFF／被停用／未設定，所以實體滿料 sensor 路徑（`MES1120~MES1620`）沒觸發，改由此盤數計數接手；
  - 操作員先前換／清實體車時沒有在彈窗上按確認，計數沒被歸零而持續累加到 100 — 因此即使實體車沒真的滿也可能報警（計數不會因出料或清機而自動歸零）。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `iTrayCount`（邏輯，軟體盤數計數）對每車上限（100 盤） | logic | 計數 < 100（帳面上還有容量） | 計數 ≥ 100（帳面判滿）。此計數只在餵料時累加、只在車體清空時歸零；出料或清機都不會改動它 | State Record・EventLog（車體盤數計數；此警報不帶 IO，故無 sensor 燈、無 Note 備註列） |
  | `SnAuto{N}_InputFullTray`（N=站號；`SnAuto1_..SnAuto6_InputFullTray`） | sensor | 要走到本警報而非 `MES1120~MES1620`，此 sensor 必須讀 OFF／停用／未設定；若它是 ON，會改觸發姊妹警報 `MES1120~MES1620` | OFF／停用／未接（確認這是計數判滿、不是實體 sensor 判滿）。它在此只作為兩條分支的判別依據，不是本警報的觸發裝置 | IOsetview（IO 燈） |
- **處置**：換／清空出料車，然後在彈窗上按 RETRY／確認。按確認後系統會清空該車資料、重新初始化堆疊並解除該站的保留狀態，重新開放餵料。注意：若你換了實體車卻沒在此彈窗上按確認，計數不會歸零，警報會一直回來。若 `SnAuto{N}_InputFullTray` 有啟用且接好，機台正常應先觸發 `MES1120~MES1620`；若它未啟用，此盤數上限就是預期的滿車偵測方式。

#### `WAR1130~WAR1630` — Auto 餵盤未到位
- **適用**：Auto1~6 各站（序號 11~16）
- **意義**：Auto 站完成餵料 Y 軸移動後，後段底部有盤 sensor 沒讀到盤，代表盤子沒有正確落在餵料位上。屬警告級（WAR），只能重試。
- **常見原因**：
  - 盤子在餵料 Y 移動過程中／之後位移、傾斜或掉落，離開了後段底部 sensor 的偵測範圍；
  - 後段底部有盤 sensor（`SnAutoN_OutputBottomHasTray`）故障或對位偏掉 — 有盤卻讀成 OFF；
  - `MAutoY_N` 餵料軸沒有真正到達教導的餵料 Y 位置，盤子沒對準到 sensor 下方；
  - 此站的餵料 Y 教導值錯誤或已飄移。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnAuto1_OutputBottomHasTray`（代表；`SnAuto2..6_OutputBottomHasTray`） | sensor | 已啟用且餵料 Y 移動後讀到 ON／到位 — 後段底部餵料位有盤 | 已啟用但讀到 OFF — Y 移動後餵料位沒偵測到盤（若 sensor 被停用或在模擬模式，此警報不會觸發，故它觸發就代表 sensor 已啟用且讀 OFF） | IOsetview（後段底部有盤 sensor 燈）與 Note 備註列（印出 [IO=<sensor 名稱>] 及期望 ON／實際的對比） |
  | `MAutoY_1`（代表；`MAutoY_2..6`）— Auto Y 餵料軸 | motor | 到達餵料 Y 教導目標，盤子落在後段底部 sensor 下方 | 可能沒到教導的餵料 Y 目標或偏掉，使盤子落在 sensor 範圍外（觸發判斷只讀 sensor、不讀馬達，馬達是上游根因，需另行查證） | MotionView（`MAutoY_N` 命令位置對實際位置／軟體極限狀態） |
- **處置**：檢查 Auto 後段餵料位上的盤，若位移或掉落就重新擺正，再按 RETRY 重新執行 Y 移動並重讀 sensor。若重試仍失敗：先在 MotionView 確認 `MAutoY_N` 有到達餵料 Y 目標，偏掉就重新教導餵料 Y 位置；再到 IOsetview 用一盤確定放正的盤驗證 `SnAutoN_OutputBottomHasTray`，有盤讀不到就是 sensor／接線／對位故障。

### 6.5 TrayArm 送盤手臂

#### `MES1721` — TrayArm 取盤受阻
- **意義**：送盤手臂被派去後方料源取盤，但料源在整個等待窗（60 秒）內始終沒準備好可取的盤，手臂只能停在 Z 上位空等，逾時後停機發此警報。
- **常見原因**：
  - 料源=Empty：後方一直沒有空盤送到，或搬運還在進行中（`SnEmpty_OutputBottomHasTray` 讀不到盤、`bRearHasTray`(邏輯) 為否，或供料流程還卡在中途交接）。
  - 料源=Empty：搬運夾爪 `C_Empty_PushTray` / `C_Empty_LeanOnTray` 在取盤當下仍在夾持狀態，盤還被搬運端佔住還沒放到後方。
  - 料源=Empty：Empty 站正在供料中或回收中，某個流程正佔用後方。
  - 料源=Loader：卸盤動作一直沒完成（`bRearReadyForPick`(邏輯) 這個放行閂始終沒亮起），或 `SnLoader_OutputBottomHasTray` 讀不到盤、後方判定為未佔用。
  - 後方殘留或卡住一片盤，使新盤無法乾淨呈現；或料源模組整個停滯，60 秒內始終無法達到可取狀態。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnEmpty_OutputBottomHasTray` | sensor | 料源=Empty 時讀到 ON（有空盤穩定停在 Empty 後方、等待交接） | 讀到 OFF（後方還沒送到盤，取盤放行條件不成立） | IOsetview（Note 括號會標示料源「(Empty)」） |
  | `C_Empty_PushTray` | cylinder | 取盤當下輸出關（縮回）——搬運端已把盤釋放到後方 | 輸出仍開（仍在夾持）——搬運夾爪還抓著盤，後方取盤條件不成立 | IOsetview |
  | `C_Empty_LeanOnTray` | cylinder | 取盤當下輸出關（縮回）——搬運端已放手 | 輸出仍開（仍在夾持）——盤仍被搬運端佔住，放行閘卡住 | IOsetview |
  | `SnLoader_OutputBottomHasTray` | sensor | 料源=Loader 時讀到 ON（卸出的空盤已穩定停在 Loader 後方） | 讀到 OFF（Loader 後方未佔用，取盤放行條件不成立） | IOsetview（Note 括號會標示料源「(Loader)」） |
  | `bRearReadyForPick`(邏輯)（Loader 放行閂） | logic | 為真——只有 Loader 卸盤流程完整走完（車體退回進料 Y、Push/Lean 夾爪確認）才會亮起 | 為否——卸盤流程沒走完或被中途打斷（例如中途 HOME），或後方變空又把它重置，手臂始終拿不到放行 | State Record・EventLog |
  | `PickWaitTimer / bPickWaitArmed`(邏輯) | logic | 已清除——放行通過、取盤在 60 秒窗內順利前進 | 已武裝且逾時仍受阻——這是本警報的直接觸發點 | State Record・EventLog（快照 TrayArmPickBlocked_<料源>；FeederDecision.txt 記錄每個閘門輸入） |
- **處置**：Note 提供 RETRY。先看 Note 括號標示的料源（Empty 或 Loader），到該後方確認實體確實有一片空盤且完全就定位。若搬運還在送盤途中，稍候再按 RETRY；若後方卡住殘留盤，先清除卡料再按 RETRY。也可在 IOsetview 觀察 `SnEmpty_OutputBottomHasTray`（或 `SnLoader_OutputBottomHasTray`）在盤到位時是否轉 ON，並確認 `C_Empty_PushTray` / `C_Empty_LeanOnTray` 在搬運端放手後輸出是否降到關。

#### `MES1722` — TrayArm 夾著一片來歷不明的盤
- **意義**：送盤手臂被派去空手取盤（照理夾爪應是空的），但夾爪的閉合到位感測讀到「已夾著盤」，若繼續下夾會疊盤，因此停機發此警報。此警報只在真機（非 DUMMY、非模擬）模式下才會啟動。
- **常見原因**：
  - 手臂帶著一片盤走到取盤點，但 `bHasTray` / `fHasTray`(邏輯) 這個攜帶閂從沒記錄到——可能是搬運中途被中斷、中途斷電，或是「單邊到位」的 HOME（HOME 只要任一側夾爪到位就保持閉合，但殘料認養需要兩側都到位，所以只有一側到位的手臂不會被認養，卻仍被派去空手取盤而夾著閉合）。
  - 整機 HOME 時夾爪裡就有一片盤：HOME 時若兩側夾爪到位感測都讀到 ON，手臂會被認養為殘料（`bHasTray`(邏輯) 設為真、`bResiduePendingNotify`(邏輯) 設為真）並一次性提示。
  - 某側夾爪到位感測（`C_TrayArm_FrontClamp` / `C_TrayArm_RearClamp` 的到位感測）卡在 ON 或調整不良，假造出「夾著盤」的假象，在實體無盤下反覆觸發此警報。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `C_TrayArm_FrontClamp` 到位感測 | sensor | 手臂真的空的時候（取盤前或 HOME 時，夾爪張開、沒夾盤）讀到 OFF | 讀到 ON（夾爪閉合感測認為夾著盤）。取盤閘門：任一側到位感測 ON 就會觸發；HOME 殘料認養：兩側都要 ON | IOsetview（TrayArm 前夾爪閉合到位感測） |
  | `C_TrayArm_RearClamp` 到位感測 | sensor | 手臂真的空的時候讀到 OFF（夾爪張開） | 讀到 ON（夾爪閉合感測認為夾著盤）。與前夾爪相同：任一側 ON 觸發取盤閘門，HOME 認養需兩側都 ON | IOsetview（TrayArm 後夾爪閉合到位感測） |
  | `bHasTray / fHasTray`(邏輯)（TrayArm 攜帶閂） | logic | 與感測一致——只有夾爪到位感測都讀 OFF（手臂真的空）時才為否 | 為否（系統認為沒帶盤）但某側夾爪到位感測卻讀 ON——這就是不同步的癥結 | State Record・EventLog（TrayArm 狀態傾印） |
  | `bResiduePendingNotify`(邏輯) | logic | 為否（沒有待提示的認養殘料） | 為真——HOME 時把兩側都到位的殘料認養了並排入一次性提示 | State Record・EventLog |
- **處置**：在 Teach 打開 TrayArm 夾爪，實體把手臂夾著的那片盤取下，再按 RETRY。不需要重開程式：只要夾爪到位感測都不再讀 ON，系統會自動解除認養，清掉 `bHasTray` / `fHasTray` / `bResiduePendingNotify`，生產即可恢復。若在實體無盤下警報仍反覆出現，到 IOsetview 確認 `C_TrayArm_FrontClamp` 與 `C_TrayArm_RearClamp` 的閉合到位感測是否調整正確、有沒有卡在 ON——調整不良的感測會在 HOME 時誤認幻影殘料、並在取盤時反覆誤觸此警報。

#### `MES1723` — TrayArm 放盤受阻
- **意義**：送盤手臂帶著盤要放到目的站，但目的站後方在整個等待窗（60 秒）內始終沒讓出位置，手臂只能等，逾時後停機發此警報。目的地會標在 Note 括號內（Auto / Empty / Color）。
- **常見原因**：
  - 目的地=Auto：目標 Auto 站後方實體還壓著一片盤（`SnAuto<n>_OutputBottomHasTray` 為 ON，使該站後方判定為佔用）。
  - 目的地=Auto：或雖然感測 OFF，但「已送達未消耗」的閂 `bRearDeliveredPending`(邏輯) 仍把後方鎖為佔用。
  - 目的地=Empty/Color：接收站後方沒清空——`SnEmpty_OutputBottomHasTray`（Empty），或 `SnColor_OutputBottomHasTray` / `SnColor_TrayPos1`（Color）仍為 ON，使清後方的等待一直不通過。
  - 目的地=Empty/Color：接收站卡在供料中（Empty 供料中 / Color 供料中）持續整個 60 秒窗，觸發防撞閘，與後方感測狀態無關。
  - 帶盤途中發生整機 HOME，把接收站的「回盤」交握清掉了；恢復時只有在「所攜帶盤尚未放下」的情況才會重新簽署回盤請求，若這步沒跑到，接收站就保留或重新填滿了後方而手臂還在等。
  - 目的地後方感測卡住或調整不良、在實體無盤下維持 ON，或接收站抬升清後方的動作在 60 秒內始終沒完成。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `SnAuto<n>_OutputBottomHasTray`（n 為目標站，Auto1..Auto6） | sensor | 讀到 OFF——目標 Auto 站後方空著、可接收放下的盤。目的站是動態的，同一閘門涵蓋 6 個 Auto 站，請看對應的那一個 | 讀到 ON——Auto 後方仍壓著盤，該站後方判定為佔用 | IOsetview（Note 括號印「(Auto)」；目標站序號要看快照裡 TrayArm 狀態傾印，不在 Note 上） |
  | `SnEmpty_OutputBottomHasTray` | sensor | 目的地=Empty 放盤時讀到 OFF（Empty 已抬升、讓出後方） | 讀到 ON——Empty 後方仍被佔用，清後方的等待始終不通過 | IOsetview（Note 括號印「(Empty)」） |
  | `SnColor_OutputBottomHasTray`（與 `SnColor_TrayPos1`） | sensor | 目的地=Color 放盤時兩者都讀 OFF（Color 已抬升、讓出後方）。Color 後方只要任一感測 ON 就算佔用，故兩者須都 OFF | 其中一個讀到 ON——Color 後方仍被佔用，清後方的等待始終不通過 | IOsetview（Note 括號印「(Color)」） |
  | 接收站狀態（Empty 供料中 / Color 供料中） | logic | 放盤時非供料中——接收站的送盤動作已完成，防撞閘放行 | 卡在供料中（Empty / Color）整個 60 秒窗——這是真機才有的阻擋來源，即使後方感測 OFF 也會觸發本警報 | State Record・EventLog（Empty/Color 狀態傾印的 Status 欄） |
  | `bRearDeliveredPending`(邏輯)（Auto 閂）/ `bReturnTray`(邏輯)（Empty/Color 接收站交握） | logic | Auto 後方簽為空（無已送達未消耗閂），且接收站的回盤交握存在，故會抬升清後方 | Auto 後方被 `bRearDeliveredPending` 鎖為佔用；或帶盤途中 HOME 後 `bReturnTray` 被清掉、恢復時該癒合步驟被跳過，接收站保留或重新填滿後方 | State Record・EventLog（Auto 傾印 RearHasTray/DeliveredPending；Empty/Color 傾印 bReturnTray/bRearHasTray） |
  | `PlaceWaitTimer / bPlaceWaitArmed`(邏輯) | logic | 已清除——後方讓出（或供料完成）、放盤前進，等待窗關閉 | 已武裝且逾時、目的地仍受阻——這是本警報的確切觸發條件 | State Record・EventLog（快照 TrayArmPlaceBlocked_<目的地>） |
- **處置**：Note 提供 RETRY。先看 Note 括號標示的目的地（Auto / Empty / Color）。該接收站後方還壓著一片盤（或接收站正在供料中）。清除／移走擋住的那片盤讓後方空出來，再按 RETRY。也可在 IOsetview 確認目的地後方感測在後方空著時讀 OFF：Auto 看目標站的 `SnAuto<n>_OutputBottomHasTray`（站序號查快照 TrayArmPlaceBlocked_Auto 內的 TrayArm 傾印），Empty 看 `SnEmpty_OutputBottomHasTray`，Color 要同時看 `SnColor_OutputBottomHasTray` 與 `SnColor_TrayPos1`。若感測在實體無盤下仍維持 ON，即為卡住／調整不良。若後方已讀空但 Empty/Color 仍觸發，多半是接收站卡在供料中而觸發防撞閘；若阻擋發生在帶盤途中整機 HOME 之後，請確認接收站確實有抬升清出後方。

### 6.6 CCD / 視覺

#### `WAR0330` — Top CCD Bin 分類讀取尚未就緒

- **意義**：Loader 站要做 Top CCD 的 Bin 分類讀取，但在真機生產模式下這個讀取一律回報失敗，於是跳出此警示。目前 Top CCD 的「Bin 分類讀取」在 HT160S 真機上尚未接通（只有另一條「2D 條碼讀取」是實作好的），所以只要開啟 `tFunction.UseCCD`（設定：使用 CCD Bin 分類讀取）就會固定失敗。
- **常見原因**：
  - `tFunction.UseCCD` 被開成 ON，但機台實際上沒有可用的 Top CCD Bin 分類讀取功能；
  - 機台跑的是真機生產模式（非 SOFT_SIMULATE、也不是模擬跑料），此時 Bin 分類讀取一律回報「讀不到」。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `tFunction.UseCCD` | 設定(邏輯) | 沒有可用的 Bin 分類 CCD 讀取時應設 OFF（此時盤格直接當成合格 IC，不會跳警示） | ON，於是真機生產模式下 Bin 讀取固定失敗並跳 WAR0330 | State Record・EventLog |
  | `tSimuData.bRunSimulation` | 邏輯 | 模擬跑料時為 ON，可讓 Bin 讀取視為成功而抑制此警示 | OFF（正式生產），走真機失敗分支 | State Record・EventLog |
  | SOFT_SIMULATE 開發／筆電模式 | 邏輯 | 開發／筆電模式成立時可抑制此警示 | 真機建置不成立，配合 UseCCD ON 就會固定跳警示 | State Record・EventLog |
- **處置**：本項是「功能尚未就緒」的邏輯警示，機台上沒有對應的感測器或馬達 LED 可查，不需要去找 IO。若機台沒有安裝 Bin 分類 CCD 讀取功能，請維修人員把設定 `tFunction.UseCCD` 關成 OFF，盤格就會被當成合格 IC、警示不再出現。若真的需要 Bin 分類讀取，需先由韌體把該功能接通後才能啟用。畫面 Note 可按 RETRY（重新移到該格再讀，真機上仍會再失敗）、SKIP（把該格標記為空 IC 後繼續）、或 TRAY END（把目前盤剩餘未檢的格子全部清為空後結束該盤）。

#### `WAR0462` — Top CCD 2D 無回應（2DID 通訊逾時）

- **意義**：Loader 站對 Top CCD 讀碼器發出拍照觸發後，在 3000 毫秒的等待視窗內始終沒有收到解碼完成的 2D 字串，逾時而跳出此警示。
- **常見原因**：
  - Top CCD 視覺 PC 有收到拍照觸發，但沒在 3000 毫秒內回傳解碼後的 2D 字串；
  - 觸發後 Top CCD 讀碼器的網路／連線斷掉或卡住，一直收不到回覆；
  - IC 上的 2D 條碼在教導的讀取位置讀不到（模糊、旋轉、缺件、打光不良）。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | Top CCD 2D 讀碼連線 | 邏輯 | 在 3000 毫秒視窗內回傳一組解碼成功的 2D 字碼 | 沒有回覆進來，一直讀不到，於是落入逾時分支 | State Record・EventLog（另可看維護頁 Top CCD 連線指示、`CosFunction.bUseTopCcd` 旗標） |
  | Top CCD 3000 毫秒等待計時 | 邏輯 | 在計時到期前被一次成功讀碼清除／重置 | 計時到期而仍未讀到 2D，觸發警示 | State Record・EventLog |
- **處置**：先確認 IC 是否確實停在教導的讀取位置、2D 條碼是否有出現且清晰（檢查對焦與打光、條碼有無缺件或歪斜）。確認 Top CCD 視覺 PC 有開機、程式在跑並能正常解碼，網路線／交換器接妥。若同時出現 `WAR16120`（TopCCD_Connect），請先把連線／線材／電源當成主因處理。畫面 Note 可按 RETRY（重新拍照並重新等待 3 秒）、MANUAL 2D（由操作員手動輸入該 IC 的 2D 碼，再走正常 Bin/Lot 綁定）、或 SKIP（把該 IC 送到 Error bin 後繼續，此路徑不會累計任何計數）。

#### `WAR0475` — 2D 碼在所有 Lot 中都查不到

- **意義**：Top CCD 已成功讀到 IC 的 2D 碼，但拿這個碼去反查所有已載入的 Lot 都找不到對應資料，於是跳出此警示。
- **常見原因**：
  - 掃到的 2D 碼不在任何已載入 Lot 的 2D-Bin 對照表內（載入了錯的或不完整的 WorkOrder/Lot）；
  - 開始生產前沒有把正確的 Lot 資料下載／載入；
  - Top CCD 誤讀成一個不存在於任何已註冊 Lot 的值。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | 已載入 Lot 的 2D→Bin 反查表 | 邏輯 | 含有掃到的 2D 碼，可查出所屬 Lot 與 Bin | 查不到，該碼不在任何已載入的 Lot 內 | State Record・EventLog |
  | 掃到的 2D 字碼 | 邏輯 | 對應到某個已載入 Lot 中已註冊的碼 | 作為查不到的值附在 Note 訊息上；手動輸入時同理 | Note 備註列 |
- **處置**：先比對 Note 備註列印出的 2D 碼與目前載入的 Lot 是否相符。若 Lot 是對的、只有這一顆碼查不到，多半是 Top CCD 誤讀（檢查讀取位置、對焦與打光）。若很多顆都查不到，通常是開始生產前載入了錯的或不完整的 Lot／WorkOrder，請重新載入正確資料。畫面 Note 可按 RETRY（重新拍照重讀）、MANUAL 2D（手動輸入該碼，仍走同一套反查）、或 SKIP（把該 IC 送到 Error Auto 並累計未知 2D 計數）。

#### `WAR0970` — Color CCD 2D 無回應（Tray ID 通訊逾時）

- **意義**：Color 站對 Color CCD 讀碼器發出拍照觸發後，在 3000 毫秒視窗內始終沒有收到解碼後的身分盤 Tray-ID 2D 字串，逾時而跳出此警示（連線在拍照前已確認正常，屬於「已觸發但逾時未解碼」）。
- **常見原因**：
  - Color CCD 視覺 PC 有收到觸發，但沒在 3000 毫秒內回傳解碼後的 Tray-ID 2D；
  - 觸發之後 Color 讀碼器連線才斷掉，導致一直收不到結果直到逾時；
  - 身分盤的 2D 條碼在教導的 `ColorRead2DXPosition` 讀不到（缺件／模糊／歪斜／打光不良）。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | Color CCD 讀碼連線 | 邏輯 | 在 3000 毫秒視窗內回傳一組非空的 Tray-ID 字串 | 一直讀不到，逾時後結束拍照並跳警示 | State Record・EventLog |
  | Color CCD 3000 毫秒拍照等待計時 | 邏輯 | 在計時到期前先取得成功結果 | 計時到期仍無結果，正是觸發 WAR0970 的條件 | State Record・EventLog |
  | `MTopCCDX_Color` | 馬達 | 已移動並停在教導的 `ColorRead2DXPosition`，讓讀碼器正對身分盤條碼 | 回報已到位，但仍讀不到碼（請確認確實到達教導位置、且條碼在讀碼器下方） | MotionView |
- **處置**：先確認 `MTopCCDX_Color` 在 MotionView 有確實到達教導的 `ColorRead2DXPosition`、讀碼器正對身分盤 2D，再檢查對焦／打光與條碼有無出現且清晰。確認 Color CCD 視覺 PC 已開機、讀碼程式在解碼；若機台沒有安裝 Color 讀碼器，可把設定 `CosFunction.bUseColorCcd`（[ColorCCD] Enable）關掉，改用模擬身分而不跳警示。畫面 Note 可按 RETRY（重新觸發並重新計時 3 秒）、MANUAL 2D（採用操作員手動輸入的身分並蓋到身分盤上）、或 SKIP（設定空身分後回到待機）。

#### `WAR16120` — Top CCD 連線尚未就緒（Loader Tray ID 無回應）

- **意義**：在 Top CCD 2D-Bin 對照路徑下、且是真機（REALLY）並啟用 Top CCD 的情況下，發現與 Top CCD 視覺 PC 的連線不在「已連線」狀態，於是跳出此警示。
- **常見原因**：
  - 到 Top CCD 視覺 PC 的網路連線不在已連線狀態（網路線沒插、視覺 PC 沒開、IP/Port 錯、程式沒在監聽）；
  - `CosFunction.bUseTopCcd`（[TopCCD] Enable）在真機上開著，但讀碼器連不上；
  - 初始化後到本次掃描之間連線斷掉，自動重連（約每 2 秒節流一次）還沒接回。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | Top CCD 連線 | 邏輯 | 處於「已連線」狀態 | 未處於已連線狀態，即為本項的實際故障 | State Record・EventLog |
  | `CosFunction.bUseTopCcd` | 設定(邏輯) | 對應 [TopCCD] Enable；僅在有 Top CCD 讀碼器且連得上時才開 ON | ON，但連線是斷的 | State Record・EventLog |
  | `CosFunction.bUse2DBinMap` | 設定(邏輯) | 開 ON 才會走 Top CCD 2D 對照路徑；本警示只有在此為 ON 時才會到達 | ON（已進入該路徑） | State Record・EventLog |
  | `HSys.LastSet.iRealDummy` | 邏輯 | 只有在真機且讀碼器已接線時為 REALLY | REALLY，但沒有實際連上 Top CCD | State Record・EventLog |
- **處置**：確認 Top CCD 視覺 PC 已開機、程式在監聽，且機台設定的 [TopCCD] Address/Port 與讀碼器相符，並檢查網路線／交換器。可在維護頁用 Top CCD 連線鈕強制重連，確認連線回到「已連線」狀態。若機台沒有安裝 Top CCD，請把 [TopCCD] Enable 關掉（讓 `CosFunction.bUseTopCcd` 為 false）以略過 2D 路徑；或在整個 2D-Bin 對照功能不使用時，把 `bUse2DBinMap` 關掉。畫面 Note 可按 RETRY（重跑本步，內含隱式重連）或 SKIP（把該格 Bin 標記為 2D 掃描失敗後回到待機）。

#### `WAR16121` — Color CCD 連線尚未就緒（Color Tray ID 無回應）

- **意義**：在真機且啟用 Color 讀碼器的情況下，Color 站要讀身分盤 2D 前發現與 Color CCD 視覺 PC 的連線不存在或不在「已連線」狀態，於是跳出此警示。
- **常見原因**：
  - 到 Color CCD 視覺 PC 的連線沒建立（連線物件為空，或狀態不是已連線）；
  - `bUseColorCcd`（[ColorCCD] Enable）開著但讀碼器連不上（視覺 PC 沒開、Address/Port 錯、線材／交換器故障）；
  - Color 讀碼器程式沒啟動或沒在設定的埠監聽。
- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | Color CCD 連線 | 邏輯 | 存在且處於「已連線」狀態 | 不存在，或不在已連線狀態 | State Record・EventLog |
  | `CosFunction.bUseColorCcd` | 設定(邏輯) | 對應 [ColorCCD] Enable（預設開啟）；僅在實體有 Color 讀碼器且連得上時才該開 ON（關 OFF 時會改用模擬身分、不會到本警示） | ON，但連線是斷的，於是走到連線檢查而跳警示 | State Record・EventLog |
- **處置**：這是連線故障、不是動作故障。確認 Color CCD 視覺 PC 已開機、讀碼程式在監聽，且 system\General.ini 內 [ColorCCD] 的 Address 與 Port 與讀碼器相符，並檢查控制器與視覺 PC 之間的線材／交換器。若機台沒有安裝 Color 讀碼器，把 [ColorCCD] Enable 設為 false（連動 `CosFunction.bUseColorCcd`），身分 2D 路徑就改用模擬而不跳警示。畫面 Note 可按 RETRY（重建連線並重新檢查）或 SKIP（設定空身分後讓掃描回到待機）。

### 6.7 SortArm 分類手臂

#### `WAR0154` — Sorting Arm X 軸即將超出行程

- **意義**：分類手臂 X 軸（`MSortingArmX`）在移動前的軟體行程檢查沒過──系統要它去的 X 目標位置，落在允許的軟體行程範圍（`SoftLimitN` ~ `SoftLimitP`）之外，所以在馬達還沒開始動之前就先擋下、發出警報。這是「移動前」的軟體保護，不是撞到實體限位開關。

- **常見原因**：
  - 某個分類手臂 X 的教導位置（例如某 Auto/Loader 分類欄的 X、或 Pitch 間距目標）被設在軟體行程範圍之外；
  - 由基準 X 加上欄位/間距算出的格位 X 目標，對目前這種盤型或欄位索引來說超出實體行程；
  - `MSortingArmX` 馬達參數裡的 `SoftLimitN` / `SoftLimitP` 設得太窄，蓋不住實際需要的行程；
  - 手臂因失步或原點回不準，導致目前位置偏掉，使得原本合理的目標算出來變成超出範圍；
  - 在教導的進階測試流程中，下達了一個超出範圍的 X 目標。

- **檢查點**：

  | 裝置 | 類型 | 應該狀態 | 目前狀態(故障) | 在哪看 |
  |---|---|---|---|---|
  | `MSortingArmX` | motor | 要去的 X 目標落在軟體行程範圍內（`SoftLimitN` ~ `SoftLimitP` 之間），允許移動 | 目標低於 `SoftLimitN` 或高於 `SoftLimitP`，移動在啟動前被拒絕 | MotionView（即時位置 + 設定的軟體行程 N/P 範圍）；Note 備註列上也會印出 target/now/N~P 數值 |
  | `Requested X target`（要求的 X 目標，邏輯） | logic | 上游算出的目的地（教導的分類手臂 X、或由格位推算的目標、或進階測試目標）落在 `SoftLimitN` ~ `SoftLimitP` 之內 | 以目前校正狀態算出的目標超出範圍；備註列會顯示 target 落在 N~P 範圍之外 | Note 備註列（target / now / N~P，單位 1/100mm）；State Record・EventLog 看前一步的移動判斷背景 |
  | `MSortingArmX SoftLimitN / SoftLimitP` | logic | 軟體行程範圍夠寬，能涵蓋所有合法的分類手臂 X 目的地（各 Auto/Loader 分類欄教導位置加上格位間距展開） | 範圍設得太窄（或整體偏移），把合法目的地卡掉、拒絕移動 | MotionView 的 `MSortingArmX` 軟體行程欄位；馬達參數表中設定的行程上下限值 |

- **處置**：先看 Note 備註列，上面會顯示 target（要去的位置）、now（目前位置）與軟體行程範圍 N ~ P（單位 1/100mm）。用這三個數值判斷：如果是某個教導位置或算出的格位目標超出範圍，需先修正該教導值；如果 now 明顯偏掉、疑似失步或原點跑掉，先讓分類手臂重新回原點使 now 回到校正狀態。修正之後才按 RETRY。若一按 RETRY 又立刻重跳同一警報，代表目標確實超出範圍，不要一直重試，請停機找維修人員（可能需放寬馬達行程上下限）。這是純軟體行程保護，IOsetview 上沒有對應的 IO 燈可看，只能對照 Note 數值與 MotionView 判讀。

---

## 七、附錄：快速查碼與位址說明

### 7.1 代碼快速定位
| 看到的代碼 | 查哪一節 |
|---|---|
| `4xxxx`（5 位、開頭 4） | 三、汽缸類 → 3.5 對照表，末碼 3=伸出失敗 / 0=縮回失敗 |
| `5xxxx`（5 位、開頭 5） | 四、馬達類 → 4.2 找馬達，末碼查 4.1 |
| `6000x` | 五、真空吸嘴類 |
| `JAM…` / `MES…` / `WAR…` | 六、系統流程類（依模組分節） |

### 7.2 IO 位址欄位說明
位址格式 `L<環號>/IP<節點>/P<埠>/b<位元>`，代表這個訊號點在 MotionNet 上的位置：第幾條環、第幾個節點、第幾個埠、第幾個位元。機台就是照這四個數字去讀取該點是 ON 還是 OFF。

- **環號 Lane**：0 起算，本機共 2 條環（只有 Lane 0、Lane 1 存在；若看到 Lane 2 表示該點讀不到）。
- **InType 極性**：0=常開(NO)、1=常閉(NC)；判讀 ON/OFF 時務必對照極性，否則會判反。
- **Enable**：0=停用（該點不讀、也不會發到位警報）。
- ⚠️ IOsetview 滑鼠停留時顯示的位址，其 IP/埠部分經過另一層編碼轉換，與本表的原始數值不一定逐字相同；**搜尋時請以 Alias 名稱為準**，位址僅作輔助。

---

*本手冊由 `system/AlarmList.csv`（開機時自動匯出）與 `system/IO_Table.csv` 交叉比對，並參照機台實際運行行為整理而成。生成日期 2026-07-13。*
