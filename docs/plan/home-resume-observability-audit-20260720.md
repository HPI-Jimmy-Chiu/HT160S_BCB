# HOME 續產可觀測性稽核 — Log 是否充足?(2026-07-20)

動機:owner 判定「回 HOME 後繼續生產」將是未來 hang-up / 流程沒接上的高發區與
經常性分析課題,要求確認現有 Log 是否足以事後重建根因。
方法:4 個平行讀碼員(2 個因用量上限未完成,範圍由其餘讀者回填)+ 1 個盲點裁判,
裁判對讀者宣稱逐項抽驗源碼(TriggerSnapshot 呼叫點、SimHomeTrace #ifdef、
uHome RecordProcess 全集、note.cpp 二次警報丟棄、csystem finalize 零日誌 — 皆確認)。

## 判定:不充足

**物理復歸層夠、邏輯決策層全盲。** drain 階段/停放/取回/伺服電力循環在真機
EventLog 有 13 條 RecordProcess,含車號+Y 位置,夠用。但:

- **keep-material 整層靜默**:InitialAllTask(true) 收尾(csystem.cpp:1352-1383,
  含 Normal/CleanOut resume mode 抉擇)+ 六模組 InitialFlag 保/清分支 + ReassertLocks
  ——真機上零日誌。任何續產斷流的「起點基線」不存在。
- **heal 守衛全部靜默觸發**:XS-1 adopt-as-delivered(簽收幽靈交付)、XS-2 re-send/skip
  裁決、LK-1 認養、CleanOut 自收 adopt/decline、TrayArm un-adopt、CG-4 re-haul——
  觸發當下的決定性 sensor 讀值揮發,快照時已消失。守衛誤癒=最壞級別完全不可重建。
- **30 格 TaskHistory 結構性保證被沖光**:count-bounded ring(SR_MAX_HISTORY=30),
  0717 實測 resume burst 106ms 灌滿 30 格;HOME 完成又無自動快照 → 事後拿到的
  TaskHistory 必然只剩雜訊。
- **全機只有 2 個自動快照觸發**(TrayArm pick/place 看門狗)。經典案例:Auto 排出閘
  要求 bFullIC && bResidueClear,W4 重武裝的殘料驗證若未完成,station 永遠滿盤閒置
  ——無看門狗、bResidueClear 又不在任何 dump → 手動快照也看不到堵點。
- **Note modal 開啟時第二顆警報被整顆丟棄**(note.cpp:780 return 0,EventLog+SECS 都沒有)。
- State Record zip **不含 EventLog tail**(客戶寄 zip=只有狀態沒有敘事);EventLog 時戳
  1 秒解析度,對 resume burst 無法排序;SECS log RAM 緩衝當機丟最後 ~1s。

## 八種故障模式 × 三要件(偵測/當場證據/時間軸)

| 模式 | 偵測 | 證據 | 時間軸 |
|---|---|---|---|
| 1 跨模組握手死鎖 | 部分(僅 TrayArm 兩看門狗) | 觸發時佳/其餘無 | 否 |
| 2 幽靈/殘舊物料狀態 | 單向(keep 失敗才有補償警報;keep 錯了無證) | 部分 | 否 |
| 3 軸停滯/取回位置錯 | 佳(13 條+MotionDetail) | 快照才有 | 部分(homed vs skipped 只在 sim trace) |
| 4 無警報純閒置 | **失敗**(StuckMs 有算無人消費) | 無 | 否 |
| 5 守衛誤癒 | **完全失敗**(最壞) | 無 | 否 |
| 6 drain/park 未完成 | 部分(TIMEOUT 不點名模組;游標隨即被 InitialAllTask 抹掉) | 被設計性銷毀 | 否 |
| 7 AMR 握手跨 HOME 失同步 | 部分(CEID 有;內部鎖生命週期全靜默) | 部分 | 否 |
| 8 快照太晚=證據已失 | 結構性確認(30 格 ring+無自動快照+zip 缺 EventLog) | — | — |

