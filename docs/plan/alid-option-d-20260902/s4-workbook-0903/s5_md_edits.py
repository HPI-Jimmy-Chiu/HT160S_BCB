# -*- coding: utf-8 -*-
# S5: update the two SECS manuals (.md are the sources of truth; LF, UTF-8, no BOM).
# Rule from secs-comm-examples-doc-maintenance: never forge a log line; annotate the
# 2026-06-26 run as pre-cutover, change only format/behaviour descriptions.
import io, os
os.chdir(r"D:\HT160S_BCB")


def rd(p):
    b = open(p, 'rb').read()
    assert b'\r\n' not in b and b[:3] != b'\xef\xbb\xbf', p
    return b.decode('utf-8')


def wr(p, s):
    open(p, 'wb').write(s.encode('utf-8'))


def rep(s, old, new, tag, cnt=1):
    assert s.count(old) == cnt, (tag, s.count(old), old[:70])
    return s.replace(old, new)


# ============================ Comm_Examples.md ============================
p = 'docs/SECS/HT160S_SECS_Comm_Examples.md'
s = rd(p)

# C1 : the ALID derivation paragraph + example table
old = ('**ALID 推導**：本機的 ALID 是以 alarm-code **字串** 做 31-poly rolling hash 算出的 U4（`alid = alid*31u + (unsigned char)byte`，'
       '`UsecegemMainFrom.cpp:150-152`）。此演算法 stateless／deterministic（同一字串永遠算出同一 ALID）；真正人類可讀的代碼承載於 **ALTX** 欄位。'
       '已驗證（python 重算 byte-for-byte 相符）：\n\n'
       '| ALTX（alarm code 字串） | ALID（U4） |\n|---|---|\n'
       '| `"Loader Tray Empty"` | `4045923824` |\n| `"SnFKCleanOut"` | `3891410149` |\n')
new = ('**ALID 推導（⚠ 自 2026-09-03 韌體 `6377aff` 起改版）**：ALID 為**固定 9 碼的號段式編碼**，'
       '`ALID = 號段 Class × 100,000,000 + 號段內碼 Payload`（`UsecegemMainFrom.cpp` `ComputeAlarmAlid()`）：'
       '號段 1=JAM / 2=WAR / 3=MES（Payload = 前綴後的數字尾，例 `MES1421` → `300001421`）；'
       '4=汽缸 4xxxx / 5=馬達 5xxxx / 6=吸嘴 6xxxx / 7=7xxxx / 8=8xxxx（Payload = 完整 5 碼代碼，例 `40000` → `400040000`）；'
       '9 = 尚未登錄成警報碌的自由字串（**不在 S5F6/S5F8 目錄內**，Payload 由字串導出；host 請讀 ALTX 的 leading token）；0 永不發送。'
       '本機值域 100,000,913 ～ 999,752,848，全部小於 2^31−1，host 可用有號 32 位元整數存放。'
       '解碼：`Class = ALID / 1e8`、`Payload = ALID % 1e8`，class 1/2/3 補零至 4 位（Payload < 10000）或 5 位，class 4～8 補零至 5 位。'
       '完整 486 筆對照（含 Class / Payload 欄）見客戶工作簿 `SECS_GEM功能_Handler_20260903.xlsx` 的「ALID」工作表。\n\n'
       '> ⚠ **本 run（2026-06-26）為舊版韌體**：當時 ALID 是 alarm-code 字串的 hash 值（8 或 10 碼），下方 log 中的 `4045923824` / `3891410149` 即該舊值，'
       '逐字保留、不改寫。同兩支警報在現行韌體的 ALID 如下（`Loader Tray Empty` 之後已登錄為代碼 `MES0920`）：\n\n'
       '| 警報（本 run 的 ALTX） | 本 run ALID（舊版 hash） | 現行 ALID（2026-09-03 起） | 備註 |\n|---|---|---|---|\n'
       '| `"Loader Tray Empty"` | `4045923824` | `300000920` | 已登錄為 `MES0920`，號段 3；現行 ALTX = `"MES0920 Loader Tray Empty"` |\n'
       '| `"SnFKCleanOut"` | `3891410149` | `991410149` | 自由字串，號段 9，不在目錄；ALTX = `"SnFKCleanOut SnFKCleanOut"` |\n').replace('警報碌', '警報碼')
