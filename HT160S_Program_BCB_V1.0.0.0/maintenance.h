//---------------------------------------------------------------------------
#ifndef maintenanceH
#define maintenanceH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include "aled.hpp"
#include "ALed.hpp"
//---------------------------------------------------------------------------
enum
{
    MAX_MAINTENANCE_MENU_COUNT = 18
};
//---------------------------------------------------------------------------
enum TMaintenanceMenuAction
{
    maShowPage = 0,
    maOpenIOView,
    maOpenTeach,
    maOpenMotorTest,
    maOpenComPort,
    maOpenSecs,
    maCloseForm
};
//---------------------------------------------------------------------------
enum
{
    TOWER_LIGHT_ROW_COUNT = 6,
    TOWER_LIGHT_COLOR_COUNT = 3,
    TOWER_LIGHT_STATE_OFF = 0,
    TOWER_LIGHT_STATE_ON = 1,
    TOWER_LIGHT_STATE_BLINK = 2
};
//---------------------------------------------------------------------------
enum TTowerLightRunState
{
    LED_Running = 0,
    LED_ErrJam = 1,
    LED_Pause = 2,
    LED_Message = 3,
    LED_Heating = 4,
    LED_Homeing = 5
};
//---------------------------------------------------------------------------
// AI(ht160s-maintainer) 20260613 : Bin Display(MCU), Top CCD, Color CCD and Lot
// WebAPI pages were moved from runtime Build*Page() builders into the DFM, so they
// now stream from the __published section below (member names preserved; the
// Load/Save/Refresh code is unchanged).
// IMPORTANT: the BCB6 form designer parses this header when you click a component
// event in the Object Inspector. Its simplified parser raises a whole-class modal
// "Incorrect method declaration in class TfMaintenance" (breaking EVERY event on
// the form) if the __published section either (a) contains any // comment among
// the members, or (b) declares a component FIELD after an event-handler METHOD.
// So keep __published in this exact order: ALL component fields first, THEN all
// "void __fastcall ...Click(...)" handlers, and put explanatory notes out here.
// When adding a new tab's controls, insert its fields into the field block above
// the handlers - do NOT append field+handler pairs per tab.
class TfMaintenance : public TForm
{
__published:
    TPanel *pnlMenu;
    TSpeedButton *spbMaintTowerLight;
    TSpeedButton *spbMaintPassword;
    TSpeedButton *spbMaintAmr;
    TSpeedButton *spbMaintFunctionDef;
    TSpeedButton *spbMaintHardware;
    TSpeedButton *spbMaintExit;
    TSpeedButton *spbMaintTeach;
    TSpeedButton *spbMaintMotor;
    TSpeedButton *spbMaintIO;
    TSpeedButton *spbMaintSECS;
    TSpeedButton *spbMaintCOM;
    TPanel *pnlClient;
    TPanel *pnlTitle;
    TPageControl *pcMaintenance;
    TTabSheet *tsMaintTowerLight;
    TPanel *Panel2;
    TPanel *Panel12;
    TALed *RGB50;
    TALed *RGB40;
    TALed *RGB30;
    TALed *RGB20;
    TALed *RGB10;
    TALed *RGB00;
    TALed *RGB01;
    TALed *RGB11;
    TALed *RGB21;
    TALed *RGB31;
    TALed *RGB41;
    TALed *RGB51;
    TALed *RGB52;
    TALed *RGB42;
    TALed *RGB32;
    TALed *RGB22;
    TALed *RGB12;
    TALed *RGB02;
    TLabel *Label3;
    TLabel *Label4;
    TLabel *Label6;
    TLabel *Label7;
    TLabel *Label43;
    TLabel *Label17;
    TBevel *Bevel1;
    TBevel *Bevel2;
    TBevel *Bevel3;
    TBevel *Bevel4;
    TPanel *Panel13;
    TPanel *Panel14;
    TPanel *Panel15;
    TPanel *Panel16;
    TPanel *Panel17;
    TPanel *Panel18;
    TRadioGroup *RadioGroup2;
    TRadioGroup *RadioGroup3;
    TRadioGroup *RadioGroup4;
    TRadioGroup *RadioGroup5;
    TRadioGroup *RadioGroup6;
    TRadioGroup *RadioGroup7;
    TPanel *Panel19;
    TSpeedButton *sbMusic1;
    TSpeedButton *sbMusic2;
    TSpeedButton *sbMusic3;
    TSpeedButton *sbMusic4;
    TTabSheet *tsMaintPassword;
    TTabSheet *tsMaintAmr;
    TMemo *memAmrStatus;
    TCheckBox *chkAmrTestMode;
    TPanel *pnlAmrTestBanner;
    TPanel *pnlAmrInject;
    TMemo *memAmrTx;
    TTabSheet *tsMaintFunctionDef;
    TTabSheet *tsMaintHardware;
    TPanel *pnlHardwareHeader;
    TPanel *pnlHardwareBody;
    TPanel *pnlFunctionDefHeader;
    TPanel *pnlFunctionDefBody;
    TPageControl *pgcFunctionDef;
    TTabSheet *tsFunctionGeneral;
    TPanel *pnlPredictiveSupplyBox;
    TCheckBox *chkUsePredictiveAutoSupply;
    TLabel *lblPredictiveSupplyHint;
    TPanel *pnlAmrDivertBox;
    TCheckBox *chkUseAmrRecoveryDivert;
    TLabel *lblAmrDivertHint;
    TPanel *pnlSkip2DBox;
    TCheckBox *chkSkipUnknown2DAlarm;
    TLabel *lblSkip2DHint;
    TTabSheet *tsMaintTeach;
    TTabSheet *tsMaintMotor;
    TTabSheet *tsMaintIO;
    TTabSheet *tsMaintSECS;
    TTabSheet *tsMaintCOM;
    TPageControl *pgcMaintHardware;
    TTabSheet *tsLoaderUnloader;
    TPanel *pnlHardwareOptionBox;
    TLabel *lblHardwareColorHint;
    TCheckBox *chkHardwareColorBinArea;
    TPanel *Panel1;
    TSpeedButton *sbUpdateTray;
    TSpeedButton *sbtReloadTray;
    TTabSheet *tsErrorMag;
    TPanel *pnlHardwareErrorBinBox;
    TLabel *lblHardwareErrorTitle;
    TLabel *lblHardwareErrorCode1000;
    TLabel *lblHardwareErrorCode1001;
    TLabel *lblHardwareErrorHint;
    TPanel *Panel3;
    TCheckBox *chkUseAMR;
    TPanel *pnlAutoEnableBox;
    TLabel *lblAutoEnableHint;
    TCheckBox *chkAutoEnable1;
    TCheckBox *chkAutoEnable2;
    TCheckBox *chkAutoEnable3;
    TCheckBox *chkAutoEnable4;
    TCheckBox *chkAutoEnable5;
    TCheckBox *chkAutoEnable6;
    TTabSheet *tsNetwork;
    TPanel *pnlNetworkBody;
    TEdit *edWebapiPath;
    TLabel *Label1;
    TTimer *tmrTowerLightBlink;
    TSpeedButton *spbMaintMCUDisplay;
    TTabSheet *tsMaintMCUDisplay;
    TPanel *pnlMCUSetup;
    TPanel *pnlMCUStatus;
    TPanel *pnlMCUTest;
    TLabel *lblMCUSetupTitle;
    TCheckBox *chkMCUEnabled;
    TLabel *lblMCUIPCap;
    TEdit *edMCUIP;
    TLabel *lblMCUPortCap;
    TComboBox *edMCUPort;
    TLabel *lblMCUReconnectCap;
    TEdit *edMCUReconnect;
    TButton *btnMCUSave;
    TButton *btnMCUReload;
    TButton *btnMCURefresh;
    TLabel *lblMCUStatusEnabled;
    TLabel *lblMCUStatusConnected;
    TLabel *lblMCUStatusQueue;
    TLabel *lblMCUStatusError;
    TLabel *lblMCUTestTitle;
    TLabel *lblMCUAddressCap;
    TEdit *edMCUAddress;
    TLabel *lblMCUTextCap;
    TEdit *edMCUText;
    TLabel *lblMCUColorCap;
    TComboBox *cbbMCUColor;
    TCheckBox *chkMCUCodeSymbol;
    TLabel *lblMCULightValueCap;
    TEdit *edMCULightValue;
    TButton *btnMCUSendDisplay;
    TButton *btnMCUSendCode;
    TButton *btnMCUSendLight;
    TMemo *memMCULog;
    TSpeedButton *spbMaintTopCcd;
    TTabSheet *tsMaintTopCcd;
    TPanel *pnlTopCcdSetup;
    TPanel *pnlTopCcdStatus;
    TPanel *pnlTopCcdTest;
    TLabel *lblTopCcdSetupTitle;
    TLabel *lblTopCcdIPCap;
    TEdit *edTopCcdIP;
    TLabel *lblTopCcdPortCap;
    TEdit *edTopCcdPort;
    TCheckBox *chkTopCcdBottomReserved;
    TCheckBox *chkTopCcdEnable;
    TButton *btnTopCcdSave;
    TButton *btnTopCcdReload;
    TButton *btnTopCcdConnect;
    TButton *btnTopCcdDisconnect;
    TLabel *lblTopCcdStatusConn;
    TLabel *lblTopCcdStatusError;
    TLabel *lblTopCcdTestTitle;
    TButton *btnTopCcdShot;
    TLabel *lblTopCcdResultCap;
    TEdit *edTopCcdResult;
    TMemo *memTopCcdLog;
    TSpeedButton *spbMaintColorCcd;
    TTabSheet *tsMaintColorCcd;
    TPanel *pnlColorCcdSetup;
    TPanel *pnlColorCcdStatus;
    TPanel *pnlColorCcdTest;
    TLabel *lblColorCcdSetupTitle;
    TLabel *lblColorCcdIPCap;
    TEdit *edColorCcdIP;
    TLabel *lblColorCcdPortCap;
    TEdit *edColorCcdPort;
    TCheckBox *chkColorCcdEnable;
    TButton *btnColorCcdSave;
    TButton *btnColorCcdReload;
    TButton *btnColorCcdConnect;
    TButton *btnColorCcdDisconnect;
    TLabel *lblColorCcdStatusConn;
    TLabel *lblColorCcdStatusError;
    TLabel *lblColorCcdTestTitle;
    TButton *btnColorCcdShot;
    TLabel *lblColorCcdResultCap;
    TEdit *edColorCcdResult;
    TMemo *memColorCcdLog;
    TSpeedButton *spbMaintLotApi;
    TTabSheet *tsMaintLotApi;
    TPanel *pnlLotApiSetup;
    TPanel *pnlLotApiStatus;
    TPanel *pnlLotApiTest;
    TLabel *lblLotApiUrl;
    TButton *btnLotApiSave;
    TButton *btnLotApiReload;
    TLabel *lblLotApiSaveHint;
    TCheckBox *chkLotApiUsePull;
    TLabel *lblLotApiStatus;
    TLabel *lblLotApiError;
    TLabel *lblLotApiTestTitle;
    TLabel *lblLotApiTestLotCap;
    TEdit *edLotApiTestLot;
    TButton *btnLotApiFetch;
    TMemo *memLotApiResult;
    TMemo *memLotApiLog;
    TSpeedButton *spbMaintFtp;
    TTabSheet *tsMaintFtp;
    TPanel *pnlFtpSetup;
    TPanel *pnlFtpStatus;
    TPanel *pnlFtpTest;
    TLabel *lblFtpHost;
    TLabel *lblFtpPort;
    TLabel *lblFtpUser;
    TLabel *lblFtpPwd;
    TLabel *lblFtpRemoteDir;
    TLabel *lblFtpSaveHint;
    TEdit *edFtpHost;
    TEdit *edFtpPort;
    TEdit *edFtpUser;
    TEdit *edFtpPwd;
    TEdit *edFtpRemoteDir;
    TCheckBox *chkFtpEnable;
    TCheckBox *chkFtpUploadReport;
    TButton *btnFtpSave;
    TButton *btnFtpReload;
    TLabel *lblFtpState;
    TLabel *lblFtpLastError;
    TLabel *lblFtpTestTitle;
    TButton *btnFtpTestConn;
    TButton *btnFtpTestUpload;
    TMemo *memFtpResult;
    TMemo *memFtpLog;
    TTabSheet *tsSortArm;
    TPanel *pnlSuckerEnableBox;
    TLabel *lblSuckerEnableHint;
    TCheckBox *chkSuckEnable1;
    TCheckBox *chkSuckEnable2;
    TCheckBox *chkSuckEnable3;
    TCheckBox *chkSuckEnable4;
    TPanel *pnlAutoSkipBox;
    TLabel *lblAutoSkipHint;
    TCheckBox *chkSortArmAutoSkip;
    TTabSheet *tsOption;
    TPanel *pnlBinDisplayBox;
    TLabel *lblBinPanelType;
    TComboBox *cbBinPanelType;
    TTabSheet *tsLotInfo;
    TPanel *pnlSortModeBox;
    TLabel *lblLotBinModeHint;
    TRadioGroup *rgSortMode;
    TPanel *pnlWhiteListModeBox;
    TCheckBox *chkWhiteListActive;
    TLabel *lblWhiteListModeHint;
    TCheckBox *cbCommType;
    TPanel *pnlSuck2QuadBox;
    TLabel *lblSuck2QuadHint;
    TCheckBox *chkSuck2QuadVacuum;
    TPanel *plLoaderSafeDistanceSet;
    TLabel *lblLoaderSafeDistance;
    TEdit *edLoaderSafeDistance;
    TLabel *lbmm;
    TLabel *lblLoaderSafeHint;
    TPanel *pnlMachineIdentity;
    TLabel *lblMachineModel;
    TEdit *edMachineModel;
    TLabel *lblHandlerID;
    TEdit *edHandlerID;
    TLabel *lblSerialNo;
    TEdit *edSerialNo;
    TPanel *pnlSettleDelay;
    TLabel *lblSettleDelayTitle;
    TLabel *lblSettle0;
    TEdit *edSettle0;
    TLabel *lblSettle1;
    TEdit *edSettle1;
    TLabel *lblSettle2;
    TEdit *edSettle2;
    TLabel *lblSettle3;
    TEdit *edSettle3;
    TLabel *lblSettle4;
    TEdit *edSettle4;
    TLabel *lblSettle5;
    TEdit *edSettle5;
    TLabel *lblSettle6;
    TEdit *edSettle6;
    TLabel *lblSettle7;
    TEdit *edSettle7;
    TLabel *lblSettle8;
    TEdit *edSettle8;
    TLabel *lblSettle9;
    TEdit *edSettle9;
    TPanel *pnlUphSampleBox;
    TLabel *lblUphMinSample;
    TLabel *lblUphMinSampleHint;
    TEdit *edUphMinSampleIC;
    TLabel *labPwHint;
    TLabel *labPwIdCaption;
    TLabel *labPwPassCaption;
    TLabel *labPwLevelCaption;
    TListBox *lbPwUsers;
    TEdit *edPwId;
    TEdit *edPwPass;
    TComboBox *cbbPwLevel;
    TButton *btnPwAddUpdate;
    TButton *btnPwDelete;
    TButton *btnPwSave;
    TButton *btnPwReload;
    void __fastcall btnMCUSaveClick(TObject *Sender);
    void __fastcall btnMCUReloadClick(TObject *Sender);
    void __fastcall btnMCURefreshClick(TObject *Sender);
    void __fastcall btnMCUSendDisplayClick(TObject *Sender);
    void __fastcall btnMCUSendCodeClick(TObject *Sender);
    void __fastcall btnMCUSendLightClick(TObject *Sender);
    void __fastcall btnTopCcdConnectClick(TObject *Sender);
    void __fastcall btnTopCcdDisconnectClick(TObject *Sender);
    void __fastcall btnTopCcdSaveClick(TObject *Sender);
    void __fastcall btnTopCcdReloadClick(TObject *Sender);
    void __fastcall btnTopCcdShotClick(TObject *Sender);
    void __fastcall btnColorCcdConnectClick(TObject *Sender);
    void __fastcall btnColorCcdDisconnectClick(TObject *Sender);
    void __fastcall btnColorCcdSaveClick(TObject *Sender);
    void __fastcall btnColorCcdReloadClick(TObject *Sender);
    void __fastcall btnColorCcdShotClick(TObject *Sender);
    void __fastcall chkTopCcdEnableClick(TObject *Sender);
    void __fastcall chkColorCcdEnableClick(TObject *Sender);
    void __fastcall btnLotApiSaveClick(TObject *Sender);
    void __fastcall btnLotApiReloadClick(TObject *Sender);
    void __fastcall btnLotApiFetchClick(TObject *Sender);
    void __fastcall btnFtpSaveClick(TObject *Sender);
    void __fastcall btnFtpReloadClick(TObject *Sender);
    void __fastcall btnFtpTestConnClick(TObject *Sender);
    void __fastcall btnFtpTestUploadClick(TObject *Sender);
    void __fastcall spbMaintenanceMenuClick(TObject *Sender);
    void __fastcall RGB00Click(TObject *Sender);
    void __fastcall sbMusic1Click(TObject *Sender);
    void __fastcall tmrTowerLightBlinkTimer(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall chkHardwareColorBinAreaClick(TObject *Sender);
    void __fastcall chkUseAMRClick(TObject *Sender);
    void __fastcall chkAmrTestModeClick(TObject *Sender);
    void __fastcall AmrInjectButtonClick(TObject *Sender);
    void __fastcall btnAgvTimeoutSaveClick(TObject *Sender);
    void __fastcall rgSortModeClick(TObject *Sender);
    void __fastcall chkWhiteListActiveClick(TObject *Sender);
    void __fastcall chkUsePredictiveAutoSupplyClick(TObject *Sender);
    void __fastcall chkUseAmrRecoveryDivertClick(TObject *Sender);
    void __fastcall chkSkipUnknown2DAlarmClick(TObject *Sender);
    void __fastcall chkAutoEnableClick(TObject *Sender);
    void __fastcall chkSuckEnableClick(TObject *Sender);
    void __fastcall chkSuck2QuadVacuumClick(TObject *Sender);
    void __fastcall chkSortArmAutoSkipClick(TObject *Sender);
    void __fastcall edLoaderSafeDistanceClick(TObject *Sender);
    void __fastcall edSettleDelayClick(TObject *Sender);
    void __fastcall edUphMinSampleICClick(TObject *Sender);
    void __fastcall PwListClick(TObject *Sender);
    void __fastcall PwIdClick(TObject *Sender);
    void __fastcall PwPassClick(TObject *Sender);
    void __fastcall PwAddUpdateClick(TObject *Sender);
    void __fastcall PwDeleteClick(TObject *Sender);
    void __fastcall PwSaveClick(TObject *Sender);
    void __fastcall PwReloadClick(TObject *Sender);
private:
    int iMaintenanceMenuCount;
    TSpeedButton *MenuButtons[MAX_MAINTENANCE_MENU_COUNT];
    TTabSheet *MenuPages[MAX_MAINTENANCE_MENU_COUNT];
    TMaintenanceMenuAction MenuActions[MAX_MAINTENANCE_MENU_COUNT];
    bool MenuBottomPins[MAX_MAINTENANCE_MENU_COUNT];
    TSpeedButton *LastClickButton;
    TALed *TowerLightLeds[TOWER_LIGHT_ROW_COUNT][TOWER_LIGHT_COLOR_COUNT];
    bool bTowerLightBlinkPhase;
    bool bLoadingHardwareSettings;   // guard: suppress save-on-click handlers during programmatic LoadHardwareSettings
    TEdit *edAgvTimeoutSec;   //AI(amr-unmanned W5) 20260722 : dynamically-built AGV handshake timeout (s) editor on the AMR page (NULL until BuildAgvTimeoutField)

    void __fastcall RegisterMaintenancePages();
    void __fastcall LayoutMaintenanceButtons();
    void __fastcall SelectMaintenancePage(int PageIndex);
    void __fastcall OpenIOView(TSpeedButton *Button);
    void __fastcall OpenTeach(TSpeedButton *Button);
    void __fastcall OpenMotorTest(TSpeedButton *Button);
    void __fastcall OpenComPort(TSpeedButton *Button);
    void __fastcall OpenSecsGemLog(TSpeedButton *Button);
    void __fastcall InitializeTowerLightPanels();
    void __fastcall LoadMaintenanceSettings();
    void __fastcall SaveMaintenanceSettings();
    void __fastcall LoadHardwareSettings();
    void __fastcall SaveHardwareSettings();
    void __fastcall RefreshHardwareSettingsStatus();
    void __fastcall ApplyHardwareEditLock();
    void __fastcall LoadMCUDisplaySettings();
    void __fastcall SaveMCUDisplaySettings();
    void __fastcall RestartMCUDisplay();
    void __fastcall RefreshMCUDisplayStatus();
    void __fastcall AddMCULog(AnsiString Text);
    void __fastcall ApplyTowerLightConfigToLeds();
    void __fastcall SetTowerLightState(int RowIndex, int ColorIndex, int State);
    void __fastcall RefreshTowerLightPanel(int RowIndex, int ColorIndex);
    void __fastcall RefreshTowerLightPanels();

    void __fastcall LoadTopCcdSettings();
    void __fastcall SaveTopCcdSettings();
    void __fastcall RefreshTopCcdStatus();
    void __fastcall AddTopCcdLog(AnsiString Text);

    void __fastcall LoadColorCcdSettings();
    void __fastcall SaveColorCcdSettings();
    void __fastcall RefreshColorCcdStatus();
    void __fastcall AddColorCcdLog(AnsiString Text);

    void __fastcall LoadLotWebApiSettings();
    void __fastcall SaveLotWebApiSettings();
    void __fastcall RefreshLotWebApiStatus();
    void __fastcall AddLotWebApiLog(AnsiString Text);
    void __fastcall LoadFtpConfigToUi();
    void __fastcall SaveFtpConfigFromUi();
    void __fastcall RefreshFtpStatus();
    void __fastcall AddFtpLog(AnsiString Text);
    void __fastcall RefreshAmrStatus();
    void __fastcall BuildAmrInjectPanel();
    void __fastcall BuildAgvTimeoutField();
    void __fastcall ShowPasswordPage();
    void __fastcall RefreshPasswordGrid();
public:
    __fastcall TfMaintenance(TComponent* Owner);
    void __fastcall OpenWorkFile();
    void __fastcall SaveWorkFile(AnsiString S);
    void __fastcall UpdateRunStateLock();
    void __fastcall SyncSortModeSelectorFromSetting();
};
//---------------------------------------------------------------------------
extern PACKAGE TfMaintenance *fMaintenance;
void LoadTowerLightSettings();
void SaveTowerLightSettings();
void SetTowerLightConfigState(int RowIndex, int ColorIndex, int State);
int GetTowerLightConfigState(int RowIndex, int ColorIndex);
bool GetTowerLightConfigOutput(int RowIndex, int ColorIndex, bool BlinkPhase);
//---------------------------------------------------------------------------
#endif
