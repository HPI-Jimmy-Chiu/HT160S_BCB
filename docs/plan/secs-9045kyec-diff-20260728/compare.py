# -*- coding: utf-8 -*-
"""Compare HT9045(KYEC) vs HT160S SECS command surface -> markdown tables."""
import json, os, re, io

HERE = os.path.dirname(os.path.abspath(__file__))
surf = json.load(open(os.path.join(HERE, "kyec9045_surface.json"), encoding='utf-8'))

# ---------------- 9045 CEID catalog (firmware .def at KYEC) ----------------
DEF = r"D:\backup_version\HT9046\KYEC\20260626\EventReport_CEID.def"
ceid9045 = {}
with open(DEF, encoding='utf-8', errors='replace') as f:
    for i, ln in enumerate(f):
        if i == 0:
            continue
        p = ln.rstrip("\n").split("\t")
        if len(p) < 3 or not p[0].strip().isdigit():
            continue
        cid = int(p[0])
        name = p[2].strip()
        name = re.sub(r'^%d\s*' % cid, '', name).strip()
        if name:
            ceid9045[cid] = name

# ---------------- HT160 CEID catalog (from source) ----------------
ceid160 = {
 1:"Handler change status", 2:"Recipe Change", 3:"Press Clear Count button",
 4:"Press Start button without IC", 5:"Press Start button with IC",
 6:"Press Pause button", 7:"Press Home button", 8:"Press One Cycle button",
 9:"Press Clean Out button", 10:"Press Tray Feed button", 11:"Press Lot Start button",
 12:"Press Lot End button", 13:"Press Exit button", 14:"Press Retry button",
 15:"Press Skip button", 16:"Press Alarm Reset button", 17:"Show Alarm",
 18:"Release Alarm", 19:"Show Message", 20:"Release Message", 21:"Switching User Level",
 22:"Enter Setup Page", 23:"Enter Maintenance Page", 24:"Enter I/O Page",
 25:"Enter Teach Page", 26:"Enter SECS GEM Page", 27:"One Cycle Finish",
 28:"Clean Out Finish", 29:"Tray Feed Finish", 30:"Time Event", 31:"Switching Real/Dummy Mode",
 35:"Auto1 Full", 36:"Auto2 Full", 37:"Auto3 Full",
 148:"Auto4 Full", 149:"Auto5 Full", 150:"Auto6 Full",
 272:"AGVSupplement", 273:"AGVLDUnLDStatus", 274:"AGVLDUnLDFinish", 275:"AGVLdID",
}
# fired but NOT registered in AddCEID (aAuto1To6.cpp:742 AutoCeid[])
ceid160_unreg = {136:"Auto1 Unloadtray", 137:"Auto2 Unloadtray", 138:"Auto3 Unloadtray",
                 140:"Auto4 Unloadtray", 141:"Auto5 Unloadtray", 142:"Auto6 Unloadtray"}

# ---------------- messages ----------------
# inbound primaries each side HANDLES with a real reply
m9045 = """1F1 1F3 1F11 1F13 1F15 1F17 1F23 2F13 2F15 2F17 2F23 2F25 2F29 2F31 2F33 2F35 2F37
2F41 2F43 5F3 5F5 5F7 6F15 6F17 6F19 6F23 7F1 7F3 7F5 7F17 7F19 10F3 10F5 14F3 100F3
101F1 101F3 101F5 101F7 101F11 103F11 110F2 110F6 110F8 120F2 125F1 125F3""".split()
m160_real = """1F1 1F3 1F11 1F13 1F15 1F17 2F13 2F15 2F17 2F25 2F31 2F33 2F35 2F37 2F41
5F3 5F5 5F7""".split()
m160_stub = "7F1 7F3 7F5 7F17 7F19 10F3 10F5 14F1".split()   # dispatched, log-only, NO reply

def key(x):
    s, f = x.split('F')
    return (int(s), int(f))

