//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#include <stdio.h>
#pragma hdrstop

#include <IniFiles.hpp>
#include <SysUtils.hpp>

#include "uteach.h"
#include "iosetview.h"
#include "csystem.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "ALed"
#pragma resource "*.dfm"
TfTeach *fTeach;
TEACH Teach;
//---------------------------------------------------------------------------
// Static layout/colors/captions for the Teach form now live in uteach.dfm. Only
// the runtime LED color convention stays here (UpdateStatusLed switches between
// these per motor state).
static const TColor TEACH_COLOR_LED_OFF=(TColor)12632256;
static const TColor TEACH_COLOR_LED_ON=(TColor)65280;
// AI(general) 20260616 : LED color convention (per RD): a signal that is in use
// shows green when triggered and gray when idle; red is reserved for "not in
// use" (the active motor is disabled). Applies to all status LEDs uniformly.
static const TColor TEACH_COLOR_LED_DISABLED=(TColor)255;
//---------------------------------------------------------------------------
__fastcall TfTeach::TfTeach(TComponent* Owner)
    : TForm(Owner)
{
    bUIBuilt=false;
    bTeachReady=false;
    bSelectingTeachItem=false;
    bHomeRunning=false;
    ActiveMotorIndex=-1;
    iHomeMotorIndex=-1;
    SelectedTeachIndex=-1;
    TECH_MAX_ITEM=0;

    // UI is fully defined in uteach.dfm; VCL auto-binds the __published members on
    // form streaming. Only the lblStatus[]/ledStatus[] arrays are non-published and
    // are mapped from the named DFM LEDs in BindDfmComponents (TfMotorTest pattern).
    for(int i=0; i<iMotLedTotalCnt; i++)
    {
        lblStatus[i]=NULL;
        ledStatus[i]=NULL;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::FormCreate(TObject *Sender)
{
    (void)Sender;
    BuildUI();
    InitialTeachParameter();
    OpenWorkFile();
    FillMotorList();
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::FormShow(TObject *Sender)
{
    (void)Sender;
    if(bUIBuilt==false)
        BuildUI();
    if(bTeachReady==false)
        InitialTeachParameter();
    OpenWorkFile();
    FillMotorList();
    if(SelectedTeachIndex<0 && TECH_MAX_ITEM>0)
        SelectTeachItem(0);
    if(tmrUpdate!=NULL)
        tmrUpdate->Enabled=true;
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::FormClose(TObject *Sender, TCloseAction &Action)
{
    (void)Sender;
    (void)Action;
    if(tmrUpdate!=NULL)
        tmrUpdate->Enabled=false;
    StopActiveMotor();
}
//---------------------------------------------------------------------------
void TfTeach::BuildUI()
{
    if(bUIBuilt)
        return;

    // All controls live in uteach.dfm (auto-bound). Only map the named status LEDs
    // into the arrays and apply the runtime grid setup (headers/widths cannot be
    // stored in the DFM for a TStringGrid).
    BindDfmComponents();

    ConfigureTeachGrid(grdEmptyTray);
    ConfigureTeachGrid(grdLoaderSort);
    ConfigureTeachGrid(grdAuto);
    ConfigureTeachGrid(grdSortZ);
    ConfigureTeachGrid(grdOthers);

    bUIBuilt=true;
}
//---------------------------------------------------------------------------
void TfTeach::BindDfmComponents()
{
    lblStatus[0]=lblStatus0;
    lblStatus[1]=lblStatus1;
    lblStatus[2]=lblStatus2;
    lblStatus[3]=lblStatus3;
    lblStatus[4]=lblStatus4;
    lblStatus[5]=lblStatus5;
    lblStatus[6]=lblStatus6;
    lblStatus[7]=lblStatus7;
    lblStatus[8]=lblStatus8;
    lblStatus[9]=lblStatus9;
    lblStatus[10]=lblStatus10;

    ledStatus[0]=ledStatus0;
    ledStatus[1]=ledStatus1;
    ledStatus[2]=ledStatus2;
    ledStatus[3]=ledStatus3;
    ledStatus[4]=ledStatus4;
    ledStatus[5]=ledStatus5;
    ledStatus[6]=ledStatus6;
    ledStatus[7]=ledStatus7;
    ledStatus[8]=ledStatus8;
    ledStatus[9]=ledStatus9;
    ledStatus[10]=ledStatus10;
}
//---------------------------------------------------------------------------
void TfTeach::ConfigureTeachGrid(TStringGrid *Grid)
{
    if(Grid==NULL)
        return;
    // Layout/colors/events are set in the DFM; only the header row text and column
    // widths must be applied in code (a TStringGrid does not stream cell content).
    Grid->Options=Grid->Options << goRowSelect << goColSizing;
    Grid->Cells[0][0]="Teach Position";
    Grid->Cells[1][0]="Motor";
    Grid->Cells[2][0]="Teach(mm)";
    Grid->Cells[3][0]="Now(mm)";
    Grid->Cells[4][0]="Soft Limit(mm)";
    Grid->ColWidths[0]=250;
    Grid->ColWidths[1]=210;
    Grid->ColWidths[2]=90;
    Grid->ColWidths[3]=90;
    Grid->ColWidths[4]=150;
}
//---------------------------------------------------------------------------
void TfTeach::ResetTeachGrid(TStringGrid *Grid)
{
    if(Grid==NULL)
        return;
    Grid->RowCount=2;
    for(int Col=0; Col<Grid->ColCount; Col++)
        Grid->Cells[Col][1]="";
}
//---------------------------------------------------------------------------
void TfTeach::AddTeachItem(TStringGrid *Grid, AnsiString GroupName, AnsiString Caption, TTrayMotor *Motor, int *iPara)
{
    int Index;
    int Row;

    if(Grid==NULL || iPara==NULL || TECH_MAX_ITEM>=MAX_TEACH_ITEM)
        return;

    Index=TECH_MAX_ITEM;
    Row=1;
    if(Grid->Cells[0][1]!=AnsiString(""))
    {
        Row=Grid->RowCount;
        Grid->RowCount=Grid->RowCount+1;
    }

    TechPara[Index].MotorSelect=(Motor==NULL)?-1:Motor->Tag;
    TechPara[Index].iPara=iPara;
    TechPara[Index].GroupName=GroupName;
    TechPara[Index].Caption=Caption;
    TechPara[Index].Grid=Grid;
    TechPara[Index].Row=Row;

    Grid->Cells[0][Row]=Caption;
    Grid->Cells[1][Row]=GetMotorCaption(TechPara[Index].MotorSelect);
    Grid->Cells[2][Row]=FormatPositionText(*iPara);
    Grid->Cells[3][Row]="";
    Grid->Cells[4][Row]=GetSoftLimitCaption(Motor);

    TECH_MAX_ITEM++;
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::InitialTeachParameter()
{
    if(bUIBuilt==false)
        return;

    TECH_MAX_ITEM=0;
    ResetTeachGrid(grdEmptyTray);
    ResetTeachGrid(grdLoaderSort);
    ResetTeachGrid(grdAuto);
    ResetTeachGrid(grdSortZ);
    ResetTeachGrid(grdOthers);

    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "EmptyCarFeedTrayYPosition", HSys.Mot.MEmptyY, &Teach.EmptyCarFeedTrayYPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "EmptyCarDischargeTrayYPosition", HSys.Mot.MEmptyY, &Teach.EmptyCarDischargeTrayYPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToEmptyXPosition", HSys.Mot.MTrayArmX, &Teach.TrayXArmToEmptyXPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToLoaderXPosition", HSys.Mot.MTrayArmX, &Teach.TrayXArmToLoaderXPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToColorXPosition", HSys.Mot.MTrayArmX, &Teach.TrayXArmToColorXPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "ColorRead2DXPosition", HSys.Mot.MTopCCDX_Color, &Teach.ColorRead2DXPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto1XPosition", HSys.Mot.MTrayArmX, &Teach.TrayXArmToAuto1XPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto2XPosition", HSys.Mot.MTrayArmX, &Teach.TrayXArmToAuto2XPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto3XPosition", HSys.Mot.MTrayArmX, &Teach.TrayXArmToAuto3XPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto4XPosition", HSys.Mot.MTrayArmX, &Teach.TrayXArmToAuto4XPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto5XPosition", HSys.Mot.MTrayArmX, &Teach.TrayXArmToAuto5XPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto6XPosition", HSys.Mot.MTrayArmX, &Teach.TrayXArmToAuto6XPosition);

    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader1CarFeedTrayYPosition", HSys.Mot.MLoaderY_1, &Teach.Loader1CarFeedTrayYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader1CarDischargeTrayYPosition", HSys.Mot.MLoaderY_1, &Teach.Loader1CarDischargeTrayYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader1CarFirstCCDYPosition", HSys.Mot.MLoaderY_1, &Teach.Loader1CarFirstCCDYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader1CarFirstSortYPosition", HSys.Mot.MLoaderY_1, &Teach.Loader1CarFirstSortYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader2CarFeedTrayYPosition", HSys.Mot.MLoaderY_2, &Teach.Loader2CarFeedTrayYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader2CarDischargeTrayYPosition", HSys.Mot.MLoaderY_2, &Teach.Loader2CarDischargeTrayYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader2CarFirstCCDYPosition", HSys.Mot.MLoaderY_2, &Teach.Loader2CarFirstCCDYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader2CarFirstSortYPosition", HSys.Mot.MLoaderY_2, &Teach.Loader2CarFirstSortYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "LoaderCarFirstCCDXPosition", HSys.Mot.MTopCCDX, &Teach.LoaderCarFirstCCDXPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "LoaderCarLastCCDXPosition", HSys.Mot.MTopCCDX, &Teach.LoaderCarLastCCDXPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToLoader1XPosition", HSys.Mot.MSortingArmX, &Teach.SortArmToLoader1XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToLoader2XPosition", HSys.Mot.MSortingArmX, &Teach.SortArmToLoader2XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto1XPosition", HSys.Mot.MSortingArmX, &Teach.SortArmToAuto1XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto2XPosition", HSys.Mot.MSortingArmX, &Teach.SortArmToAuto2XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto3XPosition", HSys.Mot.MSortingArmX, &Teach.SortArmToAuto3XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto4XPosition", HSys.Mot.MSortingArmX, &Teach.SortArmToAuto4XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto5XPosition", HSys.Mot.MSortingArmX, &Teach.SortArmToAuto5XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto6XPosition", HSys.Mot.MSortingArmX, &Teach.SortArmToAuto6XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToBottomCCDFirstXPosition", HSys.Mot.MSortingArmX, &Teach.SortArmToBottomCCDFirstXPosition);

    AddTeachItem(grdAuto, "TeachAuto", "Auto1CarFeedTrayYPosition", HSys.Mot.MAutoY_1, &Teach.Auto1CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto1CarDischargeTrayYPosition", HSys.Mot.MAutoY_1, &Teach.Auto1CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto1CarFirstSortYPosition", HSys.Mot.MAutoY_1, &Teach.Auto1CarFirstSortYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto2CarFeedTrayYPosition", HSys.Mot.MAutoY_2, &Teach.Auto2CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto2CarDischargeTrayYPosition", HSys.Mot.MAutoY_2, &Teach.Auto2CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto2CarFirstSortYPosition", HSys.Mot.MAutoY_2, &Teach.Auto2CarFirstSortYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto3CarFeedTrayYPosition", HSys.Mot.MAutoY_3, &Teach.Auto3CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto3CarDischargeTrayYPosition", HSys.Mot.MAutoY_3, &Teach.Auto3CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto3CarFirstSortYPosition", HSys.Mot.MAutoY_3, &Teach.Auto3CarFirstSortYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto4CarFeedTrayYPosition", HSys.Mot.MAutoY_4, &Teach.Auto4CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto4CarDischargeTrayYPosition", HSys.Mot.MAutoY_4, &Teach.Auto4CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto4CarFirstSortYPosition", HSys.Mot.MAutoY_4, &Teach.Auto4CarFirstSortYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto5CarFeedTrayYPosition", HSys.Mot.MAutoY_5, &Teach.Auto5CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto5CarDischargeTrayYPosition", HSys.Mot.MAutoY_5, &Teach.Auto5CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto5CarFirstSortYPosition", HSys.Mot.MAutoY_5, &Teach.Auto5CarFirstSortYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto6CarFeedTrayYPosition", HSys.Mot.MAutoY_6, &Teach.Auto6CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto6CarDischargeTrayYPosition", HSys.Mot.MAutoY_6, &Teach.Auto6CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto6CarFirstSortYPosition", HSys.Mot.MAutoY_6, &Teach.Auto6CarFirstSortYPosition);

    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z1Position", HSys.Mot.MSuckZ_1, &Teach.SortArmToLoader_1_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z2Position", HSys.Mot.MSuckZ_2, &Teach.SortArmToLoader_1_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z3Position", HSys.Mot.MSuckZ_3, &Teach.SortArmToLoader_1_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z4Position", HSys.Mot.MSuckZ_4, &Teach.SortArmToLoader_1_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z1Position", HSys.Mot.MSuckZ_1, &Teach.SortArmToLoader_2_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z2Position", HSys.Mot.MSuckZ_2, &Teach.SortArmToLoader_2_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z3Position", HSys.Mot.MSuckZ_3, &Teach.SortArmToLoader_2_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z4Position", HSys.Mot.MSuckZ_4, &Teach.SortArmToLoader_2_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z1Position", HSys.Mot.MSuckZ_1, &Teach.SortArmToAuto_1_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z2Position", HSys.Mot.MSuckZ_2, &Teach.SortArmToAuto_1_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z3Position", HSys.Mot.MSuckZ_3, &Teach.SortArmToAuto_1_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z4Position", HSys.Mot.MSuckZ_4, &Teach.SortArmToAuto_1_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z1Position", HSys.Mot.MSuckZ_1, &Teach.SortArmToAuto_2_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z2Position", HSys.Mot.MSuckZ_2, &Teach.SortArmToAuto_2_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z3Position", HSys.Mot.MSuckZ_3, &Teach.SortArmToAuto_2_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z4Position", HSys.Mot.MSuckZ_4, &Teach.SortArmToAuto_2_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z1Position", HSys.Mot.MSuckZ_1, &Teach.SortArmToAuto_3_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z2Position", HSys.Mot.MSuckZ_2, &Teach.SortArmToAuto_3_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z3Position", HSys.Mot.MSuckZ_3, &Teach.SortArmToAuto_3_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z4Position", HSys.Mot.MSuckZ_4, &Teach.SortArmToAuto_3_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z1Position", HSys.Mot.MSuckZ_1, &Teach.SortArmToAuto_4_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z2Position", HSys.Mot.MSuckZ_2, &Teach.SortArmToAuto_4_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z3Position", HSys.Mot.MSuckZ_3, &Teach.SortArmToAuto_4_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z4Position", HSys.Mot.MSuckZ_4, &Teach.SortArmToAuto_4_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z1Position", HSys.Mot.MSuckZ_1, &Teach.SortArmToAuto_5_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z2Position", HSys.Mot.MSuckZ_2, &Teach.SortArmToAuto_5_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z3Position", HSys.Mot.MSuckZ_3, &Teach.SortArmToAuto_5_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z4Position", HSys.Mot.MSuckZ_4, &Teach.SortArmToAuto_5_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z1Position", HSys.Mot.MSuckZ_1, &Teach.SortArmToAuto_6_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z2Position", HSys.Mot.MSuckZ_2, &Teach.SortArmToAuto_6_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z3Position", HSys.Mot.MSuckZ_3, &Teach.SortArmToAuto_6_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z4Position", HSys.Mot.MSuckZ_4, &Teach.SortArmToAuto_6_Z4Position);

    AddTeachItem(grdOthers, "TeachLoader", "PitchArmXMinPositoin", HSys.Mot.MPitchX, &Teach.PitchArmXMinPositoin);
    AddTeachItem(grdOthers, "TeachLoader", "PitchArmXMaxPositoin", HSys.Mot.MPitchX, &Teach.PitchArmXMaxPositoin);
    AddTeachItem(grdOthers, "TeachEmptyAndTrayX", "BottomCCDYCapturePosition", HSys.Mot.MBottomCCDY, &Teach.BottomCCDYCapturePosition);

    bTeachReady=true;
    RefreshTeachGrids();
}
//---------------------------------------------------------------------------
AnsiString TfTeach::GetTeachFileName()
{
    return HSys.CurrentDir+AnsiString("\\system\\tech.ini");
}
//---------------------------------------------------------------------------
AnsiString TfTeach::GetWorkFileTeachName(AnsiString RootPath)
{
    AnsiString S;
    AnsiString WorkFile;
    TIniFile *Ini;

    S=RootPath+AnsiString("\\system\\lastset.ini");
    if(FileExists(S)==false)
        return "";

    Ini=new TIniFile(S);
    WorkFile=Ini->ReadString("LastSet", "cob_MainWorkFile", "");
    delete Ini;

    if(WorkFile==AnsiString(""))
        return "";
    return RootPath+AnsiString("\\data\\")+WorkFile+AnsiString(".tech");
}
//---------------------------------------------------------------------------
AnsiString TfTeach::FindTeachFileName()
{
    AnsiString S;
    AnsiString RootPath;
    AnsiString OldRootPath;

    RootPath=HSys.CurrentDir;
    S=GetTeachFileName();
    if(FileExists(S))
        return S;

    S=GetWorkFileTeachName(RootPath);
    if(S!=AnsiString("") && FileExists(S))
        return S;

    S=RootPath+AnsiString("\\data\\Ztex_001.tech");
    if(FileExists(S))
        return S;
    S=RootPath+AnsiString("\\data\\Ztex.tech");
    if(FileExists(S))
        return S;
    S=RootPath+AnsiString("\\data\\EAGLEP2RIGHT.tech");
    if(FileExists(S))
        return S;

    OldRootPath=RootPath+AnsiString("\\..\\HT160S -Original 20260323");
    S=GetWorkFileTeachName(OldRootPath);
    if(S!=AnsiString("") && FileExists(S))
        return S;

    S=OldRootPath+AnsiString("\\data\\Ztex_001.tech");
    if(FileExists(S))
        return S;
    S=OldRootPath+AnsiString("\\data\\Ztex.tech");
    if(FileExists(S))
        return S;
    S=OldRootPath+AnsiString("\\data\\EAGLEP2RIGHT.tech");
    if(FileExists(S))
        return S;

    return GetTeachFileName();
}
//---------------------------------------------------------------------------
AnsiString TfTeach::GetTeachKey(int Index)
{
    if(Index<0 || Index>=TECH_MAX_ITEM)
        return "";
    if(TechPara[Index].Caption==AnsiString("BottomCCDYCapturePosition"))
        return AnsiString("edt_")+TechPara[Index].Caption;
    return AnsiString("ed_")+TechPara[Index].Caption;
}
//---------------------------------------------------------------------------
int TfTeach::ParsePositionText(AnsiString Text)
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
AnsiString TfTeach::FormatPositionText(int Value)
{
    AnsiString S;

    S.sprintf("%.2f", (double)Value/100.0);
    return S;
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::OpenWorkFile()
{
    AnsiString S;
    AnsiString Key;
    AnsiString Value;
    TIniFile *Ini;

    if(bTeachReady==false)
        InitialTeachParameter();

    S=FindTeachFileName();
    if(FileExists(S))
    {
        Ini=new TIniFile(S);
        for(int i=0; i<TECH_MAX_ITEM; i++)
        {
            if(TechPara[i].iPara!=NULL)
            {
                Key=GetTeachKey(i);
                Value=Ini->ReadString(TechPara[i].GroupName, Key, "");
                if(Value==AnsiString(""))
                    Value=Ini->ReadString(TechPara[i].GroupName, TechPara[i].Caption, "");
                if(Value!=AnsiString(""))
                    *TechPara[i].iPara=ParsePositionText(Value);
            }
        }
        delete Ini;
        if(S==GetTeachFileName())
            SetMessage(AnsiString("Loaded ")+S);
        else
            SetMessage(AnsiString("Imported ")+S+AnsiString("; SAVE to new file"));
    }
    else
    {
        SetMessage(AnsiString("Teach file not found: ")+S);
    }
    RefreshTeachGrids();
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::SaveWorkFile(AnsiString S)
{
    TIniFile *Ini;
    AnsiString Key;

    ForceDirectories(ExtractFilePath(S));
    Ini=new TIniFile(S);
    for(int i=0; i<TECH_MAX_ITEM; i++)
    {
        if(TechPara[i].iPara!=NULL)
        {
            Key=GetTeachKey(i);
            Ini->WriteString(TechPara[i].GroupName, Key, FormatPositionText(*TechPara[i].iPara));
        }
    }
    delete Ini;
    SetMessage(AnsiString("Saved ")+S);
}
//---------------------------------------------------------------------------
int TfTeach::FindTeachItem(TStringGrid *Grid, int Row)
{
    for(int i=0; i<TECH_MAX_ITEM; i++)
    {
        if(TechPara[i].Grid==Grid && TechPara[i].Row==Row)
            return i;
    }
    return -1;
}
//---------------------------------------------------------------------------
void TfTeach::SelectTeachItem(int Index)
{
    TTabSheet *Tab;

    if(Index<0 || Index>=TECH_MAX_ITEM)
        return;
    if(bSelectingTeachItem)
        return;

    bSelectingTeachItem=true;
    SelectedTeachIndex=Index;
    try
    {
        if(TechPara[Index].Grid!=NULL)
        {
            Tab=dynamic_cast<TTabSheet *>(TechPara[Index].Grid->Parent);
            if(Tab!=NULL && PageTeach!=NULL)
                PageTeach->ActivePage=Tab;
            if(TechPara[Index].Row>=1 && TechPara[Index].Row<TechPara[Index].Grid->RowCount)
                TechPara[Index].Grid->Row=TechPara[Index].Row;
        }
        SetActiveMotor(TechPara[Index].MotorSelect);
        if(TechPara[Index].iPara!=NULL && edTarget!=NULL)
            edTarget->Text=FormatPositionText(*TechPara[Index].iPara);
        SetMessage(TechPara[Index].Caption);
    }
    __finally
    {
        bSelectingTeachItem=false;
    }
}
//---------------------------------------------------------------------------
void TfTeach::RefreshTeachGrids()
{
    for(int i=0; i<TECH_MAX_ITEM; i++)
        RefreshTeachRow(i);
}
//---------------------------------------------------------------------------
void TfTeach::RefreshTeachGrid(TStringGrid *Grid)
{
    for(int i=0; i<TECH_MAX_ITEM; i++)
    {
        if(TechPara[i].Grid==Grid)
            RefreshTeachRow(i);
    }
}
//---------------------------------------------------------------------------
void TfTeach::RefreshTeachRow(int Index)
{
    TStringGrid *Grid;
    TTrayMotor *Motor;
    int Row;

    if(Index<0 || Index>=TECH_MAX_ITEM)
        return;
    Grid=TechPara[Index].Grid;
    Row=TechPara[Index].Row;
    if(Grid==NULL || Row<1 || Row>=Grid->RowCount)
        return;

    Motor=GetMotor(TechPara[Index].MotorSelect);
    Grid->Cells[0][Row]=TechPara[Index].Caption;
    Grid->Cells[1][Row]=GetMotorCaption(TechPara[Index].MotorSelect);
    Grid->Cells[2][Row]=(TechPara[Index].iPara==NULL)?AnsiString(""):FormatPositionText(*TechPara[Index].iPara);
    Grid->Cells[3][Row]=(Motor==NULL)?AnsiString(""):FormatPositionText(Motor->Position);
    Grid->Cells[4][Row]=GetSoftLimitCaption(Motor);
}
//---------------------------------------------------------------------------
void TfTeach::FillMotorList()
{
    int OldIndex;

    if(lstMotors==NULL)
        return;

    OldIndex=ActiveMotorIndex;
    lstMotors->Items->BeginUpdate();
    lstMotors->Items->Clear();
    for(int i=0; i<HSys.iTotalMotor; i++)
    {
        if(HSys.MotPtr[i]!=NULL)
            lstMotors->Items->Add(HSys.MotPtr[i]->NumberAlias);
    }
    lstMotors->Items->EndUpdate();

    if(lstMotors->Items->Count>0)
    {
        if(OldIndex<0 || OldIndex>=lstMotors->Items->Count)
            OldIndex=0;
        lstMotors->ItemIndex=OldIndex;
        SetActiveMotor(OldIndex);
    }
}
//---------------------------------------------------------------------------
void TfTeach::SetActiveMotor(int Index)
{
    if(GetMotor(Index)==NULL)
        return;

    ActiveMotorIndex=Index;
    if(lstMotors!=NULL && Index>=0 && Index<lstMotors->Items->Count)
        lstMotors->ItemIndex=Index;
    SetupSpeedControl();
    UpdateMotorMonitor();
}
//---------------------------------------------------------------------------
void TfTeach::SetupSpeedControl()
{
    TTrayMotor *Motor=GetActiveMotor();
    int Low;
    int High;
    int Speed;

    if(scbTeachSpeed==NULL || edSpeed==NULL || Motor==NULL)
        return;
    Low=Motor->GetJogLowSpeed();
    High=Motor->GetJogHighSpeed();
    if(Low<1)
        Low=1;
    if(High<Low)
        High=Low;
    scbTeachSpeed->Min=Low;
    scbTeachSpeed->Max=High;
    Speed=GetEditInt(edSpeed, Low);
    if(Speed<Low)
        Speed=Low;
    if(Speed>High)
        Speed=High;
    // edSpeed->OnChange (edSpeedChange) keeps the bar in sync; just seed the text.
    edSpeed->Text=IntToStr(Speed);
    scbTeachSpeed->Position=Speed;
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::scbTeachSpeedScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    TTrayMotor *Motor=GetActiveMotor();
    (void)Sender;
    (void)ScrollCode;

    // User dragged the bar: push value to the edit (which re-applies SetSpeed via
    // edSpeedChange) and apply directly too so the speed tracks live.
    if(edSpeed!=NULL)
        edSpeed->Text=IntToStr(ScrollPos);
    if(Motor!=NULL)
        Motor->SetSpeed(ScrollPos);
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::edSpeedChange(TObject *Sender)
{
    TTrayMotor *Motor=GetActiveMotor();
    int Speed;
    (void)Sender;

    if(scbTeachSpeed==NULL || edSpeed==NULL)
        return;
    if(edSpeed->Text==AnsiString(""))
        return;
    Speed=atoi(edSpeed->Text.c_str());
    if(Speed<scbTeachSpeed->Min)
        Speed=scbTeachSpeed->Min;
    if(Speed>scbTeachSpeed->Max)
        Speed=scbTeachSpeed->Max;
    // Position= fires OnChange (not OnScroll), so this does not re-enter here.
    scbTeachSpeed->Position=Speed;
    if(Motor!=NULL)
        Motor->SetSpeed(Speed);
}
//---------------------------------------------------------------------------
TTrayMotor *TfTeach::GetMotor(int Index)
{
    if(Index<0 || Index>=HSys.iTotalMotor || HSys.MotPtr==NULL)
        return NULL;
    return HSys.MotPtr[Index];
}
//---------------------------------------------------------------------------
TTrayMotor *TfTeach::GetActiveMotor()
{
    return GetMotor(ActiveMotorIndex);
}
//---------------------------------------------------------------------------
int TfTeach::GetEditInt(TEdit *Edit, int DefaultValue)
{
    if(Edit==NULL || Edit->Text==AnsiString(""))
        return DefaultValue;
    return atoi(Edit->Text.c_str());
}
//---------------------------------------------------------------------------
AnsiString TfTeach::GetMotorCaption(int Index)
{
    TTrayMotor *Motor=GetMotor(Index);
    if(Motor==NULL)
        return "";
    return Motor->NumberAlias;
}
//---------------------------------------------------------------------------
AnsiString TfTeach::GetSoftLimitCaption(TTrayMotor *Motor)
{
    if(Motor==NULL)
        return "";
    return FormatPositionText(Motor->GetSoftLimitN())+AnsiString(" ~ ")+FormatPositionText(Motor->GetSoftLimitP());
}
//---------------------------------------------------------------------------
void TfTeach::UpdateMotorMonitor()
{
    TTrayMotor *Motor=GetActiveMotor();
    int NowPos;
    int EncoderPos;
    bool bSysEmg;
    bool bEnabled;

    if(Motor==NULL)
    {
        if(palMotorName!=NULL)
            palMotorName->Caption="";
        if(edNowPos!=NULL)
            edNowPos->Text="";
        if(edEncoder!=NULL)
            edEncoder->Text="";
        // No active motor: reset all LEDs to idle gray (in-use, untriggered).
        for(int i=0; i<iMotLedTotalCnt; i++)
            UpdateStatusLed(i, true, false);
        return;
    }

    Motor->ScanMotorStatus();
    NowPos=Motor->ReadPos();
    EncoderPos=Motor->ReadEncoderPos();

    palMotorName->Caption=Motor->NumberAlias;
    edNowPos->Text=FormatPositionText(NowPos);
    edEncoder->Text=FormatPositionText(EncoderPos);
    if(edSpeed->Text==AnsiString(""))
        edSpeed->Text=IntToStr(Motor->GetJogLowSpeed());

    //AI(general) 20260616 : the per-axis EMG led bit (!(bMotorStatus & 0x02))
    //never reacts on this machine because EMG is wired to a system DI read by
    //IsEMGPressed(), not to each motor card's EMG input pin. Reflect the real
    //system EMG on the panel led so it matches the move/lock behaviour.
    bSysEmg=(IsEMGPressed()>0);
    bEnabled=Motor->GetEnable();
    for(int i=0; i<iMotLedTotalCnt; i++)
    {
        if(i==iEmgLed)
            UpdateStatusLed(i, bEnabled, Motor->Led[i] || bSysEmg);
        else
            UpdateStatusLed(i, bEnabled, Motor->Led[i]);
    }

    if(SelectedTeachIndex>=0)
        RefreshTeachRow(SelectedTeachIndex);
}
//---------------------------------------------------------------------------
void TfTeach::UpdateStatusLed(int LedIndex, bool Enabled, bool Value)
{
    if(LedIndex<0 || LedIndex>=iMotLedTotalCnt || ledStatus[LedIndex]==NULL)
        return;

    if(Enabled==false)
    {
        // Motor not in use -> solid red regardless of the scanned bit.
        ledStatus[LedIndex]->TrueColor=TEACH_COLOR_LED_DISABLED;
        ledStatus[LedIndex]->FalseColor=TEACH_COLOR_LED_DISABLED;
        ledStatus[LedIndex]->Value=true;
        return;
    }
    // In use -> green when triggered, gray when idle.
    ledStatus[LedIndex]->TrueColor=TEACH_COLOR_LED_ON;
    ledStatus[LedIndex]->FalseColor=TEACH_COLOR_LED_OFF;
    ledStatus[LedIndex]->Value=Value;
}
//---------------------------------------------------------------------------
bool TfTeach::CheckSortArmZHome()
{
    if(HSys.Mot.MSuckZ_1!=NULL && HSys.Mot.MSuckZ_1->bHomeFlag==false)
        return false;
    if(HSys.Mot.MSuckZ_2!=NULL && HSys.Mot.MSuckZ_2->bHomeFlag==false)
        return false;
    if(HSys.Mot.MSuckZ_3!=NULL && HSys.Mot.MSuckZ_3->bHomeFlag==false)
        return false;
    if(HSys.Mot.MSuckZ_4!=NULL && HSys.Mot.MSuckZ_4->bHomeFlag==false)
        return false;
    return true;
}
//---------------------------------------------------------------------------
bool TfTeach::CheckCanTeachMove(TTrayMotor *Motor, bool bRequireHome, bool bUseTarget, int Target)
{
    int Emg;

    if(Motor==NULL)
    {
        SetMessage("Move abort: no motor");
        return false;
    }
    if(HSys.Sys.SystemStart)
    {
        MessageDlg("Machine is running.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: system start");
        return false;
    }

    Emg=IsEMGPressed();
    if(Emg>0)
    {
        MessageDlg("EMG is pressed.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: EMG");
        return false;
    }

    Motor->ScanMotorStatus();
    if(Motor->GetEnable()==false)
    {
        MessageDlg("Motor is disabled.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: motor disable");
        return false;
    }
    if(Motor->Led[iAlarmLed] || Motor->Led[iServoalarmLed] || Motor->Led[iEmgLed])
    {
        MessageDlg("Motor alarm is active.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: motor alarm");
        return false;
    }
    if(bRequireHome && Motor->bHomeFlag==false)
    {
        MessageDlg("Motor is not home.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: motor not home");
        return false;
    }
    if(HSys.Mot.MSortingArmX!=NULL && Motor->Tag==HSys.Mot.MSortingArmX->Tag && CheckSortArmZHome()==false)
    {
        MessageDlg("Sort arm Z must be home before X move.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: Sort Z not home");
        return false;
    }
    if(bUseTarget && Motor->CheckSoftLimit(Target)==false)
    {
        MessageDlg("Target over soft limit.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: soft limit");
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void TfTeach::StartJog(bool bPositive)
{
    TTrayMotor *Motor=GetActiveMotor();
    int Speed;
    if(Motor==NULL)
        return;
    // Jog is a manual move and must work BEFORE homing (e.g. to reach the home
    // sensor), so it does NOT require bHomeFlag. Alarm/EMG/disable/soft-limit
    // checks inside CheckCanTeachMove still apply. Move/Step keep bRequireHome=true.
    if(CheckCanTeachMove(Motor, false, false, 0)==false)
        return;
    Speed=GetEditInt(edSpeed, Motor->GetJogLowSpeed());
    Motor->SetSpeed(Speed);
    if(bPositive)
    {
        Motor->JogP();
        SetMessage("Jog +");
    }
    else
    {
        Motor->JogN();
        SetMessage("Jog -");
    }
}
//---------------------------------------------------------------------------
void TfTeach::StepMove(bool bPositive)
{
    TTrayMotor *Motor=GetActiveMotor();
    int NowPos;
    int Step;
    int Target;

    if(Motor==NULL)
        return;
    NowPos=Motor->ReadPos();
    Step=ParsePositionText(edStep->Text);
    if(Step==0)
        Step=100;
    Target=bPositive?(NowPos+Step):(NowPos-Step);
    edTarget->Text=FormatPositionText(Target);
    MoveActiveMotorToTarget();
}
//---------------------------------------------------------------------------
void TfTeach::MoveActiveMotorToTarget()
{
    TTrayMotor *Motor=GetActiveMotor();
    int Target;
    int Speed;

    if(Motor==NULL)
        return;
    Target=ParsePositionText(edTarget->Text);
    if(CheckCanTeachMove(Motor, true, true, Target)==false)
        return;
    Speed=GetEditInt(edSpeed, Motor->GetJogLowSpeed());
    Motor->SetSpeed(Speed);
    Motor->MotorMove(Target);
    SetMessage(AnsiString("Move to ")+FormatPositionText(Target));
}
//---------------------------------------------------------------------------
void TfTeach::MoveSelectedTeach()
{
    if(SelectedTeachIndex<0 || SelectedTeachIndex>=TECH_MAX_ITEM)
        return;
    if(TechPara[SelectedTeachIndex].iPara==NULL)
        return;
    SelectTeachItem(SelectedTeachIndex);
    edTarget->Text=FormatPositionText(*TechPara[SelectedTeachIndex].iPara);
    MoveActiveMotorToTarget();
}
//---------------------------------------------------------------------------
void TfTeach::SetSelectedTeachFromNow()
{
    TTrayMotor *Motor=GetActiveMotor();
    int NowPos;

    if(Motor==NULL || SelectedTeachIndex<0 || SelectedTeachIndex>=TECH_MAX_ITEM)
        return;
    if(TechPara[SelectedTeachIndex].iPara==NULL)
        return;

    NowPos=Motor->ReadPos();
    *TechPara[SelectedTeachIndex].iPara=NowPos;
    edTarget->Text=FormatPositionText(NowPos);
    RefreshTeachRow(SelectedTeachIndex);
    SetMessage(AnsiString("Set ")+TechPara[SelectedTeachIndex].Caption);
}
//---------------------------------------------------------------------------
void TfTeach::StopActiveMotor()
{
    TTrayMotor *Motor=GetActiveMotor();
    if(Motor!=NULL)
        Motor->Stop();
    bHomeRunning=false;
    iHomeMotorIndex=-1;
}
//---------------------------------------------------------------------------
void TfTeach::SetMessage(AnsiString Text)
{
    if(lblMessage!=NULL)
        lblMessage->Caption=Text;
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::tmrUpdateTimer(TObject *Sender)
{
    TTrayMotor *Motor;
    AnsiString Err;
    static bool bEmgActive=false;
    (void)Sender;

    //AI(general) 20260616 : EMG only blocked the START of a move before; a move,
    //jog or home already in progress kept running. Poll EMG here and stop the
    //active axis (StopActiveMotor also aborts the home task). bEmgActive latches
    //so we stop once per press and do not advance home/jog while EMG is held.
    if(IsEMGPressed()>0)
    {
        if(!bEmgActive)
        {
            bEmgActive=true;
            StopActiveMotor();
            SetMessage("Move abort: EMG");
        }
        UpdateMotorMonitor();
        return;
    }
    bEmgActive=false;

    if(bHomeRunning && iHomeMotorIndex>=0 && iHomeMotorIndex<HSys.iTotalMotor)
    {
        Motor=HSys.MotPtr[iHomeMotorIndex];
        if(Motor!=NULL && Motor->Home(Err))
        {
            bHomeRunning=false;
            iHomeMotorIndex=-1;
            SetMessage("Home finish");
        }
        else if(Err!=AnsiString(""))
        {
            SetMessage(Err);
        }
    }
    UpdateMotorMonitor();
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::grdTeachSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{
    int Index;
    (void)ACol;
    CanSelect=true;
    if(bSelectingTeachItem)
        return;
    Index=FindTeachItem((TStringGrid *)Sender, ARow);
    if(Index>=0)
        SelectTeachItem(Index);
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::grdTeachDblClick(TObject *Sender)
{
    TStringGrid *Grid=(TStringGrid *)Sender;
    int Index=FindTeachItem(Grid, Grid->Row);
    if(Index>=0)
    {
        SelectTeachItem(Index);
        MoveSelectedTeach();
    }
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::lstMotorsClick(TObject *Sender)
{
    (void)Sender;
    if(lstMotors!=NULL)
        SetActiveMotor(lstMotors->ItemIndex);
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnSetTeachClick(TObject *Sender)
{
    (void)Sender;
    SetSelectedTeachFromNow();
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnGoTeachClick(TObject *Sender)
{
    (void)Sender;
    MoveSelectedTeach();
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnSaveClick(TObject *Sender)
{
    int Ret;
    (void)Sender;
    Ret=MessageDlg("Sure to Save ?", mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, 0);
    if(Ret==mrYes)
        SaveWorkFile(GetTeachFileName());
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnReloadClick(TObject *Sender)
{
    (void)Sender;
    OpenWorkFile();
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnIOFormClick(TObject *Sender)
{
    (void)Sender;
    if(fiosetview==NULL)
        fiosetview=new Tfiosetview(this);
    fiosetview->Show();
    fiosetview->BringToFront();
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnJogPMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Sender;
    (void)Button;
    (void)Shift;
    (void)X;
    (void)Y;
    StartJog(true);
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnJogNMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Sender;
    (void)Button;
    (void)Shift;
    (void)X;
    (void)Y;
    StartJog(false);
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnJogMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Sender;
    (void)Button;
    (void)Shift;
    (void)X;
    (void)Y;
    StopActiveMotor();
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnStepPClick(TObject *Sender)
{
    (void)Sender;
    StepMove(true);
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnStepNClick(TObject *Sender)
{
    (void)Sender;
    StepMove(false);
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnMoveClick(TObject *Sender)
{
    (void)Sender;
    MoveActiveMotorToTarget();
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnHomeClick(TObject *Sender)
{
    TTrayMotor *Motor=GetActiveMotor();
    (void)Sender;
    if(Motor==NULL)
        return;
    if(CheckCanTeachMove(Motor, false, false, 0)==false)
        return;
    Motor->InitHomeTask_forSingleAxis();
    bHomeRunning=true;
    iHomeMotorIndex=ActiveMotorIndex;
    SetMessage("Home start");
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnStopClick(TObject *Sender)
{
    (void)Sender;
    StopActiveMotor();
    SetMessage("Stop");
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnRefreshClick(TObject *Sender)
{
    (void)Sender;
    FillMotorList();
    RefreshTeachGrids();
    UpdateMotorMonitor();
}
//---------------------------------------------------------------------------