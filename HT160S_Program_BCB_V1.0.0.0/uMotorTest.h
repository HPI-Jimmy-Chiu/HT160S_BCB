//---------------------------------------------------------------------------
#ifndef uMotorTestH
#define uMotorTestH
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
#include "uQwertyKey.h"
//---------------------------------------------------------------------------
#define MAX_MOTOR_TEST_MOTOR_COUNT 64
//---------------------------------------------------------------------------
class TfMotorTest : public TForm
{
__published:
    TPanel *palMotorControl;
    TPanel *palMotorName;
    TPanel *palMessage;
    TPageControl *PageMotorTest;
    TTabSheet *tsOperate;
    TTabSheet *tsMotorParameter;
    TTabSheet *tsMotorTable;
    TTabSheet *tsInformation;
    TTabSheet *tsDriverRegister;
    TTabSheet *tsServoGuard;
    TPanel *palMotorParameterTools;
    TPanel *palMotorTableTools;
    TPanel *palDriverRegisterTools;
    TPanel *palServoGuardTools;
    TStringGrid *grdOperate;
    TStringGrid *grdMotorParameter;
    TStringGrid *grdMotorTable;
    TStringGrid *grdInformation;
    TStringGrid *grdDriverRegister;
    TStringGrid *grdServoGuard;
    TListBox *lstMotors;
    TTimer *tmrUpdate;

    TLabel *lblTitle;
    TLabel *lblNowPos;
    TLabel *lblEncoder;
    TLabel *lblSpeedPercent;
    TLabel *lblStep;
    TLabel *lblTarget;
    TLabel *lblPos1;
    TLabel *lblPos2;
    TLabel *lblLoopCount;
    TLabel *lblLoopWait;
    TLabel *lblLoopTrip;
    TLabel *lblLoopAverage;
    TLabel *lblLoopTotal;
    TLabel *lblLoopTripValue;
    TLabel *lblLoopAverageValue;
    TLabel *lblLoopTotalValue;
    TLabel *lblMotorList;
    TLabel *lblMotorTableSearch;
    TLabel *lblRegisterOffset;
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

    TEdit *edNowPos;
    TEdit *edEncoder;
    TEdit *edSpeedPercent;
    TEdit *edStep;
    TEdit *edTarget;
    TEdit *edPos1;
    TEdit *edPos2;
    TEdit *edLoopCount;
    TEdit *edMotorTableSearch;
    TEdit *edRegisterOffset;
    TComboBox *cbbLoopWait;
    TScrollBar *scrSpeedPercent;
    TCheckBox *chkMultiLoop;

