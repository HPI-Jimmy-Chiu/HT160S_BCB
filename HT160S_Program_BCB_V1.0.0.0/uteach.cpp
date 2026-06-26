//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#include <stdio.h>
#pragma hdrstop
#include "language.h"
#include "mymessbox.h"

#include <IniFiles.hpp>
#include <SysUtils.hpp>

#include "uteach.h"
#include "iosetview.h"
#include "csystem.h"
#include "cStepTrace.h"   //AI(general) 20260617 : MotorTaskLog home/limit diagnosis trace
#include "aSortArm.h"
#include "aTrayArm.h"   //AI(HT160S-Maintainer) 20260624 : TrayArmModule->IsZUpAtPosition() Z-up interlock for manual TrayArm X
#include "aEmpty.h"
#include "aColor.h"
#include "aLoader.h"
#include "aAuto1To6.h"
#include "uQwertyKey.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "ALed"
#pragma resource "*.dfm"
TfTeach *fTeach;
TEACH Teach;
TEACH TeachBase;   //AI 20260623 : Offset base; UpdateAllParameter folds Teach = TeachBase + Offset
void UpdateAllParameter();   //AI 20260623 : Offset fold trigger (cprod.cpp)
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

    bSaTestRunning=false;
    iSaTask=0;
    iSaSlot=-1;
    iSaTarget=-1;
    iSaCol=0;
    iSaRow=0;
    bSaZDown=true;

    bCarTestRunning=false;
    iCarArea=0;
    iCarPhase=0;
    bCarLoop=false;
    iCarLoopTarget=1;
    iCarLoopDone=0;
    bAutoTestRunning=false;
    iAutoIndex=0;

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
    MotorTaskLogSetActive(true);   // one-shot home/limit capture while this screen is open
    MotorTaskLog("Teach", "", "SCREEN_OPEN", "Teach shown");
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::FormClose(TObject *Sender, TCloseAction &Action)
{
    (void)Sender;
    (void)Action;
    if(tmrUpdate!=NULL)
        tmrUpdate->Enabled=false;
    StopActiveMotor();
    StopSortArmTest();
    StopCarTest();
    MotorTaskLog("Teach", "", "SCREEN_CLOSE", "Teach closed");
    MotorTaskLogSetActive(false);
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

    PopulateAdvancedCombos();
    PopulateChannelCombos();

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
    Grid->Cells[0][0]=LangT("Teach Position");
    Grid->Cells[1][0]="Motor";
    Grid->Cells[2][0]=LangT("Teach(mm)");
    Grid->Cells[3][0]=LangT("Now(mm)");
    Grid->Cells[4][0]=LangT("Soft Limit(mm)");
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

    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "EmptyCarFeedTrayYPosition", HSys.Mot.MEmptyY, &TeachBase.EmptyCarFeedTrayYPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "EmptyCarDischargeTrayYPosition", HSys.Mot.MEmptyY, &TeachBase.EmptyCarDischargeTrayYPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToEmptyXPosition", HSys.Mot.MTrayArmX, &TeachBase.TrayXArmToEmptyXPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToLoaderXPosition", HSys.Mot.MTrayArmX, &TeachBase.TrayXArmToLoaderXPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToColorXPosition", HSys.Mot.MTrayArmX, &TeachBase.TrayXArmToColorXPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "ColorRead2DXPosition", HSys.Mot.MTopCCDX_Color, &TeachBase.ColorRead2DXPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "ColorRead2DYPosition", HSys.Mot.MColorY, &TeachBase.ColorRead2DYPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "ColorTrayArmPickYPosition", HSys.Mot.MColorY, &TeachBase.ColorTrayArmPickYPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto1XPosition", HSys.Mot.MTrayArmX, &TeachBase.TrayXArmToAuto1XPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto2XPosition", HSys.Mot.MTrayArmX, &TeachBase.TrayXArmToAuto2XPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto3XPosition", HSys.Mot.MTrayArmX, &TeachBase.TrayXArmToAuto3XPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto4XPosition", HSys.Mot.MTrayArmX, &TeachBase.TrayXArmToAuto4XPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto5XPosition", HSys.Mot.MTrayArmX, &TeachBase.TrayXArmToAuto5XPosition);
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "TrayXArmToAuto6XPosition", HSys.Mot.MTrayArmX, &TeachBase.TrayXArmToAuto6XPosition);

    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader1CarFeedTrayYPosition", HSys.Mot.MLoaderY_1, &TeachBase.Loader1CarFeedTrayYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader1CarDischargeTrayYPosition", HSys.Mot.MLoaderY_1, &TeachBase.Loader1CarDischargeTrayYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader1CarFirstCCDYPosition", HSys.Mot.MLoaderY_1, &TeachBase.Loader1CarFirstCCDYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader1CarFirstSortYPosition", HSys.Mot.MLoaderY_1, &TeachBase.Loader1CarFirstSortYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader2CarFeedTrayYPosition", HSys.Mot.MLoaderY_2, &TeachBase.Loader2CarFeedTrayYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader2CarDischargeTrayYPosition", HSys.Mot.MLoaderY_2, &TeachBase.Loader2CarDischargeTrayYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader2CarFirstCCDYPosition", HSys.Mot.MLoaderY_2, &TeachBase.Loader2CarFirstCCDYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "Loader2CarFirstSortYPosition", HSys.Mot.MLoaderY_2, &TeachBase.Loader2CarFirstSortYPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "LoaderCarFirstCCDXPosition", HSys.Mot.MTopCCDX, &TeachBase.LoaderCarFirstCCDXPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToLoader1XPosition", HSys.Mot.MSortingArmX, &TeachBase.SortArmToLoader1XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToLoader2XPosition", HSys.Mot.MSortingArmX, &TeachBase.SortArmToLoader2XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto1XPosition", HSys.Mot.MSortingArmX, &TeachBase.SortArmToAuto1XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto2XPosition", HSys.Mot.MSortingArmX, &TeachBase.SortArmToAuto2XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto3XPosition", HSys.Mot.MSortingArmX, &TeachBase.SortArmToAuto3XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto4XPosition", HSys.Mot.MSortingArmX, &TeachBase.SortArmToAuto4XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto5XPosition", HSys.Mot.MSortingArmX, &TeachBase.SortArmToAuto5XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToAuto6XPosition", HSys.Mot.MSortingArmX, &TeachBase.SortArmToAuto6XPosition);
    AddTeachItem(grdLoaderSort, "TeachLoader", "SortArmToBottomCCDFirstXPosition", HSys.Mot.MSortingArmX, &TeachBase.SortArmToBottomCCDFirstXPosition);

    AddTeachItem(grdAuto, "TeachAuto", "Auto1CarFeedTrayYPosition", HSys.Mot.MAutoY_1, &TeachBase.Auto1CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto1CarDischargeTrayYPosition", HSys.Mot.MAutoY_1, &TeachBase.Auto1CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto1CarFirstSortYPosition", HSys.Mot.MAutoY_1, &TeachBase.Auto1CarFirstSortYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto2CarFeedTrayYPosition", HSys.Mot.MAutoY_2, &TeachBase.Auto2CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto2CarDischargeTrayYPosition", HSys.Mot.MAutoY_2, &TeachBase.Auto2CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto2CarFirstSortYPosition", HSys.Mot.MAutoY_2, &TeachBase.Auto2CarFirstSortYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto3CarFeedTrayYPosition", HSys.Mot.MAutoY_3, &TeachBase.Auto3CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto3CarDischargeTrayYPosition", HSys.Mot.MAutoY_3, &TeachBase.Auto3CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto3CarFirstSortYPosition", HSys.Mot.MAutoY_3, &TeachBase.Auto3CarFirstSortYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto4CarFeedTrayYPosition", HSys.Mot.MAutoY_4, &TeachBase.Auto4CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto4CarDischargeTrayYPosition", HSys.Mot.MAutoY_4, &TeachBase.Auto4CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto4CarFirstSortYPosition", HSys.Mot.MAutoY_4, &TeachBase.Auto4CarFirstSortYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto5CarFeedTrayYPosition", HSys.Mot.MAutoY_5, &TeachBase.Auto5CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto5CarDischargeTrayYPosition", HSys.Mot.MAutoY_5, &TeachBase.Auto5CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto5CarFirstSortYPosition", HSys.Mot.MAutoY_5, &TeachBase.Auto5CarFirstSortYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto6CarFeedTrayYPosition", HSys.Mot.MAutoY_6, &TeachBase.Auto6CarFeedTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto6CarDischargeTrayYPosition", HSys.Mot.MAutoY_6, &TeachBase.Auto6CarDischargeTrayYPosition);
    AddTeachItem(grdAuto, "TeachAuto", "Auto6CarFirstSortYPosition", HSys.Mot.MAutoY_6, &TeachBase.Auto6CarFirstSortYPosition);

    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToLoader_1_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToLoader_1_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToLoader_1_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToLoader_1_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToLoader_2_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToLoader_2_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToLoader_2_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToLoader_2_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_1_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_1_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_1_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_1_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_2_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_2_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_2_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_2_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_3_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_3_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_3_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_3_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_4_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_4_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_4_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_4_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_5_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_5_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_5_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_5_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_6_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_6_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_6_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_6_Z4Position);

    AddTeachItem(grdOthers, "TeachLoader", "PitchArmXMinPositoin", HSys.Mot.MPitchX, &TeachBase.PitchArmXMinPositoin);
    AddTeachItem(grdOthers, "TeachLoader", "PitchArmXMaxPositoin", HSys.Mot.MPitchX, &TeachBase.PitchArmXMaxPositoin);
    AddTeachItem(grdOthers, "TeachEmptyAndTrayX", "BottomCCDYCapturePosition", HSys.Mot.MBottomCCDY, &TeachBase.BottomCCDYCapturePosition);

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
    UpdateAllParameter();   //AI 20260623 : re-fold Teach = TeachBase + Offset after (re)load
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
    // AI(HT160S-Maintainer) 20260622 : single source of truth -- delegate to the canonical
    // TSortArmModule interlock (live Home sensor, not the sticky bHomeFlag used before) so
    // Teach / Motor Test / production all enforce the SAME rule.
    if(SortArmModule==NULL)
        return true;
    return SortArmModule->AreAllSuckersHome();
}
//---------------------------------------------------------------------------
bool TfTeach::CheckCanTeachMove(TTrayMotor *Motor, bool bRequireHome, bool bUseTarget, int Target, bool bAllowLimitAlarm)
{
    int Emg;

    if(Motor==NULL)
    {
        SetMessage(LangT("Move abort: no motor"));
        return false;
    }
    if(HSys.Sys.SystemStart)
    {
        ShowMyOKMessageNoStop(LangT("Machine is running."));
        SetMessage(LangT("Move abort: system start"));
        return false;
    }

    Emg=IsEMGPressed();
    if(Emg>0)
    {
        ShowMyOKMessageNoStop(LangT("EMG is pressed."));
        SetMessage("Move abort: EMG");
        return false;
    }

    Motor->ScanMotorStatus();
    if(Motor->GetEnable()==false)
    {
        ShowMyOKMessageNoStop(LangT("Motor is disabled."));
        SetMessage(LangT("Move abort: motor disable"));
        return false;
    }
    // A CW/CCW over-travel latches the generic ALARM led but is recoverable (home /
    // jog-away clears it). When bAllowLimitAlarm, permit start if the ONLY fault is a
    // limit; a real servo alarm or EMG still blocks. The SortArm-Z / soft-limit guards
    // below still apply, so the cylinder/nozzle interlock is not relaxed.
    bool bLimitOnlyAlarm = Motor->Led[iAlarmLed] &&
                           (Motor->Led[iCwLed] || Motor->Led[iCcwLed]) &&
                           !Motor->Led[iServoalarmLed] && !Motor->Led[iEmgLed];
    if(Motor->Led[iServoalarmLed] || Motor->Led[iEmgLed] ||
       (Motor->Led[iAlarmLed] && !(bAllowLimitAlarm && bLimitOnlyAlarm)))
    {
        ShowMyOKMessageNoStop(LangT("Motor alarm is active."));
        SetMessage(LangT("Move abort: motor alarm"));
        return false;
    }
    if(bRequireHome && Motor->bHomeFlag==false)
    {
        ShowMyOKMessageNoStop(LangT("Motor is not home."));
        SetMessage(LangT("Move abort: motor not home"));
        return false;
    }
    if(HSys.Mot.MSortingArmX!=NULL && Motor->Tag==HSys.Mot.MSortingArmX->Tag && CheckSortArmZHome()==false)
    {
        ShowMyOKMessageNoStop(LangT("Sort arm Z must be home before X move."));
        SetMessage(LangT("Move abort: Sort Z not home"));
        return false;
    }
    // AI(HT160S-Maintainer) 20260624 : TrayArm X move requires the Z lift confirmed UP -- same anti-
    // collision rule as production MoveTrayArmX. CheckCanTeachMove gates BOTH Teach Move/Step and Jog
    // (StartJog routes through here), so this one check covers every manual TrayArm X move. Active in
    // real-machine DUMMY (X motor physically moves); bypassed only under SOFT_SIMULATE inside IsZUpAtPosition.
    if(HSys.Mot.MTrayArmX!=NULL && Motor->Tag==HSys.Mot.MTrayArmX->Tag &&
       TrayArmModule!=NULL && TrayArmModule->IsZUpAtPosition()==false)
    {
        ShowMyOKMessageNoStop(LangT("Tray arm Z must be up before X move."));
        SetMessage(LangT("Move abort: Tray Z not up"));
        return false;
    }
    if(bUseTarget && Motor->CheckSoftLimit(Target)==false)
    {
        ShowMyOKMessageNoStop(LangT("Target over soft limit."));
        SetMessage(LangT("Move abort: soft limit"));
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
    // sensor), so it does NOT require bHomeFlag. A CW/CCW limit-only alarm must not
    // block jogging AWAY (bAllowLimitAlarm=true); servo alarm/EMG/disable/SortArm-Z/
    // soft-limit checks inside CheckCanTeachMove still apply. Move/Step keep
    // bRequireHome=true and bAllowLimitAlarm=false.
    if(CheckCanTeachMove(Motor, false, false, 0, true)==false)
        return;
    // Block jogging FURTHER into a lit limit; only the away direction is allowed. AI(general)
    // 20260618 : limit switches are wired by SPATIAL convention, uniform across axes
    // (Jog+ -end = CW limit/iCwLed, Jog- -end = CCW limit/iCcwLed), Direction-INDEPENDENT --
    // so a lit iCwLed blocks Jog+, a lit iCcwLed blocks Jog-. (A Direction-aware variant was
    // tried and reverted; it flipped this on Direction=1 axes. See uMotorTest::StartJog +
    // myMC88X1motor JogP/JogN for the MCD451 per-pulse-direction limit caveat.)
    Motor->ScanMotorStatus();
    if(bPositive && Motor->Led[iCwLed])
    {
        ShowMyOKMessageNoStop(LangT("CW (+) limit is triggered. Jog + is blocked; use Jog - to move away."));
        SetMessage(LangT("Jog+ abort: CW limit"));
        return;
    }
    if(!bPositive && Motor->Led[iCcwLed])
    {
        ShowMyOKMessageNoStop(LangT("CCW (-) limit is triggered. Jog - is blocked; use Jog + to move away."));
        SetMessage(LangT("Jog- abort: CCW limit"));
        return;
    }
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
            StopSortArmTest();
            StopCarTest();
            SetMessage("Move abort: EMG");
        }
        UpdateMotorMonitor();
        return;
    }
    if(bEmgActive)
    {
        //AI 20260622 : EMG just RELEASED. While the servo was relaxed the operator may
        //have hand-moved the axis; the encoder(practical) register tracked it but the
        //command register / NowPos stayed frozen, so Teach "Set" (ReadPos) recorded the
        //stale value. Snap NowPos->encoder on release (HT172 ServoOnResetPos) so Set
        //captures the real position. Active axis only; no-op for steppers/idle.
        TTrayMotor *EmgMotor=GetActiveMotor();
        if(EmgMotor!=NULL && EmgMotor->GetEnable())
            EmgMotor->ServoOnResetPos();
    }
    bEmgActive=false;

    if(bHomeRunning && iHomeMotorIndex>=0 && iHomeMotorIndex<HSys.iTotalMotor)
    {
        Motor=HSys.MotPtr[iHomeMotorIndex];
        if(Motor!=NULL && Motor->Home(Err))
        {
            //AI 20260622 : show home travel on completion in mm (2 decimals), consistent
            //with NowPos/Encoder (was an integer "steps=-(GearRatio*LastHomePos)" that
            //mixed units on screen). LastHomePos is the card real-position (1/100mm)
            //captured at home, rendered like every other position field.
            AnsiString HomeTravel=FormatPositionText(Motor->GetLastHomePos());
            bHomeRunning=false;
            iHomeMotorIndex=-1;
            SetMessage(AnsiString("Home finish, ")+HomeTravel);
        }
        else if(Err!=AnsiString(""))
        {
            SetMessage(Err);
        }
    }
    RunSortArmTest();
    RunCarTest();
    RunAutoTest();
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
    //AI 20260622 : double-click no longer RUNS to the teach position; it opens the
    //on-screen number pad (fQwertyKey) so the operator can EDIT the calibration value
    //(mm, 2dp). On OK the typed text is parsed back to 1/100mm and stored in iPara; the
    //explicit Go button (btnGoTeach) still performs the physical move.
    TStringGrid *Grid=(TStringGrid *)Sender;
    int Index=FindTeachItem(Grid, Grid->Row);
    if(Index<0 || TechPara[Index].iPara==NULL)
        return;
    SelectTeachItem(Index);
    edTarget->Text=FormatPositionText(*TechPara[Index].iPara);
    if(fQwertyKey==NULL)
        fQwertyKey=new TfQwertyKey(this);
    if(fQwertyKey->ShowQwertyKey(edTarget, N_DOUBLE, 2, false, 0, 0, TechPara[Index].Caption))
    {
        *TechPara[Index].iPara=ParsePositionText(edTarget->Text);
        RefreshTeachRow(Index);
        SetMessage(AnsiString("Edit ")+TechPara[Index].Caption+AnsiString(" = ")+FormatPositionText(*TechPara[Index].iPara));
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
    Ret=ShowMyMessageBox_YES_NO(LangT("Sure to Save ?"));
    if(Ret==TMyMessageBox::msgrtnYES)
        SaveWorkFile(GetTeachFileName());
    UpdateAllParameter();   //AI 20260623 : re-fold effective Teach after base save
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
    //AI 20260619 : Teach IO tool -> suppress iosetview restore prompt/force on close
    //(align HT172, whose Teach opens iosetview with no restore handling).
    fiosetview->bFromTeach=true;
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
    // bAllowLimitAlarm=true: HOME is the recovery path off a CW/CCW limit, so a
    // limit-only alarm must not block it (the home sequence clears the limit itself).
    if(CheckCanTeachMove(Motor, false, false, 0, true)==false)
        return;
    Motor->InitHomeTask_forSingleAxis();
    bHomeRunning=true;
    iHomeMotorIndex=ActiveMotorIndex;
    MotorTaskLog("Teach", Motor->Alias, "HOME_BTN", "operator pressed HOME");
    SetMessage(LangT("Home start"));
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnStopClick(TObject *Sender)
{
    (void)Sender;
    StopActiveMotor();
    StopSortArmTest();
    StopCarTest();
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
// Advanced page : SortArm single-nozzle point test. Mirrors the HT172 GoBtn
// pattern (read the UI selection, gate, then drive the motion) but the motion is
// the non-FSM, task-stepped TSortArmModule::MoveSuckerToCell sequence driven from
// tmrUpdate (same way bHomeRunning drives Home above).
void TfTeach::PopulateAdvancedCombos()
{
    if(cbSuckUse!=NULL && cbSuckUse->Items->Count>0 && cbSuckUse->ItemIndex<0)
        cbSuckUse->ItemIndex=0;
    if(cbToArea!=NULL && cbToArea->Items->Count>0 && cbToArea->ItemIndex<0)
        cbToArea->ItemIndex=0;
}
//---------------------------------------------------------------------------
// cbToArea index 0=Loader1,1=Loader2,2..7=Auto1..6 -> module Target code
// (1=Loader1,2=Loader2,11..16=Auto1..6). Returns -1 for no selection.
int TfTeach::ComboIndexToTarget(int Index)
{
    if(Index==0)
        return 1;
    if(Index==1)
        return 2;
    if(Index>=2 && Index<=7)
        return 11+(Index-2);
    return -1;
}
//---------------------------------------------------------------------------
TTrayMotor *TfTeach::GetSaTargetYMotor(int Target)
{
    switch(Target)
    {
        case 1:  return HSys.Mot.MLoaderY_1;
        case 2:  return HSys.Mot.MLoaderY_2;
        case 11: return HSys.Mot.MAutoY_1;
        case 12: return HSys.Mot.MAutoY_2;
        case 13: return HSys.Mot.MAutoY_3;
        case 14: return HSys.Mot.MAutoY_4;
        case 15: return HSys.Mot.MAutoY_5;
        case 16: return HSys.Mot.MAutoY_6;
    }
    return NULL;
}
//---------------------------------------------------------------------------
bool TfTeach::CheckSortArmTestReady(int SlotIndex, int Target)
{
    TTrayMotor *X=HSys.Mot.MSortingArmX;
    TTrayMotor *Y=GetSaTargetYMotor(Target);
    TTrayMotor *Z=NULL;

    if(HSys.Sys.SystemStart)
    {
        ShowMyOKMessageNoStop(LangT("Machine is running."));
        SetSaStatus(LangT("Abort: system start"));
        return false;
    }
    if(IsEMGPressed()>0)
    {
        ShowMyOKMessageNoStop(LangT("EMG is pressed."));
        SetSaStatus(LangT("Abort: EMG"));
        return false;
    }

    switch(SlotIndex)
    {
        case 0: Z=HSys.Mot.MSuckZ_1; break;
        case 1: Z=HSys.Mot.MSuckZ_2; break;
        case 2: Z=HSys.Mot.MSuckZ_3; break;
        case 3: Z=HSys.Mot.MSuckZ_4; break;
    }
    if(X==NULL || Y==NULL || Z==NULL)
    {
        SetSaStatus(LangT("Abort: motor missing"));
        return false;
    }

    X->ScanMotorStatus();
    Y->ScanMotorStatus();
    Z->ScanMotorStatus();
    if(X->GetEnable()==false || Y->GetEnable()==false || Z->GetEnable()==false)
    {
        ShowMyOKMessageNoStop(LangT("SortArm X / target Y / Suck Z must be enabled."));
        SetSaStatus(LangT("Abort: motor disabled"));
        return false;
    }
    if(X->Led[iAlarmLed] || X->Led[iServoalarmLed] ||
       Y->Led[iAlarmLed] || Y->Led[iServoalarmLed] ||
       Z->Led[iAlarmLed] || Z->Led[iServoalarmLed])
    {
        ShowMyOKMessageNoStop(LangT("Motor alarm is active."));
        SetSaStatus(LangT("Abort: motor alarm"));
        return false;
    }
    if(X->bHomeFlag==false || Y->bHomeFlag==false)
    {
        ShowMyOKMessageNoStop(LangT("SortArm X and target Y must be home."));
        SetSaStatus(LangT("Abort: not home"));
        return false;
    }
    if(CheckSortArmZHome()==false)
    {
        ShowMyOKMessageNoStop(LangT("All Suck Z must be home before X move."));
        SetSaStatus(LangT("Abort: Suck Z not home"));
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void TfTeach::RunSortArmTest()
{
    if(bSaTestRunning==false || SortArmModule==NULL)
        return;
    if(SortArmModule->MoveSuckerToCell(iSaSlot, iSaTarget, iSaCol, iSaRow, bSaZDown, iSaTask))
    {
        bSaTestRunning=false;
        SetSaStatus(LangT("Sort arm test finish"));
    }
}
//---------------------------------------------------------------------------
void TfTeach::StopSortArmTest()
{
    TTrayMotor *Y;

    if(bSaTestRunning==false)
        return;
    bSaTestRunning=false;
    iSaTask=0;
    if(HSys.Mot.MSortingArmX!=NULL)
        HSys.Mot.MSortingArmX->Stop();
    if(HSys.Mot.MSuckZ_1!=NULL)
        HSys.Mot.MSuckZ_1->Stop();
    if(HSys.Mot.MSuckZ_2!=NULL)
        HSys.Mot.MSuckZ_2->Stop();
    if(HSys.Mot.MSuckZ_3!=NULL)
        HSys.Mot.MSuckZ_3->Stop();
    if(HSys.Mot.MSuckZ_4!=NULL)
        HSys.Mot.MSuckZ_4->Stop();
    Y=GetSaTargetYMotor(iSaTarget);
    if(Y!=NULL)
        Y->Stop();
}
//---------------------------------------------------------------------------
void TfTeach::SetSaStatus(AnsiString Text)
{
    if(lblSaStatus!=NULL)
        lblSaStatus->Caption=Text;
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnSaGoClick(TObject *Sender)
{
    int Slot;
    int Target;
    int Col;
    int Row;
    AnsiString Err;
    (void)Sender;

    if(SortArmModule==NULL)
    {
        SetSaStatus(LangT("SortArm module not ready"));
        return;
    }
    if(cbSuckUse==NULL || cbToArea==NULL)
        return;
    if(bSaTestRunning)
    {
        SetSaStatus("Test already running");
        return;
    }

    Slot=cbSuckUse->ItemIndex;
    if(Slot<0)
    {
        SetSaStatus(LangT("Select a sucker"));
        return;
    }
    Target=ComboIndexToTarget(cbToArea->ItemIndex);
    if(Target<0)
    {
        SetSaStatus(LangT("Select a target area"));
        return;
    }
    // UI Col/Row are 1-based; convert to 0-based tray index.
    Col=GetEditInt(edSaCol, 0)-1;
    Row=GetEditInt(edSaRow, 0)-1;

    if(CheckSortArmTestReady(Slot, Target)==false)
        return;
    if(SortArmModule->CanMoveSuckerToCell(Slot, Target, Col, Row, Err)==false)
    {
        ShowMyOKMessageNoStop(LangT(Err));
        SetSaStatus(Err);
        return;
    }

    iSaSlot=Slot;
    iSaTarget=Target;
    iSaCol=Col;
    iSaRow=Row;
    bSaZDown=(chkSaZDown!=NULL && chkSaZDown->Checked);
    iSaTask=0;
    bSaTestRunning=true;
    SetSaStatus(LangT("Sort arm test start"));
}
//---------------------------------------------------------------------------
// Advanced page (Channel) : stacking-car destacker tests. Two groups.
//  - gbCarGoUpGoDonw : Empty / Loader / Color dual-cylinder destacker. One round =
//    GoUp then GoDown; optional Loop N. Drives the module cylinder-only Test wrappers
//    from tmrUpdate, same way bHomeRunning drives Home.
//  - gbAutoGoUp : Auto1-6 single-cylinder FrontRise, GoUp once (no loop).
void TfTeach::PopulateChannelCombos()
{
    if(cbCarArea!=NULL && cbCarArea->Items->Count>0 && cbCarArea->ItemIndex<0)
        cbCarArea->ItemIndex=0;
    if(cbAutoArea!=NULL && cbAutoArea->Items->Count>0 && cbAutoArea->ItemIndex<0)
        cbAutoArea->ItemIndex=0;
}
//---------------------------------------------------------------------------
bool TfTeach::CheckCarTestReady()
{
    if(HSys.Sys.SystemStart)
    {
        ShowMyOKMessageNoStop(LangT("Machine is running."));
        return false;
    }
    if(IsEMGPressed()>0)
    {
        ShowMyOKMessageNoStop(LangT("EMG is pressed."));
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
// cbCarArea index : 0=Empty, 1=Loader, 2=Color. Returns true when the phase is done.
bool TfTeach::CallCarGoUp(int Area, int Flag)
{
    switch(Area)
    {
        case 0: return (EmptyModule==NULL) ? true : EmptyModule->TestGoUpTray(Flag);
        case 1: return (LoaderModule==NULL) ? true : LoaderModule->TestGoUpTray(Flag);
        case 2: return (ColorModule==NULL) ? true : ColorModule->TestGoUpTray(Flag);
    }
    return true;
}
//---------------------------------------------------------------------------
bool TfTeach::CallCarGoDown(int Area, int Flag)
{
    switch(Area)
    {
        case 0: return (EmptyModule==NULL) ? true : EmptyModule->TestGoDownTray(Flag);
        case 1: return (LoaderModule==NULL) ? true : LoaderModule->TestGoDownTray(Flag);
        case 2: return (ColorModule==NULL) ? true : ColorModule->TestGoDownTray(Flag);
    }
    return true;
}
//---------------------------------------------------------------------------
void TfTeach::StartCarPhase(int Phase)
{
    iCarPhase=Phase;
    MotorTaskLog("Teach", "Car", (Phase==0)?"CAR_PHASE_GOUP":"CAR_PHASE_GODOWN",
        AnsiString("area=")+IntToStr(iCarArea)+AnsiString(" round=")+IntToStr(iCarLoopDone)+AnsiString("/")+IntToStr(iCarLoopTarget));
    if(Phase==0)
        CallCarGoUp(iCarArea, 0);
    else
        CallCarGoDown(iCarArea, 0);
}
//---------------------------------------------------------------------------
void TfTeach::RunCarTest()
{
    if(bCarTestRunning==false)
        return;

    if(iCarPhase==0)
    {
        if(CallCarGoUp(iCarArea, 1))
        {
            MotorTaskLog("Teach", "Car", "CAR_GOUP_DONE", AnsiString("round=")+IntToStr(iCarLoopDone)+AnsiString("/")+IntToStr(iCarLoopTarget));
            SetCarStatus("GoUp done, GoDown...");
            StartCarPhase(1);
        }
    }
    else
    {
        if(CallCarGoDown(iCarArea, 1))
        {
            iCarLoopDone++;
            MotorTaskLog("Teach", "Car", "CAR_GODOWN_DONE", AnsiString("round=")+IntToStr(iCarLoopDone)+AnsiString("/")+IntToStr(iCarLoopTarget)+AnsiString(" loop=")+(bCarLoop?"1":"0"));
            if(bCarLoop && iCarLoopDone<iCarLoopTarget)
            {
                SetCarStatus("Round "+IntToStr(iCarLoopDone)+"/"+IntToStr(iCarLoopTarget)+" done, next...");
                StartCarPhase(0);
            }
            else
            {
                bCarTestRunning=false;
                MotorTaskLog("Teach", "Car", "CAR_FINISH", AnsiString("rounds=")+IntToStr(iCarLoopDone));
                SetCarStatus("Car test finish ("+IntToStr(iCarLoopDone)+" round)");
            }
        }
    }
}
//---------------------------------------------------------------------------
void TfTeach::StopCarTest()
{
    bCarTestRunning=false;
    bAutoTestRunning=false;
    iCarPhase=0;
}
//---------------------------------------------------------------------------
void TfTeach::RunAutoTest()
{
    if(bAutoTestRunning==false)
        return;
    if(AutoModule==NULL || AutoModule->TestGoUpOnce(iAutoIndex, 1))
    {
        bAutoTestRunning=false;
        SetAutoStatus(LangT("Auto GoUp once finish"));
    }
}
//---------------------------------------------------------------------------
void TfTeach::SetCarStatus(AnsiString Text)
{
    if(lblCarStatus!=NULL)
        lblCarStatus->Caption=Text;
}
//---------------------------------------------------------------------------
void TfTeach::SetAutoStatus(AnsiString Text)
{
    if(lblAutoStatus!=NULL)
        lblAutoStatus->Caption=Text;
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnCarGoClick(TObject *Sender)
{
    int Area;
    int Loop;
    (void)Sender;

    if(cbCarArea==NULL)
        return;
    if(bCarTestRunning)
    {
        SetCarStatus(LangT("Test already running"));
        return;
    }
    Area=cbCarArea->ItemIndex;
    if(Area<0)
    {
        SetCarStatus(LangT("Select a car area"));
        return;
    }
    if(CheckCarTestReady()==false)
    {
        SetCarStatus(LangT("Abort: not ready"));
        return;
    }

    bCarLoop=(chkCarLoop!=NULL && chkCarLoop->Checked);
    Loop=GetEditInt(edLoopTimes, 1);
    if(Loop<1)
        Loop=1;
    iCarLoopTarget=bCarLoop?Loop:1;
    iCarLoopDone=0;
    iCarArea=Area;
    bCarTestRunning=true;
    MotorTaskLog("Teach", "Car", "CAR_GO_BTN",
        AnsiString("area=")+IntToStr(iCarArea)+AnsiString(" loopChk=")+(bCarLoop?"1":"0")+AnsiString(" loopTimes=")+IntToStr(Loop)+AnsiString(" target=")+IntToStr(iCarLoopTarget));
    StartCarPhase(0);
    SetCarStatus("Car test start (GoUp...)");
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnAutoGoUpClick(TObject *Sender)
{
    int Index;
    (void)Sender;

    if(cbAutoArea==NULL)
        return;
    if(bAutoTestRunning)
    {
        SetAutoStatus(LangT("Test already running"));
        return;
    }
    Index=cbAutoArea->ItemIndex;
    if(Index<0)
    {
        SetAutoStatus(LangT("Select an Auto"));
        return;
    }
    if(CheckCarTestReady()==false)
    {
        SetAutoStatus(LangT("Abort: not ready"));
        return;
    }

    iAutoIndex=Index;
    if(AutoModule!=NULL)
        AutoModule->TestGoUpOnce(iAutoIndex, 0);
    bAutoTestRunning=true;
    SetAutoStatus(LangT("Auto GoUp once start"));
}
//---------------------------------------------------------------------------
