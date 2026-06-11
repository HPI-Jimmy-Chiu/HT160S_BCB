//---------------------------------------------------------------------------
#ifndef iosetviewH
#define iosetviewH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Grids.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
#include <Menus.hpp>
#include <Db.hpp>
#include <DBCtrls.hpp>
#include <DBGrids.hpp>
#include <DBTables.hpp>
#include <Graphics.hpp>

#include "database.h"
#include "MyLed.h"
#include "butPa1.h"
#include <DB.hpp>
#include <jpeg.hpp>
#include "ALed.hpp"
//---------------------------------------------------------------------------
class Tfiosetview : public TForm
{
__published:
    TPanel *pn_IOSetViewMenu;
    TSpeedButton *sbIOExit;
    TRadioButton *rbGeneralIO;
    TPageControl *PageIO;
    TTabSheet *ts_IOLoader;
    TGroupBox *GroupBox1;
    TLabel *Label1;
    TImage *img1;
    TEdit *ed_OutPort_1;
    TCheckBox *cbToolBit0;
    TComboBox *ComboBox1;
    TDBNavigator *DBNavigator1;
    TDBGrid *DBGrid1;
    TStringGrid *OutputInformationGrid;
    TMemo *MemoIOMap;
    TTable *ioTable;
    TOpenDialog *OpenDialog1;
    TSaveDialog *SaveDialog1;
    TTimer *Timer1;
    TDataSource *DataSource1;
    TPopupMenu *PopupMenu1;
    TMenuItem *SaveInputMap1;
    TMyLed *MyLed1;
    TBtnPanel *btnpnl1;
    TALed *ALedTool0;
    TMyLed *MyLed36;
    TMyLed *MyLed37;
    TBtnPanel *BtnPanel6;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall BtnPanelClick(TObject *Sender);
    void __fastcall ComboBox1Change(TObject *Sender);
    void __fastcall rbGeneralIOClick(TObject *Sender);
    void __fastcall SaveInputMap1Click(TObject *Sender);
    void __fastcall SaveOutputMap1Click(TObject *Sender);
    void __fastcall sb_IO_CommunicationPadClick(TObject *Sender);
    void __fastcall sbCylinderClick(TObject *Sender);
    void __fastcall sbEnableIOChangClick(TObject *Sender);
    void __fastcall sbInputClick(TObject *Sender);
    void __fastcall sbIOExitClick(TObject *Sender);
    void __fastcall sbIORefreshClick(TObject *Sender);
    void __fastcall sbIORingLoadClick(TObject *Sender);
    void __fastcall sbOutputClick(TObject *Sender);
    void __fastcall sbVacuumClick(TObject *Sender);
    void __fastcall spbTerminalProgramClick(TObject *Sender);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall tmr_IonFanTimer(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:
    // The members below are NOT present in iosetview.dfm; they belong to the
    // code-built (dynamic) IO view UI created in BuildUI()/EnsureIOTableEditor().
    // They MUST stay out of __published, otherwise the BCB6 IDE Form Designer
    // reports "Incorrect method declaration in class Tfiosetview" when it
    // reconciles __published against the DFM (the bcc32 command-line build is
    // unaffected either way). //AI(general) 20260603
    TPanel *palHeader;
    TLabel *lblTitle;
    TLabel *lblSummary;
    TLabel *lblSelected;
    TCheckBox *chkManualOutput;
    TButton *btnRefresh;
    TButton *btnOutputOn;
    TButton *btnOutputOff;
    TButton *btnSuckerDestroy;
    TPageControl *PageControl;
    TTabSheet *tsSensors;
    TTabSheet *tsCylinders;
    TTabSheet *tsSwitches;
    TTabSheet *tsSuckers;
    TTabSheet *tsIOTable;
    TPanel *pnIOTableEditorToolbar;
    TComboBox *cbbType;
    TComboBox *cbbLane;
    TEdit *edtSearchIO;
    TSpeedButton *btnAddIO;
    TSpeedButton *btnDeleteIO;
    TSpeedButton *btnModify;
    TSpeedButton *sbUpdate;
    TStringGrid *strngrdIoTable;
    TStringList *IOTableDeletedTags;
    TStringList *ManualOutputLog;
    void __fastcall btnRefreshClick(TObject *Sender);
    void __fastcall btnOutputOnClick(TObject *Sender);
    void __fastcall btnOutputOffClick(TObject *Sender);
    void __fastcall btnSuckerDestroyClick(TObject *Sender);
    void __fastcall chkManualOutputClick(TObject *Sender);
    void __fastcall GridSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);

