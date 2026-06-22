export const meta = {
  name: 'pad-172-alignment',
  description: 'Step-by-step per-function comparison of HT160 Pad subsystem vs stable HT172; flag every parse/operation divergence with verdict + alignment recommendation',
  whenToUse: 'Align HT160 uPadInterface + ComPort Pad path to HT172 function-by-function, avoiding whole-subsystem misjudgment.',
  phases: [
    { title: 'Enumerate', detail: 'List every Pad function pair (HT160 vs HT172)' },
    { title: 'Compare',   detail: 'One agent per function pair, line-by-line parse/operation diff' },
    { title: 'Verify',    detail: 'Adversarially verify each behavior-affecting divergence + whether aligning is safe' },
    { title: 'Report',    detail: 'Write docs/pad-172-alignment-report.md' },
  ],
}

const H160 = 'D:\\HT160S_BCB\\HT160S_Program_BCB_V1.0.0.0'
const H172 = 'D:\\HT172\\HT172_Program_V1.0.25.0_20260420'
const RO = ' READ-ONLY: never edit any source file; HT172 is read-only; reads only.'
const KNOWN =
  ' KNOWN FACTS (do NOT re-flag): (1) ProcessReceiceData input-length was ALREADY FIXED >=14 -> >=13 (off-by-one that dropped 13-char frames); treat >=13 as correct. (2) TX/LED-set works and version handshake (t051120 -> t05122065 via the >=8 branch) works. (3) Intentional HT160 deviations - do NOT recommend reverting: iControlPanelMode==1 gate removed and replaced by hardware PAD_PannelEnable key; MN200 IO hardware (not the HT172 card); NO FSM allowed in HT160 (procedural / switch(Task) only); exact-equality Pad key-name matching. Focus on PARSE SEMANTICS and OPERATION correctness, not coding style.'

const PAIR_SCHEMA = { type:'object', additionalProperties:false, properties:{
  functions:{ type:'array', items:{ type:'object', additionalProperties:false, properties:{
    name:{type:'string'}, h160:{type:'string',description:'file:line in HT160 or "none"'},
    h172:{type:'string',description:'file:line in HT172 or "none"'}, note:{type:'string'} },
    required:['name','h160','h172','note'] } } }, required:['functions'] }

const CMP_SCHEMA = { type:'object', additionalProperties:false, properties:{
  name:{type:'string'},
  alignmentStatus:{type:'string', enum:['aligned','divergent','ht160-only','ht172-only']},
  summary:{type:'string'},
  divergences:{ type:'array', items:{ type:'object', additionalProperties:false, properties:{
    id:{type:'string'}, what:{type:'string'}, h160line:{type:'string'}, h172line:{type:'string'},
    affectsBehavior:{type:'boolean'},
    recommendation:{type:'string', enum:['align-to-172','keep-ht160','needs-decision']},
    why:{type:'string'} }, required:['id','what','h160line','h172line','affectsBehavior','recommendation','why'] } } },
  required:['name','alignmentStatus','summary','divergences'] }

const VERDICT_SCHEMA = { type:'object', additionalProperties:false, properties:{
  id:{type:'string'}, isReal:{type:'boolean'}, confidence:{type:'integer'},
  shouldAlignToHT172:{type:'boolean'}, risk:{type:'string', enum:['low','medium','high']},
  fix:{type:'string'} }, required:['id','isReal','confidence','shouldAlignToHT172','risk','fix'] }

phase('Enumerate')
const en = await agent(
  'List every Pad-related function PAIR to compare between HT160 (' + H160 + ') and HT172 (' + H172 + '). ' +
  'Cover uPadInterface.cpp/.h: DoScanPanelLed, DoUpdataPadStatus, ProcessReceiceData, ProcessSendDataNew, ProcessScanKey, Main232, RequestPadVersion, SendCommand, RecordLocalStatusCommand, NormalizePadInputName, IsPadKey, SyncHSysPadInputStatus, SyncHSysPadSwitchStatus, ResetComm, FormShow/init; and the Pad path in ComPort.cpp: the Pad receive callback (PadCommReceiveData / OnReceiveData), RS232Init, ConfigurePadComm, OpenWorkFile, StopPadCom. For each give the function name and file:line in BOTH trees (or "none" if only one side has it). Include any extra Pad functions you find.' + RO,
  { label:'enum:pad-fns', phase:'Enumerate', schema: PAIR_SCHEMA })
