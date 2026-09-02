//---------------------------------------------------------------------------
#ifndef GeneralSettingH
#define GeneralSettingH
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
// General tier : ship parameters + hardware-install state.
// Storage : system\General.ini (per-machine, NOT copied between machines).
// Edited only through the Maintenance (commissioning) form.
// See skill ht160s-config-tiers.
//---------------------------------------------------------------------------
// AI(ht160s-lotpassfail) 20260709 : sort-mode selector values. Top-level enum (no
// enum class - BCB6). Replaces the old bool bUseLotBinSortMode two-way toggle.
enum THT160SortMode { smNormal=0, smLotBin=1, smLotPassFail=2, smWhiteList=3 };
//---------------------------------------------------------------------------
class THT160GeneralSetting
{
private:
	AnsiString GetGeneralIniFileName();

public:
	__fastcall THT160GeneralSetting();

	// Hardware install : does this machine physically have the Color bin-area
	// hardware? This is a commissioning fact, not a paid feature.
	bool bColorBinAreaInstalled;

	// Hardware install : is this machine equipped with an AMR (autonomous
	// mobile robot) docking interface? Commissioning fact, not a paid feature.
	bool bUseAMR;

	// AI(ht160s-agv) 20260623 : sim-only per-zone max tray tolerance. Index 0=Loader,
	// 1=Empty, 2=Color, 3..8=Auto1..6. In SOFT_SIMULATE the infeed (Loader/Empty/Color)
	// drains from this down to 0 (=SnX_Input(e)nd OFF=shortage), and an Auto fills its
	// Car up to this (=InputFullTray ON=full). Edited via gbSimuSetting on the main form.
	// Stored in General.ini [SimAMR] MaxTray0..8. Default 10.
	int iSimAmrMaxTray[9];

	// AI(ht160s-loader-worktray-count) 20260713 : per-zone AMR magazine header (non-work)
	// tray composition. Index 0=Loader, 1=Empty, 2=Color, 3..8=Auto1..6 (same layout as
	// iSimAmrMaxTray). Cover = top cover/header trays; Identity = 2D identity trays; a
	// negative Identity is the "whole car is identity" sentinel (Color supply). The header
	// count (Cover + max(0,Identity)) is ADDED to the host-declared SECS work-only
	// LoaderTrayCount to get the physical car total, and drives both the FmtCarKinds display
	// split and GetFedTrayKind tagging. Stored in General.ini [AMR] CoverTray0..8 /
	// IdentityTray0..8. Defaults reproduce the previous hardcoded FmtCarKinds args.
	int iAmrCoverTray[9];
	int iAmrIdentityTray[9];

