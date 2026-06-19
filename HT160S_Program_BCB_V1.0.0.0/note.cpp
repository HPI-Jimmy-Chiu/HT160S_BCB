//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
#include "database.h"
#include "cEventLog.h"
#include "cmydef.h"
#include "csystem.h"
#include "main.h"
#include "mymessbox.h"
#include "SecsGem/UsecegemMainFrom.h"
#include "SecsGem/uHGemHT160.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "HTray"
#pragma resource "*.dfm"
TfNote *fNote=NULL;
//---------------------------------------------------------------------------
DWORD RecordHappenTime=0;
static const TColor NOTE_BG=(TColor)12761254;
static const TColor MACHINE_NORMAL=(TColor)10398010;
static const TColor TRAY_NORMAL=(TColor)14473944;
static const TColor ALARM_COLOR=clRed;
static const TColor SELECT_COLOR=clRed;
static const TColor COMMAND_NORMAL=(TColor)8404992;
static const TColor COMMAND_SKIP=(TColor)8421440;
//---------------------------------------------------------------------------
static void EnsureNote()
{
    if(fNote==NULL)
        fNote=new TfNote(Application);
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260615 : the legacy note process_*.log writer
//(GetNoteLogFileName + AppendNoteLog, full-file rewrite, no lock) is removed.
//Alarm / process / pass-time records now go through g_EventLog (cEventLog),
//matching the HT172 RecordAlarmMessage -> EventLog CSV path. See note callers
//ProcessErrMessage / RecordProcess / RecordAlarmMessagePassTime below.
//---------------------------------------------------------------------------
static void SetCommandButtonColor(TPanel *Panel, bool Selected)
{
    if(Panel==NULL)
        return;

    if(Selected)
        Panel->Color=SELECT_COLOR;
    else if(Panel->Name=="BtnSkip")
        Panel->Color=COMMAND_SKIP;
    else if(Panel->Name=="BtnStart")
        Panel->Color=clBlue;
    else
        Panel->Color=COMMAND_NORMAL;
}
//---------------------------------------------------------------------------
MyNoteStruct::MyNoteStruct()
{
    SystemErrorCode=new TStringList;
    SysFlushPanelName=new TStringList;
}
//---------------------------------------------------------------------------
MyNoteStruct::~MyNoteStruct()
{
    Clear();
    delete SystemErrorCode;
    delete SysFlushPanelName;
}
//---------------------------------------------------------------------------
void MyNoteStruct::AddSysErr(AnsiString ErrorCode, AnsiString FlushPanel)
{
    SystemErrorCode->Add(ErrorCode);
    SysFlushPanelName->Add(FlushPanel);
}
//---------------------------------------------------------------------------
void MyNoteStruct::Clear()
{
    SystemErrorCode->Clear();
    SysFlushPanelName->Clear();
}
//---------------------------------------------------------------------------
__fastcall TfNote::TfNote(TComponent* Owner)
    : TForm(Owner)
{
    int Index;

    Code=0;
    ReturnCode=0;
    fShow=false;
    KeyCode=0;
    iBackOldMemo2Y=0;
    iBackMemo2Height=0;
    fMemoPos=false;
    sObjName="";
    FlushPanel=NULL;
    FlushPanelColor=MACHINE_NORMAL;
    SystemError=new MyNoteStruct;
    bMachineLayoutBuilt=false;
    bOffBuzzer=false;

    for(Index=0; Index<6; Index++)
        Select[Index]=false;

    mtLoaderBuffer=NULL;
    mtLoader=NULL;
    mtFix1=NULL;
    mtAuto1=NULL;
    mtAuto2=NULL;
    mtAuto3=NULL;
    mtColor=NULL;
    mtEmpty=NULL;
    mtFix2=NULL;
    mtFix3=NULL;
    mtAuto1Buffer=NULL;
    mtAuto2Buffer=NULL;
    mtAuto3Buffer=NULL;
    mtInCarrier=NULL;
    mtStandbyBuffer1=NULL;
    mtStandbyBuffer2=NULL;
    mtOutCarrier=NULL;
    mtOutCarrierShuttle=NULL;
    mtInCarrierShuttle=NULL;
    mtInCarrierBuffer=NULL;
    mtOutCarrierShuttleBuffer=NULL;
    mtTNTStation1=NULL;
    mtTNTStation2=NULL;
    mtTNTStation3=NULL;
    mtAutoKnock=NULL;
    mtCatchTray=NULL;
    mtInShuttle=NULL;
    mtOutShuttle=NULL;
    mtEmptyBuffer=NULL;
    mtColorBuffer=NULL;

    pn_MachineFront=NULL;
    pn_SafeDoor1=NULL;
    pn_SafeDoor2=NULL;
    pn_SafeDoor3=NULL;
    pn_SafeDoor4=NULL;
    pn_SafeDoor5=NULL;
    pn_SafeDoor6=NULL;
    pn_SafeDoor7=NULL;
    pn_SafeDoor8=NULL;
    pn_SafeDoor9=NULL;
    pn_InShuttle=NULL;
    pn_OutShuttle=NULL;
    pn_System=NULL;
    pn_InArmF=NULL;
    pn_InArmR=NULL;
    pn_OutArm=NULL;
    pn_TrayArm=NULL;
    pn_TempAndTest=NULL;
    palSysErr=NULL;

    BuildMachineLayout();
}
//---------------------------------------------------------------------------
TPanel *__fastcall TfNote::CreateMachinePanel(TWinControl *ParentControl, AnsiString Name, AnsiString Caption, int Left, int Top, int Width, int Height, TColor Color)
{
    TPanel *Panel;

    Panel=new TPanel(this);
    Panel->Name=Name;
    Panel->Parent=ParentControl;
    Panel->Left=Left;
    Panel->Top=Top;
    Panel->Width=Width;
    Panel->Height=Height;
    Panel->Caption=Caption;
    Panel->BevelInner=bvLowered;
    Panel->BevelOuter=bvRaised;
    Panel->Color=Color;
    Panel->ParentColor=false;
    Panel->Font->Color=clWhite;
    Panel->Font->Name="Arial";
    Panel->Font->Height=-11;
    Panel->ParentFont=false;
    return Panel;
}
//---------------------------------------------------------------------------
void __fastcall TfNote::CreateMachineLabel(TWinControl *ParentControl, AnsiString Name, AnsiString Caption, int Left, int Top, int Width, int Height)
{
    TLabel *Label;

    Label=new TLabel(this);
    Label->Name=Name;
    Label->Parent=ParentControl;
    Label->Left=Left;
    Label->Top=Top;
    Label->Width=Width;
    Label->Height=Height;
    Label->Alignment=taCenter;
    Label->AutoSize=false;
    Label->Caption=Caption;
    Label->Color=NOTE_BG;
    Label->Font->Color=clWhite;
    Label->Font->Name="Arial";
    Label->Font->Height=-11;
    Label->ParentColor=false;
    Label->ParentFont=false;
}
//---------------------------------------------------------------------------
TTMyTray *__fastcall TfNote::CreateMachineTray(TWinControl *ParentControl, AnsiString Name, AnsiString Caption, int Left, int Top, int Width, int Height)
{
    TTMyTray *Tray;

    Tray=new TTMyTray(this);
    Tray->Name=Name;
    Tray->Parent=ParentControl;
    Tray->Left=Left;
    Tray->Top=Top;
    Tray->Width=Width;
    Tray->Height=Height;
    Tray->Color=TRAY_NORMAL;
    Tray->XItem=5;
    Tray->YItem=8;
    CreateMachineLabel(ParentControl, AnsiString("lab_")+Name, Caption, Left-10, Top-16, Width+20, 14);
    return Tray;
}
//---------------------------------------------------------------------------
void __fastcall TfNote::BuildMachineLayout()
{
    if(bMachineLayoutBuilt)
        return;
    if(PanelMain6==NULL)
        return;

    bMachineLayoutBuilt=true;
    PanelMain6->Color=NOTE_BG;

    palSysErr=CreateMachinePanel(PanelMain6, "palSysErr", "System Error", 790, 20, 150, 44, MACHINE_NORMAL);
    pn_MachineFront=CreateMachinePanel(PanelMain6, "pn_MachineFront", "", 24, 462, 900, 16, MACHINE_NORMAL);

    mtLoaderBuffer=CreateMachineTray(PanelMain6, "mtLoaderBuffer", "Loader Buf", 90, 380, 58, 88);
    mtLoader=CreateMachineTray(PanelMain6, "mtLoader", "Loader", 90, 252, 58, 88);
    mtEmpty=CreateMachineTray(PanelMain6, "mtEmpty", "Empty", 190, 252, 58, 88);
    mtColor=CreateMachineTray(PanelMain6, "mtColor", "Color", 260, 252, 58, 88);
    mtFix1=CreateMachineTray(PanelMain6, "mtFix1", "Fix1", 350, 252, 58, 88);
    mtFix2=CreateMachineTray(PanelMain6, "mtFix2", "Fix2", 420, 252, 58, 88);
    mtFix3=CreateMachineTray(PanelMain6, "mtFix3", "Fix3", 490, 252, 58, 88);
    mtAuto1=CreateMachineTray(PanelMain6, "mtAuto1", "Auto1", 590, 252, 58, 88);
    mtAuto2=CreateMachineTray(PanelMain6, "mtAuto2", "Auto2", 660, 252, 58, 88);
    mtAuto3=CreateMachineTray(PanelMain6, "mtAuto3", "Auto3", 730, 252, 58, 88);

    mtAuto1Buffer=CreateMachineTray(PanelMain6, "mtAuto1Buffer", "A1 Buf", 590, 380, 58, 88);
    mtAuto2Buffer=CreateMachineTray(PanelMain6, "mtAuto2Buffer", "A2 Buf", 660, 380, 58, 88);
    mtAuto3Buffer=CreateMachineTray(PanelMain6, "mtAuto3Buffer", "A3 Buf", 730, 380, 58, 88);
    mtEmptyBuffer=CreateMachineTray(PanelMain6, "mtEmptyBuffer", "Empty Buf", 190, 380, 58, 88);
    mtColorBuffer=CreateMachineTray(PanelMain6, "mtColorBuffer", "Color Buf", 260, 380, 58, 88);

    pn_SafeDoor1=CreateMachinePanel(PanelMain6, "pn_SafeDoor1", "1", 24, 252, 18, 100, MACHINE_NORMAL);
    pn_SafeDoor2=CreateMachinePanel(PanelMain6, "pn_SafeDoor2", "2", 24, 142, 18, 100, MACHINE_NORMAL);
    pn_SafeDoor3=CreateMachinePanel(PanelMain6, "pn_SafeDoor3", "3", 24, 42, 18, 92, MACHINE_NORMAL);
    pn_SafeDoor4=CreateMachinePanel(PanelMain6, "pn_SafeDoor4", "4", 44, 24, 150, 18, MACHINE_NORMAL);
    pn_SafeDoor5=CreateMachinePanel(PanelMain6, "pn_SafeDoor5", "5", 690, 24, 150, 18, MACHINE_NORMAL);
    pn_SafeDoor6=CreateMachinePanel(PanelMain6, "pn_SafeDoor6", "6", 860, 42, 18, 92, MACHINE_NORMAL);
    pn_SafeDoor7=CreateMachinePanel(PanelMain6, "pn_SafeDoor7", "7", 860, 142, 18, 100, MACHINE_NORMAL);
    pn_SafeDoor8=CreateMachinePanel(PanelMain6, "pn_SafeDoor8", "8", 860, 252, 18, 100, MACHINE_NORMAL);
    pn_SafeDoor9=CreateMachinePanel(PanelMain6, "pn_SafeDoor9", "9", 792, 48, 54, 30, MACHINE_NORMAL);

    pn_InShuttle=CreateMachinePanel(PanelMain6, "pn_InShuttle", "In Shuttle", 86, 202, 78, 26, MACHINE_NORMAL);
    pn_OutShuttle=CreateMachinePanel(PanelMain6, "pn_OutShuttle", "Out Shuttle", 630, 202, 86, 26, MACHINE_NORMAL);
    pn_System=CreateMachinePanel(PanelMain6, "pn_System", "System", 462, 372, 86, 76, MACHINE_NORMAL);
    pn_InArmF=CreateMachinePanel(PanelMain6, "pn_InArmF", "In Arm F", 48, 232, 170, 16, MACHINE_NORMAL);
    pn_InArmR=CreateMachinePanel(PanelMain6, "pn_InArmR", "In Arm R", 48, 184, 260, 16, MACHINE_NORMAL);
    pn_OutArm=CreateMachinePanel(PanelMain6, "pn_OutArm", "Out Arm", 612, 184, 260, 16, MACHINE_NORMAL);
    pn_TrayArm=CreateMachinePanel(PanelMain6, "pn_TrayArm", "", 330, 252, 18, 100, MACHINE_NORMAL);

    mtInCarrierShuttle=CreateMachineTray(PanelMain6, "mtInCarrierShuttle", "In Shuttle", 54, 142, 88, 42);
    mtInCarrier=CreateMachineTray(PanelMain6, "mtInCarrier", "In Carrier", 166, 142, 88, 42);
    mtStandbyBuffer2=CreateMachineTray(PanelMain6, "mtStandbyBuffer2", "Standby2", 288, 142, 88, 42);
    mtStandbyBuffer1=CreateMachineTray(PanelMain6, "mtStandbyBuffer1", "Standby1", 410, 142, 88, 42);
    mtOutCarrier=CreateMachineTray(PanelMain6, "mtOutCarrier", "Out Carrier", 532, 142, 88, 42);
    mtOutCarrierShuttle=CreateMachineTray(PanelMain6, "mtOutCarrierShuttle", "Out Shuttle", 654, 142, 88, 42);
    mtInCarrierBuffer=CreateMachineTray(PanelMain6, "mtInCarrierBuffer", "In Buf", 166, 74, 88, 42);
    mtOutCarrierShuttleBuffer=CreateMachineTray(PanelMain6, "mtOutCarrierShuttleBuffer", "Out Buf", 654, 74, 88, 42);

    pn_TempAndTest=CreateMachinePanel(PanelMain6, "pn_TempAndTest", "Temp And Test", 292, 28, 330, 108, MACHINE_NORMAL);
    mtTNTStation1=CreateMachineTray(pn_TempAndTest, "mtTNTStation1", "TNT1", 14, 48, 82, 40);
    mtTNTStation2=CreateMachineTray(pn_TempAndTest, "mtTNTStation2", "TNT2", 124, 48, 82, 40);
    mtTNTStation3=CreateMachineTray(pn_TempAndTest, "mtTNTStation3", "TNT3", 234, 48, 82, 40);

    mtAutoKnock=CreateMachineTray(PanelMain6, "mtAutoKnock", "Auto Knock", 812, 380, 68, 88);
    mtCatchTray=CreateMachineTray(PanelMain6, "mtCatchTray", "Catch Tray", 902, 380, 68, 88);
    mtInShuttle=CreateMachineTray(PanelMain6, "mtInShuttle", "In Shutt", 52, 74, 88, 42);
    mtOutShuttle=CreateMachineTray(PanelMain6, "mtOutShuttle", "Out Shutt", 770, 142, 88, 42);

    Reset();
}
//---------------------------------------------------------------------------
void __fastcall TfNote::SetMachinePanelColor(TPanel *Panel, TColor Color)
{
    if(Panel!=NULL)
        Panel->Color=Color;
}
//---------------------------------------------------------------------------
void __fastcall TfNote::Reset()
{
    SetMachinePanelColor(palSysErr, MACHINE_NORMAL);
    SetMachinePanelColor(pn_MachineFront, MACHINE_NORMAL);
    SetMachinePanelColor(pn_SafeDoor1, MACHINE_NORMAL);
    SetMachinePanelColor(pn_SafeDoor2, MACHINE_NORMAL);
    SetMachinePanelColor(pn_SafeDoor3, MACHINE_NORMAL);
    SetMachinePanelColor(pn_SafeDoor4, MACHINE_NORMAL);
    SetMachinePanelColor(pn_SafeDoor5, MACHINE_NORMAL);
    SetMachinePanelColor(pn_SafeDoor6, MACHINE_NORMAL);
    SetMachinePanelColor(pn_SafeDoor7, MACHINE_NORMAL);
    SetMachinePanelColor(pn_SafeDoor8, MACHINE_NORMAL);
    SetMachinePanelColor(pn_SafeDoor9, MACHINE_NORMAL);
    SetMachinePanelColor(pn_InShuttle, MACHINE_NORMAL);
    SetMachinePanelColor(pn_OutShuttle, MACHINE_NORMAL);
    SetMachinePanelColor(pn_System, MACHINE_NORMAL);
    SetMachinePanelColor(pn_InArmF, MACHINE_NORMAL);
    SetMachinePanelColor(pn_InArmR, MACHINE_NORMAL);
    SetMachinePanelColor(pn_OutArm, MACHINE_NORMAL);
    SetMachinePanelColor(pn_TrayArm, MACHINE_NORMAL);
    SetMachinePanelColor(pn_TempAndTest, MACHINE_NORMAL);
    FlushPanel=NULL;
}
//---------------------------------------------------------------------------
void __fastcall TfNote::FormShow(TObject *Sender)
{
    TPanel *RecoveryButtons[6]={BtnSkip, BtnRetry, BtnTrayFeed, BtnTrayEnd, BtnCleanOut, BtnHome};
    int KeyComp[6]={K_SKIP, K_RETRY, K_TRAY_FEED, K_TRAY_END, K_CLEAN_OUT, K_HOME};
    int Index;

    fShow=true;
    ReturnCode=0;
    if(fMain!=NULL)
    {
        Left=fMain->Left+(fMain->Width-Width)/2;
        Top=fMain->Top+(fMain->Height-Height)/2;
    }

    for(Index=0; Index<6; Index++)
    {
        Select[Index]=false;
        SetCommandButtonColor(RecoveryButtons[Index], false);
        if(KeyCode==0)
            RecoveryButtons[Index]->Visible=false;
        else
            RecoveryButtons[Index]->Visible=((KeyCode & KeyComp[Index])!=0);
    }

    //AI(ht160s-maintainer) 20260617 : light the front-panel recovery-button LEDs
    //this prompt is offering, so the operator sees which keys are live (HT172
    //note.cpp bLamp* port; DoPanelLamp pushes these onto SwFK*).
    bLampSkip    =((KeyCode & K_SKIP)!=0);
    bLampRetry   =((KeyCode & K_RETRY)!=0);
    bLampTrayFeed=((KeyCode & K_TRAY_FEED)!=0);
    bLampTrayEnd =((KeyCode & K_TRAY_END)!=0);
    bLampCleanOut=((KeyCode & K_CLEAN_OUT)!=0);

    if(FlushPanel!=NULL)
        FlushPanelColor=FlushPanel->Color;
}
//---------------------------------------------------------------------------
void __fastcall TfNote::FormClose(TObject *Sender, TCloseAction &Action)
{
    CloseBuzzerOff();
    bOffBuzzer=false;
    fShow=false;
    //AI(ht160s-maintainer) 20260617 : prompt dismissed -> clear recovery LEDs.
    bLampSkip=false;
    bLampRetry=false;
    bLampTrayFeed=false;
    bLampTrayEnd=false;
    bLampCleanOut=false;
    if(FlushPanel!=NULL)
        FlushPanel->Color=FlushPanelColor;
}
//---------------------------------------------------------------------------
void __fastcall TfNote::Timer1Timer(TObject *Sender)
{
    static bool Blink=false;

    if(fShow==false)
        return;

    //AI(HT160S-Maintainer) 20260619 : physical operator-panel keys on the alarm
    //screen (HT172 note ScanKey port). May Close() the form on Start/Pause, so
    //re-check fShow before the blink work below.
    ScanKey();
    if(fShow==false)
        return;

    if(bOffBuzzer)
        CloseBuzzerOff();
    if(FlushPanel==NULL)
        return;

    Blink=!Blink;
    if(Blink)
        FlushPanel->Color=ALARM_COLOR;
    else
        FlushPanel->Color=FlushPanelColor;
}
//---------------------------------------------------------------------------
void __fastcall TfNote::UpdateButtonStatus(TObject *Sender)
{
    TPanel *RecoveryButtons[6]={BtnSkip, BtnRetry, BtnTrayFeed, BtnTrayEnd, BtnCleanOut, BtnHome};
    //AI(HT160S-Maintainer) 20260609 : caller compares Ret against K_* bitmask (K_SKIP..K_HOME),
    //so ReturnCode must carry the matching bit value, not the sequential DFM Tag (Tag 3..6 != K 4..32).
    //Same order as FormShow KeyComp. Index resolved by Sender pointer to drop the fragile Tag-1 coupling.
    static const int KeyCodeByIndex[6]={K_SKIP, K_RETRY, K_TRAY_FEED, K_TRAY_END, K_CLEAN_OUT, K_HOME};
    TPanel *PanelPtr;
    int Index;
    int SelectIndex;

    PanelPtr=dynamic_cast<TPanel *>(Sender);
    if(PanelPtr==NULL)
        return;

    SelectIndex=-1;
    for(Index=0; Index<6; Index++)
    {
        if(RecoveryButtons[Index]==PanelPtr)
            SelectIndex=Index;
    }
    if(SelectIndex<0)
        return;

    for(Index=0; Index<6; Index++)
    {
        Select[Index]=(Index==SelectIndex);
        SetCommandButtonColor(RecoveryButtons[Index], Select[Index]);
    }
    ReturnCode=KeyCodeByIndex[SelectIndex];
}
//---------------------------------------------------------------------------
void __fastcall TfNote::BtnSkipClick(TObject *Sender)
{
    UpdateButtonStatus(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TfNote::BtnStartClick(TObject *Sender)
{
    SoftStop=false;
    SoftStart=true;
    RecordProcess("START pressed");
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TfNote::BtnPauseClick(TObject *Sender)
{
    SoftStart=false;
    SoftStop=true;
    HSys.DecStopAllMotor();
    RecordProcess("PAUSE pressed");
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TfNote::BtnOffBuzzerClick(TObject *Sender)
{
    bOffBuzzer=true;
    CloseBuzzerOff();
    RecordProcess("OFF BUZZER pressed");
}
//---------------------------------------------------------------------------
void __fastcall TfNote::FlushLabel()
{
}
//---------------------------------------------------------------------------
void __fastcall TfNote::ScanKey()
{
    //AI(HT160S-Maintainer) 20260619 : HT172 note.cpp TfNote::ScanKey port. The
    //alarm/recovery screen now responds to the physical operator panel. HT160
    //sensors carry no Tag (unlike HT172 ScanPannelKey/.Tag), so each key is read
    //directly and rising-edge latched so a held button fires once.
    //AI 20260619 : read FRONT keys too (SnFK*||SnRK*). This machine is front-panel
    //only, so the original rear-only (SnRK*) read made the alarm/recovery screen
    //ignore every physical button. Aligned with the main run screen (ScanPanelKeys)
    //and TfHome::ScanKey, which already read SnFK*||SnRK*.
    //recovery keys (Skip/Retry/TrayFeed/TrayEnd/CleanOut/Home) just select the
    //matching on-screen button (UpdateButtonStatus, same as a touch); Start and
    //Pause run the existing click handlers. SECS events mirror HT172; HT160 has
    //no PressReset/PressTrayEnd/HasICUnderMachine, so the nearest event is used.
    static bool bWasStart=false;
    static bool bWasPause=false;
    static bool bWasSkip=false;
    static bool bWasRetry=false;
    static bool bWasTrayFeed=false;
    static bool bWasTrayEnd=false;
    static bool bWasCleanOut=false;
    static bool bWasReset=false;

    TPanel *Ptr[6]={BtnSkip, BtnRetry, BtnTrayFeed, BtnTrayEnd, BtnCleanOut, BtnHome};
    bool bKey[6];
    bKey[0]=HSys.Sen.SnFKSkip.IsOn()     || HSys.Sen.SnRKSkip.IsOn();
    bKey[1]=HSys.Sen.SnFKRetry.IsOn()    || HSys.Sen.SnRKRetry.IsOn();
    bKey[2]=HSys.Sen.SnFKTrayFeed.IsOn() || HSys.Sen.SnRKTrayFeed.IsOn();
    bKey[3]=HSys.Sen.SnFKTrayEnd.IsOn()  || HSys.Sen.SnRKTrayEnd.IsOn();
    bKey[4]=HSys.Sen.SnFKCleanOut.IsOn() || HSys.Sen.SnRKCleanOut.IsOn();
    bKey[5]=HSys.Sen.SnFKHome.IsOn()     || HSys.Sen.SnRKHome.IsOn();

    for(int i=0; i<6; i++)
    {
        if(Ptr[i]->Visible && bKey[i])
            UpdateButtonStatus(Ptr[i]);
    }

    bool bStart=HSys.Sen.SnFKStart.IsOn()      || HSys.Sen.SnRKStart.IsOn();
    bool bPause=HSys.Sen.SnFKPause.IsOn()      || HSys.Sen.SnRKPause.IsOn();
    bool bReset=HSys.Sen.SnFKAlarmReset.IsOn() || HSys.Sen.SnRKAlarmReset.IsOn();

    if(bStart && bWasStart==false)
    {
        EventReport(SECS_EVENT.PressStartWithoutIC);
        BtnStartClick(this);
    }
    else if(bPause && bWasPause==false)
    {
        EventReport(SECS_EVENT.PressPause);
        BtnPauseClick(this);
    }
    else if(bKey[0] && bWasSkip==false)
    {
        RecordProcess("SKIP pressed");
        EventReport(SECS_EVENT.PressSkip);
    }
    else if(bKey[1] && bWasRetry==false)
    {
        RecordProcess("RETRY pressed");
        EventReport(SECS_EVENT.PressRetry);
    }
    else if(bKey[2] && bWasTrayFeed==false)
    {
        RecordProcess("TRAY FEED pressed");
        EventReport(SECS_EVENT.PressTrayFeed);
    }
    else if(bKey[3] && bWasTrayEnd==false)
    {
        RecordProcess("TRAY END pressed");
        EventReport(SECS_EVENT.PressTrayFeed);
    }
    else if(bKey[4] && bWasCleanOut==false)
    {
        RecordProcess("CLEAN OUT pressed");
        EventReport(SECS_EVENT.PressCleanOut);
    }
    else if(bReset && bWasReset==false)
    {
        RecordProcess("ALARM RESET pressed");
        EventReport(SECS_EVENT.PressAlarmReset);
        CloseBuzzerOff();
    }

    bWasStart=bStart;
    bWasPause=bPause;
    bWasSkip=bKey[0];
    bWasRetry=bKey[1];
    bWasTrayFeed=bKey[2];
    bWasTrayEnd=bKey[3];
    bWasCleanOut=bKey[4];
    bWasReset=bReset;
}
//---------------------------------------------------------------------------
void __fastcall TfNote::Start()
{
    BtnStartClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TfNote::LevelProcessErrMessage()
{
}
//---------------------------------------------------------------------------
bool __fastcall TfNote::CheckCodeIsExist(AnsiString Str)
{
    return false;
}
//---------------------------------------------------------------------------
void __fastcall TfNote::ProcessErrMessage(AnsiString EC, AnsiString Str, int Type)
{
    if(EC=="" && Str=="")
        return;
    RecordHappenTime=GetTickCount();
    // EventLog columns: AlarmCode=EC, Message=Str, ErrorPart="TYPE=n".
    g_EventLog.Log(EC, Str, AnsiString("TYPE=")+IntToStr(Type));
}
//---------------------------------------------------------------------------
void TfNote::GetFlushPanel(TWinControl *PCtrl, AnsiString PanelName)
{
    TPanel *PanelPtr;
    int Index;

    if(PCtrl==PanelMain6)
        FlushPanel=NULL;
    if(PCtrl==NULL)
        return;

    PanelPtr=dynamic_cast<TPanel *>(PCtrl);
    if(PanelPtr!=NULL && PanelPtr->Name==PanelName)
    {
        FlushPanel=PanelPtr;
        return;
    }

    for(Index=0; Index<PCtrl->ControlCount; Index++)
    {
        TWinControl *ChildControl=dynamic_cast<TWinControl *>(PCtrl->Controls[Index]);
        if(ChildControl!=NULL)
            GetFlushPanel(ChildControl, PanelName);
        if(FlushPanel!=NULL)
            return;
    }
}
//---------------------------------------------------------------------------
void TfNote::ChangePalPos(TPanel *Panel, int Height, int Left, int Top, int Width, bool Visible)
{
    if(Panel==NULL)
        return;
    Panel->Height=Height;
    Panel->Left=Left;
    Panel->Top=Top;
    Panel->Width=Width;
    Panel->Visible=Visible;
}
//---------------------------------------------------------------------------
static int ShowNoteAlarm(AnsiString Code, AnsiString Message, AnsiString Detail, int KCode, AnsiString FlushPanelName)
{
    EnsureNote();
    if(fNote->fShow)
        return 0;

    HSys.DecStopAllMotor();
    HSys.Sys.SystemStart=false;
    SoftStop=false;
    SoftStart=false;

    fNote->Reset();
    fNote->KeyCode=KCode;
    fNote->edtAlarmCode->Text=Code;
    fNote->edtAlarmMsg->Text=Message;
    fNote->Memo1->Clear();
    if(Detail!="")
        fNote->Memo1->Lines->Add(Detail);

    if(FlushPanelName!="")
        fNote->GetFlushPanel(fNote->PanelMain6, FlushPanelName);
    if(fNote->FlushPanel==NULL)
        fNote->FlushPanel=fNote->pn_System;

    fNote->ProcessErrMessage(Code, Message, 1);
    fNote->ShowModal();
    return fNote->ReturnCode;
}
//---------------------------------------------------------------------------
void ShowCylinderError(int Code, int Type)
{
    AnsiString AlarmCode=AnsiString().sprintf("CYL%04d", Code);
    ShowNoteAlarm(AlarmCode, "Cylinder Error", AnsiString().sprintf("Type=%d", Type), K_RETRY, "pn_System");
}
//---------------------------------------------------------------------------
void ShowMotorError(AnsiString Code)
{
    ShowMotorError(Code, "");
}
//---------------------------------------------------------------------------
void ShowMotorError(AnsiString Code, AnsiString sFunc)
{
    AnsiString Detail;

    Detail="Motor error";
    if(sFunc!="")
        Detail=Detail+AnsiString("; Func=")+sFunc;
    ShowNoteAlarm(Code, "Motor Error", Detail, K_RETRY, "pn_System");
    fAllMotorHome=false;
    ChangeRunMode(Run_Home);
}
//---------------------------------------------------------------------------
int ShowSuckError(TMySucker &Ptr, int CodeType, int KCode, AnsiString HappenRegion)
{
    AnsiString Code;
    AnsiString Message;

    Code.sprintf("SUC%03d%d", Ptr.Tag, CodeType);
    Message=Ptr.SuckerName+AnsiString(" Sucker Error");
    return ShowNoteAlarm(Code, Message, HappenRegion, KCode, "pn_InArmF");
}
//---------------------------------------------------------------------------
int ShowSuckError(TMyKitSuck &Ptr, int CodeType, int KCode, AnsiString errPart, int iDuplicate)
{
    AnsiString Code;
    AnsiString Message;

    Code.sprintf("SUC%03d%d", Ptr.Tag, CodeType);
    Message=Ptr.Name+AnsiString(" Sucker Error");
    if(iDuplicate!=0)
        Message=Message+AnsiString(" (Again!)");
    return ShowNoteAlarm(Code, Message, errPart, KCode, Ptr.FlushPanelName);
}
//---------------------------------------------------------------------------
int ShowSystemError(int CodeType, int KCode)
{
    AnsiString Code=AnsiString().sprintf("SYS%04d", CodeType);
    return ShowNoteAlarm(Code, "System Error", "", KCode, "pn_System");
}
//---------------------------------------------------------------------------
int ShowSystemError(AnsiString Name, int KCode, int iDuplicate, AnsiString Message)
{
    //AI(HT160S-Maintainer) 20260603 : alarm-code table lookup + language select + undefined-code branch,
    //architecture aligned with HT172 ShowSystemError. Registered codes render bilingual
    //message/description/panel; unregistered legacy names keep the original passthrough so
    //existing string-based callers are not regressed. Part-name wording may differ from HT172.
    AnsiString Code, Mess, Desc, PanelName;

    std::map<AnsiString, AnsiString>::iterator IterName=HSys.mapNameToAlarm.find(Name);
    if(IterName==HSys.mapNameToAlarm.end())
    {
        Mess=(Message=="")?Name:Message;
        if(iDuplicate!=0)
            Mess=Mess+AnsiString(" (Again!)");
        return ShowNoteAlarm(Name, Mess, Message, KCode, Name);
    }

    Code=IterName->second;
    HSys.IterAlarmCodeList=HSys.mapAlarmCodeList.find(Code);
    if(HSys.IterAlarmCodeList==HSys.mapAlarmCodeList.end())
    {
        int iAlarmType=Code.SubString(1, 1).ToIntDef((int)eOther);
        PanelName="pn_System";
        if(iAlarmType==eJamErr)             Mess="Jam Code Undefine Error";
        else if(iAlarmType==eMessageErr)    Mess="Message Code Undefine Error";
        else if(iAlarmType==eFunErr)        Mess="Function Code Undefine Error";
        else if(iAlarmType==eSystemMess)    Mess="System Message Code Undefine Error";
        else if(iAlarmType==eCynAlarm)      Mess="Cylinder Error Code Undefine Error";
        else if(iAlarmType==eMotorAlarm)    Mess="Motor Error Code Undefine Error";
        else if(iAlarmType==eSuckAlarm)     Mess="Suck Error Code Undefine Error";
        else if(iAlarmType==eRecordProcess) Mess="Record Process Code Undefine Error";
        else if(iAlarmType==eOther)         Mess="Other Code Undefine Error";
        else                                Mess="System Error--System Code Unknown Error";
        Desc=AnsiString().sprintf("Func: Name=%s KCode=%d", Name.c_str(), KCode);
        Code="-"+Code;
    }
    else
    {
        PanelName=HSys.IterAlarmCodeList->second.FlushPanelName;
        if(HSys.LastSet.iLanguageCountry==0)
        {
            Mess=HSys.IterAlarmCodeList->second.E_ErrMessage;
            Desc=HSys.IterAlarmCodeList->second.E_Description;
        }
        else
        {
            Mess=HSys.IterAlarmCodeList->second.C_ErrMessage;
            Desc=HSys.IterAlarmCodeList->second.C_Description;
        }
    }

    if(PanelName=="")
        PanelName="pn_System";
    Desc=StringReplace(Desc, "\\r\\n", "\r\n", TReplaceFlags()<<rfReplaceAll);
    if(Message!="")
        Desc=Desc+AnsiString("\r\n")+Message;
    if(iDuplicate!=0)
        Mess=Mess+AnsiString(" (Again!)");

    return ShowNoteAlarm(Code, Mess, Desc, KCode, PanelName);
}
//---------------------------------------------------------------------------
int ShowShuttleError(int pos, int KCode)
{
    AnsiString PanelName;

    if(pos==0)
        PanelName="pn_InShuttle";
    else
        PanelName="pn_OutShuttle";
    return ShowNoteAlarm(AnsiString().sprintf("SHT%04d", pos), "Shuttle Error", "", KCode, PanelName);
}
//---------------------------------------------------------------------------
int ShowJamError(int iJamSensor, int KCode)
{
    return ShowNoteAlarm(AnsiString().sprintf("JAM%04d", iJamSensor), "Jam Error", "", KCode, "pn_System");
}
//---------------------------------------------------------------------------
int ShowMagazineError(int iMagazine, int CodeType, int KCode)
{
    return ShowNoteAlarm(AnsiString().sprintf("MAG%02d%02d", iMagazine, CodeType), "Magazine Error", "HT160S_BCB keeps this API for old-source compatibility.", KCode, "pn_System");
}
//---------------------------------------------------------------------------
int ShowSystemCommError(int CodeType, int KCode, AnsiString Note)
{
    return ShowNoteAlarm(AnsiString().sprintf("COM%04d", CodeType), "System Communication Error", Note, KCode, "pn_System");
}
//---------------------------------------------------------------------------
int ShowCCDError(int CodeType, int KCode, AnsiString Note)
{
    return ShowNoteAlarm(AnsiString().sprintf("CCD%04d", CodeType), "CCD Error", Note, KCode, "pn_System");
}
//---------------------------------------------------------------------------
int ShowMyError(AnsiString sMyError, int KCode)
{
    return ShowNoteAlarm(sMyError, sMyError, "", KCode, "pn_System");
}
//---------------------------------------------------------------------------
int ShowTNTError(int CodeType, int KCode)
{
    return ShowNoteAlarm(AnsiString().sprintf("TNT%04d", CodeType), "TNT Error", "", KCode, "pn_TempAndTest");
}
//---------------------------------------------------------------------------
void ShowErrorMessage(AnsiString Code)
{
    ShowNoteAlarm(Code, Code, "", K_RETRY, "pn_System");
}
//---------------------------------------------------------------------------
void RecordProcess(AnsiString S)
{
    static AnsiString LastRecord="";

    if(S=="")
        return;
    if(S==LastRecord && S.Pos(" pressed")!=0)
        return;

    LastRecord=S;
    // Operator / process action: AlarmCode="PROCESS", Message=S.
    g_EventLog.Log("PROCESS", S, "");
    if(fNote!=NULL && fNote->Memo1!=NULL)
        fNote->Memo1->Lines->Add(FormatDateTime("hh:nn:ss", Now())+AnsiString(" ")+S);
}
//---------------------------------------------------------------------------
void LevelRecordProcess()
{
}
//---------------------------------------------------------------------------
void SearchMessage(AnsiString Code)
{
    RecordProcess(AnsiString("SearchMessage:")+Code);
}
//---------------------------------------------------------------------------
void RecordAlarmMessagePassTime(AnsiString AlarmCode, DWORD StartTime, AnsiString HappenTime, int Type)
{
    // Alarm pass / elapsed-time record routed to EventLog.
    g_EventLog.Log(AlarmCode, AnsiString("PASS ")+HappenTime, AnsiString("TYPE=")+IntToStr(Type));
}
//---------------------------------------------------------------------------
bool CheckAlarmIsShow()
{
    return true;
}
//---------------------------------------------------------------------------
void SetShowAlarmLocation(int iType, int iPosition)
{
}
//---------------------------------------------------------------------------
void SetShowSuckerLocation(AnsiString sSuckerName)
{
}
//---------------------------------------------------------------------------
void ShowImageTrayFuntion()
{
}
//---------------------------------------------------------------------------
AnsiString GetRefrenceCode(AnsiString S)
{
    return "No Code";
}
//---------------------------------------------------------------------------