s = rep(s, old, new, 'C1')

# C2 : the "why a hash" rationale block
old = ('> **為什麼 ALID 用字串 hash、而不是固定編號？host 該以什麼為準？** 因為 ALID 由字串確定性地算出，可避免維護一張人工編號表；'
       '但這也代表 **ALID 本身對 host 不直觀**。整合時建議 host 端 **以 ALTX 的 leading code token 作為警報的辨識依據**（ALID 用於去重／配對 SET↔CLEAR 即可），'
       '因為 code token 才是人類可讀、語意明確的代碼。')
new = ('> **host 該以什麼為準？（2026-09-03 更新）** 現行 ALID 可直接反解回警報碼（號段 1～8），host 端建議 **以 ALID 對工作簿「ALID」表查表、或直接依上式解碼**；'
       '只有號段 9（未登錄的自由字串警報）不在目錄內，該類請 **以 ALTX 的 leading code token 辨識**。舊版「ALID 為字串 hash、對 host 不直觀」的取捨自本版起不再適用。')
s = rep(s, old, new, 'C2')

# C3 : the ALTX-scope callout still cites old line numbers
s = rep(s, '實際 handler（`UsecegemMainFrom.cpp:154-156`）在', '實際 handler（`UsecegemMainFrom.cpp` `AlarmReport()`）在', 'C3')

# C4 : annotation under the verbatim equipment log block
old = ('00:01:11.861  [SECS][TX] S5F1 Alarm ALID=3891410149 ALCD=0     (CLEAR "SnFKCleanOut")\n```\n\n**實際 case log 節錄（Host 端 Simulator）**：')
new = ('00:01:11.861  [SECS][TX] S5F1 Alarm ALID=3891410149 ALCD=0     (CLEAR "SnFKCleanOut")\n```\n\n'
       '> ⚠ 上列 ALID 為 2026-06-26 舊版韌體的 hash 值。現行韌體同兩支警報分別送 `ALID=300000920`（`MES0920 Loader Tray Empty`）與 `ALID=991410149`（`SnFKCleanOut`，號段 9）。\n\n'
       '**實際 case log 節錄（Host 端 Simulator）**：')
s = rep(s, old, new, 'C4')

# C5 : FIELD TABLE row
old = '| `ALID` | U4 | Alarm ID，由 ALTX 字串 31-poly hash 而來，deterministic | `4045923824`（Loader Tray Empty）／`3891410149`（SnFKCleanOut） |'
new = ('| `ALID` | U4 | Alarm ID。**2026-09-03 起為 9 碼號段式**（`Class×1e8 + Payload`，見上方「ALID 推導」）；本 run 的值為舊版 hash | '
       '本 run：`4045923824`（Loader Tray Empty）／`3891410149`（SnFKCleanOut）；現行：`300000920` ／ `991410149` |')
s = rep(s, old, new, 'C5')

# C6 : timeline rows
s = rep(s, '| 00:01:00 | S5F1 SET | Loader Tray Empty，ALID=4045923824，ALCD=128 |',
        '| 00:01:00 | S5F1 SET | Loader Tray Empty，ALID=4045923824（舊版 hash；現行 `MES0920` = 300000920），ALCD=128 |', 'C6a')
s = rep(s, '| 00:01:05 | S5F1 SET | SnFKCleanOut，ALID=3891410149，ALCD=128 |',
        '| 00:01:05 | S5F1 SET | SnFKCleanOut，ALID=3891410149（舊版 hash；現行 991410149，號段 9），ALCD=128 |', 'C6b')