	// Sort mode (customer special request). Exclusive selector, 3 values :
	//   smNormal       : static Bin->Auto recipe table (THT160BinAreaMap).
	//   smLotBin        : dynamic (LotID,Bin)->Auto binding built at run time
	//                     (THT160LotBinBinding), key = the IC's Bin.
	//   smLotPassFail   : dynamic (LotID,PASS/FAIL)->Auto binding, key = PASS(1)/
	//                     FAIL(2) from the customer per-IC DiePass (WebService#11),
	//                     via THT160LotRegistry::GetPassFailClass, frozen at CCD
	//                     scan. At most 2 Autos bound per lot.
	// Edited on tsMaintHardware; changing it affects the in-memory routing core so
	// the operator is told to restart the software. Stored in General.ini
	// [SortMode] Mode (legacy [SortMode] UseLotBinMode read for back-compat when
	// Mode is absent). Use the Is*SortMode() helpers instead of comparing raw ints.
	int iSortMode;
	// AI(ht160s-whitelist-override) 20260717 : WhiteList is a TEMPORARY per-lot OVERLAY, not a
	// base production mode. base iSortMode stays in {smNormal,smLotBin,smLotPassFail} (sticky in
	// General.ini). bWhiteListActive rides the work order : armed at Lot Start by the local
	// chkWhiteListActive panel or the SECS SORTMODE pair, auto-cleared at Lot End. The routing
	// core reads the EFFECTIVE mode below, so while armed it behaves like smWhiteList (static
	// Bin->Auto table like Normal, but the 2D->Bin source is a local WhiteList.json loaded at Lot
	// Start; codes not in the list -> Error) and falls back to the base mode when disarmed.
	//AI(secs-66xxx-retire) 20260804 : iEffectiveSortMode USED TO be exported to the host as SVID
	// 66032; that number went with the whole 66xxx band on the customer's ruling, and the family
	// has no sort-mode SVID to move it to. It is now machine-internal only - the host can still
	// SET the mode with S2F41 LOTSTART SORTMODE but can no longer read it back.
	// NOTE: IT IS NOW A DEAD STORE and is kept only to avoid a pointless blast radius: 66032 was its
	// ONLY reader. RecomputeEffectiveSortMode() still writes it, but every machine path - the
	// Is*SortMode() helpers below, routing, the UI, the state record - calls the FUNCTION
	// GetEffectiveSortMode() instead. Removing the int would mean touching RecomputeEffectiveSortMode
	// and its five callers for no functional gain, so it stays until something else needs the slot.
	bool bWhiteListActive;
	int  iEffectiveSortMode;
	int  GetEffectiveSortMode()       { return bWhiteListActive ? smWhiteList : iSortMode; }
	void RecomputeEffectiveSortMode() { iEffectiveSortMode = GetEffectiveSortMode(); }
	void SetWhiteListActive(bool b)   { bWhiteListActive=b; RecomputeEffectiveSortMode(); }
	void SaveWhiteListOverlay();      // AI(ht160s-whitelist-override) 20260717 : persist overlay (own ini, work-order lifecycle)
	void LoadWhiteListOverlay();
	bool IsNormalSortMode()      { return GetEffectiveSortMode()==smNormal; }
	bool IsLotBinSortMode()      { return GetEffectiveSortMode()==smLotBin; }
	bool IsLotPassFailSortMode() { return GetEffectiveSortMode()==smLotPassFail; }
	bool IsWhiteListSortMode()   { return bWhiteListActive; }
	bool IsDynamicBindingMode()  { return IsLotBinSortMode() || IsLotPassFailSortMode(); }

	// AI(ht160s-predictive-supply) 20260707 : demand-aware TrayArm replenish order.
	// When true, FindTrayRequestAuto first serves an Auto that SortArm is currently
	// holding a fixed-route IC for (mirrors SortArm sucker 1..4 place order), so the
	// Auto SortArm needs next gets its empty tray before the plain lowest-index scan.
	// Only reorders Autos GetTrayRequest already approves; never changes IC routing or
	// the AMR tray kind. Off (default) = legacy lowest-index-first. Stored in General.ini
	// [SortMode] UsePredictiveAutoSupply.
	bool bUsePredictiveAutoSupply;

	// AI(ht160s-amr-divert) 20260719 : AMR recovered-tray direct supply (opt-in).
	// When true (AMR mode only), a Loader-recovered plain Normal tray may be
	// delivered straight to an Auto whose own GetTrayRequest asks for a Normal
	// tray (car already stacked identity+cover), instead of always parking at the
	// Empty rear and re-picking it later. Identity->Color and cover->Empty routes
	// are unchanged, so the AMR stack order is never violated. Off (default) =
	// legacy always-recycle. Stored in General.ini [SortMode] UseAmrRecoveryDivert.
	bool bUseAmrRecoveryDivert;

