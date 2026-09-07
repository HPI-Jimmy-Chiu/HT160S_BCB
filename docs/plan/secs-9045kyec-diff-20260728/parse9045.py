# -*- coding: utf-8 -*-
"""Parse HT9045 KYEC SECS text logs -> extract full command surface (real SML parser)."""
import os, re, json, glob, collections

LOGDIR = r"D:\backup_version\HT9046\KYEC\20260626\2026_06_08"
files = sorted(glob.glob(os.path.join(LOGDIR, "*.txt")))

# ---------------- SML tokenizer / parser ----------------
TOK = re.compile(r'<\s*([A-Za-z0-9]+)\s*\[(\d+)\]|>|"([^"]*)"|(-?0x[0-9A-Fa-f]+|-?\d+\.\d+|-?\d+)')

class Node(object):
    __slots__ = ('t', 'n', 'vals', 'kids')
    def __init__(self, t, n):
        self.t = t; self.n = n; self.vals = []; self.kids = []
    def __repr__(self):
        return "<%s[%d] %s %s>" % (self.t, self.n, self.vals, self.kids)

def parse_sml(text):
    """Return list of top-level nodes."""
    pos = 0
    stack = []
    roots = []
    for m in TOK.finditer(text):
        if m.group(1):                      # open <TYPE[n]
            node = Node(m.group(1), int(m.group(2)))
            if stack:
                stack[-1].kids.append(node)
            else:
                roots.append(node)
            stack.append(node)
        elif m.group(0) == '>':
            if stack:
                stack.pop()
        elif m.group(3) is not None:        # quoted string
            if stack:
                stack[-1].vals.append(m.group(3))
        elif m.group(4) is not None:        # number
            if stack:
                stack[-1].vals.append(m.group(4))
    return roots

def iv(node):
    """first value as int"""
    if not node.vals:
        return None
    v = node.vals[0]
    try:
        return int(v, 16) if v.lower().startswith('0x') or v.lower().startswith('-0x') else int(v)
    except ValueError:
        return None

def sv(node):
    return node.vals[0] if node.vals else ""

# ---------------- accumulators ----------------
msg_dir   = collections.defaultdict(collections.Counter)
msg_title = {}
ceids_sent      = collections.Counter()      # S6F11 actually sent
ceid_disabled   = collections.Counter()      # "be disabled, abort send"
ceid_rpt_sent   = collections.defaultdict(set)
rcmds           = collections.Counter()
rcmd_params     = collections.defaultdict(collections.Counter)
rcmd_examples   = {}
def_rptid_svids = collections.defaultdict(set)   # S2F33
def_rptid_hits  = collections.Counter()
del_rptid       = collections.Counter()          # S2F33 with empty svid list
link_ceid_rpt   = collections.defaultdict(set)   # S2F35
unlink_ceid     = collections.Counter()
enable_ceid     = collections.Counter()          # S2F37
enable_flag     = collections.Counter()
alids           = collections.Counter()
altx            = {}
alcd            = collections.defaultdict(set)
ecid_set        = collections.Counter()          # S2F15
ecid_set_val    = collections.defaultdict(set)
svid_req        = collections.Counter()          # S1F3
s6f15_ceid      = collections.Counter()
s6f19_rptid     = collections.Counter()
s10_text        = collections.Counter()
s125_bodies     = collections.Counter()
s125_ecids      = collections.Counter()

hdr      = re.compile(r'^\[(Send|Receive)\]\s+(\d{4}-\d\d-\d\d [\d:.]+)\s*$')
mhdr     = re.compile(r'^\[S(\d+)F(\d+)\]\s*(.*)$')
disabled = re.compile(r'Event Report\((\d+),(\d+)\)\s*,\s*DataID=(\d+)\s*,\s*CEID=(\d+)\s*be disabled')

