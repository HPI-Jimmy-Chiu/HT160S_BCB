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
static const TColor TEACH_COLOR_BG=(TColor)12761254;
static const TColor TEACH_COLOR_TITLE=(TColor)10263630;
static const TColor TEACH_COLOR_DARK=(TColor)8421440;
static const TColor TEACH_COLOR_GRID=(TColor)14670284;
static const TColor TEACH_COLOR_LED_OFF=(TColor)12632256;
static const TColor TEACH_COLOR_LED_ON=(TColor)65280;
static const TColor TEACH_COLOR_LED_ALARM=(TColor)255;

static const char *TEACH_LED_NAME[iMotLedTotalCnt]={
    "CW", "HOME", "CCW", "EMG", "ALARM", "Soft CW", "Soft CCW",
    "Servo Alarm", "In Pos", "Z Phase", "Servo On"
};
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

    palClient=NULL;
    palTitle=NULL;
    palFunction=NULL;
    palMotorControl=NULL;
    palMotorName=NULL;
    PageTeach=NULL;
    tsEmptyTray=NULL;
    tsLoaderSort=NULL;
    tsAuto=NULL;
    tsSortZ=NULL;
    tsOthers=NULL;
    grdEmptyTray=NULL;
    grdLoaderSort=NULL;
    grdAuto=NULL;
    grdSortZ=NULL;
    grdOthers=NULL;
    lblActiveMot=NULL;
    lblSpeed=NULL;
    lblStep=NULL;
    lblTarget=NULL;
    lblNowPos=NULL;
    lblEncoder=NULL;
    lblMotorList=NULL;
    lblMessage=NULL;
    edSpeed=NULL;
    edStep=NULL;
    edTarget=NULL;
    edNowPos=NULL;
    edEncoder=NULL;
    lstMotors=NULL;
    btnSetTeach=NULL;
    btnGoTeach=NULL;
    btnSave=NULL;
    btnReload=NULL;
    btnIOForm=NULL;
    btnClose=NULL;
    btnJogP=NULL;
    btnJogN=NULL;
    btnStepP=NULL;
    btnStepN=NULL;
    btnMove=NULL;
    btnHome=NULL;
    btnStop=NULL;
    btnRefresh=NULL;
    tmrUpdate=NULL;

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

    BindDfmComponents();

    Width=1200;
    Height=880;
    Caption="Teach";
    Color=TEACH_COLOR_BG;
    Position=poDefault;
    BorderStyle=bsSingle;
    BorderIcons=TBorderIcons();
    Font->Name="MS Sans Serif";
    Font->Height=-11;

    BuildMotorPanel();
    BuildClientPanel();

    if(tmrUpdate==NULL)
        tmrUpdate=new TTimer(this);
    tmrUpdate->Enabled=false;
    tmrUpdate->Interval=200;
    tmrUpdate->OnTimer=tmrUpdateTimer;

    bUIBuilt=true;
}
//---------------------------------------------------------------------------
void TfTeach::BindDfmComponents()
{
    int i;
    AnsiString Name;

    palClient=dynamic_cast<TPanel *>(FindComponent("palClient"));
    palTitle=dynamic_cast<TPanel *>(FindComponent("palTitle"));
    palFunction=dynamic_cast<TPanel *>(FindComponent("palFunction"));
    palMotorControl=dynamic_cast<TPanel *>(FindComponent("palMotorControl"));
    palMotorName=dynamic_cast<TPanel *>(FindComponent("palMotorName"));
    PageTeach=dynamic_cast<TPageControl *>(FindComponent("PageTeach"));
    tsEmptyTray=dynamic_cast<TTabSheet *>(FindComponent("tsEmptyTray"));
    tsLoaderSort=dynamic_cast<TTabSheet *>(FindComponent("tsLoaderSort"));
    tsAuto=dynamic_cast<TTabSheet *>(FindComponent("tsAuto"));
    tsSortZ=dynamic_cast<TTabSheet *>(FindComponent("tsSortZ"));
    tsOthers=dynamic_cast<TTabSheet *>(FindComponent("tsOthers"));
    grdEmptyTray=dynamic_cast<TStringGrid *>(FindComponent("grdEmptyTray"));
    grdLoaderSort=dynamic_cast<TStringGrid *>(FindComponent("grdLoaderSort"));
    grdAuto=dynamic_cast<TStringGrid *>(FindComponent("grdAuto"));
    grdSortZ=dynamic_cast<TStringGrid *>(FindComponent("grdSortZ"));
    grdOthers=dynamic_cast<TStringGrid *>(FindComponent("grdOthers"));
    lblActiveMot=dynamic_cast<TLabel *>(FindComponent("lblActiveMot"));
    lblSpeed=dynamic_cast<TLabel *>(FindComponent("lblSpeed"));
    lblStep=dynamic_cast<TLabel *>(FindComponent("lblStep"));
    lblTarget=dynamic_cast<TLabel *>(FindComponent("lblTarget"));
    lblNowPos=dynamic_cast<TLabel *>(FindComponent("lblNowPos"));
    lblEncoder=dynamic_cast<TLabel *>(FindComponent("lblEncoder"));
    lblMotorList=dynamic_cast<TLabel *>(FindComponent("lblMotorList"));
    lblMessage=dynamic_cast<TLabel *>(FindComponent("lblMessage"));
    edSpeed=dynamic_cast<TEdit *>(FindComponent("edSpeed"));
    edStep=dynamic_cast<TEdit *>(FindComponent("edStep"));
    edTarget=dynamic_cast<TEdit *>(FindComponent("edTarget"));
    edNowPos=dynamic_cast<TEdit *>(FindComponent("edNowPos"));
    edEncoder=dynamic_cast<TEdit *>(FindComponent("edEncoder"));
    lstMotors=dynamic_cast<TListBox *>(FindComponent("lstMotors"));
    btnSetTeach=dynamic_cast<TButton *>(FindComponent("btnSetTeach"));
    btnGoTeach=dynamic_cast<TButton *>(FindComponent("btnGoTeach"));
    btnSave=dynamic_cast<TButton *>(FindComponent("btnSave"));
    btnReload=dynamic_cast<TButton *>(FindComponent("btnReload"));
    btnIOForm=dynamic_cast<TButton *>(FindComponent("btnIOForm"));
    btnClose=dynamic_cast<TButton *>(FindComponent("btnClose"));
    btnJogP=dynamic_cast<TButton *>(FindComponent("btnJogP"));
    btnJogN=dynamic_cast<TButton *>(FindComponent("btnJogN"));
    btnStepP=dynamic_cast<TButton *>(FindComponent("btnStepP"));
    btnStepN=dynamic_cast<TButton *>(FindComponent("btnStepN"));
    btnMove=dynamic_cast<TButton *>(FindComponent("btnMove"));
    btnHome=dynamic_cast<TButton *>(FindComponent("btnHome"));
    btnStop=dynamic_cast<TButton *>(FindComponent("btnStop"));
    btnRefresh=dynamic_cast<TButton *>(FindComponent("btnRefresh"));
    tmrUpdate=dynamic_cast<TTimer *>(FindComponent("tmrUpdate"));

    for(i=0; i<iMotLedTotalCnt; i++)
    {
        Name=AnsiString("lblStatus")+IntToStr(i);
        lblStatus[i]=dynamic_cast<TLabel *>(FindComponent(Name));
        Name=AnsiString("ledStatus")+IntToStr(i);
        ledStatus[i]=dynamic_cast<TALed *>(FindComponent(Name));
    }
}
//---------------------------------------------------------------------------
void TfTeach::BuildClientPanel()
{
    if(palClient==NULL)
        palClient=new TPanel(this);
    palClient->Parent=this;
    palClient->Align=alClient;
    palClient->BevelOuter=bvNone;
    palClient->Color=TEACH_COLOR_BG;

    if(palTitle==NULL)
        palTitle=new TPanel(this);
    palTitle->Parent=palClient;
    palTitle->Align=alTop;
    palTitle->Height=58;
    palTitle->Caption="Teach";
    palTitle->Color=TEACH_COLOR_TITLE;
    palTitle->Font->Color=clYellow;
    palTitle->Font->Height=-32;
    palTitle->Font->Style=TFontStyles() << fsBold;

    if(palFunction==NULL)
        palFunction=new TPanel(this);
    palFunction->Parent=palClient;
    palFunction->Align=alTop;
    palFunction->Height=52;
    palFunction->BevelOuter=bvLowered;
    palFunction->Color=TEACH_COLOR_BG;

    if(btnSetTeach==NULL)
        btnSetTeach=CreateButton(palFunction, 10, 10, 96, 30, "SET NOW", btnSetTeachClick);
    btnSetTeach->OnClick=btnSetTeachClick;
    if(btnGoTeach==NULL)
        btnGoTeach=CreateButton(palFunction, 114, 10, 86, 30, "GO", btnGoTeachClick);
    btnGoTeach->OnClick=btnGoTeachClick;
    if(btnSave==NULL)
        btnSave=CreateButton(palFunction, 208, 10, 86, 30, "SAVE", btnSaveClick);
    btnSave->OnClick=btnSaveClick;
    if(btnReload==NULL)
        btnReload=CreateButton(palFunction, 302, 10, 86, 30, "RELOAD", btnReloadClick);
    btnReload->OnClick=btnReloadClick;
    if(btnIOForm==NULL)
        btnIOForm=CreateButton(palFunction, 396, 10, 86, 30, "IO TOOL", btnIOFormClick);
    btnIOForm->OnClick=btnIOFormClick;
    if(btnClose==NULL)
        btnClose=CreateButton(palFunction, 490, 10, 86, 30, "EXIT", btnCloseClick);
    btnClose->OnClick=btnCloseClick;
    btnClose->Cancel=true;

    if(lblMessage==NULL)
        lblMessage=CreateLabel(palFunction, 592, 15, 220, 22, "");
    lblMessage->Font->Color=clNavy;

    if(PageTeach==NULL)
        PageTeach=new TPageControl(this);
    PageTeach->Parent=palClient;
    PageTeach->Align=alClient;
    PageTeach->Font->Height=-13;

    if(tsEmptyTray==NULL)
        tsEmptyTray=CreateTeachTab("Empty / Tray X");
    if(grdEmptyTray==NULL)
        grdEmptyTray=CreateTeachGrid(tsEmptyTray);
    else
        ConfigureTeachGrid(grdEmptyTray);
    if(tsLoaderSort==NULL)
        tsLoaderSort=CreateTeachTab("Loader / Sort X");
    if(grdLoaderSort==NULL)
        grdLoaderSort=CreateTeachGrid(tsLoaderSort);
    else
        ConfigureTeachGrid(grdLoaderSort);
    if(tsAuto==NULL)
        tsAuto=CreateTeachTab("Auto 1-6");
    if(grdAuto==NULL)
        grdAuto=CreateTeachGrid(tsAuto);
    else
        ConfigureTeachGrid(grdAuto);
    if(tsSortZ==NULL)
        tsSortZ=CreateTeachTab("Sort Z");
    if(grdSortZ==NULL)
        grdSortZ=CreateTeachGrid(tsSortZ);
    else
        ConfigureTeachGrid(grdSortZ);
    if(tsOthers==NULL)
        tsOthers=CreateTeachTab("Others");
    if(grdOthers==NULL)
        grdOthers=CreateTeachGrid(tsOthers);
    else
        ConfigureTeachGrid(grdOthers);
}
//---------------------------------------------------------------------------
void TfTeach::BuildMotorPanel()
{
    int i;
    int X;
    int Y;
    TButton *Button;

    if(palMotorControl==NULL)
        palMotorControl=new TPanel(this);
    palMotorControl->Parent=this;
    palMotorControl->Align=alRight;
    palMotorControl->Width=361;
    palMotorControl->BevelInner=bvLowered;
    palMotorControl->Color=TEACH_COLOR_BG;

    if(lblActiveMot==NULL)
        lblActiveMot=CreateLabel(palMotorControl, 2, 2, 357, 34, "Activel Motor");
    lblActiveMot->Alignment=taCenter;
    lblActiveMot->Font->Color=clBlue;
    lblActiveMot->Font->Height=-24;
    lblActiveMot->Font->Style=TFontStyles() << fsBold;

    if(palMotorName==NULL)
        palMotorName=new TPanel(this);
    palMotorName->Parent=palMotorControl;
    palMotorName->SetBounds(12, 44, 337, 44);
    palMotorName->BevelOuter=bvLowered;
    palMotorName->Color=TEACH_COLOR_DARK;
    palMotorName->Font->Color=clWhite;
    palMotorName->Font->Height=-15;
    palMotorName->Caption="";

    if(lblNowPos==NULL)
        lblNowPos=CreateLabel(palMotorControl, 12, 102, 96, 20, "Now Position");
    if(lblEncoder==NULL)
        lblEncoder=CreateLabel(palMotorControl, 12, 136, 96, 20, "Encoder");
    if(lblSpeed==NULL)
        lblSpeed=CreateLabel(palMotorControl, 12, 170, 96, 20, "Speed");
    if(lblStep==NULL)
        lblStep=CreateLabel(palMotorControl, 12, 204, 96, 20, "Step");
    if(lblTarget==NULL)
        lblTarget=CreateLabel(palMotorControl, 12, 238, 96, 20, "Move To");
    lblNowPos->Font->Color=clNavy;
    lblEncoder->Font->Color=clNavy;
    lblSpeed->Font->Color=clNavy;
    lblStep->Font->Color=clNavy;
    lblTarget->Font->Color=clNavy;

    if(edNowPos==NULL)
        edNowPos=CreateEdit(palMotorControl, 116, 98, 126, 24, "", true);
    edNowPos->ReadOnly=true;
    if(edEncoder==NULL)
        edEncoder=CreateEdit(palMotorControl, 116, 132, 126, 24, "", true);
    edEncoder->ReadOnly=true;
    if(edSpeed==NULL)
        edSpeed=CreateEdit(palMotorControl, 116, 166, 126, 24, "10", false);
    if(edStep==NULL)
        edStep=CreateEdit(palMotorControl, 116, 200, 126, 24, "1.00", false);
    if(edTarget==NULL)
        edTarget=CreateEdit(palMotorControl, 116, 234, 126, 24, "0.00", false);

    Button=dynamic_cast<TButton *>(FindComponent("btnMotorSet"));
    if(Button==NULL)
        Button=CreateButton(palMotorControl, 252, 98, 96, 28, "SET", btnSetTeachClick);
    Button->OnClick=btnSetTeachClick;
    if(btnMove==NULL)
        btnMove=CreateButton(palMotorControl, 252, 232, 96, 30, "MOVE", btnMoveClick);
    btnMove->OnClick=btnMoveClick;

    if(btnJogP==NULL)
        btnJogP=CreateButton(palMotorControl, 20, 282, 90, 34, "JOG +", NULL);
    if(btnJogN==NULL)
        btnJogN=CreateButton(palMotorControl, 126, 282, 90, 34, "JOG -", NULL);
    if(btnStepP==NULL)
        btnStepP=CreateButton(palMotorControl, 232, 282, 54, 34, "+", btnStepPClick);
    btnStepP->OnClick=btnStepPClick;
    if(btnStepN==NULL)
        btnStepN=CreateButton(palMotorControl, 294, 282, 54, 34, "-", btnStepNClick);
    btnStepN->OnClick=btnStepNClick;
    btnJogP->OnMouseDown=btnJogPMouseDown;
    btnJogP->OnMouseUp=btnJogMouseUp;
    btnJogN->OnMouseDown=btnJogNMouseDown;
    btnJogN->OnMouseUp=btnJogMouseUp;

    if(btnHome==NULL)
        btnHome=CreateButton(palMotorControl, 20, 326, 96, 36, "HOME", btnHomeClick);
    btnHome->OnClick=btnHomeClick;
    if(btnStop==NULL)
        btnStop=CreateButton(palMotorControl, 132, 326, 96, 36, "STOP", btnStopClick);
    btnStop->OnClick=btnStopClick;
    if(btnRefresh==NULL)
        btnRefresh=CreateButton(palMotorControl, 244, 326, 96, 36, "REFRESH", btnRefreshClick);
    btnRefresh->OnClick=btnRefreshClick;

    for(i=0; i<iMotLedTotalCnt; i++)
    {
        X=(i<6)?20:188;
        Y=382+(i%6)*24;
        if(lblStatus[i]==NULL)
            lblStatus[i]=CreateLabel(palMotorControl, X, Y, 92, 18, TEACH_LED_NAME[i]);
        lblStatus[i]->Color=TEACH_COLOR_DARK;
        lblStatus[i]->Font->Color=clWhite;
        if(ledStatus[i]==NULL)
            ledStatus[i]=new TALed(this);
        ledStatus[i]->Parent=palMotorControl;
        ledStatus[i]->SetBounds(X+98, Y, 22, 22);
        ledStatus[i]->LEDStyle=Aled::LEDSqLarge;
        ledStatus[i]->FalseColor=TEACH_COLOR_LED_OFF;
        if(i==iAlarmLed || i==iEmgLed || i==iServoalarmLed)
            ledStatus[i]->TrueColor=TEACH_COLOR_LED_ALARM;
        else
            ledStatus[i]->TrueColor=TEACH_COLOR_LED_ON;
        ledStatus[i]->Value=false;
    }

    if(lblMotorList==NULL)
        lblMotorList=CreateLabel(palMotorControl, 20, 540, 110, 20, "Motor List");
    lblMotorList->Font->Color=clNavy;
    if(lstMotors==NULL)
        lstMotors=new TListBox(this);
    lstMotors->Parent=palMotorControl;
    lstMotors->SetBounds(20, 562, 328, 260);
    lstMotors->ItemHeight=13;
    lstMotors->OnClick=lstMotorsClick;
}
//---------------------------------------------------------------------------
TTabSheet *TfTeach::CreateTeachTab(AnsiString Caption)
{
    TTabSheet *Tab=new TTabSheet(this);
    Tab->PageControl=PageTeach;
    Tab->Caption=Caption;
    return Tab;
}
//---------------------------------------------------------------------------
TStringGrid *TfTeach::CreateTeachGrid(TWinControl *Parent)
{
    TStringGrid *Grid=new TStringGrid(this);
    Grid->Parent=Parent;
    ConfigureTeachGrid(Grid);
    return Grid;
}
//---------------------------------------------------------------------------
void TfTeach::ConfigureTeachGrid(TStringGrid *Grid)
{
    if(Grid==NULL)
        return;
    Grid->Align=alClient;
    Grid->ColCount=5;
    Grid->RowCount=2;
    Grid->FixedRows=1;
    Grid->DefaultRowHeight=24;
    Grid->Color=TEACH_COLOR_GRID;
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
    Grid->OnSelectCell=grdTeachSelectCell;
    Grid->OnDblClick=grdTeachDblClick;
}
//---------------------------------------------------------------------------
TLabel *TfTeach::CreateLabel(TWinControl *Parent, int Left, int Top, int Width, int Height, AnsiString Caption)
{
    TLabel *Label=new TLabel(this);
    Label->Parent=Parent;
    Label->SetBounds(Left, Top, Width, Height);
    Label->AutoSize=false;
    Label->Caption=Caption;
    Label->ParentColor=false;
    Label->Color=TEACH_COLOR_BG;
    Label->Font->Name="MS Sans Serif";
    Label->Font->Height=-13;
    return Label;
}
//---------------------------------------------------------------------------
TEdit *TfTeach::CreateEdit(TWinControl *Parent, int Left, int Top, int Width, int Height, AnsiString Text, bool ReadOnly)
{
    TEdit *Edit=new TEdit(this);
    Edit->Parent=Parent;
    Edit->SetBounds(Left, Top, Width, Height);
    Edit->Text=Text;
    Edit->ReadOnly=ReadOnly;
    return Edit;
}
//---------------------------------------------------------------------------
TButton *TfTeach::CreateButton(TWinControl *Parent, int Left, int Top, int Width, int Height, AnsiString Caption, TNotifyEvent OnClick)
{
    TButton *Button=new TButton(this);
    Button->Parent=Parent;
    Button->SetBounds(Left, Top, Width, Height);
    Button->Caption=Caption;
    Button->Font->Height=-13;
    if(OnClick!=NULL)
        Button->OnClick=OnClick;
    return Button;
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
    UpdateMotorMonitor();
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

    if(Motor==NULL)
    {
        if(palMotorName!=NULL)
            palMotorName->Caption="";
        if(edNowPos!=NULL)
            edNowPos->Text="";
        if(edEncoder!=NULL)
            edEncoder->Text="";
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

    for(int i=0; i<iMotLedTotalCnt; i++)
        UpdateStatusLed(i, Motor->Led[i]);

    if(SelectedTeachIndex>=0)
        RefreshTeachRow(SelectedTeachIndex);
}
//---------------------------------------------------------------------------
void TfTeach::UpdateStatusLed(int LedIndex, bool Value)
{
    if(LedIndex<0 || LedIndex>=iMotLedTotalCnt || ledStatus[LedIndex]==NULL)
        return;

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
    if(CheckCanTeachMove(Motor, true, false, 0)==false)
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
    (void)Sender;

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
void __fastcall TfTeach::btnCloseClick(TObject *Sender)
{
    (void)Sender;
    Close();
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
    Motor->InitHomeTask();
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