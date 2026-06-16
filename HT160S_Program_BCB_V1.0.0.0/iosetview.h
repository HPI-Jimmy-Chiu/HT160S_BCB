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
// AI(general) 20260613 : The IO Table editor controls and their event handlers
// were formerly created at runtime in EnsureIOTableEditor(); they now stream from
// iosetview.dfm (inside pn_IODatabase) so they live in __published below (controls
// + handlers must be __published for TReader::MethodAddress to resolve them).
// bIOTableEditorReady (private) guards the one-time grid setup + initial load.
// IMPORTANT: keep the __published section free of comments - the BCB6 form designer
// parses this header on event clicks and raises "Incorrect method declaration in
// class Tfiosetview" if it meets a comment among the members. Put notes out here.
class Tfiosetview : public TForm
{
__published:
    TPanel *pn_IOSetViewMenu;
    TSpeedButton *sbIOExit;
    TPageControl *PageIO;
    TTabSheet *ts_IOLoader;
    TGroupBox *GroupBox1;
    TLabel *Label1;
    TImage *img1;
    TEdit *ed_OutPort_1;
    TCheckBox *cbToolBit0;
    TComboBox *ComboBox1;
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
    TTabSheet *tsMN200;
    TLabel *lblMN200Summary;
    TStringGrid *grdMN200;
    TPanel *pnIOTableEditorToolbar;
    TLabel *lblIOType;
    TLabel *lblIOLane;
    TLabel *lblIOSearch;
    TComboBox *cbbType;
    TComboBox *cbbLane;
    TEdit *edtSearchIO;
    TSpeedButton *btnAddIO;
    TSpeedButton *btnDeleteIO;
    TSpeedButton *btnModify;
    TSpeedButton *sbUpdate;
    TSpeedButton *sbIOEditorRefresh;
    TStringGrid *strngrdIoTable;
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
    void __fastcall btnAddIOClick(TObject *Sender);
    void __fastcall btnDeleteIOClick(TObject *Sender);
    void __fastcall btnModifyClick(TObject *Sender);
    void __fastcall sbUpdateClick(TObject *Sender);
    void __fastcall cbbTypeChange(TObject *Sender);
    void __fastcall edtSearchIOChange(TObject *Sender);
    void __fastcall strngrdIoTableDblClick(TObject *Sender);
    void __fastcall strngrdIoTableSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);
private:
    TStringList *IOTableDeletedTags;
    TStringList *ManualOutputLog;
    bool bIOTableEditorReady;

    int iSelectRow;
    int iSelectCol;

    void RefreshCurrentView();
    void RefreshMN200();
    void RefreshLegacyIOControls();
    void RefreshLegacyIOMaps();
    void SetLegacyComponentHints();
    void SetLegacyComponentHints(TWinControl *PCtrl);
    void EnsureIOTableEditor();
    void SetupIOTableEditorGrid();
    void LoadIoTable(int iType, int iLane, int iIP);
    void SaveIoTableFromGrid();
    void FillIOTableEditorRow(int Row, TIODATA *Data);
    void AppendIOTableDataToCsv(TStringList *Lines, TStringList *Fields, TIODATA *Data);
    void AppendIOTableGridRowToCsv(TStringList *Lines, TStringList *Fields, int Row);
    bool ValidateIOTableGrid(TStringList *Errors);
    bool ValidateIOTableRow(int Row, TStringList *Errors);
    bool BackupIOTableFile(AnsiString *BackupFile);
    void PruneIOTableBackups(AnsiString BackupDir, int MaxKeep);
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
    TPageControl *GetLegacyPageIO();
    void SelectLegacyIOPageByButton(TSpeedButton *Button);
    void UpdateLegacyPageTabsVisible();
    void ClearGridRows(TStringGrid *Grid);
    void SetGridRowCount(TStringGrid *Grid, int RowCount);
    bool ToggleLegacyButtonOutput(TBtnPanel *ButtonPtr);
    void SetRefreshTimerEnabled(bool Enabled);
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
