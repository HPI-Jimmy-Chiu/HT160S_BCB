#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ---------------------------------------------------------------------------
# check-ladder-consistency.py
#
# Static (no AI, no run) audit of the HT160S switch(Task)-style state ladders.
# Flags the "a number but no action" defect class the field engineer reported:
# a state cursor is assigned a value (Task=N) that has NO matching `case N:` in
# the same switch, so the ladder jumps to nothing (silent stall risk / trap).
#
# Deterministic parser:
#   * A switch is treated as a STATE LADDER only if its switch variable is
#     assigned a numeric literal somewhere in the same function (var = N;).
#     Pure value-dispatch switches (switch(Index) in Get*Position lookups,
#     where the var is a function parameter never reassigned) are ignored.
#   * Values also "handled" by an if-guard (if(var==N) / if(var>=N ...)) before
#     the switch are treated as handled (mirrors mycylin Push/Pop Task 50).
#   * A switch that HAS a `default:` cannot silently do nothing (default runs),
#     so a missing case there is a WARN (dead/incomplete number, code smell),
#     not an ERROR. Only a missing case in a switch WITHOUT a default is a real
#     silent dead-jump -> ERROR (this is what the ladder-guard default: fixes).
#
# Exit code: 0 = clean, 1 = at least one dead-jump found (usable as a build gate
# / pre-commit check). Usage:  python scripts/ops/check-ladder-consistency.py
# ---------------------------------------------------------------------------
import re, glob, os, sys

ROOT = os.path.join(os.path.dirname(__file__), "..", "..", "HT160S_Program_BCB_V1.0.0.0")
ROOT = os.path.abspath(ROOT)

func_hdr = re.compile(r'\n([A-Za-z_][\w:\*\s&]*?\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{]*\)\s*)\{', re.S)
switch_re = re.compile(r'switch\s*\(\s*([A-Za-z_][\w\.\->\[\]]*)\s*\)\s*\{')
case_re = re.compile(r'\bcase\s+(\d+)\s*:')

def brace_match(s, open_idx):
    depth = 0
    for i in range(open_idx, len(s)):
        if s[i] == '{': depth += 1
        elif s[i] == '}':
            depth -= 1
            if depth == 0:
                return i
    return len(s) - 1

def line_of(s, idx):
    return s.count('\n', 0, idx) + 1

def audit_file(path):
    s = open(path, 'rb').read().decode('latin1')
    out = []
    for m in func_hdr.finditer(s):
        cls, meth = m.group(2), m.group(3)
        ob = s.index('{', m.end() - 1)
        ce = brace_match(s, ob)
        body = s[ob:ce]
        for sw in switch_re.finditer(body):
            var = sw.group(1)
            sob = body.index('{', sw.end() - 1)
            sce = brace_match(body, sob)
            block = body[sob:sce]
            labels = set(int(x) for x in case_re.findall(block))
            has_default = re.search(r'\bdefault\s*:', block) is not None
            # assignments to this switch var anywhere in the function body
            assign = re.compile(re.escape(var) + r'\s*=\s*(\d+)\s*;')
            targets = set(int(x) for x in assign.findall(body))
            if not targets:
                continue   # value-dispatch switch, not a ladder
            # values handled by if-guards (var==N / var>=N)
            ifh = set(int(x) for x in re.findall(re.escape(var) + r'\s*(?:==|>=|<=|>|<)\s*(\d+)', body))
            missing = sorted(t for t in targets if t not in labels and t not in ifh)
            if missing:
                ln = line_of(s, ob + sw.start())
                sev = "WARN" if has_default else "ERROR"
                out.append((sev, os.path.basename(path), cls + "::" + meth, var, ln, missing))
    return out

def main():
    findings = []
    for f in sorted(glob.glob(os.path.join(ROOT, "*.cpp"))):
        findings.extend(audit_file(f))
    errors = [x for x in findings if x[0] == "ERROR"]
    warns = [x for x in findings if x[0] == "WARN"]
    for title, items in (("ERROR (silent dead-jump: no case, no default)", errors),
                         ("WARN (dead/incomplete number; default catches it)", warns)):
        if items:
            print("ladder-consistency %s: %d" % (title, len(items)))
            print("-" * 90)
            for sev, fb, fn, var, ln, nums in items:
                print("  %-16s %-34s switch(%s) @L%d  ->  no case for %s" % (fb, fn[:34], var, ln, nums))
            print("-" * 90)
    if not findings:
        print("ladder-consistency: OK - no dead-jump (Task=N with no case N) found")
    if errors:
        print("ERRORS are 'number but no action' traps. Fix: remove the dead assignment, add the")
        print("missing case, or add a default: guard (LogLadderFault). Build gate fails.")
        return 1
    return 0

if __name__ == "__main__":
    sys.exit(main())
