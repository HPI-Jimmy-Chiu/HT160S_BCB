# Parse the HT160S SECS text log: rebuild every RX/TX SML body into a tree,
# then extract the host's S2F33 report definitions and S2F35 event links.
import re
import sys
import os
import json
from collections import OrderedDict

LOG = sys.argv[1]
OUT = sys.argv[2]
os.makedirs(OUT, exist_ok=True)

lines = open(LOG, encoding="utf-8", errors="replace").read().splitlines()

HDR = re.compile(r'^(\S+ \S+)\s+\[SECS\]\[(RX|TX)\] (S\d+F\d+) W=(\d+)(?: len=\d+ \(sent\))? body:\s*$')
OPEN = re.compile(r'^(\s*)<([A-Z]+)\[(\d+)\]\s*$')
LEAF = re.compile(r'^(\s*)<([A-Z0-9]+)\[(\d+)\]\s+(.*)>\s*$')
CLOSE = re.compile(r'^(\s*)>\s*$')


def parse_body(buf):
    """Indentation/stack parse of the pretty-printed SML block."""
    root = []
    stack = [root]
    for ln in buf:
        if not ln.strip():
            continue
        m = OPEN.match(ln)
        if m:
            node = {"t": m.group(2), "n": int(m.group(3)), "c": []}
            stack[-1].append(node)
            stack.append(node["c"])
            continue
        m = LEAF.match(ln)
        if m:
            stack[-1].append({"t": m.group(2), "n": int(m.group(3)), "v": m.group(4).strip()})
            continue
        if CLOSE.match(ln):
            if len(stack) > 1:
                stack.pop()
            continue
    return root


msgs = []
i = 0
while i < len(lines):
    m = HDR.match(lines[i])
    if not m:
        i += 1
        continue
    ts, dirn, kind, wbit = m.group(1), m.group(2), m.group(3), m.group(4)
    buf = []
    j = i + 1
    while j < len(lines) and lines[j].strip() != "":
        if HDR.match(lines[j]):
            break
        buf.append(lines[j])
        j += 1
    msgs.append({"ts": ts, "dir": dirn, "kind": kind, "w": wbit, "body": parse_body(buf), "raw": "\n".join(buf)})
    i = j

print("parsed messages: %d" % len(msgs))
kinds = OrderedDict()
for m in msgs:
    k = m["dir"] + " " + m["kind"]
    kinds[k] = kinds.get(k, 0) + 1
for k, v in sorted(kinds.items(), key=lambda kv: -kv[1]):
    print("  %-12s %d" % (k, v))


def nums(node_list):
    out = []
    for n in node_list:
        if "v" in n:
            try:
                out.append(int(n["v"]))
            except ValueError:
                out.append(n["v"])
        else:
            out.extend(nums(n["c"]))
    return out


# ---- S2F33 : L,2 { DATAID , L,a { L,2 { RPTID , L,b { SVID.. } } .. } }
reports = OrderedDict()
report_hist = []
for m in msgs:
    if m["kind"] != "S2F33" or m["dir"] != "RX":
        continue
    top = m["body"][0] if m["body"] else None
    if not top or "c" not in top or len(top["c"]) < 2:
        continue
    lst = top["c"][1]
    if "c" not in lst:
        continue
    for ent in lst["c"]:
        if "c" not in ent or len(ent["c"]) < 2:
            continue
        rid = int(ent["c"][0]["v"])
        svids = nums(ent["c"][1].get("c", []))
        reports[rid] = svids
        report_hist.append((m["ts"], rid, svids))

# ---- S2F35 : L,2 { DATAID , L,a { L,2 { CEID , L,b { RPTID.. } } .. } }
links = OrderedDict()
link_hist = []
for m in msgs:
    if m["kind"] != "S2F35" or m["dir"] != "RX":
        continue
    top = m["body"][0] if m["body"] else None
    if not top or "c" not in top or len(top["c"]) < 2:
        continue
    lst = top["c"][1]
    if "c" not in lst:
        continue
    for ent in lst["c"]:
        if "c" not in ent or len(ent["c"]) < 2:
            continue
        cid = int(ent["c"][0]["v"])
        rids = nums(ent["c"][1].get("c", []))
        links[cid] = rids
        link_hist.append((m["ts"], cid, rids))

print("\nreports defined: %d" % len(reports))
print("event links defined: %d" % len(links))

with open(os.path.join(OUT, "host_reports_S2F33.csv"), "w", encoding="utf-8") as f:
    f.write("RPTID,SVIDCount,SVIDs\n")
    for rid in sorted(reports):
        f.write('%d,%d,"%s"\n' % (rid, len(reports[rid]), " ".join(str(x) for x in reports[rid])))

with open(os.path.join(OUT, "host_links_S2F35.csv"), "w", encoding="utf-8") as f:
    f.write("CEID,RPTIDCount,RPTIDs\n")
    for cid in sorted(links):
        f.write('%d,%d,"%s"\n' % (cid, len(links[cid]), " ".join(str(x) for x in links[cid])))

allsv = sorted(set(x for v in reports.values() for x in v if isinstance(x, int)))
with open(os.path.join(OUT, "host_svids.txt"), "w", encoding="utf-8") as f:
    f.write("\n".join(str(x) for x in allsv))
print("distinct SVIDs referenced by host reports: %d" % len(allsv))

# S1F3 polls: what did the host ask for one-off, and what did we answer
s1f3 = []
for idx, m in enumerate(msgs):
    if m["kind"] == "S1F3" and m["dir"] == "RX":
        asked = nums(m["body"])
        ans = None
        for k in range(idx + 1, min(idx + 4, len(msgs))):
            if msgs[k]["kind"] == "S1F4" and msgs[k]["dir"] == "TX":
                ans = msgs[k]["raw"].strip()
                break
        s1f3.append({"ts": m["ts"], "svids": asked, "reply": ans})
with open(os.path.join(OUT, "s1f3_polls.json"), "w", encoding="utf-8") as f:
    json.dump(s1f3, f, indent=2, ensure_ascii=False)
print("S1F3 polls: %d" % len(s1f3))

# every distinct RX kind with one sample body, for eyeballing
samples = OrderedDict()
for m in msgs:
    key = m["dir"] + " " + m["kind"]
    if key not in samples:
        samples[key] = m
with open(os.path.join(OUT, "samples.txt"), "w", encoding="utf-8") as f:
    for k, m in samples.items():
        f.write("===== %s  @ %s =====\n%s\n\n" % (k, m["ts"], m["raw"].strip()))
print("wrote samples for %d message kinds" % len(samples))
