# KYEC AMR SECS 材料交握參考規格（給 HT160 專案導入用）

> 本文用途：作為 **HT160 專案導入 KYEC AMR/AGV 自動搬料 SECS 交握**的單一參考規格，可被另一個 AI agent 直接當設計依據使用。內容由 6 個領域分析（Loader / Empty / Color / Auto / Host 命令傳輸層 / CEID 對照）整合、去重、交叉校對而成。
>
> **來源**
> - 機台：HT9046LS（HT9045 codebase），客戶 KYEC 京元電子。
> - 程式碼版本（唯讀基準）：`D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP`
>   - 連線握手 S1F14 回報型號 `HT-9046LS` / 版本字串 `V3.32B2_AMR_03_MTK`（`SECSGEM_TextLog_13.txt:2562`）。
> - Log 基準（ground truth）：`D:\HT9045\docs\KYEC\2026_06_08\SECSGEM_TextLog_00.txt ~ _19.txt`（2026-06-08 真實連線；活躍生產段為 `_13`~`_19`）。
> - 機台 ID（log 內 SVID）：`1080_AMR-3781_H9046_11_070_V02_ART`。
> - 客戶代碼：`CC_KYEC_LEE=921`、`CC_KYEC_CHEN=920`、`CC_KYEC_JCTHIU=922`、`CC_KYEC_XILINX=924`、`CC_KYEC_STM=925`。
>
> **重要前置認知（整份文件的核心發現）**
> 1. 題目背景所列的「材料 CEID 94-103 / 116-118 / 130-190」這批是 `EventDescription[]` 字串表的**邏輯描述編號**，且絕大多數在本 build **沒有任何 `EventReport()` 觸發點**（為 ASE-KH 等其他客戶保留的死碼）。
> 2. KYEC 本案 AMR 交握實際使用的線上 CEID 是 **272 / 273 / 274 / 275**（AGV Supplement / LDUnLD Status / LDUnLD Finish / LD ID）。對應材料事件必須以 `SECS_EVENT` enum 的**線上整數值**為準，不可用描述表的 94-190。
> 3. Loader 補料是 **host 主導（host-pull）**，由 host 下 `S2F41 START_AGV` 啟動；Empty/Color 補料是**純機構 sensor 驅動**（內部 TrayArm，無 AMR/SECS）；Auto1/2/3 收盤是 **handler 報事件 → host 決策 → host 下 START_AGV** 的兩段式交握。

---

## 一、使用者 4 點認知查證結論

| # | 使用者認知 | 判定 | 一句結論 | 證據指引 |
|---|-----------|------|----------|----------|
| 1 | Loader：handler 透過 SECS 通知 AMR 來放料，並告知此次有幾盤；tray count=1 代表共 3 盤（載盤+上蓋+IC盤） | ⚠️ 部分正確 | 「要 AMR 來服務」方向對（CEID 273/274），但「告知幾盤」**方向相反**——盤數是 host 在 `START_AGV` 內給 handler 的；且「1=3盤（載盤+上蓋+IC盤）」語意不成立，程式無「載盤」獨立計數。 | `uHGemHT9045.cpp:1681-1684`（host 給 `LoaderTrayCount`→`iSECSSetTrayCount`）；`uLotInfo.cpp:13056`（`iAMRLDSECSTrayCount = iSECSSetTrayCount + iAMRCoverTray`）；log `SECSGEM_TextLog_15.txt:506,542` |
| 2 | Empty：handler 透過 sensor 偵測沒盤，主動要盤，直到 AMR 搬運成功 | ⚠️ 部分正確 | 「sensor 偵測沒盤→主動要盤→等到搬運成功」這個閉環**正確**，但**送出的不是 159-162**，而是泛用 CEID **272 AGVSupplement（帶 P2:1）**；要盤路徑在 `uLotInfo.cpp CheckLDLevel()`，不在 `asendic_Empty.cpp`。 | `uLotInfo.cpp:13168-13192`（sensor OFF→送 272→卡 case 100→sensor ON 才解除）；sensor `SnEmptyTrayHasTray_AGV` `uLotInfo.cpp:13163` |
| 3 | Color：同 Empty | ⚠️ 部分正確 | 「同 Empty」**正確**，但兩者皆**無 AMR/SECS 交握**——都是 sensor + 內部 TrayArm 補盤。Color 區 CEID 163/164/165、178-189 為定義但從不觸發的死碼。 | `asendic_Color.cpp:617-624,658`；CEID 死碼 `uHGemHT9045.h:179-205` / `uHGemHT9045.cpp:243-269`，全 repo 無 `EventReport(SECS_EVENT.Color_*)` |
| 4 | Auto1/2/3：滿盤後透過 SECS 通知 AMR 來收盤 | ⚠️ 部分正確 | 「透過 SECS 通知」**成立**，但非 handler 直接叫 AMR——是 **handler 報事件（136-138 退盤/35-37 滿盤）→ host 決策 → host 下 `START_AGV(AUTOn,Action)`** 兩段式；AMR 搬運握手再走 273/274（帶 P4/P5/P6）。 | `asendic_Auto.cpp:1769`（`Auto1Unloadtray+Pos`）、`:1098`（`Auto1Full+Pos`）；`uLotInfo.cpp:13097-13140`（UnLoaderAction 273/274）；log `SECSGEM_TextLog_18.txt:442,469,500`（CEID 136×2/137×3 已送出） |

> 補充：Loader/Empty/Color/Auto 四者「要不要走 AMR/SECS」並不一致——這是導入 HT160 時最容易誤解的點，務必逐站確認。

---

## 二、SECS 交握機制總覽

### 2.1 訊息方向與基本機制

- Log 標頭：`[Receive]` = host→handler；`[Send]` = handler→host。
- **Equipment-initiated 事件**：handler 觸發 `EventReport(SECS_EVENT.<名稱>)` → `UsecegemMainFrom.cpp:190` `HGem->EventReport(1, Ceid)` → **S6F11 Event Report**（DataID 固定為 1）→ host 回 **S6F12**。
- **Host-initiated 命令**：host 下 **S2F41 Remote Command (RCMD)** → handler 解析後回 **S2F42**（帶 `HCACK`：0=成功 / 1=命令無效 / 2=不可執行如機台有 IC）。
- S6F11 body 結構：
  ```
  <L[3]
    <U4[1] DataID=1>
    <U4[1] CEID>
    <L[reports]
      <L[2] <U4 RPTID> <L[n] 各值...> >
      ...
    >
  >
  ```

