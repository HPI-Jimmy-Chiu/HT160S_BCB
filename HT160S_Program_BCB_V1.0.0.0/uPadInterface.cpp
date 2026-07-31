//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
#include "language.h"
//---------------------------------------------------------------------------
#include "uPadInterface.h"
#include "database.h"
#include "cCommLog.h"
#include "HTimer.h"
#include "maintenance.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "SPComm"
#pragma link "MyLed"
#pragma link "BtnPanelLane"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfPadInterface *fPadInterface = NULL;
//---------------------------------------------------------------------------
static AnsiString NormalizePadInputName(AnsiString InputName)
{
    if(InputName==AnsiString("SnRKTray"))
        return AnsiString("SnRKTrayEnd");
    return InputName;
}
//---------------------------------------------------------------------------
static void SyncHSysPadInputStatus(AnsiString InputName, bool State)
{
    AnsiString NormalName;

    NormalName=NormalizePadInputName(InputName);
    for(int Index=0; Index<HSys.iTotalSensor; Index++)
    {
        if(HSys.SenPtr[Index].Name==NormalName || HSys.SenPtr[Index].Name==InputName ||
           (NormalName==AnsiString("SnRKTrayEnd") && HSys.SenPtr[Index].Name==AnsiString("SnRKTray")))
            HSys.SenPtr[Index].iStatus=State?1:0;
    }
}
//---------------------------------------------------------------------------
static void SyncHSysPadSwitchStatus(AnsiString SwitchName, bool State)
{
    for(int Index=0; Index<HSys.iTotalSwitch; Index++)
    {
        if(HSys.SwPtr[Index].Name==SwitchName)
        {
            HSys.SwPtr[Index].OutValue=State;
            HSys.SwPtr[Index].SetValue=State;
        }
    }
}
//---------------------------------------------------------------------------
struct TPadButtonDef
{
    const char *Caption;
    const char *PadName;
    const char *InputName;
    int Data;
    int PanelTag;
};
//---------------------------------------------------------------------------
static const TPadButtonDef PadButtonDefs[] = {
    {"Power Off",   "SwFKPowerOff",      "SnFKPowerOff",       PAD_PowerOff,      0},
    {"Power On",    "SwFKPowerOn",       "SnFKPowerOn",        PAD_PowerOn,       0},
    {"Front Enable","SwFrontActiveLed",  "SnFrontPadActive",   PAD_PannelEnable,  0},
    {"Reset",       "SwFKReset",         "SnFKReset",          PAD_Reset,         0},
    {"Pause",       "SwFKPause",         "SnFKPause",          PAD_Pause,         0},
    {"Home",        "SwFKHome",          "SnFKHome",           PAD_Home,          0},
    {"Start",       "SwFKStart",         "SnFKStart",          PAD_Start,         0},
    {"One Cycle",   "SwFKOneCycle",      "SnFKOneCycle",       PAD_OneCycle,      0},
    {"Retry",       "SwFKRetry",         "SnFKRetry",          PAD_Retry,         0},
    {"Skip",        "SwFKSkip",          "SnFKSkip",           PAD_Skip,          0},
    {"Clean Out",   "SwFKCleanOut",      "SnFKCleanOut",       PAD_CleanOut,      0},
    {"Tray Feed",   "SwFKTrayFeed",      "SnFKTrayFeed",       PAD_TrayFeed,      0},
    {"Tray End",    "SwFKTrayEnd",       "SnFKTrayEnd",        PAD_TrayEnd,       0},
    {"Alarm Reset", "SwFKAlarmReset",    "SnFKAlarmReset",     PAD_AlarmReset,    0},
    {"Power Off",   "SwRKPowerOff",      "SnRKPowerOff",       PAD_PowerOff,      1},
    {"Power On",    "SwRKPowerOn",       "SnRKPowerOn",        PAD_PowerOn,       1},
    {"Reset",       "SwRKReset",         "SnRKReset",          PAD_Reset,         1},
    {"Pause",       "SwRKPause",         "SnRKPause",          PAD_Pause,         1},
    {"Home",        "SwRKHome",          "SnRKHome",           PAD_Home,          1},
    {"Start",       "SwRKStart",         "SnRKStart",          PAD_Start,         1},
    {"One Cycle",   "SwRKOneCycle",      "SnRKOneCycle",       PAD_OneCycle,      1},
    {"Retry",       "SwRKRetry",         "SnRKRetry",          PAD_Retry,         1},
    {"Skip",        "SwRKSkip",          "SnRKSkip",           PAD_Skip,          1},
    {"Clean Out",   "SwRKCleanOut",      "SnRKCleanOut",       PAD_CleanOut,      1},
    {"Tray Feed",   "SwRKTrayFeed",      "SnRKTrayFeed",       PAD_TrayFeed,      1},
    {"Tray End",    "SwRKTrayEnd",       "SnRKTrayEnd",        PAD_TrayEnd,       1},
    {"Alarm Reset", "SwRKAlarmReset",    "SnRKAlarmReset",     PAD_AlarmReset,    1},
    {"Safe Lock",   "SwRKSafeLock",      "SnRKSafeLock",       PAD_SafeLock,      1},
    {"Step",        "SwRKManualStep",    "SnRKManualStep",     PAD_Step,          1},
    {"T Start",     "SwRKManualTStart",  "SnRKManualTStart",   PAD_TStart,        1},
    {"Rear Enable", "SwRearActiveLed",   "SnRearPadActive",    PAD_PannelEnable,  1}
};
//---------------------------------------------------------------------------
void PAD_PTR::SetItem(TMyLed *_mlEvent, TBtnPanelLane *_btnEvent, AnsiString _PadName, int _iData, AnsiString _InputName)
{
    mlEvent=_mlEvent;
    btnEvent=_btnEvent;
    PadName=_PadName;
    iData=_iData;
    InputName=_InputName;
    if(btnEvent!=NULL)
        btnEvent->Enabled=true;
}
//---------------------------------------------------------------------------
__fastcall TfPadInterface::TfPadInterface(TComponent* Owner)
    : TForm(Owner)
{
    InitialVariable();
    BuildUI();
}
//---------------------------------------------------------------------------
__fastcall TfPadInterface::~TfPadInterface()
{
    delete CommReceiveList;
    delete CommReceiveLength;
    delete CommSendList;
    delete CommSendLength;
}
//---------------------------------------------------------------------------
void TfPadInterface::InitialVariable()
{
    int i;

    PadComm=NULL;
    PadComPort="";
    bRs232Ok=false;
    bShow=false;
    bRequestVer=false;
    bScanSwitch=true;
    bSendSwitchStatusing=false;
    CheckPadItem=sizeof(PadButtonDefs)/sizeof(PadButtonDefs[0]);

    CommReceiveList=new TStringList();
    CommReceiveLength=new TStringList();
    CommSendList=new TStringList();
    CommSendLength=new TStringList();

    for(i=0; i<32; i++)
    {
        PadItem[i].mlEvent=NULL;
        PadItem[i].btnEvent=NULL;
        PadItem[i].PadName="";
        PadItem[i].InputName="";
        PadItem[i].iData=0;
        bPadInputStatus[i]=false;
        bPadStatus[i]=false;
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260731 : static skeleton moved to uPadInterface.dfm
//(DFM-first convention). Only the runtime language pass (LangT) and the
//table-driven pad grid (PadButtonDefs -> PadItem) stay code-built.
void TfPadInterface::BuildUI()
{
    Caption=LangT("Pad Interface");
    pn_PadInterfaceTitle->Caption=LangT("Pad Interface");
    sb_PadInterface_Exit->Caption=LangT("Exit");
    lb_PadInterface_ManualSend->Caption=LangT("Manual Send");
    sb_PadInterface_ManualSend->Caption=LangT("Send");
    btnResetCom->Caption=LangT("Reset COM");
    btnClearLog->Caption=LangT("Clear Log");
    cb_PadInterface_PadLedBling->Caption=LangT("Blink LED");
    tsPadFront->Caption=LangT("Front Pad");
    tsPadRear->Caption=LangT("Rear Pad");

    BuildPadPage(pn_PadInterface_Front, 0, 14);
    BuildPadPage(pn_PadInterface_Rear, 14, 17);
}
//---------------------------------------------------------------------------
void TfPadInterface::BuildPadPage(TPanel *ParentPanel, int BaseIndex, int Count)
{
    int i;
    int Col;
    int Row;

    for(i=0; i<Count; i++)
    {
        Col=i/8;
        Row=i%8;
        AddPadItem(ParentPanel, BaseIndex+i, Row, Col);
    }
}
//---------------------------------------------------------------------------
void TfPadInterface::AddPadItem(TPanel *ParentPanel, int Index, int Row, int Col)
{
    TPanel *ItemPanel;
    TMyLed *LedPtr;
    TBtnPanelLane *BtnPtr;
    int LeftBase;
    int TopBase;

    if(Index<0 || Index>=CheckPadItem)
        return;

    //AI(ht160s-maintainer) 20260731 : the grid has to fit the tab-sheet client
    //(816x353 at this form size) without scrollbars, with slack for theme/DPI
    //variation in the caption and tab-strip heights. 8 rows reach
    //18+7*40+32 = 330 and the rear page's 3rd column reaches 18+2*260+250 = 788.
    //The old 48/390 pitch overflowed both, hiding front One Cycle and rear
    //Retry / T Start / Rear Enable. Item content only reaches x=174, so the
    //narrower cell costs nothing.
    LeftBase=18+(Col*260);
    TopBase=18+(Row*40);

    ItemPanel=new TPanel(this);
    ItemPanel->Parent=ParentPanel;
    ItemPanel->Left=LeftBase;
    ItemPanel->Top=TopBase;
    ItemPanel->Width=250;
    ItemPanel->Height=32;
    ItemPanel->BevelOuter=bvLowered;

    LedPtr=new TMyLed(this);
    LedPtr->Parent=ItemPanel;
    LedPtr->Left=8;
    LedPtr->Top=8;
    LedPtr->Width=26;
    LedPtr->Height=16;
    LedPtr->Alias=PadButtonDefs[Index].InputName;
    LedPtr->Tag=PadButtonDefs[Index].PanelTag;
    LedPtr->Value=false;

    BtnPtr=new TBtnPanelLane(this);
    BtnPtr->Parent=ItemPanel;
    BtnPtr->Left=44;
    BtnPtr->Top=4;
    BtnPtr->Width=130;
    BtnPtr->Height=24;
    BtnPtr->Caption=LangT(PadButtonDefs[Index].Caption);
    BtnPtr->Alias=PadButtonDefs[Index].PadName;
    BtnPtr->Tag=PadButtonDefs[Index].PanelTag;
    BtnPtr->FalseColor=clBtnFace;
    BtnPtr->FalseFontColor=clBlack;
    BtnPtr->TrueColor=clLime;
    BtnPtr->TrueFontColor=clBlack;
    BtnPtr->Down=false;
    BtnPtr->OnMouseDown=PadButtonMouseDown;

    PadItem[Index].SetItem(LedPtr, BtnPtr, PadButtonDefs[Index].PadName, PadButtonDefs[Index].Data, PadButtonDefs[Index].InputName);
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::FormShow(TObject *Sender)
{
    (void)Sender;
    Left=(1280-Width)/2;
    Top=(1024-Height)/2;
    bShow=true;
    //AI(ht160s-maintainer) 20260620 : align to HT172 - clear stale button-down /
    //LED visual state on (re)show so the panel starts from a clean display.
    {
        int i;
        for(i=0; i<CheckPadItem; i++)
        {
            if(PadItem[i].btnEvent!=NULL)
                PadItem[i].btnEvent->Down=false;
            if(PadItem[i].mlEvent!=NULL)
                PadItem[i].mlEvent->Value=false;
        }
    }
    if(bRs232Ok)
    {
        SendCommand("t050490000000");
        SendCommand("t050491000000");
        SendCommand("t051490000000");
        SendCommand("t051491000000");
    }
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::FormClose(TObject *Sender, TCloseAction &Action)
{
    int i;

    (void)Sender;
    (void)Action;
    bShow=false;
    for(i=0; i<32; i++)
        bPadStatus[i]=false;
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::sb_PadInterface_ExitClick(TObject *Sender)
{
    (void)Sender;
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::PadButtonMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    TBtnPanelLane *BtnPanelLane;

    (void)Button;
    (void)Shift;
    (void)X;
    (void)Y;

    BtnPanelLane=dynamic_cast<TBtnPanelLane *>(Sender);
    if(BtnPanelLane==NULL)
        return;

    BtnPanelLane->Down=!BtnPanelLane->Down;
    PadButtonClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::PadButtonClick(TObject *Sender)
{
    TBtnPanelLane *BtnPanelLane;

    BtnPanelLane=dynamic_cast<TBtnPanelLane *>(Sender);
    if(BtnPanelLane==NULL)
        return;

    SendSwitchStatus(BtnPanelLane);
}
//---------------------------------------------------------------------------
void TfPadInterface::RecordLocalStatusCommand(int iAddress)
{
    AnsiString sData;
    int i;
    int iStatus;

    iStatus=0;
    for(i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].btnEvent!=NULL && PadItem[i].btnEvent->Tag==iAddress && PadItem[i].btnEvent->Down)
            iStatus|=PadItem[i].iData;
    }

    sData="t05";
    sData+=IntToHex(iAddress, 1);
    sData+=IntToHex(PAD_ControlDLC, 1);
    sData+="9";
    sData+=cb_PadInterface_PadLedBling->Checked?IntToHex(PAD_LedBling, 1):IntToHex(PAD_LedLight, 1);
    sData+=IntToHex(iStatus, 6);
    SendCommand(sData);
}
//---------------------------------------------------------------------------
bool TfPadInterface::IsPadButton(AnsiString aName)
{
    int i;

    if(aName=="")
        return false;

    for(i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].PadName==aName)
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TfPadInterface::IsPadKey(AnsiString aName)
{
    int i;

    if(aName=="")
        return false;

    aName=NormalizePadInputName(aName);

    for(i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].InputName==aName)
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TfPadInterface::GetPadSwitchStatus(AnsiString aName, bool *State)
{
    int i;

    if(State!=NULL)
        *State=false;
    if(aName=="")
        return false;

    for(i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].PadName==aName)
        {
            if(State!=NULL)
                *State=bPadStatus[i];
            return true;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
bool TfPadInterface::OpenCommPort()
{
    //AI(ht160s-maintainer) 20260620 : align to HT172 - idempotency guard so an
    //already-open port is not re-StartComm'd.
    if(bRs232Ok)
        return true;
    if(PadComm==NULL)
    {
        RecordCommunication("[Connect]", "PadComm is not assigned");
        bRs232Ok=false;
        return false;
    }

    try
    {
        PadComm->StartComm();
        bRs232Ok=true;
        RecordCommunication("[Connect]", "OK");
    }
    catch(...)
    {
        bRs232Ok=false;
        RecordCommunication("[Connect]", "FAIL");
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool TfPadInterface::CloseCommPort()
{
    if(PadComm==NULL)
        return false;

    try
    {
        PadComm->StopComm();
        bRs232Ok=false;
        RecordCommunication("[Disconnect]", "OK");
    }
    catch(...)
    {
        RecordCommunication("[Disconnect]", "FAIL");
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::ResetComm()
{
    CloseCommPort();
    OpenCommPort();
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::sb_PadInterface_ManualSendClick(TObject *Sender)
{
    if(Sender==btnResetCom)
        ResetComm();
    else
        SendCommand(ed_PadInterface_ManualSend->Text);
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::ClearLog1Click(TObject *Sender)
{
    (void)Sender;
    Memo_PadInterface->Lines->Clear();
}
//---------------------------------------------------------------------------
void TfPadInterface::RequestPadVersion()
{
    bRequestVer=false;
    SendCommand("t051120");
}
//---------------------------------------------------------------------------
void TfPadInterface::SendSwitchStatus(TBtnPanelLane *bpPtr)
{
    int i;

    if(bpPtr==NULL)
        return;

    for(i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].PadName==bpPtr->Alias)
        {
            bPadStatus[i]=bpPtr->Down;
            SyncHSysPadSwitchStatus(PadItem[i].PadName, bPadStatus[i]);
        }
    }

    RecordLocalStatusCommand((bpPtr->Tag==1)?PAD_RearControl:PAD_FrontControl);
}
//---------------------------------------------------------------------------
void TfPadInterface::SendSwitchStatus(AnsiString aName, bool Type)
{
    int i;

    //AI(ht160s-maintainer) 20260616 : while the Pad Interface form is open the
    //operator drives the panel manually, so suppress machine-driven writes to
    //avoid fighting the manual buttons (HT172 SendSwitchStatus bShow guard).
    if(bShow)
        return;

    for(i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].PadName==aName)
        {
            bPadStatus[i]=Type;
            SyncHSysPadSwitchStatus(PadItem[i].PadName, Type);
            if(PadItem[i].btnEvent!=NULL)
                PadItem[i].btnEvent->Down=Type;
            if(PadItem[i].mlEvent!=NULL)
                PadItem[i].mlEvent->Value=Type;
        }
    }
}
//---------------------------------------------------------------------------
void TfPadInterface::SendCommand(AnsiString sData)
{
    int iSize;
    AnsiString SendText;

    RecordCommunication("[Send]", sData);
    if(bRs232Ok==false || PadComm==NULL)
    {
        //AI(ht160s-maintainer) 20260620 : align to HT172 - record that the send was
        //dropped because the Pad link is not ready, instead of a silent return.
        RecordCommunication("[Connect Error]", "send dropped, Pad link not ready");
        return;
    }

    SendText=sData+AnsiString('\r');
    iSize=SendText.Length();
    if(iSize<=0)
        return;

    try
    {
        PadComm->WriteCommData(SendText.c_str(), iSize);
    }
    catch(...)
    {
        bRs232Ok=false;
        RecordCommunication("[Send]", "FAIL");
    }
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::DoScanPanelLed(int iAddress, int iKey)
{
    int i;
    int iTag;
    bool bValue;

    iTag=(iAddress>0)?1:0;
    for(i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].mlEvent!=NULL && PadItem[i].mlEvent->Tag==iTag)
        {
            bValue=((PadItem[i].iData&iKey)!=0);
            bPadInputStatus[i]=bValue;
            PadItem[i].mlEvent->Value=bValue;
            SyncHSysPadInputStatus(PadItem[i].InputName, bValue);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::DoUpdataPadStatus(int iAddress, int iKey)
{
    int i;
    int iTag;
    bool bValue;

    iTag=(iAddress>0)?1:0;
    for(i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].btnEvent!=NULL && PadItem[i].btnEvent->Tag==iTag)
        {
            bValue=((PadItem[i].iData&iKey)!=0);
            bPadStatus[i]=bValue;
            SyncHSysPadSwitchStatus(PadItem[i].PadName, bValue);
            PadItem[i].btnEvent->Down=bValue;
            if(PadItem[i].mlEvent!=NULL)
                PadItem[i].mlEvent->Value=bValue;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::ProcessReceiceData()
{
    AnsiString S;
    AnsiString sAddress;
    AnsiString aPadKey;
    int iAddress;
    int iPadKey;

    AnsiString Entry;
    AnsiString Frame;
    int iPos;

    while(CommReceiveList->Count>0)
    {
        Entry=CommReceiveList->Strings[0];
        CommReceiveList->Delete(0);
        if(CommReceiveLength->Count>0)
            CommReceiveLength->Delete(0);

        //AI(ht160s-maintainer) 20260616 : one receive event may carry several
        //"\r"-delimited frames concatenated; split and process each, matching
        //HT172 ProcessReceiceData. (A frame split ACROSS two receive events is
        //not reassembled - same limitation as HT172, harmless at this frame size.)
        do
        {
            iPos=Entry.Pos("\r");
            if(iPos>0)
                Frame=Entry.SubString(1, iPos-1);
            else
                Frame=Entry;
            Frame=Frame.Trim();

            //AI(ht160s-maintainer) 20260619 : aligned to HT172 ProcessReceiceData -
            //gate each frame type INDEPENDENTLY by its own minimum length. The
            //trailing '\r' is already stripped above, so input/switch reports need
            //>=13 (HT172 used >=14 on the buffer that still held the '\r') and the
            //version reply needs >=8. S is computed first; SubString(6,2) on a short
            //frame yields "", which matches no type. (HT172's bSendSwitchStatusing
            //reset on '90' is omitted: that flag has no consumer in HT160.)
            S=Frame.SubString(6, 2);
            if(S=="00" && Frame.Length()>=13)
            {
                //panel key / LED-scan report
                sAddress=Frame.SubString(4, 1);
                iAddress=atoi(sAddress.c_str());
                aPadKey=Frame.SubString(8, 6);
                iPadKey=StrToIntDef("0x"+aPadKey, 0);
                //a valid status frame proves the Pad panel is talking; the panel
                //Power On/Off sensors are now live for CheckMotorPowerShutDown.
                bPadEverCommunicated=true;
                DoScanPanelLed(iAddress, iPadKey);
            }
            else if(S=="20" && Frame.Length()>=8)
            {
                //version reply
                bRequestVer=true;
            }
            else if(S=="90" && Frame.Length()>=13)
            {
                //switch-status report
                sAddress=Frame.SubString(4, 1);
                iAddress=atoi(sAddress.c_str());
                aPadKey=Frame.SubString(8, 6);
                iPadKey=StrToIntDef("0x"+aPadKey, 0);
                bPadEverCommunicated=true;
                DoUpdataPadStatus(iAddress, iPadKey);
            }

            if(iPos>0)
                Entry=Entry.SubString(iPos+1, Entry.Length()-iPos);
            else
                Entry="";
        }
        while(Entry.Pos("\r")>0);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::ProcessSendDataNew()
{
    int i;
    static bool bOldPadStatus[32]={false};
    bool bNeedSendFront;
    bool bNeedSendRear;

    bNeedSendFront=false;
    bNeedSendRear=false;
    for(i=0; i<CheckPadItem; i++)
    {
        if(bOldPadStatus[i]!=bPadStatus[i])
        {
            bOldPadStatus[i]=bPadStatus[i];
            if(PadItem[i].btnEvent!=NULL && PadItem[i].btnEvent->Tag==1)
                bNeedSendRear=true;
            else
                bNeedSendFront=true;
            if(PadItem[i].btnEvent!=NULL)
                PadItem[i].btnEvent->Down=bPadStatus[i];
        }
    }

    if(bNeedSendFront)
        RecordLocalStatusCommand(PAD_FrontControl);
    if(bNeedSendRear)
        RecordLocalStatusCommand(PAD_RearControl);
}
//---------------------------------------------------------------------------
bool __fastcall TfPadInterface::ProcessScanKey(AnsiString aSenName)
{
    int i;

    for(i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].InputName==NormalizePadInputName(aSenName))
            return bPadInputStatus[i];
    }
    return false;
}
//---------------------------------------------------------------------------
void __fastcall TfPadInterface::Main232()
{
    static int Task=1;
    static HTimer HeartbeatTimer;
    static HTimer VerTimeout;
    static HTimer ReconnectDelay;

    //AI(ht160s-maintainer) 20260616 : always process inbound frames so the
    //operator can still watch live Pad status during a manual test (user
    //requirement: keep RX visible).
    ProcessReceiceData();

    //AI(ht160s-maintainer) 20260616 : while the Maintenance screen (or any of its
    //modal sub-pages - ComPort / IOView / Pad) is open, suspend ALL spin-driven
    //outbound traffic (auto LED send, scan-enable, version heartbeat, auto-
    //reconnect) so a manual test is not disturbed. Manual button sends still work
    //(they call SendCommand directly, not via spin). Same fMaintenance->Visible
    //probe as csystem.cpp; RX above stays live. Reset Task so the heartbeat
    //restarts cleanly once Maintenance closes.
    if(fMaintenance!=NULL && fMaintenance->Visible)
    {
        Task=1;
        return;
    }

    ProcessSendDataNew();

    //AI(ht160s-maintainer) 20260616 : enable physical-key scan reporting once the
    //Pad link is up (HT172 uPadInterface.cpp Main232 bScanSwitch block). Without
    //this the Pad never sends "00" status frames, so DoScanPanelLed never fires
    //and panel buttons never reach HSys -> panel control looks dead. bScanSwitch
    //is re-armed by ComPort::RS232Init on every (re)connect.
    if(bRs232Ok && bScanSwitch)
    {
        SendCommand("t051400000000");
        bScanSwitch=false;
    }

    //AI(ht160s-maintainer) 20260616 : link keep-alive + auto-reconnect, ported
    //from HT172 uPadInterface.cpp Main232 (its tray-motor gate and InitialOK
    //check are dropped - HT160 has neither). Every 10s poll the Pad version; a
    //reply means the link is healthy; if the COM has dropped (bRs232Ok==false)
    //reconnect via ResetComm. HT172 timings: 10s poll / 1s reply / 0.1s settle
    //(HTimer.Set unit is 100ms, so 100/10/1).
    switch(Task)
    {
        case 1:
            HeartbeatTimer.Set(100);
            HeartbeatTimer.On();
            Task=10;
        case 10:
            if(HeartbeatTimer.Off())
            {
                RequestPadVersion();
                VerTimeout.Set(10);
                VerTimeout.On();
                Task=20;
            }
            break;
        case 20:
            if(bRs232Ok && bRequestVer)
                Task=1;
            else if(bRs232Ok==false)
            {
                ReconnectDelay.Set(1);
                ReconnectDelay.On();
                ResetComm();
                Task=50;
            }
            else if(VerTimeout.Off())
                Task=1;
            break;
        case 50:
            if(ReconnectDelay.Off())
                Task=1;
            break;
    }
}
//---------------------------------------------------------------------------
void TfPadInterface::RecordCommunication(AnsiString aTitle, AnsiString Command)
{
    AnsiString LineText;

    if(Memo_PadInterface==NULL)
        return;

    LineText=FormatDateTime("yyyy-mm-dd hh:nn:ss.zzz", Now())+AnsiString("   ")+aTitle+AnsiString(" => ")+Command;
    Memo_PadInterface->Lines->Add(LineText);
    if(Memo_PadInterface->Lines->Count>1000)
        Memo_PadInterface->Lines->Delete(0);

    //AI(ht160s-maintainer) 20260615 : persist Pad send/connect events to the
    //daily PadLog CSV (RX is logged from ComPort::PadCommReceiveData).
    g_PadCommLog.Log(aTitle, Command);
}
//---------------------------------------------------------------------------