# C7 : glossary
s = rep(s, '| ALID | Alarm ID；本機由 ALTX 字串 31-poly hash 而來的 U4 |',
        '| ALID | Alarm ID；U4。2026-09-03 起為 9 碼號段式 `Class×100,000,000 + Payload`（號段 1=JAM 2=WAR 3=MES 4～8=數字碼家族 9=未登錄自由字串），可反解回警報碼；本文 2026-06-26 run 中的值為舊版 hash |', 'C7')
wr(p, s)
print('Comm_Examples.md updated')

# ============================ Interface_Spec.md ============================
p = 'docs/SECS/HT160S_SECS_Interface_Spec_20260727.md'
s = rd(p)

# I1 : header block
s = rep(s, '> **文件版本 Doc rev:** 2026-09-01', '> **文件版本 Doc rev:** 2026-09-03', 'I1a')
s = rep(s, '> ⚠️ **文件地位 / Document status (2026-09-01)**:對客戶交付的**單一權威文件是\n> `docs/SECS/SECS_GEM功能_Handler_20260831.xlsx`**',
        '> ⚠️ **文件地位 / Document status (2026-09-03)**:對客戶交付的**單一權威文件是\n> `docs/SECS/SECS_GEM功能_Handler_20260903.xlsx`**', 'I1b')
s = rep(s, '> (38237–38239 改為家族定義的 *Record Auto 1/2/3 Tray Count*)。補正處以「**⚠ 更正**」標示。',
        '> (38237–38239 改為家族定義的 *Record Auto 1/2/3 Tray Count*)。補正處以「**⚠ 更正**」標示。\n'
        '> 2026-09-03 追加第三次改號的補正:**(3) S5 ALID 由警報碼字串 hash 改為固定 9 碼號段式**(韌體 `6377aff`,目錄 486 筆),見 §3.5。', 'I1c')

# I2 : S5F1 row notes
s = rep(s, '| S5F1 | Alarm Report Send | E→H | (S5F2) | `L,3{ B ALCD, U4 ALID, A ALTX }` | ALCD bit7:0x80=set/0x00=clear |',
        '| S5F1 | Alarm Report Send | E→H | (S5F2) | `L,3{ B ALCD, U4 ALID, A ALTX }` | ALCD bit7:0x80=set/0x00=clear;ALID = 9 碼號段式(§3.5) |', 'I2')
s = rep(s, '| S5F6 | List Alarm Data | E→H | — | `L,n{ L,3{ B ALCD, U4 ALID, A ALTX } }` | 目錄由 AlarmList.csv/SSOT 即時產生 |',
        '| S5F6 | List Alarm Data | E→H | — | `L,n{ L,3{ B ALCD, U4 ALID, A ALTX } }` | 目錄由警報碼 SSOT 即時產生,486 筆(§3.5) |', 'I2b')

# I3 : the wrong 66011 sentence
s = rep(s, '| 66011 | Alarm Code | **無替代**,同上——警報碼即 S5F1 的 **ALID** |',
        '| 66011 | Alarm Code | **無替代**,同上——警報碼由 S5F1 的 **ALID** 反解取得(9 碼號段式,§3.5;號段 9 者改讀 ALTX 的第一個 token) |', 'I3')

# I4 : section 3.5
old = ('### 3.5 警報 / Alarms (ALID) — S5F1, S5F5/F6\n\n'
       '- ALID 目錄由機台警報 SSOT(`system\\AlarmList.csv` / `mapAlarmCodeList`)**即時**產生,約 480+ 碼。\n'
       '- 欄位:`AlarmCode, AlarmType, E_ErrMessage, C_ErrMessage, E_Description, C_Description`(中英雙語)。\n'
       '- S5F1 ALTX = `<code> <English message>`;ALCD bit7 = 0x80(set)/0x00(clear)。\n'
       '- **完整警報清單請見隨附的 `system\\AlarmList.csv`**(過長,不在本規格內展開)。\n')
