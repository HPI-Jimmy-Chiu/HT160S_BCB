// =============================================================================
// SECS S5 (alarm catalog) + S7 (recipe transfer) — DESIGN / BATTLE-PLAN workflow
// -----------------------------------------------------------------------------
// READ-ONLY. Produces a design + serial-implementation checklist document:
//   docs/secs-s5s7-battle-plan.md
//
// Mode (locked by user):
//   * No real host / SECS milestone is active yet -> this is PREPARATION.
//     The workflow must NOT start implementation; it only reads, designs, and
//     writes the plan doc. Source stays untouched; HT172 is read-only reference.
//   * Implementation (Stage C) is deliberately OUT OF SCOPE here: BCB6 is a
//     single project with a serial build gate, so coding happens later in the
//     main loop, increment by increment, per the checklist this doc produces.
//
// Already-done facts fed to agents (do not re-flag / do not undo):
//   * S5F6_ListAlarmData already returns a well-formed empty L,0 (no T3 timeout).
//   * The Pad ProcessReceiceData >=13 off-by-one fix is unrelated to S5/S7.
// =============================================================================

export const meta = {
  name: 'secs-s5s7-design-plan',
  description: 'Read-only design + battle plan to port HT172 SECS S5 alarm catalog and S7 recipe transfer into HT160; writes docs/secs-s5s7-battle-plan.md (no implementation)',
  whenToUse: 'Prepare the S5/S7 SECS catalog design + serial implementation checklist before any main-loop coding; safe while no real host milestone is active.',
  phases: [
    { title: 'Understand',  detail: 'Parallel readers: HT172 S5, HT172 S7, HT160 SECS framework, HT160 data sources, SECS simulator' },
    { title: 'Design',      detail: 'S5 and S7 each: 3 independent design approaches -> synthesized design' },
    { title: 'Plan',        detail: 'Turn each final design into a serial, build-verified implementation checklist' },
    { title: 'Adversarial', detail: 'Critics: SEMI E5 compliance + HT160 integration pitfalls' },
    { title: 'Report',      detail: 'Write docs/secs-s5s7-battle-plan.md' },
  ],
}

const H160 = 'D:\\HT160S_BCB\\HT160S_Program_BCB_V1.0.0.0'
const SG160 = H160 + '\\SecsGem'
const H172 = 'D:\\HT172\\HT172_Program_V1.0.25.0_20260420'
const SG172 = H172 + '\\SecsGem'
const SIM = 'D:\\AI_Area\\Tool\\HT160S_SECS_Simulator'
const RO = ' READ-ONLY: never edit any source file; HT172 is a read-only reference; reads only.'
const CTX =
  ' CONTEXT: No real host/SECS milestone is active yet, so this is a PREPARATION design - do NOT start implementation. ' +
  'HT160 rules: NO FSM, ASCII-only new comments, AnsiString flows, no C++11, BCB6 serial build gate (delete .obj then build; -Full for vtable/struct changes). ' +
  'Already done (do not undo/re-flag): S5F6_ListAlarmData returns a well-formed empty L,0 (no T3 timeout). ' +
  'Key HT172 refs: S5 AddAlarmList uHGemHT172.cpp:362, S5F6 :1158, SetAlamData/ReadAlamData/WriteAlamData uHGemEquipment.cpp:6249/6257/6303, ReportAlarm :6319; ' +
  'S7 S7F2 :1362, S7F4 :1411, S7F6 :1520/1557. ' +
  'Known HT160 incompatibilities: THGem has NO form/StringGrid (uses TList registries); recipe is FOLDER-based (CosFunction GetDataRootPath/RecipeExists), not flat PPID.ini; no memoPPBody; S9F7_IllegalData/LocalAcknowledge primitives are ABSENT (only S9F3 exists).'

