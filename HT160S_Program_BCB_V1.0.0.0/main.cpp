//---------------------------------------------------------------------------

#include "IncludeAllHeader.h"
#pragma hdrstop

#include "main.h"
#include "database.h"
#include "cmydef.h"
#include "uruncontrol.h"
#include "iosetview.h"
#include "uteach.h"
#include "language.h"
#include "setup.h"
#include "data.h"
#include "maintenance.h"
#include "uMotorTest.h"
#include "uOffset.h"
#include "uspeed.h"
#include "systools.h"
#include "note.h"
#include "SecsGem/UsecegemMainFrom.h"
#include "SecsGem/uHGemHT160.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "HTray"
#pragma link "ALed"
#pragma link "ALed"
#pragma link "MyLed"
#pragma resource "*.dfm"
TfMain *fMain;
//---------------------------------------------------------------------------
static void ShowTopForm(TForm *FormPtr, TSpeedButton *ButtonPtr)
{
    if(FormPtr != NULL) {
        FormPtr->ShowModal();
    }

    if(ButtonPtr != NULL) {
        ButtonPtr->Down=false;
    }
}
//---------------------------------------------------------------------------
static void SmokeShowTopForm(TForm *FormPtr)
{
    if(FormPtr != NULL) {
        FormPtr->Show();
        FormPtr->Update();
        FormPtr->Hide();
    }
}
//---------------------------------------------------------------------------
static TEdit *__fastcall CreateHiddenEdit(TComponent *Owner, TWinControl *ParentControl, const char *EditName)
{
    TEdit *EditPtr = new TEdit(Owner);
    EditPtr->Name = EditName;
    EditPtr->Text = "";
    EditPtr->Parent = ParentControl;
    EditPtr->Left = 0;
    EditPtr->Top = 0;
    EditPtr->Width = 120;
    EditPtr->Height = 21;
    EditPtr->Visible = false;
    return EditPtr;
}
//---------------------------------------------------------------------------
static void NormalizeMainRunSettings()
{
    if(HSys.LastSet.iRealDummy < DUMMY || HSys.LastSet.iRealDummy > REALLY)
        HSys.LastSet.iRealDummy = DUMMY;
    if(HSys.LastSet.iStartMode < 0 || HSys.LastSet.iStartMode > 1)
        HSys.LastSet.iStartMode = 0;
}
//---------------------------------------------------------------------------
static AnsiString GetMainLastSetFileName()
{
    return HSys.CurrentDir + AnsiString("\\system\\lastset.ini");
}
//---------------------------------------------------------------------------
static void LoadMainRunSettingsFromIni()
{
    TIniFile *Ini = new TIniFile(GetMainLastSetFileName());
    try
    {
        HSys.LastSet.iLanguageCountry = Ini->ReadInteger("System", "iLanguageCountry", HSys.LastSet.iLanguageCountry);
        HSys.LastSet.iStartMode = Ini->ReadInteger("System", "iStartMode", HSys.LastSet.iStartMode);
        HSys.LastSet.iRealDummy = Ini->ReadInteger("System", "RealDummy", HSys.LastSet.iRealDummy);
    }
    __finally
    {
        delete Ini;
    }
    NormalizeMainRunSettings();
}
//---------------------------------------------------------------------------
static void SaveMainRunSettingsToIni()
{
    NormalizeMainRunSettings();
    TIniFile *Ini = new TIniFile(GetMainLastSetFileName());
    try
    {
        Ini->WriteInteger("System", "iLanguageCountry", HSys.LastSet.iLanguageCountry);
        Ini->WriteInteger("System", "iStartMode", HSys.LastSet.iStartMode);
        Ini->WriteInteger("System", "RealDummy", HSys.LastSet.iRealDummy);
    }
    __finally
    {
        delete Ini;
    }
}
//---------------------------------------------------------------------------
static AnsiString GetRunModeLogText()
{
    if(HSys.LastSet.iRealDummy == REALLY)
        return "Change Real IC Mode";
    if(HSys.LastSet.iRealDummy == HAS_TRAY)
        return "Change Has Tray Mode";
    return "Change Dummy Mode";
}
//---------------------------------------------------------------------------
static AnsiString GetFeatureStatusName(int BadgeIndex)
{
    switch(BadgeIndex)
    {
        case eMainFeatureSECS:     return "SECS";
        case eMainFeatureSafeDoor: return "SAFE";
    }

    return "";
}
//---------------------------------------------------------------------------
static AnsiString GetFeatureStatusDefaultValue(int BadgeIndex)
{
    switch(BadgeIndex)
    {
        case eMainFeatureSECS:     return "OFF";
        case eMainFeatureSafeDoor: return "NORMAL";
    }

    return "";
}
//---------------------------------------------------------------------------
static TColor GetFeatureStatusDefaultColor(int BadgeIndex)
{
    switch(BadgeIndex)
    {
        case eMainFeatureSECS:     return clTeal;
        case eMainFeatureSafeDoor: return clGreen;
    }

    return clGray;
}
//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
    : TForm(Owner)
{
    int StatusIndex;

    for(StatusIndex = 0; StatusIndex < MAIN_FEATURE_STATUS_COUNT; StatusIndex++)
    {
        FeatureStatusPanels[StatusIndex] = NULL;
        FeatureStatusNameLabels[StatusIndex] = NULL;
        FeatureStatusValueLabels[StatusIndex] = NULL;
    }

    edLotNo = NULL;
    edWaferLot = NULL;
    edCusDevice = NULL;
    edInsertion = NULL;
    edFlowID = NULL;
    edOperator = NULL;
    edtRunCard = NULL;

    if(ComponentState.Contains(csDesigning))
        return;

    BuildFeatureStatusBadges();

    edLotNo = CreateHiddenEdit(this, this, "edLotNo");
    edWaferLot = CreateHiddenEdit(this, this, "edWaferLot");
    edCusDevice = CreateHiddenEdit(this, this, "edCusDevice");
    edInsertion = CreateHiddenEdit(this, this, "edInsertion");
    edFlowID = CreateHiddenEdit(this, this, "edFlowID");
    edOperator = CreateHiddenEdit(this, this, "edOperator");
    edtRunCard = CreateHiddenEdit(this, this, "edtRunCard");

    LoadMainRunSettingsFromIni();
    LoadRunModePicture();
    LoadStartModePicture();

    if(pgcMain != NULL)
    {
        for(int PageIndex = 0; PageIndex < pgcMain->PageCount; PageIndex++)
            pgcMain->Pages[PageIndex]->TabVisible = false;
        pgcMain->ActivePage = tsMain;
    }

    if(pgcMonitor != NULL)
    {
        for(int PageIndex = 0; PageIndex < pgcMonitor->PageCount; PageIndex++)
            pgcMonitor->Pages[PageIndex]->TabVisible = false;
        pgcMonitor->ActivePage = TabSheet10;
    }

    if(sbMotionView != NULL)
        sbMotionView->Down = true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::BuildFeatureStatusBadges()
{
    const int BadgeWidth = 132;
    const int BadgeHeight = 30;
    const int GapX = 6;
    const int GapY = 6;
    int StatusIndex;
    int ColumnIndex;
    int RowIndex;
    TPanel *BadgePanel;
    TLabel *NameLabel;
    TLabel *ValueLabel;

    if(pnlFeatureStatus == NULL)
        return;

    pnlFeatureStatus->Left = 12;
    pnlFeatureStatus->Top = 206;
    pnlFeatureStatus->Width = 556;
    pnlFeatureStatus->Height = 72;
    pnlFeatureStatus->Color = clWhite;

    for(StatusIndex = 0; StatusIndex < MAIN_FEATURE_STATUS_COUNT; StatusIndex++)
    {
        ColumnIndex = StatusIndex % MAIN_FEATURE_STATUS_COLUMNS;
        RowIndex = StatusIndex / MAIN_FEATURE_STATUS_COLUMNS;

        BadgePanel = new TPanel(this);
        BadgePanel->Name = AnsiString("pnlFeatureBadge") + IntToStr(StatusIndex + 1);
        BadgePanel->Parent = pnlFeatureStatus;
        BadgePanel->Left = ColumnIndex * (BadgeWidth + GapX);
        BadgePanel->Top = RowIndex * (BadgeHeight + GapY);
        BadgePanel->Width = BadgeWidth;
        BadgePanel->Height = BadgeHeight;
        BadgePanel->Caption = "";
        BadgePanel->BevelInner = bvLowered;
        BadgePanel->BevelOuter = bvRaised;
        BadgePanel->Color = clBtnFace;
        BadgePanel->ParentColor = false;
        BadgePanel->Visible = (StatusIndex <= eMainFeatureSafeDoor);
        FeatureStatusPanels[StatusIndex] = BadgePanel;

        NameLabel = new TLabel(this);
        NameLabel->Name = AnsiString("lblFeatureName") + IntToStr(StatusIndex + 1);
        NameLabel->Parent = BadgePanel;
        NameLabel->Left = 4;
        NameLabel->Top = 2;
        NameLabel->Width = 38;
        NameLabel->Height = 24;
        NameLabel->Alignment = taCenter;
        NameLabel->AutoSize = false;
        NameLabel->Caption = GetFeatureStatusName(StatusIndex);
        NameLabel->Color = clBtnFace;
        NameLabel->Font->Charset = ANSI_CHARSET;
        NameLabel->Font->Color = clNavy;
        NameLabel->Font->Height = -11;
        NameLabel->Font->Name = "Arial";
        NameLabel->Font->Style = TFontStyles() << fsBold;
        NameLabel->ParentColor = false;
        NameLabel->ParentFont = false;
        FeatureStatusNameLabels[StatusIndex] = NameLabel;

        ValueLabel = new TLabel(this);
        ValueLabel->Name = AnsiString("lblFeatureValue") + IntToStr(StatusIndex + 1);
        ValueLabel->Parent = BadgePanel;
        ValueLabel->Left = 43;
        ValueLabel->Top = 4;
        ValueLabel->Width = 84;
        ValueLabel->Height = 20;
        ValueLabel->Alignment = taCenter;
        ValueLabel->AutoSize = false;
        ValueLabel->Color = clBtnFace;
        ValueLabel->Font->Charset = ANSI_CHARSET;
        ValueLabel->Font->Color = GetFeatureStatusDefaultColor(StatusIndex);
        ValueLabel->Font->Height = -12;
        ValueLabel->Font->Name = "Arial";
        ValueLabel->Font->Style = TFontStyles() << fsBold;
        ValueLabel->ParentColor = false;
        ValueLabel->ParentFont = false;
        FeatureStatusValueLabels[StatusIndex] = ValueLabel;

        if(StatusIndex <= eMainFeatureSafeDoor)
            SetFeatureStatusBadge(StatusIndex, GetFeatureStatusDefaultValue(StatusIndex), GetFeatureStatusDefaultColor(StatusIndex));
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMain::SetFeatureStatusBadge(int BadgeIndex, AnsiString ValueText, TColor ValueColor)
{
    if(BadgeIndex < 0 || BadgeIndex >= MAIN_FEATURE_STATUS_COUNT)
        return;
    if(FeatureStatusValueLabels[BadgeIndex] == NULL)
        return;

    FeatureStatusValueLabels[BadgeIndex]->Caption = ValueText;
    FeatureStatusValueLabels[BadgeIndex]->Font->Color = ValueColor;
    if(FeatureStatusPanels[BadgeIndex] != NULL)
        FeatureStatusPanels[BadgeIndex]->Visible = true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbLaguageClick(TObject *Sender)
{
    if(fLan == NULL)
        fLan = new TfLan(this);
    ShowTopForm(fLan, sbLaguage);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbProductClick(TObject *Sender)
{
    if(fSetup == NULL)
        fSetup = new TfSetup(this);
    ShowTopForm(fSetup, sbProduct);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbMaintanceClick(TObject *Sender)
{
    if(fMaintenance == NULL)
        fMaintenance = new TfMaintenance(this);
    ShowTopForm(fMaintenance, sbMaintance);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbOffsetClick(TObject *Sender)
{
    if(fOffset == NULL)
        fOffset = new TfOffset(this);
    ShowTopForm(fOffset, sbOffset);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbSpeedClick(TObject *Sender)
{
    if(fSpeed == NULL)
        fSpeed = new TfSpeed(this);
    ShowTopForm(fSpeed, sbSpeed);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbToolClick(TObject *Sender)
{
    if(FormSysTools == NULL)
        FormSysTools = new TFormSysTools(this);
    ShowTopForm(FormSysTools, sbTool);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbMessageClick(TObject *Sender)
{
    if(fNote == NULL)
        fNote = new TfNote(this);
    ShowTopForm(fNote, sbMessage);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbExitClick(TObject *Sender)
{
    if(sbExit != NULL)
        sbExit->Down = false;
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbMonitorClick(TObject *Sender)
{
    if(pgcMain != NULL && tsMonitorView != NULL)
        pgcMain->ActivePage = tsMonitorView;
    if(sbMonitor != NULL)
        sbMonitor->Down = false;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btnMainShowClick(TObject *Sender)
{
    if(pgcMain != NULL && tsMain != NULL)
        pgcMain->ActivePage = tsMain;
    if(btnMainShow != NULL)
        btnMainShow->Down = false;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbRecordClick(TObject *Sender)
{
    if(pgcMonitor != NULL && TabRecord != NULL)
        pgcMonitor->ActivePage = TabRecord;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbMotorViewClick(TObject *Sender)
{
    if(pgcMonitor != NULL && TabSheet7 != NULL)
        pgcMonitor->ActivePage = TabSheet7;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbMotionViewClick(TObject *Sender)
{
    if(pgcMonitor != NULL && TabSheet10 != NULL)
        pgcMonitor->ActivePage = TabSheet10;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbOtherClick(TObject *Sender)
{
    if(pgcMonitor != NULL && TabOther != NULL)
        pgcMonitor->ActivePage = TabOther;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::spbStripPosClick(TObject *Sender)
{
    if(pgcMonitor != NULL && tsStripPos != NULL)
        pgcMonitor->ActivePage = tsStripPos;
}
//---------------------------------------------------------------------------
void TfMain::LoadRunModePicture()
{
    NormalizeMainRunSettings();

    if(sbRealIcon != NULL)
        sbRealIcon->Visible = (HSys.LastSet.iRealDummy == REALLY);
    if(sbDummyIcon != NULL)
        sbDummyIcon->Visible = (HSys.LastSet.iRealDummy != REALLY);
    if(pnRealDummy == NULL)
        return;

    if(HSys.LastSet.iRealDummy == REALLY)
    {
        pnRealDummy->Caption = "Real";
        pnRealDummy->Font->Color = clRed;
    }
    else if(HSys.LastSet.iRealDummy == HAS_TRAY)
    {
        pnRealDummy->Caption = "HasTray";
        pnRealDummy->Font->Color = clBlack;
    }
    else
    {
        pnRealDummy->Caption = "Dummy";
        pnRealDummy->Font->Color = clRed;
    }
}
//---------------------------------------------------------------------------
void TfMain::LoadStartModePicture()
{
    NormalizeMainRunSettings();

    if(sbInitial != NULL)
        sbInitial->Visible = (HSys.LastSet.iStartMode == 0);
    if(sbContinue != NULL)
        sbContinue->Visible = (HSys.LastSet.iStartMode != 0);
    if(pnStartMode == NULL)
        return;

    if(HSys.LastSet.iStartMode == 0)
        pnStartMode->Caption = "Initial";
    else
        pnStartMode->Caption = "Continue";
    pnStartMode->Font->Color = clRed;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::pnStartModeClick(TObject *Sender)
{
    if(HSys.Sys.SystemStart)
        return;

    if(HSys.LastSet.iStartMode == 0)
    {
        HSys.LastSet.iStartMode = 1;
        RecordProcess("Change Continue Mode");
    }
    else
    {
        HSys.LastSet.iStartMode = 0;
        RecordProcess("Change Initial Start Mode");
    }

    LoadStartModePicture();
    SaveMainRunSettingsToIni();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::pnRealDummyClick(TObject *Sender)
{
    if(HSys.Sys.SystemStart)
        return;

    HSys.LastSet.iRealDummy++;
    if(HSys.LastSet.iRealDummy > REALLY)
        HSys.LastSet.iRealDummy = DUMMY;

    LoadRunModePicture();
    SaveMainRunSettingsToIni();
    RecordProcess(GetRunModeLogText());
    EventReport(SECS_EVENT.RealDummy);
}
//---------------------------------------------------------------------------
bool __fastcall TfMain::SmokeProbeTopForms(AnsiString &OpenedForms, AnsiString &ErrorText)
{
    AnsiString ProbeStage;

    OpenedForms = "";
    ErrorText = "";

    try
    {
        ProbeStage = "Language:create";
        if(fLan == NULL)
            fLan = new TfLan(this);
        ProbeStage = "Language:show";
        SmokeShowTopForm(fLan);
        OpenedForms += "Language";

        ProbeStage = "Product:create";
        if(fSetup == NULL)
            fSetup = new TfSetup(this);
        ProbeStage = "Product:show";
        SmokeShowTopForm(fSetup);
        OpenedForms += ",Product";

        ProbeStage = "Maintance:create";
        if(fMaintenance == NULL)
            fMaintenance = new TfMaintenance(this);
        ProbeStage = "Maintance:show";
        SmokeShowTopForm(fMaintenance);
        OpenedForms += ",Maintance";

        ProbeStage = "Teach:create";
        if(fTeach == NULL)
            fTeach = new TfTeach(this);
        ProbeStage = "Teach:show";
        SmokeShowTopForm(fTeach);
        OpenedForms += ",Teach";

        ProbeStage = "MotorTest:create";
        if(fMotorTest == NULL)
            fMotorTest = new TfMotorTest(this);
        ProbeStage = "MotorTest:show";
        SmokeShowTopForm(fMotorTest);
        OpenedForms += ",MotorTest";

        ProbeStage = "Offset:create";
        if(fOffset == NULL)
            fOffset = new TfOffset(this);
        ProbeStage = "Offset:show";
        SmokeShowTopForm(fOffset);
        OpenedForms += ",Offset";

        ProbeStage = "Speed:create";
        if(fSpeed == NULL)
            fSpeed = new TfSpeed(this);
        ProbeStage = "Speed:show";
        SmokeShowTopForm(fSpeed);
        OpenedForms += ",Speed";

        ProbeStage = "Tools:create";
        if(FormSysTools == NULL)
            FormSysTools = new TFormSysTools(this);
        ProbeStage = "Tools:show";
        SmokeShowTopForm(FormSysTools);
        OpenedForms += ",Tools";

        ProbeStage = "Message:create";
        if(fNote == NULL)
            fNote = new TfNote(this);
        ProbeStage = "Message:show";
        SmokeShowTopForm(fNote);
        OpenedForms += ",Message";
        return true;
    }
    catch(Exception &E)
    {
        ErrorText = ProbeStage + ":" + E.Message;
        if(OpenedForms != "")
            ErrorText = ErrorText + ";OPENED=" + OpenedForms;
    }
    catch(...)
    {
        ErrorText = ProbeStage + ":unknown exception";
        if(OpenedForms != "")
            ErrorText = ErrorText + ";OPENED=" + OpenedForms;
    }

    return false;
}
//---------------------------------------------------------------------------



void __fastcall TfMain::sbHome1Click(TObject *Sender)
{
#ifndef SOFT_SIMULATE
    int ret=ShowMyMessageBox_YES_NO("Confirm home?");
    if(ret==TMyMessageBox::msgrtnYES)
#endif
    {
        RecordProcess("HOME pressed");
        EventReport(SECS_EVENT.PressHome);
        ChangeRunMode(Run_Home);
        HSys.Sys.SystemStart=true;                                              //20140411 wei

        fAllMotorHome=false;
        SoftStart=true;
        bHomeByStart=false;
    }
#ifndef SOFT_SIMULATE
    else
        return;
#endif    
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbOneCycle1Click(TObject *Sender)
{
    if(HSys.Sys.RunMode==Run_Normal || HSys.Sys.RunMode==Run_CleanOut)
    {
        RecordProcess("ONE CYCLE pressed");
        EventReport(SECS_EVENT.PressOneCycle);
        ChangeRunMode(Run_OneCycle);
    }    
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbCleanOut1Click(TObject *Sender)
{
    if(HSys.Sys.RunMode==Run_Normal)
    {
        RecordProcess("CLEAN OUT pressed");
        EventReport(SECS_EVENT.PressCleanOut);
        ChangeRunMode(Run_CleanOut);
    }    
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbStart1Click(TObject *Sender)
{
    Start();        //JerryYang 20250106 : Run check    
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbPause1Click(TObject *Sender)
{
//    if(USE_SECS_GEM && HSys.FuncB.bN04_RunCheck && bPhysicalStart==true)        //JerryYang 20250106 : Run check
//    {
//        bPhysicalStart=false;
//    }
//    if(HSys.Sys.SystemStart==true)
//    {
//        RecordProcess("PAUSE pressed");
//        EventReport(SECS_EVENT.PressPause);
//        HSys.Sys.SystemStart=false;                                             //20140411 wei
//    }
//    SoftStop=true;    
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbStoreHangupClick(TObject *Sender)
{
//    StoreHangupData();    
}
//---------------------------------------------------------------------------
void TfMain::Start()
{
    if(HSys.Sys.SystemStart==false)
    {
        if(edLotNo->Text=="")                                                   //Steven 20240625 : block start without lot ID
        {
            ShowMyMessage("Please Enter LotID !");
            return;
        }
//        CheckContinusStartIsReady();                                            //Sam 20240710 : StartMode exception handling
//
//        RecordProcess("START pressed");
//
//        if(HasICUnderMachine())
//            EventReport(SECS_EVENT.PressStartWithIC);
//        else
//            EventReport(SECS_EVENT.PressStartWithoutIC);
//
//        if(USE_SECS_GEM && HSys.FuncB.bN04_RunCheck)
//        {
//            bPhysicalStart=true;
//            Timer6->Enabled=true;
//        }
//        else
//        {
//            HSys.Sys.SystemStart=true;                                              //20140411 wei
//            SoftStart=true;
//        }
//
//        if(fAllMotorHome==false)
//        {
//            bHomeByStart=true;
//        }
//        flagOneCycleTrayEnd=false;
    }
}
//---------------------------------------------------------------------------
