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
    MAIN_FEATURE_STATUS_COUNT = 8,
    MAIN_FEATURE_STATUS_COLUMNS = 4
};

enum TMainFeatureStatusIndex
{
    eMainFeatureSECS = 0,
    eMainFeatureSafeDoor,
    eMainFeatureReserve1,
    eMainFeatureReserve2,
    eMainFeatureReserve3,
    eMainFeatureReserve4,
    eMainFeatureReserve5,
    eMainFeatureReserve6
};
//---------------------------------------------------------------------------
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
    TPanel *palMainStatus_En;
    TPanel *pnlLight;
    TALed *ledYellow;
    TALed *ledRed;
    TALed *ledGreen;
    TLabel *lblRunMode;
    TSpeedButton *sbDummyIcon;
    TSpeedButton *sbRealIcon;
    TSpeedButton *sbInitial;
    TSpeedButton *sbContinue;
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
    TSpeedButton *spbStripPos;
    TSpeedButton *sb;
    TPanel *pnlControlBtn;
    TSpeedButton *sbHome1;
    TSpeedButton *sbStart1;
    TSpeedButton *sbPause1;
    TSpeedButton *sbOneCycle1;
    TSpeedButton *sbCleanOut1;
    TSpeedButton *sbStoreHangup;
    TPageControl *pgcMonitor;
    TTabSheet *TabSheet10;
    TPanel *PanelMain6;
    TPanel *pnlLed;
    TALed *ledRed1;
    TALed *ledYellow1;
    TALed *ledGreen1;
    TPanel *pnlPillar;
    TPanel *plLoaderChLeft;
    TPanel *plAuto3ChRight;
    TPanel *plAuto3ChLeft;
    TPanel *plAuto3TrayWork;
    TTMyTray *mtAuto3TrayWork;
    TPanel *plLoaderLTrayWork;
    TTMyTray *mtLoaderLTrayWork;
    TPanel *plLoaderRTrayWork;
    TTMyTray *mtLoaderRTrayWork;
    TPanel *plLoaderRChRight;
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
    TPanel *plSortArmName;
    TPanel *plTrayArmName;
    TPanel *palSortArm1;
    TMyLed *ledSortArm1ZA;
    TMyLed *ledSortArm1ZB;
    TMyLed *ledSortArm1ZE;
    TMyLed *ledSortArm1ZF;
    TPanel *plTrayArm;
    TTabSheet *TabSheet7;
    TStringGrid *sgMotorStatus;
    TTabSheet *TabRecord;
    TPanel *Panel12;
    TPageControl *PageControlTaskView;
    TTabSheet *tsTaskRecord;
    TListBox *lbTaskRecord;
    TTabSheet *TabOther;
    TPanel *Panel13;
    TPageControl *PageControl3;
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
    TTabSheet *tsStripPos;
    TPageControl *PageControl4;
    TTabSheet *TabSheet4;
    TScrollBox *ScrollBox1;
    TTMyTray *TMyTray1;
    TTMyTray *TMyTray5;
    TTabSheet *TabSheet5;
    TScrollBox *ScrollBox2;
    TTMyTray *TMyTray2;
    TTMyTray *TMyTray6;
    TTabSheet *TabSheet6;
    TScrollBox *ScrollBox3;
    TTMyTray *TMyTray3;
    TTMyTray *TMyTray7;
    TTabSheet *TabSheet8;
    TScrollBox *ScrollBox4;
    TTMyTray *TMyTray4;
    TTMyTray *TMyTray8;
    TTabSheet *tsOffset;
    TMemo *MemoOffset;
    TTabSheet *TabSheet3;
    TMemo *MemoDownCCD;
    TMemo *MemoTopCCD;
    TMemo *MemoAudioCCD;
    TTabSheet *TabSheet9;
    TMemo *MemoEventLog;
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
    TLabel *lblLotInfo;
    TPanel *palloadingCount;
    TPanel *palUnloadingCount;
    TPanel *palloseCnt;
    TPanel *Panel2;
    TPanel *Panel9;
    TGroupBox *grpTrack1;
    TLabel *lblAuto06Cnt;
    TPanel *palAutoInfo01;
    TLabel *lblAuto01Cnt;
    TPanel *palAuto01Bin;
    TButton *btnSetFullTray_Auto1;
    TPanel *palAuto01ID;
    TPanel *palAuto01Cnt;
    TPanel *palAuto01LotCnt;
    TPanel *pnlAuto01ICCnt;
    TPanel *pnlAuto01EmptyCnt;
    TPanel *palAuto01In;
    TPanel *palAutoInfo02;
    TLabel *lblAuto02Cnt;
    TPanel *palAuto02Bin;
    TButton *btnSetFullTray_Auto2;
    TPanel *palAuto02ID;
    TPanel *palAuto02Cnt;
    TPanel *palAuto02LotCnt;
    TPanel *pnlAuto02EmptyCnt;
    TPanel *pnlAuto02ICCnt;
    TPanel *palAuto02In;
    TPanel *palAutoInfo03;
    TLabel *lblAuto03Cnt;
    TPanel *palAuto03Bin;
    TButton *btnSetFullTray_Auto3;
    TPanel *palAuto03ID;
    TPanel *palAuto03Cnt;
    TPanel *palAuto03LotCnt;
    TPanel *pnlAuto03EmptyCnt;
    TPanel *pnlAuto03ICCnt;
    TPanel *palAuto03In;
    TPanel *palAutoInfo04;
    TLabel *lblAuto04Cnt;
    TPanel *palAuto04Bin;
    TButton *btnSetFullTray_Auto4;
    TPanel *palAuto04ID;
    TPanel *palAuto04Cnt;
    TPanel *palAuto04LotCnt;
    TPanel *pnlAuto04ICCnt;
    TPanel *pnlAuto04EmptyCnt;
    TPanel *palAuto04In;
    TPanel *palAutoInfo05;
    TLabel *lblAuto05Cnt;
    TPanel *palAuto05Bin;
    TButton *btnSetFullTray_Auto5;
    TPanel *palAuto05ID;
    TPanel *palAuto05Cnt;
    TPanel *palAuto05LotCnt;
    TPanel *pnlAuto05ICCnt;
    TPanel *pnlAuto05EmptyCnt;
    TPanel *palAuto05In;
    TPanel *palAuto06Bin;
    TButton *btnSetFullTray_Auto6;
    TPanel *palAuto06ID;
    TPanel *palAuto06Cnt;
    TPanel *palAuto06LotCnt;
    TPanel *pnlAuto06EmptyCnt;
    TPanel *pnlAuto06ICCnt;
    TPanel *palAuto06In;
    TPanel *pal1;
    TSpeedButton *btnTranspose;
    TStringGrid *UPH_StringGrid;
    TStringGrid *sgProductInfo;
    TPanel *pnlLogMenu;
    TSpeedButton *spbTrayStatus;
    TSpeedButton *sbTimeData;
    TSpeedButton *apbLogs;
    TSpeedButton *btnTrayMap;
    TPageControl *pgcLog;
    TTabSheet *tsTrayStatus;
    TMemo *MemoBinCount;
    TGroupBox *grpLoaderR;
    TTMyTray *mtSortRecv;
    TPanel *lblLoaderCarID;
    TGroupBox *grpLoaderL;
    TLabel *lblLoadCurrBin_1;
    TTMyTray *mtWorkArea;
    TPanel *lblLoadCurrID_1;
    TTabSheet *tsLogs;
    TSpeedButton *sbHome;
    TSpeedButton *sbStart;
    TSpeedButton *sbPause;
    TSpeedButton *sbRetry;
    TSpeedButton *sbSkip;
    TSpeedButton *sbTrayEnd;
    TSpeedButton *sbTrayFeed;
    TSpeedButton *sbOneCycle;
    TSpeedButton *sbCleanOut;
    TSpeedButton *sbReset;
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
    TTabSheet *tsOtherTool;
    TGroupBox *grpNotUse;
    TLabel *lblNotUse;
    TPanel *palNotUseCnt;
    TPanel *palNotUseBin;
    TPanel *palNotUseID;
    TPanel *palNotUse;
    TTMyTray *myNotUse;
    TPanel *palNoUseIn;
    TPanel *palNoUseICCnt;
    TPanel *palNoUseEmptyCnt;
    TPanel *palNotUseLotCnt;
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
    TPanel *plLoaderRLabel;
    TMyLed *ledTrayArm;
    TPageControl *PageControl2;
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
    TPanel *plCCDMotorColor;
    TCheckBox *chkLoadTray;
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
    void __fastcall spbStripPosClick(TObject *Sender);
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
    void __fastcall FormShow(TObject *Sender);
    void __fastcall btnLotStartClick(TObject *Sender);
    void __fastcall btnLotEndClick(TObject *Sender);
    void __fastcall btnAddLotClick(TObject *Sender);
    void __fastcall btnEditLotClick(TObject *Sender);
    void __fastcall btnRemoveLotClick(TObject *Sender);
    void __fastcall sgLotListClick(TObject *Sender);