const READ_SCHEMA = { type:'object', additionalProperties:false, properties:{
  summary:{type:'string'},
  facts:{ type:'array', items:{ type:'object', additionalProperties:false, properties:{
    topic:{type:'string'}, file:{type:'string'}, line:{type:'integer'},
    detail:{type:'string'}, codeQuote:{type:'string'} },
    required:['topic','file','line','detail','codeQuote'] } } },
  required:['summary','facts'] }

const DESIGN_SCHEMA = { type:'object', additionalProperties:false, properties:{
  subsystem:{type:'string', enum:['S5','S7']},
  approachName:{type:'string'},
  dataStructures:{type:'string', description:'HT160-adapted structs/TLists/fields'},
  functions:{ type:'array', items:{type:'string'}, description:'function signatures to add/implement, with file' },
  fileLayout:{type:'string', description:'which files change, on-disk files (e.g. AlarmData.def / recipe folder)'},
  persistence:{type:'string'},
  tradeoffs:{type:'string'},
  risks:{type:'string'} },
  required:['subsystem','approachName','dataStructures','functions','fileLayout','persistence','tradeoffs','risks'] }

const FINAL_SCHEMA = { type:'object', additionalProperties:false, properties:{
  subsystem:{type:'string', enum:['S5','S7']},
  chosenApproach:{type:'string'},
  rationale:{type:'string', description:'why chosen + ideas grafted from runners-up'},
  design:{type:'string', description:'the consolidated final design in detail'},
  openDecisions:{ type:'array', items:{type:'string'} } },
  required:['subsystem','chosenApproach','rationale','design','openDecisions'] }

const PLAN_SCHEMA = { type:'object', additionalProperties:false, properties:{
  subsystem:{type:'string', enum:['S5','S7']},
  increments:{ type:'array', items:{ type:'object', additionalProperties:false, properties:{
    step:{type:'integer'}, title:{type:'string'},
    files:{ type:'array', items:{type:'string'} },
    detail:{type:'string'},
    objToDelete:{type:'string'}, buildFlag:{type:'string', enum:['-Clean','-Full']},
    simTest:{type:'string', description:'SECS simulator verification for this increment'},
    risk:{type:'string', enum:['low','medium','high']} },
    required:['step','title','files','detail','objToDelete','buildFlag','simTest','risk'] } } },
  required:['subsystem','increments'] }

const CRITIC_SCHEMA = { type:'object', additionalProperties:false, properties:{
  findings:{ type:'array', items:{ type:'object', additionalProperties:false, properties:{
    area:{type:'string', enum:['semi-e5-compliance','ht160-integration','build-gate','host-scenario','data-source','other']},
    severity:{type:'string', enum:['low','medium','high']},
    issue:{type:'string'}, fix:{type:'string'} },
    required:['area','severity','issue','fix'] } } },
  required:['findings'] }

