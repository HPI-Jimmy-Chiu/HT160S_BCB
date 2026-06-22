export const meta = {
  name: 'b-region-fixmethods',
  description: 'Expand a concrete, actionable fix method for each remaining needs-decision (B) item from the planned-but-unimplemented audit',
  whenToUse: 'After the audit + the 4 applied fixes; produce step-by-step fix methods for the remaining B items.',
  phases: [
    { title: 'Load',   detail: 'Load remaining B items (exclude the already-fixed ones)' },
    { title: 'Expand', detail: 'One agent per item: confirm current code + concrete fix steps' },
    { title: 'Report', detail: 'Write docs/planned-not-implemented-fixmethods.md' },
  ],
}

const H160 = 'D:\\HT160S_BCB\\HT160S_Program_BCB_V1.0.0.0'
const H172 = 'D:\\HT172\\HT172_Program_V1.0.25.0_20260420'
const RO = ' READ-ONLY: never edit any source file; HT172 is read-only; reads only.'
const REPORT = 'D:\\HT160S_BCB\\docs\\planned-not-implemented-report.md'
const JSON_SRC = 'C:\\Users\\JIMMYC~1\\AppData\\Local\\Temp\\claude\\D--HT160S-BCB\\c199289a-890d-4cff-82fb-ffc0d841fe58\\tasks\\wwynsdqgx.output'

const BITEMS_SCHEMA = { type:'object', additionalProperties:false, properties:{
  items:{ type:'array', items:{ type:'object', additionalProperties:false, properties:{
    id:{type:'string'}, category:{type:'string'}, title:{type:'string'},
    file:{type:'string'}, line:{type:'integer'},
    fixSummary:{type:'string'}, needsDecision:{type:'string'} },
    required:['id','category','title','file','line','fixSummary','needsDecision'] } } },
  required:['items'] }

const FIXMETHOD_SCHEMA = { type:'object', additionalProperties:false, properties:{
  id:{type:'string'},
  confirmedCurrentState:{type:'string'},
  fixSteps:{ type:'array', items:{type:'string'} },
  filesToTouch:{ type:'array', items:{type:'string'} },
  buildVerify:{type:'string'},
  risk:{type:'string', enum:['low','medium','high']},
  effort:{type:'string', enum:['S','M','L']},
  blockers:{type:'string'},
  confidence:{type:'integer'} },
  required:['id','confirmedCurrentState','fixSteps','filesToTouch','buildVerify','risk','effort','blockers','confidence'] }

phase('Load')
const loaded = await agent(
  'Load the list of REMAINING needs-decision (B) items from the planned-but-unimplemented audit. ' +
  'Primary source: the structured JSON at ' + JSON_SRC + ' (use result.needsDecision array; each entry has .finding{id,category,title,file,line} and .verdict{fixSummary,needsDecision}). ' +
  'If that file is missing/unreadable, parse the markdown at ' + REPORT + ' section 四 instead. ' +
  'EXCLUDE the items already fixed this session: F3 (RecordSafeDoorStates), F13 (S5F6 alarm list), F14 (mySMCmotor) and the A-region edMCUMaxQueue. ' +
  'Return every remaining item with id, category, title, file, line, fixSummary, needsDecision.',
  { label:'load:b-items', phase:'Load', schema: BITEMS_SCHEMA })
const items = (loaded && loaded.items) ? loaded.items : []
log('Loaded ' + items.length + ' remaining B items')

phase('Expand')
const expanded = await pipeline(items,
  (it) => agent(
    'Produce a CONCRETE, actionable fix method for this audit item. Item: ' + JSON.stringify(it) + '\n' +
    'HT160 root: ' + H160 + ' ; HT172 reference (read-only): ' + H172 + '.\n' +
    'Open the actual CURRENT HT160 code at file:line and the HT172 counterpart if relevant. Output: confirmedCurrentState (what the code is NOW, with file:line), fixSteps (numbered, exact files/functions/symbols, what to port from 172 adapted to HT160 procedural / no-FSM / AnsiString / ASCII-comment / no-C++11 style), filesToTouch, buildVerify (which .obj to delete + the build cmd scripts/ops/build-ht160s.ps1), risk, effort (S/M/L), blockers (decisions/hardware still needed), confidence. Do NOT apply anything.' + RO,
    { label:'exp:' + (it.id || 'item'), phase:'Expand', schema: FIXMETHOD_SCHEMA })
  .then((m) => ({ item:it, method:m }))
)

phase('Report')
const ok = expanded.filter(Boolean)
const rep = await agent(
  'Write a Traditional-Chinese deliverable to D:\\HT160S_BCB\\docs\\planned-not-implemented-fixmethods.md (UTF-8, no BOM) expanding the fix method for each remaining B item. ' +
  'Source: ' + JSON.stringify(ok) + '\n\n' +
  'Structure: 摘要 + a table (id | category | title | effort | risk | confidence | 仍需決策?) + per-item detailed sections grouped by category (stub, gap-hardware, orphan-ui, dead-config, unhandled-state, todo). Each item section: 位置(file:line), 現況, 修法步驟(numbered), 要動的檔, build驗證, 風險/工數, 仍需決策. ' +
  'Keep code identifiers / file:line ASCII; prose 繁體中文. WRITE ONLY that one .md; never edit source.' + RO +
  ' After writing, reply with the absolute path, item count, and a 6-line 繁中 summary of the quickest safe wins (low risk + S effort + no blockers).',
  { label:'report:fixmethods', phase:'Report' })

return { task:'B-region fix-method expansion', items: ok.length, report: rep }