### 2.2 Report 與 SVID 由 host 動態定義（關鍵機制）

持久化檔（如 `EventReport_*.def`）僅存 Report1→SVID1027，其餘**全靠連線時動態定義**，所以 **log 才是真實 payload 來源**。連線序（`SECSGEM_TextLog_13.txt`）：

```
S1F13 Connect → S1F17 ON-LINE
→ S2F37 Disable all（先全關，<L[2] <Bool 0x00> <L[0]>> @2589）
→ S5F3 Enable Alarm（<L[2] <B 0x01> <L[0]>> @2613）
→ S2F33 清空 → S2F35 清空
→ 大量 S2F33（定義 RPTID→SVID）
→ 大量 S2F35（link CEID→RPTID，log_13 共 35 個 link 區塊）
→ 最後 S2F37 Enable（啟用指定 CEID）
```

**S2F33 報表定義（RPTID→SVID，KYEC 2026-06-08 實證）**

| RPTID | SVID 內容 |
|-------|-----------|
| 502 | `1006 1007 1011 3 1501 1517 1518 1513`（狀態/批次上下文，幾乎掛在每個 CEID） |
| 513 | `1151 1152 1153 1154 1155 1156` |
| 503 | `1420 6001 6002` |
| 504 | `20001 20002 20003` |
| **2000** | `38202 38205 38206 38207 38219 38220 38221`（**材料/AMR 相關 SVID**） |
| **2001** | `38222 ... 38236`（15 個，計數陣列） |

**S2F35 link（CEID→RPTID，KYEC host 實際啟用，實證 `SECSGEM_TextLog_13.txt:5814`起）**

```
CEID 1   -> 501 513 502 505 506 507 508 509 514 512 800 801
CEID 76  -> (同上)        CEID 42 -> 501 513 502 505 506 507 508 509
CEID 49  -> ...900        CEID 26 -> 503 501 502 513 508
CEID 48  -> 504 502       CEID 34 -> 510 502 521
CEID 80  -> 502 509 700
CEID 272 -> 502 2000 2001    CEID 273 -> 502 2000
CEID 274 -> 502 2000         CEID 275 -> 502 2000
(其餘單值：27/44/45/13/15/9/10/14/54/70/73/78/19/125/126/212/213/58/59/61/63 → 502 或單一 report)
```

> **「abort send」語意**：handler 內部觸發了某 CEID 但 host 當時未 enable，log 會印 `Event Report(6,11) , DataID=1 , CEID=N be disabled , abort send !!!`，該事件不送出。例：CEID=80 在 HALT 期間反覆 abort（`SECSGEM_TextLog_18.txt:2`）。

### 2.3 材料/AMR 三個關鍵 SVID（位元字串載體）

材料事件 272/273/274 的「哪個站」資訊用 `P1..P6` 位元字串表達，落在 RPTID 2000 的某個槽位（依事件不同槽位不同）。對應全域字串變數（`SECSGEM/uHGemHT9045_SV.cpp:801-803`）：

| SVID | 名稱 | 全域字串變數 | 用於 CEID |
|------|------|--------------|-----------|
| 38219 | AMR Supplement Bin | `asSupplementBin` | 272 |
| 38220 | LD UnLD Check AGV | `asLDUnLDCheckAGV` | 273 |
| 38221 | LD UnLD Finish AGV | `asLDUnLDFinsihAGV` | 274 |

**P 位元對應站別**：`P1=Loader`、`P2=Empty`、`P3=Color`、`P4=Auto1`、`P5=Auto2`、`P6=Auto3`（UnLoaderAction `uLotInfo.cpp:13120-13122`）。

---

## 三、四大材料流程

### 3.1 Loader 補料（host-pull，走 CEID 273/274）

**觸發條件**
- host 下 `S2F41 START_AGV`，內含 `<L[2] <A "Loader"> <A "Action">>` 子命令（及 `LoaderTrayCount`）。
- handler 解析（`uHGemHT9045.cpp:1676-1680`）：`bLoaderSECSActionFlag[0]=true; fLotInfo->InitialLoaderTask(0)` → `LoaderAction(0)`（`uLotInfo.cpp:13000`）從 case 1 進 case 100。
- 註：另有機構面取盤狀態機 `DoSupplyNewICTray()`（`asendic_Loader.cpp:62`），由 sensor/馬達 `fHasTray` 驅動，負責把料車實體 tray 搬到等待位，並在 case 1300 送通用 CEID 66 `LoadTrayFinish`（`asendic_Loader.cpp:778,802,830`）；此路徑**非 AMR 專屬**，與下面的 AMR 交握是兩條獨立路徑。

**handler 送出之 SECS（兩段）**
1. `case 100`（`uLotInfo.cpp:13018`）：缸體就位後組 `P1:1,P2:0,...,P6:0`（P1=Loader），塞進 `asLDUnLDCheckAGV`，呼叫 `EventReport(SECS_EVENT.AGVLDUnLDStatus)`（CEID **273**，`uLotInfo.cpp:13028`），進 case 200 等 sensor `SnLoaderTrayHasTray_AGV` 確認到盤。
2. AMR 放完盤（`case 300`，`uLotInfo.cpp:13053`）：更新盤數並送 `EventReport(SECS_EVENT.AGVLDUnLDFinish)`（CEID **274**，`uLotInfo.cpp:13041`）。`case 400` 清旗標。

**內容（實際 log body）**

host → handler `S2F41 START_AGV`（`SECSGEM_TextLog_15.txt:505-547`, Receive 15:13:24）：
```
[S2F41] Remote Command with Parameters
<L[2]
  <A[9] "START_AGV">
  <L[10]
    <L[2] <A[6] "Loader"> <A[6] "Action"> >
    <L[2] <A[5] "Empty">  <A[2] "NA"> >
    <L[2] <A[5] "Color">  <A[2] "NA"> >
    <L[2] <A[5] "AUTO1">  <A[2] "NA"> >  ... (AUTO2~AUTO5 皆 NA)
    <L[2] <A[15] "LoaderTrayCount"> <A[1] "1"> >
    <L[2] <A[13] "LoaderICCount">   <A[1] "3"> >
  >
>
```
handler 回 `S2F42 <L[2] <B[1] 0x00> <L[0]>>`（HCACK=0，`SECSGEM_TextLog_15.txt:550`）。

