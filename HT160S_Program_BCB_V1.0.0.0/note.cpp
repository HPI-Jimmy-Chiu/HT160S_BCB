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
//AI(ht160s-maintainer) 20260627 : Note recovery-lamp blink heartbeat (HT172 FlushFlag
//analog). Toggled every Timer1 tick (250ms) while the modal Note is up; FlushLabel
//blinks each OFFERED recovery key's panel LED on the ON phase, solid when SELECTED.
static bool NoteBlinkPhase=false;
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
    ManualText="";
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

    //AI(ht160s-maintainer) 20260627 : do NOT set the recovery bLamp* here. While this
    //Note is shown it is modal (ShowModal), so MainProc -> DoSystemMessage -> DoPanelLamp
    //is suspended and a set-once here never reaches the Pad LEDs (that was the bug).
    //FlushLabel(), pumped from Timer1Timer every 250ms tick, derives the lamps from the
    //offered (Visible) / selected (Select[]) keys instead -- matches HT172 note.cpp.
    if((KeyCode & K_MANUAL_2D)!=0)
    {
        edtManual2D->Visible=true;
        edtManual2D->Text="";
        try { edtManual2D->SetFocus(); } catch(...) {}
    }
    else
        edtManual2D->Visible=false;

    if(FlushPanel!=NULL)
        FlushPanelColor=FlushPanel->Color;

    //AI(HT160S-Maintainer) 20260622 : the alarm Note is modal and suspends MainProc, so the
    //per-scan DoSystemMessage LED_ErrJam buzzer driver never runs while it is up. Kick it here
    //so an alarm is audible the instant the screen appears (honours the OFF BUZZER latch).
    if(bOffBuzzer==false)
        PlayAlarmBuzzer();
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
    //AI(ht160s-panel-sensitivity) 20260706 : Timer1 lowered 250ms->10ms (note.dfm) so
    //physical operator-panel keys are scanned responsively while this modal Note
    //suspends MainProc (HT172 note Timer1 is 1ms). ScanKey/FlushLabel/DoSystemMessage
    //are per-scan idempotent (normal MainProc runs them far more often than 10ms), but
    //the recovery-key + FlushPanel blink must stay ~2 Hz, so advance the blink phase
    //only every 25 ticks (25 x 10ms = 250ms, the original cadence).
    static int iBlinkDiv=0;

    if(fShow==false)
        return;

    //AI(HT160S-Maintainer) 20260619 : physical operator-panel keys on the alarm
    //screen (HT172 note ScanKey port). May Close() the form on Start/Pause, so
    //re-check fShow before the blink work below.
    ScanKey();
    if(fShow==false)
        return;

    //AI(ht160s-maintainer) 20260627 : keep the front-panel recovery-key LEDs alive while
    //this modal Note suspends MainProc (same reason as the FormShow PlayAlarmBuzzer kick).
    //Toggle the blink heartbeat, recompute bLamp* from the offered/selected keys, then
    //DoSystemMessage() pushes them to the Pad (it is the sole DoPanelLamp caller and also
    //refreshes Start/Pause + tower light + buzzer). HT172 note.cpp Timer1 does the same
    //(FlushLabel() then DoSystemMessage()).
    //AI(ht160s-panel-sensitivity) 20260706 : ~250ms blink heartbeat, decoupled from
    //the 10ms scan tick (advance both blink phases together every 25 ticks).
    iBlinkDiv++;
    if(iBlinkDiv>=25)
    {
        iBlinkDiv=0;
        NoteBlinkPhase=!NoteBlinkPhase;
        Blink=!Blink;
    }

    FlushLabel();
    DoSystemMessage();

    if(bOffBuzzer)
        CloseBuzzerOff();
    if(FlushPanel==NULL)
        return;

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
void __fastcall TfNote::edtManual2DKeyPress(TObject *Sender, char &Key)
{
    //AI(ht160s-ccd-manual2d) : operator hand-enters / handheld-scans the 2D code.
    //A trailing CR (scanner terminator or Enter) commits: capture text, set the
    //manual return code, log it, then close. Does NOT resume the machine -- only the
    //Start button runs it (operator boundary).
    if(Key==13)
    {
        Key=0;
        if(edtManual2D->Text.Trim()=="")
            return;
        ManualText=edtManual2D->Text.Trim();
        ReturnCode=K_MANUAL_2D;
        RecordProcess(AnsiString("MANUAL 2D entered : ")+ManualText);
        Close();
    }
}
//---------------------------------------------------------------------------
void __fastcall TfNote::BtnStartClick(TObject *Sender)
{
    //AI(HT160S-Maintainer) 20260623 : align with HT172/HT9045 BtnStartClick+Start
    //gating. START (resume) only dismisses the alarm when the operator has
    //selected one of the offered recovery keys, OR when the note offers no
    //recovery keys at all (KeyCode==0, a pure informational message). If recovery
    //keys ARE offered but none is selected, START does nothing so the operator
    //cannot resume past an unaddressed alarm. Replaces the old unconditional
    //Close() that let any alarm be cleared by a single key press.
    int Index;

    for(Index=0; Index<6; Index++)
    {
        if(Select[Index])
        {
            SoftStop=false;
            SoftStart=true;
            RecordProcess("START pressed");
            Close();
            return;
        }
    }

    if(KeyCode==0)
    {
        SoftStop=false;
        SoftStart=true;
        RecordProcess("START pressed");
        Close();
    }
}
//---------------------------------------------------------------------------
void __fastcall TfNote::BtnPauseClick(TObject *Sender)
{
    //AI(HT160S-Maintainer) 20260623 : align with HT172/HT9045 BtnPauseClick
    //gating. PAUSE (stop) only dismisses the alarm when the operator has selected
    //one of the offered recovery keys, OR when the note offers no recovery keys
    //(KeyCode==0). If recovery keys ARE offered but none is selected, PAUSE does
    //nothing -- the operator MUST pick a recovery action first. Replaces the old
    //unconditional Close() that let PAUSE clear every alarm (non-compliant).
    int Index;

    for(Index=0; Index<6; Index++)
    {
        if(Select[Index])
        {
            SoftStart=false;
            SoftStop=true;
            HSys.DecStopAllMotor();
            RecordProcess("PAUSE pressed");
            Close();
            return;
        }
    }

    if(KeyCode==0)
    {
        SoftStart=false;
        SoftStop=true;
        HSys.DecStopAllMotor();
        RecordProcess("PAUSE pressed");
        Close();
    }
}
//---------------------------------------------------------------------------
void __fastcall TfNote::BtnOffBuzzerClick(TObject *Sender)
{
    bOffBuzzer=true;
    CloseBuzzerOff();
    //AI(secs-kyec-rcmd4) 20260728 : acknowledging the buzzer also releases any SECS host
    //panel override (S2F41 PP_MUSIC / PP_SIGNALTOWER), matching HT9045 which clears the pair
    //from message-box / note acknowledge. Without it, Timer1 re-drives DoSystemMessage()
    //100x/s and the override would come straight back.
    ClearSecsPanelOverride();
    RecordProcess("OFF BUZZER pressed");
}
//---------------------------------------------------------------------------
void __fastcall TfNote::FlushLabel()
{
    //AI(ht160s-maintainer) 20260627 : HT172 note.cpp FlushLabel port (reduced to the 5
    //recovery keys that have a physical SwFK* LED -- Home/Manual2D have no panel LED, so
    //they are not listed, exactly as HT172 omits SwRKHome from DoPanelLamp). Index order
    //matches FormShow KeyComp / UpdateButtonStatus Select[] {SKIP,RETRY,TRAY_FEED,TRAY_END,
    //CLEAN_OUT}. A SELECTED key is solid; an OFFERED-but-not-selected (Visible) key blinks
    //on the NoteBlinkPhase ON half. Pushed to the Pad by the DoSystemMessage()->DoPanelLamp()
    //call that follows in Timer1Timer.
    TPanel *Ptr[5]={BtnSkip, BtnRetry, BtnTrayFeed, BtnTrayEnd, BtnCleanOut};
    bool   *bPtr[5]={&bLampSkip, &bLampRetry, &bLampTrayFeed, &bLampTrayEnd, &bLampCleanOut};
    int i;

    for(i=0; i<5; i++)
    {
        if(Select[i])
            *bPtr[i]=true;
        else if(NoteBlinkPhase && Ptr[i]->Visible)
            *bPtr[i]=true;
        else
            *bPtr[i]=false;
    }

    //AI(HT160S-Maintainer) 20260701 : HT172 note.cpp FlushLabel Start/Pause "invitation
    //blink" (HT172 lines 298-317). Once the operator has selected an offered recovery key,
    //or when the alarm offers no recovery keys at all (KeyCode==0, pure acknowledge), blink
    //the physical Start AND Pause LEDs on the NoteBlinkPhase ON half to cue "press Start to
    //resume". DoSystemMessage yields bLampStart/bLampPause to this while fNote->fShow (see
    //csystem.cpp), so these values reach the Pad via the DoPanelLamp() call that follows in
    //Timer1Timer. HT160 buttons are TPanel (no on-screen FalseColor blink) -- panel LED only.
    bool bAnySelected=false;
    for(i=0; i<6; i++)
    {
        if(Select[i])
            bAnySelected=true;
    }
    if((bAnySelected || KeyCode==0) && NoteBlinkPhase)
    {
        bLampStart=true;
        bLampPause=true;
    }
    else
    {
        bLampStart=false;
        bLampPause=false;
    }
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
        {
            UpdateButtonStatus(Ptr[i]);
            //AI(HT160S-Maintainer) 20260701 : selecting any offered recovery key from the
            //physical panel also acknowledges the alarm buzzer, matching HT172 note.cpp:211
            //and HT9045 note.cpp:1422 (both clear bAlarmBuzzer in this same key loop). The
            //next Timer1 DoSystemMessage() then keeps LED_ErrJam muted; FormClose resets
            //bOffBuzzer so the next alarm sounds again. Touch-screen selection is unchanged.
            bOffBuzzer=true;
        }
    }

    bool bStart=HSys.Sen.SnFKStart.IsOn()      || HSys.Sen.SnRKStart.IsOn();
    bool bPause=HSys.Sen.SnFKPause.IsOn()      || HSys.Sen.SnRKPause.IsOn();
    bool bReset=HSys.Sen.SnFKAlarmReset.IsOn() || HSys.Sen.SnRKAlarmReset.IsOn();

    if(bStart && bWasStart==false)
    {
        EventReport(SECS_EVENT.DoStart);
        BtnStartClick(this);
    }
    else if(bPause && bWasPause==false)
    {
        EventReport(SECS_EVENT.DoPause);
        BtnPauseClick(this);
    }
    else if(bKey[0] && bWasSkip==false)
    {
        RecordProcess("SKIP pressed");
        EventReport(SECS_EVENT.DoSkip);
    }
    else if(bKey[1] && bWasRetry==false)
    {
        RecordProcess("RETRY pressed");
        EventReport(SECS_EVENT.DoRetry);
    }
    else if(bKey[2] && bWasTrayFeed==false)
    {
        RecordProcess("TRAY FEED pressed");
        EventReport(SECS_EVENT.DoTrayFeed);
    }
    else if(bKey[3] && bWasTrayEnd==false)
    {
        RecordProcess("TRAY END pressed");
        //AI(secs-ceid-align9045) 20260729 : the TRAY END key used to report DoTrayFeed, i.e.
        //the exact same CEID as the TRAY FEED key one branch up, so the host could not tell
        //the two panel keys apart. HT9045 has a dedicated Tray End event (CEID 31 DoTrayEnd,
        //note.cpp DoTrayEnd) - use it. Panel behaviour is unchanged; only the reported id is.
        EventReport(SECS_EVENT.DoTrayEnd);
    }
    else if(bKey[4] && bWasCleanOut==false)
    {
        RecordProcess("CLEAN OUT pressed");
        EventReport(SECS_EVENT.DoCleanOut);
    }
    else if(bReset && bWasReset==false)
    {
        RecordProcess("ALARM RESET pressed");
        EventReport(SECS_EVENT.DoAlarmReset);
        //AI(HT160S-Maintainer) 20260701 : latch the OFF BUZZER acknowledge (HT172
        //note.cpp bAlarmBuzzer=false / HT9045 note AlarmReset parity) so the per-scan
        //DoSystemMessage LED_ErrJam driver stays muted. Without this latch the Timer1
        //DoSystemMessage() call that runs right after ScanKey() re-drove the buzzer the
        //same 250ms tick, so the physical ALARM RESET key appeared to do nothing. The
        //on-screen Off Buzzer button already worked because it sets bOffBuzzer=true.
        bOffBuzzer=true;
        CloseBuzzerOff();
        //AI(secs-kyec-rcmd4) 20260728 : the panel ALARM RESET key also releases any SECS host
        //panel override (S2F41 PP_MUSIC / PP_SIGNALTOWER). This is HT9045's primary release
        //site ported. The main-screen twin lives in TfMain::ScanPanelKeys for the no-dialog
        //case, which HT9045 covers and HT160 did not.
        ClearSecsPanelOverride();
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
    {
        //AI(ht160s-obsv-p0) 20260720 : a second alarm raised behind an open Note modal was
        //silently discarded (no EventLog, no SECS) - offline timelines then miss a real
        //alarm. Record the drop; single-modal behavior unchanged.
        RecordProcess("ALARM DROPPED (modal busy): "+Code+" "+Message);
        return 0;
    }

    HSys.DecStopAllMotor();
    HSys.Sys.SystemStart=false;
    SoftStop=false;
    SoftStart=false;

    fNote->Reset();
    fNote->KeyCode=KCode;
    fNote->ManualText="";
    fNote->edtAlarmCode->Text=Code;
    fNote->edtAlarmMsg->Text=Message;
    fNote->Memo1->Clear();
    if(Detail!="")
        fNote->Memo1->Lines->Add(Detail);

    if(FlushPanelName!="")
        fNote->GetFlushPanel(fNote->PanelMain6, FlushPanelName);
    if(fNote->FlushPanel==NULL)
        fNote->FlushPanel=fNote->pn_System;

    //AI(ht160s-obsv-p2) 20260720 : persist the Detail (sensor expect/actual/IO line) in the
    //EventLog Message column - it previously lived only in the on-screen Memo.
    fNote->ProcessErrMessage(Code, (Detail!="") ? (Message+" | "+Detail) : Message, 1);
    AlarmReport(Code, Message, true);    //AI(ht160s-secsgem) 20260625 : S5F1 alarm set
    DWORD dwPauseStart=GetTickCount();   //AI(HT160S-Maintainer) 20260626 : measure operator pause
    fNote->ShowModal();
    int iPauseSec=(int)((GetTickCount()-dwPauseStart)/1000);
    AlarmReport(Code, Message, false);   //AI(ht160s-secsgem) 20260625 : S5F1 alarm clear (operator handled)

    //AI(HT160S-Maintainer) 20260626 : persist the operator recovery decision + pause time
    //into the EventLog (Recovery/PauseTime columns) so a shipped log shows how each alarm
    //was cleared, aligning post-mortem with HT172/HT9045. ReturnCode holds one K_ bit.
    AnsiString sRecovery;
    switch(fNote->ReturnCode)
    {
        case K_SKIP:      sRecovery="SKIP";      break;
        case K_RETRY:     sRecovery="RETRY";     break;
        case K_TRAY_FEED: sRecovery="TRAY_FEED"; break;
        case K_TRAY_END:  sRecovery="TRAY_END";  break;
        case K_CLEAN_OUT: sRecovery="CLEAN_OUT"; break;
        case K_HOME:      sRecovery="HOME";      break;
        case K_MANUAL_2D: sRecovery="MANUAL_2D"; break;
        default:          sRecovery="START";     break;
    }
    g_EventLog.LogRecovery(sRecovery, iPauseSec, Code, Message);
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
//AI(ht160s-maintainer) 20260625 : motor soft-limit alarm via the full Note panel
//(code + message + numeric detail line), mirroring HT9045 ShowErrorMessage("WAR0154",
//.., "X=..") for the In-Arm X limit. Detail carries the explicit target / soft-limit
//values. RETRY lets the operator correct teach / re-home and resume.
int ShowMotorLimitError(AnsiString Code, AnsiString Message, AnsiString Detail)
{
    return ShowNoteAlarm(Code, Message, Detail, K_RETRY, "pn_System");
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260630 : motor-aware overload. Builds the Note Detail from
//the motor NumberAlias ("[M0x] <Alias>", the Motor-view token) plus SoftLimitDetail
//(target/now/limit), so a soft-limit alarm names the axis to open on the Motor view.
//Caller passes the per-motor registered over-limit code pMot->AlarmName[eMotOverLimitErr].
//NULL pMot -> empty Detail. ASCII; no C++11; AnsiString flows.
int ShowMotorLimitError(AnsiString Code, AnsiString Message, TMyMotor *pMot, int p)
{
    AnsiString Detail;
    if(pMot!=NULL)
        Detail=pMot->NumberAlias+AnsiString("  ")+pMot->SoftLimitDetail(p);
    return ShowNoteAlarm(Code, Message, Detail, K_RETRY, "pn_System");
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
//AI(HT160S-Maintainer) 20260626 : code-carrying overload. Aligns the alarm to a
//HT9045 code string (WAR/JAM/MES) shown to the operator and sent as the SECS AlarmID,
//while sMyError keeps the human-readable detail. Legacy 1-arg form (string as both
//code and message) is preserved for not-yet-migrated callers.
int ShowMyError(AnsiString Code, AnsiString sMyError, int KCode)
{
    return ShowNoteAlarm(Code, sMyError, "", KCode, "pn_System");
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260630 : Detail-carrying overload. Same as the
//Code+message form but forwards a Detail string (e.g. a TriggerLine IO token)
//to ShowNoteAlarm instead of hard-coding "". Code stays byte-stable (SECS ALID
//is hashed from Code only); the IO context rides in Detail.
int ShowMyError(AnsiString Code, AnsiString sMyError, AnsiString Detail, int KCode)
{
    return ShowNoteAlarm(Code, sMyError, Detail, KCode, "pn_System");
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260630 : device-typed trigger-line token. Turns a sensor
//into a canonical string naming its IOsetview Alias (TMySensor.Name), its address
//(Card/Lane/IP/Port/Bit via the bound TMyIo) and expected-vs-actual state, so an
//alarm Detail can point the operator straight at the IO point. NULL Input prints
//addr(unbound). ASCII-only; no C++11; AnsiString flows. Non-const ref because
//IsOn()/GetLane() etc are non-const members.
AnsiString TriggerLine(TMySensor &Sn, bool bExpectedOn)
{
    AnsiString Addr;
    if(Sn.Input==NULL)
        Addr="addr(unbound)";
    else
        Addr=AnsiString().sprintf("addr(Card=%d Lane=%d IP=%d Port=%d Bit=%d)",
                                  Sn.Input->GetCard(), Sn.Input->GetLane(),
                                  Sn.Input->GetIP(), Sn.Input->GetPort(),
                                  Sn.Input->GetBit());
    AnsiString Expect;
    if(bExpectedOn)
        Expect="ON";
    else
        Expect="OFF";
    AnsiString Actual;
    if(Sn.IsOn())
        Actual="ON";
    else
        Actual="OFF";
    return Sn.Name + " expect=" + Expect + " actual=" + Actual + " " + Addr;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260630 : sensor-aware overload. Injects a SHORT IO tag
//[IO=<Alias>] into the persisted Message (EventLog + SECS ALTX) AND the FULL
//TriggerLine into the on-screen Detail, from one TMySensor pointer. NULL pSn ->
//no tag (safe for accessor-returned pointers). Code unchanged -> SECS ALID stable.
int ShowMyError(AnsiString Code, AnsiString Msg, TMySensor *pSn, bool bExpectedOn, int KCode)
{
    if(pSn==NULL)
        return ShowMyError(Code, Msg, KCode);
    AnsiString Full=TriggerLine(*pSn, bExpectedOn);
    AnsiString Msg2=Msg+" [IO="+pSn->Name+"]";
    return ShowMyError(Code, Msg2, Full, KCode);
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
