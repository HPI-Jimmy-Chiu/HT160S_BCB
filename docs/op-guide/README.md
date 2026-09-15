# docs/op-guide — HT160S 現場操作員（OP）文件

給京元竹南 HT160S 現場 OP 的快速版操作文件（md + pdf 同步輸出）。工程版完整手冊在 `docs/manual/`。

| 文件 | 內容 | 檔案 |
| --- | --- | --- |
| HT160S-OP-001 | 非 AMR 模式（人工上下料）操作流程：開機→建批→放料→Home→Start→補料收料→清機→Lot End，含 5 張流程圖 | [HT160S_OP操作流程_非AMR模式.md](HT160S_OP操作流程_非AMR模式.md) / [.pdf](HT160S_OP操作流程_非AMR模式.pdf) |
| HT160S-OP-002 | AMR 模式（無人搬運車）下 OP 要做 / 不要做的事、交接流程、WAR0962 / WAR0963 等例外處理，含 3 張流程圖 | [HT160S_OP操作說明_AMR模式.md](HT160S_OP操作說明_AMR模式.md) / [.pdf](HT160S_OP操作說明_AMR模式.pdf) |

## 重生方式

```
python scripts/ops/gen-op-guide-flowcharts.py     # 重畫 img/flow-*.svg（純 Python，改圖內容改此檔）
python scripts/ops/build-op-guide-pdf.py          # md -> pdf（需 markdown 模組 + Chrome/Edge）
```

`img/` 內的 png 截圖複製自 `docs/manual/screenshots/`（模擬程式擷取，與實機 UI 相同）。

## 依據

- 程式現況 `HT160S_Program_BCB_V1.0.0.0`（2026-09-15 `feat/amr-lot-identity-d4`）
- 工程手冊 `docs/manual/03-panel-startup.md`、`04-main-screen.md`、`12-secs-amr.md`、`13-alarms.md`、`14-module-flows.md`、`16-faq.md`
- 京元現場設定 `system/General.ini`（`[AGV] AmrFeedWaitSec=60 / AgvTimeoutSec=300 / LoaderCallsAmr=0 / ErrorLaneCallsAmr=1`）