handler → host `S6F11` CEID 273（`SECSGEM_TextLog_15.txt:563-590`, Send 15:13:25）：
```
<L[3]
  <U4[1] 1> <U4[1] 273>
  <L[2]
    <L[2] <U4[1] 502> <L[8] ... "1080_AMR-3781_H9046_11_070_V02_ART" ...> >
    <L[2] <U4[1] 2000> <L[7] "NA" "NA" "NA" "NA" "" "P1:1,P2:0,P3:0,P4:0,P5:0,P6:0" "" > >
  >
>
```
handler → host `S6F11` CEID 274（`SECSGEM_TextLog_15.txt:617-644`, Send 15:14:26，約 60s 後）：`P1:1` 改落在 list 第 7 槽（Finish 槽），來源 `asLDUnLDFinsihAGV`。host 兩次都回 `S6F12 <B[1] 0x00>`。

**host/AMR 回應**：host 先以 `START_AGV` 命令啟動補料；handler 對 AMR 的「派工」透過 CEID 273（Status, P1:1）通知；AMR 實體搬完後 handler 以 sensor 確認到盤，回 CEID 274（Finish, P1:1）。host 對兩個 S6F11 都以 S6F12(0x00) ack。

**序列圖（step list）**
1. host → handler：`S2F41 START_AGV(Loader/Action, LoaderTrayCount="1", LoaderICCount="3")`
2. handler → host：`S2F42 HCACK=0`
3. handler 設 `iSECSSetTrayCount=1`，跑 `LoaderAction(0)` → case 100
4. handler → host：`S6F11 CEID=273`（P1:1 = 要 AMR 服務 Loader）→ host 回 S6F12
5. AMR 實體放盤；handler 等 `SnLoaderTrayHasTray_AGV` ON
6. case 300：`iAMRLDSECSTrayCount = iSECSSetTrayCount + iAMRCoverTray`，設 `bAMRReceiveAGVStart=true`
7. handler → host：`S6F11 CEID=274`（P1:1 = Loader 補料完成）→ host 回 S6F12
8. case 400：清 `bLoaderActionFlag/bLoaderSECSActionFlag`

**tray count 語意（使用者認知第 1 點細節）**
- `LoaderTrayCount` 是 **host→handler**，存入 `iSECSSetTrayCount`（`uHGemHT9045.cpp:1681-1684`：`iSECSSetTrayCount=atoi(S2.c_str())`）。
- 總交握盤數 `iAMRLDSECSTrayCount = iSECSSetTrayCount + iAMRCoverTray`（`uLotInfo.cpp:13056`、`:13079`）。
- `iAMRCoverTray`（上蓋盤數）由 TrayOrder 設定決定，值為 1/2/3（`cTrayAssignment.cpp:249-254`）：
  ```cpp
  if(TrayForm.iTrayOrder[1]==eNoUse)      iAMRCoverTray=1;
  else if(TrayForm.iTrayOrder[2]==eNoUse) iAMRCoverTray=2;
  else                                    iAMRCoverTray=3;
  ```
- 「1=3盤（載盤+上蓋+IC盤）」**不成立**：程式無「載盤」獨立計數，總數只由「IC 盤 + 上蓋盤」兩項組成。若現場真觀察到「1→3盤」，最可能是 `iAMRCoverTray==2`（1 IC + 2 上蓋）的設定，需以現場 `TrayForm.iTrayOrder[]` 確認。
- `LoaderICCount="3"` 是 **IC 顆數**，且 **handler 完全沒解析此欄位**（START_AGV 解析迴圈無 `LoaderICCount` 分支，全 repo grep 無 code match）——不是盤數、不進任何盤數計算。
- 「上蓋盤不放 IC」邏輯來源：`asendic_Loader.cpp:808` 以 `iLoaderTrayCountCal >= (iAMRLDNowTrayCount - iAMRCoverTray)` 判斷，超過 IC 盤數之後的盤設 `NULL_IC`（`asendic_Loader.cpp:810-816`）。
- UI 顯示：`labNowTrayCount`（`uLotInfo.cpp:13057`、`main.cpp:12863`）、累計 `labLoaderTrayCount`（`uLotInfo.cpp:13216`）、差異 `LabDiffTrayCount`（`uLotInfo.cpp:13064`）。

---

### 3.2 Empty 補料（純機構 sensor 驅動，無 AMR/SECS）

**結論**：Empty「要盤」是 sensor 閉環，但要注意兩條路徑：CEID 159-162 為死碼；真正的 AMR 補料（若該站接 AMR）走泛用 272/273/274（P2）。在本案 KYEC 2026-06-08 log 中，Empty 區一直有盤，**未觸發任何 P2:1 事件**。

**兩個不同的「沒盤」sensor，勿混淆**

| 用途 | Sensor | 出處 |
|------|--------|------|
| 生產流程偵測（機構內部，觸發 `DoAutoEmpty` 補車盤） | `SenEmptyHasTray` / `SenEmptyCWDete` / `SenEmptyCarHasTray` / `SenEmptyCCWDete` | `asendic_Empty.cpp:696,825,242-243` |
| **AMR 補料偵測（對外要盤用）** | **`SnEmptyTrayHasTray_AGV`（=556）** | `cmydef.cpp:1294`、`uLotInfo.cpp:13163`（注解 `kevin 20220520 add AGV Empty`） |

**觸發路徑（`CheckLDLevel(iAuto==1)`，由 Timer4 `CheckAMRAction()` 週期呼叫，`uLotInfo.cpp:13222-13228`）**
```
SenIndex[3] = {SnLoaderTrayHasTray_AGV, SnEmptyTrayHasTray_AGV, SnColorTrayHasTray_AGV};  // uLotInfo.cpp:13163
```
- case 1（沒盤→要盤）：`Sen[SnEmptyTrayHasTray_AGV].IsOff()==true` → 組 `str[1]="P1:0,P2:1,P3:0,P4:0,P5:0,P6:0"`（`uLotInfo.cpp:13174`）寫入 `asSupplementBin` → `EventReport(SECS_EVENT.AGVSupplement)`（CEID **272**，`uLotInfo.cpp:13180-13181`），`bLoaderActionFlag[1]=true` 鎖定，進 case 100。
  - 註：Loader（iAuto==0）有 `iTrayCount==0` 才不要的閘門；Empty/Color 無此閘門，沒盤就要。
- case 100（等搬運成功才收尾）：直到 `Sen[SnEmptyTrayHasTray_AGV].IsOn()==true` 才 `Task=1` 解除（`uLotInfo.cpp:13188-13192`）——這就是使用者說的「直到 AMR 搬運成功」。

**中段握手**（若該站走 AMR）：`LoaderAction(iAuto==1)` 以 `asLDUnLDCheckAGV`（P2:1）發 CEID 273（`uLotInfo.cpp:13028`），sensor 確認後發 CEID 274（`uLotInfo.cpp:13041`）。

