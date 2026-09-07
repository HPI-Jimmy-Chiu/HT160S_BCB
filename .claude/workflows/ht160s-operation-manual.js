export const meta = {
  name: 'ht160s-operation-manual',
  description: 'Generate the HT160S full engineering operation manual (Traditional Chinese Markdown) by reading every screen form + process module, modeled on the HT9045/9046 operation-manual structure and customized for HT160S.',
  whenToUse: 'Produce or regenerate the HT160S operation manual after UI/feature changes. Fans out one reader per screen + per process module into structured specs, then one writer per chapter into grounded Markdown, then a completeness critic.',
  phases: [
    { title: 'Read',    detail: 'one reader per screen form + per process module -> structured spec (source-grounded)' },
    { title: 'Write',   detail: 'one writer per manual chapter -> Traditional Chinese Markdown from the specs' },
    { title: 'Critique', detail: 'completeness critic flags gaps / unverified claims across all chapters' },
  ],
}

// ---------------------------------------------------------------------------
// Paths
// ---------------------------------------------------------------------------
const SRC = 'D:/HT160S_BCB/HT160S_Program_BCB_V1.0.0.0'
const SG  = SRC + '/SecsGem'
const SHOT = 'screenshots' // relative path used inside the generated Markdown

// ---------------------------------------------------------------------------
// READER UNITS - each becomes one reader subagent producing one SPEC.
// kind 'screen' = a VCL form the operator sees; 'module' = a process/control
// module with no direct UI (documented as operation flow).
// big .dfm files must be read SELECTIVELY (grep / partial Read) - they exceed
// the read limit. The reader is told this.
// ---------------------------------------------------------------------------
const UNITS = [
  { id: 'main', kind: 'screen', title: 'Main screen / 主畫面',
    files: ['main.dfm', 'main.cpp', 'main.h'], big: ['main.dfm'],
    focus: 'Top toolbar speed-buttons (Language/Product/Maintance/Offset/Speed/Tools/Message/Monitor/Exit), the run buttons, the PageControl tabs (Main, Lot, Tray Status, Logs, Time Data, Map Tray), the production counters (Total/Fail/Summary, Unload Auto1..6), the Recipe Name / User fields, and the status indicators (HALT, SECS ON/OFF, SAFE, NORMAL, AMR ON/OFF, Real/Dummy, Start Mode). For each control give its on-screen label and what pressing/reading it does.' },

  { id: 'runcontrol', kind: 'module', title: 'Run control & machine lifecycle / 啟動停止生命週期',
    files: ['uruncontrol.cpp', 'uruncontrol.h', 'csystem.cpp', 'uHome.cpp', 'uHome.dfm', 'uHome.h'], big: ['csystem.cpp'],
    focus: 'The START / PAUSE / STOP / HOME lifecycle, the SystemStart flag and the MachineStart/Pause/Stop/HomeAbort command layer, the HOME monitor form (TfHome), Real vs Dummy mode, Start Mode (Normal/Initial/...), the safety door + EMG gating, and which faults silently drop SystemStart vs pop an alarm. Describe the operator-visible sequence of powering on -> login -> HOME -> START.' },

  { id: 'maintenance', kind: 'screen', title: 'Maintenance / Tools screen / 維護畫面',
    files: ['maintenance.dfm', 'maintenance.cpp', 'maintenance.h'], big: ['maintenance.dfm'],
    focus: 'Every tab/panel of the maintenance form: tower-light + buzzer (Music Select) settings, heating row, hardware enable settings (per-sucker enable, per-Auto enable, Color CCD enable), and any manual actuator/test controls. Note which settings are saved to config and which fire immediately.' },

  { id: 'setup', kind: 'screen', title: 'Setup / Config screen / 設定畫面',
    files: ['setup.dfm', 'setup.cpp', 'setup.h'], big: ['setup.dfm'],
    focus: 'The machine configuration screen: the major configuration groups/tabs, the parameters the user can set, recipe/product selection if present here, and the config tiers. setup.dfm is ~1MB - use Grep to list the TTabSheet/TGroupBox/TPanel captions and the labelled edit fields rather than reading the whole file.' },

  { id: 'product', kind: 'module', title: 'Product / Recipe & By Lot+Bin sort mode / 配方與分流',
    files: ['cprod.cpp'], big: ['cprod.cpp'],
    focus: 'Recipe/product data model and selection, the sorting Bin->Auto mapping, and the dynamic By (LotID,Bin)->Auto sort mode (per-Auto enable, hardware edit lock, inherit-record prompt). Describe how an operator picks/edits a recipe and how Bin assignment to output stackers works.' },

  { id: 'teach', kind: 'screen', title: 'Teach screen / 教導畫面',
    files: ['uteach.dfm', 'uteach.cpp', 'uteach.h'], big: [],
    focus: 'The teach screen: motor/axis selection, jog +/- (and the +/right -/left mapping), teach-position list and how a position is taught/saved, the on-screen keypad, HOME step display, and the suck/home interlocks enforced while teaching. Positions are stored in 1/100mm (100 units/mm).' },

  { id: 'offset', kind: 'screen', title: 'Offset screen / 偏移畫面',
    files: ['uOffset.dfm', 'uOffset.cpp', 'uOffset.h'], big: [],
    focus: 'The Offset screen: which axes/positions can be offset, how an offset value is entered and applied (effective = base teach + offset, applied at apply-time fold), and the relationship between .tech base positions, .ofs offset files, and effective positions.' },

  { id: 'speed', kind: 'screen', title: 'Speed screen / 速度畫面',
    files: ['uspeed.dfm', 'uspeed.cpp', 'uspeed.h'], big: [],
    focus: 'The Speed & accel/decel settings screen: which axes/conditions have configurable speed, accel, decel, and how values are entered and saved.' },

  { id: 'iosetview', kind: 'screen', title: 'I/O monitor (IOsetview) / 輸出入監看',
    files: ['iosetview.dfm', 'iosetview.cpp', 'iosetview.h'], big: ['iosetview.dfm'],
    focus: 'The I/O monitor screen: how inputs and outputs are grouped (by station/board), the LED color convention, how an output can be forced/toggled (and any safety gating on that), the IO Table view, and the MN200/IO address model. iosetview.dfm is ~475KB - Grep for the group captions and LED/label names instead of full read.' },

  { id: 'motortest', kind: 'screen', title: 'Motor Test screen / 馬達測試畫面',
    files: ['uMotorTest.dfm', 'uMotorTest.cpp', 'uMotorTest.h'], big: ['uMotorTest.cpp'],
    focus: 'The Motor Test screen: per-axis selection, jog +/-, HOME, move-to-position, the limit/alarm LED behavior, HomeType per motor, and the LED color convention (green=in-use&triggered, gray=in-use&idle, red=not-in-use). Note any servo power-cycle / alarm-clear behavior surfaced here.' },

  { id: 'secs', kind: 'screen', title: 'SECS/GEM interface / SECS-GEM 介面',
    files: ['UsecegemMainFrom.cpp', 'UsecegemMainFrom.h', 'uHGemHT160.cpp', 'uHGemHT160.h', 'uHGemEquipment.h', 'uHGemLogForm.h'],
    dir: SG, big: ['UsecegemMainFrom.cpp', 'uHGemHT160.cpp'],
    focus: 'The SECS/GEM screen and host-communication features: online/offline + local/remote control state, the SECS ON/OFF indicator on main, key CEIDs/events reported (e.g. CEID272), the message/log view, and what the operator can configure (device IDs, communication enable). Keep it operator/integrator level.' },

  { id: 'agv', kind: 'module', title: 'AMR / AGV station (E87) / 無人車站',
    files: ['uAgvStation.cpp', 'uAgvStation.h'], dir: SG, big: ['uAgvStation.cpp'],
    focus: 'The AMR/AGV coordination: the AMR ON/OFF state on main, the Ready/Finish handshake (P1-P3), full-car/shortage reporting (CEID272), START_AGV, the TrayArm cylinder lock during car exchange, and the car-taken sensor. Describe the operator/AMR interaction flow.' },

  { id: 'alarms', kind: 'module', title: 'Alarm / Note message system / 警報訊息系統',
    files: ['note.cpp', 'note.dfm', 'note.h', 'cmydef.cpp', 'cmydef.h'], big: ['cmydef.cpp', 'cmydef.h'],
    focus: 'The alarm/message subsystem: the TfNote full-alarm popup vs ShowMyOKMessage (OK) vs ShowMyMessageBox_YES_NO, how a Note is dismissed (recovery-key gated unless info note KeyCode==0), the buzzer/tower-light tie-in, and the catalogue of alarm codes/messages defined in cmydef (list representative alarm names + their meaning + recovery action). This feeds the alarm-recovery chapter.' },

  { id: 'loader', kind: 'module', title: 'Loader module / 進料模組',
    files: ['aLoader.cpp', 'aLoader.h'], big: ['aLoader.cpp'],
    focus: 'The Loader (input) module flow: how a source tray is received, 2D/ID read, indexed, and fed; the Loader Y safe distance; the task/step states; and the operator-visible behavior on main (Loader 2D Left/Right, Loader ID, Current Sorting Bin).' },

  { id: 'sortarm', kind: 'module', title: 'SortArm + TrayArm modules / 分類臂與盤臂',
    files: ['aSortArm.cpp', 'aSortArm.h', 'aTrayArm.cpp', 'aTrayArm.h'], big: ['aSortArm.cpp', 'aTrayArm.cpp'],
    focus: 'SortArm: the suckers, the SortArmX-move all-suckers-home interlock, pick/place flow. TrayArm: the Z-up interlock before X moves, tray pickup/place. Describe the sort/transport sequence and the anti-collision interlocks (which bypass only compile-time SOFT_SIMULATE, never runtime DUMMY).' },

  { id: 'color', kind: 'module', title: 'Color module / 顏色模組',
    files: ['aColor.cpp', 'aColor.h'], big: ['aColor.cpp'],
    focus: 'The Color module: identity-tray supply, the ColorY front/back transport (front receive+2D read, then move to rear for TrayArm pickup; two taught positions ColorRead2DYPosition + ColorTrayArmPickYPosition), the AMR-demand supply gate (bSupplyRequested), and the CCD 2D/color read.' },

  { id: 'empty', kind: 'module', title: 'Empty-tray module / 空盤模組',
    files: ['aEmpty.cpp', 'aEmpty.h'], big: ['aEmpty.cpp'],
    focus: 'The Empty-tray module: empty-tray supply/handling, the dual-cylinder clamp (DoClampTray lean-stop-first/push-last), the tray-state action latches (tray-down=has-tray, MotorY-takes=no-tray), and operator-visible behavior.' },

  { id: 'auto', kind: 'module', title: 'Auto1-6 output stackers / 出料堆疊',
    files: ['aAuto1To6.cpp', 'aAuto1To6.h'], big: ['aAuto1To6.cpp'],
    focus: 'The Auto1..6 output stacker modules: how sorted trays are stacked into outputs, per-Auto enable, full/shortage detection, the Unload Auto1..6 counters on main, and how Bin->Auto routing reaches them.' },
]

