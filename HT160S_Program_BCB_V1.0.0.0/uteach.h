//---------------------------------------------------------------------------
#ifndef uteachH
#define uteachH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Grids.hpp>

#include "aled.hpp"
#include "ALed.hpp"

#include "database.h"
//---------------------------------------------------------------------------
#define MAX_TEACH_ITEM 96
//---------------------------------------------------------------------------
typedef struct
{
    int LoadCassetteFirstRingToLoadArmZPosition;
    int LoadCassetteFirstRingToSensorZPosition;
    int LoadArmToLoadCassetteRingYPosition;
    int LoadArmToRead2DBarcodeYPosition;
    int LoadArmToCatchArmYPosition;
    int LoadArmToWaitYPosition;
    int LoadArmToHoleRotateRingYPosition;
    int LoadArmToLoadRailInYPosition;
    int LoadArmToLoadRailOutYPosition;

    int TapeCutterMotorToStartRPosition;
    int TapeInToStartYPosition;
    int TapeInToEndYPosition;
    int TapeInToEndYWaitPosition;

    int TapeShuttleToRingCatchArmXPosition;
    int TapeShuttleToTapeInXPosition;

    int RingCatchArmToLoadArmPickZPosition;
    int RingCatchArmToLoadArmPickDownZPosition;
    int RingCatchArmToTapeShuttlePlaceZPosition;
    int TapeIn_WaitTapeVacuumDistance;

    int EmptyCarFeedTrayYPosition;
    int EmptyCarDischargeTrayYPosition;

    int TrayXArmToEmptyXPosition;
    int TrayXArmToLoaderXPosition;
    int TrayXArmToColorXPosition;       //AI(HT160S-Maintainer) 20260605 : AMR identity-tray pickup at Color station
    int ColorRead2DXPosition;           //AI(HT160S-Maintainer) 20260608 : Color 2D CCD reader X (on stepper, like Loader Top CCD)
    int ColorRead2DYPosition;           //AI(HT160S-Maintainer) 20260622 : Color carriage Y (front) for the 2D read
    int ColorTrayArmPickYPosition;      //AI(HT160S-Maintainer) 20260622 : Color carriage Y (rear) for TrayArm pickup
    int TrayXArmToAuto1XPosition;
    int TrayXArmToAuto2XPosition;
    int TrayXArmToAuto3XPosition;
    int TrayXArmToAuto4XPosition;
    int TrayXArmToAuto5XPosition;
    int TrayXArmToAuto6XPosition;

    int Loader1CarFeedTrayYPosition;
    int Loader1CarDischargeTrayYPosition;
    int Loader1CarFirstCCDYPosition;
    int Loader1CarFirstSortYPosition;
    int Loader2CarFeedTrayYPosition;
    int Loader2CarDischargeTrayYPosition;
    int Loader2CarFirstCCDYPosition;
    int Loader2CarFirstSortYPosition;

    int LoaderCarFirstCCDXPosition;

    int SortArmToLoader1XPosition;
    int SortArmToLoader2XPosition;
    int SortArmToAuto1XPosition;
    int SortArmToAuto2XPosition;
    int SortArmToAuto3XPosition;
    int SortArmToAuto4XPosition;
    int SortArmToAuto5XPosition;
    int SortArmToAuto6XPosition;
    int SortArmToBottomCCDFirstXPosition;

    int Auto1CarFeedTrayYPosition;
    int Auto1CarDischargeTrayYPosition;
    int Auto1CarFirstSortYPosition;
    int Auto2CarFeedTrayYPosition;
    int Auto2CarDischargeTrayYPosition;
    int Auto2CarFirstSortYPosition;
    int Auto3CarFeedTrayYPosition;
    int Auto3CarDischargeTrayYPosition;
    int Auto3CarFirstSortYPosition;
    int Auto4CarFeedTrayYPosition;
    int Auto4CarDischargeTrayYPosition;
    int Auto4CarFirstSortYPosition;
    int Auto5CarFeedTrayYPosition;
    int Auto5CarDischargeTrayYPosition;
    int Auto5CarFirstSortYPosition;
    int Auto6CarFeedTrayYPosition;
    int Auto6CarDischargeTrayYPosition;
    int Auto6CarFirstSortYPosition;

    int SortArmToLoader_1_Z1Position;
    int SortArmToLoader_1_Z2Position;
    int SortArmToLoader_1_Z3Position;
    int SortArmToLoader_1_Z4Position;
    int SortArmToLoader_2_Z1Position;
    int SortArmToLoader_2_Z2Position;
    int SortArmToLoader_2_Z3Position;
    int SortArmToLoader_2_Z4Position;

    int SortArmToAuto_1_Z1Position;
    int SortArmToAuto_1_Z2Position;
    int SortArmToAuto_1_Z3Position;
    int SortArmToAuto_1_Z4Position;
    int SortArmToAuto_2_Z1Position;
    int SortArmToAuto_2_Z2Position;
    int SortArmToAuto_2_Z3Position;
    int SortArmToAuto_2_Z4Position;
    int SortArmToAuto_3_Z1Position;
    int SortArmToAuto_3_Z2Position;
    int SortArmToAuto_3_Z3Position;
    int SortArmToAuto_3_Z4Position;
    int SortArmToAuto_4_Z1Position;
    int SortArmToAuto_4_Z2Position;
    int SortArmToAuto_4_Z3Position;
    int SortArmToAuto_4_Z4Position;
    int SortArmToAuto_5_Z1Position;
    int SortArmToAuto_5_Z2Position;
    int SortArmToAuto_5_Z3Position;
    int SortArmToAuto_5_Z4Position;
    int SortArmToAuto_6_Z1Position;
    int SortArmToAuto_6_Z2Position;
    int SortArmToAuto_6_Z3Position;
    int SortArmToAuto_6_Z4Position;

    int PitchArmXMinPositoin;
    int PitchArmXMaxPositoin;
    int BottomCCDYCapturePosition;
}TEACH;
//---------------------------------------------------------------------------
typedef struct
{
    int MotorSelect;
    int *iPara;
    AnsiString GroupName;
    AnsiString Caption;
    TStringGrid *Grid;
    int Row;
}TECH_PARA;
//---------------------------------------------------------------------------
class TfTeach : public TForm
{
__published:
    TPanel *palClient;
    TPanel *palTitle;
    TPanel *palFunction;
    TPanel *palMotorControl;
    TPanel *palMotorName;
    TPageControl *PageTeach;
    TTabSheet *tsEmptyTray;
    TTabSheet *tsLoaderSort;
    TTabSheet *tsAuto;
    TTabSheet *tsSortZ;
    TTabSheet *tsOthers;
    TStringGrid *grdEmptyTray;
    TStringGrid *grdLoaderSort;
    TStringGrid *grdAuto;
    TStringGrid *grdSortZ;
    TStringGrid *grdOthers;
    TLabel *lblActiveMot;
    TLabel *lblSpeed;
    TLabel *lblStep;
    TLabel *lblTarget;
    TLabel *lblNowPos;
    TLabel *lblEncoder;
    TLabel *lblMotorList;
    TLabel *lblMessage;
    TLabel *lblStatus0;
    TLabel *lblStatus1;
    TLabel *lblStatus2;
    TLabel *lblStatus3;
    TLabel *lblStatus4;
    TLabel *lblStatus5;
    TLabel *lblStatus6;
    TLabel *lblStatus7;
    TLabel *lblStatus8;
    TLabel *lblStatus9;
    TLabel *lblStatus10;
    TALed *ledStatus0;
    TALed *ledStatus1;
    TALed *ledStatus2;
    TALed *ledStatus3;
    TALed *ledStatus4;
    TALed *ledStatus5;
    TALed *ledStatus6;
    TALed *ledStatus7;
    TALed *ledStatus8;
    TALed *ledStatus9;
    TALed *ledStatus10;
    TEdit *edSpeed;
    TEdit *edStep;
    TEdit *edTarget;
    TEdit *edNowPos;
    TEdit *edEncoder;
    TScrollBar *scbTeachSpeed;
    TListBox *lstMotors;
    TButton *btnSetTeach;
    TButton *btnGoTeach;
    TButton *btnSave;
    TButton *btnReload;
    TButton *btnIOForm;
    TButton *btnMotorSet;
    TButton *btnJogP;
    TButton *btnJogN;
    TButton *btnStepP;
    TButton *btnStepN;
    TButton *btnMove;
    TButton *btnHome;
    TButton *btnStop;
    TButton *btnRefresh;
    TTimer *tmrUpdate;
    TTabSheet *tsAdvanced;
    TPageControl *pgcAdvanced;
    TTabSheet *tsSortArm;
    TGroupBox *gbSortArmPickPlace;
    TLabel *lbSuckUse;
    TLabel *lbToArea;
    TLabel *lbSaCol;
    TLabel *lbSaRow;
    TLabel *lblSaStatus;
    TComboBox *cbSuckUse;
    TComboBox *cbToArea;
    TEdit *edSaCol;
    TEdit *edSaRow;
    TCheckBox *chkSaZDown;
    TButton *btnSaGo;
    TTabSheet *tsChannel;
    TGroupBox *gbCarGoUpGoDonw;
    TLabel *lbCarArea;
    TLabel *lbCarLoopTimes;
    TLabel *lblCarStatus;
    TComboBox *cbCarArea;
    TCheckBox *chkCarLoop;
    TEdit *edLoopTimes;
    TButton *btnCarGo;
    TGroupBox *gbAutoGoUp;
    TLabel *lbAutoArea;
    TLabel *lblAutoStatus;
    TComboBox *cbAutoArea;
    TButton *btnAutoGoUp;

    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall tmrUpdateTimer(TObject *Sender);
    void __fastcall grdTeachSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);
    void __fastcall grdTeachDblClick(TObject *Sender);
    void __fastcall lstMotorsClick(TObject *Sender);
    void __fastcall scbTeachSpeedScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
    void __fastcall edSpeedChange(TObject *Sender);
    void __fastcall btnSetTeachClick(TObject *Sender);
    void __fastcall btnGoTeachClick(TObject *Sender);
    void __fastcall btnSaveClick(TObject *Sender);
    void __fastcall btnReloadClick(TObject *Sender);
    void __fastcall btnIOFormClick(TObject *Sender);
    void __fastcall btnJogPMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall btnJogNMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall btnJogMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall btnStepPClick(TObject *Sender);
    void __fastcall btnStepNClick(TObject *Sender);
    void __fastcall btnMoveClick(TObject *Sender);
    void __fastcall btnHomeClick(TObject *Sender);
    void __fastcall btnStopClick(TObject *Sender);
    void __fastcall btnRefreshClick(TObject *Sender);
    void __fastcall btnSaGoClick(TObject *Sender);
    void __fastcall btnCarGoClick(TObject *Sender);
    void __fastcall btnAutoGoUpClick(TObject *Sender);