## 補強清單(裁判定版)

### P0(全部=單檔單慣用法,不動控制時序)— **SHIPPED `e6b01a1` 2026-07-20**
(三建置 EXIT=0 + selftest 實測:七條 HOME-RESUME EventLog 行 + HomeResumeDone 快照 zip 皆已產生)
1. **resume manifest**:InitialAllTask(true) 尾端每模組一行「kept/wiped+關鍵 latch」+
   「Home finished: mode=<Normal|CleanOut> keepMaterial=1 byStart=<0|1>」(csystem.cpp:1383)
2. **自動快照 TriggerSnapshot("HomeResumeDone")**:InitialAllTask(true) 返回後立即(csystem.cpp:1360)
3. **heal 守衛 breadcrumb**:XS-1/XS-2/LK-1/case-9000 自收/un-adopt 每處一行
   (branch+決定性 sensor 讀值;抄 TA_DIVERT 慣用法 aTrayArm.cpp:1073)
4. **殘料驗證閘可見化**:bResidueClear+bDischargeTailPending+Feed/DischargeTask 入
   DescribeStation;bNeedResidueCheck/bResidueArmed 入 DescribeHolding;W4 重武裝+驗證
   verdict 各一行
5. **modal 期間被丟警報改記錄**:note.cpp:781 return 前 log「[DROPPED: modal busy]」
6. **AD-1 尾段 latch set/consume 各一行**(aAuto1To6.cpp:161/1715)

### P1 — 全數 SHIPPED (`5432441` + `c2ef5ee`) 2026-07-20
(#8 drain點名+HomeDrainTimeout快照 / #9 AGV鎖三處 / #11 毫秒時戳 / #12 SECS即時flush = `5432441`;
#7 StuckMs看門狗(auto snapshot,[Observability]StuckSnapshotSec 預設300s) / #10 zip收當日+前日
EventLog = `c2ef5ee`,與 whitelist-override 同車提交。全數 runtime-verified,三建置 EXIT=0)

### P1
7. 通用 StuckMs 看門狗(SystemStart=1 且模組 Task 停滯>閾值 → 自動 TriggerSnapshot;
   計算已存在 cStateRecordHT160.cpp:335-381,只差消費者)
8. drain TIMEOUT 點名:六模組收斂布林+內部游標,在 InitialAllTask 抹掉**之前**記錄
9. AGV 鎖生命週期三處 log(HOME 凍結 early-return/ReassertLocks 逐站結果/watchdog 強制釋放)
10. State Record zip 收錄當日+前日 EventLog(仿 CaptureSecsLog)
11. EventLog 時戳加毫秒(.zzz,同欄位相容)
12. SECS log 狀態性 stream(S2F41/S5F1)立即 flush 或例外掛鉤 flush

### P2 — #13 軸摘要 / #15 快照自記 / #16(tier-skip+Detail欄) SHIPPED;#14 StepTrace自動武裝與 #16b re-arm計數 未做

### P2
13. 真機版每輪 HOME 軸摘要(homed/skipped/disabled;字串 SimHomeTrace 已組好)
14. StepTrace 於 HOME+續產前 N 秒自動武裝並收進 zip(子任務游標唯一的連續史)
15. TriggerSnapshot 自身成敗入 EventLog(7z 失敗現在被吞)
16. 殘料驗證 tier-skip 一次性 log;看門狗 re-arm 計數;警報 Detail 欄入 EventLog

## 施工建議
觀測性 P0 包**先行**(在守衛 D/B/Manifest 之前):風險最低、不碰時序,且之後上機驗證
D/B/Manifest 全靠這些 log 當驗證儀器。與 [[home-posture-manifest-plan-20260720]] 的
D-4(體位檢查失敗自動 State Record)同向,合併為同一輪「觀測性 commit 群」。

殘餘不確定:HOME monitor 提前關閉路徑/Timer1 顯示層未被獨立複審(兩讀者撞上限),
惟該區前輪已修並有記憶佐證,風險低。