// ---------------------------------------------------------------------------
// CHAPTERS - each becomes one writer subagent. units[] selects which SPECs it
// receives. ref = the analogous HT9045 chapter (for STRUCTURE/STYLE only -
// content must be HT160S-specific). shots[] = screenshot placeholders to embed.
// ---------------------------------------------------------------------------
const CHAPTERS = [
  { no: '01', file: '01-safety.md', title: '安全須知', units: ['runcontrol', 'maintenance'],
    ref: 'HT9045 Ch1 安全須知 (安全規則 / 警告標誌 / 安全防護功能)',
    shots: [{ f: 'tower-light.png', cap: '三色塔燈與蜂鳴器' }],
    guidance: 'General machine-safety chapter: operator safety rules, warning-label meanings, and the machine SAFETY features that exist in HT160S - emergency stop (EMG), safety door interlock, the three-color tower light + buzzer states, and the principle that anti-collision interlocks always run (only compile-time SOFT_SIMULATE bypasses them). Keep generic safety advice clearly separated from HT160S-specific protections. Where a specific label/threshold is unknown, mark 待補.' },

  { no: '02', file: '02-overview.md', title: '系統概觀與機構', units: ['loader', 'sortarm', 'color', 'empty', 'auto', 'main'],
    ref: '(無直接對應；綜合各模組做機台總覽)',
    shots: [{ f: 'main-overview.png', cap: '主畫面總覽' }],
    guidance: 'High-level introduction: what an HT160S does (tray sorting / handling machine), the main mechanical modules (Loader 進料, Color 顏色, Empty 空盤, SortArm 分類臂 + suckers, TrayArm 盤臂, Auto1-6 出料堆疊, TopCCD), and the overall material flow from input tray -> 2D/color read -> sort by Bin -> output stacker. A simple ASCII/Markdown flow diagram of the material path is welcome. Do not deep-dive each module here (that is Chapter 14) - this is orientation only.' },

  { no: '03', file: '03-panel-startup.md', title: '操作面板與開機啟動', units: ['main', 'runcontrol'],
    ref: 'HT9045 Ch2 操作面板 + Ch4 主畫面 (啟動相關)',
    shots: [{ f: 'main-overview.png', cap: '主畫面與工具列' }, { f: 'screen-home.png', cap: 'HOME 復歸監看畫面' }],
    guidance: 'The operator panel and the day-1 startup procedure: power on, login (User), perform HOME (復歸) and what the HOME monitor shows, then START / PAUSE / STOP. Document the toolbar speed-buttons, the Real/Dummy toggle, the Start Mode selector, and every status indicator (HALT, SECS, SAFE, NORMAL, AMR). Give a numbered "首次開機到開始生產" procedure. Cross-reference Chapter 13 for what happens on a fault.' },

  { no: '04', file: '04-main-screen.md', title: '主畫面詳解', units: ['main'],
    ref: 'HT9045 Ch3 View Menu + Ch4 主畫面',
    shots: [
      { f: 'main-overview.png', cap: 'Main 分頁' },
      { f: 'main-lot.png', cap: 'Lot 分頁' },
      { f: 'main-traystatus.png', cap: 'Tray Status 分頁' },
      { f: 'main-logs.png', cap: 'Logs 分頁' },
      { f: 'main-timedata.png', cap: 'Time Data 分頁' },
      { f: 'main-maptray.png', cap: 'Map Tray 分頁' },
    ],
    guidance: 'Walk through each PageControl tab on the main screen (Main, Lot, Tray Status, Logs, Time Data, Map Tray): what each tab shows and the controls on it. Document the production counters (Total / Fail / Summary, Unload Auto1..6), the Recipe Name and User fields. One subsection per tab, each with its own screenshot placeholder.' },

  { no: '05', file: '05-maintenance.md', title: '維護畫面 (Maintenance)', units: ['maintenance'],
    ref: 'HT9045 Ch5 Tools',
    shots: [{ f: 'screen-maintenance.png', cap: '維護畫面' }],
    guidance: 'Document the Maintenance screen tab-by-tab: tower-light + buzzer (Music Select) configuration, the (hidden) heating row, hardware enable settings (per-sucker, per-Auto, Color CCD), and any manual actuator/test controls. Use a table per group: control | 設定值/動作 | 說明. Note which settings persist to config vs fire immediately.' },

  { no: '06', file: '06-config.md', title: '設定 (Config / Setup)', units: ['setup', 'product'],
    ref: 'HT9045 Ch6 Config',
    shots: [{ f: 'screen-setup.png', cap: '設定畫面' }, { f: 'screen-product.png', cap: '配方/產品設定' }],
    guidance: 'Document the Setup/Config screen: each configuration group/tab and its parameters, and the recipe/product management + Bin->Auto sort mapping. Use tables for parameter lists (參數 | 範圍/預設 | 說明). Cover the config tiers. Where a parameter meaning is not certain from source, mark 待補 rather than guessing.' },

  { no: '07', file: '07-teach.md', title: '教導 (Teach)', units: ['teach'],
    ref: 'HT9045 (對應座標教導概念)',
    shots: [{ f: 'screen-teach.png', cap: '教導畫面' }],
    guidance: 'Document the Teach screen: selecting an axis, jogging (note the +/right -/left mapping), teaching and saving a position, the on-screen keypad, the HOME step display, and the suck/all-suckers-home interlock active during teach. State clearly that positions are stored in 1/100mm (100 units = 1mm). Give a numbered "如何教導一個位置" procedure with a safety warning callout.' },

  { no: '08', file: '08-offset.md', title: '偏移 (Offset)', units: ['offset'],
    ref: 'HT9045 Ch7 Offset',
    shots: [{ f: 'screen-offset.png', cap: '偏移畫面' }],
    guidance: 'Document the Offset screen: which positions can be offset, entering/applying an offset, and the model 有效值 = 教導基準 (.tech) + 偏移 (.ofs), folded at apply time. Explain when to use Offset vs re-Teach. Numbered procedure + warning callout.' },

  { no: '09', file: '09-speed.md', title: '速度 (Speed)', units: ['speed'],
    ref: 'HT9045 Ch8 Speed',
    shots: [{ f: 'screen-speed.png', cap: '速度畫面' }],
    guidance: 'Document the Speed screen: the per-axis / per-condition Speed, Accel, Decel settings and how to change and save them. Table: 軸/條件 | 速度 | 加速 | 減速 | 說明. Warn against setting unsafe speeds.' },

  { no: '10', file: '10-io.md', title: '輸出入監看 (I/O)', units: ['iosetview'],
    ref: 'HT9045 Ch9 I/O',
    shots: [{ f: 'screen-iosetview.png', cap: 'I/O 監看畫面' }],
    guidance: 'Document the I/O monitor: how inputs/outputs are grouped (by station/board), the LED color convention, how to read a sensor state and (if allowed) force an output, the IO Table view, and the MN200/IO address model (Lane=ring). Strong safety callout about forcing outputs on a live machine.' },

  { no: '11', file: '11-motor-test.md', title: '馬達測試 (Motor Test)', units: ['motortest'],
    ref: '(HT160S 專屬畫面)',
    shots: [{ f: 'screen-motortest.png', cap: '馬達測試畫面' }],
    guidance: 'Document the Motor Test screen: selecting an axis, jog +/-, HOME, move-to-position, the limit/alarm LED behavior and the LED color convention, HomeType per motor, and the servo alarm-clear behavior (latched alarms clear only via SwMotorRelay Off->On / power-cycle). Strong safety callout: motors physically move even in DUMMY.' },

  { no: '12', file: '12-secs-amr.md', title: 'SECS/GEM 與 AMR/AGV', units: ['secs', 'agv'],
    ref: 'HT9045 Ch13 測試機介面 (通訊介面概念)',
    shots: [{ f: 'secs-main.png', cap: 'SECS/GEM 畫面' }],
    guidance: 'Document host communication: the SECS/GEM screen (online/offline, local/remote, the SECS ON/OFF main indicator, key events/CEIDs, the message log), and the AMR/AGV station - the AMR ON/OFF state, the Ready/Finish handshake (P1-P3), full-car/shortage reporting (CEID272), START_AGV, and the TrayArm lock during car exchange. Integrator-level; mark device-id/count specifics as 待補 (hardware-dependent).' },

  { no: '13', file: '13-alarms.md', title: '警報訊息與排除', units: ['alarms', 'runcontrol'],
    ref: 'HT9045 Ch10 Message + 錯誤訊息與排除手冊',
    shots: [],
    guidance: 'Document the alarm system and recovery: the difference between the full-alarm Note popup, the OK message box, and the Yes/No box; how an alarm is dismissed (must pick a recovery key unless it is an info note); the buzzer/tower-light behavior on alarm; and a TABLE of representative alarm messages with 原因 and 排除方式 (use the alarm catalogue from the spec - do NOT invent codes; list only those present, and mark the list as representative/not exhaustive). Add the iron rule: any fault that drops SystemStart pops an alarm.' },

  { no: '14', file: '14-module-flows.md', title: '各模組運作流程', units: ['loader', 'sortarm', 'color', 'empty', 'auto'],
    ref: 'HT9045 Ch11 工作時間序 (時序/做法概念)',
    shots: [],
    guidance: 'Deep-dive each process module as an operation flow: Loader 進料, Color 顏色, Empty 空盤, SortArm 分類臂(+suckers) / TrayArm 盤臂, Auto1-6 出料堆疊. One section per module: 功能 / 動作時序 (numbered or arrow-flow) / 互鎖與安全 / 操作員可見狀態. Emphasize the anti-collision interlocks. This is the engineering heart of the manual.' },

  { no: '15', file: '15-lotbin-mode.md', title: 'By Lot+Bin 分流模式', units: ['product'],
    ref: 'HT9045 Ch15 Color Tray (專屬功能設定的對應)',
    shots: [{ f: 'main-lot.png', cap: 'Lot 分頁與分流設定' }],
    guidance: 'Document the HT160S-specific By (LotID,Bin)->Auto dynamic sort mode: what it does, when to use it, the per-Auto enable, the hardware-edit lock, and the inherit-record prompt. Numbered "如何設定一筆 Lot+Bin 分流" procedure. Cross-reference Chapter 6 (recipe) and Chapter 4 (Lot tab).' },

  { no: '16', file: '16-faq.md', title: '常見問題 (FAQ)', units: ['alarms', 'runcontrol', 'loader', 'sortarm'],
    ref: 'HT9045 Ch16 FAQ',
    shots: [],
    guidance: 'Produce a FAQ of likely operator questions for HT160S, derived ONLY from the provided specs (startup/HOME, why a module stalls, how to clear a servo alarm, what a status indicator means, how to switch Real/Dummy, etc.). Q/A format. For any answer not fully supported by the specs, phrase it conservatively and append 待現場確認. Do NOT fabricate procedures.' },
]