private:
    bool bUIBuilt;
    bool bTeachReady;
    bool bSelectingTeachItem;
    bool bHomeRunning;
    int ActiveMotorIndex;
    int iHomeMotorIndex;
    int SelectedTeachIndex;

    bool bSaTestRunning;
    int iSaTask;
    int iSaSlot;
    int iSaTarget;
    int iSaCol;
    int iSaRow;
    bool bSaZDown;

    bool bCarTestRunning;
    int iCarArea;
    int iCarPhase;
    bool bCarLoop;
    int iCarLoopTarget;
    int iCarLoopDone;
    bool bAutoTestRunning;
    int iAutoIndex;

    TLabel *lblStatus[iMotLedTotalCnt];
    TALed *ledStatus[iMotLedTotalCnt];

    TECH_PARA TechPara[MAX_TEACH_ITEM];
    int TECH_MAX_ITEM;

    void BuildUI();
    void BindDfmComponents();
    void ConfigureTeachGrid(TStringGrid *Grid);
    void ResetTeachGrid(TStringGrid *Grid);
    void AddTeachItem(TStringGrid *Grid, AnsiString GroupName, AnsiString Caption, TTrayMotor *Motor, int *iPara);
    int FindTeachItem(TStringGrid *Grid, int Row);
    void SelectTeachItem(int Index);
    void RefreshTeachGrids();
    void RefreshTeachGrid(TStringGrid *Grid);
    void RefreshTeachRow(int Index);
    void FillMotorList();
    void SetupSpeedControl();
    void SetActiveMotor(int Index);
    TTrayMotor *GetMotor(int Index);
    TTrayMotor *GetActiveMotor();
    int GetEditInt(TEdit *Edit, int DefaultValue);
    AnsiString GetTeachFileName();
    AnsiString GetWorkFileTeachName(AnsiString RootPath);
    AnsiString FindTeachFileName();
    AnsiString GetTeachKey(int Index);
    int ParsePositionText(AnsiString Text);
    AnsiString FormatPositionText(int Value);
    AnsiString GetMotorCaption(int Index);
    AnsiString GetSoftLimitCaption(TTrayMotor *Motor);
    void UpdateMotorMonitor();
    void UpdateStatusLed(int LedIndex, bool Enabled, bool Value);
    void StartJog(bool bPositive);
    void StepMove(bool bPositive);
    void MoveActiveMotorToTarget();
    void MoveSelectedTeach();
    void SetSelectedTeachFromNow();
    void StopActiveMotor();
    void SetMessage(AnsiString Text);
    bool CheckSortArmZHome();
    bool CheckCanTeachMove(TTrayMotor *Motor, bool bRequireHome, bool bUseTarget, int Target, bool bAllowLimitAlarm=false);
    void PopulateAdvancedCombos();
    int ComboIndexToTarget(int Index);
    TTrayMotor *GetSaTargetYMotor(int Target);
    bool CheckSortArmTestReady(int SlotIndex, int Target);
    void RunSortArmTest();
    void StopSortArmTest();
    void SetSaStatus(AnsiString Text);
    void PopulateChannelCombos();
    bool CheckCarTestReady();
    bool CallCarGoUp(int Area, int Flag);
    bool CallCarGoDown(int Area, int Flag);
    void StartCarPhase(int Phase);
    void RunCarTest();
    void StopCarTest();
    void RunAutoTest();
    void SetCarStatus(AnsiString Text);
    void SetAutoStatus(AnsiString Text);
public:
    __fastcall TfTeach(TComponent* Owner);
    void __fastcall InitialTeachParameter();
    void __fastcall OpenWorkFile();
    void __fastcall SaveWorkFile(AnsiString S);
};
//---------------------------------------------------------------------------
extern TEACH Teach;
extern TEACH TeachBase;   //AI 20260623 : Offset base; effective Teach = TeachBase + Offset
extern PACKAGE TfTeach *fTeach;
//---------------------------------------------------------------------------
#endif