obs = set()
for k in surf['messages']:
    s, f = k[1:].split('F')
    if int(f) % 2 == 1:                      # primaries only
        obs.add("%sF%s" % (s, f))

s9, s160r, s160s = set(m9045), set(m160_real), set(m160_stub)
s160all = s160r | s160s

out = io.StringIO()
W = out.write

def tbl(title, rows, hdr):
    W("\n#### %s\n\n" % title)
    W("| " + " | ".join(hdr) + " |\n")
    W("|" + "|".join(["---"] * len(hdr)) + "|\n")
    for r in rows:
        W("| " + " | ".join(str(x) for x in r) + " |\n")

W("## A. Message layer (SxFy primaries)\n")
both = sorted(s9 & s160r, key=key)
only9 = sorted(s9 - s160all, key=key)
only160 = sorted(s160all - s9, key=key)
stub_in9 = sorted(s9 & s160s, key=key)
W("\n- BOTH (both really reply): %s\n" % " ".join(both))
W("- 9045 only (HT160 no dispatch -> log-only S9F3, host T3 timeout): %s\n" % " ".join(only9))
W("- 9045 has, HT160 dispatches but STUB (no reply -> T3 timeout): %s\n" % " ".join(stub_in9))
W("- HT160 only: %s\n" % " ".join(only160))
W("\nKYEC host actually sent in this log: %s\n" % " ".join(sorted(obs, key=key)))
gap_hot = sorted((set(obs) & (s9 - s160r)), key=key)
W("\n**HOT gap (host really sent it AND HT160 would not answer): %s**\n" % " ".join(gap_hot))

# ---------------- RCMD ----------------
r9045 = ["PAUSE","STOP","ONE_CYCLE","RESET","CONTINUE_RETEST_ART","CONTINUE_START_ART",
 "CLEAN_AUTO_SORT_COUNT","RETEST_MRT","INITIAL_START_MRT","CONTINUE_START_MRT","REMOTE_SAVE",
 "AUTOSITEMAP","AUTO_RETEST","TRAY_FEED","INITIAL_START_ART","INITIAL_START",
 "DOWNLOAD_RECIPE_BY_FTP","CLEAN_OUT","SWITCH_TO_FT","SWITCH_TO_RT","ONLINE_LOCAL",
 "ONLINE_REMOTE","START_LOT","START_AQL","START_AGV","REMOTE_START","START","HALT","PP_MUSIC",
 "PP_SIGNALTOWER","AUTO_CLEAN","PP_PASSWORD","PP_SELECT","LOTSTART","AUTHORITY_CHECK",
 "SET_LOT_INFO","DEVTEMPOFFSETADJUST","CLEAR_LOT_INFO","LOTORDER","TRAY_MAP","STOP_LOT",
 "EESUG_OFFSET","CLOSE_ONECYCLE","TESTTEMPSETTING","ENERGY_SAVING","YIELD_FAIL",
 "REMOTE_UPDATE_PROGRAM"]
r160 = ["SET_LOT_INFO","PAUSE","CLEARCOUNT","ONLINE_REMOTE","ONLINE","ONLINE_LOCAL",
        "LOTSTART","START","START_AGV","STOP","HOME"]
obs_rcmd = set(surf['rcmds'].keys())
a, b = set(r9045), set(r160)
W("\n\n## B. Remote command layer (S2F41 RCMD)\n")
W("\n- BOTH (%d): %s\n" % (len(a & b), " ".join(sorted(a & b))))
W("- HT160 only (%d): %s\n" % (len(b - a), " ".join(sorted(b - a))))
W("- 9045 only (%d): %s\n" % (len(a - b), " ".join(sorted(a - b))))
W("\nKYEC host actually sent (%d): %s\n" % (len(obs_rcmd), " ".join(sorted(obs_rcmd))))
W("\n**HOT gap (host really sent it, HT160 has no branch): %s**\n" % " ".join(sorted(obs_rcmd - b)))