    TButton *btnJogP;
    TButton *btnJogN;
    TButton *btnStepP;
    TButton *btnStepN;
    TButton *btnMove;
    TButton *btnHome;
    TButton *btnStop;
    TButton *btnRefresh;
    TButton *btnSetPos1;
    TButton *btnSetPos2;
    TButton *btnGoPos1;
    TButton *btnGoPos2;
    TButton *btnGoSoftN;
    TButton *btnGoSoftP;
    TButton *btnLoopStart;
    TButton *btnLoopStop;
    TButton *btnSave;
    TButton *btnReload;
    TButton *btnClose;
    TButton *btnParamSave;
    TButton *btnParamReload;
    TButton *btnParamValidate;
    TButton *btnMotorTableReload;
    TButton *btnMotorTableFind;
    TButton *btnMotorTableLocateActive;
    TButton *btnMotorTableEdit;
    TButton *btnMotorTableSave;
    TButton *btnRegisterRead;
    TButton *btnRegisterReadDefault;
    TButton *btnServoGuardOn;
    TButton *btnServoGuardOff;
    TButton *btnServoApplyOn;
    TButton *btnServoApplyOff;

    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall tmrUpdateTimer(TObject *Sender);
    void __fastcall lstMotorsClick(TObject *Sender);
    void __fastcall grdMotorParameterSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);
    void __fastcall grdMotorParameterDblClick(TObject *Sender);
    void __fastcall grdMotorTableDblClick(TObject *Sender);
    void __fastcall edMotorInputClick(TObject *Sender);
    void __fastcall btnJogPMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall btnJogNMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall btnJogMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall btnStepPClick(TObject *Sender);
    void __fastcall btnStepNClick(TObject *Sender);
    void __fastcall btnMoveClick(TObject *Sender);
    void __fastcall btnHomeClick(TObject *Sender);
    void __fastcall btnStopClick(TObject *Sender);
    void __fastcall btnRefreshClick(TObject *Sender);
    void __fastcall btnSetPos1Click(TObject *Sender);
    void __fastcall btnSetPos2Click(TObject *Sender);
    void __fastcall btnGoPos1Click(TObject *Sender);
    void __fastcall btnGoPos2Click(TObject *Sender);
    void __fastcall btnGoSoftNClick(TObject *Sender);
    void __fastcall btnGoSoftPClick(TObject *Sender);
    void __fastcall btnLoopStartClick(TObject *Sender);
    void __fastcall btnLoopStopClick(TObject *Sender);
    void __fastcall btnSaveClick(TObject *Sender);
    void __fastcall btnReloadClick(TObject *Sender);
    void __fastcall btnCloseClick(TObject *Sender);
    void __fastcall btnParamSaveClick(TObject *Sender);
    void __fastcall btnParamReloadClick(TObject *Sender);
    void __fastcall btnParamValidateClick(TObject *Sender);
    void __fastcall btnMotorTableReloadClick(TObject *Sender);
    void __fastcall btnMotorTableFindClick(TObject *Sender);
    void __fastcall btnMotorTableLocateActiveClick(TObject *Sender);
    void __fastcall btnMotorTableEditClick(TObject *Sender);
    void __fastcall btnMotorTableSaveClick(TObject *Sender);
    void __fastcall btnRegisterReadClick(TObject *Sender);
    void __fastcall btnRegisterReadDefaultClick(TObject *Sender);
    void __fastcall btnServoGuardOnClick(TObject *Sender);
    void __fastcall btnServoGuardOffClick(TObject *Sender);
    void __fastcall btnServoApplyOnClick(TObject *Sender);
    void __fastcall btnServoApplyOffClick(TObject *Sender);
    void __fastcall scrSpeedPercentScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