**內容**：與 Loader 同結構，差別在 P 位元為 P2。本案 log **全程無 P2:1 send**（掃 `_13`~`_19` 僅見 P1/P4/P5），故 Empty 補料的真實 S6F11 body 待現場補一筆 `P2:1` trace。log 中的 `<U4 38219/38220/38221>`（`_13.txt:5761-5763` 等）只是 host S2F33 定義 SVID，非 Empty event 的 send。

**`asendic_Empty.cpp` 與 SECS 的關係**：`asendic_Empty.cpp` 是純機構狀態機（飛梭升降、Fix 氣缸、TrayID 讀取、料盤前後送），**不送任何 SECS**。AMR 模式下其補車盤由 `DoAutoEmpty()` 的 `TrayForm.bEnableAMR` 分支放行（`asendic_Empty.cpp:681-684`）。它與 `uLotInfo.cpp` 要盤路徑透過共同 sensor `SnEmptyTrayHasTray_AGV` 間接協調，**無函式呼叫關係**。

**序列圖（step list，AMR 模式）**
1. Timer4 → `CheckLDLevel(1)`；偵測 `SnEmptyTrayHasTray_AGV` OFF
2. handler → host：`S6F11 CEID=272`（`asSupplementBin` = P2:1）→ host 回 S6F12
3. (若走 AMR 握手) handler → host：`S6F11 CEID=273`（P2:1）；AMR 放盤
4. sensor `SnEmptyTrayHasTray_AGV` ON → handler → host：`S6F11 CEID=274`（P2:1）
5. case 100 解除，回 Task=1

---

### 3.3 Color 補料（純機構 sensor 驅動，無 AMR/SECS；Color CEID 為死碼）

**結論**：與 Empty 同邏輯——sensor + 內部 TrayArm 補盤，**無 SECS/AMR 交握**。Color 區 CEID 163/164/165、178-189 為定義但從不觸發的死碼。

**核心旗標**（`cmydef.cpp:2657,2662`）：`int iReceiveColorTray`（收/退盤工作態）、`bool fColorCanSupplyNewTray`（可否補新盤，預設 true）。

**主入口 `DoAutoColor()`（`asendic_Color.cpp:598`）**
- `:617-624`：`iReceiveColorTray!=0` → `DoAutoColorReceive()`；`fColorCanSupplyNewTray==false` → return（不補盤）。
- `:658` case 1：以 `Sen[SenColorHasTray].IsOn()` 等 sensor 判斷有無盤，決定走補盤（case 100→200）。
- `:785-816` case 100：依 `MOT[MMColor_Car].fHasTray` 與 `MTrayX` 位置決定 `InitLoadNewColorTrayToCarTask()`（補新盤）或 `DoColorTrayToRear()`。
- 補盤動作 `DoLoadNewColorTrayToCar()`（`asendic_Color.cpp:38`）即 Tray Group 升降機構（CylinderUp→Middle→Lower，case 1→60→200→410→420），與 Empty/Loader 同一套機構模板。完成時 `iTrayTotal[2]++`（`:249`，索引 2=Color）。
- 盤來自機台內部 TrayArm（`acatchtray.cpp`），`fColorCanSupplyNewTray` 由 TrayArm 在 `acatchtray.cpp:2776,6722,7045` 等處設定——屬內部交握，**全程不發任何 SECS**。

**Color 區 CEID 死碼證據**
- enum：`SECSGEM/uHGemHT9045.h:179-205`（`Color_PreLoadTray`=163, `ColorOnlyOneTray`=164, `ColorNoTray`=165, `Color_PutTrayToAuto1-6`=178-183, `Color_PutCoverToAuto1-6`=184-189）。
- 描述：`SECSGEM/uHGemHT9045.cpp:243-269`。
- 全 codebase **無任何 `EventReport(SECS_EVENT.Color_*)` 呼叫**（KenHsieh 20230918 註解：為 ASEKH_K1/K3 客戶保留）。
- log 佐證：2026-06-08 host 實際 enable/送出的 CEID 僅 80（10962 次，多被 disable）、14、93、44、27、141、55；**完全無 159-165、178-189**。

**Color vs Empty 對照**

| 面向 | Empty | Color | 是否相同 |
|------|-------|-------|----------|
| 補料觸發 | sensor 無盤 + `fEmptyCanSupplyNewTray` | sensor 無盤 + `fColorCanSupplyNewTray` | 相同 |
| 升降機構 | Tray Group 模板 | 同模板 | 相同 |
| 補料來源 | 內部 TrayArm（非 AMR） | 內部 TrayArm（非 AMR） | 相同 |
| SECS 補料交握 | 無（159-162 dead） | 無（163-165 dead） | 相同 |
| 顏色盤辨識 | 無 | `Sen[SnColorColorTrayDetect]` + `USE_COLOR_TRAY_SENSOR` 黑/白盤辨識，誤判發 `WAR1451`（`asendic_Color.cpp:566-573,671-678`） | **Color 獨有** |
| TrayID 2D 讀取 | — | case 52/53 透過 `RespondASECom("@e02026")` 讀 `TrayID[2][0]`，逾時 `WAR16335`（`asendic_Color.cpp:132-181`；走 ASE 序列埠，非 SECS） | **Color 獨有** |

---

### 3.4 Auto1/2/3 收盤/退盤（handler 報事件 → host 決策 → host START_AGV）

**狀態機覆蓋範圍（只有 Auto1~3）**
- 主迴圈 `csystem.cpp:8680` `for(int i=0;i<cmpt;i++) DoAutoReceiveBinTray(i)`，`cmpt` 預設 3（`csystem.cpp:6410,6692`；`AUTO3_IS_MAGAZINE==1` 時降為 2，`:8678`）。`Pos` 只會是 0/1/2。
- sensor/物件全用 `+Pos`：滿盤 `SnAuto1IsFull+Pos`、預警 `SnAuto1IsPreAlarm+Pos`（`cmydef.h:645,1040`），馬達 `MMAuto1+Pos`、`MMAuto1_Car+Pos`。
- `asendic_Auto2.cpp` 無任何 SECS 觸發；`asendic_Auto_RT.cpp:515` 是同函式 RT 模式呼叫。