// ---------------------------------------------------------------------------
// Schemas
// ---------------------------------------------------------------------------
const SPEC_SCHEMA = {
  type: 'object', additionalProperties: false,
  required: ['id', 'title', 'purpose', 'controls', 'procedures', 'flow', 'interlocks', 'settings', 'alarms', 'unknowns'],
  properties: {
    id: { type: 'string' },
    title: { type: 'string', description: 'screen/module title' },
    purpose: { type: 'string', description: '2-4 sentence purpose, in Traditional Chinese' },
    controls: {
      type: 'array', description: 'UI controls (screens) - empty for pure modules',
      items: {
        type: 'object', additionalProperties: false,
        required: ['name', 'kind', 'label', 'function'],
        properties: {
          name: { type: 'string', description: 'component/identifier name' },
          kind: { type: 'string', description: 'button/edit/grid/led/tab/checkbox/...' },
          label: { type: 'string', description: 'on-screen caption; "[中文標籤]" if non-ASCII/garbled' },
          function: { type: 'string', description: 'what it does, Traditional Chinese' },
        },
      },
    },
    procedures: {
      type: 'array', description: 'operator procedures',
      items: {
        type: 'object', additionalProperties: false,
        required: ['name', 'steps'],
        properties: {
          name: { type: 'string' },
          steps: { type: 'array', items: { type: 'string' } },
          notes: { type: 'string' },
        },
      },
    },
    flow: { type: 'array', description: 'action/state sequence for modules (Traditional Chinese)', items: { type: 'string' } },
    interlocks: { type: 'array', description: 'interlocks / safety conditions (Traditional Chinese)', items: { type: 'string' } },
    settings: {
      type: 'array', description: 'configurable parameters/settings',
      items: {
        type: 'object', additionalProperties: false,
        required: ['name', 'meaning'],
        properties: {
          name: { type: 'string' },
          meaning: { type: 'string' },
          range_or_default: { type: 'string' },
        },
      },
    },
    alarms: {
      type: 'array', description: 'alarm/message codes or names present in source (do not invent)',
      items: {
        type: 'object', additionalProperties: false,
        required: ['name', 'meaning'],
        properties: {
          name: { type: 'string' },
          meaning: { type: 'string' },
          recovery: { type: 'string' },
        },
      },
    },
    unknowns: { type: 'array', description: 'things that could not be determined from source and need field confirmation', items: { type: 'string' } },
  },
}

