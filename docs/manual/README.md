# HT160S 操作手冊（工程版）— 索引

本目錄為 HT160S 操作手冊的 Markdown 主檔。各章內容由 workflow 自原始碼萃取後撰寫，需經人工校對後再匯出 Word 正式交付。

## 章節

| 章 | 標題 | 檔案 |
| --- | --- | --- |
| 封面 | 文件資訊與慣例 | [00-cover.md](00-cover.md) |
| 01 | 安全須知 | [01-safety.md](01-safety.md) |
| 02 | 系統概觀與機構 | [02-overview.md](02-overview.md) |
| 03 | 操作面板與開機啟動 | [03-panel-startup.md](03-panel-startup.md) |
| 04 | 主畫面詳解 | [04-main-screen.md](04-main-screen.md) |
| 05 | 維護畫面 (Maintenance) | [05-maintenance.md](05-maintenance.md) |
| 06 | 設定 (Config / Setup) | [06-config.md](06-config.md) |
| 07 | 教導 (Teach) | [07-teach.md](07-teach.md) |
| 08 | 偏移 (Offset) | [08-offset.md](08-offset.md) |
| 09 | 速度 (Speed) | [09-speed.md](09-speed.md) |
| 10 | 輸出入監看 (I/O) | [10-io.md](10-io.md) |
| 11 | 馬達測試 (Motor Test) | [11-motor-test.md](11-motor-test.md) |
| 12 | SECS/GEM 與 AMR/AGV | [12-secs-amr.md](12-secs-amr.md) |
| 13 | 警報訊息與排除 | [13-alarms.md](13-alarms.md) |
| 14 | 各模組運作流程 | [14-module-flows.md](14-module-flows.md) |
| 15 | By Lot+Bin 分流模式 | [15-lotbin-mode.md](15-lotbin-mode.md) |
| 16 | 常見問題 (FAQ) | [16-faq.md](16-faq.md) |

## 附錄（自機台設定檔產生，用於補齊各章「待補」）

| 附錄 | 標題 | 檔案 | 來源 |
| --- | --- | --- | --- |
| A | 現場驗證清單（119 項待補彙整） | [A1-field-checklist.md](A1-field-checklist.md) | 各章 unknowns |
| B | 全機 I/O 對照表（327 點） | [B1-io-table.md](B1-io-table.md) | `system/IO_Table.csv` |
| C | 軸（馬達）對照表（20 軸） | [C1-motor-table.md](C1-motor-table.md) | `system/Mot_Table.csv` |
| D | 警報碼一覽（516 筆） | [D1-alarm-list.md](D1-alarm-list.md) | `system/AlarmList.csv` |

> 編輯審查意見見 [_review-notes.md](_review-notes.md)（workflow 完整性審查自動產生，含補完優先序建議）。

## 如何重新產生本手冊

```
# 1. 產生章節 Markdown（背景執行，完成後寫入本目錄）
Workflow({ name: "ht160s-operation-manual" })

# 2. 擷取畫面截圖（驅動模擬程式 GUI，約 2 分鐘）
powershell -ExecutionPolicy Bypass -File scripts\ops\capture-ht160s-screens.ps1

# 3. 匯出 Word（使用 docx skill）
```

- Workflow 腳本：[.claude/workflows/ht160s-operation-manual.js](../../.claude/workflows/ht160s-operation-manual.js)
- 截圖腳本：[scripts/ops/capture-ht160s-screens.ps1](../../scripts/ops/capture-ht160s-screens.ps1)
- 參考手冊結構：HT9045/9046/HT1032AT 操作手冊（第 17 版）

## 校對狀態

每章開頭如有 `> 【待補：...】` 標記，表示該處需現場確認。完成校對後請移除標記並更新封面版本。
