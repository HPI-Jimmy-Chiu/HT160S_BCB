export const meta = {
  name: 'ht160s-port-statmain',
  description: 'Port HT172 statMain status bar into HT160S (renamed stbMain): investigate sources + produce a reviewable, byte-safe edit plan. Stops at the plan (no edits, no build).',
  phases: [
    { title: 'Investigate', detail: '4 parallel readers: HT172 statMain wiring, HT160S main form, support infra, maintenance form' },
    { title: 'Design', detail: 'synthesize one concrete byte-safe edit plan (8 panels + new maintenance fields + #ifdef sim indicator + time subsystem + SECS SV)' },
    { title: 'Critique', detail: 'completeness + encoding/rename-gap adversarial critic' },
  ],
}

const HT172 = 'D:/HT172/HT172_Program_V1.0.25.0_20260420'
const HT160 = 'D:/HT160S_BCB/HT160S_Program_BCB_V1.0.0.0'

// ---- schemas ---------------------------------------------------------------
const FINDINGS_SCHEMA = {
  type: 'object',
  additionalProperties: false,
  properties: {
    area: { type: 'string' },
    summary: { type: 'string' },
    items: {
      type: 'array',
      items: {
        type: 'object',
        additionalProperties: false,
        properties: {
          file: { type: 'string' },
          location: { type: 'string', description: 'line range or unique anchor text' },
          role: { type: 'string' },
          verbatim: { type: 'string', description: 'exact snippet copied byte-for-byte when asked' },
        },
        required: ['file', 'role'],
      },
    },
    gapsOrRecommendations: { type: 'array', items: { type: 'string' } },
  },
  required: ['area', 'summary', 'items'],
}

const EDIT_PLAN_SCHEMA = {
  type: 'object',
  additionalProperties: false,
  properties: {
    overview: { type: 'string' },
    newSymbols: {
      type: 'array',
      items: {
        type: 'object',
        additionalProperties: false,
        properties: {
          name: { type: 'string' },
          kind: { type: 'string', description: 'global | maintenance-field | dfm-component | event-handler | const' },
          file: { type: 'string' },
          declaration: { type: 'string' },
        },
        required: ['name', 'kind', 'file'],
      },
    },
    edits: {
      type: 'array',
      items: {
        type: 'object',
        additionalProperties: false,
        properties: {
          order: { type: 'integer' },
          file: { type: 'string' },
          kind: { type: 'string', description: 'dfm-inject | cpp-edit | h-edit | new-file' },
          anchor: { type: 'string', description: 'exact existing text used to locate the edit' },
          oldSnippet: { type: 'string' },
          newSnippet: { type: 'string' },
          byteSafeMethod: { type: 'string', description: 'bcb6-bytesafe-edit.ps1 | manual-dfm-splice | new-file' },
          encodingNotes: { type: 'string' },
          rationale: { type: 'string' },
        },
        required: ['order', 'file', 'kind', 'newSnippet', 'byteSafeMethod'],
      },
    },
    buildVerify: { type: 'array', items: { type: 'string' } },
    risks: { type: 'array', items: { type: 'string' } },
    openQuestions: { type: 'array', items: { type: 'string' } },
  },
  required: ['overview', 'edits', 'buildVerify'],
}

const CRITIC_SCHEMA = {
  type: 'object',
  additionalProperties: false,
  properties: {
    complete: { type: 'boolean' },
    missingReferences: { type: 'array', items: { type: 'string' } },
    encodingRisks: { type: 'array', items: { type: 'string' } },
    renameGaps: { type: 'array', items: { type: 'string' } },
    decisionConflicts: { type: 'array', items: { type: 'string' }, description: 'where the plan violates a locked decision' },
    verdict: { type: 'string' },
  },
  required: ['complete', 'verdict'],
}

