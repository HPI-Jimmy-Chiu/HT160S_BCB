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

	// Sort mode (customer special request) : when true, classify ICs By Lot+Bin
	// (dynamic (LotID,Bin)->Auto binding built at run time, see THT160LotBinBinding);
	// when false, Normal mode (static Bin->Auto recipe table, THT160BinAreaMap).
	// Two-way exclusive toggle. Edited on tsMaintHardware; changing it affects the
	// in-memory routing core so the operator is told to restart the software.
	// Stored in General.ini [SortMode] UseLotBinMode.
	bool bUseLotBinSortMode;

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

	// Diagnostic : when true, SortArm pops a modal message before each Auto Z-down
	// comparing actual vs expected motor positions. Per-machine engineering check,
	// default OFF. Toggle via [Diagnostic] ShowSortArmPlaceCheck in General.ini.
	bool bShowSortArmPlaceCheck;

	// Safety : minimum encoder gap (motor counts) the two Loader-Y cars must keep
	// from each other before either car is allowed to move, used only when the
	// opposite car is clamping a tray. Larger = more conservative. Per-machine
	// commissioning value. Set via [Safety] LoaderYSafeDistance in General.ini.
	int iLoaderYSafeDistance;

	// Hardware install : does this machine physically have the LED bin display
	// boards (HT9046 style, COM connected)? Commissioning fact. When false the
	// bin display controller stays idle. Set via [BinDisplay] in General.ini.
	bool bBinDisplayInstalled;
	// Bin display COM port name (e.g. "COM5") and label hold time in seconds.
	AnsiString sBinDispComPort;
	int iBinDispDelaySec;
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

	// Log retention (days). Day/month sub-folders older than this are deleted
	// when a log channel opens / rolls over to a new day. 0 = keep forever.
	// Audit logs (EventLog/WebAPI) are kept longer than the high-volume
	// comm/diagnostic logs (BindisplayLog/PadLog/MotorTaskLog). Stored in
	// General.ini [LogRetention]. See cCsvDailyLog::SetRetentionDays.
	int iLogRetentionEventDays;   // EventLog, WebAPI
	int iLogRetentionCommDays;    // BindisplayLog, PadLog, MotorTaskLog

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
