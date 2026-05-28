# HT160S_BCB Project Guidelines

## Project Role

HT160S_BCB is the primary development workspace.

- Primary development root: `D:\HT160S_BCB`
- Primary BCB6 project folder: `HT160S_Program_BCB_V1.0.0.0/`
- Primary project file: `HT160S_Program_BCB_V1.0.0.0/ht160s.bpr`
- HT172 reference root: `D:\HT172\HT172_Program_V1.0.25.0_20260420/`

## Absolute HT172 Write Boundary

HT172 is reference-only.

- Never modify, create, delete, format, rename, compile into, or generate files under `D:\HT172`.
- Read and search HT172 files only for comparison and behavior analysis.
- If a requested change appears to require editing HT172, stop and redirect the implementation to HT160S_BCB.
- If the user requests an action that conflicts with these constraints, clarify the request and confirm whether an exception should be made before proceeding.
- Do not copy HT172 runtime data, machine data, `system/`, `data/`, `EXE/`, logs, or customer files into HT160S_BCB. If the user explicitly asks, confirm whether the request applies to all or specific items before proceeding.

## No FSM In HT160

HT160 development must stay non-FSM.

- Do not introduce `FSMRunner`, `FSM_GOTO`, FSM transition tables, `*Step.h`, `*Table.cpp`, `*Exec.cpp`, or any new FSM-style architecture.
- Do not create an `FSM/` module for HT160.
- Do not use the HT172 `fsm-refactor` skill or patterns for HT160 work.
- When porting a feature from HT172 that uses FSM-style code, extract the behavior and rewrite it into HT160's existing procedural, VCL event, or `switch(Task)` style.
- Preserve HT160 legacy naming and control-flow style unless the user explicitly asks for a different design.

## Development Policy

### File Handling Rules

- Edit only files under `D:\HT160S_BCB` unless the user explicitly asks for a non-HT172 external artifact.
- Keep changes narrow and easy to review.
- Search HT160 first before adding a new function, class, or callback.
- Prefer adapting existing HT160 code over copying HT172 code blocks directly.
- `dclusr60.bpi` and BCB6 user component package dependencies are allowed when legacy HT160S or ported UI/features require them. Do not remove these dependencies only because they are external; keep `.bpr`, `.mak`, package lists, and `#pragma link` wiring consistent, and report missing local package setup clearly if BCB6 cannot resolve them.
- Use a local HT160-compatible replacement only when the required component package is unavailable, unsuitable for the target machine, or the user explicitly asks to avoid the package dependency.

### Coding Style Guidelines

- Preserve BCB6 compatibility: no C++11 features such as `auto`, `nullptr`, lambdas, range-for, `enum class`, or `std::string` substitutions for existing `AnsiString` flows.
- Source files (`.cpp`, `.h`, `.dfm`, `.rc`) may use Big5/CP950. Preserve encoding when editing legacy files.

### Machine-Control Safety

- Do not use blocking loops or `Sleep()` in machine-control paths unless the existing HT160 pattern already requires it and the risk is documented.

## Build Verification Contract

HT160 code must not be handed back in a broken state.

- After every C++/DFM/project-file edit, run the narrowest useful BCB6 compile check.
- For changed `.cpp` files, delete the related `.obj` before compiling so stale objects cannot hide failures.
- For project wiring changes, run a full `make -f ht160s.mak` build.
- If compile or link fails, fix the root cause and rerun verification before final response.
- If BCB6 tools are unavailable, report that verification could not run and do not claim the code is build-clean.
- Preferred verification script: `scripts/ops/build-ht160s.ps1`.

```powershell
cd D:\HT160S_BCB
powershell -ExecutionPolicy Bypass -File .\scripts\ops\build-ht160s.ps1 -Clean
```

## Porting Workflow From HT172 0420

1. Read the HT172 0420 source as reference only.
2. Search HT160 for the closest existing implementation.
3. Produce a short difference map: source files, target files, behavior, dependencies, motor/IO/sensor assumptions, and risks.
4. Implement only in HT160.
5. Verify with the narrowest possible compile or static check.
6. Report what changed, what was intentionally not copied, and any machine-control risk.

## Reports And Notes

Use UTF-8 for documentation and governance files. Keep operational notes in `docs/`, `.github/`, or `memories/` under HT160S_BCB. Do not write report output into HT172.