// ---- locked decisions (from user) ------------------------------------------
const DECISIONS = `
LOCKED DECISIONS (do not deviate):
1. Faithful port of HT172 statMain status bar, RENAMED to "stbMain" everywhere (component, all references). The enum eMainState/emsVersion keeps its meaning.
2. Full 8 panels: [0]=software version, [1]=Model, [2]=Handler/Machine ID, [3]=Serial No, [4..6]=spare (one of them is the SIM indicator), [7]=live system time.
3. Panels [1][2][3] need data: ADD new MachineID / SerialNo / Model fields to the HT160S maintenance form (HT160S has none), persist them, copy into new global AnsiStrings, write them to the panels - mirroring HT172 MyFunctionB::Update().
4. SIM indicator = COMPILE-TIME ONLY: guarded by #ifdef SOFT_SIMULATE (defined in MachineType.h). When defined, one spare panel shows red text (e.g. "SIMULATE"); when not defined the code is not compiled and the panel stays blank. Do NOT use IsSoftSimulate() (which also returns true for runtime DUMMY) - this indicator is about the BUILD type and is intentionally separate from the existing Real/Dummy status icon (sbRealIcon).
5. Red text in a TStatusBar panel = set that panel Style=psOwnerDraw and draw red in an OnDrawPanel handler. The OnDrawPanel handler MUST be in __published (DFM-wired events in private/public throw EReadError at form ctor). Keep all component fields BEFORE all event-handler methods in the class body; no comments inside the __published form-class body (BCB6 designer raises "Incorrect method declaration").
6. SECS SV "System Time" (HT172 used SV 1027) IS to be mirrored onto the time panel, using HT160S SetSVDataPointer convention and a free/appropriate SV id.

ENCODING / EDIT CONSTRAINTS (BCB6, hard rules):
- Legacy .cpp/.h/.dfm are Big5/CP950 with CRLF. Preserve byte-for-byte. No UTF-8 conversion, no BOM.
- Edit .cpp/.h ONLY via scripts/ops/bcb6-bytesafe-edit.ps1 (the plain Edit tool re-encodes UTF-8 and mangles Big5).
- Edit .dfm by manual byte-safe splice. NEVER open the form in the BCB designer and NEVER designer-save (it silently drops components + matching __published decls and rewrites CRLF->LF).
- New comments in source must be ASCII English only.
- No C++11 (no auto/nullptr/lambda/range-for/enum class). Keep AnsiString flows. Procedural / VCL-event style, no FSM.
`

// ---- Phase 1: investigate --------------------------------------------------
phase('Investigate')