const CHAPTER_SCHEMA = {
  type: 'object', additionalProperties: false,
  required: ['no', 'file', 'title', 'markdown', 'unknowns'],
  properties: {
    no: { type: 'string' },
    file: { type: 'string' },
    title: { type: 'string' },
    markdown: { type: 'string', description: 'full Traditional Chinese Markdown body of this chapter (starts with a single H1)' },
    unknowns: { type: 'array', items: { type: 'string' }, description: 'items marked 待補 in this chapter' },
  },
}

const CRITIQUE_SCHEMA = {
  type: 'object', additionalProperties: false,
  required: ['overall', 'gaps'],
  properties: {
    overall: { type: 'string', description: 'overall completeness assessment, Traditional Chinese' },
    gaps: {
      type: 'array',
      items: {
        type: 'object', additionalProperties: false,
        required: ['chapter', 'issue', 'suggestion'],
        properties: {
          chapter: { type: 'string' },
          issue: { type: 'string' },
          suggestion: { type: 'string' },
        },
      },
    },
  },
}

// ---------------------------------------------------------------------------
// Shared writing rules embedded in every writer prompt
// ---------------------------------------------------------------------------
const WRITE_RULES =
  '寫作規則（務必遵守）：\n' +
  '1. 內文一律使用繁體中文；但「畫面控制項名稱、按鈕標籤、參數識別字、軸名、訊號名」保留原文（英文/識別字）。\n' +
  '2. 只能根據提供的 SPEC 資料撰寫；不得杜撰數值、不存在的按鈕或流程。資料不足處用 `> 【待補：說明】` 標註，並收進 unknowns。\n' +
  '3. 控制項清單用表格：| 控制項 | 類型 | 功能 |。參數清單用表格：| 參數 | 範圍/預設 | 說明 |。\n' +
  '4. 操作步驟用有序清單（1. 2. 3.）。安全/互鎖提示用引言區塊：`> ⚠️ 注意：...`。\n' +
  '5. 章節以單一 H1 開頭（# 第N章 標題），小節用 ##/###。\n' +
  '6. 截圖以 Markdown 圖片插入並附圖說與擷取方式，格式：\n' +
  '   `![圖說](screenshots/檔名.png)`\n' +
  '   `> 圖 N-x 圖說。（擷取方式：如何到達此畫面）`\n' +
  '   即使該截圖檔可能尚未存在也要插入（作為版位）。\n' +
  '7. 專業手冊語氣，精簡、條列、可操作；不要行銷詞藻。\n'