// ---------------------------------------------------------------------------
phase('Understand')
const reads = await parallel([
  function(){ return agent(
    'Read HT172 S5 (alarm) implementation under ' + SG172 + ': AddAlarmList (uHGemHT172.cpp:362), S5F6_ListAlarmData (:1158), and THGem SetAlamData/ReadAlamData/WriteAlamData (uHGemEquipment.cpp:6249/6257/6303), ReportAlarm (:6319). Explain the operating model: the catalog store (StringGrid + AlarmData.def), how the machine alarm map populates it, how S5F6 replies to S5F5, and the S5F1 alarm push. Quote file:line.' + RO + CTX,
    { label:'read:ht172-s5', phase:'Understand', schema: READ_SCHEMA }) },
  function(){ return agent(
    'Read HT172 S7 (recipe) implementation under ' + SG172 + ': S7F2_ProcessProgramLoadGrant (uHGemHT172.cpp:1362), S7F4_ProcessProgramAcknowledge (:1411), S7F6_ProcessProgramData (:1520 and :1557). Explain the operating model: recipe file model (flat data\\PPID.ini), HCACK validation, download (F4) save, upload (F6) read+reply, use of memoPPBody, and the S9F7_IllegalData/LocalAcknowledge primitives it relies on. Quote file:line.' + RO + CTX,
    { label:'read:ht172-s7', phase:'Understand', schema: READ_SCHEMA }) },
  function(){ return agent(
    'Map the HT160 SECS framework under ' + SG160 + ': the SECS-II codec (DataItemOut / DataItemIn / GetDataItemLenAndType[AndDelete] / type constants), InitLocalHead/SendLocalData reply pattern (cite existing S1F4/S2F14 handlers in uHGemHT160.cpp), the Dispatch router (uHGemClass.cpp) and how S5/S7 are routed, the HSMS-state gate, S9F3_Unrecognized (and CONFIRM S9F7_IllegalData and LocalAcknowledge are ABSENT), and THGem structure (no form; SV/EC/CEID use TList registries). Quote file:line.' + RO + CTX,
    { label:'read:ht160-framework', phase:'Understand', schema: READ_SCHEMA }) },
  function(){ return agent(
    'Map the HT160 data sources for S5/S7 under ' + H160 + ': (S5) database.h MyAlarmCodeStruct + mapAlarmCodeList + IterAlarmCodeList, CreateSystemAlarmCode() in database.cpp and the system\\AlarmList.csv dump - list exactly which code families it covers (cylinder/motor/sucker/system/sensor?). (S7) the recipe manager / CosFunction.cpp: GetDataRootPath(), RecipeExists(), GetRecipeFileName(), and the on-disk recipe FOLDER format (setup.ini / BinAreaMap.ini). Also THGem CurrentDirectory (uHGemEquipment.cpp:23). Quote file:line.' + RO + CTX,
    { label:'read:ht160-datasrc', phase:'Understand', schema: READ_SCHEMA }) },
  function(){ return agent(
    'Inspect the shared SECS test tool at ' + SIM + ' (READ-ONLY): what SECS/GEM messages can it send and verify, and how would it be used to round-trip test S5F5->S5F6, S5F1, S7F1->F2, S7F3->F4, S7F5->F6? Summarize its capabilities and any config needed. Quote file paths.' + RO + CTX,
    { label:'read:simulator', phase:'Understand', schema: READ_SCHEMA }) },
])
const facts = JSON.stringify(reads.filter(Boolean))
log('Understand: ' + reads.filter(Boolean).length + ' readers done')

// ---------------------------------------------------------------------------
phase('Design')
const SUBS = ['S5','S7']
const finals = await parallel(SUBS.map(function(sub){
  return function(){
    // 3 diverse design approaches for this subsystem
    var angles = (sub==='S5')
      ? ['minimal: TList-only catalog, no .def persistence, S5F6 reply only',
         'full parity: TList + AlarmData.def persistence + editable Enable, mirrors HT172',
         'pragmatic: TList catalog + optional persistence flag, S5F1 reporting stubbed for a later work item']
      : ['upload-only: S7F5/F6 read recipe folder and reply (no download)',
         'full: S7F1-F4 download + S7F5/F6 upload, folder<->PPID mapping, add S9F7 primitive',
         'gated: add S9F7 + S7F2 grant + log-acknowledge stubs that are SEMI-valid, defer file IO']
    return parallel(angles.map(function(a, i){
      return function(){ return agent(
        'Design approach #' + (i+1) + ' (' + a + ') for porting SECS ' + sub + ' into HT160. Base it on the reader facts:\n' + facts + '\n' +
        'Produce a concrete HT160-adapted design (no FSM, TList not StringGrid, folder-based recipe, AnsiString). Give data structures, function signatures with files, on-disk layout, persistence, tradeoffs, risks.' + CTX + RO,
        { label:'design:'+sub+'#'+(i+1), phase:'Design', schema: DESIGN_SCHEMA }) }
    })).then(function(proposals){
      var ps = proposals.filter(Boolean)
      return agent(
        'You are the lead architect. Pick the best SECS ' + sub + ' design for HT160 and synthesize a final design, grafting good ideas from the runners-up. Proposals:\n' + JSON.stringify(ps) + '\n' +
        'Reader facts:\n' + facts + '\nFavor lowest-risk that still meets a real host need; note open decisions (the product decisions D1-D5).' + CTX + RO,
        { label:'design:'+sub+':final', phase:'Design', schema: FINAL_SCHEMA })
    })
  }
}))
const finalsArr = finals.filter(Boolean)
log('Design: ' + finalsArr.length + ' final designs')

