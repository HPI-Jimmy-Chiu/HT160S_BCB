# HOME 汽缸體位 Manifest + 感測讀值硬化 — 設計計畫(2026-07-20)

來源:京元竹南 2026-07-17 State Record 三包分析(CASE-KYEC_CHEN-20260715-001)
+ 機構 owner 口述特性(rise1 假亮 InputHasTray、TrayArm Z-down 假亮 OutputBottomHasTray、
PushTray_On reed=夾盤真值、定點 sensor 不隨 Y 車)。前置清理已完成:
未安裝汽缸家族 16 顆全數移除(commit `6317107`,含 IO_Table.csv -48 列;
**部署時客戶機台 system\IO_Table.csv 必須同步**)。

## 已拍板決議

| # | 決議 |
|---|---|
| D-1 | 消費點遇 rise1 未確認在下:**不 mint、不盲動、原地等**;超 10s(可設定)→ 具名 Note(K_RETRY,點名 FrontRiseTray_1 未回位)。完成動作只能由擁有狀態的 ladder 做;盲降 rise1 有摔疊倉風險(aLoader.cpp DoFrontDestackDown case 6 安全閘自證) |
| D-2 | 檢查點 1 = uHome case1,drain 收斂/逾時/跳過之後、停放快照前:只驗 Class 1 |
| D-3 | 檢查點 2 = case200 全軸歸位後、case300 取回前:驗 Class 1+2+3(取回前夾爪期望恆 Off,取回自身每步 Push() reed 自驗) |
| D-4 | 不合格 → 具名彙總 Note(逐顆列名稱/期望/實際),按鍵 RETRY(重跑 drain 再驗)/ CONTINUE(EventLog 記名承擔);**自動觸發 State Record dump(TriggerReason=HomePostureCheckFail)**,訊息明示「通常為程式流程未涵蓋或硬體異常,State Record 已打包,請將 D:\HT160S_StateRecord 最新資料夾交工程師」 |
| D-5 | AMR 非 IDLE 凍結站:manifest 條目跳過,回 IDLE 時補驗該站切片(phase-2) |
| D-6 | TrayArm 夾爪:持盤(fHasTray 或任一 clamp OnSensor 亮)時跳過不驗(residue-hold 設計) |
| D-7 | 分離爪語意(全部分離爪適用):**常態=伸出持疊=Pop 態;Push=打開(非常態瞬間)**。無 reed 且永不加裝 → 只驗 out-bit。程式內「extend Separate」註解與機構相反,實作時順手修正 |
| D-8 | 不自動矯正任何汽缸(相位互鎖/摔倉風險);矯正=操作員 iosetview 手動 |

## 體位表 v1(移除後全機 39 顆,全覆蓋)

### Class 1 — destack 機構(檢查點 1+2)
| 汽缸 | 期望 | 驗證 |
|---|---|---|
| C_Loader/Empty/Color_FrontRiseTray_1/_2、C_Auto1..6_FrontRiseTray(12 顆) | Pop(在下) | reed(Off reed,Enable-gated) |
| C_Loader/Empty/Color_FrontSeparateTray_1(3 顆) | Pop(**伸出持疊**) | out-bit only(無 reed) |

### Class 2 — 10 車夾爪(僅檢查點 2)
| 汽缸 | 期望 | 驗證 |
|---|---|---|
| Loader1/2、Auto1..6、Empty、Color 的 PushTray+LeanOnTray(20 顆) | Off(全放,case50/60 之後) | reed(Enable-gated) |

### Class 3 — TrayArm(僅檢查點 2)
| 汽缸 | 期望 | 驗證 |
|---|---|---|
| C_TrayArmZ_Up / C_TrayArmZ_Down | Up=On 且 Down=Off | reed |
| C_TrayArm_FrontClamp / RearClamp | Off;**持盤時跳過**(D-6) | reed+latch 雙判準 |

### Class 5 — SortArm 真空(phase-2)
slot bHasIC=允許 On,否則 Off(SP-1 drain 已對帳,收編為驗證項)

## 施工序(依既定優先)

1. **D|幽靈盤三點封堵**:LK-1(aLoader case10)、CleanOut 自收(case9000 分支)、9500 confirm
   三處 InputHasTray 讀值加「rise1 Off-reed 確認在下」前提(D-1 政策);同一 sensor 雙防線
   相關性失效問題就此解除
2. **B|停放快照雙判準**:park gate 改 `fHasTray OR PushTray_On reed`(無聲失盤封堵);
   ghost 分支(帳=true、reed=OFF)候選:(a)不停放+EventLog+人工確認(推薦)/(b)照停放讓取回逾時自曝 — **待拍板**
3. **Manifest 兩檢查點落地**(本計畫主體;復用 ts_IOSelfTest 巡檢核心+自動 State Record)
4. **A|讀值語意層鋪開**:InputHasTray 有效=rise1 在下;OutputBottomHasTray 有效=TrayArm 不在該站或 Z 已升。
   先套會寫狀態的(XS-1/XS-2 簽收、W1 Color keep、RefreshRearState),再套只擋流程的(finish/警報)
5. **C|Empty/Color carry gate 改讀 reed** — 前提待確認(見開放問題)
6. **E|UX/指引**:Lot-Start 偵測 bRunSimulation+REALLY 警告視窗;MES0921 增「重新盤點」鍵;
   客戶測試指引(手動餵盤關 AMR/無 IC 乾跑用 HAS_TRAY 檔;HOME 回夾=帶盤續產新功能)

## 開放問題(下輪先問)
1. B 的 ghost 分支 (a)/(b)(推薦 a)
2. Empty/Color 夾缸 reed 語意是否同 Loader PushTray(空夾 OFF/夾到盤 ON)→ 決定 C 可否直做
3. 事實 B(TrayArm Z-down 假亮 OutputBottom)適用範圍:僅 Loader?或 Empty/Color/Auto 各站 rear 皆同型?→ 決定 A 的鋪開範圍
4. 檢查點逾時秒數/Note 警報碼配碼(走 alarm-registry 穩定碼)

## 客戶案件對應(0717 三包結論)
- MES0921 連炸=bUseAMR=1+全日無 host(iSecsCarTrayCount=0→總數回退 iSimAmrMaxTray=10)+手動餵 28 盤,防護正常觸發
- HOME 後夾爪回夾=W3 PARK&RE-ACQUIRE 設計行為,MotionDetail 證明 Loader2 取回成功(home=1、cmd=46824=停放位)
- 空吸 SUC0011 ×26=Enable Simulation(bRunSimulation)+REALLY 檔位:TopCCD 逐格造碼+全格 HAS_OK_IC,實體無 IC → 真空確認失敗