	// AI(ht160s-whitelist) 20260727 : F1 operator opt-in. When true, a 2D code that READS
	// OK but is not found in any loaded lot is routed SILENTLY to the Error Auto (same body
	// as the WhiteList reject / the WAR0475 K_SKIP path) instead of raising the BLOCKING
	// WAR0475 modal that halts the whole Loader task loop per unmatched IC. Off (default) =
	// legacy operator Retry/Skip/Manual modal. A genuinely misread GOOD IC also lands in
	// Error when on, so keep it operator-armed. Stored in General.ini [SortMode] SkipUnknown2DAlarm.
	bool bSkipUnknown2DAlarm;

	//AI(ht160s-ccd-2dsanitize) 20260807 : when true, every comma in a READER-supplied
	// 2D code (Top CCD socket reply, manual/handheld entry) is replaced with an
	// underscore at the read source, so routing keys, Production_Log, Soter CSV and
	// SECS all carry one consistent comma-free form. Covers half-width ',' plus the
	// full-width comma in Big5 (A1 41) and UTF-8 (EF BC 8C) byte forms. The customer
	// work order (WebAPI JSON) already carries the underscore form by agreement, so
	// the loaded lot table is deliberately NOT rewritten. Idle-only toggle (flipping
	// it mid-lot would mix both forms in one run). Edited on the maintenance
	// hardware CCD tab. Stored in General.ini [SortMode] Ccd2DCommaToUnderscore.
	bool bCcd2DCommaToUnderscore;
	AnsiString SanitizeScanned2D(const AnsiString &sRaw);

	// Per-Auto enable (By Lot+Bin mode only). When bAutoEnabled[i]==false, Auto(i+1)
	// is skipped by THT160LotBinBinding::ResolveAuto so no new (LotID,Bin) pair binds
	// to it; existing bindings still resolve. Index 0..5 = Auto1..Auto6. Default all
	// true. Operator-editable on tsMaintHardware at any time (warns to restart).
	// Stored in General.ini [SortMode] AutoEnabled0..5.
	bool bAutoEnabled[6];

	// Hardware install : per-nozzle (SortArm sucker) enable. When bSuckerEnabled[i]
	// ==false, SortArm slot i is never selected to pick/place (FindPickCells skips it
	// and anchors the first ENABLED sucker to the found cell), so a broken/clogged
	// nozzle can be taken out of service without stopping the machine. Index 0..3 =
	// nozzle 1..4. At least one must stay enabled. Default all true. Operator-editable
	// on tsMaintHardware (page locked while a lot runs). Read live each pick cycle, so
	// no restart needed. Stored in General.ini [HardwareInstall] SuckerEnabled0..3.
	bool bSuckerEnabled[4];

	// Hardware install : Suck2 quad-vacuum mode. The machine variant mounts only the
	// Suck2 nozzle (Suck1/3/4 not installed) and plumbs all 4 vacuum generator circuits
	// (Suck1..4_On/_Off/sensor) to that single nozzle. When true, sucker index 1 becomes
	// the gang master (all 4 valves driven together; pick = all 4 sensors ON, release =
	// all 4 OFF, any mismatch alarms via the original SUCxxxx flow) and the effective
	// pick mask is forced to Nozzle2 only. Latched once at boot in
	// LoadSuckerParameterFromDataBase, so a RESTART is required after changing it.
	// Operator-editable on tsOption (warns to restart). Stored in General.ini
	// [HardwareInstall] Suck2QuadVacuum.
	bool bSuck2QuadVacuum;

	// Diagnostic : when true, SortArm pops a modal message before each Auto Z-down
	// comparing actual vs expected motor positions. Per-machine engineering check,
	// default OFF. Toggle via [Diagnostic] ShowSortArmPlaceCheck in General.ini.
	bool bShowSortArmPlaceCheck;
	// AI(ht160s-maintainer) 20260627 : SortArm tray-datum bias (1/100mm). The taught SortArm
	// base is the calibration datum; the first cell = base + bias + XStart/YStart. These are
	// per-machine commissioning offsets, edited directly in General.ini [SortArm]
	// XDatumBias/YDatumBias (default -1000 = -10mm each). Replaces the old bUseTrayDatumModel
	// gate (removed) and the compile-time SORT_ARM_X/Y_DATUM_BIAS constants. CCD scan no
	// longer applies any datum.
	int iSortArmXDatumBias;
	int iSortArmYDatumBias;