const fns = (en && en.functions) ? en.functions : []
log('Enumerated ' + fns.length + ' Pad function pairs')

phase('Compare')
const compared = await pipeline(fns,
  (fn) => agent(
    'Carefully compare this ONE Pad function between HT160 and HT172, LINE BY LINE. Read BOTH full implementations. Function: ' + JSON.stringify(fn) + '\n' +
    'HT160 root: ' + H160 + ' ; HT172: ' + H172 + '.\n' +
    'Report alignmentStatus and, for each divergence: what differs, h160line, h172line, whether it affects DATA PARSING or OPERATION (affectsBehavior), a recommendation (align-to-172 / keep-ht160 / needs-decision) and why. Be meticulous about: SubString positions/indices, length thresholds, hex parsing, frame split/Trim/\\r handling, loop termination, send framing, address/Tag mapping. Do NOT jump to conclusions; cite exact lines.' + KNOWN + RO,
    { label:'cmp:' + (fn.name || 'fn'), phase:'Compare', schema: CMP_SCHEMA }),
  (cmp, fn) => {
    const divs = (cmp && cmp.divergences) ? cmp.divergences : []
    const flagged = divs.filter((d) => d.affectsBehavior)
    if (!flagged.length) return { fn:fn, cmp:cmp, verdicts:[] }
    return parallel(flagged.map((d) => () =>
      agent(
        'Adversarially VERIFY this HT160-vs-HT172 Pad divergence and whether aligning HT160 to HT172 is CORRECT and SAFE (some HT160 deviations are intentional and better). Try to refute. Divergence: ' + JSON.stringify(d) + '\n' +
        'Function: ' + (fn.name || '') + '\nHT160: ' + H160 + ' ; HT172: ' + H172 + '.\n' +
        'Decide isReal, confidence, shouldAlignToHT172, risk, and a concrete HT160-side fix (procedural, no FSM, AnsiString, ASCII comments) - do NOT apply.' + KNOWN + RO,
        { label:'vfy:' + (fn.name || '') + ':' + (d.id || ''), phase:'Verify', schema: VERDICT_SCHEMA })
      .then((v) => ({ div:d, verdict:v }))
    )).then((vs) => ({ fn:fn, cmp:cmp, verdicts:vs.filter(Boolean) }))
  }
)

phase('Report')
const safe = compared.filter(Boolean)
const rep = await agent(
  'Write a Traditional-Chinese alignment report to D:\\HT160S_BCB\\docs\\pad-172-alignment-report.md (UTF-8, no BOM). ' +
  'Source (per-function comparison + verdicts): ' + JSON.stringify(safe) + '\n\n' +
  'Structure: (1) 摘要 + how this per-function method avoids the earlier whole-subsystem misjudgment; (2) per-function alignment table (function | status | #behavior-affecting divergences | recommendation); (3) detailed sections: A) 可安全對齊 172 (shouldAlignToHT172=true, high confidence) each with concrete fix + file:line; B) 需決策; C) HT160 刻意差異(保留) incl. iControlPanelMode->hardware key, the already-fixed >=13, MN200, no-FSM. ' +
  'Keep code identifiers / file:line ASCII; prose 繁體中文. WRITE ONLY that one .md; never edit source.' + RO +
  ' After writing, reply with the absolute path and a 6-line 繁中 summary naming the top safe-to-align items.',
  { label:'report:pad-align', phase:'Report' })

return { task:'HT160 Pad vs HT172 per-function alignment', functionPairs: safe.length, report: rep }