**滿盤判定（兩條獨立路徑）**
1. 軟體計數滿：`asendic_Auto.cpp:520` case 850，`MOT[MMAuto1+Pos].Tray.FullIC()` → Task=900；AMR 計數 `:548 iUnloaderTrayCountCal[Pos]++`，`:564 >= TrayForm.iFullTrayCount` 時設 `bNeedCoverTray[Pos]=true; bAMRFullTray[Pos]=true`（受 `TrayForm.bEnableAMR && bEnableAMRLoader==false` 等保護）。
2. 實體滿盤 sensor `SnAuto1IsFull+Pos`：
   - case 900（`:625-632`）：僅 `CUSTOMER_CODE==CC_KYEC_XILINX || CC_KYEC_LEE` 時 sensor On 跳 Task=902（`MES%d20` 重試），**不送 SECS**。
   - case 1100（`:1076-1118`）：sensor On，非「不立即停機/PreAlarm/UnloadEnd 後才 Alarm」分支時，`:1097-1098 if(IniConfig.bEnable_SECS_GEM==true) EventReport(SECS_EVENT.Auto1Full+Pos)` 後彈 `MES%d20`（MES1120/1220/1320）。

**送出的 CEID**

| 觸發點 | 程式碼 | CEID 表達式 | 解出值 (Pos=0/1/2) | enum 來源 |
|--------|--------|-------------|--------------------|-----------|
| 滿盤 sensor On（case 1100） | `asendic_Auto.cpp:1098` | `Auto1Full+Pos` | 35 / 36 / 37 | `uHGemHT9045.h:50-52` |
| 退盤完成（case 1500） | `asendic_Auto.cpp:1769` | `Auto1Unloadtray+Pos` | 136 / 137 / 138 | `uHGemHT9045.h:152-154` |
| AMR 滿盤補料（case 1358） | `asendic_Auto.cpp:1643` | `AGVSupplement` | 272 | `uHGemHT9045.h:288` |

- `case 900` 原本（`:614`）有一份 `Auto1Unloadtray` 已被**註解掉**（Steven 20210826 改為「退盤完成才發報」）；真正送出只在 case 1500 退盤完成後（`:1768-1769`，受 `ULDStackDelay[Pos].Off()` 與 `bEnable_SECS_GEM` 保護）。

**AMR 收盤握手**：`UnLoaderAction()`（`uLotInfo.cpp:13097`）以 CEID 273/274 帶 P4/P5/P6（`:13120-13122,13135-13137`），以 `Sen[SnAuto1TrayHasTray].IsOn()==false`（盤被取走）作完成判定。

**內容（log ground truth）**
- 本 session（2026-06-08）退盤事件 CEID 136×2、137×3 已送出，但 report list 為**空** `<L[0]>`（host enable event 但未 link SVID）：
  ```
  <L[3] <U4 1> <U4 136> <L[0]>>   (SECSGEM_TextLog_18.txt:442,469)
  <L[3] <U4 1> <U4 137> <L[0]>>   (SECSGEM_TextLog_18.txt:500,527; _18.txt:7021)
  ```
- 當天的 AMR 收盤由 host 主導，`START_AGV` 帶 `AUTO2="Action"`（`SECSGEM_TextLog_19.txt:696`），其餘 NA、無 LoaderTrayCount → 觸發 `bUnLoaderSECSActionFlag[1]=true; InitialUnLoaderTask(1)`。
- CEID 35/36/37（Auto Full）與 272（AGVSupplement）本 session **未送出**（Auto1/2/3 未進滿盤需停機狀態）。

**序列圖（step list）**
1. Auto 盤放滿 → case 1100 sensor On → handler → host：`S6F11 CEID=35/36/37`（Full，本案未觸發）
2. 退盤完成 case 1500 → handler → host：`S6F11 CEID=136/137/138`（Unloadtray，body 空 report）→ host 回 S6F12
3. host 決策後 → handler：`S2F41 START_AGV(AUTOn/Action)`
4. handler 設 `bUnLoaderSECSActionFlag[Pos]=true`，跑 `UnLoaderAction(Pos)`
5. handler → host：`S6F11 CEID=273`（P4/P5/P6 = 該 Auto 站）；AMR 取盤
6. `Sen[SnAuto1TrayHasTray].IsOn()==false`（盤取走）→ handler → host：`S6F11 CEID=274`（同 P 位元）

**+Pos 錯位陷阱（重要）**：enum 排列**非連續**——`Auto1Full=35/36/37` 但 `Auto4Full=148/149/150`（`uHGemHT9045.h:164-166`）；`Auto1Unloadtray=136-138` 但 `Auto4Unloadtray=145-147`（`:161-163`）。`Auto1Full+Pos`/`Auto1Unloadtray+Pos` **只在 Pos=0~2 正確**；Pos≥3 會錯位（`Auto1Full+3=38=Fix1Full`，`Auto1Unloadtray+3=139=DoVisualSortLotStart`）。本 build 因 `cmpt≤3` 鎖死不會發生，但 HT160 若擴充到 6 個 Auto 區，此寫法必須改成查表（enum 註解 `//Steven 20230907 : For HT-9011UC`）。

---

## 四、Host 命令與傳輸層 + RCMD 對照表

### 4.1 S2F41 Remote Command 分派機制

- 所有 RCMD 在 `uHGemHT9045.cpp` 的 `HT9045Gem::S2F42_Host_Command_Acknowledge()` 內以 `else if(S.AnsiPos("...")==1)` 鏈分派（函式名雖為 S2F42，實際解析進來的 S2F41 body 並組回 S2F42 ACK）。
- 命令字串先 `UpperCase()`（`:1079`），用 `AnsiPos(...)==1`（開頭比對）。**較長命令必須排在較短前面**：`START_LOT`@1537 → `START_AQL`@1601 → `START_AGV`@1655 → `REMOTE_START`@1725 → `START`@1748；同理 `INITIAL_START_ART`@1383 須排在 `INITIAL_START`@1400 前。
- 回覆統一以 `HCACK` 填入 S2F42（0=成功 / 1=命令無效 / 2=不可執行）。
- body 結構：`<L[2] <A RCMD> <L[n] <L[2] <A 參數名> <值>> ...>>`，參數空時為 `<L[0]>`。

### 4.2 RCMD 對照表