	// AI(ht160s-pick-retry) 20260702 : SortArm pick suck-fail automatic retries before the
	// operator alarm. Each retry lifts Z to safe and re-approaches the SAME tray cell
	// (HT172 DoPickFromLoader hardcodes 3; here tunable). 0 = alarm on the first failure.
	// Stored in General.ini [SortArm] PickRetryCount.
	int iSortArmPickRetryCount;
	int iSortArmZMoveGuardMs;   //AI(bcb6-172align) 20260723 : pick/place SuckZ down-move guard timeout ms; still not done + Home on -> alarm

	// AI(ht160s-autoskip) 20260714 : when true, a SortArm cell that still fails to pick
	// after iSortArmPickRetryCount retries is auto-skipped (written off EMPTY_IC, the arm
	// carries on) instead of raising the operator pick-error alarm. Read live each pick
	// cycle, so no restart needed. Default OFF. Stored in General.ini [SortArm] AutoSkipOnPickFail.
	bool bSortArmAutoSkipOnPickFail;
	// AI(ht160s-prepick) 20260806 : on-site note 7 "need check auto has tray, then sortarm pick
	// up ic". SortArm refuses to Z-down and suck until the Auto that IC is routed to already
	// holds a tray that can receive it, so it can never end up holding an IC with nowhere to put
	// it (SelectPlaceAuto has no timeout, no alarm and no fallback for a fixed-route IC).
	// This is the BOUNDED-WAIT budget in seconds : once the gate has held the pick this long
	// continuously, MES1921 names the Auto being waited for. The clock is re-armed on every
	// machine pause / resume and after each alarm, so a paused machine can never accumulate
	// wait time and cry wolf on resume (the same defect the stuck watchdog had).
	//   >0  = gate on, alarm after this many seconds (default 300 = 5 min, user's ruling)
	//    0  = gate on, silent hold, never alarms
	//   -1  = gate OFF entirely (pre-20260806 behaviour: pick first, then wait to place)
	// Stored in General.ini [SortArm] PrePickAutoWaitSec; edited on the maintenance Option page.
	int iSortArmPrePickAutoWaitSec;

