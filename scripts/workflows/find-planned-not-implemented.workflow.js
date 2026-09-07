// =============================================================================
// HT160S_BCB - "Planned but not implemented" detection workflow (READ-ONLY)
// -----------------------------------------------------------------------------
// Goal: find features that were PLANNED in HT160S_BCB but have NO working
//       implementation (internal dead-ends + footprint-backed reference gaps).
//
// Mode (locked by user):
//   * DETECTION ONLY. No source edits. No build trigger. Reference trees are
//     read-only at the tool layer (write-boundary hook).
//   * Reference baselines are READ for comparison only:
//       - Hardware  : old full HT160S source (SECSGem_ToBCB6)
//       - Software  : HT172 0420
//   * B-class (reference gaps) is STRICT: only reported when HT160S_BCB itself
//     keeps a "planning footprint" (stub / referencing comment / dead config /
//     empty form / half-ported module). Otherwise judged "by design, not needed".
//
// Output: one structured result object returned to the main loop, which then
//   writes docs/planned-not-implemented-report.md (UTF-8). The script itself
//   does not write files.
// =============================================================================

export const meta = {
  name: 'find-planned-not-implemented',
  description: 'Read-only scan of HT160S_BCB for planned-but-unimplemented features; strict reference-gap filter; report only',
  whenToUse: 'Audit HT160S_BCB for stubs, dead UI/config, unhandled states, and footprint-backed gaps vs HT172-0420 and old-160 hardware source.',
  phases: [
    { title: 'Map',     detail: 'Parallel readers build a module/form/handler map of HT160S_BCB' },
    { title: 'Find',    detail: '7 parallel finders, one signal category each (read-only)' },
    { title: 'Verify',  detail: 'Adversarial verifier per candidate; strict reject for footprint-less B-class' },
    { title: 'Report',  detail: 'Classify >=80% vs needs-decision; synthesize report payload' },
  ],
}

// ----------------------------------------------------------------------------
// Fixed paths (all local D: drive). HT160S_BCB is the ONLY writable tree.
// ----------------------------------------------------------------------------
const HT160 = 'D:\\HT160S_BCB\\HT160S_Program_BCB_V1.0.0.0'
const HT172 = 'D:\\HT172\\HT172_Program_V1.0.25.0_20260420'
const OLD160 = 'D:\\HT160S -Original 20260323\\Code_V300A\\Program_HT160S_20240806_20241111-SECSGem_ToBCB6'

const READONLY_NOTE =
  'WRITE BOUNDARY: do NOT edit any file. ' + HT172 + ' and ' + OLD160 +
  ' are READ-ONLY reference trees (cpp/h/dfm must never change). Reads only.'

// ----------------------------------------------------------------------------
// Schemas
// ----------------------------------------------------------------------------
const FINDING_SCHEMA = {
  type: 'object',
  additionalProperties: false,
  properties: {
    findings: {
      type: 'array',
      items: {
        type: 'object',
        additionalProperties: false,
        properties: {
          id:          { type: 'string', description: 'short stable id, e.g. F3-aColor-btnReset' },
          category:    { type: 'string', enum: ['stub','todo','orphan-ui','dead-config','unhandled-state','gap-ht172','gap-hardware'] },
          title:       { type: 'string' },
          file:        { type: 'string', description: 'HT160S_BCB file path' },
          line:        { type: 'integer' },
          evidence:    { type: 'string', description: 'quoted code/marker proving it is a dead-end or stub' },
          plannedFrom: { type: 'string', description: 'why we believe it was PLANNED (marker text / DFM binding / config key / ref-project module + path)' },
          footprint:   { type: 'string', description: 'for gap-* only: the HT160S_BCB planning footprint that justifies reporting; empty if none' },
          confidence:  { type: 'integer', description: '0-100 initial confidence this is genuinely planned-but-unimplemented' },
        },
        required: ['id','category','title','file','line','evidence','plannedFrom','footprint','confidence'],
      },
    },
  },
  required: ['findings'],
}

const VERDICT_SCHEMA = {
  type: 'object',
  additionalProperties: false,
  properties: {
    id:            { type: 'string' },
    isReal:        { type: 'boolean', description: 'true = genuinely planned-but-unimplemented after trying to refute' },
    confidence:    { type: 'integer', description: '0-100 post-verification' },
    refutation:    { type: 'string', description: 'what you checked to try to disprove it (callers, alt impl, by-design)' },
    risk:          { type: 'string', enum: ['low','medium','high'], description: 'risk of a fix touching this' },
    singlePoint:   { type: 'boolean', description: 'true = localized single-point fix' },
    fixSummary:    { type: 'string', description: 'concrete minimal fix proposal (NOT applied)' },
    needsDecision: { type: 'string', description: 'if a spec/scope decision is required, state the question + options; else empty' },
  },
  required: ['id','isReal','confidence','refutation','risk','singlePoint','fixSummary','needsDecision'],
}

