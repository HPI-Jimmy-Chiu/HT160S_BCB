//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#include <string.h>
#pragma hdrstop
#include "language.h"

#include <IniFiles.hpp>
#include <SysUtils.hpp>
#include <Graphics.hpp>

#include "uOffset.h"
#include "uteach.h"
#include "uQwertyKey.h"
#include "CosFunction.h"
#include "mymessbox.h"
#include "cEventLog.h"           //AI(ht160s-offset-audit) 20260731 : offset change history sink
#include "UserRoleManager.h"     //AI(ht160s-offset-audit) 20260731 : who made the change
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
void UpdateAllParameter();   //AI 20260623 : Offset fold (cprod.cpp)
TfOffset *fOffset;
RUN_OFFSET Offset;
//---------------------------------------------------------------------------
__fastcall TfOffset::TfOffset(TComponent* Owner)
    : TForm(Owner)
{
    bUIBuilt=false;
    OffsetItemCount=0;
    popGrid=NULL;
    popRow=-1;
    bOffsetBaseCaptured=false;
    memset(OffsetBaseVal, 0, sizeof(OffsetBaseVal));
    BuildUI();
    InitialOffsetParameter();
    OpenWorkFile();
    SnapshotOffsetValues();   //AI(ht160s-offset-audit) 20260731 : baseline for the change history
}
//---------------------------------------------------------------------------
void TfOffset::BuildUI()
{
    if(bUIBuilt)
        return;

    Caption=LangT("Offset");
    Position=poScreenCenter;
    BorderStyle=bsSingle;
    ClientWidth=720;
    ClientHeight=640;
    OnShow=FormShowHandler;

    palTitle=new TPanel(this);
    palTitle->Parent=this;
    palTitle->Align=alTop;
    palTitle->Height=32;
    palTitle->Caption=LangT("Offset - per-workfile position fine-tune (mm)");
    palTitle->Font->Style=TFontStyles()<<fsBold;

    palButtons=new TPanel(this);
    palButtons->Parent=this;
    palButtons->Align=alBottom;
    palButtons->Height=52;

    btnApply=new TButton(this);
    btnApply->Parent=palButtons;
    btnApply->Left=16; btnApply->Top=10; btnApply->Width=150; btnApply->Height=32;
    btnApply->Caption="Apply / Save";
    btnApply->OnClick=btnApplyClick;

    btnReAlign=new TButton(this);
    btnReAlign->Parent=palButtons;
    btnReAlign->Left=180; btnReAlign->Top=10; btnReAlign->Width=190; btnReAlign->Height=32;
    btnReAlign->Caption=LangT("Re-alignment (bake)");
    btnReAlign->OnClick=btnReAlignClick;

    btnExit=new TButton(this);
    btnExit->Parent=palButtons;
    btnExit->Left=560; btnExit->Top=10; btnExit->Width=140; btnExit->Height=32;
    btnExit->Caption=LangT("Exit");
    btnExit->OnClick=btnExitClick;

    btnClear=new TButton(this);
    btnClear->Parent=palButtons;
    btnClear->Left=384; btnClear->Top=10; btnClear->Width=160; btnClear->Height=32;
    btnClear->Caption=LangT("Clear All");
    btnClear->OnClick=btnClearClick;

    palExplain=new TPanel(this);
    palExplain->Parent=this;
    palExplain->Align=alTop;
    palExplain->Height=26;
    palExplain->Caption="";

    lblExplain=new TLabel(this);
    lblExplain->Parent=palExplain;
    lblExplain->Align=alClient;
    lblExplain->Layout=tlCenter;
    lblExplain->Caption="Double-click a row to edit offset. Right-click for Max/Min limit.";

    popLimit=new TPopupMenu(this);
    miSetMax=new TMenuItem(popLimit);
    miSetMax->Caption="Set Max Limit ...";
    miSetMax->OnClick=SetMaxClick;
    popLimit->Items->Add(miSetMax);
    miSetMin=new TMenuItem(popLimit);
    miSetMin->Caption="Set Min Limit ...";
    miSetMin->OnClick=SetMinClick;
    popLimit->Items->Add(miSetMin);

    PageOffset=new TPageControl(this);
    PageOffset->Parent=this;
    PageOffset->Align=alClient;

    tsLoader=new TTabSheet(this);
    tsLoader->PageControl=PageOffset;
    tsLoader->Caption=LangT("Loader");
    tsSortArm=new TTabSheet(this);
    tsSortArm->PageControl=PageOffset;
    tsSortArm->Caption=LangT("SortArm");
    tsAuto=new TTabSheet(this);
    tsAuto->PageControl=PageOffset;
    tsAuto->Caption=LangT("Auto");

    grdLoader=new TStringGrid(this);
    grdLoader->Parent=tsLoader;
    grdLoader->Align=alClient;
    grdSortArm=new TStringGrid(this);
    grdSortArm->Parent=tsSortArm;
    grdSortArm->Align=alClient;
    grdAuto=new TStringGrid(this);
    grdAuto->Parent=tsAuto;
    grdAuto->Align=alClient;

    ConfigureGrid(grdLoader);
    ConfigureGrid(grdSortArm);
    ConfigureGrid(grdAuto);

    edScratch=new TEdit(this);
    edScratch->Parent=this;
    edScratch->Visible=false;

    PageOffset->ActivePageIndex=0;
    bUIBuilt=true;
}
//---------------------------------------------------------------------------
void TfOffset::ConfigureGrid(TStringGrid *Grid)
{
    if(Grid==NULL)
        return;
    Grid->ColCount=4;
    Grid->FixedCols=0;
    Grid->FixedRows=1;
    Grid->RowCount=2;
    Grid->DefaultRowHeight=24;
    Grid->Options=Grid->Options << goRowSelect << goColSizing << goVertLine << goHorzLine;
    Grid->Cells[0][0]=LangT("Offset Item");
    Grid->Cells[1][0]=LangT("Offset(mm)");
    Grid->Cells[2][0]=LangT("Min(mm)");
    Grid->Cells[3][0]=LangT("Max(mm)");
    Grid->ColWidths[0]=300;
    Grid->ColWidths[1]=110;
    Grid->ColWidths[2]=95;
    Grid->ColWidths[3]=95;
    Grid->OnDblClick=GridDblClick;
    Grid->OnMouseDown=GridMouseDown;
    Grid->OnSelectCell=GridSelectCell;
    Grid->PopupMenu=popLimit;
}
//---------------------------------------------------------------------------
void TfOffset::ResetGrid(TStringGrid *Grid)
{
    if(Grid==NULL)
        return;
    Grid->RowCount=2;
    for(int Col=0; Col<Grid->ColCount; Col++)
        Grid->Cells[Col][1]="";
}
//---------------------------------------------------------------------------
void TfOffset::AddOffsetItem(TStringGrid *Grid, AnsiString GroupName, AnsiString Caption, int *iPara, int iMax, int iMin)
{
    int Index, Row;
    if(Grid==NULL || iPara==NULL || OffsetItemCount>=OFFSET_MAX_ITEM)
        return;
    Index=OffsetItemCount;
    Row=1;
    if(Grid->Cells[0][1]!=AnsiString(""))
    {
        Row=Grid->RowCount;
        Grid->RowCount=Grid->RowCount+1;
    }
    OffsetPara[Index].iPara=iPara;
    OffsetPara[Index].GroupName=GroupName;
    OffsetPara[Index].Caption=Caption;
    OffsetPara[Index].iMax=iMax;
    OffsetPara[Index].iMin=iMin;
    OffsetPara[Index].Grid=Grid;
    OffsetPara[Index].Row=Row;
    //AI 20260701 : seed the caption cell NOW (mirror uteach.cpp AddTeachItem:237).
    //The next AddOffsetItem tests Cells[0][1]!="" to decide a new row; without this
    //seed every item collapsed onto Row 1 and RowCount never grew (blank grids).
    Grid->Cells[0][Row]=Caption;
    OffsetItemCount++;
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::InitialOffsetParameter()
{
    if(bUIBuilt==false)
        BuildUI();
    OffsetItemCount=0;
    ResetGrid(grdLoader);
    ResetGrid(grdSortArm);
    ResetGrid(grdAuto);
    AddOffsetItem(grdLoader, "OffsetLoader", "Loader1CarFeedTrayYPosition", &Offset.Loader1CarFeedTrayYPosition, 100000, -100000);
    AddOffsetItem(grdLoader, "OffsetLoader", "Loader1CarDischargeTrayYPosition", &Offset.Loader1CarDischargeTrayYPosition, 100000, -100000);
    AddOffsetItem(grdLoader, "OffsetLoader", "Loader1CarFirstCCDYPosition", &Offset.Loader1CarFirstCCDYPosition, 100000, -100000);
    AddOffsetItem(grdLoader, "OffsetLoader", "Loader1CarFirstSortYPosition", &Offset.Loader1CarFirstSortYPosition, 100000, -100000);
    AddOffsetItem(grdLoader, "OffsetLoader", "Loader2CarFeedTrayYPosition", &Offset.Loader2CarFeedTrayYPosition, 100000, -100000);
    AddOffsetItem(grdLoader, "OffsetLoader", "Loader2CarDischargeTrayYPosition", &Offset.Loader2CarDischargeTrayYPosition, 100000, -100000);
    AddOffsetItem(grdLoader, "OffsetLoader", "Loader2CarFirstCCDYPosition", &Offset.Loader2CarFirstCCDYPosition, 100000, -100000);
    AddOffsetItem(grdLoader, "OffsetLoader", "Loader2CarFirstSortYPosition", &Offset.Loader2CarFirstSortYPosition, 100000, -100000);
    AddOffsetItem(grdLoader, "OffsetLoader", "LoaderCarFirstCCDXPosition", &Offset.LoaderCarFirstCCDXPosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader1XPosition", &Offset.SortArmToLoader1XPosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader2XPosition", &Offset.SortArmToLoader2XPosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto1XPosition", &Offset.SortArmToAuto1XPosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto2XPosition", &Offset.SortArmToAuto2XPosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto3XPosition", &Offset.SortArmToAuto3XPosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto4XPosition", &Offset.SortArmToAuto4XPosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto5XPosition", &Offset.SortArmToAuto5XPosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto6XPosition", &Offset.SortArmToAuto6XPosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToBottomCCDFirstXPosition", &Offset.SortArmToBottomCCDFirstXPosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_1_Z1Position", &Offset.SortArmToLoader_1_Z1Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_1_Z2Position", &Offset.SortArmToLoader_1_Z2Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_1_Z3Position", &Offset.SortArmToLoader_1_Z3Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_1_Z4Position", &Offset.SortArmToLoader_1_Z4Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_2_Z1Position", &Offset.SortArmToLoader_2_Z1Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_2_Z2Position", &Offset.SortArmToLoader_2_Z2Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_2_Z3Position", &Offset.SortArmToLoader_2_Z3Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_2_Z4Position", &Offset.SortArmToLoader_2_Z4Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_1_Z1Position", &Offset.SortArmToAuto_1_Z1Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_1_Z2Position", &Offset.SortArmToAuto_1_Z2Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_1_Z3Position", &Offset.SortArmToAuto_1_Z3Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_1_Z4Position", &Offset.SortArmToAuto_1_Z4Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_2_Z1Position", &Offset.SortArmToAuto_2_Z1Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_2_Z2Position", &Offset.SortArmToAuto_2_Z2Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_2_Z3Position", &Offset.SortArmToAuto_2_Z3Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_2_Z4Position", &Offset.SortArmToAuto_2_Z4Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_3_Z1Position", &Offset.SortArmToAuto_3_Z1Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_3_Z2Position", &Offset.SortArmToAuto_3_Z2Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_3_Z3Position", &Offset.SortArmToAuto_3_Z3Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_3_Z4Position", &Offset.SortArmToAuto_3_Z4Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_4_Z1Position", &Offset.SortArmToAuto_4_Z1Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_4_Z2Position", &Offset.SortArmToAuto_4_Z2Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_4_Z3Position", &Offset.SortArmToAuto_4_Z3Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_4_Z4Position", &Offset.SortArmToAuto_4_Z4Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_5_Z1Position", &Offset.SortArmToAuto_5_Z1Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_5_Z2Position", &Offset.SortArmToAuto_5_Z2Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_5_Z3Position", &Offset.SortArmToAuto_5_Z3Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_5_Z4Position", &Offset.SortArmToAuto_5_Z4Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_6_Z1Position", &Offset.SortArmToAuto_6_Z1Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_6_Z2Position", &Offset.SortArmToAuto_6_Z2Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_6_Z3Position", &Offset.SortArmToAuto_6_Z3Position, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_6_Z4Position", &Offset.SortArmToAuto_6_Z4Position, 100000, -100000);
    AddOffsetItem(grdAuto, "OffsetAuto", "Auto1CarFirstSortYPosition", &Offset.Auto1CarFirstSortYPosition, 100000, -100000);
    AddOffsetItem(grdAuto, "OffsetAuto", "Auto2CarFirstSortYPosition", &Offset.Auto2CarFirstSortYPosition, 100000, -100000);
    AddOffsetItem(grdAuto, "OffsetAuto", "Auto3CarFirstSortYPosition", &Offset.Auto3CarFirstSortYPosition, 100000, -100000);
    AddOffsetItem(grdAuto, "OffsetAuto", "Auto4CarFirstSortYPosition", &Offset.Auto4CarFirstSortYPosition, 100000, -100000);
    AddOffsetItem(grdAuto, "OffsetAuto", "Auto5CarFirstSortYPosition", &Offset.Auto5CarFirstSortYPosition, 100000, -100000);
    AddOffsetItem(grdAuto, "OffsetAuto", "Auto6CarFirstSortYPosition", &Offset.Auto6CarFirstSortYPosition, 100000, -100000);
    LoadOffsetLimits();
    RefreshGrids();
}
//---------------------------------------------------------------------------
int TfOffset::FindItem(TStringGrid *Grid, int Row)
{
    for(int i=0; i<OffsetItemCount; i++)
        if(OffsetPara[i].Grid==Grid && OffsetPara[i].Row==Row)
            return i;
    return -1;
}
//---------------------------------------------------------------------------
void TfOffset::RefreshGrids()
{
    for(int i=0; i<OffsetItemCount; i++)
        RefreshRow(i);
}
//---------------------------------------------------------------------------
void TfOffset::RefreshRow(int Index)
{
    TStringGrid *Grid;
    int Row;
    if(Index<0 || Index>=OffsetItemCount)
        return;
    Grid=OffsetPara[Index].Grid;
    Row=OffsetPara[Index].Row;
    if(Grid==NULL)
        return;
    Grid->Cells[0][Row]=OffsetPara[Index].Caption;
    Grid->Cells[1][Row]=FormatOffsetText(*OffsetPara[Index].iPara);
    Grid->Cells[2][Row]=FormatOffsetText(OffsetPara[Index].iMin);
    Grid->Cells[3][Row]=FormatOffsetText(OffsetPara[Index].iMax);
}
//---------------------------------------------------------------------------
int TfOffset::ParseOffsetText(AnsiString Text)
{
    double Value;
    if(Text==AnsiString(""))
        return 0;
    Value=atof(Text.c_str());
    if(Value>=0.0)
        return (int)(Value*100.0+0.5);
    return (int)(Value*100.0-0.5);
}
//---------------------------------------------------------------------------
AnsiString TfOffset::FormatOffsetText(int Value)
{
    AnsiString S;
    S.sprintf("%.2f", (double)Value/100.0);
    return S;
}
//---------------------------------------------------------------------------
AnsiString TfOffset::GetOffsetKey(int Index)
{
    if(Index<0 || Index>=OffsetItemCount)
        return "";
    return AnsiString("ed_")+OffsetPara[Index].Caption;
}
//---------------------------------------------------------------------------
AnsiString TfOffset::GetOffsetFileName()
{
    AnsiString wf;
    wf=RecipeManager.GetCurrentRecipeName();
    if(wf==AnsiString(""))
        return HSys.CurrentDir+AnsiString("\\system\\offset.ini");
    return HSys.CurrentDir+AnsiString("\\data\\")+wf+AnsiString(".ofs");
}
//---------------------------------------------------------------------------
AnsiString TfOffset::GetLimitFileName()
{
    return HSys.CurrentDir+AnsiString("\\system\\OffsetLimit.ini");
}
//---------------------------------------------------------------------------
void TfOffset::SeedLimitFile()
{
    AnsiString S;
    TIniFile *Ini;
    S=GetLimitFileName();
    ForceDirectories(ExtractFilePath(S));
    Ini=new TIniFile(S);
    for(int i=0; i<OffsetItemCount; i++)
    {
        Ini->WriteString("Offset", OffsetPara[i].Caption+AnsiString("_Max"), FormatOffsetText(OffsetPara[i].iMax));
        Ini->WriteString("Offset", OffsetPara[i].Caption+AnsiString("_Min"), FormatOffsetText(OffsetPara[i].iMin));
    }
    delete Ini;
}
//---------------------------------------------------------------------------
void TfOffset::LoadOffsetLimits()
{
    AnsiString S, V;
    TIniFile *Ini;
    S=GetLimitFileName();
    if(FileExists(S)==false)
    {
        SeedLimitFile();
        return;
    }
    Ini=new TIniFile(S);
    for(int i=0; i<OffsetItemCount; i++)
    {
        V=Ini->ReadString("Offset", OffsetPara[i].Caption+AnsiString("_Max"), "");
        if(V!=AnsiString(""))
            OffsetPara[i].iMax=ParseOffsetText(V);
        V=Ini->ReadString("Offset", OffsetPara[i].Caption+AnsiString("_Min"), "");
        if(V!=AnsiString(""))
            OffsetPara[i].iMin=ParseOffsetText(V);
    }
    delete Ini;
}
//---------------------------------------------------------------------------
void TfOffset::SaveOneLimit(int Index)
{
    AnsiString S;
    TIniFile *Ini;
    if(Index<0 || Index>=OffsetItemCount)
        return;
    S=GetLimitFileName();
    ForceDirectories(ExtractFilePath(S));
    Ini=new TIniFile(S);
    Ini->WriteString("Offset", OffsetPara[Index].Caption+AnsiString("_Max"), FormatOffsetText(OffsetPara[Index].iMax));
    Ini->WriteString("Offset", OffsetPara[Index].Caption+AnsiString("_Min"), FormatOffsetText(OffsetPara[Index].iMin));
    delete Ini;
}
//---------------------------------------------------------------------------
AnsiString TfOffset::GetOffsetExplain(AnsiString Caption)
{
    if(Caption.Pos("_Z")>0)
        return "SortArm sucker Z height fine-tune at this station";
    if(Caption.Pos("BottomCCDFirstX")>0)
        return LangT("SortArm X stop at Bottom-CCD first column");
    if(Caption.Pos("SortArmTo")==1)
        return LangT("SortArm X stop fine-tune at this station");
    if(Caption.Pos("LoaderCarFirstCCDX")==1)
        return LangT("Loader top-CCD X capture fine-tune");
    if(Caption.Pos("Loader")==1)
        return "Loader carriage Y fine-tune (feed/discharge/CCD/sort)";
    if(Caption.Pos("Auto")==1)
        return LangT("Auto carriage first-sort Y fine-tune");
    return "Position offset (mm), added on top of base teach";
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::OpenWorkFile()
{
    AnsiString S, Value;
    TIniFile *Ini;
    if(bUIBuilt==false)
        BuildUI();
    if(OffsetItemCount==0)
        InitialOffsetParameter();
    for(int i=0; i<OffsetItemCount; i++)
        if(OffsetPara[i].iPara!=NULL)
            *OffsetPara[i].iPara=0;
    S=GetOffsetFileName();
    if(FileExists(S))
    {
        Ini=new TIniFile(S);
        for(int i=0; i<OffsetItemCount; i++)
        {
            if(OffsetPara[i].iPara!=NULL)
            {
                Value=Ini->ReadString(OffsetPara[i].GroupName, GetOffsetKey(i), "");
                if(Value!=AnsiString(""))
                    *OffsetPara[i].iPara=ParseOffsetText(Value);
            }
        }
        delete Ini;
    }
    RefreshGrids();
    UpdateAllParameter();
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::SaveWorkFile()
{
    AnsiString S;
    TIniFile *Ini;
    S=GetOffsetFileName();
    ForceDirectories(ExtractFilePath(S));
    Ini=new TIniFile(S);
    for(int i=0; i<OffsetItemCount; i++)
        if(OffsetPara[i].iPara!=NULL)
            Ini->WriteString(OffsetPara[i].GroupName, GetOffsetKey(i), FormatOffsetText(*OffsetPara[i].iPara));
    delete Ini;
}
//---------------------------------------------------------------------------
void TfOffset::EditOffsetRow(TStringGrid *Grid, int Row)
{
    int idx;
    double mn, mx;
    idx=FindItem(Grid, Row);
    if(idx<0)
        return;
    mn=(double)OffsetPara[idx].iMin/100.0;
    mx=(double)OffsetPara[idx].iMax/100.0;
    edScratch->Text=FormatOffsetText(*OffsetPara[idx].iPara);
    if(fQwertyKey->ShowQwertyKey(edScratch, N_DOUBLE, 2, true, mn, mx, OffsetPara[idx].Caption))
    {
        *OffsetPara[idx].iPara=ParseOffsetText(edScratch->Text);
        RefreshRow(idx);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::GridDblClick(TObject *Sender)
{
    TStringGrid *Grid;
    Grid=dynamic_cast<TStringGrid *>(Sender);
    if(Grid==NULL)
        return;
    EditOffsetRow(Grid, Grid->Row);
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::GridMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    TStringGrid *Grid;
    int Col, Row;
    (void)Shift;
    Grid=(TStringGrid *)Sender;
    if(Button!=mbRight)
        return;
    Col=0; Row=0;
    Grid->MouseToCell(X, Y, Col, Row);
    if(Row<1)
        return;
    popGrid=Grid;
    popRow=Row;
    Grid->Row=Row;
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::GridSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{
    TStringGrid *Grid;
    int idx;
    (void)ACol;
    CanSelect=true;
    Grid=(TStringGrid *)Sender;
    idx=FindItem(Grid, ARow);
    if(idx<0)
    {
        lblExplain->Caption="";
        return;
    }
    lblExplain->Caption=OffsetPara[idx].Caption+AnsiString("  :  ")+GetOffsetExplain(OffsetPara[idx].Caption);
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::FormShowHandler(TObject *Sender)
{
    (void)Sender;
    OpenWorkFile();
    SnapshotOffsetValues();   //AI(ht160s-offset-audit) 20260731 : OpenWorkFile reloads from the .ofs and silently discards any un-Applied edit, so re-baseline here
}
//---------------------------------------------------------------------------
//AI(ht160s-offset-audit) 20260731 : capture the CURRENTLY PERSISTED values as the change-history
//baseline. Called after every OpenWorkFile (form show / construct) and after every successful
//save, so the baseline always mirrors what is on disk.
void TfOffset::SnapshotOffsetValues()
{
    int i;
    for(i=0; i<OffsetItemCount && i<OFFSET_MAX_ITEM; i++)
    {
        if(OffsetPara[i].iPara==NULL)
            OffsetBaseVal[i]=0;
        else
            OffsetBaseVal[i]=*OffsetPara[i].iPara;
    }
    bOffsetBaseCaptured=true;
}
//---------------------------------------------------------------------------
//AI(ht160s-offset-audit) 20260731 : HT9045-style parameter change history (handlerlog.cpp
//TMyLog::Compare_Diff shape : user / field / old => new). One EventLog row per CHANGED field,
//written at APPLY time - never at edit time. Keypad-accepted values live only in the in-RAM
//Offset struct; btnExit just Close()s and the next FormShow reloads the .ofs, silently dropping
//them, so an edit-time log would record changes that never happened. Nothing here can stop the
//machine or block the save : logging is write-only and must never become a new failure mode.
void TfOffset::LogOffsetChanges(AnsiString sAction)
{
    AnsiString sUser;
    AnsiString sMsg;
    int i;

    if(bOffsetBaseCaptured==false)
        return;
    sUser=UserRoleManager.GetUserID();
    if(sUser.Trim()=="")
        sUser="(no login)";
    for(i=0; i<OffsetItemCount && i<OFFSET_MAX_ITEM; i++)
    {
        if(OffsetPara[i].iPara==NULL)
            continue;
        if(*OffsetPara[i].iPara==OffsetBaseVal[i])
            continue;
        sMsg=sAction+AnsiString(" | user=")+sUser
            +AnsiString(" | ")+OffsetPara[i].GroupName+AnsiString(".")+OffsetPara[i].Caption
            +AnsiString(" : ")+FormatOffsetText(OffsetBaseVal[i])
            +AnsiString(" => ")+FormatOffsetText(*OffsetPara[i].iPara);
        g_EventLog.Log("PARAM_OFFSET", sMsg, OffsetPara[i].Caption);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::btnApplyClick(TObject *Sender)
{
    (void)Sender;
    if(ShowMyMessageBox_YES_NO("Save offset and apply ?")!=TMyMessageBox::msgrtnYES)
        return;
    SaveWorkFile();
    UpdateAllParameter();
    //AI(ht160s-offset-audit) 20260731 : the completion popup that used to sit here was
    //ShowMyMessage(), i.e. the STOPPING alarm surface - it calls DecStopAllMotor() and clears
    //SystemStart before it even paints, keeps the buzzer button visible so it beeps, and drives
    //the tower light to Pause. The machine alarmed and stopped in order to announce that a save
    //SUCCEEDED, and together with the YES/NO confirm above that is the "offset jumps two alarms"
    //the operator reported on site 20260730 (SECS CEID 19 -> 73 -> 73, three visits, ~1 s apart).
    //Feedback is now a non-modal caption, and the change itself is recorded to the EventLog
    //(HT9045 parameter-history alignment) instead of being announced and then forgotten.
    LogOffsetChanges("Offset apply");
    SnapshotOffsetValues();
    lblExplain->Caption=LangT("Offset saved and applied.");
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::btnReAlignClick(TObject *Sender)
{
    (void)Sender;
    if(ShowMyMessageBox_YES_NO("Re-alignment: fold current offset into base teach and reset offset to 0 ?")!=TMyMessageBox::msgrtnYES)
        return;
    TeachBase.Loader1CarFeedTrayYPosition += Offset.Loader1CarFeedTrayYPosition;
    TeachBase.Loader1CarDischargeTrayYPosition += Offset.Loader1CarDischargeTrayYPosition;
    TeachBase.Loader1CarFirstCCDYPosition += Offset.Loader1CarFirstCCDYPosition;
    TeachBase.Loader1CarFirstSortYPosition += Offset.Loader1CarFirstSortYPosition;
    TeachBase.Loader2CarFeedTrayYPosition += Offset.Loader2CarFeedTrayYPosition;
    TeachBase.Loader2CarDischargeTrayYPosition += Offset.Loader2CarDischargeTrayYPosition;
    TeachBase.Loader2CarFirstCCDYPosition += Offset.Loader2CarFirstCCDYPosition;
    TeachBase.Loader2CarFirstSortYPosition += Offset.Loader2CarFirstSortYPosition;
    TeachBase.LoaderCarFirstCCDXPosition += Offset.LoaderCarFirstCCDXPosition;
    TeachBase.SortArmToLoader1XPosition += Offset.SortArmToLoader1XPosition;
    TeachBase.SortArmToLoader2XPosition += Offset.SortArmToLoader2XPosition;
    TeachBase.SortArmToAuto1XPosition += Offset.SortArmToAuto1XPosition;
    TeachBase.SortArmToAuto2XPosition += Offset.SortArmToAuto2XPosition;
    TeachBase.SortArmToAuto3XPosition += Offset.SortArmToAuto3XPosition;
    TeachBase.SortArmToAuto4XPosition += Offset.SortArmToAuto4XPosition;
    TeachBase.SortArmToAuto5XPosition += Offset.SortArmToAuto5XPosition;
    TeachBase.SortArmToAuto6XPosition += Offset.SortArmToAuto6XPosition;
    TeachBase.SortArmToBottomCCDFirstXPosition += Offset.SortArmToBottomCCDFirstXPosition;
    TeachBase.SortArmToLoader_1_Z1Position += Offset.SortArmToLoader_1_Z1Position;
    TeachBase.SortArmToLoader_1_Z2Position += Offset.SortArmToLoader_1_Z2Position;
    TeachBase.SortArmToLoader_1_Z3Position += Offset.SortArmToLoader_1_Z3Position;
    TeachBase.SortArmToLoader_1_Z4Position += Offset.SortArmToLoader_1_Z4Position;
    TeachBase.SortArmToLoader_2_Z1Position += Offset.SortArmToLoader_2_Z1Position;
    TeachBase.SortArmToLoader_2_Z2Position += Offset.SortArmToLoader_2_Z2Position;
    TeachBase.SortArmToLoader_2_Z3Position += Offset.SortArmToLoader_2_Z3Position;
    TeachBase.SortArmToLoader_2_Z4Position += Offset.SortArmToLoader_2_Z4Position;
    TeachBase.SortArmToAuto_1_Z1Position += Offset.SortArmToAuto_1_Z1Position;
    TeachBase.SortArmToAuto_1_Z2Position += Offset.SortArmToAuto_1_Z2Position;
    TeachBase.SortArmToAuto_1_Z3Position += Offset.SortArmToAuto_1_Z3Position;
    TeachBase.SortArmToAuto_1_Z4Position += Offset.SortArmToAuto_1_Z4Position;
    TeachBase.SortArmToAuto_2_Z1Position += Offset.SortArmToAuto_2_Z1Position;
    TeachBase.SortArmToAuto_2_Z2Position += Offset.SortArmToAuto_2_Z2Position;
    TeachBase.SortArmToAuto_2_Z3Position += Offset.SortArmToAuto_2_Z3Position;
    TeachBase.SortArmToAuto_2_Z4Position += Offset.SortArmToAuto_2_Z4Position;
    TeachBase.SortArmToAuto_3_Z1Position += Offset.SortArmToAuto_3_Z1Position;
    TeachBase.SortArmToAuto_3_Z2Position += Offset.SortArmToAuto_3_Z2Position;
    TeachBase.SortArmToAuto_3_Z3Position += Offset.SortArmToAuto_3_Z3Position;
    TeachBase.SortArmToAuto_3_Z4Position += Offset.SortArmToAuto_3_Z4Position;
    TeachBase.SortArmToAuto_4_Z1Position += Offset.SortArmToAuto_4_Z1Position;
    TeachBase.SortArmToAuto_4_Z2Position += Offset.SortArmToAuto_4_Z2Position;
    TeachBase.SortArmToAuto_4_Z3Position += Offset.SortArmToAuto_4_Z3Position;
    TeachBase.SortArmToAuto_4_Z4Position += Offset.SortArmToAuto_4_Z4Position;
    TeachBase.SortArmToAuto_5_Z1Position += Offset.SortArmToAuto_5_Z1Position;
    TeachBase.SortArmToAuto_5_Z2Position += Offset.SortArmToAuto_5_Z2Position;
    TeachBase.SortArmToAuto_5_Z3Position += Offset.SortArmToAuto_5_Z3Position;
    TeachBase.SortArmToAuto_5_Z4Position += Offset.SortArmToAuto_5_Z4Position;
    TeachBase.SortArmToAuto_6_Z1Position += Offset.SortArmToAuto_6_Z1Position;
    TeachBase.SortArmToAuto_6_Z2Position += Offset.SortArmToAuto_6_Z2Position;
    TeachBase.SortArmToAuto_6_Z3Position += Offset.SortArmToAuto_6_Z3Position;
    TeachBase.SortArmToAuto_6_Z4Position += Offset.SortArmToAuto_6_Z4Position;
    TeachBase.Auto1CarFirstSortYPosition += Offset.Auto1CarFirstSortYPosition;
    TeachBase.Auto2CarFirstSortYPosition += Offset.Auto2CarFirstSortYPosition;
    TeachBase.Auto3CarFirstSortYPosition += Offset.Auto3CarFirstSortYPosition;
    TeachBase.Auto4CarFirstSortYPosition += Offset.Auto4CarFirstSortYPosition;
    TeachBase.Auto5CarFirstSortYPosition += Offset.Auto5CarFirstSortYPosition;
    TeachBase.Auto6CarFirstSortYPosition += Offset.Auto6CarFirstSortYPosition;
    memset(&Offset, 0, sizeof(Offset));
    if(fTeach!=NULL)
        fTeach->SaveWorkFile(HSys.CurrentDir+AnsiString("\\system\\tech.ini"));
    SaveWorkFile();
    UpdateAllParameter();
    RefreshGrids();
    //AI(ht160s-offset-audit) 20260731 : same swap as btnApply - a success notice must not run the
    //stopping alarm box. Re-alignment is the destructive one (it rewrites system\tech.ini and
    //zeroes all 56 offsets), so its every field change is worth a history row.
    LogOffsetChanges("Offset re-align (folded into base teach)");
    SnapshotOffsetValues();
    lblExplain->Caption=LangT("Re-alignment done: offset folded into base teach.");
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::btnExitClick(TObject *Sender)
{
    (void)Sender;
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::btnClearClick(TObject *Sender)
{
    (void)Sender;
    if(ShowMyMessageBox_YES_NO("Clear ALL offset values to 0 ? (press Apply to save and take effect)")!=TMyMessageBox::msgrtnYES)
        return;
    memset(&Offset, 0, sizeof(Offset));
    RefreshGrids();
    //AI(ht160s-offset-audit) 20260731 : Clear All writes NOTHING to disk, yet it used to run the
    //stopping alarm box too. No history row here on purpose - the zeros are not persisted until
    //Apply, and btnApply's LogOffsetChanges will record them against the same on-disk baseline.
    lblExplain->Caption=LangT("All offsets cleared. Press Apply to save and take effect, or Exit to discard.");
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::SetMaxClick(TObject *Sender)
{
    int idx;
    (void)Sender;
    idx=FindItem(popGrid, popRow);
    if(idx<0)
        return;
    edScratch->Text=FormatOffsetText(OffsetPara[idx].iMax);
    if(fQwertyKey->ShowQwertyKey(edScratch, N_DOUBLE, 2, false, 0, 0, OffsetPara[idx].Caption+AnsiString(" Max(mm)")))
    {
        OffsetPara[idx].iMax=ParseOffsetText(edScratch->Text);
        SaveOneLimit(idx);
        RefreshRow(idx);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfOffset::SetMinClick(TObject *Sender)
{
    int idx;
    (void)Sender;
    idx=FindItem(popGrid, popRow);
    if(idx<0)
        return;
    edScratch->Text=FormatOffsetText(OffsetPara[idx].iMin);
    if(fQwertyKey->ShowQwertyKey(edScratch, N_DOUBLE, 2, false, 0, 0, OffsetPara[idx].Caption+AnsiString(" Min(mm)")))
    {
        OffsetPara[idx].iMin=ParseOffsetText(edScratch->Text);
        SaveOneLimit(idx);
        RefreshRow(idx);
    }
}
//---------------------------------------------------------------------------