	// Safety : minimum encoder gap (motor counts) the two Loader-Y cars must keep
	// from each other before either car is allowed to move, used only when the
	// opposite car is clamping a tray. Larger = more conservative. Per-machine
	// commissioning value. Set via [Safety] LoaderYSafeDistance in General.ini.
	int iLoaderYSafeDistance;
	// SettleDelay : operator-tunable mechanism settle dwells (ms). Default = old fixed values.
	int iEmptyDestackSettleMs;
	int iColorDestackSettleMs;
	int iLoaderDestackSettleMs;
	int iAutoPushConfirmSettleMs;
	int iAutoDischargePostYSettleMs;
	//AI(auto-per-station) 20260802 : how many Auto stations may have a feed/discharge ladder
	//in flight at the same time. The six output stations are mechanically independent (six
	//separate MAutoY axes, no shared rail, no ownership token - see Mot_Table M06-M11), but
	//historically ONE linear ladder served them one at a time, so Auto2 waited on Auto1.
	//  0 = LEGACY. The original single linear ladder, bit for bit. This is the rollback:
	//      one ini value and a restart, no firmware change.
	//  1..6 = per-station ladders, at most this many running concurrently.
	//      6 = fully independent, which is the stated requirement.
	//Escalate 0 -> 1 -> 2 -> 6 on the machine: the only thing software cannot predict is
	//whether plant air can drive several Lean+Push+FrontRise strokes at once.
	int iAutoConcurrency;
	//AI(secs-skipiccount) 20260802 : ask the operator how many ICs they physically removed
	//when they clear an alarm with SKIP, and publish it as SVID 37010 + CEID 78 for the
	//host's inventory reconciliation. HT9045 does this unconditionally (note.cpp:2178,
	//"Many ICs Taken Out From The Tray :"), but on HT-160S it puts an EXTRA DIALOG on a
	//path the operator hits every day, so it is opt-in.
	//  false (default) = no prompt, no CEID 78. Behaviour bit-identical to today.
	//  true            = prompt on SKIP, then report the number.
	bool bAskSkipICCount;
	//AI(secs-e30-gate) 20260803 : GEM control-state commissioning settings, [SECS] section.
	//  iInitialControlState = the GEM control state the machine boots into, in the GEM-standard
	//  domain (1 = Off-Line / 4 = On-Line Local / 5 = On-Line Remote). Default 5. That domain is
	//  HT160Gem::iControlState's, which SVID 66002 used to publish verbatim; 66002 was retired on
	//  20260804 and the host now reads the same state on SVID 4 in 9045's 1/2/3 domain.
	//  MUST NOT default to Off-Line: E30 lets the host go on-line only from HOST OFF-LINE, and on
	//  2026-07-31 the host's S1F17 was the only thing that ever brought this machine on-line, so a
	//  machine that boots EQUIPMENT OFF-LINE with no operator at the panel is a production stop.
	//  This is a COMMISSIONING key, not an operator toggle - the operator uses the SECS tab.
	int iInitialControlState;
	//  bAcceptHostOnlineRequest = may the host's S1F17 / RCMD ONLINE_* take the tool on-line at
	//  all. HT9045's GemCheckBoxAcceptHostOnlineRequest ported ([GEM] AcceptHostOnlineRequest
	//  there, default true). false => ONLACK=1 / HCACK=2 refusal. Default true.
	bool bAcceptHostOnlineRequest;
	//AI(secs-s2f37-silenceall) 20260808 : field escape hatch for a host that silences the tool and
	//  never un-silences it. [SECS] IgnoreHostSilenceAll, DEFAULT FALSE = strict E5: an S2F37 that
	//  leaves zero CEIDs enabled really does stop every S6F11, which is what the host asked for.
	//  TRUE = the tool refuses to be left completely mute: the per-CEID flags the host wrote are
	//  still applied, but host report management is released, so the enable gate falls back to
	//  "send everything" until the host enables something specific. Deliberately keyed on the
	//  RESULT (zero enabled) rather than the packet shape, and deliberately NOT a blanket
	//  "ignore S2F37" - a host that disables named CEIDs is always obeyed, and a later targeted
	//  S2F37 CEED=1 re-arms the gate with exactly the ids it names, so this cannot over-report.
	//  Why it exists: on 2026-08-07 the KYEC host opened all 8 sessions with S2F37 CEED=FALSE +
	//  empty list and never sent CEED=TRUE, so 567 S6F11 were dropped and no event data reached
	//  the host all shift. Leave it FALSE unless a host is known to skip its own enable step.
	bool bSecsIgnoreHostSilenceAll;
	int iHomeReacquireOffsetCnt;
	int iStuckSnapshotSec;   //AI(ht160s-obsv-p1) 20260720 : auto State Record when a module Task sits unchanged this many seconds while running (0=off)
	int iRise1SettleWaitSec;       //AI(ht160s-anti-ghost-d) 20260720 : Loader case-10 rise1-not-retracted wait before the named MES0925 Note (s)
	int iHomeDrainTimeoutSec;      //AI(ht160s-home-resume-drain) 20260711 : HOME cylinder-drain stage timeout (s); on expiry the round falls back to park/removal   //AI(ht160s-home-resume-w3c) 20260711 : HOME re-acquire approach offset (1/100mm; front stopper rises this far clear of the parked tray edge). Sign/direction verified on-machine per carriage.
	int iAutoFrontRiseDwellMs;
	int iAutoCleanOutRiseDwellMs;
	int iTrayArmClampSettleMs;
	int iEmptyFeedClampSettleMs;
	int iColorFeedClampSettleMs;