// ---------------------------------------------------------------------------
// PHASE 1 - READ: one reader per unit -> SPEC (barrier: writers need all specs)
// ---------------------------------------------------------------------------
phase('Read')
log('讀取 ' + UNITS.length + ' 個畫面/模組來源 ...')

const specsArr = await parallel(UNITS.map(function (u) {
  return function () {
    const base = u.dir || SRC
    const fileList = u.files.map(function (f) { return base + '/' + f }).join('\n  ')
    const bigNote = (u.big && u.big.length)
      ? ('\n注意：' + u.big.join(', ') + ' 檔案很大，請用 Grep 找出元件/標籤/分頁標題與處理函式，再選擇性 Read 重點段落，不要整檔讀取。')
      : ''
    return agent(
      '你是 HT160S 手冊的來源分析員。請「只讀不改」分析以下 ' + u.kind + ' 的原始碼，產出結構化規格給手冊撰寫者。\n\n' +
      '單元：' + u.title + '（id=' + u.id + '）\n' +
      '檔案：\n  ' + fileList + bigNote + '\n\n' +
      '分析重點：' + u.focus + '\n\n' +
      '重要：\n' +
      '- 原始碼是 Big5 編碼，中文註解/標籤在你眼中可能呈現亂碼；遇到亂碼標籤請填 "[中文標籤]"，不要猜測內容。\n' +
      '- 以英文/識別字記錄元件名稱、按鈕、軸名、訊號名、參數名。\n' +
      '- purpose 與功能說明請用繁體中文。\n' +
      '- 不確定或無法從原始碼判定的，放進 unknowns，不要編造。\n' +
      '- controls 僅用於有 UI 的畫面；純模組可留空，改用 flow 描述動作時序。\n' +
      '回傳符合 schema 的物件。',
      { label: 'read:' + u.id, phase: 'Read', schema: SPEC_SCHEMA }
    )
  }
}))