const readers = [
  {
    label: 'read:ht172-statmain',
    prompt: `You are READ-ONLY. Map the HT172 "statMain" TStatusBar so it can be ported to HT160S. Reference root: ${HT172} (read-only). Return EXACT verbatim snippets (copy byte-for-byte) for each:
1. main.dfm - the "object statMain: TStatusBar" block (~line 15371). Return the FULL block incl. all 8 Panels items (Alignment/Width), SimplePanel, SizeGrip, and the parent container it sits in.
2. main.h - the "TStatusBar *statMain;" declaration (~line 144) AND the "enum eMainState { emsVersion=0 };" (~line 1382). Return both verbatim.
3. main.cpp - lines ~34-41 (version-string assignment with QC/CG compile-flag branches): what macro/#define selects QC/CG, and what "MainVersion" is/where it is defined. AND line ~315 (AddMyTimeStringShow(... Panels->Items[7], 0)) with surrounding context (which function it is in).
4. database.cpp - MyFunctionB::Update() (~3356-3377): how asModel/asHandlerID/asSerialNo are sourced (edN01_MachineID/edSerialNo/...) and written to panels [1][2][3]. Also FIND where asModel/asHandlerID/asSerialNo are DECLARED (which file, global AnsiString?).
5. systools.cpp - the full time-string subsystem: AddMyTimeStringShow (~996), RefreshMyTimeString (~1018), the ShowTimeStringList container, the MyTimeStringShowList struct definition, and WHERE/HOW RefreshMyTimeString is ticked each second. Also line ~1134 program-start log that reads panel[0].
6. SecsGem/uHGemHT172.cpp - line ~103, the SV 1027 "System Time" SetSVDataPointer binding to Panels->Items[7] (verbatim).
Report each as an item (file/location/role/verbatim). In gapsOrRecommendations, list exactly what HT160S must replicate (time subsystem, version const, identity globals, SV).`,
  },
  {
    label: 'read:ht160s-mainform',
    prompt: `You are READ-ONLY (report only). Map the HT160S main form to place a bottom TStatusBar to be named "stbMain". Root: ${HT160}.
1. main.h - the TfMain class: confirm there is NO existing TStatusBar; report the page-control layout (pgcMain/tsMain etc, ~lines 97-330) and the __published section structure (where component fields end and event handlers begin - needed for adding a new component + OnDrawPanel handler in the correct order).
2. main.dfm - the TfMain form dimensions, and the BOTTOM region. Find components near the form bottom (e.g. sbRealIcon, sbStartIcon and any status panel) with their Left/Top/Width/Height so a new Align=alBottom status bar will not overlap. Recommend the EXACT dfm insertion point and placement so stbMain shows across all tabs (prefer alBottom on the form, or report the best container).
3. main.cpp - the TfMain constructor / FormCreate / startup path that is the analog of HT172 main.cpp lines 34-41 and 315 - i.e. where the version string should be set and where the time-string registration call should hook. Confirm there is no existing clock/time display.
Report verbatim anchors for each insertion site.`,
  },
  {
    label: 'read:ht160s-infra',
    prompt: `You are READ-ONLY (report only). Map HT160S support infrastructure needed by the statMain port. Root: ${HT160}.
1. systools.cpp + systools.h - confirm TFormSysTools currently lacks AddMyTimeStringShow / RefreshMyTimeString / ShowTimeStringList / MyTimeStringShowList. List what TFormSysTools does have. Identify the time globals available (SystemYear/SystemMonth/SystemDate/SystemHour/SystemMin/SystemSec or equivalent) and whether a periodic (per-second) timer already ticks systools/main so a ported RefreshMyTimeString can be called each second. Report where that tick is.
2. SecsGem/uHGemHT160.cpp - the SetSVDataPointer registration pattern (show 2 example lines verbatim) and recommend a free/appropriate SV id for "System Time" (HT172 used 1027); report nearby SV ids already in use.
3. Find where a software VERSION string constant lives or should live. Search for any existing version literal (e.g. "V1.0", a #define, an AnsiString). Recommend how/where to define MainVersion HT172-style. Confirm whether QC/CG compile flags exist in HT160S (if not, the version suffix logic is dropped).
4. Find the program-start logging call (HT172 used RecordProcess(...)). Report the HT160S analog (cEventLog / RecordProcess / etc) so the "Program Start with version ..." line can be replicated.
Report verbatim anchors + concrete recommendations.`,
  },
  {
    label: 'read:ht160s-maintenance',
    prompt: `You are READ-ONLY (report only). Map the HT160S maintenance form to ADD MachineID / SerialNo / Model fields (the data source for status-bar panels 1-3, which HT160S currently lacks). Root: ${HT160}.
1. maintenance.h / maintenance.dfm - confirm there are NO MachineID/SerialNo/Model/FactoryID edit fields. Identify the best TabSheet/panel to add 3 new TEdit fields. Show ONE existing edit-field group verbatim as a template (its component naming convention, Left/Top layout, the matching __published declaration, and the field-vs-handler ordering in the .h).
2. maintenance.cpp - how existing maintenance fields are LOADED from and SAVED to config (which ini / data file) on form open / apply. Show the load+save site for one existing field verbatim as a template, so the 3 new fields get persisted across restarts.
3. database.cpp (HT160S) - find the analog of HT172 MyFunctionB::Update() that copies maintenance fields into globals, OR report that none exists. Recommend WHERE to add the copy into new globals asModel/asHandlerID/asSerialNo and WHERE those global AnsiStrings should be declared (which .cpp + extern in which .h).
Report verbatim anchors + a concrete recommendation for field placement + naming that follows HT160S conventions (do NOT blindly reuse HT172 edN01_* names unless HT160S uses that scheme).`,
  },
]

const findings = (await parallel(
  readers.map(function (r) {
    return function () {
      return agent(r.prompt, { label: r.label, phase: 'Investigate', schema: FINDINGS_SCHEMA })
    }
  })
)).filter(Boolean)