| RCMD | 程式位置 | 參數結構（log 實證） | Handler 行為 | 備註 |
|------|----------|----------------------|--------------|------|
| **`START_AGV`** | `uHGemHT9045.cpp:1655` | `<L[10] {Loader,Action}{Empty,NA}{Color,NA}{AUTO1..5,NA/Action}{LoaderTrayCount,"1"}{LoaderICCount,"3"}>` | 逐對解析：`Loader+Action`→`bLoaderSECSActionFlag[0]=true; InitialLoaderTask(0)`；`Empty`→`[1]`；`Color`→`[2]`；`AUTO1/2/3+Action`→`bUnLoaderSECSActionFlag[0..2]=true; InitialUnLoaderTask(0..2)`；`LoaderTrayCount`→`iSECSSetTrayCount=atoi(值)`。HCACK=0。 | **AMR 派車核心命令。** `AUTO4/AUTO5` 被讀但無分支（落空）。`LoaderICCount` **未被解析**（靜默忽略，待確認消費點）。 |
| `START` | `:1748` | 多為 `<L[0]>` | `bUseSECSGEMAutoStart` 開啟時：AMR 模式且 `bLotStart==true` 設 `bSameSetupFileNoDownload=true; bAMRReceiveStart=true; CheckActionFlag()`；非 AMR 直接 `fMain->Start()`。 | AMR 流程的「實際啟動」由 `START_AGV` 鋪 ActionFlag 後，`START` 觸發 `CheckActionFlag()`。 |
| `START_LOT` | `:1537` | key∈`LOTID/DEVICEID/OPERATORID/RUN_MODE` | 填 `fLotInfo` 對應欄位後 `sbSECSLotStartClick()`；未知 key→HCACK=1。 | Onsemi 引入；KYEC log 未見。 |
| `START_AQL` | `:1601` | key∈`COUNT/BIN`（U4） | 需 `bI52_bAQLSortMode==true` 且機台無 IC；設 `iAQLCount/iAQLBin`，`SetAQLMode(true)`。 | KYEC log 未見。 |
| `ONE_CYCLE` | `:1101` | `<L[0]>` | `bSECSOneCycleComm=true; BtnOneCycleClick()`，HCACK=0。 | log 中極高頻（HALT 待機時 host 反覆下達）。 |
| `LOTSTART` | `:2081` | `<L[0]>` | `sbSECSLotStart->Down=true; sbSECSLotStartClick()`。 | TSMC lot 查核；KYEC 連線時亦下達（`_16/_17/_18`）。 |
| `PP_MUSIC` | `:1808` | `<L[1] <L[2] <A ""> <U4 class>>>` | 空 list→關閉；否則設 `iSECS_GEM_PPMUSIC_CONTROL_CLASS`。 | 音樂塔控制。 |
| `PP_SIGNALTOWER` | `:1837` | `<L[3] {RED,U4}{GREEN,U4}{YELLOW,U4}>` | 設 RED/GREEN/YELLOW，空 list→關閉。 | 三色燈控制。 |
| `ENERGY_SAVING` | `:2857`（完全比對 `S==`） | `<L[1] <L[2] <A "STATE"> <U4 0/1>>>` | 有 ATC：機台無 IC 且非 Contact 模式且 `bC05_PowerSaveATC` 開→STATE=1 進省電；STATE=0 退出。失敗 HCACK=2。 | 有 IC/Contact 模式/功能未開皆拒。 |
| `CLEAN_AUTO_SORT_COUNT` | `:1145` | 多無參數 | picker/shuttle/index 有 IC→HCACK=2；否則清 ART 計數/yield/低良率清潔計數（`CC_JCET` 另有額外分支）。 | 清 Auto 分類計數與良率統計。 |
| `SWITCH_TO_FT` | `:1465` | 無參數 | `iSecsGemSwitchFTRT=1; HCACK=fMain->FTClick()`。`SystemStart` 時 HCACK=2。 | 對應 `SWITCH_TO_RT`@1494。 |
| `INITIAL_START_ART` | `:1383` | 無參數 | `HasICUnderMachine()==false`→`SetRunStartMode(rsmInitial_ART)`；AMR 模式設 `bSameSetupFileNoDownload=true`（Eastsun 20260420）。有 IC→HCACK=2。 | 與 `INITIAL_START`@1400、`INITIAL_START_MRT`@1285 區分。 |

其他同函式分派（補充）：`PAUSE/STOP`@1081、`RESET`@1108、`CONTINUE_RETEST_ART`@1114、`RETEST_MRT`@1270、`AUTOSITEMAP`@1333、`AUTO_RETEST`@1349、`TRAY_FEED`@1367、`ONLINE_LOCAL`@1527、`ONLINE_REMOTE`@1532、`REMOTE_START`@1725、`HALT`@1794、`AUTO_CLEAN`@1881、`PP_PASSWORD`@1895、`PP_SELECT`@1926、`AUTHORITY_CHECK`@2088、`YIELD_FAIL`@3006（僅 `CC_KYEC_LEE`）。

> RCMD 解析位置：`uHGemHT9045.cpp:1655` `S.AnsiPos("START_AGV")==1`。

### 4.3 其他傳輸層訊息

- **S1F3/S1F4 Selected Status Request**：host 查 `<L[1] <U4 1011>>`，handler 回 `<L[1] <A "HALT">>`（或 `"Alarm"` 等執行狀態字串）（`SECSGEM_TextLog_19.txt:560`）。SVID 1011 = 機台執行狀態文字。
- **S2F37 Enable/Disable Event Report**：連線初期 `<L[2] <Bool 0x00> <L[0]>>` 全停（`_13.txt:2589`），定義完 report/link 再 enable。
- **S5F1/S5F2/S5F3 Alarm**：`S5F3 <L[2] <B 0x01> <L[0]>>` 啟用全 alarm（`_13.txt:2613`）。S5F1 body：`<L[6] <A 時間戳> <U4 ALCD> <U4 ALID> <A alarm文字> <A 來源> <A 附加>>`，例 `ALID 215015154 "ATC Hot Air2 Temperature OverHigh!" "Temp. Controller"`（`_14.txt:63`）。alarm 同時併送 S6F11 與 S5F1。
- **S125F1/F2 自訂 EC Data Send（KYEC 專用，pig 2014.08.27）**：handler `HTGem::S125F2_EnableDisableECDataAcknowledge()`（`uHGemClass.cpp:2627`），dispatch `uHGemEquipment.cpp:9098`（S125F1→F2）、`:9101`（S125F3→F4 LevelSetting，`uHGemHT9045.cpp:5133`）。body：`<L[2] <B 模式> <L[n] ECID...>>`，host 用 `<B 0x80> <L[45] {1501,3540,...,8501-8506}>` 一次帶 45 個 ECID（`_13.txt:5255`）。格式錯誤回 `S9F7_IllegalData`。
- **Online/Offline**：S1F13 Connect（host `CJ_EAP`/`1.0.00`）→ S1F14（handler 回 `HT-9046LS`/`V3.32B2_AMR_03_MTK`）→ S1F17 Request ON-LINE → S1F18 ACK（`_13.txt:2557-2585`）。RCMD 層另有 `ONLINE_LOCAL`/`ONLINE_REMOTE`。
- **S2F15 New Equipment Constant Send**：host 設 EC（如 `<U4 35011> <A "True">`），handler 回 S2F16，並於 `S2F15_CheckNewEquipmentConstant()`（`uHGemHT9045.cpp:474`）依 ECID 區段觸發 ReloadParameter/寫檔。