const specMap = {}
const specList = specsArr.filter(Boolean)
for (var i = 0; i < specList.length; i++) { specMap[specList[i].id] = specList[i] }
log('完成 ' + specList.length + '/' + UNITS.length + ' 份來源規格。開始撰寫章節 ...')

// ---------------------------------------------------------------------------
// PHASE 2 - WRITE: one writer per chapter (parallel; each needs its specs)
// ---------------------------------------------------------------------------
phase('Write')

const chapters = await parallel(CHAPTERS.map(function (c) {
  return function () {
    const specsForChapter = c.units
      .map(function (id) { return specMap[id] })
      .filter(Boolean)
    const shotLines = (c.shots && c.shots.length)
      ? c.shots.map(function (s) { return '  - screenshots/' + s.f + ' — ' + s.cap }).join('\n')
      : '  - （本章不需截圖）'
    return agent(
      '你是 HT160S 操作手冊的章節撰寫者，產出一章的繁體中文 Markdown。\n\n' +
      '章節：第 ' + c.no + ' 章　' + c.title + '\n' +
      '參考手冊對應結構（僅供「編排與風格」參考，內容必須是 HT160S 專屬，不可照抄）：' + c.ref + '\n\n' +
      '本章撰寫指引：' + c.guidance + '\n\n' +
      '本章可用的截圖版位（請依寫作規則第 6 點插入）：\n' + shotLines + '\n\n' +
      '本章可依據的來源規格（JSON，已從原始碼萃取）：\n' +
      '```json\n' + JSON.stringify(specsForChapter, null, 1) + '\n```\n\n' +
      WRITE_RULES + '\n' +
      '請回傳 schema 物件：no="' + c.no + '", file="' + c.file + '", title="' + c.title + '", markdown=完整章節內容, unknowns=本章所有待補項目清單。',
      { label: 'write:' + c.no, phase: 'Write', schema: CHAPTER_SCHEMA }
    )
  }
})).then(function (arr) { return arr.filter(Boolean) })

