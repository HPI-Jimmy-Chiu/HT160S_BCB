//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ComPort.h"
#include "database.h"
#include "uPadInterface.h"
#include <IniFiles.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "SPComm"
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
    PadComm=NULL;
    BuildUI();
    OpenWorkFile();
}
//---------------------------------------------------------------------------
__fastcall TfComPort::~TfComPort()
{
    StopAllCom();
}
//---------------------------------------------------------------------------
void TfComPort::BuildUI()
{
    Caption="COM Port";
    Width=700;
    Height=430;
    Position=poScreenCenter;
    OnShow=FormShow;
    OnClose=FormClose;

    pnlTop=new TPanel(this);
    pnlTop->Parent=this;
    pnlTop->Align=alTop;
    pnlTop->Height=42;
    pnlTop->Caption="Pad COM Port";
    pnlTop->Font->Style=TFontStyles()<<fsBold;

    sbExit=new TButton(this);
    sbExit->Parent=pnlTop;
    sbExit->Caption="Exit";
    sbExit->Width=80;
    sbExit->Height=26;
    sbExit->Left=pnlTop->Width-90;
    sbExit->Top=8;
    sbExit->Anchors=TAnchors()<<akTop<<akRight;
    sbExit->OnClick=sbExitClick;

    pnlSetting=new TPanel(this);
    pnlSetting->Parent=this;
    pnlSetting->Align=alTop;
    pnlSetting->Height=64;
    pnlSetting->BevelOuter=bvNone;

    labPadCom=new TLabel(this);
    labPadCom->Parent=pnlSetting;
    labPadCom->Caption="Pad COM";
    labPadCom->Left=12;
    labPadCom->Top=22;

    cbPadComm=new TComboBox(this);
    cbPadComm->Parent=pnlSetting;
    cbPadComm->Left=78;
    cbPadComm->Top=18;
    cbPadComm->Width=100;
    cbPadComm->Style=csDropDown;
    PopulateComList();

    spbResetCom=new TButton(this);
    spbResetCom->Parent=pnlSetting;
    spbResetCom->Caption="Open / Reset";
    spbResetCom->Left=190;
    spbResetCom->Top=16;
    spbResetCom->Width=95;
    spbResetCom->OnClick=spbResetComClick;

    btnStopCom=new TButton(this);
    btnStopCom->Parent=pnlSetting;
    btnStopCom->Caption="Close";
    btnStopCom->Left=294;
    btnStopCom->Top=16;
    btnStopCom->Width=70;
    btnStopCom->OnClick=btnStopComClick;

    sbUpdate=new TButton(this);
    sbUpdate->Parent=pnlSetting;
    sbUpdate->Caption="Save";
    sbUpdate->Left=372;
    sbUpdate->Top=16;
    sbUpdate->Width=70;
    sbUpdate->OnClick=sbUpdateClick;

    btnClearMemo=new TButton(this);
    btnClearMemo->Parent=pnlSetting;
    btnClearMemo->Caption="Clear Log";
    btnClearMemo->Left=450;
    btnClearMemo->Top=16;
    btnClearMemo->Width=82;
    btnClearMemo->OnClick=btnClearMemoClick;

    pnlManual=new TPanel(this);
    pnlManual->Parent=this;
    pnlManual->Align=alTop;
    pnlManual->Height=54;
    pnlManual->BevelOuter=bvNone;

    labManualSend=new TLabel(this);
    labManualSend->Parent=pnlManual;
    labManualSend->Caption="Manual Send";
    labManualSend->Left=12;
    labManualSend->Top=18;

    edPanelSend_Com=new TEdit(this);
    edPanelSend_Com->Parent=pnlManual;
    edPanelSend_Com->Left=100;
    edPanelSend_Com->Top=14;
    edPanelSend_Com->Width=250;
    edPanelSend_Com->Text="t051120";

    sbPanelSend_Com=new TButton(this);
    sbPanelSend_Com->Parent=pnlManual;
    sbPanelSend_Com->Caption="Send";
    sbPanelSend_Com->Left=360;
    sbPanelSend_Com->Top=13;
    sbPanelSend_Com->Width=70;
    sbPanelSend_Com->OnClick=sbPanelSend_ComClick;

    pnlLog=new TPanel(this);
    pnlLog->Parent=this;
    pnlLog->Align=alClient;
    pnlLog->BevelOuter=bvNone;

    memoPanelCom=new TMemo(this);
    memoPanelCom->Parent=pnlLog;
    memoPanelCom->Align=alClient;
    memoPanelCom->ScrollBars=ssVertical;

    PadComm=new TComm(this);
    PadComm->OnReceiveData=PadCommReceiveData;
}
//---------------------------------------------------------------------------
void TfComPort::PopulateComList()
{
    int i;

    if(cbPadComm==NULL)
        return;

    cbPadComm->Items->Clear();
    for(i=1; i<=32; i++)
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
void TfComPort::ConfigurePadComm()
{
    PadComm->CommName="\\\\.\\"+cbPadComm->Text;
    PadComm->ReadIntervalTimeout=1;
    PadComm->Parity=None;
    PadComm->BaudRate=PAD_DEFAULT_BAUD;
    PadComm->ByteSize=_8;
    PadComm->ParityCheck=false;
    PadComm->StopBits=_1;
    PadComm->OnReceiveData=PadCommReceiveData;
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
    RS232Init();
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::btnStopComClick(TObject *Sender)
{
    (void)Sender;
    StopAllCom();
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
    Ini->WriteInteger(PAD_COM_SECTION, "BaudRate", PAD_DEFAULT_BAUD);
    delete Ini;
}
//---------------------------------------------------------------------------
void __fastcall TfComPort::OpenWorkFile()
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString ComName;

    FileName=GetWorkFileName();
    if(FileExists(FileName)==false)
    {
        if(cbPadComm!=NULL)
            cbPadComm->Text=PAD_DEFAULT_COM;
        return;
    }

    Ini=new TIniFile(FileName);
    ComName=Ini->ReadString(PAD_COM_SECTION, PAD_COM_PORT_KEY, PAD_DEFAULT_COM);
    delete Ini;

    if(cbPadComm!=NULL)
    {
        if(cbPadComm->Items->IndexOf(ComName)<0)
            cbPadComm->Items->Add(ComName);
        cbPadComm->Text=ComName;
    }
}
//---------------------------------------------------------------------------
bool __fastcall TfComPort::RS232Init()
{
    EnsurePadInterface();
    StopAllCom();
    ConfigurePadComm();

    try
    {
        PadComm->StartComm();
        fPadInterface->bRs232Ok=true;
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
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void TfComPort::StopAllCom()
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
void TfComPort::Spin()
{
    if(fPadInterface!=NULL)
        fPadInterface->Main232();
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
    MemoAddString(memoPanelCom, "[Recv]", RecvText);

    if(UnitText==AnsiString("t05"))
    {
        EnsurePadInterface();
        fPadInterface->CommReceiveLength->Add(IntToStr((int)BufferLength));
        fPadInterface->CommReceiveList->Add(RecvText);
    }
}
//---------------------------------------------------------------------------