# ---------------- CEID ----------------
W("\n\n## C. Event layer (CEID)\n")
c9, c160 = set(ceid9045), set(ceid160) | set(ceid160_unreg)
same_num_same_meaning = []
same_num_diff_meaning = []
# hand-audited overrides: the token heuristic gets these wrong in both directions
FORCE_ALIGNED  = {272, 273, 274, 275}   # AMR* vs AGV* = same semantics, naming only
FORCE_CONFLICT = {22, 23}               # "Enter <X> Page" shares tokens but X differs
for c in sorted(c9 & c160):
    n9 = ceid9045[c]
    n1 = ceid160.get(c) or ceid160_unreg.get(c)
    if c in FORCE_ALIGNED:
        hit = True
    elif c in FORCE_CONFLICT:
        hit = False
    else:
        t9 = re.sub(r'[^a-z0-9]', '', n9.lower())
        t1 = re.sub(r'[^a-z0-9]', '', n1.lower())
        hit = (t9 in t1) or (t1 in t9)
        if not hit:
            toks9 = set(re.findall(r'[a-z]+|\d+', n9.lower()))
            toks1 = set(re.findall(r'[a-z]+|\d+', n1.lower()))
            hit = len(toks9 & toks1) >= 2
    (same_num_same_meaning if hit else same_num_diff_meaning).append((c, n9, n1))

W("\n- 9045 firmware CEID catalog (EventReport_CEID.def): %d defined\n" % len(c9))
W("- HT160 CEID: %d registered + %d fired-but-unregistered = %d\n"
  % (len(ceid160), len(ceid160_unreg), len(c160)))
W("- Numbers present on BOTH sides: %d (aligned meaning %d / CONFLICTING meaning %d)\n"
  % (len(c9 & c160), len(same_num_same_meaning), len(same_num_diff_meaning)))
W("- Numbers only in HT160: %s\n" % (" ".join(str(x) for x in sorted(c160 - c9)) or "(none)"))
W("- Numbers only in 9045: %d (see report)\n" % len(c9 - c160))

tbl("C-1. Same number, ALIGNED meaning (safe)",
    [(c, n9, n1) for c, n9, n1 in same_num_same_meaning], ["CEID", "HT9045", "HT160S"])
tbl("C-2. Same number, DIFFERENT meaning (host will misread)",
    [(c, n9, n1) for c, n9, n1 in same_num_diff_meaning], ["CEID", "HT9045", "HT160S"])

sent = surf['ceids_sent_s6f11']
dis = surf['ceids_disabled_abort']
live = sorted(set(int(k) for k in sent) | set(int(k) for k in dis))
rows = []
for c in live:
    rows.append((c, ceid9045.get(c, "?"), sent.get(str(c), 0), dis.get(str(c), 0),
                 "YES" if c in c160 else "no",
                 (ceid160.get(c) or ceid160_unreg.get(c) or "-")))
tbl("C-3. CEIDs the 9045 actually FIRED at KYEC on 2026-06-08 vs HT160 coverage",
    rows, ["CEID", "HT9045 meaning", "S6F11 sent", "aborted(disabled)", "HT160 has #", "HT160 meaning of that #"])

json.dump({
 'messages': {'both': both, 'only9045': only9, 'stub160': stub_in9, 'only160': only160,
              'observed': sorted(obs, key=key), 'hot_gap': gap_hot},
 'rcmd': {'both': sorted(a & b), 'only160': sorted(b - a), 'only9045': sorted(a - b),
          'observed': sorted(obs_rcmd), 'hot_gap': sorted(obs_rcmd - b)},
 'ceid': {'aligned': same_num_same_meaning, 'conflict': same_num_diff_meaning,
          'only160': sorted(c160 - c9), 'only9045': sorted(c9 - c160)},
}, open(os.path.join(HERE, "compare_result.json"), 'w', encoding='utf-8'),
   ensure_ascii=False, indent=1)

print(out.getvalue())
open(os.path.join(HERE, "compare_tables.md"), 'w', encoding='utf-8').write(out.getvalue())
