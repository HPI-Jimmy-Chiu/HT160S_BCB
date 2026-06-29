//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
#include "language.h"

#include "database.h"
#include "maintenance.h"
#include "ComPort.h"
#include "CosFunction.h"
#include "GeneralSetting.h"
#include "MyBinDisp.h"
#include "TopCcdSocket.h"
#include "ColorCcdSocket.h"
#include "CosFunction.h"
#include "LotWebApiClient.h"
#include "iosetview.h"
#include "uteach.h"
#include "uMotorTest.h"
#include "uQwertyKey.h"
#include "UserRoleManager.h"   //AI(ht160s-password) 20260624 : account book + level gating
#include "mymessbox.h"
#include "SecsGem/uHGemLogForm.h"   //AI(ht160s-secsgem) 20260611 : ShowSecsGemLog
#include "SecsGem/uAgvStation.h"   //AI(ht160s-agv) 20260625 : AgvCoord.DescribeAgvState for AMR tab
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "ALed"
#pragma link "ALed"
#pragma resource "*.dfm"
TfMaintenance *fMaintenance;
static bool bMusicTestOn[4]={false,false,false,false};
static const TColor TOWER_LIGHT_GREEN_OFF=(TColor)22272;
static const TColor TOWER_LIGHT_YELLOW_OFF=(TColor)881034;
static const char *TOWER_LIGHT_INI_GROUP="tsMaintTowerLight";
static AnsiString MaintenanceWorkFileName="";
static int TowerLightConfig[TOWER_LIGHT_ROW_COUNT][TOWER_LIGHT_COLOR_COUNT];
static bool bTowerLightConfigLoaded=false;
static const int DefaultTowerLightConfig[TOWER_LIGHT_ROW_COUNT][TOWER_LIGHT_COLOR_COUNT]=
{
    {TOWER_LIGHT_STATE_ON,  TOWER_LIGHT_STATE_OFF, TOWER_LIGHT_STATE_OFF},
    {TOWER_LIGHT_STATE_OFF, TOWER_LIGHT_STATE_OFF, TOWER_LIGHT_STATE_BLINK},
    {TOWER_LIGHT_STATE_OFF, TOWER_LIGHT_STATE_ON,  TOWER_LIGHT_STATE_OFF},
    {TOWER_LIGHT_STATE_OFF, TOWER_LIGHT_STATE_BLINK, TOWER_LIGHT_STATE_OFF},
    {TOWER_LIGHT_STATE_OFF, TOWER_LIGHT_STATE_BLINK, TOWER_LIGHT_STATE_OFF},
    {TOWER_LIGHT_STATE_OFF, TOWER_LIGHT_STATE_BLINK, TOWER_LIGHT_STATE_OFF}
};
//---------------------------------------------------------------------------
typedef struct
{
    TTabSheet *Page;
    TSpeedButton *Button;
    TMaintenanceMenuAction Action;
    bool PinBottom;
} TMaintenancePageDef;
//---------------------------------------------------------------------------
static TColor GetTowerLightTrueColor(int ColorIndex)
{
    if(ColorIndex==0)
        return clLime;
    if(ColorIndex==1)
        return clYellow;
    return clRed;
}
//---------------------------------------------------------------------------
static TColor GetTowerLightFalseColor(int ColorIndex)
{
    if(ColorIndex==0)
        return TOWER_LIGHT_GREEN_OFF;
    if(ColorIndex==1)
        return TOWER_LIGHT_YELLOW_OFF;
    return clMaroon;
}
//---------------------------------------------------------------------------
static int NormalizeTowerLightState(int State)
{
    if(State<TOWER_LIGHT_STATE_OFF || State>TOWER_LIGHT_STATE_BLINK)
        return TOWER_LIGHT_STATE_OFF;
    return State;
}
//---------------------------------------------------------------------------
static AnsiString GetTowerLightIniFileName()
{
    AnsiString RootPath=HSys.CurrentDir;

    if(MaintenanceWorkFileName!=AnsiString(""))
        return MaintenanceWorkFileName;

    if(RootPath==AnsiString(""))
        RootPath="..";
    return RootPath+AnsiString("\\system\\maintance.ini");
}
//---------------------------------------------------------------------------
static void SetMaintenanceWorkFileName(AnsiString FileName)
{
    AnsiString RootPath;

    if(FileName!=AnsiString(""))
    {
        MaintenanceWorkFileName=FileName;
        return;
    }

    RootPath=HSys.CurrentDir;
    if(RootPath==AnsiString(""))
        RootPath="..";
    MaintenanceWorkFileName=RootPath+AnsiString("\\system\\maintance.ini");
}
//---------------------------------------------------------------------------
static int ReadEditInt(TEdit *Edit, int DefaultValue)
{
    int Value;

    if(Edit==NULL || Edit->Text.Trim()==AnsiString(""))
        return DefaultValue;
    Value=atoi(Edit->Text.c_str());
    return Value;
}
//---------------------------------------------------------------------------
static TLabel *CreateMaintLabel(TComponent *Owner, TWinControl *Parent, int Left, int Top,
    int Width, int Height, AnsiString Caption)
{
    TLabel *Label=new TLabel(Owner);
    Label->Parent=Parent;
    Label->Left=Left;
    Label->Top=Top;
    Label->Width=Width;
    Label->Height=Height;
    Label->AutoSize=false;
    Label->Caption=Caption;
    Label->ParentColor=true;
    return Label;
}
//---------------------------------------------------------------------------
static TEdit *CreateMaintEdit(TComponent *Owner, TWinControl *Parent, int Left, int Top,
    int Width, AnsiString Text)
{
    TEdit *Edit=new TEdit(Owner);
    Edit->Parent=Parent;
    Edit->Left=Left;
    Edit->Top=Top;
    Edit->Width=Width;
    Edit->Height=24;
    Edit->Text=Text;
    return Edit;
}
//---------------------------------------------------------------------------
static TCheckBox *CreateMaintCheckBox(TComponent *Owner, TWinControl *Parent, int Left, int Top,
    int Width, AnsiString Caption)
{
    TCheckBox *CheckBox=new TCheckBox(Owner);
    CheckBox->Parent=Parent;
    CheckBox->Left=Left;
    CheckBox->Top=Top;
    CheckBox->Width=Width;
    CheckBox->Height=24;
    CheckBox->Caption=Caption;
    return CheckBox;
}
//---------------------------------------------------------------------------
static TButton *CreateMaintButton(TComponent *Owner, TWinControl *Parent, int Left, int Top,
    int Width, AnsiString Caption, TNotifyEvent OnClick)
{
    TButton *Button=new TButton(Owner);
    Button->Parent=Parent;
    Button->Left=Left;
    Button->Top=Top;
    Button->Width=Width;
    Button->Height=28;
    Button->Caption=Caption;
    Button->OnClick=OnClick;
    return Button;
}
//---------------------------------------------------------------------------
static AnsiString GetTowerLightLedName(int RowIndex, int ColorIndex)
{
    return AnsiString("RGB")+IntToStr(RowIndex)+IntToStr(ColorIndex);
}
//---------------------------------------------------------------------------
static void LoadTowerLightDefaultConfig()
{
    int RowIndex;
    int ColorIndex;

    for(RowIndex=0; RowIndex<TOWER_LIGHT_ROW_COUNT; RowIndex++)
    {
        for(ColorIndex=0; ColorIndex<TOWER_LIGHT_COLOR_COUNT; ColorIndex++)
            TowerLightConfig[RowIndex][ColorIndex]=DefaultTowerLightConfig[RowIndex][ColorIndex];
    }
}
//---------------------------------------------------------------------------
void LoadTowerLightSettings()
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString LedName;
    bool Value;
    bool Blink;
    int RowIndex;
    int ColorIndex;

    if(bTowerLightConfigLoaded)
        return;

    LoadTowerLightDefaultConfig();
    FileName=GetTowerLightIniFileName();
    if(FileExists(FileName))
    {
        Ini=new TIniFile(FileName);
        for(RowIndex=0; RowIndex<TOWER_LIGHT_ROW_COUNT; RowIndex++)
        {
            for(ColorIndex=0; ColorIndex<TOWER_LIGHT_COLOR_COUNT; ColorIndex++)
            {
                LedName=GetTowerLightLedName(RowIndex, ColorIndex);
                Value=Ini->ReadInteger(TOWER_LIGHT_INI_GROUP, LedName+AnsiString("_Value"),
                    DefaultTowerLightConfig[RowIndex][ColorIndex]!=TOWER_LIGHT_STATE_OFF)!=0;
                Blink=Ini->ReadInteger(TOWER_LIGHT_INI_GROUP, LedName+AnsiString("_Blink"),
                    DefaultTowerLightConfig[RowIndex][ColorIndex]==TOWER_LIGHT_STATE_BLINK)!=0;

                if(Value)
                    TowerLightConfig[RowIndex][ColorIndex]=Blink ? TOWER_LIGHT_STATE_BLINK : TOWER_LIGHT_STATE_ON;
                else
                    TowerLightConfig[RowIndex][ColorIndex]=TOWER_LIGHT_STATE_OFF;
            }
        }
        delete Ini;
    }
    bTowerLightConfigLoaded=true;
}
//---------------------------------------------------------------------------
void SaveTowerLightSettings()
{
    TIniFile *Ini;
    AnsiString LedName;
    int RowIndex;
    int ColorIndex;
    int State;

    LoadTowerLightSettings();
    Ini=new TIniFile(GetTowerLightIniFileName());
    for(RowIndex=0; RowIndex<TOWER_LIGHT_ROW_COUNT; RowIndex++)
    {
        for(ColorIndex=0; ColorIndex<TOWER_LIGHT_COLOR_COUNT; ColorIndex++)
        {
            LedName=GetTowerLightLedName(RowIndex, ColorIndex);
            State=TowerLightConfig[RowIndex][ColorIndex];
            Ini->WriteInteger(TOWER_LIGHT_INI_GROUP, LedName+AnsiString("_Blink"),
                (State==TOWER_LIGHT_STATE_BLINK)?1:0);
            Ini->WriteInteger(TOWER_LIGHT_INI_GROUP, LedName+AnsiString("_Value"),
                (State!=TOWER_LIGHT_STATE_OFF)?1:0);
        }
    }
    delete Ini;
}
//---------------------------------------------------------------------------
void SetTowerLightConfigState(int RowIndex, int ColorIndex, int State)
{
    LoadTowerLightSettings();
    if(RowIndex<0 || RowIndex>=TOWER_LIGHT_ROW_COUNT)
        return;
    if(ColorIndex<0 || ColorIndex>=TOWER_LIGHT_COLOR_COUNT)
        return;
    TowerLightConfig[RowIndex][ColorIndex]=NormalizeTowerLightState(State);
}
//---------------------------------------------------------------------------
int GetTowerLightConfigState(int RowIndex, int ColorIndex)
{
    LoadTowerLightSettings();
    if(RowIndex<0 || RowIndex>=TOWER_LIGHT_ROW_COUNT)
        return TOWER_LIGHT_STATE_OFF;
    if(ColorIndex<0 || ColorIndex>=TOWER_LIGHT_COLOR_COUNT)
        return TOWER_LIGHT_STATE_OFF;
    return TowerLightConfig[RowIndex][ColorIndex];
}
//---------------------------------------------------------------------------
bool GetTowerLightConfigOutput(int RowIndex, int ColorIndex, bool BlinkPhase)
{
    int State=GetTowerLightConfigState(RowIndex, ColorIndex);

    if(State==TOWER_LIGHT_STATE_ON)
        return true;
    if(State==TOWER_LIGHT_STATE_BLINK)
        return true;   //AI(HT160S-Maintainer) 20260622 : tower light must NOT blink (user) -> BLINK shows STEADY ON (BlinkPhase now unused)
    return false;
}
//---------------------------------------------------------------------------
static void ConfigureTowerLightLed(TALed *Led, int ColorIndex)
{
    if(Led==NULL)
        return;

    Led->TrueColor=GetTowerLightTrueColor(ColorIndex);
    Led->FalseColor=GetTowerLightFalseColor(ColorIndex);
    Led->Interval=300;
    Led->LEDStyle=Aled::LEDSqLarge;
}
//---------------------------------------------------------------------------
__fastcall TfMaintenance::TfMaintenance(TComponent* Owner)
    : TForm(Owner)
{
    int PageIndex;
    int RowIndex;
    int ColorIndex;

    iMaintenanceMenuCount=0;
    LastClickButton=NULL;
    bTowerLightBlinkPhase=false;
    //AI(ht160s-maintainer) 20260613 : Bin Display (MCU) / Top CCD / Color CCD / Lot
    //WebAPI page controls now stream from the DFM, so they are already assigned by the
    //time this body runs - do not NULL them and do not runtime-build the pages.

    for(PageIndex=0; PageIndex<MAX_MAINTENANCE_MENU_COUNT; PageIndex++)
    {
        MenuButtons[PageIndex]=NULL;
        MenuPages[PageIndex]=NULL;
        MenuActions[PageIndex]=maShowPage;
        MenuBottomPins[PageIndex]=false;
    }

    for(RowIndex=0; RowIndex<TOWER_LIGHT_ROW_COUNT; RowIndex++)
    {
        for(ColorIndex=0; ColorIndex<TOWER_LIGHT_COLOR_COUNT; ColorIndex++)
        {
            TowerLightLeds[RowIndex][ColorIndex]=NULL;
        }
    }

    bLoadingHardwareSettings=false;
    RegisterMaintenancePages();
    LayoutMaintenanceButtons();
    InitializeTowerLightPanels();
    LoadMaintenanceSettings();

    //AI(ht160s-maintainer) 20260613 : tmrTowerLightBlink now streams from the DFM
    //(Interval=300, OnTimer wired there, Enabled=False). Enable it here so the first
    //tick can only fire after the tower-light LEDs/config are initialized above.
    tmrTowerLightBlink->Enabled=true;

    SelectMaintenancePage(0);
    //AI(HT160S-Maintainer) 20260608 : need2-D : read saved Top CCD endpoint at
    //power-on so the maintenance page IP/Port fields reflect General.ini even
    //before the page is opened (socket itself is configured by LoadConfig()).
    LoadTopCcdSettings();
    EnsureTopCcdSocketCreated();

    //AI(HT160S-Maintainer) 20260612 : read saved Color CCD endpoint + enable at
    //power-on so the maintenance page reflects General.ini before it is opened.
    LoadColorCcdSettings();
    EnsureColorCcdSocketCreated();

    //AI(general) 20260611 : Lot WebAPI client power-on config + creation
    LoadLotWebApiSettings();
    EnsureLotWebApiClientCreated();
}
//---------------------------------------------------------------------------
static AnsiString GetTopCcdIniFileName()
{
    AnsiString RootPath=HSys.CurrentDir;
    if(RootPath==AnsiString(""))
        RootPath="..";
    return RootPath+AnsiString("\\system\\General.ini");
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::LoadTopCcdSettings()
{
    TIniFile *Ini;
    AnsiString FileName;

    FileName=GetTopCcdIniFileName();
    Ini=new TIniFile(FileName);
    try
    {
        if(edTopCcdIP!=NULL)
            edTopCcdIP->Text=Ini->ReadString("TopCCD", "Address", "172.16.8.89");
        if(edTopCcdPort!=NULL)
            edTopCcdPort->Text=IntToStr(Ini->ReadInteger("TopCCD", "Port", 5001));
        if(chkTopCcdEnable!=NULL)
            chkTopCcdEnable->Checked=Ini->ReadBool("TopCCD", "Enable", true);
    }
    __finally
    {
        delete Ini;
    }
    CosFunction.bUseTopCcd=(chkTopCcdEnable!=NULL) ? chkTopCcdEnable->Checked : true;
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SaveTopCcdSettings()
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString Address;
    int Port;

    FileName=GetTopCcdIniFileName();
    ForceDirectories(ExtractFilePath(FileName));
    Address=(edTopCcdIP!=NULL) ? edTopCcdIP->Text.Trim() : AnsiString("172.16.8.89");
    if(Address==AnsiString(""))
        Address="172.16.8.89";
    Port=ReadEditInt(edTopCcdPort, 5001);
    if(Port<=0 || Port>65535)
        Port=5001;

    Ini=new TIniFile(FileName);
    try
    {
        Ini->WriteString("TopCCD", "Address", Address);
        Ini->WriteInteger("TopCCD", "Port", Port);
        Ini->WriteBool("TopCCD", "Enable", (chkTopCcdEnable!=NULL) ? chkTopCcdEnable->Checked : true);
    }
    __finally
    {
        delete Ini;
    }

    if(edTopCcdIP!=NULL)
        edTopCcdIP->Text=Address;
    if(edTopCcdPort!=NULL)
        edTopCcdPort->Text=IntToStr(Port);

    EnsureTopCcdSocketCreated();
    if(TopCcdSocket!=NULL)
        TopCcdSocket->SetEndpoint(Address, Port);
    CosFunction.bUseTopCcd=(chkTopCcdEnable!=NULL) ? chkTopCcdEnable->Checked : true;
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RefreshTopCcdStatus()
{
    AnsiString Code;
    bool bConnected;

    if(TopCcdSocket!=NULL)
    {
        TopCcdSocket->TopCcdPoll();
        bConnected=TopCcdSocket->IsTopCcdConnected();
        if(lblTopCcdStatusConn!=NULL)
            lblTopCcdStatusConn->Caption=AnsiString("Connected: ")+(bConnected ? "YES" : "NO");
        if(lblTopCcdStatusError!=NULL)
            lblTopCcdStatusError->Caption=AnsiString("Last Error: ")+TopCcdSocket->GetLastError();
        if(TopCcdSocket->TopCcdGetResult(Code))
        {
            if(edTopCcdResult!=NULL && Code!=edTopCcdResult->Text)
            {
                edTopCcdResult->Text=Code;
                AddTopCcdLog(AnsiString("Recv 2D = ")+Code);
            }
        }
    }
    else
    {
        if(lblTopCcdStatusConn!=NULL)
            lblTopCcdStatusConn->Caption=LangT("Connected: -");
        if(lblTopCcdStatusError!=NULL)
            lblTopCcdStatusError->Caption=LangT("Last Error: not started");
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::AddTopCcdLog(AnsiString Text)
{
    if(memTopCcdLog==NULL)
        return;
    memTopCcdLog->Lines->Add(FormatDateTime("hh:nn:ss", Now())+AnsiString(" ")+Text);
    while(memTopCcdLog->Lines->Count>200)
        memTopCcdLog->Lines->Delete(0);
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnTopCcdSaveClick(TObject *Sender)
{
    (void)Sender;
    SaveTopCcdSettings();
    AddTopCcdLog("Save General.ini [TopCCD] endpoint");
    RefreshTopCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnTopCcdReloadClick(TObject *Sender)
{
    (void)Sender;
    LoadTopCcdSettings();
    EnsureTopCcdSocketCreated();
    if(TopCcdSocket!=NULL)
        TopCcdSocket->LoadConfig();
    AddTopCcdLog("Reload General.ini [TopCCD] endpoint");
    RefreshTopCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnTopCcdConnectClick(TObject *Sender)
{
    (void)Sender;
    SaveTopCcdSettings();
    EnsureTopCcdSocketCreated();
    if(TopCcdSocket!=NULL)
    {
        if(TopCcdSocket->TopCcdConnect())
            AddTopCcdLog(AnsiString("Connecting to ")+TopCcdSocket->GetAddress()+AnsiString(":")+IntToStr(TopCcdSocket->GetPort()));
        else
            AddTopCcdLog(AnsiString("Connect failed: ")+TopCcdSocket->GetLastError());
    }
    RefreshTopCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnTopCcdDisconnectClick(TObject *Sender)
{
    (void)Sender;
    if(TopCcdSocket!=NULL)
    {
        TopCcdSocket->TopCcdDisconnect();
        AddTopCcdLog("Disconnect");
    }
    RefreshTopCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnTopCcdShotClick(TObject *Sender)
{
    (void)Sender;
    EnsureTopCcdSocketCreated();
    if(TopCcdSocket!=NULL)
    {
        if(!TopCcdSocket->IsTopCcdConnected())
        {
            AddTopCcdLog("Not connected, please Connect first");
        }
        else
        {
            TopCcdSocket->TopCcdTriggerShot();
            AddTopCcdLog("Trigger shot (send LON)");
        }
    }
    RefreshTopCcdStatus();
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260612 : Color-station 2D reader maintenance page.
//Mirrors the Top CCD page (Connect/Disconnect/Trigger Shot + status + log) so the
//Color CCD can be verified manually on-machine, and adds an Enable checkbox that
//persists to [ColorCCD] Enable and drives connect/disconnect (HT172 behavior).
//Shares General.ini via GetTopCcdIniFileName().
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::LoadColorCcdSettings()
{
    TIniFile *Ini;
    AnsiString FileName;

    FileName=GetTopCcdIniFileName();
    Ini=new TIniFile(FileName);
    try
    {
        if(edColorCcdIP!=NULL)
            edColorCcdIP->Text=Ini->ReadString("ColorCCD", "Address", "172.16.8.100");
        if(edColorCcdPort!=NULL)
            edColorCcdPort->Text=IntToStr(Ini->ReadInteger("ColorCCD", "Port", 5000));
        if(chkColorCcdEnable!=NULL)
            chkColorCcdEnable->Checked=Ini->ReadBool("ColorCCD", "Enable", true);
    }
    __finally
    {
        delete Ini;
    }
    CosFunction.bUseColorCcd=(chkColorCcdEnable!=NULL) ? chkColorCcdEnable->Checked : true;
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SaveColorCcdSettings()
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString Address;
    int Port;
    bool Enable;

    FileName=GetTopCcdIniFileName();
    ForceDirectories(ExtractFilePath(FileName));
    Address=(edColorCcdIP!=NULL) ? edColorCcdIP->Text.Trim() : AnsiString("172.16.8.100");
    if(Address==AnsiString(""))
        Address="172.16.8.100";
    Port=ReadEditInt(edColorCcdPort, 5000);
    if(Port<=0 || Port>65535)
        Port=5000;
    Enable=(chkColorCcdEnable!=NULL) ? chkColorCcdEnable->Checked : true;

    Ini=new TIniFile(FileName);
    try
    {
        Ini->WriteString("ColorCCD", "Address", Address);
        Ini->WriteInteger("ColorCCD", "Port", Port);
        Ini->WriteBool("ColorCCD", "Enable", Enable);
    }
    __finally
    {
        delete Ini;
    }

    if(edColorCcdIP!=NULL)
        edColorCcdIP->Text=Address;
    if(edColorCcdPort!=NULL)
        edColorCcdPort->Text=IntToStr(Port);

    CosFunction.bUseColorCcd=Enable;
    EnsureColorCcdSocketCreated();
    if(ColorCcdSocket!=NULL)
        ColorCcdSocket->SetEndpoint(Address, Port);
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RefreshColorCcdStatus()
{
    AnsiString Code;
    bool bConnected;

    if(ColorCcdSocket!=NULL)
    {
        ColorCcdSocket->ColorCcdPoll();
        bConnected=ColorCcdSocket->IsColorCcdConnected();
        if(lblColorCcdStatusConn!=NULL)
            lblColorCcdStatusConn->Caption=AnsiString("Connected: ")+(bConnected ? "YES" : "NO");
        if(lblColorCcdStatusError!=NULL)
            lblColorCcdStatusError->Caption=AnsiString("Last Error: ")+ColorCcdSocket->GetLastError();
        if(ColorCcdSocket->ColorCcdGetResult(Code))
        {
            if(edColorCcdResult!=NULL && Code!=edColorCcdResult->Text)
            {
                edColorCcdResult->Text=Code;
                AddColorCcdLog(AnsiString("Recv 2D = ")+Code);
            }
        }
    }
    else
    {
        if(lblColorCcdStatusConn!=NULL)
            lblColorCcdStatusConn->Caption=LangT("Connected: -");
        if(lblColorCcdStatusError!=NULL)
            lblColorCcdStatusError->Caption=LangT("Last Error: not started");
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::AddColorCcdLog(AnsiString Text)
{
    if(memColorCcdLog==NULL)
        return;
    memColorCcdLog->Lines->Add(FormatDateTime("hh:nn:ss", Now())+AnsiString(" ")+Text);
    while(memColorCcdLog->Lines->Count>200)
        memColorCcdLog->Lines->Delete(0);
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnColorCcdSaveClick(TObject *Sender)
{
    (void)Sender;
    SaveColorCcdSettings();
    AddColorCcdLog("Save General.ini [ColorCCD] endpoint/enable");
    RefreshColorCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnColorCcdReloadClick(TObject *Sender)
{
    (void)Sender;
    LoadColorCcdSettings();
    EnsureColorCcdSocketCreated();
    if(ColorCcdSocket!=NULL)
        ColorCcdSocket->LoadConfig();
    AddColorCcdLog("Reload General.ini [ColorCCD] endpoint/enable");
    RefreshColorCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnColorCcdConnectClick(TObject *Sender)
{
    (void)Sender;
    SaveColorCcdSettings();
    EnsureColorCcdSocketCreated();
    if(ColorCcdSocket!=NULL)
    {
        if(ColorCcdSocket->ColorCcdConnect())
            AddColorCcdLog(AnsiString("Connecting to ")+ColorCcdSocket->GetAddress()+AnsiString(":")+IntToStr(ColorCcdSocket->GetPort()));
        else
            AddColorCcdLog(AnsiString("Connect failed: ")+ColorCcdSocket->GetLastError());
    }
    RefreshColorCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnColorCcdDisconnectClick(TObject *Sender)
{
    (void)Sender;
    if(ColorCcdSocket!=NULL)
    {
        ColorCcdSocket->ColorCcdDisconnect();
        AddColorCcdLog("Disconnect");
    }
    RefreshColorCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnColorCcdShotClick(TObject *Sender)
{
    (void)Sender;
    EnsureColorCcdSocketCreated();
    if(ColorCcdSocket!=NULL)
    {
        if(!ColorCcdSocket->IsColorCcdConnected())
        {
            AddColorCcdLog("Not connected, please Connect first");
        }
        else
        {
            ColorCcdSocket->ColorCcdTriggerShot();
            AddColorCcdLog("Trigger shot (send LON)");
        }
    }
    RefreshColorCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::chkTopCcdEnableClick(TObject *Sender)
{
    (void)Sender;
    //AI(HT160S-Maintainer) 20260624 : per-CCD Enable. Persists to [TopCCD] Enable
    //and drives CosFunction.bUseTopCcd. OFF -> aLoader feeds simulated Top CCD 2D
    //codes instead of polling the camera (REALLY only; HAS_TRAY/DUMMY always sim).
    SaveTopCcdSettings();
    EnsureTopCcdSocketCreated();
    if(chkTopCcdEnable!=NULL && chkTopCcdEnable->Checked)
    {
        if(TopCcdSocket!=NULL)
            TopCcdSocket->TopCcdConnect();
        AddTopCcdLog("Enable Top CCD -> connect");
    }
    else
    {
        if(TopCcdSocket!=NULL)
            TopCcdSocket->TopCcdDisconnect();
        AddTopCcdLog("Disable Top CCD -> use simulated 2D");
    }
    RefreshTopCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::chkColorCcdEnableClick(TObject *Sender)
{
    (void)Sender;
    //AI(HT160S-Maintainer) 20260612 : mirror HT172 Update2DParameter : the Enable
    //checkbox drives connect/disconnect and persists to [ColorCCD] Enable.
    SaveColorCcdSettings();
    EnsureColorCcdSocketCreated();
    if(chkColorCcdEnable!=NULL && chkColorCcdEnable->Checked)
    {
        if(ColorCcdSocket!=NULL)
            ColorCcdSocket->ColorCcdConnect();
        AddColorCcdLog("Enable Color CCD -> connect");
    }
    else
    {
        if(ColorCcdSocket!=NULL)
            ColorCcdSocket->ColorCcdDisconnect();
        AddColorCcdLog("Disable Color CCD -> disconnect");
    }
    RefreshColorCcdStatus();
}
//---------------------------------------------------------------------------
//AI(general) 20260611 : Lot WebAPI maintenance page (fetch 2D/Bin by Lot name)
static bool bLotApiResultPending=false;
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::LoadLotWebApiSettings()
{
    EnsureLotWebApiClientCreated();
    if(LotWebApiClient!=NULL)
    {
        LotWebApiClient->LoadConfig();
        if(edWebapiPath!=NULL)
            edWebapiPath->Text=LotWebApiClient->GetBaseUrl();
        if(chkLotApiUsePull!=NULL)
            chkLotApiUsePull->Checked=LotWebApiClient->GetUsePull();
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SaveLotWebApiSettings()
{
    AnsiString Url;

    Url=(edWebapiPath!=NULL) ? edWebapiPath->Text.Trim() : AnsiString("");
    if(Url==AnsiString(""))
        Url="http://127.0.0.1:8160/lot/";

    EnsureLotWebApiClientCreated();
    if(LotWebApiClient!=NULL)
    {
        LotWebApiClient->SetBaseUrl(Url);
        if(chkLotApiUsePull!=NULL)
            LotWebApiClient->SetUsePull(chkLotApiUsePull->Checked);
        LotWebApiClient->SaveConfig();
        Url=LotWebApiClient->GetBaseUrl();
    }
    if(edWebapiPath!=NULL)
        edWebapiPath->Text=Url;
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RefreshLotWebApiStatus()
{
    AnsiString Body;
    bool bOk;
    int HttpStatus;

    if(lblLotApiUrl!=NULL && edWebapiPath!=NULL)
        lblLotApiUrl->Caption=AnsiString("URL: ")+edWebapiPath->Text;

    if(LotWebApiClient==NULL)
    {
        if(lblLotApiStatus!=NULL)
            lblLotApiStatus->Caption=LangT("State: not started");
        return;
    }

    if(lblLotApiError!=NULL)
        lblLotApiError->Caption=AnsiString("Last Error: ")+LotWebApiClient->GetLastError();

    if(LotWebApiClient->IsBusy())
    {
        if(lblLotApiStatus!=NULL)
            lblLotApiStatus->Caption=AnsiString("State: busy (")+LotWebApiClient->GetCurrentLot()+AnsiString(")");
    }

    // Consume the result exactly once when a manual fetch is pending.
    if(bLotApiResultPending && LotWebApiClient->GetResult(Body, bOk, HttpStatus))
    {
        bLotApiResultPending=false;
        if(lblLotApiStatus!=NULL)
        {
            lblLotApiStatus->Caption=AnsiString("State: done http=")+IntToStr(HttpStatus)+
                AnsiString(bOk ? " OK" : " (no data)");
        }
        if(memLotApiResult!=NULL)
        {
            memLotApiResult->Clear();
            memLotApiResult->Lines->Add(AnsiString("HTTP ")+IntToStr(HttpStatus)+
                AnsiString(bOk ? " OK" : " FAIL"));
            memLotApiResult->Lines->Add(Body);
        }
        AddLotWebApiLog(AnsiString("Fetch done http=")+IntToStr(HttpStatus)+
            AnsiString(" len=")+IntToStr(Body.Length()));
    }
    else if(!LotWebApiClient->IsBusy() && !bLotApiResultPending)
    {
        if(lblLotApiStatus!=NULL && lblLotApiStatus->Caption.Pos("busy")>0)
            lblLotApiStatus->Caption=LangT("State: idle");
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::AddLotWebApiLog(AnsiString Text)
{
    if(memLotApiLog==NULL)
        return;
    memLotApiLog->Lines->Add(FormatDateTime("hh:nn:ss", Now())+AnsiString(" ")+Text);
    while(memLotApiLog->Lines->Count>200)
        memLotApiLog->Lines->Delete(0);
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RefreshAmrStatus()
{
    // AI(ht160s-agv) 20260625 : live dump of the AGV coordinator lock/handshake
    // state into the AMR maintenance tab. AgvCoord is a global object (not a
    // pointer), so guard only the memo control. DescribeAgvState() is read-only.
    if(memAmrStatus==NULL)
        return;
    memAmrStatus->Lines->Text = AgvCoord.DescribeAgvState();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnLotApiSaveClick(TObject *Sender)
{
    AnsiString Url;

    (void)Sender;
    SaveLotWebApiSettings();
    Url=(edWebapiPath!=NULL) ? edWebapiPath->Text : AnsiString("");
    AddLotWebApiLog(AnsiString("Save General.ini [LotWebApi] BaseUrl=")+Url);
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnLotApiReloadClick(TObject *Sender)
{
    (void)Sender;
    LoadLotWebApiSettings();
    AddLotWebApiLog("Reload General.ini [LotWebApi] BaseUrl");
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnLotApiFetchClick(TObject *Sender)
{
    AnsiString Lot;

    (void)Sender;
    EnsureLotWebApiClientCreated();
    if(LotWebApiClient==NULL)
        return;

    // Save current URL first so the test uses the on-screen endpoint.
    SaveLotWebApiSettings();

    Lot=(edLotApiTestLot!=NULL) ? edLotApiTestLot->Text.Trim() : AnsiString("");
    if(Lot==AnsiString(""))
    {
        AddLotWebApiLog("Fetch aborted: empty Lot ID");
        return;
    }

    if(LotWebApiClient->IsBusy())
    {
        AddLotWebApiLog("Fetch aborted: a request is still in flight");
        return;
    }

    if(memLotApiResult!=NULL)
        memLotApiResult->Clear();

    if(LotWebApiClient->StartLotRequest(Lot))
    {
        bLotApiResultPending=true;
        if(lblLotApiStatus!=NULL)
            lblLotApiStatus->Caption=AnsiString("State: busy (")+Lot+AnsiString(")");
        AddLotWebApiLog(AnsiString("Fetch start Lot=")+Lot);
    }
    else
    {
        AddLotWebApiLog(AnsiString("Fetch start failed: ")+LotWebApiClient->GetLastError());
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::InitializeTowerLightPanels()
{
    int RowIndex;
    int ColorIndex;

    TowerLightLeds[0][0]=RGB00;
    TowerLightLeds[0][1]=RGB01;
    TowerLightLeds[0][2]=RGB02;
    TowerLightLeds[1][0]=RGB10;
    TowerLightLeds[1][1]=RGB11;
    TowerLightLeds[1][2]=RGB12;
    TowerLightLeds[2][0]=RGB20;
    TowerLightLeds[2][1]=RGB21;
    TowerLightLeds[2][2]=RGB22;
    TowerLightLeds[3][0]=RGB30;
    TowerLightLeds[3][1]=RGB31;
    TowerLightLeds[3][2]=RGB32;
    TowerLightLeds[4][0]=RGB40;
    TowerLightLeds[4][1]=RGB41;
    TowerLightLeds[4][2]=RGB42;
    TowerLightLeds[5][0]=RGB50;
    TowerLightLeds[5][1]=RGB51;
    TowerLightLeds[5][2]=RGB52;

    for(RowIndex=0; RowIndex<TOWER_LIGHT_ROW_COUNT; RowIndex++)
    {
        for(ColorIndex=0; ColorIndex<TOWER_LIGHT_COLOR_COUNT; ColorIndex++)
            ConfigureTowerLightLed(TowerLightLeds[RowIndex][ColorIndex], ColorIndex);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::LoadMaintenanceSettings()
{
    TIniFile *Ini;
    AnsiString FileName;
    TRadioGroup *RadioGroups[TOWER_LIGHT_ROW_COUNT];
    int RowIndex;
    int Value;

    LoadTowerLightSettings();
    FileName=GetTowerLightIniFileName();
    if(FileExists(FileName))
    {
        RadioGroups[0]=RadioGroup2;
        RadioGroups[1]=RadioGroup3;
        RadioGroups[2]=RadioGroup4;
        RadioGroups[3]=RadioGroup5;
        RadioGroups[4]=RadioGroup6;
        RadioGroups[5]=RadioGroup7;

        Ini=new TIniFile(FileName);
        for(RowIndex=0; RowIndex<TOWER_LIGHT_ROW_COUNT; RowIndex++)
        {
            if(RadioGroups[RowIndex]!=NULL)
            {
                Value=Ini->ReadInteger(TOWER_LIGHT_INI_GROUP, RadioGroups[RowIndex]->Name,
                    RadioGroups[RowIndex]->ItemIndex);
                if(Value>=0 && Value<RadioGroups[RowIndex]->Items->Count)
                    RadioGroups[RowIndex]->ItemIndex=Value;
            }
        }
        delete Ini;
    }
    ApplyTowerLightConfigToLeds();
    LoadHardwareSettings();
    LoadMCUDisplaySettings();
    RefreshMCUDisplayStatus();
    LoadTopCcdSettings();
    RefreshTopCcdStatus();
    LoadColorCcdSettings();
    RefreshColorCcdStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SaveMaintenanceSettings()
{
    TIniFile *Ini;
    TRadioGroup *RadioGroups[TOWER_LIGHT_ROW_COUNT];
    int RowIndex;

    SaveTowerLightSettings();
    Ini=new TIniFile(GetTowerLightIniFileName());
    RadioGroups[0]=RadioGroup2;
    RadioGroups[1]=RadioGroup3;
    RadioGroups[2]=RadioGroup4;
    RadioGroups[3]=RadioGroup5;
    RadioGroups[4]=RadioGroup6;
    RadioGroups[5]=RadioGroup7;
    for(RowIndex=0; RowIndex<TOWER_LIGHT_ROW_COUNT; RowIndex++)
    {
        if(RadioGroups[RowIndex]!=NULL)
            Ini->WriteInteger(TOWER_LIGHT_INI_GROUP, RadioGroups[RowIndex]->Name, RadioGroups[RowIndex]->ItemIndex);
    }
    delete Ini;
    SaveHardwareSettings();
    SaveMCUDisplaySettings();
    SaveTopCcdSettings();
    SaveColorCcdSettings();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::LoadHardwareSettings()
{
    GeneralSetting.Load();
    BinAreaMap.LoadDefault();
    bLoadingHardwareSettings=true;
    if(chkHardwareColorBinArea!=NULL)
        chkHardwareColorBinArea->Checked=GeneralSetting.bColorBinAreaInstalled;
    if(chkUseAMR!=NULL)
        chkUseAMR->Checked=GeneralSetting.bUseAMR;
    if(cbBinPanelType!=NULL)
    {
        int idx=GeneralSetting.iBinDispPanelType;
        if(idx<0 || idx>1) idx=0;
        cbBinPanelType->ItemIndex=idx;
    }
    if(cbCommType!=NULL)
        cbCommType->Checked=GeneralSetting.bBinDispUseMyComm;
    //AI(ht160s-ccd-teach-test) 20260628 : chkUseTrayDatumModel unwired - GeneralSetting.bUseTrayDatumModel
    //was removed (replaced by ini-only iSortArmXDatumBias/iSortArmYDatumBias). Remove the dead checkbox
    //from the maintenance form when finishing the datum-bias refactor.
    if(chkUseLotBinMode!=NULL)
        chkUseLotBinMode->Checked=GeneralSetting.bUseLotBinSortMode;
    {
        TCheckBox *AutoChk[6];
        int a;
        AutoChk[0]=chkAutoEnable1; AutoChk[1]=chkAutoEnable2; AutoChk[2]=chkAutoEnable3;
        AutoChk[3]=chkAutoEnable4; AutoChk[4]=chkAutoEnable5; AutoChk[5]=chkAutoEnable6;
        for(a=0; a<6; a++)
            if(AutoChk[a]!=NULL)
                AutoChk[a]->Checked=GeneralSetting.bAutoEnabled[a];
    }
    {
        TCheckBox *SuckChk[4];
        int s;
        SuckChk[0]=chkSuckEnable1; SuckChk[1]=chkSuckEnable2;
        SuckChk[2]=chkSuckEnable3; SuckChk[3]=chkSuckEnable4;
        for(s=0; s<4; s++)
            if(SuckChk[s]!=NULL)
                SuckChk[s]->Checked=GeneralSetting.bSuckerEnabled[s];
    }
    //AI(ht160s-maintainer) 20260624 : Loader safe distance is stored as 1/100mm
    //(teach/encoder domain) but edited in mm; show mm = stored/100. Existing
    //below-range configs are displayed verbatim; only an operator re-save changes them.
    if(edLoaderSafeDistance!=NULL)
    {
        AnsiString S;
        S.sprintf("%.2f", (double)GeneralSetting.iLoaderYSafeDistance/100.0);
        edLoaderSafeDistance->Text=S;
    }
    //AI(ht160s-statusbar) 20260624 : load machine identity into the edit fields
    //(persisted in General.ini [MachineIdentity] by GeneralSetting). Guarded for
    //NULL in case the DFM block is absent on an older form file.
    if(edMachineModel!=NULL)
        edMachineModel->Text=GeneralSetting.sMachineModel;
    if(edHandlerID!=NULL)
        edHandlerID->Text=GeneralSetting.sHandlerID;
    if(edSerialNo!=NULL)
        edSerialNo->Text=GeneralSetting.sSerialNo;
    if(edSettle0!=NULL) edSettle0->Text=IntToStr(GeneralSetting.iEmptyDestackSettleMs);
    if(edSettle1!=NULL) edSettle1->Text=IntToStr(GeneralSetting.iColorDestackSettleMs);
    if(edSettle2!=NULL) edSettle2->Text=IntToStr(GeneralSetting.iLoaderDestackSettleMs);
    if(edSettle3!=NULL) edSettle3->Text=IntToStr(GeneralSetting.iAutoPushConfirmSettleMs);
    if(edSettle4!=NULL) edSettle4->Text=IntToStr(GeneralSetting.iAutoDischargePostYSettleMs);
    if(edSettle5!=NULL) edSettle5->Text=IntToStr(GeneralSetting.iAutoFrontRiseDwellMs);
    if(edSettle6!=NULL) edSettle6->Text=IntToStr(GeneralSetting.iAutoCleanOutRiseDwellMs);
    if(edSettle7!=NULL) edSettle7->Text=IntToStr(GeneralSetting.iTrayArmClampSettleMs);
    if(edSettle8!=NULL) edSettle8->Text=IntToStr(GeneralSetting.iEmptyFeedClampSettleMs);
    if(edSettle9!=NULL) edSettle9->Text=IntToStr(GeneralSetting.iColorFeedClampSettleMs);
    bLoadingHardwareSettings=false;
    RefreshHardwareSettingsStatus();
    ApplyHardwareEditLock();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SaveHardwareSettings()
{
    if(chkHardwareColorBinArea!=NULL)
        GeneralSetting.bColorBinAreaInstalled=chkHardwareColorBinArea->Checked;
    if(chkUseAMR!=NULL)
        GeneralSetting.bUseAMR=chkUseAMR->Checked;
    if(cbBinPanelType!=NULL)
    {
        int idx=cbBinPanelType->ItemIndex;
        if(idx<0) idx=0;
        GeneralSetting.iBinDispPanelType=idx;
    }
    if(cbCommType!=NULL)
        GeneralSetting.bBinDispUseMyComm=cbCommType->Checked;
    //AI(ht160s-ccd-teach-test) 20260628 : chkUseTrayDatumModel unwired (bUseTrayDatumModel removed).
    if(chkUseLotBinMode!=NULL)
        GeneralSetting.bUseLotBinSortMode=chkUseLotBinMode->Checked;
    {
        TCheckBox *AutoChk[6];
        int a;
        AutoChk[0]=chkAutoEnable1; AutoChk[1]=chkAutoEnable2; AutoChk[2]=chkAutoEnable3;
        AutoChk[3]=chkAutoEnable4; AutoChk[4]=chkAutoEnable5; AutoChk[5]=chkAutoEnable6;
        for(a=0; a<6; a++)
            if(AutoChk[a]!=NULL)
                GeneralSetting.bAutoEnabled[a]=AutoChk[a]->Checked;
    }
    {
        TCheckBox *SuckChk[4];
        int s;
        SuckChk[0]=chkSuckEnable1; SuckChk[1]=chkSuckEnable2;
        SuckChk[2]=chkSuckEnable3; SuckChk[3]=chkSuckEnable4;
        for(s=0; s<4; s++)
            if(SuckChk[s]!=NULL)
                GeneralSetting.bSuckerEnabled[s]=SuckChk[s]->Checked;
    }
    //AI(ht160s-statusbar) 20260624 : capture machine identity from the edits before
    //GeneralSetting.Save(), then push it to the cmydef globals + status-bar panels.
    if(edMachineModel!=NULL)
        GeneralSetting.sMachineModel=edMachineModel->Text;
    if(edHandlerID!=NULL)
        GeneralSetting.sHandlerID=edHandlerID->Text;
    if(edSerialNo!=NULL)
        GeneralSetting.sSerialNo=edSerialNo->Text;
    GeneralSetting.Save();
    BinAreaMap.LoadDefault();
    UpdateMachineIdentity();
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
//AI(ht160s-lotbin) 20260615 : Lock the Loader/Unloader hardware checkboxes while a
//lot is running (MachineRun.bRunning, set at Lot Start, cleared at Lot End). These
//are commissioning / sort-routing facts; changing them mid-lot would corrupt the
//in-progress sort. bRunning is in-memory only, so a restart (without a started lot)
//leaves the page editable again - matching the agreed "lock from Start to End" scope.
void __fastcall TfMaintenance::ApplyHardwareEditLock()
{
    TCheckBox *Locked[13];
    bool bEnable;
    int i;

    bEnable=(MachineRun.bRunning==false);
    Locked[0]=chkHardwareColorBinArea;
    Locked[1]=chkUseAMR;
    Locked[2]=chkUseLotBinMode;
    Locked[3]=chkAutoEnable1; Locked[4]=chkAutoEnable2; Locked[5]=chkAutoEnable3;
    Locked[6]=chkAutoEnable4; Locked[7]=chkAutoEnable5; Locked[8]=chkAutoEnable6;
    Locked[9]=chkSuckEnable1; Locked[10]=chkSuckEnable2;
    Locked[11]=chkSuckEnable3; Locked[12]=chkSuckEnable4;
    for(i=0; i<13; i++)
        if(Locked[i]!=NULL)
            Locked[i]->Enabled=bEnable;

    if(pnlHardwareHeader!=NULL)
    {
        if(bEnable)
            pnlHardwareHeader->Caption=LangT("Hardware install setup");
        else
            pnlHardwareHeader->Caption=LangT("Hardware install setup (locked - lot running, end lot to edit)");
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RefreshHardwareSettingsStatus()
{
    if(lblHardwareErrorCode1000!=NULL)
        lblHardwareErrorCode1000->Caption=AnsiString("1000 = 2D scan fail -> ")+BinAreaMap.GetAreaName(BinAreaMap.GetAreaByErrorBin(HT160_BIN_ERROR_2D_SCAN_FAIL));
    if(lblHardwareErrorCode1001!=NULL)
        lblHardwareErrorCode1001->Caption=AnsiString("1001 = no bin setting -> ")+BinAreaMap.GetAreaName(BinAreaMap.GetAreaByErrorBin(HT160_BIN_ERROR_NO_BIN_SETTING));
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260615 : convert a display-label string to the bin
//value encoding: -1 blank(X), 0..99 digits, 100..125 A..Z.
static int BinTextToDispValue(AnsiString s)
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
//AI(ht160s-maintainer) 20260615 : page repurposed from TCP MCU to COM bin
//display. Settings live in General.ini [BinDisplay] (GeneralSetting). The old
//TCP MaxQueue field has been removed; edMCUPort is reused as the Baud field.
void __fastcall TfMaintenance::LoadMCUDisplaySettings()
{
    if(chkMCUEnabled!=NULL)
        chkMCUEnabled->Checked=GeneralSetting.bBinDisplayInstalled;
    if(edMCUIP!=NULL)
        edMCUIP->Text=GeneralSetting.sBinDispComPort;
    if(edMCUReconnect!=NULL)
        edMCUReconnect->Text=IntToStr(GeneralSetting.iBinDispDelaySec);
    // edMCUPort is the old TCP-port edit, repurposed as the Baud field.
    if(edMCUPort!=NULL)
        edMCUPort->Text=IntToStr(GeneralSetting.iBinDispBaud);
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SaveMCUDisplaySettings()
{
    int Delay;

    GeneralSetting.bBinDisplayInstalled=(chkMCUEnabled!=NULL && chkMCUEnabled->Checked);
    if(edMCUIP!=NULL && edMCUIP->Text.Trim()!=AnsiString(""))
        GeneralSetting.sBinDispComPort=edMCUIP->Text.Trim();
    Delay=ReadEditInt(edMCUReconnect, 5);
    if(Delay<1)
        Delay=1;
    GeneralSetting.iBinDispDelaySec=Delay;

    // edMCUPort is a Baud dropdown (combo). Fall back to 9600 (HT9046 standard) on junk.
    int Baud;
    Baud=9600;
    if(edMCUPort!=NULL && edMCUPort->Text.Trim()!=AnsiString(""))
        Baud=atoi(edMCUPort->Text.c_str());
    if(Baud<300)
        Baud=9600;
    GeneralSetting.iBinDispBaud=Baud;
    GeneralSetting.Save();

    if(edMCUReconnect!=NULL)
        edMCUReconnect->Text=IntToStr(Delay);
    if(edMCUPort!=NULL)
        edMCUPort->Text=IntToStr(Baud);
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RestartMCUDisplay()
{
    // Re-apply COM endpoint + per-unit labels and (re)start the controller.
    EnsureComPortCreated(Application);
    if(fComPort!=NULL)
        fComPort->ConfigureBinDisplay();
    RefreshMCUDisplayStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RefreshMCUDisplayStatus()
{
    bool bCreated;

    bCreated=(HSys.BinDisCtrl!=NULL);
    if(lblMCUStatusEnabled!=NULL)
        lblMCUStatusEnabled->Caption=AnsiString("Installed: ")+(GeneralSetting.bBinDisplayInstalled ? "YES" : "NO");
    if(lblMCUStatusConnected!=NULL)
        lblMCUStatusConnected->Caption=AnsiString("COM: ")+(bCreated ? HSys.BinDisCtrl->GetComPort() : AnsiString("-"));
    if(lblMCUStatusQueue!=NULL)
        lblMCUStatusQueue->Caption=AnsiString("Status: ")+(bCreated ? HSys.BinDisCtrl->GetRunStatus() : AnsiString("-"));
    if(lblMCUStatusError!=NULL)
        lblMCUStatusError->Caption=AnsiString("Units: ")+(bCreated ? IntToStr(HSys.BinDisCtrl->GetTotalInstalledUnit()) : AnsiString("0"));
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::AddMCULog(AnsiString Text)
{
    if(memMCULog==NULL)
        return;
    memMCULog->Lines->Add(FormatDateTime("hh:nn:ss", Now())+AnsiString(" ")+Text);
    while(memMCULog->Lines->Count>200)
        memMCULog->Lines->Delete(0);
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::ApplyTowerLightConfigToLeds()
{
    TALed *Led;
    int RowIndex;
    int ColorIndex;
    int State;

    for(RowIndex=0; RowIndex<TOWER_LIGHT_ROW_COUNT; RowIndex++)
    {
        for(ColorIndex=0; ColorIndex<TOWER_LIGHT_COLOR_COUNT; ColorIndex++)
        {
            Led=TowerLightLeds[RowIndex][ColorIndex];
            if(Led==NULL)
                continue;
            State=GetTowerLightConfigState(RowIndex, ColorIndex);
            Led->Blink=false;   //AI(HT160S-Maintainer) 20260622 : no blink on the tower-light grid -- show BLINK config as steady-on
            Led->Value=(State!=TOWER_LIGHT_STATE_OFF);
            Led->Invalidate();
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SetTowerLightState(int RowIndex, int ColorIndex, int State)
{
    if(RowIndex<0 || RowIndex>=TOWER_LIGHT_ROW_COUNT)
        return;
    if(ColorIndex<0 || ColorIndex>=TOWER_LIGHT_COLOR_COUNT)
        return;

    SetTowerLightConfigState(RowIndex, ColorIndex, State);
    ApplyTowerLightConfigToLeds();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RefreshTowerLightPanel(int RowIndex, int ColorIndex)
{
    TALed *Led;

    if(RowIndex<0 || RowIndex>=TOWER_LIGHT_ROW_COUNT)
        return;
    if(ColorIndex<0 || ColorIndex>=TOWER_LIGHT_COLOR_COUNT)
        return;

    Led=TowerLightLeds[RowIndex][ColorIndex];
    if(Led==NULL)
        return;
    Led->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RefreshTowerLightPanels()
{
    int RowIndex;
    int ColorIndex;

    for(RowIndex=0; RowIndex<TOWER_LIGHT_ROW_COUNT; RowIndex++)
    {
        for(ColorIndex=0; ColorIndex<TOWER_LIGHT_COLOR_COUNT; ColorIndex++)
            RefreshTowerLightPanel(RowIndex, ColorIndex);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::tmrTowerLightBlinkTimer(TObject *Sender)
{
    (void)Sender;
    RefreshTowerLightPanels();
    RefreshMCUDisplayStatus();
    RefreshTopCcdStatus();
    RefreshColorCcdStatus();
    RefreshLotWebApiStatus();
    RefreshAmrStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::OpenWorkFile()
{
    SetMaintenanceWorkFileName(AnsiString(""));
    bTowerLightConfigLoaded=false;
    LoadMaintenanceSettings();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SaveWorkFile(AnsiString S)
{
    SetMaintenanceWorkFileName(S);
    ForceDirectories(ExtractFilePath(GetTowerLightIniFileName()));
    SaveMaintenanceSettings();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RegisterMaintenancePages()
{
    TMaintenancePageDef PageDefs[]={
        {tsMaintTowerLight,  spbMaintTowerLight,  maShowPage, false},
        {tsMaintPassword,    spbMaintPassword,    maShowPage, false},
        {tsMaintSoftSimu,    spbMaintSoftSimu,    maShowPage, false},
        {tsMaintFunctionDef, spbMaintFunctionDef, maShowPage, false},
        {tsMaintHardware,    spbMaintHardware,    maShowPage, false},
        {tsMaintIO,          spbMaintIO,          maOpenIOView, false},
        {tsMaintTeach,       spbMaintTeach,       maOpenTeach, false},
        {tsMaintMotor,       spbMaintMotor,       maOpenMotorTest, false},
        {tsMaintCOM,         spbMaintCOM,         maOpenComPort, false},
        {tsMaintMCUDisplay,  spbMaintMCUDisplay,  maShowPage, false},
        {tsMaintTopCcd,      spbMaintTopCcd,      maShowPage, false},
        {tsMaintColorCcd,    spbMaintColorCcd,    maShowPage, false},
        {tsMaintLotApi,      spbMaintLotApi,      maShowPage, false},
        {tsMaintSECS,        spbMaintSECS,        maOpenSecs, false},
        {NULL,               spbMaintExit,        maCloseForm, true}
    };
    int PageIndex;
    int PageCount;

    PageCount=sizeof(PageDefs)/sizeof(PageDefs[0]);
    if(PageCount>MAX_MAINTENANCE_MENU_COUNT)
        PageCount=MAX_MAINTENANCE_MENU_COUNT;

    iMaintenanceMenuCount=PageCount;
    for(PageIndex=0; PageIndex<iMaintenanceMenuCount; PageIndex++)
    {
        MenuPages[PageIndex]=PageDefs[PageIndex].Page;
        MenuButtons[PageIndex]=PageDefs[PageIndex].Button;
        MenuActions[PageIndex]=PageDefs[PageIndex].Action;
        MenuBottomPins[PageIndex]=PageDefs[PageIndex].PinBottom;

        if(MenuPages[PageIndex]!=NULL)
            MenuPages[PageIndex]->TabVisible=false;

        if(MenuButtons[PageIndex]!=NULL)
        {
            MenuButtons[PageIndex]->Tag=PageIndex;
            MenuButtons[PageIndex]->AllowAllUp=true;
            MenuButtons[PageIndex]->GroupIndex=1;
            MenuButtons[PageIndex]->OnClick=spbMaintenanceMenuClick;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::LayoutMaintenanceButtons()
{
    const int ButtonLeft=8;
    const int ButtonTop=8;
    const int ButtonWidth=180;
    const int ButtonHeight=50;
    const int ButtonGap=6;
    int PageIndex;
    int TopPos;

    TopPos=ButtonTop;
    for(PageIndex=0; PageIndex<iMaintenanceMenuCount; PageIndex++)
    {
        if(MenuButtons[PageIndex]==NULL)
            continue;

        if(MenuBottomPins[PageIndex])
        {
            MenuButtons[PageIndex]->SetBounds(ButtonLeft, pnlMenu->ClientHeight-ButtonHeight-ButtonTop, ButtonWidth, ButtonHeight);
        }
        else
        {
            MenuButtons[PageIndex]->SetBounds(ButtonLeft, TopPos, ButtonWidth, ButtonHeight);
            TopPos += ButtonHeight + ButtonGap;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SelectMaintenancePage(int PageIndex)
{
    if(PageIndex<0 || PageIndex>=iMaintenanceMenuCount)
        return;
    if(MenuActions[PageIndex]==maCloseForm)
    {
        Close();
        return;
    }
    if(MenuActions[PageIndex]==maOpenIOView)
    {
        OpenIOView(MenuButtons[PageIndex]);
        return;
    }
    if(MenuActions[PageIndex]==maOpenComPort)
    {
        OpenComPort(MenuButtons[PageIndex]);
        return;
    }
    if(MenuActions[PageIndex]==maOpenTeach)
    {
        OpenTeach(MenuButtons[PageIndex]);
        return;
    }
    if(MenuActions[PageIndex]==maOpenMotorTest)
    {
        OpenMotorTest(MenuButtons[PageIndex]);
        return;
    }
    if(MenuActions[PageIndex]==maOpenSecs)
    {
        OpenSecsGemLog(MenuButtons[PageIndex]);
        return;
    }

    if(pcMaintenance==NULL || MenuPages[PageIndex]==NULL || MenuButtons[PageIndex]==NULL)
        return;

    pcMaintenance->ActivePage=MenuPages[PageIndex];
    if(MenuPages[PageIndex]==tsMaintPassword)
        ShowPasswordPage();   //AI(ht160s-password) 20260624 : build-once + refresh + level lock
    pnlTitle->Caption=MenuPages[PageIndex]->Caption;
    MenuButtons[PageIndex]->Down=true;
    LastClickButton=MenuButtons[PageIndex];
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::spbMaintenanceMenuClick(TObject *Sender)
{
    TSpeedButton *Button;

    Button=dynamic_cast<TSpeedButton *>(Sender);
    if(Button==NULL)
        return;

    SelectMaintenancePage(Button->Tag);
}
//---------------------------------------------------------------------------
//AI(poka-yoke) 20260616 : run-state lock for maintenance screens. While the
//  machine is running, disable the menu buttons that open setup/diagnostic
//  screens (IO View / Teach / Motor Test / Com Port) so the operator simply
//  cannot click them. This is a pure visual interlock - it never calls
//  ShowMyMessage (which would DecStopAllMotor + clear SystemStart and stop the
//  machine). SECS log is intentionally left enabled (its EC editing is
//  idle-guarded internally). Called every cycle from UpdateRunControlFlag so it
//  self-heals when the machine stops. The old silent-return guards inside each
//  Open* stay as a harmless backstop.
void __fastcall TfMaintenance::UpdateRunStateLock()
{
    int PageIndex;
    bool bRunning;
    bool bLocked;

    bRunning=HSys.Sys.SystemStart;
    for(PageIndex=0; PageIndex<iMaintenanceMenuCount; PageIndex++)
    {
        if(MenuButtons[PageIndex]==NULL)
            continue;
        bLocked=(MenuActions[PageIndex]==maOpenIOView ||
                 MenuActions[PageIndex]==maOpenTeach ||
                 MenuActions[PageIndex]==maOpenMotorTest ||
                 MenuActions[PageIndex]==maOpenComPort);
        if(bLocked)
            MenuButtons[PageIndex]->Enabled=(bRunning==false);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::OpenIOView(TSpeedButton *Button)
{
    TSpeedButton *PreviousButton;

    PreviousButton=LastClickButton;
    if(HSys.Sys.SystemStart)
    {
        if(Button!=NULL)
            Button->Down=false;
        if(PreviousButton!=NULL)
            PreviousButton->Down=true;
        return;
    }
//    RecordProcess("Enter IO");
//    EventReport(SECS_EVENT.EnterIOPage);
    if(fiosetview==NULL)
        fiosetview=new Tfiosetview(this);

    //AI 20260619 : Maintenance open -> normal restore-on-close (IC->force / else ask).
    fiosetview->bFromTeach=false;
    if(fiosetview->Visible)
        fiosetview->BringToFront();
    else
        fiosetview->ShowModal();

    if(Button!=NULL)
        Button->Down=false;
    if(PreviousButton!=NULL)
        PreviousButton->Down=true;
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::OpenTeach(TSpeedButton *Button)
{
    TSpeedButton *PreviousButton;

    PreviousButton=LastClickButton;
    if(HSys.Sys.SystemStart)
    {
        if(Button!=NULL)
            Button->Down=false;
        if(PreviousButton!=NULL)
            PreviousButton->Down=true;
        return;
    }

    if(fTeach==NULL)
        fTeach=new TfTeach(this);

    if(fTeach->Visible)
        fTeach->BringToFront();
    else
        fTeach->ShowModal();

    if(Button!=NULL)
        Button->Down=false;
    if(PreviousButton!=NULL)
        PreviousButton->Down=true;
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::OpenMotorTest(TSpeedButton *Button)
{
    TSpeedButton *PreviousButton;

    PreviousButton=LastClickButton;
    if(HSys.Sys.SystemStart)
    {
        if(Button!=NULL)
            Button->Down=false;
        if(PreviousButton!=NULL)
            PreviousButton->Down=true;
        return;
    }

    if(fMotorTest==NULL)
        fMotorTest=new TfMotorTest(this);

    if(fMotorTest->Visible)
        fMotorTest->BringToFront();
    else
        fMotorTest->ShowModal();

    if(Button!=NULL)
        Button->Down=false;
    if(PreviousButton!=NULL)
        PreviousButton->Down=true;
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::OpenComPort(TSpeedButton *Button)
{
    TSpeedButton *PreviousButton;

    PreviousButton=LastClickButton;
    if(HSys.Sys.SystemStart)
    {
        if(Button!=NULL)
            Button->Down=false;
        if(PreviousButton!=NULL)
            PreviousButton->Down=true;
        return;
    }

    EnsureComPortCreated(Application);
    if(fComPort->Visible)
        fComPort->BringToFront();
    else
        fComPort->ShowModal();

    if(Button!=NULL)
        Button->Down=false;
    if(PreviousButton!=NULL)
        PreviousButton->Down=true;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : spbMaintSECS opens the SECS/GEM monitor window
//  (non-modal). EC editing inside is idle-guarded, so the monitor itself may be
//  viewed while the machine runs; we only restore the menu button toggle state.
void __fastcall TfMaintenance::OpenSecsGemLog(TSpeedButton *Button)
{
    TSpeedButton *PreviousButton;

    PreviousButton=LastClickButton;

    ShowSecsGemLog();

    if(Button!=NULL)
        Button->Down=false;
    if(PreviousButton!=NULL)
        PreviousButton->Down=true;
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnMCUSaveClick(TObject *Sender)
{
    (void)Sender;
    SaveMCUDisplaySettings();
    RestartMCUDisplay();
    AddMCULog("Save bin display settings (General.ini) and restart COM");
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnMCUReloadClick(TObject *Sender)
{
    (void)Sender;
    GeneralSetting.Load();
    LoadMCUDisplaySettings();
    RefreshMCUDisplayStatus();
    AddMCULog("Reload bin display settings");
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnMCUSendDisplayClick(TObject *Sender)
{
    int Address;
    int Color;
    int Value;
    AnsiString Txt;

    (void)Sender;
    if(HSys.BinDisCtrl==NULL)
        return;
    Txt="";
    if(edMCUText!=NULL)
        Txt=edMCUText->Text;
    Address=ReadEditInt(edMCUAddress, 0);
    Color=ReadEditInt(edMCULightValue, 3);
    Value=BinTextToDispValue(Txt);
    HSys.BinDisCtrl->SetUnitLabel(Address, Value, Color);
    AddMCULog(AnsiString("Send Display addr=")+IntToStr(Address)+AnsiString(" text=")+Txt+AnsiString(" color=")+IntToStr(Color));
    RefreshMCUDisplayStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnMCUSendCodeClick(TObject *Sender)
{
    int Address;
    int Value;
    AnsiString Txt;

    (void)Sender;
    if(HSys.BinDisCtrl==NULL)
        return;
    Txt="";
    if(edMCUText!=NULL)
        Txt=edMCUText->Text;
    Address=ReadEditInt(edMCUAddress, 0);
    Value=BinTextToDispValue(Txt);
    HSys.BinDisCtrl->SetUnitBin(Address, Value);
    AddMCULog(AnsiString("Send Code addr=")+IntToStr(Address)+AnsiString(" text=")+Txt);
    RefreshMCUDisplayStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnMCUSendLightClick(TObject *Sender)
{
    int Address;
    int Color;

    (void)Sender;
    if(HSys.BinDisCtrl==NULL)
        return;
    Address=ReadEditInt(edMCUAddress, 0);
    Color=ReadEditInt(edMCULightValue, 3);
    HSys.BinDisCtrl->SetUnitColor(Address, Color);
    AddMCULog(AnsiString("Send Light addr=")+IntToStr(Address)+AnsiString(" color=")+IntToStr(Color));
    RefreshMCUDisplayStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnMCURefreshClick(TObject *Sender)
{
    (void)Sender;
    RefreshMCUDisplayStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RGB00Click(TObject *Sender)
{
    TALed *Led;
    int RowIndex;
    int ColorIndex;
    int State;

    Led=dynamic_cast<TALed *>(Sender);
    if(Led==NULL)
        return;

    RowIndex=Led->Tag/3;
    ColorIndex=Led->Tag%3;
    if(RowIndex<0 || RowIndex>5 || ColorIndex<0 || ColorIndex>2)
        return;

    //AI(HT160S-Maintainer) 20260622 : blink removed (user) -- the grid cell toggles
    //ON <-> OFF only (no BLINK state). Read Value only (the grid no longer shows Blink).
    if(Led->Value==true)
        State=TOWER_LIGHT_STATE_OFF;
    else
        State=TOWER_LIGHT_STATE_ON;
    SetTowerLightState(RowIndex, ColorIndex, State);
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::sbMusic1Click(TObject *Sender)
{
    TSpeedButton *Button;
    int MusicIndex;
    int SwitchIndex;
    int Index;

    Button=dynamic_cast<TSpeedButton *>(Sender);
    if(Button==NULL)
        return;

    MusicIndex=Button->Tag-1;
    if(MusicIndex<0 || MusicIndex>=4)
        return;

    if(bMusicTestOn[MusicIndex])
    {
        CloseBuzzerOff();
        bMusicTestOn[MusicIndex]=false;
        return;
    }

    CloseBuzzerOff();
    for(Index=0; Index<4; Index++)
        bMusicTestOn[Index]=false;

    SwitchIndex=HSys.Sw.SwMusic1.Tag+MusicIndex;
    if(HSys.SwPtr!=NULL && SwitchIndex>=0 && SwitchIndex<HSys.iTotalSwitch)
    {
        HSys.SwPtr[SwitchIndex].On();
        bMusicTestOn[MusicIndex]=true;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::FormClose(TObject *Sender, TCloseAction &Action)
{
    (void)Sender;
    (void)Action;
    SaveWorkFile(GetTowerLightIniFileName());
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::FormShow(TObject *Sender)
{
    (void)Sender;
    OpenWorkFile();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::chkHardwareColorBinAreaClick(TObject *Sender)
{
    if(bLoadingHardwareSettings)
        return;
    (void)Sender;
    if(chkHardwareColorBinArea!=NULL)
        GeneralSetting.bColorBinAreaInstalled=chkHardwareColorBinArea->Checked;
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::chkUseAMRClick(TObject *Sender)
{
    if(bLoadingHardwareSettings)
        return;
    (void)Sender;
    if(chkUseAMR!=NULL)
        GeneralSetting.bUseAMR=chkUseAMR->Checked;
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
//AI(ht160s-lotbin) 20260615 : Sort mode toggle (Normal <-> By Lot+Bin). This flag
//drives the routing core (GetMappedAutoIndex) and the dynamic binding table, which
//are read all over the run loop, so a clean restart is the safe way to apply it.
//Warn (do not force) the operator, matching the user's "remind, not enforce" rule.
void __fastcall TfMaintenance::chkUseLotBinModeClick(TObject *Sender)
{
    if(bLoadingHardwareSettings)
        return;
    (void)Sender;
    if(chkUseLotBinMode!=NULL)
        GeneralSetting.bUseLotBinSortMode=chkUseLotBinMode->Checked;
    RefreshHardwareSettingsStatus();
    ShowMyMessage("Sort mode changed. Please restart the software so the new "
                  "classification mode takes effect cleanly.");
}
//---------------------------------------------------------------------------
//AI(ht160s-lotbin) 20260615 : Per-Auto enable (By Lot+Bin mode only). Disabled
//Autos are skipped by THT160LotBinBinding::ResolveAuto when binding new (LotID,Bin)
//pairs. The flag is read in the run loop, so warn (do not force) the operator to
//restart, matching the Sort-mode toggle behavior above.
void __fastcall TfMaintenance::chkAutoEnableClick(TObject *Sender)
{
    if(bLoadingHardwareSettings)
        return;
    TCheckBox *AutoChk[6];
    int a;

    (void)Sender;
    AutoChk[0]=chkAutoEnable1; AutoChk[1]=chkAutoEnable2; AutoChk[2]=chkAutoEnable3;
    AutoChk[3]=chkAutoEnable4; AutoChk[4]=chkAutoEnable5; AutoChk[5]=chkAutoEnable6;
    for(a=0; a<6; a++)
        if(AutoChk[a]!=NULL)
            GeneralSetting.bAutoEnabled[a]=AutoChk[a]->Checked;
    RefreshHardwareSettingsStatus();
    ShowMyMessage("Auto enable changed. Please restart the software so the new "
                  "Lot+Bin routing takes effect cleanly.");
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260616 : Per-nozzle (SortArm sucker) enable. Unchecked
//slots are skipped by FindPickCells (which anchors the first ENABLED sucker to the
//found cell), so a broken nozzle can be taken out of service. Read live each pick
//cycle and the page is locked while a lot runs, so no restart is needed. At least
//one nozzle must stay enabled - if the operator unchecks the last one, re-check it
//and warn instead of leaving the machine unable to pick.
void __fastcall TfMaintenance::chkSuckEnableClick(TObject *Sender)
{
    if(bLoadingHardwareSettings)
        return;
    TCheckBox *SuckChk[4];
    int s;
    int iEnabledCount;

    SuckChk[0]=chkSuckEnable1; SuckChk[1]=chkSuckEnable2;
    SuckChk[2]=chkSuckEnable3; SuckChk[3]=chkSuckEnable4;

    iEnabledCount=0;
    for(s=0; s<4; s++)
        if(SuckChk[s]!=NULL && SuckChk[s]->Checked)
            iEnabledCount++;

    if(iEnabledCount==0)
    {
        TCheckBox *Box=dynamic_cast<TCheckBox *>(Sender);
        if(Box!=NULL)
            Box->Checked=true;
        ShowMyMessage(LangT("At least one nozzle must stay enabled."));
        return;
    }

    for(s=0; s<4; s++)
        if(SuckChk[s]!=NULL)
            GeneralSetting.bSuckerEnabled[s]=SuckChk[s]->Checked;
    GeneralSetting.Save();
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260624 : Loader safe distance = minimum separation
//between the two Loader-Y cars, consumed live by aLoader IsLoaderYMoveSafe.
//Stored as 1/100mm in GeneralSetting.iLoaderYSafeDistance to match the teach/
//encoder domain; operator edits in mm. Entry is via the on-screen keypad
//(fQwertyKey), which clamps to 325..650 mm on OK; a YES/NO confirm gates the
//save and the value persists live (mirrors the sucker-enable handler). The
//redundant clamp below is a no-op safety net on the operator path only - it is
//deliberately NOT placed in SaveHardwareSettings, so existing below-range
//configs are never silently bumped on form close.
void __fastcall TfMaintenance::edLoaderSafeDistanceClick(TObject *Sender)
{
    double mm;
    int v;
    AnsiString S;

    if(bLoadingHardwareSettings)
        return;
    (void)Sender;
    if(edLoaderSafeDistance==NULL || fQwertyKey==NULL)
        return;
    if(fQwertyKey->ShowQwertyKey(edLoaderSafeDistance, N_DOUBLE, 2, true, 325.0, 650.0, LangT("Loader Safe Distance (mm)"))==false)
        return;
    if(ShowMyMessageBox_YES_NO(LangT("Save Loader safe distance?"))!=1)
    {
        S.sprintf("%.2f", (double)GeneralSetting.iLoaderYSafeDistance/100.0);
        edLoaderSafeDistance->Text=S;
        return;
    }
    mm=atof(edLoaderSafeDistance->Text.c_str());
    if(mm<325.0)
        mm=325.0;
    if(mm>650.0)
        mm=650.0;
    v=(int)(mm*100.0+0.5);
    GeneralSetting.iLoaderYSafeDistance=v;
    GeneralSetting.Save();
    S.sprintf("%.2f", (double)v/100.0);
    edLoaderSafeDistance->Text=S;
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
//AI(ht160s-settle-panel) 20260628 : map a settle-delay edit Tag to its GeneralSetting
//member (single shared handler dispatches by Tag, mirrors chkAutoEnableClick).
static int* SettleDelayValPtr(int Tag, AnsiString &title)
{
    switch(Tag)
    {
        case 0: title=LangT("Empty destack settle (ms)"); return &GeneralSetting.iEmptyDestackSettleMs;
        case 1: title=LangT("Color destack settle (ms)"); return &GeneralSetting.iColorDestackSettleMs;
        case 2: title=LangT("Loader destack settle (ms)"); return &GeneralSetting.iLoaderDestackSettleMs;
        case 3: title=LangT("Auto push confirm settle (ms)"); return &GeneralSetting.iAutoPushConfirmSettleMs;
        case 4: title=LangT("Auto discharge-Y settle (ms)"); return &GeneralSetting.iAutoDischargePostYSettleMs;
        case 5: title=LangT("Auto front-rise dwell (ms)"); return &GeneralSetting.iAutoFrontRiseDwellMs;
        case 6: title=LangT("Auto cleanout-rise dwell (ms)"); return &GeneralSetting.iAutoCleanOutRiseDwellMs;
        case 7: title=LangT("TrayArm clamp settle (ms)"); return &GeneralSetting.iTrayArmClampSettleMs;
        case 8: title=LangT("Empty feed-clamp settle (ms)"); return &GeneralSetting.iEmptyFeedClampSettleMs;
        case 9: title=LangT("Color feed-clamp settle ms; 0=skip inline confirm"); return &GeneralSetting.iColorFeedClampSettleMs;
    }
    return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::edSettleDelayClick(TObject *Sender)
{
    if(bLoadingHardwareSettings)
        return;
    if(fQwertyKey==NULL || Sender==NULL)
        return;
    TEdit *ed=(TEdit*)Sender;
    AnsiString title;
    int *pv=SettleDelayValPtr(ed->Tag, title);
    if(pv==NULL)
        return;
    if(fQwertyKey->ShowQwertyKey(ed, N_INTEGER, 0, true, 0.0, 5000.0, title)==false)
        return;
    if(ShowMyMessageBox_YES_NO(LangT("Save settle time?"))!=1)
    {
        ed->Text=IntToStr(*pv);
        return;
    }
    int v=ed->Text.ToIntDef(*pv);
    if(v<0)
        v=0;
    if(v>5000)
        v=5000;
    *pv=v;
    GeneralSetting.Save();
    ed->Text=IntToStr(v);
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//AI(ht160s-password) 20260628 : tsMaintPassword child controls now live in the
// DFM (designer-visible). This routine runs on every page open: it fills the
// level combo once, applies all bilingual captions via LangT so they follow the
// language toggle, sets the role-based edit lock, and refreshes the user list.
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::ShowPasswordPage()
{
    int i;
    bool bCanEdit;

    if(cbbPwLevel!=NULL && cbbPwLevel->Items->Count==0)
    {
        for(i=ROLE_OPERATION; i<=ROLE_HONPREC; i++)
            cbbPwLevel->Items->Add(IntToStr(i)+" - "+THT160UserRoleManager::GetLevelName(i));
        cbbPwLevel->ItemIndex=ROLE_OPERATION;
    }

    if(labPwIdCaption!=NULL)    labPwIdCaption->Caption=LangT("Account ID");
    if(labPwPassCaption!=NULL)  labPwPassCaption->Caption=LangT("Password");
    if(labPwLevelCaption!=NULL) labPwLevelCaption->Caption=LangT("Level");
    if(btnPwAddUpdate!=NULL)    btnPwAddUpdate->Caption=LangT("Add / Update");
    if(btnPwDelete!=NULL)       btnPwDelete->Caption=LangT("Delete");
    if(btnPwSave!=NULL)         btnPwSave->Caption=LangT("Save to File");
    if(btnPwReload!=NULL)       btnPwReload->Caption=LangT("Reload");

    RefreshPasswordGrid();

    bCanEdit=UserRoleManager.HasLevel(ROLE_ENGINEER);
    if(edPwId!=NULL)         edPwId->Enabled=bCanEdit;
    if(edPwPass!=NULL)       edPwPass->Enabled=bCanEdit;
    if(cbbPwLevel!=NULL)     cbbPwLevel->Enabled=bCanEdit;
    if(btnPwAddUpdate!=NULL) btnPwAddUpdate->Enabled=bCanEdit;
    if(btnPwDelete!=NULL)    btnPwDelete->Enabled=bCanEdit;
    if(btnPwSave!=NULL)      btnPwSave->Enabled=bCanEdit;
    if(btnPwReload!=NULL)    btnPwReload->Enabled=bCanEdit;
    if(labPwHint!=NULL)
    {
        if(bCanEdit)
            labPwHint->Caption=LangT("Accounts: ID / password / level 0-3. Stored in system\\login.txt.");
        else
            labPwHint->Caption=LangT("View only. Engineer level (2) or above is required to edit.");
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RefreshPasswordGrid()
{
    int i;
    AnsiString Line;

    if(lbPwUsers==NULL)
        return;

    lbPwUsers->Items->BeginUpdate();
    try
    {
        lbPwUsers->Items->Clear();
        for(i=0; i<UserRoleManager.GetUserCount(); i++)
        {
            Line=UserRoleManager.GetUserID(i);
            while(Line.Length()<16)
                Line=Line+" ";
            Line=Line+"  Lv"+IntToStr(UserRoleManager.GetUserLevel(i))+" "+
                 THT160UserRoleManager::GetLevelName(UserRoleManager.GetUserLevel(i));
            lbPwUsers->Items->Add(Line);
        }
    }
    __finally
    {
        lbPwUsers->Items->EndUpdate();
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::PwListClick(TObject *Sender)
{
    int idx;

    (void)Sender;
    if(lbPwUsers==NULL || edPwId==NULL || cbbPwLevel==NULL)
        return;
    idx=lbPwUsers->ItemIndex;
    if(idx<0 || idx>=UserRoleManager.GetUserCount())
        return;
    edPwId->Text=UserRoleManager.GetUserID(idx);
    cbbPwLevel->ItemIndex=UserRoleManager.GetUserLevel(idx);
    if(edPwPass!=NULL)
        edPwPass->Text="";
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::PwIdClick(TObject *Sender)
{
    (void)Sender;
    if(edPwId==NULL || fQwertyKey==NULL || edPwId->Enabled==false)
        return;
    fQwertyKey->ShowQwertyKey(edPwId, N_NO_SPACE, 0, false, 0, 0, LangT("Account ID"));
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::PwPassClick(TObject *Sender)
{
    (void)Sender;
    if(edPwPass==NULL || fQwertyKey==NULL || edPwPass->Enabled==false)
        return;
    fQwertyKey->ShowQwertyKey(edPwPass, N_PASSWORD|N_NO_SPACE, 0, false, 0, 0, LangT("Password"));
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::PwAddUpdateClick(TObject *Sender)
{
    AnsiString sID, sPass;
    int iLevel;

    (void)Sender;
    if(edPwId==NULL || edPwPass==NULL || cbbPwLevel==NULL)
        return;
    sID=edPwId->Text.Trim();
    sPass=edPwPass->Text;
    iLevel=cbbPwLevel->ItemIndex;
    if(sID==AnsiString(""))
    {
        ShowMyMessage(LangT("Please enter an account ID."));
        return;
    }
    if(sPass==AnsiString(""))
    {
        ShowMyMessage(LangT("Please enter a password."));
        return;
    }
    if(!UserRoleManager.IsValidLevel(iLevel))
    {
        ShowMyMessage(LangT("Please select a level (0-3)."));
        return;
    }
    if(UserRoleManager.AddOrUpdateUser(sID, sPass, iLevel)==false)
    {
        ShowMyMessage(LangT("Account table is full (max 30)."));
        return;
    }
    edPwPass->Text="";
    RefreshPasswordGrid();
    ShowMyMessage(LangT("Account saved in memory. Press 'Save to File' to keep it."));
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::PwDeleteClick(TObject *Sender)
{
    int idx, iLevel;
    AnsiString sID;

    (void)Sender;
    if(lbPwUsers==NULL)
        return;
    idx=lbPwUsers->ItemIndex;
    if(idx<0 || idx>=UserRoleManager.GetUserCount())
    {
        ShowMyMessage(LangT("Please select an account to delete."));
        return;
    }
    sID=UserRoleManager.GetUserID(idx);
    iLevel=UserRoleManager.GetUserLevel(idx);
    if(ShowMyMessageBox_YES_NO(Format(LangT("Delete account: %s ?"), ARRAYOFCONST((sID)))) !=1)
        return;
    UserRoleManager.DeleteUser(sID, iLevel);
    RefreshPasswordGrid();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::PwSaveClick(TObject *Sender)
{
    (void)Sender;
    SavePassword();
    ShowMyMessage(LangT("User accounts saved to system\\login.txt."));
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::PwReloadClick(TObject *Sender)
{
    (void)Sender;
    ReadPassword();
    RefreshPasswordGrid();
    if(edPwId!=NULL)    edPwId->Text="";
    if(edPwPass!=NULL)  edPwPass->Text="";
    ShowMyMessage(LangT("User accounts reloaded from system\\login.txt."));
}