const MAP_SCHEMA = {
  type: 'object',
  additionalProperties: false,
  properties: {
    modules: {
      type: 'array',
      items: {
        type: 'object',
        additionalProperties: false,
        properties: {
          file:        { type: 'string' },
          role:        { type: 'string' },
          form:        { type: 'string', description: 'bound DFM/form if any' },
          entryPoints: { type: 'array', items: { type: 'string' } },
        },
        required: ['file','role','form','entryPoints'],
      },
    },
  },
  required: ['modules'],
}

// ----------------------------------------------------------------------------
// Phase 1 - module map. Split the ~54 .cpp into groups for parallel readers.
// ----------------------------------------------------------------------------
phase('Map')

const MAP_GROUPS = [
  { label: 'map:auto',   hint: 'aAuto1To6, aColor, aEmpty, aLoader, aSortArm, aTrayArm, csystem' },
  { label: 'map:motion', hint: 'MotorAndIO/*, uMotorTest, uHome, mycylin, MyKitSuck, myio*, iosetview' },
  { label: 'map:io-dev', hint: 'myswitch, mysensor, ComPort, MyBinDisp, TopCcdSocket, ColorCcdSocket, deviceinfo' },
  { label: 'map:ui-cfg', hint: 'main, setup, GeneralSetting, Config, uteach, uOffset, uspeed, note, mymessbox, uPadInterface, uQwertyKey' },
  { label: 'map:data',   hint: 'database, data, cprod, cEventLog, cCommLog, cCsvDailyLog, cStepTrace, cStateRecordHT160, LotWebApiClient, AutomationServer, UserRoleManager, maintenance, cSelfCheck' },
]

const mapParts = await parallel(MAP_GROUPS.map(function (g) {
  return function () {
    return agent(
      'Build a module map for the HT160S_BCB modules in this group: ' + g.hint + '.\n' +
      'Source root: ' + HT160 + '\n' +
      'For each .cpp/.h in the group, report: file, one-line role, bound DFM/form (if any), and key public entry points (functions / VCL event handlers).\n' +
      READONLY_NOTE,
      { label: g.label, phase: 'Map', schema: MAP_SCHEMA }
    )
  }
}))
const moduleMap = mapParts.filter(Boolean).reduce(function (acc, p) { return acc.concat(p.modules) }, [])
log('Module map: ' + moduleMap.length + ' modules cataloged')

// ----------------------------------------------------------------------------
// Phase 2 - finders. Barrier (parallel): we need all candidates before dedup.
// ----------------------------------------------------------------------------
phase('Find')

const FINDERS = [
  {
    key: 'stub', label: 'find:stub',
    prompt:
      'Scan HT160S_BCB (' + HT160 + ') for STUB / empty-body functions: a function declared with an empty body, only "return;", only a comment, or only a debug log, that looks like it was meant to do real work. ' +
      'Ignore legitimately-empty VCL handlers that are wired but intentionally blank ONLY if clearly intended to be empty. ' +
      'Report file:line, the function signature, and quoted body as evidence.',
  },
  {
    key: 'todo', label: 'find:todo',
    prompt:
      'Scan HT160S_BCB (' + HT160 + ') for planning markers: TODO, FIXME, XXX, HACK, and Chinese markers like 未完成, 待補, 暫時, 尚未, 還沒, 之後做, 待實作. ' +
      'For each, report file:line, the quoted comment, and what feature it implies was planned.',
  },
  {
    key: 'orphan-ui', label: 'find:orphan-ui',
    prompt:
      'Scan HT160S_BCB (' + HT160 + ') for ORPHAN UI: DFM components (TButton, TMenuItem, TCheckBox, TSpeedButton...) whose OnClick/event handler exists but is empty or does nothing meaningful, OR a handler that is declared but the body is a no-op. ' +
      'Cross-check the .dfm event binding against the .cpp handler body. Report the component name, the handler, file:line, and evidence.',
  },
  {
    key: 'dead-config', label: 'find:dead-config',
    prompt:
      'Scan HT160S_BCB (' + HT160 + ') for DEAD CONFIG: ini keys / config struct fields / Mot_Table or IO_Table columns / global option flags that are DEFINED or READ-IN but never actually consumed by control logic (assigned but never read, or read into a variable that is never used). ' +
      'Report the key/field, where defined, and evidence that nothing uses it.',
  },
  {
    key: 'unhandled-state', label: 'find:unhandled-state',
    prompt:
      'Scan HT160S_BCB (' + HT160 + ') for UNHANDLED STATES: enum values / Task or Step state constants / mode codes that are DEFINED but never produced, or produced but never handled in any switch/if branch (a planned state with no behavior). ' +
      'Report the constant, its definition, and the missing handling site.',
  },
  {
    key: 'gap-ht172', label: 'find:gap-ht172',
    prompt:
      'Compare HT160S_BCB (' + HT160 + ') against the HT172 0420 software reference (' + HT172 + '). ' +
      'Find SOFTWARE-STRUCTURE features/modules/forms/handlers that HT172 has and HT160S_BCB lacks OR only half-implemented. ' +
      'STRICT RULE: only report a gap if HT160S_BCB itself shows a PLANNING FOOTPRINT for it (a stub, a referencing comment, a dead config key, an empty/partial form, a half-ported module, an unused include). Put that footprint (with file:line) in the "footprint" field. ' +
      'If HT160 has NO footprint, DO NOT report it (assume by-design difference - HT160 is a different machine). Do NOT report FSM internals (HT160 has no FSM by policy). ' +
      READONLY_NOTE,
  },
  {
    key: 'gap-hardware', label: 'find:gap-hardware',
    prompt:
      'Compare HT160S_BCB (' + HT160 + ') against the OLD full HT160S source (' + OLD160 + ') which is the HARDWARE-feature baseline (it contains Inspection, ThermalSystem, MyTempture_*, aCatchArm, gwiopm, myio_ISA/PISO64/SynTek, WinWayATCInterface, etc). ' +
      'Find HARDWARE/device features the old machine had that HT160S_BCB lacks or only stubs. ' +
      'STRICT RULE: only report a gap if HT160S_BCB keeps a PLANNING FOOTPRINT for it (stub function, referencing comment, dead config/IO entry, empty form, half-ported driver). Put that footprint (file:line) in "footprint". ' +
      'If no footprint exists in HT160, DO NOT report it (the BCB6 rebuild may have intentionally dropped that hardware). ' +
      READONLY_NOTE,
  },
]

