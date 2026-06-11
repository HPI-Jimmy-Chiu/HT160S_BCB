//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop

#include "database.h"
#include "maintenance.h"
#include "ComPort.h"
#include "CosFunction.h"
#include "GeneralSetting.h"
#include "MCUDisplay.h"
#include "TopCcdSocket.h"
#include "iosetview.h"
#include "uteach.h"
#include "uMotorTest.h"
#include "mymessbox.h"
#include "SecsGem/uHGemLogForm.h"   //AI(ht160s-secsgem) 20260611 : ShowSecsGemLog
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
static const char *MCU_DISPLAY_INI_GROUP="Setup";
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
static AnsiString GetMCUDisplayIniFileName()
{
    AnsiString RootPath=HSys.CurrentDir;
    if(RootPath==AnsiString(""))
        RootPath="..";
    return RootPath+AnsiString("\\system\\MCU.ini");
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
        return BlinkPhase;
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
    tmrTowerLightBlink=NULL;
    bTowerLightBlinkPhase=false;
    spbMaintMCUDisplay=NULL;
    tsMaintMCUDisplay=NULL;
    chkMCUEnabled=NULL;
    edMCUIP=NULL;
    edMCUPort=NULL;
    edMCUMaxQueue=NULL;
    edMCUReconnect=NULL;
    edMCUAddress=NULL;
    edMCUText=NULL;
    cbbMCUColor=NULL;
    chkMCUCodeSymbol=NULL;
    edMCULightValue=NULL;
    lblMCUStatusEnabled=NULL;
    lblMCUStatusConnected=NULL;
    lblMCUStatusQueue=NULL;
    lblMCUStatusError=NULL;
    memMCULog=NULL;

    spbMaintTopCcd=NULL;
    tsMaintTopCcd=NULL;
    edTopCcdIP=NULL;
    edTopCcdPort=NULL;
    chkTopCcdBottomReserved=NULL;
    lblTopCcdStatusConn=NULL;
    lblTopCcdStatusError=NULL;
    edTopCcdResult=NULL;
    memTopCcdLog=NULL;

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

    BuildMCUDisplayPage();
    BuildTopCcdPage();
    RegisterMaintenancePages();
    LayoutMaintenanceButtons();
    InitializeTowerLightPanels();
    LoadMaintenanceSettings();

    tmrTowerLightBlink=new TTimer(this);
    tmrTowerLightBlink->Interval=300;
    tmrTowerLightBlink->OnTimer=tmrTowerLightBlinkTimer;
    tmrTowerLightBlink->Enabled=true;

    SelectMaintenancePage(0);
    //AI(HT160S-Maintainer) 20260608 : need2-D : read saved Top CCD endpoint at
    //power-on so the maintenance page IP/Port fields reflect General.ini even
    //before the page is opened (socket itself is configured by LoadConfig()).
    LoadTopCcdSettings();
    EnsureTopCcdSocketCreated();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::BuildMCUDisplayPage()
{
    TPanel *Panel;
    TPanel *TestPanel;
    TPanel *StatusPanel;

    if(spbMaintMCUDisplay==NULL)
    {
        spbMaintMCUDisplay=new TSpeedButton(this);
        spbMaintMCUDisplay->Parent=pnlMenu;
        spbMaintMCUDisplay->Caption="Bin Display";
        spbMaintMCUDisplay->Font->Height=-15;
        spbMaintMCUDisplay->ParentFont=false;
        spbMaintMCUDisplay->AllowAllUp=true;
        spbMaintMCUDisplay->GroupIndex=1;
    }

    if(tsMaintMCUDisplay==NULL)
    {
        tsMaintMCUDisplay=new TTabSheet(this);
        tsMaintMCUDisplay->PageControl=pcMaintenance;
        tsMaintMCUDisplay->Caption="Bin Display";
    }

    Panel=new TPanel(this);
    Panel->Parent=tsMaintMCUDisplay;
    Panel->Left=20;
    Panel->Top=20;
    Panel->Width=780;
    Panel->Height=210;
    Panel->BevelOuter=bvLowered;
    Panel->Caption="";
    Panel->Color=(TColor)12761254;

    CreateMaintLabel(this, Panel, 16, 14, 160, 20, "Bin Display TCP Setup");
    chkMCUEnabled=CreateMaintCheckBox(this, Panel, 16, 46, 120, "Enabled");
    CreateMaintLabel(this, Panel, 16, 82, 80, 20, "IP");
    edMCUIP=CreateMaintEdit(this, Panel, 108, 78, 160, "127.0.0.1");
    CreateMaintLabel(this, Panel, 16, 118, 80, 20, "Port");
    edMCUPort=CreateMaintEdit(this, Panel, 108, 114, 80, "7000");
    CreateMaintLabel(this, Panel, 310, 82, 100, 20, "Max Queue");
    edMCUMaxQueue=CreateMaintEdit(this, Panel, 420, 78, 80, "500");
    CreateMaintLabel(this, Panel, 310, 118, 120, 20, "Reconnect(ms)");
    edMCUReconnect=CreateMaintEdit(this, Panel, 420, 114, 80, "3000");
    CreateMaintButton(this, Panel, 560, 76, 90, "Save", btnMCUSaveClick);
    CreateMaintButton(this, Panel, 660, 76, 90, "Reload", btnMCUReloadClick);
    CreateMaintButton(this, Panel, 560, 114, 190, "Refresh Status", btnMCURefreshClick);

    StatusPanel=new TPanel(this);
    StatusPanel->Parent=Panel;
    StatusPanel->Left=16;
    StatusPanel->Top=152;
    StatusPanel->Width=734;
    StatusPanel->Height=42;
    StatusPanel->BevelOuter=bvLowered;
    StatusPanel->Caption="";
    lblMCUStatusEnabled=CreateMaintLabel(this, StatusPanel, 8, 12, 130, 20, "Enabled: -");
    lblMCUStatusConnected=CreateMaintLabel(this, StatusPanel, 146, 12, 150, 20, "Connected: -");
    lblMCUStatusQueue=CreateMaintLabel(this, StatusPanel, 304, 12, 110, 20, "Queue: 0");
    lblMCUStatusError=CreateMaintLabel(this, StatusPanel, 420, 12, 300, 20, "Last Error: ");

    TestPanel=new TPanel(this);
    TestPanel->Parent=tsMaintMCUDisplay;
    TestPanel->Left=20;
    TestPanel->Top=246;
    TestPanel->Width=780;
    TestPanel->Height=210;
    TestPanel->BevelOuter=bvLowered;
    TestPanel->Caption="";
    TestPanel->Color=(TColor)12761254;

    CreateMaintLabel(this, TestPanel, 16, 14, 180, 20, "Manual Test");
    CreateMaintLabel(this, TestPanel, 16, 52, 80, 20, "Address");
    edMCUAddress=CreateMaintEdit(this, TestPanel, 108, 48, 80, "0");
    CreateMaintLabel(this, TestPanel, 16, 88, 80, 20, "Text");
    edMCUText=CreateMaintEdit(this, TestPanel, 108, 84, 80, "9");
    CreateMaintLabel(this, TestPanel, 16, 124, 80, 20, "Color");
    cbbMCUColor=new TComboBox(this);
    cbbMCUColor->Parent=TestPanel;
    cbbMCUColor->Left=108;
    cbbMCUColor->Top=120;
    cbbMCUColor->Width=120;
    cbbMCUColor->Height=24;
    cbbMCUColor->Style=csDropDownList;
    cbbMCUColor->Items->Add("GREEN");
    cbbMCUColor->Items->Add("RED");
    cbbMCUColor->ItemIndex=0;
    chkMCUCodeSymbol=CreateMaintCheckBox(this, TestPanel, 260, 48, 130, "Symbol Code");
    CreateMaintLabel(this, TestPanel, 260, 88, 100, 20, "Light Value");
    edMCULightValue=CreateMaintEdit(this, TestPanel, 370, 84, 80, "0");
    CreateMaintButton(this, TestPanel, 520, 48, 150, "Send Display", btnMCUSendDisplayClick);
    CreateMaintButton(this, TestPanel, 520, 86, 150, "Send Code", btnMCUSendCodeClick);
    CreateMaintButton(this, TestPanel, 520, 124, 150, "Send Light", btnMCUSendLightClick);

    memMCULog=new TMemo(this);
    memMCULog->Parent=tsMaintMCUDisplay;
    memMCULog->Left=20;
    memMCULog->Top=474;
    memMCULog->Width=780;
    memMCULog->Height=300;
    memMCULog->ScrollBars=ssVertical;
    memMCULog->ReadOnly=true;
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
void __fastcall TfMaintenance::BuildTopCcdPage()
{
    TPanel *Panel;
    TPanel *TestPanel;
    TPanel *StatusPanel;

    if(spbMaintTopCcd==NULL)
    {
        spbMaintTopCcd=new TSpeedButton(this);
        spbMaintTopCcd->Parent=pnlMenu;
        spbMaintTopCcd->Caption="Top CCD";
        spbMaintTopCcd->Font->Height=-15;
        spbMaintTopCcd->ParentFont=false;
        spbMaintTopCcd->AllowAllUp=true;
        spbMaintTopCcd->GroupIndex=1;
    }

    if(tsMaintTopCcd==NULL)
    {
        tsMaintTopCcd=new TTabSheet(this);
        tsMaintTopCcd->PageControl=pcMaintenance;
        tsMaintTopCcd->Caption="Top CCD";
    }

    Panel=new TPanel(this);
    Panel->Parent=tsMaintTopCcd;
    Panel->Left=20;
    Panel->Top=20;
    Panel->Width=780;
    Panel->Height=170;
    Panel->BevelOuter=bvLowered;
    Panel->Caption="";
    Panel->Color=(TColor)12761254;

    CreateMaintLabel(this, Panel, 16, 14, 200, 20, "Top CCD TCP Setup");
    CreateMaintLabel(this, Panel, 16, 50, 80, 20, "IP");
    edTopCcdIP=CreateMaintEdit(this, Panel, 108, 46, 160, "172.16.8.89");
    CreateMaintLabel(this, Panel, 16, 86, 80, 20, "Port");
    edTopCcdPort=CreateMaintEdit(this, Panel, 108, 82, 80, "5001");
    chkTopCcdBottomReserved=CreateMaintCheckBox(this, Panel, 310, 48, 240, "Bottom CCD (reserved)");
    chkTopCcdBottomReserved->Enabled=false;
    CreateMaintButton(this, Panel, 560, 44, 90, "Save", btnTopCcdSaveClick);
    CreateMaintButton(this, Panel, 660, 44, 90, "Reload", btnTopCcdReloadClick);
    CreateMaintButton(this, Panel, 560, 82, 90, "Connect", btnTopCcdConnectClick);
    CreateMaintButton(this, Panel, 660, 82, 90, "Disconnect", btnTopCcdDisconnectClick);

    StatusPanel=new TPanel(this);
    StatusPanel->Parent=Panel;
    StatusPanel->Left=16;
    StatusPanel->Top=118;
    StatusPanel->Width=734;
    StatusPanel->Height=42;
    StatusPanel->BevelOuter=bvLowered;
    StatusPanel->Caption="";
    lblTopCcdStatusConn=CreateMaintLabel(this, StatusPanel, 8, 12, 180, 20, "Connected: -");
    lblTopCcdStatusError=CreateMaintLabel(this, StatusPanel, 200, 12, 520, 20, "Last Error: ");

    TestPanel=new TPanel(this);
    TestPanel->Parent=tsMaintTopCcd;
    TestPanel->Left=20;
    TestPanel->Top=206;
    TestPanel->Width=780;
    TestPanel->Height=110;
    TestPanel->BevelOuter=bvLowered;
    TestPanel->Caption="";
    TestPanel->Color=(TColor)12761254;

    CreateMaintLabel(this, TestPanel, 16, 14, 200, 20, "Manual Shot (LON)");
    CreateMaintButton(this, TestPanel, 16, 48, 150, "Trigger Shot", btnTopCcdShotClick);
    CreateMaintLabel(this, TestPanel, 190, 52, 80, 20, "Result");
    edTopCcdResult=CreateMaintEdit(this, TestPanel, 280, 48, 400, "");
    edTopCcdResult->ReadOnly=true;

    memTopCcdLog=new TMemo(this);
    memTopCcdLog->Parent=tsMaintTopCcd;
    memTopCcdLog->Left=20;
    memTopCcdLog->Top=332;
    memTopCcdLog->Width=780;
    memTopCcdLog->Height=300;
    memTopCcdLog->ScrollBars=ssVertical;
    memTopCcdLog->ReadOnly=true;
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
    }
    __finally
    {
        delete Ini;
    }
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
            lblTopCcdStatusConn->Caption="Connected: -";
        if(lblTopCcdStatusError!=NULL)
            lblTopCcdStatusError->Caption="Last Error: not started";
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
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::LoadHardwareSettings()
{
    GeneralSetting.Load();
    BinAreaMap.LoadDefault();
    if(chkHardwareColorBinArea!=NULL)
        chkHardwareColorBinArea->Checked=GeneralSetting.bColorBinAreaInstalled;
    if(chkUseAMR!=NULL)
        chkUseAMR->Checked=GeneralSetting.bUseAMR;
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SaveHardwareSettings()
{
    if(chkHardwareColorBinArea!=NULL)
        GeneralSetting.bColorBinAreaInstalled=chkHardwareColorBinArea->Checked;
    if(chkUseAMR!=NULL)
        GeneralSetting.bUseAMR=chkUseAMR->Checked;
    GeneralSetting.Save();
    BinAreaMap.LoadDefault();
    RefreshHardwareSettingsStatus();
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
void __fastcall TfMaintenance::LoadMCUDisplaySettings()
{
    TIniFile *Ini;
    AnsiString FileName;

    FileName=GetMCUDisplayIniFileName();
    Ini=new TIniFile(FileName);
    try
    {
        if(chkMCUEnabled!=NULL)
            chkMCUEnabled->Checked=Ini->ReadBool(MCU_DISPLAY_INI_GROUP, "Enabled", true);
        if(edMCUIP!=NULL)
            edMCUIP->Text=Ini->ReadString(MCU_DISPLAY_INI_GROUP, "IP", "127.0.0.1");
        if(edMCUPort!=NULL)
            edMCUPort->Text=IntToStr(Ini->ReadInteger(MCU_DISPLAY_INI_GROUP, "Port", 7000));
        if(edMCUMaxQueue!=NULL)
            edMCUMaxQueue->Text=IntToStr(Ini->ReadInteger(MCU_DISPLAY_INI_GROUP, "MaxQueue", 500));
        if(edMCUReconnect!=NULL)
            edMCUReconnect->Text=IntToStr(Ini->ReadInteger(MCU_DISPLAY_INI_GROUP, "ReconnectIntervalMs", 3000));
    }
    __finally
    {
        delete Ini;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SaveMCUDisplaySettings()
{
    TIniFile *Ini;
    AnsiString FileName;
    int Port;
    int MaxQueue;
    int ReconnectInterval;

    FileName=GetMCUDisplayIniFileName();
    ForceDirectories(ExtractFilePath(FileName));
    Port=ReadEditInt(edMCUPort, 7000);
    MaxQueue=ReadEditInt(edMCUMaxQueue, 500);
    ReconnectInterval=ReadEditInt(edMCUReconnect, 3000);
    if(Port<=0 || Port>65535)
        Port=7000;
    if(MaxQueue<1)
        MaxQueue=500;
    if(ReconnectInterval<500)
        ReconnectInterval=500;

    Ini=new TIniFile(FileName);
    try
    {
        Ini->WriteBool(MCU_DISPLAY_INI_GROUP, "Enabled", chkMCUEnabled!=NULL ? chkMCUEnabled->Checked : true);
        Ini->WriteString(MCU_DISPLAY_INI_GROUP, "IP", edMCUIP!=NULL ? edMCUIP->Text.Trim() : AnsiString("127.0.0.1"));
        Ini->WriteInteger(MCU_DISPLAY_INI_GROUP, "Port", Port);
        Ini->WriteInteger(MCU_DISPLAY_INI_GROUP, "MaxQueue", MaxQueue);
        Ini->WriteInteger(MCU_DISPLAY_INI_GROUP, "ReconnectIntervalMs", ReconnectInterval);
    }
    __finally
    {
        delete Ini;
    }

    if(edMCUPort!=NULL)
        edMCUPort->Text=IntToStr(Port);
    if(edMCUMaxQueue!=NULL)
        edMCUMaxQueue->Text=IntToStr(MaxQueue);
    if(edMCUReconnect!=NULL)
        edMCUReconnect->Text=IntToStr(ReconnectInterval);
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RestartMCUDisplay()
{
    EnsureMCUDisplayCreated(Application);
    if(HT160MCUDisplay!=NULL)
    {
        HT160MCUDisplay->Stop();
        HT160MCUDisplay->Start();
        HT160MCUDisplay->Spin();
    }
    RefreshMCUDisplayStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::RefreshMCUDisplayStatus()
{
    bool bCreated;

    bCreated=(HT160MCUDisplay!=NULL);
    if(bCreated)
        HT160MCUDisplay->Spin();
    if(lblMCUStatusEnabled!=NULL)
        lblMCUStatusEnabled->Caption=AnsiString("Enabled: ")+(bCreated && HT160MCUDisplay->IsEnabled() ? "YES" : "NO");
    if(lblMCUStatusConnected!=NULL)
        lblMCUStatusConnected->Caption=AnsiString("Connected: ")+(bCreated && HT160MCUDisplay->IsConnected() ? "YES" : "NO");
    if(lblMCUStatusQueue!=NULL)
        lblMCUStatusQueue->Caption=AnsiString("Queue: ")+(bCreated ? IntToStr(HT160MCUDisplay->GetQueueCount()) : AnsiString("0"));
    if(lblMCUStatusError!=NULL)
        lblMCUStatusError->Caption=AnsiString("Last Error: ")+(bCreated ? HT160MCUDisplay->GetLastError() : AnsiString("not started"));
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
            Led->Blink=(State==TOWER_LIGHT_STATE_BLINK);
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
    AddMCULog("Save MCU.ini and restart TCP client");
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnMCUReloadClick(TObject *Sender)
{
    (void)Sender;
    LoadMCUDisplaySettings();
    RestartMCUDisplay();
    AddMCULog("Reload MCU.ini and restart TCP client");
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnMCUSendDisplayClick(TObject *Sender)
{
    int Address;
    AnsiString ColorText;

    (void)Sender;
    SaveMCUDisplaySettings();
    RestartMCUDisplay();
    Address=ReadEditInt(edMCUAddress, 0);
    ColorText=(cbbMCUColor!=NULL && cbbMCUColor->Text!=AnsiString("")) ? cbbMCUColor->Text : AnsiString("GREEN");
    SetMCUBinDisplay(Address, edMCUText!=NULL ? edMCUText->Text : AnsiString("9"), ColorText);
    AddMCULog(AnsiString("Send Display addr=")+IntToStr(Address)+AnsiString(" text=")+(edMCUText!=NULL ? edMCUText->Text : AnsiString("9"))+AnsiString(" color=")+ColorText);
    RefreshMCUDisplayStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnMCUSendCodeClick(TObject *Sender)
{
    int Address;
    bool bSymbol;

    (void)Sender;
    SaveMCUDisplaySettings();
    RestartMCUDisplay();
    Address=ReadEditInt(edMCUAddress, 0);
    bSymbol=(chkMCUCodeSymbol!=NULL && chkMCUCodeSymbol->Checked);
    SetMCUBinCode(Address, edMCUText!=NULL ? edMCUText->Text : AnsiString("9"), bSymbol);
    AddMCULog(AnsiString("Send Code addr=")+IntToStr(Address)+AnsiString(" text=")+(edMCUText!=NULL ? edMCUText->Text : AnsiString("9")));
    RefreshMCUDisplayStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::btnMCUSendLightClick(TObject *Sender)
{
    int Address;
    int LightValue;

    (void)Sender;
    SaveMCUDisplaySettings();
    RestartMCUDisplay();
    Address=ReadEditInt(edMCUAddress, 0);
    LightValue=ReadEditInt(edMCULightValue, 0);
    SetMCUBinLight(Address, LightValue);
    AddMCULog(AnsiString("Send Light addr=")+IntToStr(Address)+AnsiString(" value=")+IntToStr(LightValue));
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

    if(Led->Value==true && Led->Blink==false)
        State=TOWER_LIGHT_STATE_BLINK;
    else if(Led->Value==true && Led->Blink==true)
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
    (void)Sender;
    if(chkHardwareColorBinArea!=NULL)
        GeneralSetting.bColorBinAreaInstalled=chkHardwareColorBinArea->Checked;
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::chkUseAMRClick(TObject *Sender)
{
    (void)Sender;
    if(chkUseAMR!=NULL)
        GeneralSetting.bUseAMR=chkUseAMR->Checked;
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
