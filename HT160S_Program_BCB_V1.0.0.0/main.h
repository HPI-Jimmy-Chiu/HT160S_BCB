//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "aled.hpp"
#include "HTray.h"
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include "ALed.hpp"
#include "MyLed.h"
//---------------------------------------------------------------------------
enum
{
    MAIN_FEATURE_STATUS_COUNT = 8
};

//AI(secs-kyec-rcmd4) 20260728 : TfMain::OneCycleCore() result, mirroring eMachineStartResult
//(csystem.h). The operator button turns anything other than ocStarted into a ShowMyMessage;
//the SECS S2F41 "ONE_CYCLE" branch maps it to HCACK : ocStarted->0, ocRejBusy->4, rest->2.
//AI(secs-audit-fix) 20260729 : ocRejStopped split out of ocRejBusy. HT160S's published S2F42
//table (docs/SECS spec 3.4) is 0=ok / 1=invalid / 2=cannot perform / 4=busy, and SEMI E5 makes
//HCACK=4 a POSITIVE ack meaning "will be performed, completion signalled later by an event".
//A machine that is STOPPED is not busy, and with CEID 41 now emitted on cycle finish the
//"later" promise became falsifiable : the host would wait forever for an event that can never
//come. On the KYEC 2026-06-08 traffic this was the DOMINANT case - 8 of 11 ONE_CYCLE commands
//arrived while the equipment reported HALT. ocRejStopped -> HCACK=2 tells the truth.
enum eOneCycleResult
{
    ocStarted,
    ocRejMode,
    ocRejBusy,
    ocRejStopped,
    ocRejNotReady
};

//AI(ht160s-mainui) 20260617 : main status-badge grid limits. The SECS/SAFE/AMR
//  badges are auto-arranged by LayoutFeatureBadges() into a COLS x ROWS grid on
//  pnlFeatureStatus (the home screen is cramped on the machine, so the badges must
//  wrap instead of running off one row). To add more badges later: drop a new
//  pnlFeatureBadgeN panel in main.dfm, bind it in BuildFeatureStatusBadges(), and
//  the layout follows automatically. If the visible badge count exceeds the grid
//  capacity the extras are hidden and a one-time runtime warning fires - bump
//  MAIN_FEATURE_BADGE_COLS / _ROWS (and widen pnlFeatureStatus) to make room.
enum
{
    MAIN_FEATURE_BADGE_COLS     = 3,
    MAIN_FEATURE_BADGE_ROWS     = 2,
    MAIN_FEATURE_BADGE_CAPACITY = MAIN_FEATURE_BADGE_COLS * MAIN_FEATURE_BADGE_ROWS
};