for fn in files:
    with open(fn, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.read().splitlines()
    cur_dir = None
    i = 0
    while i < len(lines):
        ln = lines[i]
        m = hdr.match(ln)
        if m:
            cur_dir = m.group(1); i += 1; continue
        d = disabled.search(ln)
        if d:
            ceid_disabled[int(d.group(4))] += 1; i += 1; continue
        mm = mhdr.match(ln)
        if not mm:
            i += 1; continue

        s, fnum, title = int(mm.group(1)), int(mm.group(2)), mm.group(3).strip()
        key = "S%dF%d" % (s, fnum)
        body = []
        j = i + 1
        while j < len(lines):
            b = lines[j]
            if b.startswith('[Send]') or b.startswith('[Receive]') or b.startswith('===') or b.startswith('---'):
                break
            body.append(b); j += 1
        i = j
        msg_dir[key][cur_dir or '?'] += 1
        if title:
            msg_title[key] = title
        roots = parse_sml("\n".join(body))
        if not roots:
            continue
        r = roots[0]

        # ---- per-message semantics
        if key == 'S6F11':                      # L[3] DATAID CEID L{ L[2] RPTID L{V} }
            if len(r.kids) >= 3:
                ceid = iv(r.kids[1])
                if ceid is not None:
                    ceids_sent[ceid] += 1
                    for rp in r.kids[2].kids:
                        if rp.kids:
                            rid = iv(rp.kids[0])
                            if rid is not None:
                                ceid_rpt_sent[ceid].add(rid)
        elif key == 'S2F41':                    # L[2] RCMD L{ L[2] CPNAME CPVAL }
            if r.kids:
                cmd = sv(r.kids[0])
                rcmds[cmd] += 1
                if len(r.kids) >= 2:
                    prm = []
                    for p in r.kids[1].kids:
                        if len(p.kids) >= 2:
                            nm, vl = sv(p.kids[0]), sv(p.kids[1]) or (p.kids[1].vals[0] if p.kids[1].vals else "")
                            rcmd_params[cmd][nm] += 1
                            prm.append("%s=%s" % (nm, vl))
                    if prm and cmd not in rcmd_examples:
                        rcmd_examples[cmd] = prm
        elif key == 'S2F33':                    # L[2] DATAID L{ L[2] RPTID L{SVID} }
            if len(r.kids) >= 2:
                for rp in r.kids[1].kids:
                    if rp.kids:
                        rid = iv(rp.kids[0])
                        if rid is None:
                            continue
                        def_rptid_hits[rid] += 1
                        if len(rp.kids) >= 2 and rp.kids[1].kids:
                            for s_ in rp.kids[1].kids:
                                x = iv(s_)
                                if x is not None:
                                    def_rptid_svids[rid].add(x)
                        else:
                            del_rptid[rid] += 1
        elif key == 'S2F35':                    # L[2] DATAID L{ L[2] CEID L{RPTID} }
            if len(r.kids) >= 2:
                for rp in r.kids[1].kids:
                    if rp.kids:
                        cid = iv(rp.kids[0])
                        if cid is None:
                            continue
                        if len(rp.kids) >= 2 and rp.kids[1].kids:
                            for s_ in rp.kids[1].kids:
                                x = iv(s_)
                                if x is not None:
                                    link_ceid_rpt[cid].add(x)
                        else:
                            unlink_ceid[cid] += 1
        elif key == 'S2F37':                    # L[2] CEED L{CEID}
            if r.kids:
                enable_flag[sv(r.kids[0]) or str(iv(r.kids[0]))] += 1
                if len(r.kids) >= 2:
                    for c in r.kids[1].kids:
                        x = iv(c)
                        if x is not None:
                            enable_ceid[x] += 1
        elif key == 'S5F1':                     # L[3] ALCD ALID ALTX
            if len(r.kids) >= 3:
                a = iv(r.kids[1])
                if a is not None:
                    alids[a] += 1
                    altx[a] = sv(r.kids[2])
                    c = iv(r.kids[0])
                    if c is not None:
                        alcd[a].add(c)
        elif key == 'S2F15':                    # L{ L[2] ECID ECV }
            for p in r.kids:
                if p.kids:
                    e = iv(p.kids[0])
                    if e is not None:
                        ecid_set[e] += 1
                        if len(p.kids) >= 2:
                            ecid_set_val[e].add(sv(p.kids[1]) or str(iv(p.kids[1])))
        elif key == 'S1F3':                     # L{SVID}
            for c in r.kids:
                x = iv(c)
                if x is not None:
                    svid_req[x] += 1
            if not r.kids and r.t != 'L':
                x = iv(r)
                if x is not None:
                    svid_req[x] += 1
        elif key == 'S6F15':
            x = iv(r)
            if x is not None:
                s6f15_ceid[x] += 1
        elif key == 'S6F19':
            x = iv(r)
            if x is not None:
                s6f19_rptid[x] += 1
        elif key in ('S10F3', 'S10F5'):
            s10_text[sv(r.kids[1]) if len(r.kids) >= 2 else sv(r)] += 1
        elif key in ('S125F1', 'S125F2'):
            s125_bodies["\n".join(body)[:200]] += 1
            for p in r.kids:
                x = iv(p)
                if x is not None:
                    s125_ecids[x] += 1

def srt(d):
    return dict(sorted(d.items()))

out = {
 'source_dir': LOGDIR,
 'files': [os.path.basename(f) for f in files],
 'messages': {k: dict(v) for k, v in sorted(msg_dir.items(),
              key=lambda x: (int(x[0][1:].split('F')[0]), int(x[0].split('F')[1])))},
 'message_titles': msg_title,
 'ceids_sent_s6f11': srt(ceids_sent),
 'ceids_disabled_abort': srt(ceid_disabled),
 'ceid_rptids_used': {str(k): sorted(v) for k, v in sorted(ceid_rpt_sent.items())},
 'rcmds': srt(rcmds),
 'rcmd_params': {k: srt(dict(v)) for k, v in sorted(rcmd_params.items())},
 'rcmd_param_examples': rcmd_examples,
 's2f33_rptid_svids': {str(k): sorted(v) for k, v in sorted(def_rptid_svids.items())},
 's2f33_rptid_hits': srt(def_rptid_hits),
 's2f33_delete_rptid': srt(del_rptid),
 's2f35_link_ceid_rptids': {str(k): sorted(v) for k, v in sorted(link_ceid_rpt.items())},
 's2f35_unlink_ceid': srt(unlink_ceid),
 's2f37_enable_ceid': srt(enable_ceid),
 's2f37_flag': dict(enable_flag),
 's5f1_alids': srt(alids),
 's5f1_altx': {str(k): v for k, v in sorted(altx.items())},
 's5f1_alcd': {str(k): sorted(v) for k, v in sorted(alcd.items())},
 's2f15_ecids': srt(ecid_set),
 's2f15_ecid_values': {str(k): sorted(v) for k, v in sorted(ecid_set_val.items())},
 's1f3_svid_req': srt(svid_req),
 's6f15_ceid': srt(s6f15_ceid),
 's6f19_rptid': srt(s6f19_rptid),
 's10_text': dict(s10_text),
 's125_ecids': srt(s125_ecids),
}
# all SVIDs the host ever referenced
allsv = set()
for v in def_rptid_svids.values():
    allsv |= v
allsv |= set(svid_req.keys())
out['all_referenced_svids'] = sorted(allsv)
out['all_referenced_ceids'] = sorted(set(ceids_sent) | set(ceid_disabled) | set(link_ceid_rpt) |
                                     set(enable_ceid) | set(unlink_ceid) | set(s6f15_ceid))

outpath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "kyec9045_surface.json")
with open(outpath, 'w', encoding='utf-8') as f:
    json.dump(out, f, ensure_ascii=False, indent=1)
print("WROTE", outpath)
for k in ('messages','ceids_sent_s6f11','ceids_disabled_abort','ceid_rptids_used','rcmds',
          'rcmd_params','rcmd_param_examples','s2f33_rptid_hits','s2f33_delete_rptid',
          's2f35_link_ceid_rptids','s2f35_unlink_ceid','s2f37_enable_ceid','s2f37_flag',
          's5f1_alids','s5f1_altx','s2f15_ecids','s2f15_ecid_values','s1f3_svid_req',
          's6f15_ceid','s6f19_rptid','s10_text','s125_ecids'):
    print("\n==== %s ====" % k)
    print(json.dumps(out[k], ensure_ascii=False))
print("\n==== #referenced SVIDs ====", len(out['all_referenced_svids']))
print("==== s2f33 RPTIDs ====", sorted(def_rptid_svids.keys()))
