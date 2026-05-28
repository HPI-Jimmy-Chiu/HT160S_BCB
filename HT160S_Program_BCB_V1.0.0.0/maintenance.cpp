//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop

#include "database.h"
#include "maintenance.h"
#include "ComPort.h"
#include "CosFunction.h"
#include "iosetview.h"
#include "uteach.h"
#include "uMotorTest.h"
#include "mymessbox.h"
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

    RegisterMaintenancePages();
    LayoutMaintenanceButtons();
    InitializeTowerLightPanels();
    LoadMaintenanceSettings();

    tmrTowerLightBlink=new TTimer(this);
    tmrTowerLightBlink->Interval=300;
    tmrTowerLightBlink->OnTimer=tmrTowerLightBlinkTimer;
    tmrTowerLightBlink->Enabled=true;

    SelectMaintenancePage(0);
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
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::LoadHardwareSettings()
{
    LoadCosFunctionMachineOption();
    BinAreaMap.LoadDefault();
    if(chkHardwareColorBinArea!=NULL)
        chkHardwareColorBinArea->Checked=CosFunction.bColorBinAreaInstalled;
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfMaintenance::SaveHardwareSettings()
{
    if(chkHardwareColorBinArea!=NULL)
        CosFunction.bColorBinAreaInstalled=chkHardwareColorBinArea->Checked;
    SaveCosFunctionMachineOption();
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
        {tsMaintSECS,        spbMaintSECS,        maShowPage, false},
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

    RegisterIOViewStreamClasses();
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
        CosFunction.bColorBinAreaInstalled=chkHardwareColorBinArea->Checked;
    RefreshHardwareSettingsStatus();
}
//---------------------------------------------------------------------------