    TStringGrid *grdSensors;
    TStringGrid *grdCylinders;
    TStringGrid *grdSwitches;
    TStringGrid *grdSuckers;
    TStringGrid *grdIOTable;

    int SelectedKind;
    int SelectedIndex;
    int SelectedRow;
    int SelectedCol;
    int iSelectRow;
    int iSelectCol;

    void BuildUI();
    void BuildHeader();
    void BuildPages();
    void SetupGrid(TStringGrid *Grid, int ColCount, const char **Headers, const int *Widths);
    TStringGrid *CreateGrid(TWinControl *Parent, int ColCount, const char **Headers, const int *Widths);
    void RefreshAll();
    void RefreshCurrentView();
    void RefreshSummary();
    void RefreshSensors();
    void RefreshCylinders();
    void RefreshSwitches();
    void RefreshSuckers();
    void RefreshIOTable();
    void RefreshLegacyIOControls();
    void RefreshLegacyIOMaps();
    void SetLegacyComponentHints();
    void SetLegacyComponentHints(TWinControl *PCtrl);
    void EnsureIOTableEditor();
    void HideLegacyIOTableEditor();
    void SetupIOTableEditorGrid();
    void LoadIoTable(int iType, int iLane, int iIP);
    void SaveIoTableFromGrid();
    void FillIOTableEditorRow(int Row, TIODATA *Data);
    void AppendIOTableDataToCsv(TStringList *Lines, TStringList *Fields, TIODATA *Data);
    void AppendIOTableGridRowToCsv(TStringList *Lines, TStringList *Fields, int Row);
    bool ValidateIOTableGrid(TStringList *Errors);
    bool ValidateIOTableRow(int Row, TStringList *Errors);
    bool BackupIOTableFile(AnsiString *BackupFile);
    bool IsIOTableGridRowBlank(int Row);
    bool RowMatchesIOTableFilter(TIODATA *Data, int TypeFilter, int LaneFilter, AnsiString SearchText);
    int FindIOTableGridRowByTag(int Tag);
    AnsiString GetIOTableNote(TIODATA *Data);
    AnsiString IOTableCellFromInt(int Value);
    AnsiString IOTableIPToCell(TIODATA *Data);
    AnsiString IOTablePortToCell(TIODATA *Data);
    AnsiString GetIOTableGridCell(int Row, int Col);
    bool IsInputIOType(AnsiString TypeName);
    bool IsOutputIOType(AnsiString TypeName);
    bool IsIOTableKnownType(AnsiString TypeName);
    bool IsIntegerText(AnsiString Value, bool AllowBlank);
    bool IsIPCellText(AnsiString Value, bool AllowBlank);
    bool IsPortCellText(AnsiString Value, bool AllowBlank);
    bool IsValidMapIOData(TIODATA *Data);
    bool IsPadCommunicationData(TIODATA *Data);
    bool IsPadCommunicationInput(AnsiString AliasName);
    bool IsPadCommunicationOutput(AnsiString AliasName);
    bool ResolvePadCommunicationInputState(AnsiString AliasName, bool *State);
    bool ResolvePadCommunicationOutputState(AnsiString AliasName, bool *State);
    TIODATA *FindLegacyIODataByAlias(AnsiString AliasName, bool InputSide);
    AnsiString FormatMapAddress(TIODATA *Data);
    void AppendLegacyIODiagnostics();
    void CollectLegacyComponentDiagnostics(TWinControl *PCtrl, TStringList *Lines, int *UnboundInput, int *UnboundOutput);
    void AppendManualOutputLogToMemo(TMemo *MemoPtr);
    void LogManualOutputAction(AnsiString TargetName, AnsiString ActionName, AnsiString ResultText);
    void SyncPadSwitchStatus(AnsiString SwitchName, bool State);
    void ShowInputInformation();
    void ShowOutputInformation();
    void FillIOMapGrid(TStringGrid *Grid, bool InputSide);
    void SaveIOMapGrid(TStringGrid *Grid, AnsiString FileName);
    void SaveIOMap(bool InputSide);
    bool ResolveLegacyLedState(AnsiString AliasName, bool *State);
    bool ResolveLegacyButtonState(AnsiString AliasName, bool *State);
    void UpdateSelectedInfo();
    void UpdateManualButtons();
    TPageControl *GetLegacyPageIO();
    void SelectLegacyIOPageByButton(TSpeedButton *Button);
    void UpdateLegacyPageTabsVisible();
    void ClearGridRows(TStringGrid *Grid);
    void SetGridRowCount(TStringGrid *Grid, int RowCount);
    int CountIOType(AnsiString TypeName);
    TIODATA *GetIODataByFilteredRow(AnsiString TypeName, int RowIndex);
    AnsiString FormatAddress(int Lane, int IP, int Port, int Bit);
    AnsiString FormatIODataAddress(TIODATA *Data);
    AnsiString FormatIODriver(TIODATA *Data);
    AnsiString FormatIODriver(TMyIo *IOPtr);
    AnsiString FormatSensor(TMySensor *Sensor);
    AnsiString FormatSwitch(TMySwitch *SwitchPtr);
    AnsiString FormatEnable(bool Flag);
    AnsiString FormatEnableInt(int Flag);
    AnsiString FormatOnOff(bool Flag);
    bool IsManualOutputEnabled();
    bool CanManualOutput(AnsiString *Reason);
    bool CanLegacyManualOutput(AnsiString *Reason);
    void ShowManualOutputBlocked(AnsiString Reason);
    bool GetSelectedSwitch(TMySwitch **SwitchPtr);
    bool GetSelectedCylinder(TMyCylinder **CylinderPtr);
    bool GetSelectedSucker(TMySucker **SuckerPtr);
    bool IsLegacyGeneralIOMode();
    bool ToggleLegacyButtonOutput(TBtnPanel *ButtonPtr);
    void SetRefreshTimerEnabled(bool Enabled);
    void __fastcall btnAddIOClick(TObject *Sender);
    void __fastcall btnDeleteIOClick(TObject *Sender);
    void __fastcall btnModifyClick(TObject *Sender);
    void __fastcall sbUpdateClick(TObject *Sender);
    void __fastcall cbbTypeChange(TObject *Sender);
    void __fastcall edtSearchIOChange(TObject *Sender);
    void __fastcall strngrdIoTableDblClick(TObject *Sender);
    void __fastcall strngrdIoTableSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);
public:
    __fastcall Tfiosetview(TComponent* Owner);
    __fastcall ~Tfiosetview();
    bool fShow;
};
//---------------------------------------------------------------------------
extern PACKAGE Tfiosetview *fiosetview;
// iosetview.dfm uses custom components (TMyLed/TBtnPanel/TALed) whose design
// packages are NOT installed in this IDE (HT160S_BCB uses local compatibility
// units, registered only at design time via RegisterComponents). Their classes
// must therefore be registered at runtime before Tfiosetview is streamed, or
// CreateForm throws "Class Txxx not found". Call this once before any
// CreateForm(Tfiosetview). The function is idempotent (guarded by a static).
void RegisterIOViewStreamClasses();
//---------------------------------------------------------------------------
#endif