//AI(ht160s-statusbar) 20260624 : symbolic stbMain panel indices (port of HT172
// main.h eMainState). emsVersion(0)=software version; 1=Model 2=HandlerID
// 3=SerialNo are filled by UpdateMachineIdentity(); emsSim(5)=compile-time
// SIMULATE indicator (red, owner-drawn, #ifdef SOFT_SIMULATE only); 7=live clock.
enum eMainState
{
    emsVersion = 0,
    emsModel,
    emsHandlerID,
    emsSerialNo,
    emsSpare4,
    emsSim,
    emsSpare6,
    emsTime
};
//---------------------------------------------------------------------------
enum TMainFeatureStatusIndex
{
    eMainFeatureSECS = 0,
    eMainFeatureSafeDoor,
    eMainFeatureAMR,            //AI(ht160s-agv) 20260615 : AMR/AGV ON-OFF status badge
    eMainFeatureSortMode,       //AI(ht160s-whitelist-override) 20260717 : effective sort-mode badge
    eMainFeatureReserve3,
    eMainFeatureReserve4,
    eMainFeatureReserve5,
    eMainFeatureReserve6
};
//---------------------------------------------------------------------------
// AI : design notes for members/methods declared below. Kept out of the class
// body so the BCB6 form designer never meets a comment inside it (a comment in a
// VCL form class - the __published section above all - makes the designer raise
// "Incorrect method declaration in class TfMain" when you click an event).
//   iLastSecsBadgeState     - last SECS badge state shown (-1=unset); edge-trigger
//                             guard so VCL is touched only on change. (secsgem 20260612)
//   Lot WebAPI pull state (lot-webapi 20260612, Stage 4, machine flow):
//     bLotApiPullActive     - a non-blocking pull is in flight.
//     sLotApiPullLot        - lot being pulled (for logging).
//     bLotApiPullAll        - arms the "pull all lots" sweep: manual Lot Start pulls
//                             EVERY lot in the registry, one at a time (the WebAPI
//                             client is single-request).
//     iLotApiPullCursor     - raw registry slot being pulled NOW (advances only after
//                             a lot succeeds or its retries are used up).
//     iLotApiRetryCount     - failed attempts on the current lot.
//   FeatureBadgeSecsClick() - SECS badge -> open GEM log. (secsgem 20260611)
//   sgLotListDblClick()     - double-click a Lot row to inspect the 2D/Bin data
//                             downloaded for that Lot (operator confirms the work-order
//                             JSON arrived). Wired to sgLotList->OnDblClick in ctor.
//   RefreshLotListFromRegistry() - public so the SECS S2F42 SET_LOT_INFO handler can
//                             reproject LotRegistry onto the on-screen Lot list (single
//                             source of truth). (general 20260610)
//   RequestLotDataFromWebApi()/PollLotDataWebApi() - Stage 4 non-blocking pull: Request
//                             kicks off an async GET (no modal; also used by the SECS
//                             LOTSTART handler on the HSMS thread); Poll runs every
//                             MainProc cycle and loads the response into LotRegistry.
//   StartNextLotApiPull()   - advance the sweep: find the next registry lot from
//                             iLotApiPullCursor and start its pull; ends/clears the
//                             sweep when none remain or bLotApiPullAll is off.
//   StartLotWebApiPullAll() - arm a sweep over the whole registry; shared by manual
//                             LotStart AND the SECS LOTSTART handler so BOTH pull every
//                             lot's 2D/Bin data, not just the first.
//   SaveLastLotList()/LoadLastLotList()/RestoreLastWorkOrder() - restore last used work
//                             order on power-on (ref HT172 FormShow ReadLastDataIni):
//                             persist/restore manual Lot list + active Lot No across
//                             restarts; also auto-load today's JSON lot. (need1 20260608)
//   UpdateSecsFeatureBadge() - sync the SECS feature badge to the live HSMS link state;
//                             driven once/sec by the SECS engine timer (HGem->Timer1 ->
//                             HT160Gem::RefreshSecsBadge); edge-triggered.
//   pgcMonitor subtree (pgcMonitor .. MemoEventLog) - decls recovered 2026-06-18 after an
//                             IDE save dropped them. They are component fields, so they sit
//                             with the other __published fields; ALL fields must precede the
//                             event-handler methods or the form designer rejects event edits.
class TfMain : public TForm
{
__published:	// IDE-managed Components
    TPageControl *pgcMain;
    TTabSheet *tsMain;
    TPanel *pnlMain;
    TPanel *pnlMainMenu;
    TSpeedButton *sbLaguage;
    TSpeedButton *sbProduct;
    TSpeedButton *sbMaintance;
    TSpeedButton *sbOffset;
    TSpeedButton *sbSpeed;
    TSpeedButton *sbTool;
    TSpeedButton *sbMessage;
    TSpeedButton *sbExit;
    TSpeedButton *sbMonitor;
    TPanel *pnlBinCntInfo;
    TPanel *pnlMain1;
    TPanel *pnlMain2;
    TPanel *pnlSetting;
    TPanel *palMainStatus;
    TPanel *pnlFeatureStatus;
    TPanel *pnlFeatureBadge1;
    TLabel *lblFeatureName1;
    TLabel *lblFeatureValue1;
    TPanel *pnlFeatureBadge2;
    TLabel *lblFeatureName2;
    TLabel *lblFeatureValue2;
    TPanel *pnlFeatureBadge3;
    TLabel *lblFeatureName3;
    TLabel *lblFeatureValue3;
    TPanel *pnlFeatureBadge4;
    TLabel *lblFeatureName4;
    TLabel *lblFeatureValue4;
    TPanel *palMainStatus_En;
    TPanel *pnlLight;
    TALed *ledYellow;
    TALed *ledRed;
    TALed *ledGreen;
    TLabel *lblRunMode;
    TSpeedButton *sbRealIcon;
    TSpeedButton *sbStartIcon;
    TLabel *lbStartMode;
    TPanel *pnRealDummy;
    TPanel *pnStartMode;
    TTabSheet *tsMonitorView;
    TPanel *pnlMonitorView;
    TPanel *pnlMonitorMenu;
    TSpeedButton *sbRecord;
    TSpeedButton *sbMotorView;
    TSpeedButton *sbMotionView;
    TSpeedButton *sbOther;
    TSpeedButton *btnMainShow;
    TPanel *pnlControlBtn;
    TSpeedButton *sbHome1;
    TSpeedButton *sbStart1;
    TSpeedButton *sbPause1;
    TSpeedButton *sbOneCycle1;
    TSpeedButton *sbCleanOut1;
    TSpeedButton *sbStoreHangup;
    TLabel *lbl_WorkFile;
    TComboBox *cb_WorkFile;
    TLabel *lblUserSelect;
    TComboBox *cbbUserSelect;
    TPanel *Panel7;
    TLabel *Label1;
    TSpeedButton *btnClearCount;
    TLabel *lblUnloadingCount;
    TLabel *lblloseCnt;
    TSpeedButton *sbPaperSummary;
    TPanel *palloadingCount;
    TPanel *palUnloadingCount;
    TPanel *palloseCnt;
    TPanel *Panel2;
    TPanel *Panel9;
    TGroupBox *grpTrack1;
    TPanel *palAutoInfo01;
    TLabel *lblAuto01Cnt;
    TPanel *palAuto01Bin;
    TPanel *palAuto01ID;
    TPanel *palAuto01Cnt;
    TPanel *palAutoInfo02;
    TLabel *lblAuto02Cnt;
    TPanel *palAuto02Bin;
    TPanel *palAuto02ID;
    TPanel *palAuto02Cnt;
    TPanel *palAutoInfo03;
    TLabel *lblAuto03Cnt;
    TPanel *palAuto03Bin;
    TPanel *palAuto03ID;
    TPanel *palAuto03Cnt;
    TPanel *pal1;
    TStringGrid *UPH_StringGrid;
    TPanel *pnlLogMenu;
    TSpeedButton *spbTrayStatus;
    TSpeedButton *sbTimeData;
    TSpeedButton *apbLogs;
    TSpeedButton *btnTrayMap;
    TPageControl *pgcLog;
    TTabSheet *tsTrayStatus;
    TGroupBox *grpLoaderR;
    TTMyTray *mtLoaderR;
    TPanel *lblLoaderCarID;
    TGroupBox *grpLoaderL;
    TTMyTray *mtLoaderL;
    TPanel *lblLoadCurrID_1;
    TTabSheet *tsLogs;
    TListBox *lstLog;
    TTabSheet *tsTimeData;
    TStringGrid *sgTimeData;
    TPanel *btSaveTimeData;
    TTabSheet *tsMapTray;
    TMemo *Memo1;
    TPageControl *PageControl1;
    TTabSheet *tsSimulation;
    TGroupBox *gbSimuSetting;
    TCheckBox *cbEnableSimulation;
    TButton *btnLoadSimuData;
    TStringGrid *sgSimMaxTray;
    TButton *btnSaveSimMax;
    TTabSheet *tsOtherTool;
    TPageControl *pgcWorkOrder;
    TTabSheet *tsLotInfo;
    TLabel *lblLotNo;
    TGroupBox *grpLotManualEdit;
    TEdit *edLotNo;
    TStringGrid *sgLotList;
    TButton *btnAddLot;
    TButton *btnEditLot;
    TButton *btnRemoveLot;
    TButton *btnLotStart;
    TButton *btnLotEnd;
    TLabel *lblTargetLot2D;
    TLabel *lbl2DCount;
    TStringGrid *sg2DBinEdit;
    TButton *btn2DAddRow;
    TButton *btn2DDelRow;
    TButton *btn2DCommit;
    TButton *btn2DClear;
    TButton *btn2DPaste;
    TButton *btn2DImport;
    TLabel *lblLotListHint;
    TPanel *plLotNumberAuto1;
    TPanel *plLotNumberAuto2;
    TPanel *plLotNumberAuto3;
    TGroupBox *GroupBox1;
    TPanel *palAutoInfo04;
    TLabel *lblAuto04Cnt;
    TPanel *palAuto04Bin;
    TPanel *palAuto04ID;
    TPanel *palAuto04Cnt;
    TPanel *plLotNumberAuto4;
    TPanel *palAutoInfo05;
    TLabel *lblAuto05Cnt;
    TPanel *palAuto05Bin;
    TPanel *palAuto05ID;
    TPanel *palAuto05Cnt;
    TPanel *plLotNumberAuto5;
    TPanel *palAutoInfo06;
    TLabel *lblAuto06Cnt;
    TPanel *palAuto06Bin;
    TPanel *palAuto06ID;
    TPanel *palAuto06Cnt;
    TPanel *plLotNumberAuto6;
    TPanel *Panel1;
    TStringGrid *sgProductInfo;
    TPageControl *pgcMonitor;
    TTabSheet *tsMotionView;
    TPanel *PanelMain6;
    TPanel *pnlLed;
    TALed *ledRed1;
    TALed *ledYellow1;
    TALed *ledGreen1;
    TPanel *pnlPillar;
    TPanel *plLoaderChLeft;
    TPanel *plLoaderRChRight;
    TPanel *plAuto3ChRight;
    TPanel *plAuto3ChLeft;
    TPanel *plAuto3TrayWork;
    TTMyTray *mtAuto3TrayWork;
    TPanel *plLoaderLTrayWork;
    TTMyTray *mtLoaderLTrayWork;
    TPanel *plLoaderRTrayWork;
    TTMyTray *mtLoaderRTrayWork;
    TPanel *plAuto4ChRight;
    TPanel *plAuto4ChLeft;
    TPanel *plAuto4TrayWork;
    TTMyTray *mtAuto4TrayWork;
    TPanel *plAuto5ChRight;
    TPanel *plAuto5ChLeft;
    TPanel *plAuto5TrayWork;
    TTMyTray *mtAuto5TrayWork;
    TPanel *plEmptyChRight;
    TPanel *plEmptyChLeft;
    TPanel *plEmptyTrayWork;
    TTMyTray *mtEmptyTrayWork;
    TPanel *plAuto1ChRight;
    TPanel *plAuto1ChLeft;
    TPanel *plAuto1TrayWork;
    TTMyTray *mtAuto1TrayWork;
    TPanel *plAuto2ChRight;
    TPanel *plAuto2ChLeft;
    TPanel *plAuto2TrayWork;
    TTMyTray *mtAuto2TrayWork;
    TPanel *plCCDMotorLoader;
    TPanel *plEmptyLabel;
    TPanel *plLoaderLLabel;
    TPanel *plAuto1Label;
    TPanel *plAuto2Label;
    TPanel *plAuto3Label;
    TPanel *plAuto4Label;
    TPanel *plAuto5Label;
    TPanel *plAuto6ChLeft;
    TPanel *plAuto6ChRight;
    TPanel *plAuto6Label;
    TPanel *plColorChLeft;
    TPanel *plColorChRight;
    TPanel *plColorLabel;
    TPanel *plAuto6TrayWork;
    TTMyTray *mtAuto6TrayWork;
    TPanel *plColorTrayWork;
    TTMyTray *mtColorTrayWork;
    TPanel *plTrayArmName;
    TPanel *plTrayArm;
    TMyLed *ledTrayArm;
    TPanel *plLoaderRLabel;
    TPanel *palSortArm1;
    TMyLed *ledSortArm1ZA;
    TMyLed *ledSortArm1ZB;
    TMyLed *ledSortArm1ZE;
    TMyLed *ledSortArm1ZF;
    TPanel *plSortArmName;
    TPanel *plCCDMotorColor;
    TCheckBox *chkLoadTray;
    TTabSheet *tsMotorView;
    TStringGrid *sgMotorStatus;
    TTabSheet *TabRecord;
    TPanel *Panel12;
    TPageControl *PageControlTaskView;
    TTabSheet *tsTaskRecord;
    TListBox *lbTaskRecord;
    TTabSheet *TabOther;
    TPanel *Panel13;
    TPageControl *pgcDiagnostic;
    TTabSheet *TabSheet2;
    TStringGrid *StringGridLockInfo;
    TTabSheet *TabSheet13;
    TMemo *Memo2;
    TButton *Button2;
    TButton *Button4;
    TButton *Button1;
    TTabSheet *tsIOStatus;
    TMemo *memoIOStatus;
    TTabSheet *TabSuckStatus;
    TMemo *memoSuckStatus;
    TTabSheet *tsModuleStatus;
    TStringGrid *sgModuleStatus;
    TLabel *lbCarTrayCount_Loader;
    TLabel *lbCarTrayCount_Empty;
    TLabel *lbCarTrayCount_Color;
    TLabel *lbCarTrayCount_Auto1;
    TLabel *lbCarTrayCount_Auto2;
    TLabel *lbCarTrayCount_Auto3;
    TLabel *lbCarTrayCount_Auto4;
    TLabel *lbCarTrayCount_Auto5;
    TLabel *lbCarTrayCount_Auto6;
    TStatusBar *stbMain;
    TTabSheet *ts2DBinManual;
    void __fastcall sbLaguageClick(TObject *Sender);
    void __fastcall sbProductClick(TObject *Sender);
    void __fastcall sbMaintanceClick(TObject *Sender);
    void __fastcall sbOffsetClick(TObject *Sender);
    void __fastcall sbSpeedClick(TObject *Sender);
    void __fastcall sbToolClick(TObject *Sender);
    void __fastcall sbMessageClick(TObject *Sender);
    void __fastcall sbExitClick(TObject *Sender);
    void __fastcall sbMonitorClick(TObject *Sender);
    void __fastcall btnMainShowClick(TObject *Sender);
    void __fastcall sbRecordClick(TObject *Sender);
    void __fastcall sbMotorViewClick(TObject *Sender);
    void __fastcall sbMotionViewClick(TObject *Sender);
    void __fastcall sbOtherClick(TObject *Sender);
    void __fastcall spbTrayStatusClick(TObject *Sender);
    void __fastcall sbTimeDataClick(TObject *Sender);
    void __fastcall apbLogsClick(TObject *Sender);
    void __fastcall btnTrayMapClick(TObject *Sender);
    void __fastcall pnStartModeClick(TObject *Sender);
    void __fastcall pnRealDummyClick(TObject *Sender);
    void __fastcall cb_WorkFileChange(TObject *Sender);
    void __fastcall cb_WorkFileDropDown(TObject *Sender);
    void __fastcall cbbUserSelectChange(TObject *Sender);
    void __fastcall sbHome1Click(TObject *Sender);
    void __fastcall sbOneCycle1Click(TObject *Sender);
    void __fastcall sbCleanOut1Click(TObject *Sender);
    void __fastcall sbStart1Click(TObject *Sender);
    void __fastcall sbPause1Click(TObject *Sender);
    void __fastcall sbStoreHangupClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall cbEnableSimulationClick(TObject *Sender);
    void __fastcall btnLoadSimuDataClick(TObject *Sender);
    void __fastcall btnSaveSimMaxClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall btnLotStartClick(TObject *Sender);
    void __fastcall btnLotEndClick(TObject *Sender);
    void __fastcall btnClearCountClick(TObject *Sender);
    void __fastcall btnAddLotClick(TObject *Sender);
    void __fastcall btnEditLotClick(TObject *Sender);
    void __fastcall btnRemoveLotClick(TObject *Sender);
    void __fastcall sgLotListClick(TObject *Sender);
    void __fastcall btn2DAddRowClick(TObject *Sender);
    void __fastcall btn2DDelRowClick(TObject *Sender);
    void __fastcall btn2DCommitClick(TObject *Sender);
    void __fastcall btn2DClearClick(TObject *Sender);
    void __fastcall btn2DPasteClick(TObject *Sender);
    void __fastcall btn2DImportClick(TObject *Sender);
    void __fastcall pgcWorkOrderChange(TObject *Sender);
    void __fastcall stbMainDrawPanel(TStatusBar *StatusBar, TStatusPanel *Panel, const TRect &Rect);
private:	// User declarations
    TPanel *FeatureStatusPanels[MAIN_FEATURE_STATUS_COUNT];
    TLabel *FeatureStatusNameLabels[MAIN_FEATURE_STATUS_COUNT];
    TLabel *FeatureStatusValueLabels[MAIN_FEATURE_STATUS_COUNT];
    bool bUpdatingMainSelections;
    int  iLastSecsBadgeState;
    int  iLastSortModeBadge;   //AI(ht160s-whitelist-override) 20260717 : edge-trigger guard for the sort-mode badge
    bool bLotApiPullActive;
    AnsiString sLotApiPullLot;
    bool bLotApiPullAll;
    int  iLotApiPullCursor;
    int  iLotApiRetryCount;
    bool bFeatureBadgeOverflowWarned;   //AI(ht160s-mainui) 20260617 : one-shot guard for the badge-grid overflow warning
    void __fastcall BuildFeatureStatusBadges();
    void __fastcall LayoutFeatureBadges();   //AI(ht160s-mainui) 20260617 : arrange visible badges into the COLS x ROWS grid
    void __fastcall FeatureBadgeSecsClick(TObject *Sender);
    void __fastcall UpdateWorkFileComboBox();
    void __fastcall RefreshMainUserSelect();
    void __fastcall InitSimulateScreenBinding();
    void __fastcall SyncMonitorTrayDivision();
    void __fastcall SetupLotListGrid();
    void __fastcall Setup2DBinGrid();
    void __fastcall Reload2DBinGridFromRegistry();
    void __fastcall Refresh2DBinHeader();
    bool __fastcall Is2DEditLocked();
    int  __fastcall GetLotListCount();
    void __fastcall sgLotListDblClick(TObject *Sender);
    void __fastcall ShowLotDetail(AnsiString LotID);
    void __fastcall SaveLastLotList();
    void __fastcall LoadLastLotList();
    void __fastcall RestoreLastWorkOrder();
    void __fastcall AppActivate(TObject *Sender);   //AI(HT160S-Maintainer) 20260622 : Application->OnActivate; re-front modal child after desktop task-switch (z-order hang fix)
public:		// User declarations
    __fastcall TfMain(TComponent* Owner);
    void __fastcall RefreshLotListFromRegistry();
    void __fastcall SaveWorkOrder();
    bool __fastcall LoadWorkOrder();
    bool __fastcall LoadWhiteListFile();   //AI(ht160s-whitelist) 20260715 : WhiteList mode 2D->Bin loader
    void __fastcall ArchiveWorkOrderToLotStory();
    bool __fastcall ArchiveDiscardedWorkOrder(AnsiString Reason);
    void __fastcall RequestLotDataFromWebApi(AnsiString LotID);
    void __fastcall PollLotDataWebApi();
    void __fastcall PollFtpUploadResults();   //AI(ht160s-ftp) 20260721 : drain FTP upload results -> EventLog
    void __fastcall StartNextLotApiPull();
    void __fastcall StartLotWebApiPullAll();
    void __fastcall UpdateSecsFeatureBadge();
    void __fastcall UpdateAmrFeatureBadge();   //AI(ht160s-agv) 20260615 : sync AMR badge to GeneralSetting.bUseAMR
    void __fastcall UpdateSortModeFeatureBadge();   //AI(ht160s-whitelist-override) 20260717 : sync sort-mode badge to effective mode
    void __fastcall SetFeatureStatusBadge(int BadgeIndex, AnsiString ValueText, TColor ValueColor);
    void __fastcall SetSimulateScreenStatus();
    void __fastcall ShowMotorInfo();
    void __fastcall ShowUnloadAutoInfo();   //AI(ht160s-motion-view) 20260618 : fill Unload Auto1~6 Bin/Lot/ID/Cnt panels
    void __fastcall ShowCarTrayCount();      //AI(ht160s-agv) 20260624 : PanelMain6 header per-zone trays-on-AMR-car count
    void __fastcall ShowProductInfo();       //AI(ht160s-uph) 20260707 : live Lot/UPH product-info grid (HT172 sgProductInfo parity)
    void __fastcall ShowTrayUphHistory();    //AI(ht160s-uph) 20260707 : rolling per-tray UPH history + Avg (HT172 UPH_StringGrid parity)
    void __fastcall ClearProductInfoAtLotStart();
    void __fastcall FreezeProductInfoAtLotEnd();
    void __fastcall LotStartCore(AnsiString FirstLot, AnsiString Origin);   //AI(secs-lot-additive) 20260730 : shared modal-free Lot-Start body (btnLotStart + SECS LOTSTART); caller registers the lots
    void __fastcall DoLotEndProcess();   //AI(ht160s-overcount-tripqueue D3) 20260721 : shared Lot-End body (btnLotEnd + CleanOut-finish auto path)
    void __fastcall EmitCleanOutOK();    //AI(ht160s-overcount-tripqueue D3) 20260721 : S6F11 CEID42 CleanOutFinish (self-gates on HSMS SELECTED); called from csystem CleanOut-finish before Lot End
    void __fastcall RefreshEventLogView();
    void __fastcall RefreshModuleStatusGrid();   //AI(ht160s-status) 20260703 : Module Status diagnostic sheet pump (throttled; called from DoSystemMessage)
    bool __fastcall SmokeProbeTopForms(AnsiString &OpenedForms, AnsiString &ErrorText);
    void LoadRunModePicture();
    void LoadStartModePicture();
    void Start();
    void DoStartArm();   //AI(machine-command-layer) 20260625 : arm half of Start; only MachineStart() calls it
    void HomeCore();     //AI(machine-command-layer) 20260625 : shared HOME sequence (Home button + SECS HOME)
    bool CleanOutCore();   //AI(secs-rcmd-9045) 20260729 : shared Clean Out arm (Clean Out button + SECS CLEAN_OUT); false = not in Run_Normal, nothing armed
    bool CheckLotDataReady(AnsiString &Reason);
    void ScanPanelKeys();   //AI(HT160S-Maintainer) 20260617 : physical operator-panel key dispatch (HT172 ScanKey port)
    void __fastcall EmitOneCycleOK();   //AI(secs-kyec-rcmd4) 20260728 : S6F11 CEID41 OneCycleFinish (self-gates on HSMS SELECTED); called from the csystem OneCycle-finish dispatcher
    void __fastcall EmitRunStatusChange();   //AI(secs-ceid-align9045) 20260729 : S6F11 CEID27 RunStatus (machine-state text changed); called from csystem SetMainStatus
    void __fastcall EmitSafeDoorChange();    //AI(secs-ceid-align9045) 20260729 : S6F11 CEID123 SafeDoorOnOff (any safety-door sensor edge); called from csystem ReportSafeDoorChangeToSecs
    void __fastcall EmitEnterIOPage();       //AI(secs-ceid-align9045) 20260729 : S6F11 CEID21 EnterIO; called from maintenance.cpp (that unit has no SECS includes)
    void __fastcall EmitMessageBoxClosed();  //AI(secs-ceid-align9045) 20260729 : S6F11 CEID73 MymessboxOK; called from mymessbox.cpp FormClose
    eOneCycleResult OneCycleCore(bool bRequireRunning, AnsiString &Reason);   //AI(secs-kyec-rcmd4) 20260728 : shared One Cycle gate (operator button + SECS ONE_CYCLE); bRequireRunning is the SECS-only stale-arm guard
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