// ---------------------------------------------------------------------------
phase('Plan')
const plans = await parallel(finalsArr.map(function(fd){
  return function(){ return agent(
    'Turn this final SECS ' + fd.subsystem + ' design into a SERIAL, build-verified implementation checklist for the HT160 main loop (NOT parallel). Design:\n' + JSON.stringify(fd) + '\n' +
    'Each increment: title, files, what to change/port (with HT172 ref line), which .obj to delete, build flag (-Clean/-Full; -Full for vtable/struct), the SECS-simulator test for that increment, and risk. Order so each increment builds green on its own.' + CTX + RO,
    { label:'plan:'+fd.subsystem, phase:'Plan', schema: PLAN_SCHEMA }) }
}))
const plansArr = plans.filter(Boolean)

// ---------------------------------------------------------------------------
phase('Adversarial')
const critics = await parallel(finalsArr.map(function(fd){
  return function(){ return agent(
    'Adversarially review this final SECS ' + fd.subsystem + ' design for HT160. Try to break it. Design:\n' + JSON.stringify(fd) + '\nReader facts:\n' + facts + '\n' +
    'Hunt for: SEMI E5 non-compliance (message shapes, ACK codes, W-bit replies), HT160 integration pitfalls (no form, folder recipe, missing S9F7/LocalAcknowledge, HSMS gating), build-gate hazards (vtable/struct), and missed host scenarios. For each: area, severity, issue, concrete fix.' + CTX + RO,
    { label:'critic:'+fd.subsystem, phase:'Adversarial', schema: CRITIC_SCHEMA }) }
}))
const criticsArr = critics.filter(Boolean)

// ---------------------------------------------------------------------------
phase('Report')
const rep = await agent(
  'Write a Traditional-Chinese battle-plan document to D:\\HT160S_BCB\\docs\\secs-s5s7-battle-plan.md (UTF-8, no BOM). ' +
  'Inputs: final designs ' + JSON.stringify(finalsArr) + ' ; implementation checklists ' + JSON.stringify(plansArr) + ' ; adversarial findings ' + JSON.stringify(criticsArr) + ' ; reader facts ' + facts + '\n\n' +
  'Structure: (1) 摘要 + 狀態(無 host 里程碑, 此為備用設計, 實作延後); (2) 前置決策表 D0-D5; (3) HT172 如何運作 (S5 / S7) 摘要; (4) HT160 定案設計 (S5 / S7) 含資料結構/函式/檔案佈局/持久化; (5) 序列實作清單 (逐增量: 檔/改動/刪obj/build flag/模擬器測項/風險) - 強調主迴圈序列、build gate; (6) 對抗審查發現 (依嚴重度); (7) 風險與鐵則 (HT172唯讀/no-FSM/ASCII/vtable->-Full/先補S9F7/勿動無關進入點). ' +
  'Keep code identifiers/file:line ASCII; prose 繁體中文. WRITE ONLY that one .md; never edit source.' + RO +
  ' After writing, reply with the absolute path and an 8-line 繁中 summary: the chosen S5 and S7 approaches, the #1 adversarial finding each, and the recommended first increment.',
  { label:'report:battle-plan', phase:'Report' })

return {
  task: 'SECS S5/S7 design + battle plan (read-only, implementation deferred)',
  finalDesigns: finalsArr.length,
  plans: plansArr.length,
  criticFindings: criticsArr.reduce(function(n,c){ return n + ((c.findings||[]).length) }, 0),
  report: rep,
}
