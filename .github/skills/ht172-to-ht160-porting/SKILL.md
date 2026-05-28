---
name: ht172-to-ht160-porting
description: "Use when porting, comparing, or adapting features from HT172 0420 into HT160S_BCB. Enforces HT172 read-only, HT160 write-only, BCB6 compatibility, and no FSM architecture. Triggers: port HT172, move 0420 feature, migrate 172 to 160, compare HT172 HT160, transplant feature."
---

# HT172 To HT160 Porting Skill

## Purpose

Port field-stable behavior from HT172 0420 into HT160S_BCB while keeping HT172 read-only and keeping HT160 non-FSM.

## Hard Boundaries

- Source reference: `D:\HT172\HT172_Program_V1.0.25.0_20260420/`
- Target implementation: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0/`
- Never write to `D:\HT172`.
- Never copy HT172 runtime data, customer data, logs, or EXE output.
- Do not introduce FSM architecture into HT160.

## No FSM Translation Rule

If the HT172 source uses FSM-style files or APIs, treat them as behavioral documentation only.

Forbidden in HT160 output:

- `FSMRunner`
- `FSM_GOTO`
- `FSM/` directories
- `*Step.h`, `*Table.cpp`, `*Exec.cpp`
- transition table driven execution

Translate sequences into HT160's existing style, such as procedural functions, VCL handlers, or `switch(Task)` state blocks already present in HT160.

## Required Workflow

1. **Source read**: identify the HT172 0420 files, functions, flags, and UI/runtime data involved.
2. **Target search**: search HT160 for equivalent functions, forms, globals, motors, IO names, and data structures.
3. **Difference map**: list source behavior, target location, missing dependencies, incompatible names, and machine-control risks.
4. **Implementation plan**: choose the smallest HT160-only edit.
5. **Edit**: modify HT160 only.
6. **Verify**: compile or run the narrowest available static check.
7. **Report**: state what changed, what HT172 reference was used, what was not copied, and what remains risky.

## Porting Checklist

- Existing HT160 equivalent searched first.
- HT172 read-only source path recorded.
- HT160 target file selected.
- Motor, IO, sensor, switch, sucker, and UI dependencies checked.
- Data/config copy avoided unless explicitly requested.
- BCB6 syntax preserved.
- Big5/CP950 source encoding preserved.
- No FSM names, files, macros, or architecture added.
- BCB6 compile/build verification completed after edits.
- Compile failures fixed before final response.

## Common Decisions

| HT172 source pattern | HT160 action |
| --- | --- |
| FSM step/table/exec split | Extract behavior and rewrite into HT160 legacy flow |
| HT172 hardware name missing in HT160 | Map to HT160 equivalent or stop for user confirmation |
| HT172 runtime config dependency | Recreate only required code-level behavior; do not copy data file |
| HT172 WebAPI/dashboard asset | Port only if HT160 has WebAPI target and user requests it |
| HT172 alarm code behavior | Preserve meaning if applicable, but confirm HT160 alarm registry first |

## Final Response Requirements

Include:

- HT172 reference files read.
- HT160 files changed.
- No-FSM confirmation.
- Data not copied confirmation.
- Verification result or reason verification could not run.
