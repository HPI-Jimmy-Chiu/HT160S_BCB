# ADVERSARIAL RE-VERIFICATION - HT160S ALID Option D (Amendments 1, 1b, A5, 2)
Date 2026-09-02. Every number below was recomputed in this agent's own python from
first principles. Harness in this directory:
  cpp_xlit.py  line-by-line transliteration of the PROPOSED BCB6 ComputeAlarmAlid
               (change file lines 48-212), byte-level, BCB6 signed-char semantics
  u.py         AlarmList.csv universe + registered encode        -> stdout
  u3.py        the 532-row produced CSV re-derived                -> u3.out
  d9045.py     9046LS recompute + AlarmData.def cross-check + full intersection -> d9045.out
  attack.py    boundary / brute force / injectivity / collision hunt -> attack.out
  spot.py      49 spot checks (28 doc claims + 21 CSV stride rows)  -> spot.out

## MY NUMBERS (all recomputed, none taken from the prior runs)
system/AlarmList.csv (WORKTREE): 486 lines / 485 data rows, md5 73e76f289844430582165095758c5126, 0 dup codes.
  HEAD copy: 485 lines / 484 rows - WAR0963 ABSENT. It is a boot artifact committed as source.
  WAR0963 IS registered by COMMITTED source (database.cpp:1023), so 485 is right for shipping firmware.
REGISTERED 485: 485 distinct ALIDs, 0 collisions, 485/485 exactly 9 digits, min 100000913 (JAM0913),
  max 600060005 (60005), classes {1:14,2:15,3:36,4:234,5:180,6:6}, round-trip 485/485,
  0 above 2^31-1, 485/485 changed vs legacy. AlarmType census: JAM=0 x14, MES=1 x36, WAR=1 x2, WAR=8 x13.
FULL 532 (485 + 47 free strings): 532 distinct, 0 collisions, min 100000913, max 999752848,
  532/532 9 digits, changed 530 / unchanged 2, old >2^31-1 = 46, old max 3992930430 (a class-9 string),
  old max over the 485 catalog = 3184282107 (JAM1611). Class-9 payload span 1441895..99752848.
PREFIXED: 65 codes, tail lengths {4:63, 5:2}; 5+ digit tails with a leading zero = 0;
  rejected by 1b = 0; the 11 leading-zero 4-digit tails match the ratified list exactly.
5-CHAR NUMERICS: first-char {4:234, 5:180, 6:6}; starting 7 or 8 = 0.
C++ vs TABLE: 532/532 agree. 1 apparent mismatch was my own cp950 re-encode of the Big5 row;
  encoding those bytes as latin-1 (= the real Big5 bytes) gives 923646915 = the table. 0 real mismatches.
CLASSES 1..8 FULL ACCEPTED SPACE: 350,000 strings -> 350,000 distinct ALIDs. INJECTIVE.
  Round-trip failures 0. Class census {1:100000,2:100000,3:100000,4:10000,5:10000,6:10000,7:10000,8:10000}.
NON-CANONICAL SWEEP: 36,000 strings ("0dddd", 3-digit, 6-digit tails) -> 100% class 9;
  0 of them land on a canonical class-1..8 ALID.
9046LS: AlarmCodeList.txt 2,738 non-blank lines, 2,738 distinct keys (JAM 596 / WAR 1846 / MES 295 / "41").
  Recompute vs AlarmData.def field 9 (0-indexed col 8): 0 MISMATCHES of 2,738, 0 one-sided.
  2,737 nine-digit, leading digits {1:596, 2:1846, 3:295}, min 101000109, max 330003021,
  payload span 1000101..31031293, payload<1e6 = 0, in [7e8,9e8) = 0.
INTERSECTION (all 350,000 reachable classes 1..8) x 9046LS = 0 (exhaustive, not sampled).
  Class 9 band [9e8,1e9) vs 9046LS max 330003021 -> structurally 0.
  PESSIMISTIC variant (cMyDB "41" row typed 3 = 300000000) -> intersection = 1 (HT160S MES0000, not in service).
RANDOM SWEEP 400,000 arbitrary 1..60-char strings: all exactly 9 digits, 0 zero-values,
  0 land in 9046LS, 3,515 class-9 collisions (expected; Az/B[ -> 900002137 confirmed).
SPOT CHECKS: 28 letter/decode-spec claims + 21 CSV stride rows = 49 checks, 0 failures.
EXTERNAL CROSS-CHECK of the class-9 arithmetic against a REAL 2026-06-26 wire capture:
  LegacyAlarmHash("Loader Tray Empty") = 4045923824 and ("SnFKCleanOut") = 3891410149,
  matching docs/SECS/HT160S_SECS_Comm_Examples.md:785-786 byte-for-byte.
WORKBOOK 0831 (md5 e0ff9e2c182dc6521fd706a2db00abda, Excel lock file PRESENT):
  ALID sheet 481 code rows at 2..482, blank 483, footnotes A484/A485, merged A484:E484,
  freeze_panes = A269 (stray), widths A14/B18/C8/D16/E86, 5 cols.
  Missing exactly 4: MES1025, MES1428, MES1921, WAR0963. Extras: 0.
  Literal hits - A484 {雜湊:3, 無號:1, 2^31:1, 3184282107:1, 2054803979:1, 2502907625:1, 481:2};
  A485 {481:2, 3891410149:1}; 功能E16 {雜湊:5, 無號:1, 2^31:1, 3184282107:1, 2054803979:1, 481:1};
  功能E18 {481:6}; 功能E31 {481:1}; 功能E17 no hits. All match the DOCS plan exactly.
GENERATOR: --selftest 0 failures; --audit 485 rows / 0 violations / 447 ALTX>40 (max 82);
  --verify-table vs the p3 CSV = 485 compared, 0 mismatches, 0 one-sided. Exit 0.
MachineType.h:7 worktree = "//#define SOFT_SIMULATE" (REAL-MACHINE), HEAD = "#define SOFT_SIMULATE".