---

## 五、材料事件 CEID 對照總表

> **編號釐清**：`EventDescription[]` 字串開頭的編號（94 Transfer Blocked … 130 Material Receive … 136-190）是**邏輯描述編號**，與線上 CEID 整數值不同。線上 CEID = `SECS_EVENT.<name>` enum 整數值。要對應「材料相關事件」**必須用 enum 整數值**。

### 5.1 本案實際使用的 AMR CEID（這才是真正的材料交握）

| CEID | 事件名 | 程式觸發點 | log 證據（2026-06-08） | RPTID 組成 |
|------|--------|------------|------------------------|------------|
| 272 | AGVSupplement（要料） | `uLotInfo.cpp:13181`（CheckLDLevel sensor IsOff）、`csystem.cpp:6614,6668,9408`、`asendic_Auto.cpp:1643`、`acatchtray.cpp:6342,6399` | 已送出 ×2（`_19.txt:366`、`_16.txt:5551`） | 502 + 2000 + 2001 |
| 273 | AGVLDUnLDStatus（交握中） | `uLotInfo.cpp:13028`(Loader)、`:13125`(UnLoader)；手動 `:12962` | 已送出 ×4（`_15.txt:564`、`_17.txt:1985`…） | 502 + 2000 |
| 274 | AGVLDUnLDFinish（搬運完成） | `uLotInfo.cpp:13041`(Loader)、`:13140`(UnLoader)；手動 `:12954` | 已送出 ×4（`_15.txt:618`…） | 502 + 2000 |
| 275 | AGVLdID（回報 Tray/Lot ID） | `acatchtray.cpp:5491` | 已送出 ×2（`_18.txt:1182`、`_16.txt:5621`） | 502 + 2000 |
| 66 | LoadTrayFinish | `asendic_Loader.cpp:778,802,830` | 待確認（建議 grep `CEID=66`） | — |
| 136/137/138 | Auto1/2/3 Unloading tray | `asendic_Auto.cpp:1769`（`Auto1Unloadtray+Pos`） | 136×2、137×3 已送出（`_18.txt:442,469,500,527`）；body 為空 `<L[0]>` | 空（host enable 但未 link SVID） |
| 35/36/37 | Auto1/2/3 Full | `asendic_Auto.cpp:1098`（`Auto1Full+Pos`） | 未送出（本案 Auto 未進滿盤需停機） | — |

### 5.2 實際 payload 解碼（ground truth）

**CEID 272 AGVSupplement**（`SECSGEM_TextLog_19.txt:366`）：
```
<U4[1] 272>
  RPTID 502  <L[8]>
    "LQ50SIJAG2"                          ← Lot ID / Job (10碼)
    "AGV"                                 ← 搬運種類 / carrier kind
    "RT1 ART"                             ← 製程/狀態欄（隨情境變："Cleaning"/"15"/"Alarm"）
    "2026060819001838"                    ← timestamp (yyyymmddhhmmssss)
    "1080_AMR-3781_H9046_11_070_V02_ART"  ← Recipe/Setup 檔名
    I4 9 / I4 2 / I4 1                    ← 數量/模式參數（待確認，疑似 site/mode/count）
  RPTID 2000 <L[7]>                       ← Tray ID 槽位 + P 位元
    "ERROR" / "5000...0133" / "5000...0139" / "NA" / "P1:0,P2:0,P3:0,P4:1,P5:0,P6:0" / "" / ""
  RPTID 2001 <L[15]>                      ← 計數陣列（I4 多筆）
```

**CEID 273 LDUnLD Status**（`SECSGEM_TextLog_15.txt:564`）：
```
<U4[1] 273>
  RPTID 502 <L[8]>: "" "" "Alarm" "2026060815132527" "1080_..._ART" I4 1 I4 2 I4 1
  RPTID 2000 <L[7]>: "NA" "NA" "NA" "NA" "" "P1:1,P2:0,P3:0,P4:0,P5:0,P6:0" ""
                      ← P1:1 = 此次針對 Loader
```

**CEID 275 AGVLdID**（`SECSGEM_TextLog_18.txt:1182`）：
```
<U4[1] 275>
  RPTID 502 <L[8]>: "LQ50SIJAG2" "AGV" "15" "2026060818040334" "1080_..._ART" I4 9 I4 2 I4 1
  RPTID 2000 <L[7]>: "50000000062480000000000000000139"  ← Tray ID 1
                     "50000000062480000000000000000133"  ← Tray ID 2
                     "50000000062480000000000000000130"  ← Tray ID 3
                     "NA" "" "" ""
```

### 5.3 題目背景所列 CEID 的「死碼/未送出」清單（HT160 勿照抄）

下列在本 build **無 `EventReport()` 觸發點**（僅存於 `EventDescription[]` 描述表），log 亦從未出現：

- **94**（TransferBlocked, `:172`）、**99**（CassetteOutComplete, `:177`）、**100**（CassetteUnclamped, `:178`）、**116/117/118**（Ready To Load No Lot/Tray/Cassette, `:194-196`）、**130**（MaterialReceive, `:209`）、**131**（SlotMapCountOK, `:211`）。
- **154-158**（Loader hasn't/full/only one/ready to unload/finish, `:234-238`）、**159-162**（Empty 系列, `:239-242`）、**163-165**（Color 系列, `:243-245`）、**166-177**（Empty put tray/cover to Auto1-6, `:246-257`）、**178-189**（Color put tray/cover to Auto1-6, `:258-269`）、**190**（Auto1 load tray finish, `:270`）。
- **145-147**（Auto4/5/6 Unloadtray）、**148-150**（Auto4/5/6 Full, `:224-229`）：`Auto1*+Pos` 只到 Pos=2，Auto4-6 為他機種（HT-9011UC）保留，本機未接線。

下列**有觸發點但需 MR/卡匣模式**（本案 AMR/Tray 模式未進入，故未送出）：