log('完成 ' + chapters.length + '/' + CHAPTERS.length + ' 章。進行完整性審查 ...')

// ---------------------------------------------------------------------------
// PHASE 3 - CRITIQUE: completeness critic over the chapter outline + unknowns
// ---------------------------------------------------------------------------
phase('Critique')

const outline = chapters.map(function (c) {
  return '第' + c.no + '章 ' + c.title + ' — 待補 ' + (c.unknowns ? c.unknowns.length : 0) + ' 項：' +
    ((c.unknowns && c.unknowns.length) ? c.unknowns.join('；') : '無')
}).join('\n')

const critique = await agent(
  '你是 HT160S 操作手冊的完整性審查員。以下是已產出的章節大綱與各章待補項目。\n\n' +
  outline + '\n\n' +
  '請評估：(1) 章節涵蓋是否完整（對照一台 tray 分類機應有的操作手冊）；(2) 哪些待補項目最關鍵、建議如何補（現場確認 / 截圖 / 查特定原始碼）；(3) 是否有重要主題缺漏（如安全、開機、警報排除、模組時序、SECS/AMR、教導/偏移/速度、I/O）。' +
  '用繁體中文回傳 overall 與 gaps。',
  { phase: 'Critique', schema: CRITIQUE_SCHEMA }
)

return {
  generatedChapters: chapters.length,
  totalChapters: CHAPTERS.length,
  specsRead: specList.length,
  chapters: chapters,
  critique: critique,
}