private:	// User declarations
    TPanel *FeatureStatusPanels[MAIN_FEATURE_STATUS_COUNT];
    TLabel *FeatureStatusNameLabels[MAIN_FEATURE_STATUS_COUNT];
    TLabel *FeatureStatusValueLabels[MAIN_FEATURE_STATUS_COUNT];
    bool bUpdatingMainSelections;
    int  iLastSecsBadgeState;   //AI(ht160s-secsgem) 20260612 : last SECS badge state shown (-1=unset); edge-trigger guard so VCL is touched only on change
    void __fastcall BuildFeatureStatusBadges();
    void __fastcall FeatureBadgeSecsClick(TObject *Sender);   //AI(ht160s-secsgem) 20260611 : SECS badge -> open GEM log
    void __fastcall UpdateWorkFileComboBox();
    void __fastcall RefreshMainUserSelect();
    void __fastcall InitSimulateScreenBinding();
    void __fastcall SyncMonitorTrayDivision();
    void __fastcall SetupLotListGrid();
    int  __fastcall GetLotListCount();
    //AI(HT160S-Maintainer) 20260608 : need1 : restore last used work order on
    //power-on (ref HT172 FormShow ReadLastDataIni). Persist/restore the manual
    //Lot list + active Lot No across restarts; also auto-load today's JSON lot.
    void __fastcall SaveLastLotList();
    void __fastcall LoadLastLotList();
    void __fastcall RestoreLastWorkOrder();
public:		// User declarations
    __fastcall TfMain(TComponent* Owner);
    //AI(general) 20260610 : public so the SECS S2F42 SET_LOT_INFO handler can
    // reproject LotRegistry onto the on-screen Lot list (single source of truth).
    void __fastcall RefreshLotListFromRegistry();
    //AI(ht160s-secsgem) 20260612 : sync the SECS feature badge to the live HSMS
    // link state. Driven once per second by the SECS engine timer (HGem->Timer1
    // -> HT160Gem::RefreshSecsBadge). Edge-triggered: only repaints on change.
    void __fastcall UpdateSecsFeatureBadge();
    void __fastcall SetFeatureStatusBadge(int BadgeIndex, AnsiString ValueText, TColor ValueColor);
    void __fastcall SetSimulateScreenStatus();
    void __fastcall ShowMotorInfo();
    bool __fastcall SmokeProbeTopForms(AnsiString &OpenedForms, AnsiString &ErrorText);
    void LoadRunModePicture();
    void LoadStartModePicture();
    void Start();
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
