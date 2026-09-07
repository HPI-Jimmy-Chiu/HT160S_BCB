export const meta = {
  name: 'ht160s-home-selftest',
  description: 'Build HT160S sim, run the --selftest-home headless full-machine HOME, judge the trace, and (on failure) PROPOSE a minimal fix diff for human review',
  whenToUse: 'Regression-check the full-machine HOME after touching home/motor/driver code. Goal: every Mot_Table-enabled axis reaches HOMED, M13/M18 stay disabled.',
  phases: [
    { title: 'Build',   detail: 'build-ht160s.ps1 (sim build)' },
    { title: 'RunSim',  detail: 'ht160s.exe --selftest-home, capture exit code + HomeTrace.log' },
    { title: 'Judge',   detail: 'parse trace: all enabled axes HOMED, disabled not run' },
    { title: 'Propose', detail: 'on failure only: propose a minimal fix diff (NOT applied)' },
  ],
}

const ROOT = 'D:/HT160S_BCB'
const EXE = ROOT + '/EXE/ht160s.exe'
const LOG = ROOT + '/EXE/HomeTrace.log'
const BUILD = ROOT + '/scripts/ops/build-ht160s.ps1'

const BUILD_SCHEMA = {
  type: 'object', additionalProperties: false,
  required: ['ok', 'summary'],
  properties: {
    ok: { type: 'boolean', description: 'true if build exit code 0 and no Error E*/Fatal lines' },
    summary: { type: 'string', description: 'one-line result; on failure include the first error line' },
  },
}
const RUN_SCHEMA = {
  type: 'object', additionalProperties: false,
  required: ['exitCode', 'logExists', 'logText'],
  properties: {
    exitCode: { type: 'integer', description: 'process exit code: 0=home completed, 2=timeout, 1=did not complete' },
    logExists: { type: 'boolean' },
    logText: { type: 'string', description: 'full text of HomeTrace.log' },
  },
}
const JUDGE_SCHEMA = {
  type: 'object', additionalProperties: false,
  required: ['pass', 'enabledHomed', 'failures', 'notes'],
  properties: {
    pass: { type: 'boolean' },
    enabledHomed: { type: 'integer', description: 'count of EN axes shown HOMED' },
    failures: {
      type: 'array',
      items: {
        type: 'object', additionalProperties: false,
        required: ['axis', 'reason'],
        properties: { axis: { type: 'string' }, reason: { type: 'string' } },
      },
    },
    notes: { type: 'string' },
  },
}
const PROPOSE_SCHEMA = {
  type: 'object', additionalProperties: false,
  required: ['rootCause', 'files', 'diff'],
  properties: {
    rootCause: { type: 'string' },
    files: { type: 'array', items: { type: 'string' } },
    diff: { type: 'string', description: 'unified diff of the proposed minimal fix; NOT applied' },
  },
}

phase('Build')
const build = await agent(
  `Build the HT160S simulation. Using the PowerShell tool, run:\n` +
  `  & "${BUILD}" -Clean\n` +
  `Wait for it to finish. The build is CLEAN only if the final exit code is 0 and there are no "Error E" or "Fatal" lines (W8066 "Unreachable code" warnings are EXPECTED and harmless - they come from SOFT_SIMULATE early-returns). ` +
  `If the link step fails with an undefined/unresolved symbol that looks like a header change, re-run with -Full instead and report that. Return the verdict.`,
  { phase: 'Build', schema: BUILD_SCHEMA }
)
if (!build.ok) {
  log('Build FAILED: ' + build.summary)
  return { stage: 'build', pass: false, detail: build.summary }
}
log('Build clean.')

phase('RunSim')
const run = await agent(
  `Run the HT160S headless HOME self-test. Using the PowerShell tool:\n` +
  `  1. If "${LOG}" exists, delete it.\n` +
  `  2. $p = Start-Process -FilePath "${EXE}" -ArgumentList "--selftest-home" -PassThru ; then $p.WaitForExit(60000).\n` +
  `  3. If it did NOT exit within 60s, $p.Kill() and report exitCode 1.\n` +
  `  4. Otherwise report $p.ExitCode (0=home completed, 2=timeout).\n` +
  `Then Read "${LOG}" and return its FULL text verbatim in logText.`,
  { phase: 'RunSim', schema: RUN_SCHEMA }
)
log('Sim exit code ' + run.exitCode + ', log present=' + run.logExists)

phase('Judge')
const judge = await agent(
  `Judge this full-machine HOME self-test trace. GOAL: every Mot_Table-ENABLED axis must reach HOMED; the only disabled axes are M13 MBottomCCDY and M18 MPitchX, which must NOT run.\n\n` +
  `Process exit code was ${run.exitCode} (0=home completed).\n\nTRACE:\n${run.logText}\n\n` +
  `PASS requires ALL of: (a) the trace ends with "HOME ROUND DONE"; (b) in "ALL MOTOR HOME STATUS" every axis marked "EN" is also "HOMED"; (c) exactly M13 and M18 are "-- NOThomed"; (d) exit code 0. ` +
  `List any axis that is EN but not HOMED, or any "--" axis other than M13/M18, as a failure.`,
  { phase: 'Judge', schema: JUDGE_SCHEMA }
)

if (judge.pass) {
  log('PASS - ' + judge.enabledHomed + ' enabled axes HOMED, M13/M18 correctly skipped.')
  return { stage: 'judge', pass: true, enabledHomed: judge.enabledHomed, notes: judge.notes }
}

phase('Propose')
log('FAIL - ' + judge.failures.length + ' axis issue(s); proposing a fix diff (NOT applying).')
const proposal = await agent(
  `The HOME self-test FAILED. Failing axes/reasons:\n${JSON.stringify(judge.failures, null, 2)}\n\n` +
  `Judge notes: ${judge.notes}\n\nFull trace:\n${run.logText}\n\n` +
  `Investigate the HT160S source (READ ONLY - do not edit any file): ProcessMotorHome in HT160S_Program_BCB_V1.0.0.0/uHome.cpp, the MC88X1 driver HT160S_Program_BCB_V1.0.0.0/MotorAndIO/myMC88X1motor.cpp, and motor setup in HT160S_Program_BCB_V1.0.0.0/database.cpp. ` +
  `Identify the most likely root cause and PROPOSE a minimal fix as a unified diff. Constraints: do NOT modify files; new comments ASCII English only; prefer changes confined to SOFT_SIMULATE paths unless the root cause is clearly outside them. Return rootCause, files, and the diff.`,
  { phase: 'Propose', schema: PROPOSE_SCHEMA }
)
return {
  stage: 'propose', pass: false,
  failures: judge.failures,
  rootCause: proposal.rootCause,
  files: proposal.files,
  diff: proposal.diff,
}
