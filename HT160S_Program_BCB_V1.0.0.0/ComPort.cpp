//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ComPort.h"
#include "database.h"
#include "uPadInterface.h"
#include "MyBinDisp.h"
#include "GeneralSetting.h"
#include "CosFunction.h"     //AI(ht160s-bindisplay) 20260617 : BinAreaMap error-bin area for LED color
#include "cCommLog.h"
#include "mymessbox.h"
#include <IniFiles.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "SPComm"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfComPort *fComPort = NULL;
//---------------------------------------------------------------------------
static const char *PAD_COM_SECTION = "Pad";
static const char *PAD_COM_PORT_KEY = "CommName";
static const char *PAD_DEFAULT_COM = "COM1";
static const int PAD_DEFAULT_BAUD = 115200;
//---------------------------------------------------------------------------
void EnsureComPortCreated(TComponent *Owner)
{
    if(fComPort!=NULL)
        return;
    if(Owner==NULL)
        Owner=Application;
    fComPort=new TfComPort(Owner);
}
//---------------------------------------------------------------------------
void SpinComPort()
{
    if(fComPort!=NULL)
        fComPort->Spin();
}
//---------------------------------------------------------------------------
__fastcall TfComPort::TfComPort(TComponent* Owner)
    : TForm(Owner)
{
    bShow=false;
    bPadAutoStarted=false;
    BinComm=NULL;
    PopulateComList();
    OpenWorkFile();
    ConfigureBinDisplay();
}
//---------------------------------------------------------------------------
__fastcall TfComPort::~TfComPort()
{
    StopAllCom();
}
//---------------------------------------------------------------------------
void TfComPort::PopulateComList()
{
    if(cbPadComm==NULL)
        return;

    cbPadComm->Items->Clear();
    for(int i=1; i<=32; i++)
        cbPadComm->Items->Add(AnsiString("COM")+IntToStr(i));
    cbPadComm->Text=PAD_DEFAULT_COM;
}
//---------------------------------------------------------------------------
AnsiString TfComPort::GetWorkFileName()
{
    AnsiString RootPath=HSys.CurrentDir;
    if(RootPath==AnsiString(""))
        RootPath="..";
    return RootPath+AnsiString("\\system\\ComPort.ini");
}
//---------------------------------------------------------------------------
void TfComPort::EnsurePadInterface()
{
    if(fPadInterface==NULL)
        fPadInterface=new TfPadInterface(Application);

    fPadInterface->PadComm=PadComm;
    fPadInterface->PadComPort=cbPadComm->Text;
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260616 : Pad baud is user-selectable (cbPadBaud) and
//persisted in ComPort.ini. Default follows old-160 PadInterfacePara.ini (115200).
int TfComPort::GetSelectedBaud()
{
    int Baud=0;
    if(cbPadBaud!=NULL)
        Baud=atoi(cbPadBaud->Text.c_str());
    if(Baud<=0)
        Baud=PAD_DEFAULT_BAUD;
    return Baud;
}
//---------------------------------------------------------------------------
void TfComPort::ConfigurePadComm()
{
    PadComm->CommName="\\\\.\\"+cbPadComm->Text;
    PadComm->ReadIntervalTimeout=1;
    PadComm->Parity=None;
    PadComm->BaudRate=GetSelectedBaud();
    PadComm->ByteSize=_8;
    PadComm->ParityCheck=false;
    PadComm->StopBits=_1;
    PadComm->OnReceiveData=PadCommReceiveData;
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260615 : wire HSys.BinDisCtrl to a runtime COM and the
//General.ini endpoint. The controller opens/closes the port itself inside Spin()
//(StartComport); here we only prepare the TComm line settings and the run gate.
void TfComPort::ConfigureBinDisplay()
{
    // AI(ht160s-bindisplay) 20260623 : (re)create the controller to match the
    // configured panel type. Done here, not the SYSTEM_MODULAR ctor, because the
    // ctor runs before GeneralSetting.Load (PanelType unknown then). A later
    // PanelType change takes effect on the next ConfigureBinDisplay (startup /
    // maintenance Apply). PanelType: 0 = LED (HT9046, default), 1 = TFT (HT9011).
    {
        int iWantKind=0;
        if(GeneralSetting.iBinDispPanelType==1)
            iWantKind=1;
        if(HSys.BinDisCtrl==NULL || HSys.BinDisCtrl->GetPanelKind()!=iWantKind)
        {
            if(HSys.BinDisCtrl!=NULL)
            {
                delete HSys.BinDisCtrl;
                HSys.BinDisCtrl=NULL;
            }
            if(iWantKind==1)
                HSys.BinDisCtrl=new TMyBinDispTFT;
            else
                HSys.BinDisCtrl=new TMyBinDispHT9046;
        }
    }
    if(HSys.BinDisCtrl==NULL)
        return;
    if(BinComm==NULL)
        BinComm=new TComm(this);

    //AI(ht160s-maintainer) 20260616 : 9600bps needs ~100ms inter-byte gap to keep a
    //reply frame in one OnReceiveData (matches HT172 commBinDisStore). At 1ms the
    //9600 reply was split per-byte and CommBinReceiveData overwrites sReadBuffer each
    //event, so Pos(sCheckWord) never matched -> "no reply received".
    BinComm->ReadIntervalTimeout=100;
    BinComm->Parity=None;
    BinComm->BaudRate=GeneralSetting.iBinDispBaud;
    BinComm->ByteSize=_8;
    BinComm->ParityCheck=false;
    BinComm->StopBits=_1;

    HSys.BinDisCtrl->CommBin=BinComm;
    HSys.BinDisCtrl->SetComParity(None);
    HSys.BinDisCtrl->SetComPort(GeneralSetting.sBinDispComPort);
    HSys.BinDisCtrl->SetDelayTime(GeneralSetting.iBinDispDelaySec);
    HSys.BinDisCtrl->SetUsedBinNumber(BIN_DISP_UNIT_COUNT);
    for(int i=0; i<BIN_DISP_UNIT_COUNT; i++)
        HSys.BinDisCtrl->InstalledUnit(i);
    HSys.BinDisCtrl->InitialOK=true;

    ApplyBinDisplayConfig();

    // Only kick the state machine when the hardware is present. Leaving it
    // un-started keeps bFirstInit true so a later enable still runs task 1
    // (which opens the COM port) rather than jumping straight to GetStatus.
    if(GeneralSetting.bBinDisplayInstalled)
        HSys.BinDisCtrl->ProcessStopStart(true);
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260615 : convert one display-label string to the
//WriteTargetBin value encoding: -1 blank(X), 0..99 digits, 100..125 A..Z.
static int BinTextToValue(AnsiString s)
{
    s=s.Trim();
    if(s=="")
        return -1;
    char c=s[1];
    if(c>='0' && c<='9')
        return atoi(s.c_str());
    if(c>='A' && c<='Z')
        return 100+(c-'A');
    if(c>='a' && c<='z')
        return 100+(c-'a');
    return -1;
}
//---------------------------------------------------------------------------
//AI(ht160s-bindisplay) 20260617 : LED color now follows error/non-error status,
//not the per-unit General.ini Color0..8 (which is left for manual-test use).
//Rule (user 2026-06-17): Loader / Empty / Color and every NON-error Auto show
//green; the single Error Auto shows red. Mirrors 9011UC cShowBinSelect color
//logic (fail bin -> red, else green). HT9046 color code: 1=Red 2=Green 3=R+G.
//The error Auto is BinAreaMap.GetErrorBinArea() (set on the Bin page, fixed
//during a lot). Area enum -> unit index by (unit = area - 1):
//  Empty=1->0 Loader=2->1 Auto1=3->2 ... Auto6=8->7 Color=9->8.
void TfComPort::ApplyBinDisplayConfig()
{
    static const int BIN_COLOR_RED   = 1;
    static const int BIN_COLOR_GREEN = 2;

    if(HSys.BinDisCtrl==NULL)
        return;

    int ErrorArea=BinAreaMap.GetErrorBinArea();   // area enum of the Error Auto

    for(int i=0; i<BIN_DISP_UNIT_COUNT; i++)
    {
        int Value=BinTextToValue(GeneralSetting.sBinDispText[i]);
        int Color=((i+1)==ErrorArea) ? BIN_COLOR_RED : BIN_COLOR_GREEN;
        HSys.BinDisCtrl->SetUnitLabel(i, Value, Color);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::FormShow(TObject *Sender)
{
    (void)Sender;
    OpenWorkFile();
    bShow=true;
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::FormClose(TObject *Sender, TCloseAction &Action)
{
    (void)Sender;
    (void)Action;
    bShow=false;
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::sbExitClick(TObject *Sender)
{
    (void)Sender;
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::sbUpdateClick(TObject *Sender)
{
    (void)Sender;
    SaveWorkFile();
    MemoAddString(memoPanelCom, "[Save]", GetWorkFileName());
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::spbResetComClick(TObject *Sender)
{
    (void)Sender;
    SaveWorkFile();
    RS232Init(true);
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::btnStopComClick(TObject *Sender)
{
    (void)Sender;
    StopPadCom();
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::btnClearMemoClick(TObject *Sender)
{
    (void)Sender;
    if(memoPanelCom!=NULL)
        memoPanelCom->Lines->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::sbPanelSend_ComClick(TObject *Sender)
{
    (void)Sender;
    EnsurePadInterface();
    fPadInterface->SendCommand(edPanelSend_Com->Text);
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::SaveWorkFile()
{
    TIniFile *Ini;

    Ini=new TIniFile(GetWorkFileName());
    Ini->WriteString(PAD_COM_SECTION, PAD_COM_PORT_KEY, cbPadComm->Text);
    Ini->WriteInteger(PAD_COM_SECTION, "BaudRate", GetSelectedBaud());
    delete Ini;
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::OpenWorkFile()
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString ComName;
    int Baud;

    FileName=GetWorkFileName();
    if(FileExists(FileName)==false)
    {
        if(cbPadComm!=NULL)
            cbPadComm->Text=PAD_DEFAULT_COM;
        if(cbPadBaud!=NULL)
            cbPadBaud->Text=IntToStr(PAD_DEFAULT_BAUD);
        return;
    }

    Ini=new TIniFile(FileName);
    ComName=Ini->ReadString(PAD_COM_SECTION, PAD_COM_PORT_KEY, PAD_DEFAULT_COM);
    Baud=Ini->ReadInteger(PAD_COM_SECTION, "BaudRate", PAD_DEFAULT_BAUD);
    delete Ini;

    if(cbPadComm!=NULL)
    {
        if(cbPadComm->Items->IndexOf(ComName)<0)
            cbPadComm->Items->Add(ComName);
        cbPadComm->Text=ComName;
    }

    if(cbPadBaud!=NULL)
    {
        if(Baud<=0)
            Baud=PAD_DEFAULT_BAUD;
        if(cbPadBaud->Items->IndexOf(IntToStr(Baud))<0)
            cbPadBaud->Items->Add(IntToStr(Baud));
        cbPadBaud->Text=IntToStr(Baud);
    }
}
//---------------------------------------------------------------------------
bool __fastcall TfComPort::RS232Init(bool bNotifyOperator)
{
    EnsurePadInterface();
    StopPadCom();
    ConfigurePadComm();

    try
    {
        PadComm->StartComm();
        fPadInterface->bRs232Ok=true;
        //AI(ht160s-maintainer) 20260616 : re-arm the physical-key scan enable so
        //Main232 re-sends t051400000000 on every (re)connect, not just first boot.
        fPadInterface->bScanSwitch=true;
        MemoAddString(memoPanelCom, "[Connect]", cbPadComm->Text+AnsiString(" OK"));
        fPadInterface->SendCommand("t050490000000");
        fPadInterface->SendCommand("t050491000000");
        fPadInterface->SendCommand("t051490000000");
        fPadInterface->SendCommand("t051491000000");
    }
    catch(...)
    {
        fPadInterface->bRs232Ok=false;
        MemoAddString(memoPanelCom, "[Connect]", cbPadComm->Text+AnsiString(" FAIL"));
        //AI(ht160s-maintainer) 20260620 : on an OPERATOR-initiated reconnect only,
        //surface the failure with a NON-stopping prompt. Never ShowMyMessage here -
        //it DecStopAllMotor()+ShowModal(); the unattended Spin() auto-open path
        //(bNotifyOperator=false) must not block or stop the machine at startup.
        if(bNotifyOperator)
            ShowMyOKMessageNoStop("Pad COM "+cbPadComm->Text+" connect failed");
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260617 : Pad-only stop. Pad and the Bin display are two
//independent physical COM lines; a Pad (re)connect or the dialog Stop button must
//NOT tear down the Bin display. RS232Init/btnStopCom call this, not StopAllCom.
void TfComPort::StopPadCom()
{
    try
    {
        if(PadComm!=NULL)
            PadComm->StopComm();
    }
    catch(...)
    {
    }

    if(fPadInterface!=NULL)
        fPadInterface->bRs232Ok=false;

    MemoAddString(memoPanelCom, "[Disconnect]", "Pad COM closed");
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260617 : stop BOTH lines; only for app shutdown
//(~TfComPort). Closes Pad via StopPadCom then the Bin-display COM.
void TfComPort::StopAllCom()
{
    StopPadCom();

    if(HSys.BinDisCtrl!=NULL)
        HSys.BinDisCtrl->ProcessStopStart(false);
    try
    {
        if(BinComm!=NULL)
            BinComm->StopComm();
    }
    catch(...)
    {
    }
}
//---------------------------------------------------------------------------
void TfComPort::Spin()
{
    //AI(ht160s-maintainer) 20260616 : auto-open the Pad COM once on the first
    //spin tick so the panel connects at software start (old-160 opened it from
    //maintenance init). The Open/Reset button still re-opens manually.
    if(bPadAutoStarted==false)
    {
        bPadAutoStarted=true;
        RS232Init();
    }
    if(fPadInterface!=NULL)
        fPadInterface->Main232();
    if(HSys.BinDisCtrl!=NULL)
        HSys.BinDisCtrl->Spin();
}
//---------------------------------------------------------------------------
void TfComPort::MemoAddString(TMemo *Memo, AnsiString Title, AnsiString Text)
{
    AnsiString LineText;

    if(Memo==NULL)
        return;

    LineText=FormatDateTime("yyyy-mm-dd hh:nn:ss.zzz", Now())+AnsiString("   ")+Title+AnsiString(" => ")+Text;
    Memo->Lines->Add(LineText);
    if(Memo->Lines->Count>1000)
        Memo->Lines->Delete(0);

    //AI(ht160s-maintainer) 20260615 : persist the Pad COM memo to the daily
    //PadLog CSV so connect/disconnect/recv can be traced after a restart.
    g_PadCommLog.Log(Title, Text);
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::PadCommReceiveData(TObject *Sender, Pointer Buffer, WORD BufferLength)
{
    char *DataPtr;
    AnsiString RecvText;
    AnsiString UnitText;
    int i;

    (void)Sender;
    if(Buffer==NULL || BufferLength==0)
        return;

    DataPtr=(char *)Buffer;
    for(i=0; i<BufferLength; i++)
        RecvText+=AnsiString(DataPtr[i]);

    UnitText=RecvText.SubString(1, 3);

    //AI(ht160s-maintainer) 20260616 : match HT172 - only log/buffer valid Pad
    //frames (header "t05"). Logging every raw receive event printed line noise,
    //echoes and partial fragments to the memo as Big5 garbage ("w....?").
    if(UnitText==AnsiString("t05"))
    {
        MemoAddString(memoPanelCom, "[Recv]", RecvText);
        EnsurePadInterface();
        fPadInterface->CommReceiveLength->Add(IntToStr((int)BufferLength));
        fPadInterface->CommReceiveList->Add(RecvText);
    }
}
//---------------------------------------------------------------------------