log(`Investigate done: ${findings.length}/${readers.length} reader reports collected`)

// ---- Phase 2: design -------------------------------------------------------
phase('Design')

const designPrompt = `You are a senior BCB6 (C++Builder 6) engineer producing a CONCRETE, REVIEWABLE edit plan to port the HT172 "statMain" status bar into HT160S (renamed "stbMain"). You will NOT apply edits - you only author the plan a human will review, then a teammate applies byte-safe and builds.

${DECISIONS}

You have these investigation reports (JSON):
${JSON.stringify(findings, null, 2)}

Produce the edit plan. Requirements:
- Cover EVERY HT172 statMain touch-point and its HT160S equivalent: the DFM component (8 panels, with one spare panel set Style=psOwnerDraw for the sim indicator), main.h declaration + eMainState enum + the __published OnDrawPanel handler decl, main.cpp version-string set + time registration + the #ifdef SOFT_SIMULATE sim-panel write + the OnDrawPanel red-draw body, the time-string subsystem ported into HT160S TFormSysTools (and where its per-second tick is wired), the new global AnsiStrings, the maintenance form 3 new fields + their load/save persistence + copy-into-globals (mirroring MyFunctionB::Update), the SECS SV "System Time" binding, and the program-start version log line.
- For each edit give: order, file, kind (dfm-inject|cpp-edit|h-edit|new-file), a UNIQUE anchor (existing text to locate), oldSnippet (when modifying), newSnippet (the exact bytes to introduce - ASCII-only comments), byteSafeMethod (bcb6-bytesafe-edit.ps1 for .cpp/.h; manual-dfm-splice for .dfm), encodingNotes, rationale.
- buildVerify: the exact build-gate steps - delete the changed .obj, scripts/ops/build-ht160s.ps1 -Clean (or -Full if struct/class layout changed), then the real-machine build check (comment out #define SOFT_SIMULATE in MachineType.h -> -Full -> expect EXIT 0 -> restore the define -> rebuild), then scripts/ops/check-ht160s-source-encoding.ps1.
- risks: call out the DFM/Big5/designer-save hazards specifically.
- openQuestions: anything the readers could not resolve (e.g. ambiguous insertion point, missing tick).
Be exact and self-consistent. Prefer minimal, surgical edits that match surrounding HT160S style.`

const plan = await agent(designPrompt, { label: 'design:edit-plan', phase: 'Design', schema: EDIT_PLAN_SCHEMA })

// ---- Phase 3: critique -----------------------------------------------------
phase('Critique')

const criticPrompt = `You are an adversarial reviewer of a BCB6 port edit plan. Default to skepticism. Root HT160S: ${HT160}; reference HT172: ${HT172}. You MAY read files to verify the plan's anchors actually exist and that no statMain reference was missed.

${DECISIONS}

The proposed edit plan (JSON):
${JSON.stringify(plan, null, 2)}

The investigation reports (JSON):
${JSON.stringify(findings, null, 2)}

Check rigorously and report:
- missingReferences: any HT172 statMain touch-point (DFM, enum, version, model/id/serial, time subsystem + its tick, SV, program-start log) NOT covered, or any anchor in the plan that does not actually exist in the target file.
- encodingRisks: any edit that would corrupt Big5/CRLF, any .dfm edit not using manual byte-safe splice, any .cpp/.h edit not via bcb6-bytesafe-edit.ps1, any non-ASCII new comment, any designer-save dependency.
- renameGaps: any place still calling it statMain instead of stbMain; any eMainState reference left dangling.
- decisionConflicts: anything violating the LOCKED DECISIONS (esp. sim indicator must be #ifdef SOFT_SIMULATE not IsSoftSimulate(); OnDrawPanel must be __published; fields-before-handlers; no comments in __published body; full 8 panels; maintenance fields persisted).
Set complete=true only if the plan is faithful, safe, and ready for a human to apply. Give a concise verdict with the top fixes needed.`

const critic = await agent(criticPrompt, { label: 'critic:completeness', phase: 'Critique', schema: CRITIC_SCHEMA })

return { plan: plan, critic: critic, readerCount: findings.length }