	// Safety (short-term mechanical interlock) : the Empty and Loader front
	// separate-tray cylinders (C_Empty_FrontSeparateTray_1 /
	// C_Loader_FrontSeparateTray_1) physically clash if both extend at the same
	// time. When true (default), each side waits to extend its separate-tray
	// cylinder while the other side's output is commanded out, serializing the two.
	// Set [Safety] FrontSeparateInterlock=0 in General.ini once the mechanism is
	// reworked so both can run concurrently again.
	bool bFrontSeparateInterlock;

	// Hardware install : does this machine physically have the LED bin display
	// boards (HT9046 style, COM connected)? Commissioning fact. When false the
	// bin display controller stays idle. Set via [BinDisplay] in General.ini.
	bool bBinDisplayInstalled;
	// Bin display COM port name (e.g. "COM5") and label hold time in seconds.
	AnsiString sBinDispComPort;
	int iBinDispDelaySec;
	int iAmrFeedWaitSec;   //AI(ht160s-agv) 20260626 : seconds to wait for AMR magazine refill before Loader-empty alarm (HT9046 600s)
	int iAmrHandshakeWaitSec;   //AI(ht160s-agv) coordinator PREP/READY watchdog aging limit (seconds) before releasing the AMR lock
	int iAgvTimeoutSec;   //AI(amr-unmanned W1) 20260721 : unified AGV handshake timeout (seconds) -> WAR0962; serves Normal full-collect wait, CleanOut collect wait, Empty/Color supply wait (was iCleanOutAmrWaitSec, never consumed)
	//AI(amr-loader-nocall) 20260902 : does the LOADER (P1) proactively CALL the AMR when its
	// source runs dry? Owner ruling 20260902 : DEFAULT FALSE. A dry Loader source is this
	// machine's end-of-lot signal, not a refill request - the aLoader source-dry path enters
	// Clean Out on its own once iAmrFeedWaitSec elapses. With this false the coordinator emits
	// no CEID 272 for P1 and takes no handshake state; a HOST-INITIATED START_AGV("Loader")
	// still works (BeginPrep does not require CALLED), so refill scheduling moves to the EAP.
	// Set [AGV] LoaderCallsAmr=1 to restore the pre-20260902 behaviour without a rebuild.
	// Empty (P2) / Color (P3) are NOT affected - they keep calling.
	bool bLoaderCallsAmr;
	// Machine identity (status-bar panels 1-3). Persisted in General.ini
	// [MachineIdentity]. sMachineModel defaults "HT160S". These are the HT160
	// source of truth; UpdateMachineIdentity() copies them into the cmydef
	// as* globals for HT172 API parity.
	AnsiString sMachineModel;
	AnsiString sHandlerID;
	AnsiString sSerialNo;
	//AI(secs-operatorid) 20260803 : SVID/ECID 1007 Operator ID, slot 2 of the KYEC
	// host's RPTID 502. CANONICAL store - the main-screen edtSysOperatorID edit and
	// the host S2F15 write both land here, and SVID 1007 is bound DIRECTLY to this
	// member (no RefreshSVData snapshot), exactly like sHandlerID / SVID 1002.
	// HT9045 keeps the same value in a TEdit on fLotInfo (uHGemHT9045_EC.cpp:59);
	// HT160S cannot bind a VCL control because its THGem dereferences raw typed
	// pointers only, so the AnsiString is the binding point and the edit mirrors it.
	// Defaults to "Operator" so the host never reads A[0] "" on an uncommissioned set.
	AnsiString sOperatorID;
	// Serial baud rate for the bin display COM line. Old-160 ran the LED board
	// through an external MCU.exe TCP bridge, so the handler kept no baud; the
	// HT9046 hardware standard (matching HT172) is 9600-8-N-1. Settable here so
	// the field is changeable on the maintenance page without a rebuild.
	int iBinDispBaud;
	// Bin display panel protocol: 0 = LED (HT9046, default), 1 = TFT (HT9011
	// graphic panel). Selected/swapped in ConfigureBinDisplay (after Load).
	int iBinDispPanelType;
	// When false (default), routine bin-display TX/Recv frames are NOT written
	// to the BindisplayLog CSV (only open/close + error events are), so the
	// daily comm file does not balloon during production. Set true for full
	// frame-level tracing. Stored in General.ini [BinDisplay] LogVerbose.
	bool bBinDispLogVerbose;
	bool bBinDispUseMyComm;   // false = SPComm TComm (default), true = self-built TMyComm

