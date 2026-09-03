//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#include <stdio.h>
#pragma hdrstop
#include "language.h"
#include "mymessbox.h"
#include "note.h"   //AI(ht160s-sortarm) 20260703 : fNote visibility for the fault-popup test-halt trigger

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
    bSaZRecovering=false;
    bSaAllZUpRunning=false;

    bCarTestRunning=false;
    iCarArea=0;
    iCarPhase=0;
    bCarLoop=false;
    iCarLoopTarget=1;
    iCarLoopDone=0;
    bAutoTestRunning=false;
    iAutoIndex=0;

    bTaTestRunning=false;
    iTaTask=0;
    iTaChannel=-1;
    bTaIsGrab=false;

    bCcdTestRunning=false;
    iCcdTask=0;
    iCcdLoaderNo=-1;
    iCcdCol=0;
    iCcdRow=0;
    bColorCcdTestRunning=false;

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
    StopTrayArmTest();
    StopCcdTest();
    StopColorCcdTest();
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
    PopulateTrayArmCombos();
    PopulateCcdCombos();

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
    Grid->Cells[1][0]=LangT("Motor");
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
    AddTeachItem(grdEmptyTray, "TeachEmptyAndTrayX", "ColorReceiveTrayYPosition", HSys.Mot.MColorY, &TeachBase.ColorReceiveTrayYPosition);
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
            StopTrayArmTest();
            StopCcdTest();
            StopColorCcdTest();
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
    //AI(ht160s-sortarm) 20260703 (Req2) : systematic mid-motion fault halt with TWO triggers, so
    //every current AND future teach motion stops the moment something goes wrong (a new test is
    //covered once its flag is in AnyAdvancedTestRunning() and its Stop in StopAllAdvancedTests()).
    //Flags are cleared (StopAllAdvancedTests) BEFORE any modal, so the timer - which keeps firing
    //under a ShowModal - cannot re-enter and re-drive motion / re-spam the popup.
    //(A) A fault popup (TfNote / TMyMessageBox) is on screen : by project convention every
    //    operator-facing fault surfaces through these two singleton modal forms, so their
    //    visibility is a high-coverage "something abnormal was raised" signal (covers limit /
    //    interlock errors that are NOT servo alarms). Stop SILENTLY - the fault popup already
    //    informs the operator; adding our own would double-popup.
    //(B) A real (off-limit) servo alarm with NO popup : in Teach (SystemStart==false)
    //    ScanAllMotorStatus raises no popup, so a mid-motion servo alarm is silent and the poll
    //    loop would hang. Stop and notify (this is the only path that raises a popup here).
    if(AnyAdvancedTestRunning())
    {
        AnsiString FaultWhy;
        if(IsFaultPopupShowing())
        {
            StopAllAdvancedTests();
            SetMessage(LangT("Test stopped : alarm / message shown"));
            UpdateMotorMonitor();
            return;
        }
        if(HasRealServoAlarm(FaultWhy))
        {
            StopAllAdvancedTests();
            SetMessage(FaultWhy);
            ShowMyOKMessageNoStop(FaultWhy);
            UpdateMotorMonitor();
            return;
        }
    }
    RunSaAllZUp();
    RunSortArmTest();
    RunCarTest();
    RunAutoTest();
    RunTrayArmTest();
    RunCcdTest();
    RunColorCcdTest();
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
    StopTrayArmTest();
    StopCcdTest();
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
    //AI(ht160s-sortarm) 20260703 (Req4) : suck-Z Home is NO LONGER a hard gate here. If a nozzle
    //is off its Home sensor the test starts in recovery (btnSaGoClick sets bSaZRecovering) and
    //RunSortArmTest lifts all suck-Z to the safe position, then re-verifies Home; only a genuine
    //failure (still not home after the lift) alarms + stops there. X/Y Home and all motor-alarm
    //checks above still hard-gate the start. CheckSortArmZHome() is retained for other callers.
    return true;
}
//---------------------------------------------------------------------------
void TfTeach::RunSortArmTest()
{
    if(bSaTestRunning==false || SortArmModule==NULL)
        return;
    //AI(ht160s-sortarm) 20260703 (Req4) : recovery phase. The suckers were not on Home at GO;
    //lift all suck-Z to the safe position, then re-verify the live Home sensor. Home now lit ->
    //resume the normal pick/place. Still not lit after reaching safe -> genuine fault : stop the
    //test (so the polling timer stops re-driving motion) and notify. SortArmZToSafePos() is
    //poll-to-done (re-called each tick, no command spam; true only when every suck-Z has arrived).
    if(bSaZRecovering)
    {
        if(SortArmModule->SortArmZToSafePos())
        {
            if(SortArmModule->AreAllSuckersHome())
            {
                bSaZRecovering=false;
                SetSaStatus(LangT("Suckers back home : continue"));
            }
            else
            {
                StopSortArmTest();
                SetSaStatus(LangT("Abort: Suck Z not home after safe move"));
                ShowMyOKMessageNoStop(LangT("Suck Z did not return Home after moving to the safe position."));
            }
        }
        return;
    }
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

    //AI(ht160s-sortarm) 20260703 : also stops the standalone All-Z-up lift and clears the
    //not-home recovery flag, so the central fault guard (and EMG) halts either path here.
    if(bSaTestRunning==false && bSaAllZUpRunning==false)
        return;
    bSaTestRunning=false;
    bSaAllZUpRunning=false;
    bSaZRecovering=false;
    iSaTask=0;
    //AI(ht160s-sortarm) 20260703 : abort any in-flight MoveSuckerToCell so it bails before the next
    //axis move. Guards the case where this stop fires from the timer fault-guard while a fault modal
    //raised INSIDE MoveSortArmX (case 30) is on screen : without this, dismissing the modal lets the
    //outer frame fall through to the Y move (one stray motion after the operator acknowledges).
    if(SortArmModule!=NULL)
        SortArmModule->AbortCurrentMove();
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
//AI(ht160s-sortarm) 20260703 (Req3) : "All Z Up" button. Lift every suck-Z to the safe
//position at the motor's normal working speed (Z up is always collision-free). One-shot arm :
//gate here, then RunSaAllZUp (driven from tmrUpdate) polls SortArmZToSafePos() to completion.
//Counted as an advanced motion (AnyAdvancedTestRunning), so the central servo-alarm guard
//stops it too. MoveTo is poll-to-done, so the poll re-issues safely without command spam.
void __fastcall TfTeach::btnSaAllZUpClick(TObject *Sender)
{
    (void)Sender;
    if(SortArmModule==NULL)
    {
        SetSaStatus(LangT("SortArm module not ready"));
        return;
    }
    if(bSaTestRunning || bSaAllZUpRunning)
    {
        SetSaStatus("Test already running");
        return;
    }
    if(HSys.Sys.SystemStart)
    {
        ShowMyOKMessageNoStop(LangT("Machine is running."));
        return;
    }
    if(IsEMGPressed()>0)
    {
        ShowMyOKMessageNoStop(LangT("EMG is pressed."));
        return;
    }
    bSaAllZUpRunning=true;
    SetSaStatus(LangT("All Z up : lifting to safe Z..."));
}
//---------------------------------------------------------------------------
void TfTeach::RunSaAllZUp()
{
    if(bSaAllZUpRunning==false || SortArmModule==NULL)
        return;
    if(SortArmModule->SortArmZToSafePos())
    {
        bSaAllZUpRunning=false;
        SetSaStatus(LangT("All Z at safe position"));
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-sortarm) 20260703 (Req2) : single OR-list of every Advanced-page motion flag.
//ADD A NEW TEST'S RUNNING FLAG HERE so the central mid-motion fault guard protects it too.
bool TfTeach::AnyAdvancedTestRunning()
{
    return bSaTestRunning || bSaAllZUpRunning || bCarTestRunning || bAutoTestRunning ||
           bTaTestRunning || bCcdTestRunning || bColorCcdTestRunning;
}
//---------------------------------------------------------------------------
//AI(ht160s-sortarm) 20260703 (Req2) : mirror csystem.cpp ScanAllMotorStatus - a real
//(off-limit) servo alarm on any ENABLED motor is the hard fault. A bare CW/CCW limit latches
//iAlarmLed but is recoverable, so it is skipped (avoids nuisance stops on a parked limit).
//SOFT_SIMULATE has no card and never alarms, so the scan is compiled out there (same as
//production ScanAllMotorStatus). Why carries the axis name for the operator popup.
bool TfTeach::HasRealServoAlarm(AnsiString &Why)
{
    Why="";
#ifdef SOFT_SIMULATE
    return false;
#else
    if(HSys.MotPtr==NULL)
        return false;
    for(int i=0; i<HSys.iTotalMotor; i++)
    {
        TTrayMotor *M=HSys.MotPtr[i];
        if(M==NULL || M->GetEnable()==false)
            continue;
        M->ScanMotorStatus();
        if(M->Led[iAlarmLed] && M->ReadServoAlarmOn() &&
           !(M->Led[iCwLed] || M->Led[iCcwLed]))
        {
            Why=AnsiString("Test stopped : servo alarm on ")+M->NumberAlias;
            return true;
        }
    }
    return false;
#endif
}
//---------------------------------------------------------------------------
//AI(ht160s-sortarm) 20260703 (Req2) : true while a fault popup is on screen. Every operator-facing
//fault routes through these two singleton modal forms (project convention : never VCL MessageDlg),
//so their visibility is a high-coverage "something abnormal was raised" signal for the central
//test-halt guard. NULL-safe : the globals are auto-created VCL forms but guard anyway.
bool TfTeach::IsFaultPopupShowing()
{
    return (fNote!=NULL && fNote->Visible) ||
           (MyMessageBox!=NULL && MyMessageBox->Visible);
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
    if(bSaTestRunning || bSaAllZUpRunning)
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
        //AI(ht160s-ccd-teach-test) 20260628 : foolproof violation - stop ALL Advanced tests, do
        //NOT execute, then prompt (operator-chosen behavior; row/col out of tray-form range).
        StopAllAdvancedTests();
        ShowMyMessage(LangT(Err));
        SetSaStatus(Err);
        return;
    }

    iSaSlot=Slot;
    iSaTarget=Target;
    iSaCol=Col;
    iSaRow=Row;
    bSaZDown=(chkSaZDown!=NULL && chkSaZDown->Checked);
    //AI(ht160s-sortarm) 20260703 (Req4) : if the suckers are NOT on their Home sensor, do NOT
    //abort at the gate. Start the test in RECOVERY : RunSortArmTest first lifts all suck-Z to the
    //safe position, then re-reads the Home sensor. Only if it STILL is not lit do we alarm + stop.
    bSaZRecovering=(SortArmModule->AreAllSuckersHome()==false);
    iSaTask=0;
    bSaTestRunning=true;
    SetSaStatus(bSaZRecovering ? LangT("Suckers not home : lifting to safe Z...") : LangT("Sort arm test start"));
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
    //AI(ht160s-agv-ruleB) 20260818 : RULE B - the Advanced car test drives the SAME front
    //riser/separator cylinders as the production GoUp/GoDown (TestGoUpTray / TestGoDownTray
    //call straight into DoFrontDestackDown / the module GoUp ladders), and had no AMR gate at
    //all. Refuse to START while that station is serving an AMR, and say why - an engineer
    //staring at a dead button needs the reason on screen. Area : 0=Empty, 1=Loader, 2=Color.
    if((Area==0 && EmptyModule!=NULL  && EmptyModule->IsAmrLocked())  ||
       (Area==1 && LoaderModule!=NULL && LoaderModule->IsAmrLocked()) ||
       (Area==2 && ColorModule!=NULL  && ColorModule->IsAmrLocked()))
    {
        ShowMyOKMessageNoStop(LangT("AMR handoff in progress at this station."));
        SetCarStatus(LangT("Abort: AMR handoff"));
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
    //AI(ht160s-agv-ruleB) 20260818 : RULE B - TestGoUpOnce calls DoFrontRiseOnce, the same
    //stroke the production discharge uses to stack into the output car. Refuse to START it
    //while this Auto is serving an AMR.
    if(AutoModule!=NULL && AutoModule->IsAmrLocked(Index))
    {
        ShowMyOKMessageNoStop(LangT("AMR handoff in progress at this station."));
        SetAutoStatus(LangT("Abort: AMR handoff"));
        return;
    }

    iAutoIndex=Index;
    if(AutoModule!=NULL)
        AutoModule->TestGoUpOnce(iAutoIndex, 0);
    bAutoTestRunning=true;
    SetAutoStatus(LangT("Auto GoUp once start"));
}
//---------------------------------------------------------------------------
// Advanced page (Tray Arm) : TrayArm transport-arm grab/place test. Both groups
// share one running flag (only one test at a time). Grab sources = Empty/Color/
// Loader; place targets = Auto1-6 + recycle Empty/Color. The motion is the pure
// dry-run TTrayArmModule::TestGrabFromChannel/TestPlaceToChannel (the SAME physical
// primitives as production DoPick/DoPlace, with NO tray-tracking mutation), stepped
// from tmrUpdate, same way bHomeRunning drives Home.
void TfTeach::PopulateTrayArmCombos()
{
    if(cbTaGrabChannel!=NULL && cbTaGrabChannel->Items->Count>0 && cbTaGrabChannel->ItemIndex<0)
        cbTaGrabChannel->ItemIndex=0;
    if(cbTaPlaceChannel!=NULL && cbTaPlaceChannel->Items->Count>0 && cbTaPlaceChannel->ItemIndex<0)
        cbTaPlaceChannel->ItemIndex=0;
}
//---------------------------------------------------------------------------
// cbTaPlaceChannel index 0..5=Auto1..6, 6=Empty, 7=Color -> eTrayArmChannel id.
// (Grab combo maps 1:1 : index 0/1/2 == TACH_EMPTY/COLOR/LOADER.) -1 = no selection.
int TfTeach::ComboIndexToPlaceChannel(int Index)
{
    if(Index>=0 && Index<=5)
        return TACH_AUTO1+Index;
    if(Index==6)
        return TACH_EMPTY;
    if(Index==7)
        return TACH_COLOR;
    return -1;
}
//---------------------------------------------------------------------------
bool TfTeach::CheckTrayArmTestReady()
{
    TTrayMotor *X=HSys.Mot.MTrayArmX;

    if(HSys.Sys.SystemStart)
    {
        ShowMyOKMessageNoStop(LangT("Machine is running."));
        SetTaStatus(LangT("Abort: system start"));
        return false;
    }
    if(IsEMGPressed()>0)
    {
        ShowMyOKMessageNoStop(LangT("EMG is pressed."));
        SetTaStatus(LangT("Abort: EMG"));
        return false;
    }
    if(X==NULL)
    {
        SetTaStatus(LangT("Abort: motor missing"));
        return false;
    }
    X->ScanMotorStatus();
    if(X->GetEnable()==false)
    {
        ShowMyOKMessageNoStop(LangT("TrayArm X must be enabled."));
        SetTaStatus(LangT("Abort: motor disabled"));
        return false;
    }
    if(X->Led[iAlarmLed] || X->Led[iServoalarmLed])
    {
        ShowMyOKMessageNoStop(LangT("Motor alarm is active."));
        SetTaStatus(LangT("Abort: motor alarm"));
        return false;
    }
    if(X->bHomeFlag==false)
    {
        ShowMyOKMessageNoStop(LangT("TrayArm X must be home."));
        SetTaStatus(LangT("Abort: not home"));
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void TfTeach::RunTrayArmTest()
{
    bool bDone;

    if(bTaTestRunning==false || TrayArmModule==NULL)
        return;
    if(bTaIsGrab)
        bDone=TrayArmModule->TestGrabFromChannel(iTaChannel, iTaTask);
    else
        bDone=TrayArmModule->TestPlaceToChannel(iTaChannel, iTaTask);
    if(bDone)
    {
        bTaTestRunning=false;
        SetTaStatus(bTaIsGrab?LangT("TrayArm grab test finish"):LangT("TrayArm place test finish"));
    }
}
//---------------------------------------------------------------------------
void TfTeach::StopTrayArmTest()
{
    if(bTaTestRunning==false)
        return;
    bTaTestRunning=false;
    iTaTask=0;
    if(HSys.Mot.MTrayArmX!=NULL)
        HSys.Mot.MTrayArmX->Stop();
}
//---------------------------------------------------------------------------
void TfTeach::SetTaStatus(AnsiString Text)
{
    if(lblTaStatus!=NULL)
        lblTaStatus->Caption=Text;
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnTaGrabClick(TObject *Sender)
{
    int Channel;
    AnsiString Err;
    (void)Sender;

    if(TrayArmModule==NULL)
    {
        SetTaStatus(LangT("TrayArm module not ready"));
        return;
    }
    if(cbTaGrabChannel==NULL)
        return;
    if(bTaTestRunning)
    {
        SetTaStatus(LangT("Test already running"));
        return;
    }
    // Grab combo index maps 1:1 to the channel id (0=Empty, 1=Color, 2=Loader).
    Channel=cbTaGrabChannel->ItemIndex;
    if(Channel<0)
    {
        SetTaStatus(LangT("Select a grab source"));
        return;
    }
    if(CheckTrayArmTestReady()==false)
        return;
    if(TrayArmModule->CanTestTrayArm(Channel, true, Err)==false)
    {
        ShowMyOKMessageNoStop(LangT(Err));
        SetTaStatus(Err);
        return;
    }

    iTaChannel=Channel;
    bTaIsGrab=true;
    iTaTask=1;
    bTaTestRunning=true;
    SetTaStatus(LangT("TrayArm grab test start"));
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnTaPlaceClick(TObject *Sender)
{
    int Channel;
    AnsiString Err;
    (void)Sender;

    if(TrayArmModule==NULL)
    {
        SetTaStatus(LangT("TrayArm module not ready"));
        return;
    }
    if(cbTaPlaceChannel==NULL)
        return;
    if(bTaTestRunning)
    {
        SetTaStatus(LangT("Test already running"));
        return;
    }
    Channel=ComboIndexToPlaceChannel(cbTaPlaceChannel->ItemIndex);
    if(Channel<0)
    {
        SetTaStatus(LangT("Select a place target"));
        return;
    }
    if(CheckTrayArmTestReady()==false)
        return;
    if(TrayArmModule->CanTestTrayArm(Channel, false, Err)==false)
    {
        ShowMyOKMessageNoStop(LangT(Err));
        SetTaStatus(Err);
        return;
    }

    iTaChannel=Channel;
    bTaIsGrab=false;
    iTaTask=1;
    bTaTestRunning=true;
    SetTaStatus(LangT("TrayArm place test start"));
}
//---------------------------------------------------------------------------
// Shared on-screen numpad for an INTEGER cell index 1..MaxValue, range-enforced. MaxValue is the
// live tray-form count (GetTrayXCount/GetTrayYCount). The numpad clamps into [1,MaxValue] and
// writes the value back into Edit->Text; returns true on OK (caller reads Edit->Text only then).
// Reused by the CCD test and the SortArm Col/Row edits (the user's numpad+limit retrofit).
bool TfTeach::EditCellWithNumpad(TEdit *Edit, int MaxValue, AnsiString Caption)
{
    if(Edit==NULL)
        return false;
    if(MaxValue<1)
        MaxValue=1;
    if(fQwertyKey==NULL)
        fQwertyKey=new TfQwertyKey(this);
    return fQwertyKey->ShowQwertyKey(Edit, N_INTEGER, 0, true, 1.0, (double)MaxValue, Caption);
}
//---------------------------------------------------------------------------
// Stop EVERY Advanced test (foolproof violation path : halt all motion, then do not execute).
void TfTeach::StopAllAdvancedTests()
{
    StopSortArmTest();
    StopCarTest();
    StopTrayArmTest();
    StopCcdTest();
    StopColorCcdTest();
    //AI(ht160s-sortarm) 20260703 : on any full stop (alarm/message halt, EMG, foolproof), initialize
    //EVERY Advanced-page test's task/step/progress to its start value - unconditionally, even for a
    //test that was not running (whose StopXxxTest early-returns). This guarantees nothing can resume
    //mid-sequence after a halt; the operator must re-start from the beginning. ADD A NEW TEST'S TASK
    //VAR HERE so it is initialized too. (ColorCcd has no step variable - simple move-to-position.)
    iSaTask=0;
    bSaZRecovering=false;
    iCarPhase=0;
    iCarLoopDone=0;
    iTaTask=0;
    iCcdTask=0;
}
//---------------------------------------------------------------------------
// Advanced page (CCD) : Top CCD move-to-cell test. Like SortArm, drive the CCD to a tray Cell
// (Column/Row) on LoaderR / LoaderL. Col/Row use the shared numpad (EditCellWithNumpad) limited by
// the tray-form counts; a foolproof check at GO stops all tests + prompts on any out-of-range value.
// Color is NOT a cell channel (identity trays have no IC cell grid), so only LoaderR/LoaderL are
// offered. Motion is the task-stepped LoaderModule->MoveCcdToCell, advanced each tick from tmrUpdate.
void TfTeach::PopulateCcdCombos()
{
    if(cbCcdChannel!=NULL && cbCcdChannel->Items->Count>0 && cbCcdChannel->ItemIndex<0)
        cbCcdChannel->ItemIndex=0;
}
//---------------------------------------------------------------------------
// cbCcdChannel index 0=LoaderR (LoaderNo 2, MLoaderY_2 right), 1=LoaderL (LoaderNo 1, MLoaderY_1
// left). -1 = no selection.
int TfTeach::ComboIndexToLoaderNo(int Index)
{
    if(Index==0)
        return 2;
    if(Index==1)
        return 1;
    return -1;
}
//---------------------------------------------------------------------------
bool TfTeach::CheckCcdTestReady(int LoaderNo)
{
    TTrayMotor *X=HSys.Mot.MTopCCDX;
    TTrayMotor *Y=(LoaderNo==2) ? HSys.Mot.MLoaderY_2 : HSys.Mot.MLoaderY_1;

    if(HSys.Sys.SystemStart)
    {
        ShowMyOKMessageNoStop(LangT("Machine is running."));
        SetCcdStatus(LangT("Abort: system start"));
        return false;
    }
    if(IsEMGPressed()>0)
    {
        ShowMyOKMessageNoStop(LangT("EMG is pressed."));
        SetCcdStatus(LangT("Abort: EMG"));
        return false;
    }
    if(X==NULL || Y==NULL)
    {
        SetCcdStatus(LangT("Abort: motor missing"));
        return false;
    }
    X->ScanMotorStatus();
    Y->ScanMotorStatus();
    if(X->GetEnable()==false || Y->GetEnable()==false)
    {
        ShowMyOKMessageNoStop(LangT("Top CCD X and Loader Y must be enabled."));
        SetCcdStatus(LangT("Abort: motor disabled"));
        return false;
    }
    if(X->Led[iAlarmLed] || X->Led[iServoalarmLed] ||
       Y->Led[iAlarmLed] || Y->Led[iServoalarmLed])
    {
        ShowMyOKMessageNoStop(LangT("Motor alarm is active."));
        SetCcdStatus(LangT("Abort: motor alarm"));
        return false;
    }
    if(X->bHomeFlag==false || Y->bHomeFlag==false)
    {
        ShowMyOKMessageNoStop(LangT("Top CCD X and Loader Y must be home."));
        SetCcdStatus(LangT("Abort: not home"));
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void TfTeach::RunCcdTest()
{
    if(bCcdTestRunning==false || LoaderModule==NULL)
        return;
    if(LoaderModule->MoveCcdToCell(iCcdLoaderNo, iCcdCol, iCcdRow, iCcdTask))
    {
        bCcdTestRunning=false;
        SetCcdStatus(LangT("CCD move to cell finish"));
    }
}
//---------------------------------------------------------------------------
void TfTeach::StopCcdTest()
{
    if(bCcdTestRunning==false)
        return;
    bCcdTestRunning=false;
    iCcdTask=0;
    if(HSys.Mot.MTopCCDX!=NULL)
        HSys.Mot.MTopCCDX->Stop();
    if(HSys.Mot.MLoaderY_1!=NULL)
        HSys.Mot.MLoaderY_1->Stop();
    if(HSys.Mot.MLoaderY_2!=NULL)
        HSys.Mot.MLoaderY_2->Stop();
}
//---------------------------------------------------------------------------
void TfTeach::SetCcdStatus(AnsiString Text)
{
    if(lblCcdStatus!=NULL)
        lblCcdStatus->Caption=Text;
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::edCcdColClick(TObject *Sender)
{
    (void)Sender;
    if(LoaderModule==NULL)
        return;
    EditCellWithNumpad(edCcdCol, LoaderModule->GetTrayXCount(), LangT("Column"));
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::edCcdRowClick(TObject *Sender)
{
    (void)Sender;
    if(LoaderModule==NULL)
        return;
    EditCellWithNumpad(edCcdRow, LoaderModule->GetTrayYCount(), LangT("Row"));
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::edSaColClick(TObject *Sender)
{
    (void)Sender;
    if(SortArmModule==NULL)
        return;
    EditCellWithNumpad(edSaCol, SortArmModule->GetTrayXCount(), LangT("Column"));
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::edSaRowClick(TObject *Sender)
{
    (void)Sender;
    if(SortArmModule==NULL)
        return;
    EditCellWithNumpad(edSaRow, SortArmModule->GetTrayYCount(), LangT("Row"));
}
//---------------------------------------------------------------------------
bool TfTeach::CheckColorCcdTestReady()
{
    //AI(ht160s-color-ccd-test) 20260628 : Color CCD photo-position test ready-gate. Mirrors
    //CheckCcdTestReady but checks the two Color axes : carriage MColorY + CCD reader
    //MTopCCDX_Color. Identity trays have no IC cell grid, so there is no Loader/Column/Row here.
    TTrayMotor *Y=HSys.Mot.MColorY;
    TTrayMotor *X=HSys.Mot.MTopCCDX_Color;

    if(HSys.Sys.SystemStart)
    {
        ShowMyOKMessageNoStop(LangT("Machine is running."));
        SetColorCcdStatus(LangT("Abort: system start"));
        return false;
    }
    if(IsEMGPressed()>0)
    {
        ShowMyOKMessageNoStop(LangT("EMG is pressed."));
        SetColorCcdStatus(LangT("Abort: EMG"));
        return false;
    }
    if(X==NULL || Y==NULL)
    {
        SetColorCcdStatus(LangT("Abort: motor missing"));
        return false;
    }
    X->ScanMotorStatus();
    Y->ScanMotorStatus();
    if(X->GetEnable()==false || Y->GetEnable()==false)
    {
        ShowMyOKMessageNoStop(LangT("Color CCD X and Color Y must be enabled."));
        SetColorCcdStatus(LangT("Abort: motor disabled"));
        return false;
    }
    if(X->Led[iAlarmLed] || X->Led[iServoalarmLed] ||
       Y->Led[iAlarmLed] || Y->Led[iServoalarmLed])
    {
        ShowMyOKMessageNoStop(LangT("Motor alarm is active."));
        SetColorCcdStatus(LangT("Abort: motor alarm"));
        return false;
    }
    if(X->bHomeFlag==false || Y->bHomeFlag==false)
    {
        ShowMyOKMessageNoStop(LangT("Color CCD X and Color Y must be home."));
        SetColorCcdStatus(LangT("Abort: not home"));
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void TfTeach::RunColorCcdTest()
{
    //AI(ht160s-color-ccd-test) 20260628 : driven each tick from tmrUpdate. MoveColorCcdToScan
    //commands MColorY (-> ColorRead2DYPosition) and MTopCCDX_Color (-> ColorRead2DXPosition)
    //TOGETHER and returns true only when BOTH are in position (the same XY-parallel motion used
    //by production DoFeedTray case 3000). Move-only test : it positions the axes, it does NOT
    //fire the CCD (mirrors the Loader move-to-cell test). Parks in place on finish.
    if(bColorCcdTestRunning==false || ColorModule==NULL)
        return;
    if(ColorModule->MoveColorCcdToScan())
    {
        bColorCcdTestRunning=false;
        SetColorCcdStatus(LangT("Color CCD move to photo position finish"));
    }
}
//---------------------------------------------------------------------------
void TfTeach::StopColorCcdTest()
{
    if(bColorCcdTestRunning==false)
        return;
    bColorCcdTestRunning=false;
    if(HSys.Mot.MColorY!=NULL)
        HSys.Mot.MColorY->Stop();
    if(HSys.Mot.MTopCCDX_Color!=NULL)
        HSys.Mot.MTopCCDX_Color->Stop();
}
//---------------------------------------------------------------------------
void TfTeach::SetColorCcdStatus(AnsiString Text)
{
    if(lblColorCcdStatus!=NULL)
        lblColorCcdStatus->Caption=Text;
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnColorCcdGoClick(TObject *Sender)
{
    //AI(ht160s-color-ccd-test) 20260628 : start the Color CCD photo-position test. No channel /
    //Column / Row (identity tray has no cell grid). Gate hardware-ready, then let tmrUpdate drive
    //RunColorCcdTest to position MColorY + MTopCCDX_Color together. Park in place on finish.
    (void)Sender;

    if(ColorModule==NULL)
    {
        SetColorCcdStatus(LangT("Color module not ready"));
        return;
    }
    if(bColorCcdTestRunning)
    {
        SetColorCcdStatus(LangT("Test already running"));
        return;
    }
    if(CheckColorCcdTestReady()==false)
        return;

    bColorCcdTestRunning=true;
    SetColorCcdStatus(LangT("Color CCD move to photo position start"));
}
//---------------------------------------------------------------------------
void __fastcall TfTeach::btnCcdGoClick(TObject *Sender)
{
    int LoaderNo;
    int Col;
    int Row;
    AnsiString Err;
    (void)Sender;

    if(LoaderModule==NULL)
    {
        SetCcdStatus(LangT("Loader module not ready"));
        return;
    }
    if(cbCcdChannel==NULL)
        return;
    if(bCcdTestRunning)
    {
        SetCcdStatus(LangT("Test already running"));
        return;
    }
    LoaderNo=ComboIndexToLoaderNo(cbCcdChannel->ItemIndex);
    if(LoaderNo<0)
    {
        SetCcdStatus(LangT("Select a CCD area"));
        return;
    }
    // UI Col/Row are 1-based; convert to 0-based tray index.
    Col=GetEditInt(edCcdCol, 0)-1;
    Row=GetEditInt(edCcdRow, 0)-1;

    if(CheckCcdTestReady(LoaderNo)==false)
        return;
    if(LoaderModule->CanMoveCcdToCell(LoaderNo, Col, Row, Err)==false)
    {
        //AI(ht160s-ccd-teach-test) 20260628 : foolproof violation - stop ALL Advanced tests, do
        //NOT execute, then prompt (operator-chosen behavior; row/col out of tray-form range).
        StopAllAdvancedTests();
        ShowMyMessage(LangT(Err));
        SetCcdStatus(Err);
        return;
    }

    iCcdLoaderNo=LoaderNo;
    iCcdCol=Col;
    iCcdRow=Row;
    iCcdTask=1;
    bCcdTestRunning=true;
    SetCcdStatus(LangT("CCD move to cell start"));
}
//---------------------------------------------------------------------------
