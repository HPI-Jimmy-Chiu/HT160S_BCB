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
    MAX_MAINTENANCE_MENU_COUNT = 16
};
//---------------------------------------------------------------------------
enum TMaintenanceMenuAction
{
    maShowPage = 0,
    maOpenIOView,
    maOpenTeach,
    maOpenMotorTest,
    maOpenComPort,
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
class TfMaintenance : public TForm
{
__published:
    TPanel *pnlMenu;
    TSpeedButton *spbMaintTowerLight;
    TSpeedButton *spbMaintPassword;
    TSpeedButton *spbMaintSoftSimu;
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
    TTabSheet *tsMaintSoftSimu;
    TTabSheet *tsMaintFunctionDef;
    TTabSheet *tsMaintHardware;
    TPanel *pnlHardwareHeader;
    TPanel *pnlHardwareBody;
    TTabSheet *tsMaintTeach;
    TTabSheet *tsMaintMotor;
    TTabSheet *tsMaintIO;
    TTabSheet *tsMaintSECS;
    TTabSheet *tsMaintCOM;
    TPageControl *PageControl1;
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
    void __fastcall spbMaintenanceMenuClick(TObject *Sender);
    void __fastcall RGB00Click(TObject *Sender);
    void __fastcall sbMusic1Click(TObject *Sender);
    void __fastcall tmrTowerLightBlinkTimer(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall chkHardwareColorBinAreaClick(TObject *Sender);
private:
    int iMaintenanceMenuCount;
    TSpeedButton *MenuButtons[MAX_MAINTENANCE_MENU_COUNT];
    TTabSheet *MenuPages[MAX_MAINTENANCE_MENU_COUNT];
    TMaintenanceMenuAction MenuActions[MAX_MAINTENANCE_MENU_COUNT];
    bool MenuBottomPins[MAX_MAINTENANCE_MENU_COUNT];
    TSpeedButton *LastClickButton;
    TALed *TowerLightLeds[TOWER_LIGHT_ROW_COUNT][TOWER_LIGHT_COLOR_COUNT];
    TTimer *tmrTowerLightBlink;
    bool bTowerLightBlinkPhase;

    void __fastcall RegisterMaintenancePages();
    void __fastcall LayoutMaintenanceButtons();
    void __fastcall SelectMaintenancePage(int PageIndex);
    void __fastcall OpenIOView(TSpeedButton *Button);
    void __fastcall OpenTeach(TSpeedButton *Button);
    void __fastcall OpenMotorTest(TSpeedButton *Button);
    void __fastcall OpenComPort(TSpeedButton *Button);
    void __fastcall InitializeTowerLightPanels();
    void __fastcall LoadMaintenanceSettings();
    void __fastcall SaveMaintenanceSettings();
    void __fastcall LoadHardwareSettings();
    void __fastcall SaveHardwareSettings();
    void __fastcall RefreshHardwareSettingsStatus();
    void __fastcall ApplyTowerLightConfigToLeds();
    void __fastcall SetTowerLightState(int RowIndex, int ColorIndex, int State);
    void __fastcall RefreshTowerLightPanel(int RowIndex, int ColorIndex);
    void __fastcall RefreshTowerLightPanels();
public:
    __fastcall TfMaintenance(TComponent* Owner);
    void __fastcall OpenWorkFile();
    void __fastcall SaveWorkFile(AnsiString S);
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