	// Log retention (days). Day/month sub-folders older than this are deleted
	// when a log channel opens / rolls over to a new day. 0 = keep forever.
	// Audit logs (EventLog/WebAPI) are kept longer than the high-volume
	// comm/diagnostic logs (BindisplayLog/PadLog/MotorTaskLog). Stored in
	// General.ini [LogRetention]. See cCsvDailyLog::SetRetentionDays.
	int iLogRetentionEventDays;   // EventLog, WebAPI
	int iLogRetentionCommDays;    // BindisplayLog, PadLog, MotorTaskLog
	//AI(ht160s-workorder-backup) 20260630 : LotStory Discarded safety backups.
	int iLogRetentionDiscardedDays; // LotStory Discarded (work-order backups)
	//AI(ht160s-uph) 20260706 : per-tray/lot UPH logs under UPHLog (month buckets).
	int iLogRetentionUPHLogDays;    // UPHLog per-lot folders
	int iLogRetentionProdDailyDays; // Production_Log Daily aggregate (per-day prod CSV)
	//AI(staterecord-retention) 20260901 : State Record snapshots under SaveRoot
	// (D:\HT160S_StateRecord by default). This tree had NO retention at all - nothing in
	// the codebase ever deleted a snapshot - while 2026-08-31 on-site produced 8 and 6
	// zips in ONE day on the two machines (HomeResumeDone x5 + StuckWatchdog x3 / x4 + 2
	// manual), and every snapshot re-copies the WHOLE day's SECS + Production + Soter logs,
	// so same-day growth is quadratic in the snapshot count. 0 = keep forever (opt out).
	// Only files matching the machine's own "YYYY-MM-DD HH_MM_SS" stamp are ever removed,
	// so anything an engineer renamed or dropped into that folder by hand is never touched.
	int iLogRetentionStateRecordDays; // State Record snapshot zips + leftover folders

	// AI(ht160s-uph) 20260709 : min-sample warm-up guard for the on-screen + SECS UPH.
	// Early in a lot the elapsed window is tiny, so TotalIC*3600/sec spikes to a huge
	// bogus figure. Until TotalIC reaches this threshold the UPH panel shows "--" and
	// SVID reads 0. 0 = auto (one full tray, from live TrayForm geometry); >0 = fixed
	// IC count. Edited on the maintenance tsFunctionGeneral tab. Stored in General.ini
	// [UPH] MinSampleIC. Does NOT change GetCalculateUPH (lot-end value stays exact).
	int iUphMinSampleIC;

	// Per-unit fixed label text + color, old-160 style. Index order (P0 lock):
	// 0=Empty 1=Loader 2..7=Auto1..6 8=Color. Text is one char: digit/letter/blank.
	// Color is the raw LED code sent to the board (e.g. 1 or 3).
	AnsiString sBinDispText[9];
	int        iBinDispColor[9];

	void SetDefault();
	void Load();
	void Save();
};
//---------------------------------------------------------------------------
extern THT160GeneralSetting GeneralSetting;
//---------------------------------------------------------------------------
#endif