const finderResults = await parallel(FINDERS.map(function (f) {
  return function () {
    return agent(
      f.prompt + '\n\nBe precise and cite file:line with quoted evidence. Set confidence honestly. ' + READONLY_NOTE,
      { label: f.label, phase: 'Find', schema: FINDING_SCHEMA }
    )
  }
}))

// Dedup across all finders (barrier justified: cross-finder overlap is common).
const seen = {}
const candidates = []
finderResults.filter(Boolean).forEach(function (r) {
  (r.findings || []).forEach(function (fd) {
    var k = (fd.category + '|' + (fd.file || '') + '|' + (fd.line || 0) + '|' + (fd.title || '')).toLowerCase()
    if (seen[k]) return
    seen[k] = true
    candidates.push(fd)
  })
})
log('Finders produced ' + candidates.length + ' unique candidates')

// ----------------------------------------------------------------------------
// Phase 3 - adversarial verification, one verifier per candidate (parallel).
// ----------------------------------------------------------------------------
phase('Verify')

const verdicts = await parallel(candidates.map(function (c) {
  return function () {
    return agent(
      'Adversarially VERIFY this HT160S_BCB candidate "planned-but-unimplemented" finding. Try hard to REFUTE it.\n\n' +
      'Candidate: ' + JSON.stringify(c) + '\n\n' +
      'Refute by checking: (a) is the function/handler actually called or the config actually consumed somewhere (grep callers)? ' +
      '(b) is it intentionally empty / by-design for HT160? ' +
      '(c) for gap-* findings: is the "footprint" real and specific? If the footprint is weak or absent, set isReal=false (STRICT). ' +
      'Source root: ' + HT160 + '. References (read-only): ' + HT172 + ' , ' + OLD160 + '.\n' +
      'Then give a concrete MINIMAL fix proposal (do NOT apply it), risk, whether it is a single-point fix, and post-verification confidence. ' +
      'If a fix needs a spec/scope decision, fill needsDecision. ' + READONLY_NOTE,
      { label: 'verify:' + c.id, phase: 'Verify', schema: VERDICT_SCHEMA }
    ).then(function (v) { return { finding: c, verdict: v } })
  }
}))

// ----------------------------------------------------------------------------
// Phase 4 - classify and assemble report payload (plain code, no agent).
//   A) auto-fixable: isReal, confidence>=80, low risk, single-point, no decision
//   B) needs-decision: everything else that is real
// ----------------------------------------------------------------------------
phase('Report')

const real = verdicts.filter(Boolean).filter(function (x) { return x.verdict && x.verdict.isReal })

const autoFixable = real.filter(function (x) {
  var v = x.verdict
  return v.confidence >= 80 && v.risk === 'low' && v.singlePoint && !(v.needsDecision && v.needsDecision.trim())
})
const needsDecision = real.filter(function (x) {
  return autoFixable.indexOf(x) === -1
})

function byCat(list) {
  var m = {}
  list.forEach(function (x) {
    var c = x.finding.category
    if (!m[c]) m[c] = 0
    m[c]++
  })
  return m
}

log('Classified: ' + autoFixable.length + ' auto-fixable (>=80%), ' + needsDecision.length + ' need decision')

return {
  generatedFor: 'HT160S_BCB planned-but-unimplemented audit',
  mode: 'detection-only (no edits, no build)',
  totals: {
    candidates: candidates.length,
    real: real.length,
    autoFixable: autoFixable.length,
    needsDecision: needsDecision.length,
  },
  byCategory: { autoFixable: byCat(autoFixable), needsDecision: byCat(needsDecision) },
  autoFixable: autoFixable,
  needsDecision: needsDecision,
}
