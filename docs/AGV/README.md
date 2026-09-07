# docs/AGV — AMR / AGV / SECS 材料交握文件索引

HT160S 的 AMR（KYEC AutoUP）/ AGV / E87 SECS 對接相關文件集中於此。

## 規格與對接

| 文件 | 說明 |
|---|---|
| [HT160S_E87_AGV_Operation_Manual.md](HT160S_E87_AGV_Operation_Manual.md) | HT160S E87 / AGV SECS 對接操作手冊（另有 `.html` 版） |
| [HT160S_E87_AGV_Communication_Draft_20260527.md](HT160S_E87_AGV_Communication_Draft_20260527.md) | E87 / AGV 通訊草案（早期規格） |
| [KYEC_AMR_SECS_Handshake_HT9045_20260625.md](KYEC_AMR_SECS_Handshake_HT9045_20260625.md) | KYEC AMR SECS 材料交握參考規格（給 HT160 導入用） |
| [HT9045_vs_HT160_SECS_Diff_20260625.md](HT9045_vs_HT160_SECS_Diff_20260625.md) | HT9045 vs HT160 SECS 命令面差異報告 |

## 根因分析 / 修正驗證

| 文件 | 說明 |
|---|---|
| [HT160S_Loader_WorkTray_Count_Fix_Verification_20260713.md](HT160S_Loader_WorkTray_Count_Fix_Verification_20260713.md) | Loader 工作盤數顯示異常（8 送 2 餵卻剩 4）根因與修正 `111b976` 三重驗證報告 |
| [loader_worktray_count_harness.cpp](loader_worktray_count_harness.cpp) | 上述報告用的決定論邏輯 harness（g++ 可直接編譯重跑，回歸驗證用） |

---

*相關記憶（Claude memory）：`ht9045-loader-amr-traykind-model`、`ht9045-cleanout-trigger-model`、
`agv-devicecount-and-stub-audit`、`secs-9045-vs-160-diff`。*