- **95**（`MR/acatchcassette.cpp:1646,1722,5568,5589`）、**96**（`:1796,1823,5618,5633`）、**97**（`:1976`）、**98**（`:4152`）、**101**（`:2023`）、**102**（`:2004,2034`；另 `csystem.cpp:6603` Eastsun 20260326 AMR 收盤前清材料 `UnloadComplete`）、**103**（`:4154`）。

---

## 六、HT160 導入注意 / 差異提醒

1. **CEID 號碼不可寫死**：線上 CEID 整數值由 `SECS_EVENT` enum 決定，與 `EventDescription[]` 描述編號（94-190）不同。HT160 導入時必須以目標 build 的 enum 整數值為準。本案材料/AMR 走 **272/273/274/275**，不是 94-103/130-190。
2. **Report 內容由 host 動態定義**：RPTID→SVID（S2F33）與 CEID→RPTID（S2F35）皆在連線時動態建立，持久化 def 只存 Report1→SVID1027。HT160 不可假設某 CEID 帶哪些 SVID——以 host 端連線定義為準。同一 CEID 在不同 host 設定下 body 可能完全不同（甚至空 `<L[0]>`，如本案 136/137）。
3. **KYEC 客製分支（`CC_KYEC_*`）**：
   - `asendic_Auto.cpp:625-632` case 900 滿盤 sensor 跳 Task=902 僅 `CC_KYEC_XILINX || CC_KYEC_LEE`。
   - `CLEAN_AUTO_SORT_COUNT` 對 `CC_JCET` 有額外 yield/site 清除分支。
   - `YIELD_FAIL` RCMD（`uHGemHT9045.cpp:3006`）僅 `CC_KYEC_LEE`。
   - HT160 若非 KYEC 客戶，這些分支不會走到，需另行確認對應流程。
4. **四站交握機制不一致**（最易誤解）：
   - Loader = **host-pull**（host 下 `START_AGV(Loader/Action)` 啟動，handler 回 273/274）。
   - Empty/Color = **純機構 sensor 驅動**，內部 TrayArm 補盤，**無 SECS/AMR**（159-165 死碼）。若 HT160 需要 Empty/Color 走 AMR，須確認是否啟用 `CheckLDLevel` 的 272 路徑（本案該站有盤、未觸發）。
   - Auto1/2/3 = **handler 報事件（136-138/35-37）→ host 決策 → host 下 `START_AGV(AUTOn/Action)`** 兩段式。
5. **`+Pos` 錯位陷阱**：`Auto1Full+Pos`、`Auto1Unloadtray+Pos` 因 enum 非連續，**只在 Pos=0~2 正確**。HT160 若擴充至 6 個 Auto 出料區（Pos≥3），此寫法會發錯 CEID（如 `Auto1Full+3=38=Fix1Full`），**必須改成查表**。
6. **RCMD 開頭比對排序**：`AnsiPos(...)==1` 是前綴比對，較長命令（`START_LOT/START_AQL/START_AGV/REMOTE_START`）必須排在 `START` 前；`INITIAL_START_ART` 須排在 `INITIAL_START` 前。HT160 新增 RCMD 時務必維持此排序，否則會被短命令攔截。
7. **`LoaderICCount` 未被消費**：host 在 `START_AGV` 送 `LoaderICCount`（IC 顆數），但 handler 解析迴圈**無對應分支**（靜默忽略）。HT160 若需用 IC 顆數，須自行加 code。
8. **tray count 語意**：`LoaderTrayCount` = host 給的 IC 盤數；總交握盤數 = `iSECSSetTrayCount + iAMRCoverTray`；`iAMRCoverTray`（上蓋盤）由 `TrayForm.iTrayOrder[]` 設定決定（1/2/3）。**沒有「載盤」這個獨立計數**，「1→3盤」非固定規則。
9. **兩個「沒盤」sensor 別混**：生產流程用 `SenEmptyHasTray`/`SenColorHasTray` 等；AMR 對外要盤用 `SnEmptyTrayHasTray_AGV(=556)`/`SnColorTrayHasTray_AGV`/`SnLoaderTrayHasTray_AGV`。
10. **`asendic_*.cpp` 不發 SECS**：`asendic_Loader/Empty/Color.cpp` 是機構狀態機，AMR/SECS 交握全在 `uLotInfo.cpp`（`LoaderAction`/`UnLoaderAction`/`CheckLDLevel`，由 Timer4 `CheckAMRAction` 驅動），兩者透過共同 AGV sensor 間接協調，**無函式呼叫關係**。例外：`asendic_Loader.cpp` 的通用 CEID 66 與 `asendic_Auto.cpp` 的 35-37/136-138/272 是機構檔內直接 emit。

---

## 附錄：未決 / 待確認清單

1. **`iAMRCoverTray` 在 KYEC 該機的實際值**（1/2/3）需由現場 `TrayForm.iTrayOrder[]` 設定檔確認，才能定論「1 盤 IC → 幾盤總交握」。
2. **CEID 66 `LoadTrayFinish`** 是否在 6/08 生產段實際被 host enable 並送出，log 未檢出；建議在 `_13`~`_19` 再 grep `CEID=66`。
3. **Empty 補料真實 S6F11 body（`P2:1`）** 本批 log 未出現（當天 Empty 站一直有盤），需現場補抓一筆 `P2:1` trace 確認。
4. **CEID 159-162 / 163-165 / 166-189 是否在其他客戶版本/設定有接線**：本 KYEC 版確認未接線（死碼）；若 HT160 規格書要求這些事件，須確認是否本 build 故意改用 272/273/274 泛用機制取代，或需另行接線。
5. **Auto 滿盤/退盤完整 trace**：本 session Auto 未進滿盤，CEID 35/36/37/138/272 未送出（136/137 雖送出但 body 為空）；需一份「Auto 真的放滿並退盤」期間的 log 驗證 `Auto1Full`/`AGVSupplement` 的實際 S6F11 body 與 host 是否 link SVID。
6. **`SECS_EVENT` enum 名稱 ↔ 線上整數值的完整逐一映射**（確認 272-275 ↔ 哪些材料事件名，及描述編號 94-190 ↔ enum 整數的對照）需另查 enum 定義。
7. **CEID 272 RPTID 502 中的 `I4 9 / I4 2 / I4 1`** 三個整數欄位語意（疑似 site/mode/count）待確認。
8. **`LoaderICCount` 是否在他處被消費**（目前 START_AGV handler 未讀取）待確認。
9. **START_AGV 中 `AUTO4/AUTO5` 無對應分支**是否為刻意（本機僅 AUTO1-3 走 UnLoader task）待確認。