private:
    bool bUIBuilt;
    bool bHomeRunning;
    bool bLoopRunning;
    bool bLoopTargetIsPos2;
    bool bLoopWaiting;
    bool bLoopMultiMode;
    bool bMotorParameterDirty;
    bool bMotorTableDirty;
    int ActiveMotorIndex;
    int iHomeMotorIndex;
    int iLoopMotorIndex;
    int iLoopTarget;
    int iLoopRemainCount;
    int iLoopFinishedCount;
    DWORD dwLoopWaitUntil;
    DWORD dwLoopStartTick;
    DWORD dwLoopLegStartTick;
    int Pos1[MAX_MOTOR_TEST_MOTOR_COUNT];
    int Pos2[MAX_MOTOR_TEST_MOTOR_COUNT];
    bool bMultiLoopMotor[MAX_MOTOR_TEST_MOTOR_COUNT];
    int MultiLoopTarget[MAX_MOTOR_TEST_MOTOR_COUNT];

    TLabel *lblStatus[iMotLedTotalCnt];
    TALed *ledStatus[iMotLedTotalCnt];

    void BuildUI();
    void BindDfmComponents();
    void BuildPageArea();
    void ArrangeOperatePage();
    void ConfigureOperateGrid();
    void ConfigureMotorParameterGrid();
    void ConfigureMotorTableGrid();
    void ConfigureInformationGrid();
    void ConfigureDriverRegisterGrid();
    void ConfigureServoGuardGrid();
    void FillMotorList();
    void SetActiveMotor(int Index);
    TTrayMotor *GetMotor(int Index);
    TTrayMotor *GetActiveMotor();
    int GetMotorCount();
    int GetEditInt(TEdit *Edit, int DefaultValue);
    bool ShowMotorTestKeyboard(TEdit *Edit, int Function, int DecimalPoint, bool CheckRange, double MinValue, double MaxValue, AnsiString TitleText);
    int ParsePositionText(AnsiString Text);
    AnsiString FormatPositionText(int Value);
    AnsiString BoolText(bool Value);
    AnsiString GetMotorTestIniFileName();
    bool ConfirmDiscardMotorParameterEdit();
    bool ConfirmDiscardMotorTableEdit();
    bool CheckNoUnsavedMotorParameter(AnsiString ActionText);
    bool CheckNoUnsavedMotorTable(AnsiString ActionText);
    bool CheckMotorDataEditIdle();
    bool IsMotorParameterEditableColumn(int ColIndex);
    AnsiString GetMotorParameterCsvName(int ColIndex);
    bool ValidateMotorParameterValue(int ColIndex, AnsiString InputText, AnsiString &DisplayText, AnsiString &CsvText, AnsiString &ErrorText);
    bool ValidateMotorParameterRow(int RowIndex, AnsiString &ErrorText);
    int FindCsvColumn(TStringList *HeaderList, AnsiString ColumnName);
    int FindMotorTableRow(TStringList *LineList, TTrayMotor *Motor, TStringList *HeaderList);
    AnsiString GetMotorTableColumnName(int ColumnIndex);
    bool ValidateMotorTableCsv(AnsiString &SummaryText, AnsiString &ErrorText);
    bool BackupMotorTable(AnsiString &BackupPath, AnsiString &ErrorText);
    void LoadMotorTableGrid();
    bool IsMotorTableEditableColumn(int ColIndex);
    bool ValidateMotorTableEditValue(int ColIndex, AnsiString InputText, AnsiString &DisplayText, AnsiString &ErrorText);
    bool ValidateMotorTableGrid(AnsiString &SummaryText, AnsiString &ErrorText);
    bool BuildMotorTableLinesFromGrid(TStringList *LineList, AnsiString &ErrorText);
    bool BuildMotorTableChangeList(TStringList *ChangeList, AnsiString &PreviewText, int &ChangeCount, AnsiString &ErrorText);
    AnsiString GetMotorTableLogFileName();
    bool AppendMotorTableSaveLog(AnsiString BackupPath, TStringList *ChangeList, AnsiString &ErrorText);
    bool FindMotorTableText(AnsiString Text, bool bStartAfterCurrent);
    void LocateActiveMotorInTable();
    void SelectMotorTableCell(int ColIndex, int RowIndex);
    void EditMotorTableCell();
    void SaveMotorTableGridToFile();
    void EditMotorParameterCell(int RowIndex, int ColIndex);
    void SaveMotorParameterToFile();
    void ReloadActiveMotorParameter();
    void LoadLoopPositions();
    void SaveLoopPositions();
    void RefreshOperateGrid();
    void RefreshMotorParameterGrid();
    void RefreshInformationGrid();
    bool ParseRegisterOffset(AnsiString Text, DWORD &Offset);
    void ReadDriverRegister(bool bDefaultList);
    AnsiString GetServoGuardLogFileName();
    bool AppendServoGuardLog(TStringList *GuardList, AnsiString &ErrorText);
    void SetServoGuardRow(int RowIndex, AnsiString Item, AnsiString Value, AnsiString Note);
    bool ExecuteServoPowerGuard(bool bServoOn, bool bApplyMode);
    void RunServoPowerGuard(bool bServoOn);
    void ApplyServoPower(bool bServoOn);
    void RefreshAllGrids();
    void UpdateMotorMonitor();
    void UpdateStatusLed(int LedIndex, bool Value);
    void UpdateActivePositionEdits();
    void UpdateSpeedScrollFromEdit();
    void ResetLoopStatisticLabels();
    void UpdateLoopStatisticLabels(DWORD TripMS, DWORD AverageMS);
    void SetMessage(AnsiString Text);
    bool CheckSortArmZHome();
    bool CheckCanMotorMove(TTrayMotor *Motor, bool bRequireHome, bool bUseTarget, int Target);
    int ApplySpeedPercent(TTrayMotor *Motor);
    void StartJog(bool bPositive);
    void StepMove(bool bPositive);
    void MoveActiveMotorToTarget();
    void MoveActiveMotorToPosition(int Target);
    void MoveActiveMotorToSoftLimit(bool bPositive);
    void StopActiveMotor();
    void SetPositionFromNow(int PosIndex);
    void StartLoopMove();
    void StartMultiLoopMove();
    void StopLoopMove(bool bStopMotor);
    void RunNextLoopMove();
    void RunNextMultiLoopMove();
    int GetLoopWaitMS();
    void StartLoopWaitOrNext();
    void OnLoopTargetArrived(TTrayMotor *Motor);
    void OnMultiLoopTargetsArrived();
    bool IsLoopTargetArrived(TTrayMotor *Motor);
    bool IsMotorAtTarget(TTrayMotor *Motor, int Target);
    int GetSelectedMultiLoopCount();
    bool CheckAllMultiLoopTargetsArrived();

public:
    __fastcall TfMotorTest(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfMotorTest *fMotorTest;
//---------------------------------------------------------------------------
#endif