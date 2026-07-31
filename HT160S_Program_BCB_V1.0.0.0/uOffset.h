//---------------------------------------------------------------------------
#ifndef uOffsetH
#define uOffsetH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Grids.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260623 : Offset feature (Route A1, persistent fold).
//Per-product / per-workfile position fine-tune (0.01mm). 56 fields, each mirrors
//a live TEACH base field. UpdateAllParameter() folds Teach = TeachBase + Offset
//(cprod.cpp); stored per workfile in data\<workfile>.ofs as INI. Excluded (no
//live TEACH base): LoaderCarLastCCDX, SortArmToBottomCCD Z1-4.
typedef struct
{
    // Loader1/2 carriage Y (8)
    int Loader1CarFeedTrayYPosition;
    int Loader1CarDischargeTrayYPosition;
    int Loader1CarFirstCCDYPosition;
    int Loader1CarFirstSortYPosition;
    int Loader2CarFeedTrayYPosition;
    int Loader2CarDischargeTrayYPosition;
    int Loader2CarFirstCCDYPosition;
    int Loader2CarFirstSortYPosition;
    // Loader top-CCD X (1)
    int LoaderCarFirstCCDXPosition;
    // SortArm X to Loader/Auto/BottomCCD (9)
    int SortArmToLoader1XPosition;
    int SortArmToLoader2XPosition;
    int SortArmToAuto1XPosition;
    int SortArmToAuto2XPosition;
    int SortArmToAuto3XPosition;
    int SortArmToAuto4XPosition;
    int SortArmToAuto5XPosition;
    int SortArmToAuto6XPosition;
    int SortArmToBottomCCDFirstXPosition;
    // SortArm sucker Z over Loader1/2 (8)
    int SortArmToLoader_1_Z1Position;
    int SortArmToLoader_1_Z2Position;
    int SortArmToLoader_1_Z3Position;
    int SortArmToLoader_1_Z4Position;
    int SortArmToLoader_2_Z1Position;
    int SortArmToLoader_2_Z2Position;
    int SortArmToLoader_2_Z3Position;
    int SortArmToLoader_2_Z4Position;
    // SortArm sucker Z over Auto1..6 (24)
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
    // Auto1..6 first-sort Y (6)
    int Auto1CarFirstSortYPosition;
    int Auto2CarFirstSortYPosition;
    int Auto3CarFirstSortYPosition;
    int Auto4CarFirstSortYPosition;
    int Auto5CarFirstSortYPosition;
    int Auto6CarFirstSortYPosition;
}RUN_OFFSET;
//---------------------------------------------------------------------------
#define OFFSET_MAX_ITEM 60
typedef struct
{
    int *iPara;
    AnsiString GroupName;
    AnsiString Caption;
    int iMax;
    int iMin;
    TStringGrid *Grid;
    int Row;
}OFFSET_PARA;
//---------------------------------------------------------------------------
class TfOffset : public TForm
{
__published:
public:
    __fastcall TfOffset(TComponent* Owner);
    void __fastcall OpenWorkFile();
    void __fastcall SaveWorkFile();
    void __fastcall InitialOffsetParameter();
private:
    TPanel *palTitle;
    TPanel *palExplain;
    TLabel *lblExplain;
    TPanel *palButtons;
    TPageControl *PageOffset;
    TTabSheet *tsLoader;
    TTabSheet *tsSortArm;
    TTabSheet *tsAuto;
    TStringGrid *grdLoader;
    TStringGrid *grdSortArm;
    TStringGrid *grdAuto;
    TButton *btnApply;
    TButton *btnReAlign;
    TButton *btnExit;
    TButton *btnClear;
    TEdit *edScratch;
    TPopupMenu *popLimit;
    TMenuItem *miSetMax;
    TMenuItem *miSetMin;
    TStringGrid *popGrid;
    int popRow;
    OFFSET_PARA OffsetPara[OFFSET_MAX_ITEM];
    int OffsetItemCount;
    bool bUIBuilt;
    int OffsetBaseVal[OFFSET_MAX_ITEM];
    bool bOffsetBaseCaptured;
    void SnapshotOffsetValues();
    void LogOffsetChanges(AnsiString sAction);
    void BuildUI();
    void ConfigureGrid(TStringGrid *Grid);
    void ResetGrid(TStringGrid *Grid);
    void AddOffsetItem(TStringGrid *Grid, AnsiString GroupName, AnsiString Caption, int *iPara, int iMax, int iMin);
    void RefreshGrids();
    void RefreshRow(int Index);
    int FindItem(TStringGrid *Grid, int Row);
    AnsiString GetOffsetFileName();
    AnsiString GetOffsetKey(int Index);
    AnsiString GetLimitFileName();
    void LoadOffsetLimits();
    void SeedLimitFile();
    void SaveOneLimit(int Index);
    AnsiString GetOffsetExplain(AnsiString Caption);
    int ParseOffsetText(AnsiString Text);
    AnsiString FormatOffsetText(int Value);
    void EditOffsetRow(TStringGrid *Grid, int Row);
    void __fastcall FormShowHandler(TObject *Sender);
    void __fastcall GridDblClick(TObject *Sender);
    void __fastcall GridMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall GridSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);
    void __fastcall btnApplyClick(TObject *Sender);
    void __fastcall btnReAlignClick(TObject *Sender);
    void __fastcall btnExitClick(TObject *Sender);
    void __fastcall btnClearClick(TObject *Sender);
    void __fastcall SetMaxClick(TObject *Sender);
    void __fastcall SetMinClick(TObject *Sender);
};
//---------------------------------------------------------------------------
extern PACKAGE TfOffset *fOffset;
extern RUN_OFFSET Offset;
//---------------------------------------------------------------------------
#endif