new = ('### 3.5 警報 / Alarms (ALID) — S5F1, S5F5/F6, S5F7/F8\n\n'
       '- ALID 目錄由機台警報 SSOT(`mapAlarmCodeList`,開機時傾印為 `system\\AlarmList.csv`)**即時**產生,**486 碼**;S5F5/S5F7 的 ALID 過濾清單未實作,一律回完整目錄;本機無 per-ALID 啟用表,S5F8 恆等於 S5F6。\n'
       '- **⚠ 更正(2026-09-03,韌體 `6377aff`):ALID 改為固定 9 碼的號段式編碼**,不再是警報碼字串的 hash:\n'
       '    - `ALID = 號段 Class × 100,000,000 + 號段內碼 Payload`;號段 1=JAM、2=WAR、3=MES(Payload = 前綴後的數字尾,例 `MES1421` → `300001421`、`JAM0913` → `100000913`、`WAR16120` → `200016120`);'
       '4=汽缸 4xxxx、5=馬達 5xxxx、6=吸嘴 6xxxx、7=7xxxx、8=8xxxx(Payload = 完整 5 碼代碼,例 `40000` → `400040000`);9 = 尚未登錄成警報碼的自由字串警報(**不在目錄**,Payload 由字串導出);0 永不發送。\n'
       '    - 解碼:`Class = ALID / 100,000,000`、`Payload = ALID % 100,000,000`;class 1/2/3 → 前綴 + Payload 補零至 4 位(Payload < 10000)或 5 位;class 4～8 → Payload 補零至 5 位;class 9 → 讀 ALTX 的第一個 token。\n'
       '    - 本機值域 100,000,913 ～ 999,752,848,恆 9 碼且全部小於 2,147,483,647 → host 可用有號 32 位元整數存放。與 HT-9046LS 的 9 碼 ALID 空間零交集(9046LS 首位 1/2/3 且第 2-3 位為單元編號 ≥ 01;本機號段 1/2/3 第 2-3 位恆為 00)。\n'
       '    - S5F1 與 S5F6/S5F8 由同一函式產生 ALID,同一支警報兩處恆等。全部 486 筆的數值皆與 2026-09-03 前不同,**韌體與 host 字典須在同一維護窗口整批切換**;歷史 log 的舊值請用新舊對照表換算,勿以新規則反推。\n'
       '- 欄位:`AlarmCode, AlarmType, E_ErrMessage, C_ErrMessage, E_Description, C_Description`(中英雙語)。\n'
       '- ALTX:S5F6/S5F8 目錄 = `<code> <English message>`;S5F1 事件 = `<code> <當下介面語言的訊息>`(中文介面為 Big5/CP950 原始位元組,請勿以 UTF-8 解碼);ALCD bit7 = 0x80(set)/0x00(clear),**S5F1 的 ALCD 低 7 位恆為 0(不帶類別)**,分類請以 ALID 查表或取號段。\n'
       '- 號段 9 的兩類例外:吸嘴碼族 `SUC<3 位索引><1 位錯誤別>` 12 個、以及安全門/緊急停止/馬達電源/離子風扇/氣壓/各軸 `_MotOverLimitErr`/分選臂與 TrayArm 英文長句等自由字串警報(其 ALTX 為同一字串重複兩次);host 請預留「號段 9 = 未知 ALID,顯示 ALTX」路徑。本方後續登錄這些警報成正式代碼時,其 ALID 會再變一次。\n'
       '- **完整 486 筆對照請見工作簿 `SECS_GEM功能_Handler_20260903.xlsx` 的「ALID」工作表**(A–E 欄同 0831 版順序,F/G 欄為 Class / Payload);`system\\AlarmList.csv` 不含 ALID 欄。\n')
s = rep(s, old, new, 'I4')
wr(p, s)
print('Interface_Spec.md updated')
