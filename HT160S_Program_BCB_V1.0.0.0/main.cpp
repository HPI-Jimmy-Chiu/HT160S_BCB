//---------------------------------------------------------------------------

#include "IncludeAllHeader.h"
#pragma hdrstop

#include <shellapi.h>   //AI(general) 20260608 : ShellExecute for Explorer /select on snapshot zip
#include "main.h"
#include "database.h"
#include "cStateRecordHT160.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "cprod.h"
#include "UserRoleManager.h"
#include "uruncontrol.h"
#include "iosetview.h"
#include "uteach.h"
#include "language.h"
#include "setup.h"
#include "data.h"
#include "maintenance.h"
#include "uMotorTest.h"
#include "uHome.h"
#include "uOffset.h"
#include "uspeed.h"
#include "systools.h"
#include "deviceinfo.h"
#include "aLoader.h"
#include "SecsGem/UsecegemMainFrom.h"
#include "SecsGem/uHGemHT160.h"
#include "SecsGem/uHGemLogForm.h"   //AI(ht160s-secsgem) 20260611 : SECS/GEM log monitor window
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "HTray"
#pragma link "ALed"
#pragma link "ALed"
#pragma link "MyLed"
#pragma resource "*.dfm"
TfMain *fMain;
//AI(HT160S-Maintainer) 20260603 : central alarm object (raise-hand queue), created in TfMain ctor
HAlarm *Alarm = NULL;
//---------------------------------------------------------------------------
static void ShowTopForm(TForm *FormPtr, TSpeedButton *ButtonPtr)
{
    if(FormPtr != NULL)
    {
        FormPtr->ShowModal();
    }
    if(ButtonPtr != NULL)
    {
        ButtonPtr->Down=false;
    }
}
//---------------------------------------------------------------------------
static void SmokeShowTopForm(TForm *FormPtr)
{
    if(FormPtr != NULL)
    {
        FormPtr->Show();
        FormPtr->Update();
        FormPtr->Hide();
    }
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
static bool IsSameMainText(AnsiString LeftText, AnsiString RightText)
{
    return (LeftText.Trim().UpperCase()==RightText.Trim().UpperCase());
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

    bUpdatingMainSelections = false;
    iLastSecsBadgeState = -1;   //AI(ht160s-secsgem) 20260612 : force the first periodic tick to paint the real HSMS state

    if(ComponentState.Contains(csDesigning))
        return;

    //AI(HT160S-Maintainer) 20260603 : create central alarm object before any module can raise an alarm
    if(Alarm==NULL)
        Alarm = new HAlarm(this);

    BuildFeatureStatusBadges();

    LoadMainRunSettingsFromIni();
    LoadRunModePicture();
    LoadStartModePicture();
    UserRoleManager.InitializeByBuildMode();

    bUpdatingMainSelections = true;
    UpdateWorkFileComboBox();
    RefreshMainUserSelect();
    bUpdatingMainSelections = false;

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

    SetupLotListGrid();                                                         //AI(HT160S-Maintainer) 20260604 : init multi-lot manual list grid
}
//---------------------------------------------------------------------------
void __fastcall TfMain::UpdateWorkFileComboBox()
{
    TSearchRec SearchRecord;
    AnsiString CurrentRecipe;
    AnsiString RecipeName;
    bool bFoundCurrent;
    int SearchResult;
    int ItemIndex;

    if(cb_WorkFile == NULL)
        return;

    RecipeManager.EnsureCurrentRecipeDir();
    CurrentRecipe = RecipeManager.GetCurrentRecipeName();
    bFoundCurrent = false;

    cb_WorkFile->Items->BeginUpdate();
    try
    {
        cb_WorkFile->Items->Clear();
        SearchResult = FindFirst(RecipeManager.GetDataRootPath()+AnsiString("\\*.*"), faAnyFile, SearchRecord);
        if(SearchResult == 0)
        {
            while(SearchResult == 0)
            {
                if((SearchRecord.Attr & faDirectory) != 0 &&
                   SearchRecord.Name != AnsiString(".") &&
                   SearchRecord.Name != AnsiString(".."))
                {
                    RecipeName = RecipeManager.NormalizeRecipeName(SearchRecord.Name);
                    cb_WorkFile->Items->Add(RecipeName);
                    if(IsSameMainText(RecipeName, CurrentRecipe))
                        bFoundCurrent = true;
                }
                SearchResult = FindNext(SearchRecord);
            }
            FindClose(SearchRecord);
        }
        if(!bFoundCurrent)
            cb_WorkFile->Items->Add(CurrentRecipe);
    }
    __finally
    {
        cb_WorkFile->Items->EndUpdate();
    }

    cb_WorkFile->Text = CurrentRecipe;
    for(ItemIndex=0; ItemIndex<cb_WorkFile->Items->Count; ItemIndex++)
    {
        if(IsSameMainText(cb_WorkFile->Items->Strings[ItemIndex], CurrentRecipe))
        {
            cb_WorkFile->ItemIndex = ItemIndex;
            break;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMain::RefreshMainUserSelect()
{
    int RoleLevel;

    if(cbbUserSelect == NULL)
        return;

    cbbUserSelect->Items->BeginUpdate();
    try
    {
        cbbUserSelect->Items->Clear();
        cbbUserSelect->Items->Add(UserRoleManager.GetLevelName(ROLE_OPERATION));
        cbbUserSelect->Items->Add(UserRoleManager.GetLevelName(ROLE_SUPERVISOR));
        cbbUserSelect->Items->Add(UserRoleManager.GetLevelName(ROLE_ENGINEER));
        cbbUserSelect->Items->Add(UserRoleManager.GetLevelName(ROLE_HONPREC));
    }
    __finally
    {
        cbbUserSelect->Items->EndUpdate();
    }

    RoleLevel = UserRoleManager.GetLevel();
    if(RoleLevel >= 0 && RoleLevel < cbbUserSelect->Items->Count)
        cbbUserSelect->ItemIndex = RoleLevel;
    cbbUserSelect->Text = UserRoleManager.GetLevelName();
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

        //AI(ht160s-secsgem) 20260611 : make the SECS status badge open the GEM log.
        //  Only wire/show it when the SECS/GEM paid feature is enabled; otherwise
        //  hide the badge entirely (the engine is also not booted in that case).
        if(StatusIndex == eMainFeatureSECS)
        {
            if(CosFunction.bUseSecsGem)
            {
                BadgePanel->OnClick = FeatureBadgeSecsClick;
                NameLabel->OnClick  = FeatureBadgeSecsClick;
                ValueLabel->OnClick = FeatureBadgeSecsClick;
                BadgePanel->Cursor  = crHandPoint;
                NameLabel->Cursor   = crHandPoint;
                ValueLabel->Cursor  = crHandPoint;
                BadgePanel->ShowHint = true;
                BadgePanel->Hint     = "Open SECS/GEM Log";
            }
            else
            {
                BadgePanel->Visible = false;   // feature not purchased -> no badge
            }
        }
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
//AI(ht160s-secsgem) 20260612 : sync the main-screen SECS badge to the live HSMS
//  link state. The badge text was previously a one-time static default ("OFF")
//  set in BuildFeatureStatusBadges and never updated, so it stayed OFF even when
//  the link was SELECTED. This reads HGem's live state and repaints the badge.
//
//  Resource note (req 3): this is called once per second from the SECS engine
//  timer, but it is edge-triggered - it only touches the VCL label when the
//  3-state code actually changes (cheap int read + compare otherwise).
void __fastcall TfMain::UpdateSecsFeatureBadge()
{
    //AI(ht160s-secsgem) 20260612 : feature off -> badge is hidden, nothing to do.
    if(!CosFunction.bUseSecsGem)
        return;

    //AI(ht160s-secsgem) 20260612 : 0=OFF/not connected, 1=CONNECT (TCP up, not
    //  yet SELECTED), 2=ONLINE (HSMS SELECTED). NULL engine -> treat as OFF.
    int State = 0;
    if(HGem != NULL)
    {
        if(HGem->IsSelected())
            State = 2;
        else if(HGem->IsConnected())
            State = 1;
        else
            State = 0;
    }

    //AI(ht160s-secsgem) 20260612 : edge-trigger guard - skip the VCL write when
    //  nothing changed since the last tick.
    if(State == iLastSecsBadgeState)
        return;
    iLastSecsBadgeState = State;

    switch(State)
    {
        case 2:  SetFeatureStatusBadge(eMainFeatureSECS, "ONLINE",  clGreen);  break;
        case 1:  SetFeatureStatusBadge(eMainFeatureSECS, "CONNECT", clOlive);  break;
        default: SetFeatureStatusBadge(eMainFeatureSECS, "OFF",     clGray);   break;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbLaguageClick(TObject *Sender)
{
    ShowTopForm(fLan, sbLaguage);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbProductClick(TObject *Sender)
{
    ShowTopForm(fSetup, sbProduct);
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : open the SECS/GEM log monitor when the operator
//  clicks the SECS status badge on the main feature panel.
void __fastcall TfMain::FeatureBadgeSecsClick(TObject *Sender)
{
    //AI(ht160s-secsgem) 20260611 : defensive gate - never open the SECS log view
    //  when the paid feature is off (badge should already be hidden).
    if(!CosFunction.bUseSecsGem)
        return;
    ShowSecsGemLog();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbMaintanceClick(TObject *Sender)
{
    ShowTopForm(fMaintenance, sbMaintance);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbOffsetClick(TObject *Sender)
{
    ShowTopForm(fOffset, sbOffset);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbSpeedClick(TObject *Sender)
{
    ShowTopForm(fSpeed, sbSpeed);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbToolClick(TObject *Sender)
{
    ShowTopForm(FormSysTools, sbTool);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbMessageClick(TObject *Sender)
{
    ShowTopForm(fNote, sbMessage);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
//AI(general) 20260601 : stop the run-control thread before the VCL message
//loop ends (ref HT172 main.cpp "MyThread->Terminate(); MyThread->WaitFor();").
//Must run here while messages are still pumped, otherwise the worker thread's
//Synchronize(MainProc) can never complete and WaitFor would deadlock.
void __fastcall TfMain::FormClose(TObject *Sender, TCloseAction &Action)
{
    if(MyThread != NULL)
    {
        MyThread->Terminate();
        MyThread->WaitFor();
    }

    //AI(HT160S-Maintainer) 20260603 : release central alarm object
    if(Alarm != NULL)
    {
        delete Alarm;
        Alarm = NULL;
    }
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
// Motion View motor-position monitor gate. Only refresh motor->panel positions
// while the Motion View page is visible (performance guard, matches HT172).
void __fastcall TfMain::SetSimulateScreenStatus()
{
    static bool bLastStatus = false;
    static bool bSimuInited = false;

    if(HSys.MotPtr == NULL)
        return;

    // One-time motor->panel binding (lazy, after motors are initialized).
    if(bSimuInited == false)
    {
        InitSimulateScreenBinding();
        bSimuInited = true;
    }

    bool bStatus = (pgcMain != NULL && pgcMain->ActivePage == tsMonitorView &&
                    pgcMonitor != NULL && pgcMonitor->ActivePage == TabSheet10);

    if(bStatus != bLastStatus)
    {
        bLastStatus = bStatus;
        for(int i = 0; i < HSys.iTotalMotor; i++)
        {
            if(HSys.MotPtr[i] != NULL)
                HSys.MotPtr[i]->SetShowSimulateCompomentFlag(bStatus);
        }

        // On entering the Motion View page, re-sync the content grids to the
        // active Recipe Col/Row. Covers Recipe change / Lot Start that happened
        // before the page was opened, with zero per-frame ini I/O.
        if(bStatus)
            SyncMonitorTrayDivision();
    }

    if(bStatus)
    {
        // Poll current position so bound panels track the machine live.
        // ReadPos() internally calls UpdateSimulateCompomentPosition().
        for(int i = 0; i < HSys.iTotalMotor; i++)
        {
            if(HSys.MotPtr[i] != NULL)
                HSys.MotPtr[i]->ReadPos();
        }

        // Hide each moving-tray CONTENT grid (mt*TrayWork) until its car
        // actually holds a tray. Loader L/R show a full tray on feed; Auto1..6
        // stay hidden until TrayArm delivers an empty tray, then show empty and
        // fill as SortArm places ICs. Driven by fHasTray every frame because the
        // discharge path clears fHasTray directly (not via ClearTray).
        // Display only - the position carrier panel (pl*TrayWork) is unaffected.
        if(HSys.VMotPtr != NULL)
        {
            for(int i = 0; i < HSys.iTotalVMotor; i++)
            {
                if(HSys.VMotPtr[i] != NULL)
                    HSys.VMotPtr[i]->UpdateTrayVisibleByHasTray();
            }
        }
    }
}
//---------------------------------------------------------------------------
// Motor View page (TabSheet7) live status grid. Ported from HT172 0420
// TfMain::ShowMotorInfo(). One-time header build, then per-frame fill of
// Target/Position/Encoder plus a colored LED block per motor status bit.
// Only refreshes while the Motor View page is actually visible (perf guard).
//AI(HT160S-Maintainer) 20260610 : populate empty Motor View grid (ref HT172 0420)
void __fastcall TfMain::ShowMotorInfo()
{
    static bool flag=false;
    TRect R;

    if(sgMotorStatus==NULL || pgcMonitor==NULL)
        return;
    if(pgcMonitor->ActivePage!=TabSheet7)                                       // only update when Motor View is shown //
        return;

    if(flag==false)
    {
        flag=true;
        sgMotorStatus->ColCount=iMotLedTotalCnt+4;
        sgMotorStatus->ColWidths[0]=120;
        sgMotorStatus->Cells[ 0][0]="Motor Name";
        sgMotorStatus->Cells[ 1][0]="Target";
        sgMotorStatus->Cells[ 2][0]="Position";
        sgMotorStatus->Cells[ 3][0]="Encoder";
        sgMotorStatus->Cells[ 4][0]="CW";
        sgMotorStatus->Cells[ 5][0]="HOME";
        sgMotorStatus->Cells[ 6][0]="CCW";
        sgMotorStatus->Cells[ 7][0]="Emg";
        sgMotorStatus->Cells[ 8][0]="Alarm";
        sgMotorStatus->Cells[ 9][0]="SoftCW";
        sgMotorStatus->Cells[10][0]="SoftCCW";
        sgMotorStatus->Cells[11][0]="ServoAlarm";
        sgMotorStatus->Cells[12][0]="InPos";
        sgMotorStatus->Cells[13][0]="Z Phase";
        sgMotorStatus->Cells[14][0]="ServoOn";
        sgMotorStatus->RowCount=HSys.iTotalMotor+2;
        for(int i=0; i<HSys.iTotalMotor; i++)
        {
            if(HSys.MotPtr[i]==NULL)
                continue;
            sgMotorStatus->Cells[0][i+2]=HSys.MotPtr[i]->NumberAlias;
        }
    }

    if(pgcMain==NULL || pgcMain->ActivePage!=tsMonitorView)
        return;

    for(int i=0; i<HSys.iTotalMotor; i++)
    {
        if(HSys.MotPtr[i]==NULL || HSys.MotPtr[i]->GetEnable()==false)
            continue;

        if(HSys.Sys.SystemStart==false)
            HSys.MotPtr[i]->ReadPos();
        sgMotorStatus->Cells[1][i+2]=HSys.MotPtr[i]->TargetPosition;
        sgMotorStatus->Cells[2][i+2]=HSys.MotPtr[i]->Position;
        sgMotorStatus->Cells[3][i+2]=HSys.MotPtr[i]->EncoderPosition;
        HSys.MotPtr[i]->ScanMotorStatus();
        for(int j=0; j<iMotLedTotalCnt; j++)
        {
            R=sgMotorStatus->CellRect(4+j, i+2);
            R.Top++;
            R.Bottom-=1;
            R.Left++;
            R.Right-=2;

            if(HSys.MotPtr[i]->Led[j])
            {
                if(j==iHomeLed)
                {
                    sgMotorStatus->Canvas->Brush->Color=clGreen;
                }
                else
                {
                    if(j==iInposLed)
                    {
                        if(HSys.Sys.SystemStart)
                            sgMotorStatus->Canvas->Brush->Color=clGreen;
                        else
                            sgMotorStatus->Canvas->Brush->Color=clRed;
                    }
                    else
                    {
                        sgMotorStatus->Canvas->Brush->Color=clRed;
                    }
                }
                sgMotorStatus->Canvas->FillRect(R);
            }
            else
            {
                sgMotorStatus->Canvas->Brush->Color=sgMotorStatus->Color;
                sgMotorStatus->Canvas->FillRect(R);
            }
        }
    }
}
//---------------------------------------------------------------------------
// D1 moving-tray panel: ONE panel per column. Panel TOP follows the physical
// Y motor (Rear pixel 73 .. Car pixel 595); tray CONTENT comes from the virtual
// motor grid. akLeft => vertical (Top) travel. Display only - no machine risk.
// If a panel travels the wrong way on-machine, swap the 73/595 pixel ends.
static void BindMovingTrayPanel(TTrayMotor *PosMot, TTrayMotor *ContentMot,
                                TPanel *Panel, TTMyTray *Tray)
{
    if(PosMot != NULL && Panel != NULL)
        PosMot->SetSimulateCompoment(Panel, akLeft,
            PosMot->GetSoftLimitN(), PosMot->GetSoftLimitP(), 595, 73);
    if(ContentMot != NULL && Tray != NULL)
        ContentMot->SetHTrayPanel(Tray);
}
//---------------------------------------------------------------------------
// Read the active Recipe tray Col/Row (XDivision/YDivision) from the in-memory
// TrayForm structure (CosFunction) - the SAME single source the Loader/SortArm/
// Auto modules read. TrayForm is (re)loaded at boot, on recipe change and after
// a Setup save, so this no longer re-parses setup.ini on every Monitor refresh.
// Clamped to the Tray.Data array bounds so the Monitor grid can never exceed
// what Refresh() can paint.
//AI(HT160S-Maintainer) 20260608 : source tray geometry from TrayForm structure
static void ReadRecipeTrayDivision(int &X, int &Y)
{
    X=TrayForm.XDivision;
    Y=TrayForm.YDivision;
    if(X<1) X=1;
    if(X>MAX_TRAY_X) X=MAX_TRAY_X;
    if(Y<1) Y=1;
    if(Y>MAX_TRAY_Y) Y=MAX_TRAY_Y;
}
//---------------------------------------------------------------------------
// Apply Recipe Col/Row to one Monitor content grid. Setting XItem/YItem rebuilds
// and CLEARS the grid cells (TTMyTray::SetXItem -> ClearCell), so the bound
// virtual motor must repaint its content afterwards. Only touch the grid when
// the dimensions actually changed (avoids needless clear/flicker). Display only.
static void ApplyTrayDivisionToPanel(TTMyTray *Tray, TTrayMotor *ContentMot, int X, int Y)
{
    if(Tray==NULL)
        return;
    if(Tray->XItem==X && Tray->YItem==Y)
        return;
    Tray->XItem=X;
    Tray->YItem=Y;
    if(ContentMot!=NULL)
        ContentMot->Refresh();   // repaint cells from Tray.Data after the resize
}
//---------------------------------------------------------------------------
// Bind Motion View moving carriages to their X-axis motors. Display only.
// Fact range = motor soft limits; Ref range = pixel Left span of the track bar.
// If a carriage moves the wrong direction on the machine, swap the pixel ends
// (or the soft-limit args) here during visual calibration - no logic risk.
void __fastcall TfMain::InitSimulateScreenBinding()
{
    // Tray Arm X carriage (horizontal): track bar Panel36 Left 114..945,
    // carriage plTrayArm width 97 => Left range 114..848.
    if(HSys.Mot.MTrayArmX != NULL && plTrayArm != NULL)
        HSys.Mot.MTrayArmX->SetSimulateCompoment(plTrayArm, akTop,
            HSys.Mot.MTrayArmX->GetSoftLimitN(), HSys.Mot.MTrayArmX->GetSoftLimitP(),
            114, 848);

    // Sort Arm X carriage (horizontal): track bar Panel153 Left 114..945,
    // carriage palSortArm1 width 109 => Left range 114..836.
    if(HSys.Mot.MSortingArmX != NULL && palSortArm1 != NULL)
        HSys.Mot.MSortingArmX->SetSimulateCompoment(palSortArm1, akTop,
            HSys.Mot.MSortingArmX->GetSoftLimitN(), HSys.Mot.MSortingArmX->GetSoftLimitP(),
            114, 836);

    // Top CCD scan carriage (horizontal): same track as the arms,
    // carriage plCCDMotorLoader width 89 => Left range 114..856.
    if(HSys.Mot.MTopCCDX != NULL && plCCDMotorLoader != NULL)
        HSys.Mot.MTopCCDX->SetSimulateCompoment(plCCDMotorLoader, akTop,
            HSys.Mot.MTopCCDX->GetSoftLimitN(), HSys.Mot.MTopCCDX->GetSoftLimitP(),
            288, 456);

    // Sort Arm suck-Z strokes shown as small vertical LED travel on palSortArm1.
    // akLeft => vertical (Top) move; Fact = soft limits (self-calibrating).
    // Pixel Top 7 (up) .. 14 (down) within the 41px arm panel - display only.
    if(HSys.Mot.MSuckZ_1 != NULL && ledSortArm1ZA != NULL)
        HSys.Mot.MSuckZ_1->SetSimulateCompoment(ledSortArm1ZA, akLeft,
            HSys.Mot.MSuckZ_1->GetSoftLimitN(), HSys.Mot.MSuckZ_1->GetSoftLimitP(), 7, 14);
    if(HSys.Mot.MSuckZ_2 != NULL && ledSortArm1ZB != NULL)
        HSys.Mot.MSuckZ_2->SetSimulateCompoment(ledSortArm1ZB, akLeft,
            HSys.Mot.MSuckZ_2->GetSoftLimitN(), HSys.Mot.MSuckZ_2->GetSoftLimitP(), 7, 14);
    if(HSys.Mot.MSuckZ_3 != NULL && ledSortArm1ZE != NULL)
        HSys.Mot.MSuckZ_3->SetSimulateCompoment(ledSortArm1ZE, akLeft,
            HSys.Mot.MSuckZ_3->GetSoftLimitN(), HSys.Mot.MSuckZ_3->GetSoftLimitP(), 7, 14);
    if(HSys.Mot.MSuckZ_4 != NULL && ledSortArm1ZF != NULL)
        HSys.Mot.MSuckZ_4->SetSimulateCompoment(ledSortArm1ZF, akLeft,
            HSys.Mot.MSuckZ_4->GetSoftLimitN(), HSys.Mot.MSuckZ_4->GetSoftLimitP(), 7, 14);

    //AI(ht172-to-ht160-porting) 20260609 : also wire the four Sort Arm Z LEDs
    //to the sucker group so each lights when its slot holds an IC (HT172
    //behavior; driven from TSortArmModule::UpdateKitSuckState). Slots 0..3 map
    //to ledSortArm1ZA/ZB/ZE/ZF (same order as MSuckZ_1..4). Display only.
    if(ledSortArm1ZA != NULL) HSys.Suck.SortArmSuck.SetMyLed(0, 0, ledSortArm1ZA);
    if(ledSortArm1ZB != NULL) HSys.Suck.SortArmSuck.SetMyLed(0, 1, ledSortArm1ZB);
    if(ledSortArm1ZE != NULL) HSys.Suck.SortArmSuck.SetMyLed(0, 2, ledSortArm1ZE);
    if(ledSortArm1ZF != NULL) HSys.Suck.SortArmSuck.SetMyLed(0, 3, ledSortArm1ZF);

    // Auto1..6 / Empty / Loader L-R moving tray panels (D1: one panel per
    // column). POSITION from physical HSys.Mot.M*Y; CONTENT from virtual
    // HSys.VMot.MM*Y. See BindMovingTrayPanel for the pixel travel range.
    BindMovingTrayPanel(HSys.Mot.MAutoY_1, HSys.VMot.MMAutoY_1, plAuto1TrayWork, mtAuto1TrayWork);
    BindMovingTrayPanel(HSys.Mot.MAutoY_2, HSys.VMot.MMAutoY_2, plAuto2TrayWork, mtAuto2TrayWork);
    BindMovingTrayPanel(HSys.Mot.MAutoY_3, HSys.VMot.MMAutoY_3, plAuto3TrayWork, mtAuto3TrayWork);
    BindMovingTrayPanel(HSys.Mot.MAutoY_4, HSys.VMot.MMAutoY_4, plAuto4TrayWork, mtAuto4TrayWork);
    BindMovingTrayPanel(HSys.Mot.MAutoY_5, HSys.VMot.MMAutoY_5, plAuto5TrayWork, mtAuto5TrayWork);
    BindMovingTrayPanel(HSys.Mot.MAutoY_6, HSys.VMot.MMAutoY_6, plAuto6TrayWork, mtAuto6TrayWork);

    BindMovingTrayPanel(HSys.Mot.MEmptyY, HSys.VMot.MMEmptyY, plEmptyTrayWork, mtEmptyTrayWork);

    // Loader has two physical Y lanes: MLoaderY_1 (Left) and MLoaderY_2 (Right).
    BindMovingTrayPanel(HSys.Mot.MLoaderY_1, HSys.VMot.MMLoaderY_1, plLoaderLTrayWork, mtLoaderLTrayWork);
    BindMovingTrayPanel(HSys.Mot.MLoaderY_2, HSys.VMot.MMLoaderY_2, plLoaderRTrayWork, mtLoaderRTrayWork);

    // Size every content grid to the active Recipe Col/Row (XDivision/YDivision)
    // instead of leaving them at the fixed dfm 4x5.
    SyncMonitorTrayDivision();
}
//---------------------------------------------------------------------------
// Sync every Monitor moving-tray CONTENT grid (mt*TrayWork) to the active Recipe
// Col/Row. Without this the grids stay at their fixed dfm 4x5 and never follow
// the recipe's XDivision/YDivision. Called at bind time and whenever the user
// enters the Motion View page (covers Recipe change / Lot Start). Display only -
// never gates motion, IO, vacuum, or sensors.
void __fastcall TfMain::SyncMonitorTrayDivision()
{
    int X=1, Y=1;
    ReadRecipeTrayDivision(X, Y);

    ApplyTrayDivisionToPanel(mtAuto1TrayWork, HSys.VMot.MMAutoY_1, X, Y);
    ApplyTrayDivisionToPanel(mtAuto2TrayWork, HSys.VMot.MMAutoY_2, X, Y);
    ApplyTrayDivisionToPanel(mtAuto3TrayWork, HSys.VMot.MMAutoY_3, X, Y);
    ApplyTrayDivisionToPanel(mtAuto4TrayWork, HSys.VMot.MMAutoY_4, X, Y);
    ApplyTrayDivisionToPanel(mtAuto5TrayWork, HSys.VMot.MMAutoY_5, X, Y);
    ApplyTrayDivisionToPanel(mtAuto6TrayWork, HSys.VMot.MMAutoY_6, X, Y);

    ApplyTrayDivisionToPanel(mtEmptyTrayWork, HSys.VMot.MMEmptyY, X, Y);

    ApplyTrayDivisionToPanel(mtLoaderLTrayWork, HSys.VMot.MMLoaderY_1, X, Y);
    ApplyTrayDivisionToPanel(mtLoaderRTrayWork, HSys.VMot.MMLoaderY_2, X, Y);
}
//---------------------------------------------------------------------------
// Simulation IC: assign virtual ICs to incoming trays without CCD hardware.
// Runtime flag only (not SOFT_SIMULATE); sensors/cylinders still operate.
void __fastcall TfMain::cbEnableSimulationClick(TObject *Sender)
{
    (void)Sender;
    tSimuData.bRunSimulation=cbEnableSimulation->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btnLoadSimuDataClick(TObject *Sender)
{
    //AI(HT160S-Maintainer) 20260604 : P2 auto-fill 5 simulate Lots, 20 unique 2D
    //codes each (2D_Simu_1..100, never repeated), Bin cycling 1..6. Loads the
    //reverse-lookup registry (LotRegistry) then projects it onto sgLotList via
    //the single shared refresh (same path SECS / JSON use).
    const int SimuLotCount=5;
    const int SimuCodePerLot=20;
    AnsiString DupExistingLot;
    AnsiString LotID;
    AnsiString Code;
    int Bin;
    int CodeSeq;

    (void)Sender;
    tSimuData.Clear();
    tSimuData.bRunSimulation=cbEnableSimulation->Checked;

    LotRegistry.Clear();

    CodeSeq=0;
    for(int LotIdx=0; LotIdx<SimuLotCount; LotIdx++)
    {
        LotID=AnsiString("SIMU_LOT_")+AnsiString((char)('A'+LotIdx));
        LotRegistry.AddLot(LotID, HT160_LOT_SOURCE_OFFLINE, "", "");

        for(int j=0; j<SimuCodePerLot; j++)
        {
            CodeSeq++;
            Code=AnsiString("2D_Simu_")+IntToStr(CodeSeq);
            Bin=((CodeSeq-1)%6)+1;
            LotRegistry.AddItem(LotID, Code, Bin, DupExistingLot);
        }
    }

    //AI(general) 20260610 : single source of truth = LotRegistry; project once.
    RefreshLotListFromRegistry();
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
void __fastcall TfMain::cb_WorkFileDropDown(TObject *Sender)
{
    (void)Sender;

    bUpdatingMainSelections = true;
    UpdateWorkFileComboBox();
    bUpdatingMainSelections = false;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::cb_WorkFileChange(TObject *Sender)
{
    AnsiString SelectedRecipe;
    AnsiString CurrentRecipe;
    AnsiString LogText;

    (void)Sender;
    if(bUpdatingMainSelections || cb_WorkFile == NULL)
        return;

    CurrentRecipe = RecipeManager.GetCurrentRecipeName();
    if(cb_WorkFile->Text.Trim() == AnsiString(""))
    {
        cb_WorkFile->Text = CurrentRecipe;
        ShowMyMessage("Recipe name cannot be empty.");
        return;
    }

    if(HSys.Sys.SystemStart)
    {
        cb_WorkFile->Text = CurrentRecipe;
        ShowMyMessage("Can not change recipe while machine is running.");
        return;
    }

    SelectedRecipe = RecipeManager.NormalizeRecipeName(cb_WorkFile->Text);
    if(!RecipeManager.RecipeExists(SelectedRecipe))
    {
        cb_WorkFile->Text = CurrentRecipe;
        ShowMyMessage("Recipe does not exist.");
        return;
    }

    if(IsSameMainText(SelectedRecipe, CurrentRecipe))
    {
        cb_WorkFile->Text = CurrentRecipe;
        return;
    }

    RecipeManager.SetCurrentRecipeName(SelectedRecipe);
    RecipeManager.SaveLastRecipeName();
    RecipeManager.EnsureCurrentRecipeDir();

    //AI(HT160S-Maintainer) 20260608 : reload the Tray geometry structure for the
    //newly selected recipe so Loader/SortArm/Auto and the Monitor read the right
    //values immediately (single source = TrayForm, not the Setup form UI).
    TrayForm.Load();
    bUpdatingMainSelections = true;
    UpdateWorkFileComboBox();
    bUpdatingMainSelections = false;

    if(fSetup != NULL)
        fSetup->OpenWorkFile();
    if(fTeach != NULL)
        fTeach->OpenWorkFile();
    if(fMaintenance != NULL)
        fMaintenance->OpenWorkFile();

    //AI(HT160S-Maintainer) 20260608 : refresh Monitor Col/Row grid for new recipe
    SyncMonitorTrayDivision();

    LogText = AnsiString("Change Recipe to ")+RecipeManager.GetCurrentRecipeName();
    RecordProcess(LogText);
    EventReport(SECS_EVENT.RecipeChange);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::cbbUserSelectChange(TObject *Sender)
{
    int RoleLevel;
    int OldRoleLevel;
    AnsiString LogText;

    (void)Sender;
    if(bUpdatingMainSelections || cbbUserSelect == NULL)
        return;

    RoleLevel = cbbUserSelect->ItemIndex;
    if(!UserRoleManager.IsValidLevel(RoleLevel))
    {
        bUpdatingMainSelections = true;
        RefreshMainUserSelect();
        bUpdatingMainSelections = false;
        return;
    }

    OldRoleLevel = UserRoleManager.GetLevel();
    if(RoleLevel == ROLE_OPERATION)
    {
        UserRoleManager.SetUserToOperation(true);
    }
    else
    {
        #ifdef SOFT_SIMULATE
        UserRoleManager.ForceLevel(RoleLevel, UserRoleManager.GetLevelName(RoleLevel));
        #else
        ShowMyMessage("User password login is not available yet.");
        bUpdatingMainSelections = true;
        RefreshMainUserSelect();
        bUpdatingMainSelections = false;
        return;
        #endif
    }

    bUpdatingMainSelections = true;
    RefreshMainUserSelect();
    bUpdatingMainSelections = false;

    if(OldRoleLevel != UserRoleManager.GetLevel())
    {
        LogText = AnsiString("Change User to ")+UserRoleManager.GetLevelName();
        RecordProcess(LogText);
        EventReport(SECS_EVENT.ChangeUser);
    }
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
    //AI(HT160S-Maintainer) 20260602 : HT172-style home. Press Home starts the
    //  full-machine home (reuse Run_Home engine) and shows the Home monitor
    //  non-modally; the monitor auto-closes when homing finishes (see uHome).
#ifndef SOFT_SIMULATE
    int ret=ShowMyMessageBox_YES_NO("Confirm home?");
    if(ret==TMyMessageBox::msgrtnYES)
#endif
    {
        fHome->lstHomeMsg->Clear();
        fHome->lstHomeMsg->Items->Insert(0, "Starting home procedure....");
        fHome->Show();

        RecordProcess("HOME pressed");
        EventReport(SECS_EVENT.PressHome);
        ChangeRunMode(Run_Home);
        HSys.Sys.SystemStart=true;                                              //20140411 wei

        fAllMotorHome=false;
        ArmMotorHome();                                                        //AI(HT160S-Maintainer) 20260602 : force fresh full-machine home
        SoftStart=true;
        bHomeByStart=false;
    }
#ifndef SOFT_SIMULATE
    else
        return;
#endif
    sbHome1->Down=false;
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
        HSys.Sys.bCleanOut=true;
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
    if(HSys.Sys.SystemStart==true)
    {
        RecordProcess("PAUSE pressed");
        EventReport(SECS_EVENT.PressPause);
        HSys.Sys.SystemStart=false;                                             //20140411 wei
    }
    SoftStop=true;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbStoreHangupClick(TObject *Sender)
{
    //AI(general) 20260608 : manual State Record snapshot (no FSM). Packages
    //TaskHistory.csv + MachineState.ini + full config into a single zip under
    //D:\HT160S_StateRecord\ for offline analysis.
    if(gStateRecord==NULL)
        return;
    Screen->Cursor = crHourGlass;
    bool bOk = gStateRecord->TriggerSnapshot("Manual");
    Screen->Cursor = crDefault;
    if(bOk)
    {
        //AI(general) 20260608 : no MessageBox - open Explorer with the zip selected.
        AnsiString Zip = gStateRecord->LastSnapshotZip;
        if(Zip!="" && FileExists(Zip))
        {
            AnsiString Param = "/select,\"" + Zip + "\"";
            ShellExecute(Handle, "open", "explorer.exe", Param.c_str(), NULL, SW_SHOWNORMAL);
        }
        else
        {
            //fallback : open the State Record root folder
            ShellExecute(Handle, "open", "explorer.exe", "D:\\HT160S_StateRecord\\", NULL, SW_SHOWNORMAL);
        }
    }
    else
        ShowMyMessage("State Record snapshot failed (check 7-Zip / disk).");
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
        RecordProcess("START pressed");
        tSimuData.bRunSimulation=cbEnableSimulation->Checked;                   //sync simulation IC flag from UI at lot start
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
            HSys.Sys.SystemStart=true;                                              //20140411 wei
            SoftStart=true;
            g_DeviceInfo.OnLotStart(edLotNo->Text, Now());                          //AI(HT160S-Maintainer) 20260603 : start per-IC production trace batch
            if(LoaderModule!=NULL)
                LoaderModule->SetCurrentLotNumber(edLotNo->Text);                   //AI(HT160S-Maintainer) 20260604 : P3 2D->Bin lookup keyed by lot number
//        }
//
        if(fAllMotorHome==false)
        {
            bHomeByStart=true;
        }
//        flagOneCycleTrayEnd=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TfMain::FormShow(TObject *Sender)
{
    (void)Sender;
    #ifdef SOFT_SIMULATE
//    cbEnableSimulation->Checked=true;
//    btnLoadSimuData->Click();
    #endif

    //AI(HT160S-Maintainer) 20260608 : power-on restore is two SEPARATE halves.
    //  1) Setup geometry (tray Col/Row/Pitch) : owned by the TrayForm structure
    //     (CosFunction). Already loaded at boot in InitialCosFunction(); reload
    //     here so the Monitor grid is painted from the recipe, not .dfm defaults.
    //     This is the half the earlier RestoreLastWorkOrder()-only fix missed.
    //  2) Work order (Lot list + run counters) : restored by RestoreLastWorkOrder.
    TrayForm.Load();
    SyncMonitorTrayDivision();
    RestoreLastWorkOrder();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btnLotStartClick(TObject *Sender)
{
    //AI(HT160S-Maintainer) 20260604 : P1 push the manual Lot list into the
    //multi-lot registry (offline source), then start the sort run. Each Lot's
    //2D->Bin items are loaded separately (simulate fill or JSON LoadLatest).
    AnsiString FirstLot;
    AnsiString LotText;

    (void)Sender;
    if(sgLotList==NULL)
        return;

    if(GetLotListCount()==0)
    {
        ShowMyMessage("Please add at least one Lot to the list !");
        return;
    }

    FirstLot="";
    for(int RowIndex=1; RowIndex<sgLotList->RowCount; RowIndex++)
    {
        LotText=sgLotList->Cells[0][RowIndex].Trim();
        if(LotText=="")
            continue;
        LotRegistry.AddLot(LotText, HT160_LOT_SOURCE_OFFLINE, "", "");
        if(FirstLot=="")
            FirstLot=LotText;
    }

    MachineRun.bRunning=true;
    MachineRun.iActiveLotCount=LotRegistry.GetLotCount();

    edLotNo->Text=FirstLot;
    //AI(HT160S-Maintainer) 20260608 : need1 : persist the started work order so
    //the next power-on can restore it (see RestoreLastWorkOrder / FormShow).
    SaveLastLotList();
    RecordProcess("LOT START pressed");
    Start();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btnLotEndClick(TObject *Sender)
{
    //AI(HT160S-Maintainer) 20260604 : P1 stop the sort run (HT172 LotEnd analog).
    (void)Sender;
    if(HSys.Sys.SystemStart==true)
    {
        RecordProcess("LOT END pressed");
        HSys.Sys.SystemStart=false;
    }
    SoftStop=true;
    MachineRun.bRunning=false;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260604 : Lot Manual Edit list helpers (multi-lot queue, UI layer)
void __fastcall TfMain::SetupLotListGrid()
{
    if(sgLotList==NULL)
        return;

    sgLotList->ColCount=1;
    sgLotList->FixedCols=0;
    sgLotList->FixedRows=1;
    if(sgLotList->RowCount<2)
        sgLotList->RowCount=2;
    sgLotList->Cells[0][0]="Lot No.";

    for(int RowIndex=1; RowIndex<sgLotList->RowCount; RowIndex++)
        sgLotList->Cells[0][RowIndex]="";
}
//---------------------------------------------------------------------------
int __fastcall TfMain::GetLotListCount()
{
    int LotCount=0;

    if(sgLotList==NULL)
        return 0;

    for(int RowIndex=1; RowIndex<sgLotList->RowCount; RowIndex++)
    {
        if(sgLotList->Cells[0][RowIndex].Trim()!="")
            LotCount++;
    }
    return LotCount;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260608 : need1 : last-used work order persistence.
//The manual Lot list + active Lot No are written to system\LastLotList.ini at
//Lot Start and reloaded at power-on, so the operator's previous work order
//reappears (HT160S previously always opened with an empty list). The JSON-
//delivered work order (HT160S_LotInfo) is auto-loaded first via LoadLatest;
//this file is the fallback for offline / simulated lots.
//---------------------------------------------------------------------------
static AnsiString GetLastLotListFileName()
{
    return HSys.CurrentDir + AnsiString("\\system\\LastLotList.ini");
}
//---------------------------------------------------------------------------
void __fastcall TfMain::SaveLastLotList()
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString LotText;
    int SavedCount;

    if(sgLotList==NULL)
        return;

    FileName=GetLastLotListFileName();
    ForceDirectories(ExtractFilePath(FileName));
    Ini=new TIniFile(FileName);
    try
    {
        Ini->EraseSection("LotList");
        SavedCount=0;
        for(int RowIndex=1; RowIndex<sgLotList->RowCount; RowIndex++)
        {
            LotText=sgLotList->Cells[0][RowIndex].Trim();
            if(LotText=="")
                continue;
            Ini->WriteString("LotList", AnsiString("Lot")+IntToStr(SavedCount), LotText);
            SavedCount++;
        }
        Ini->WriteInteger("LotList", "Count", SavedCount);
        Ini->WriteString("LotList", "ActiveLot", (edLotNo!=NULL) ? edLotNo->Text.Trim() : AnsiString(""));
    }
    __finally
    {
        delete Ini;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMain::LoadLastLotList()
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString LotText;
    AnsiString ActiveLot;
    int SavedCount;
    int Row;

    if(sgLotList==NULL)
        return;

    FileName=GetLastLotListFileName();
    if(!FileExists(FileName))
        return;

    Ini=new TIniFile(FileName);
    try
    {
        SavedCount=Ini->ReadInteger("LotList", "Count", 0);
        ActiveLot=Ini->ReadString("LotList", "ActiveLot", "");

        SetupLotListGrid();
        if(SavedCount<=0)
            return;

        sgLotList->RowCount=1+SavedCount;
        Row=0;
        for(int Index=0; Index<SavedCount; Index++)
        {
            LotText=Ini->ReadString("LotList", AnsiString("Lot")+IntToStr(Index), "").Trim();
            if(LotText=="")
                continue;
            sgLotList->Cells[0][1+Row]=LotText;
            LotRegistry.AddLot(LotText, HT160_LOT_SOURCE_OFFLINE, "", "");
            Row++;
        }
        if(sgLotList->RowCount<2)
            sgLotList->RowCount=2;
        if(edLotNo!=NULL)
            edLotNo->Text=ActiveLot;
    }
    __finally
    {
        delete Ini;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMain::RefreshLotListFromRegistry()
{
    TLotRunInfo *Lot;
    int LotCount;

    if(sgLotList==NULL)
        return;

    LotCount=LotRegistry.GetLotCount();
    SetupLotListGrid();
    if(LotCount<=0)
        return;

    sgLotList->RowCount=1+LotCount;
    for(int Index=0; Index<LotCount; Index++)
    {
        Lot=LotRegistry.GetLot(Index);
        if(Lot!=NULL)
            sgLotList->Cells[0][1+Index]=Lot->sLotID;
    }
    if(sgLotList->RowCount<2)
        sgLotList->RowCount=2;
    if(edLotNo!=NULL && LotRegistry.GetLot(0)!=NULL)
        edLotNo->Text=LotRegistry.GetLot(0)->sLotID;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::RestoreLastWorkOrder()
{
    bool bDuplicate;
    AnsiString DupCode;

    //A) restore production run-data counters (ref HT172 ReadLastDataIni).
    ReadLastDataIni();

    //B) auto-load today's newest delivered work order (2D->Bin JSON). When a
    //   fresh lot table exists, drive the Lot list display from the registry.
    bDuplicate=false;
    DupCode="";
    if(LotRegistry.LoadLatest(bDuplicate, DupCode))
    {
        RefreshLotListFromRegistry();
        return;
    }

    //C) no JSON work order : restore the last manually-used Lot list.
    LoadLastLotList();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btnAddLotClick(TObject *Sender)
{
    AnsiString LotText;
    int LotIndex;

    if(edLotNo==NULL)
        return;

    LotText=edLotNo->Text.Trim();
    if(LotText=="")
    {
        ShowMyMessage("Please Enter LotID !");
        return;
    }

    //AI(general) 20260610 : add through LotRegistry (single source of truth),
    // then reproject the grid. AddLot is idempotent (existing LotID re-activates),
    // matching the SECS / Simu paths. <0 means the 64-lot registry is full.
    LotIndex=LotRegistry.AddLot(LotText, HT160_LOT_SOURCE_OFFLINE, "", "");
    if(LotIndex<0)
    {
        ShowMyMessage("Lot list is full !");
        return;
    }
    RefreshLotListFromRegistry();
    if(sgLotList!=NULL && (LotIndex+1)<sgLotList->RowCount)
        sgLotList->Row=LotIndex+1;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btnEditLotClick(TObject *Sender)
{
    AnsiString OldLot;
    AnsiString NewLot;
    int SelectedRow;

    if(sgLotList==NULL || edLotNo==NULL)
        return;

    SelectedRow=sgLotList->Row;
    if(SelectedRow<1 || SelectedRow>=sgLotList->RowCount)
        return;

    OldLot=sgLotList->Cells[0][SelectedRow].Trim();
    NewLot=edLotNo->Text.Trim();
    if(NewLot=="")
    {
        ShowMyMessage("Please Enter LotID !");
        return;
    }
    if(OldLot=="" || OldLot==NewLot)
        return;

    //AI(general) 20260610 : rename in LotRegistry, then reproject the grid.
    LotRegistry.RenameLot(OldLot, NewLot);
    RefreshLotListFromRegistry();
    if(SelectedRow<sgLotList->RowCount)
        sgLotList->Row=SelectedRow;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btnRemoveLotClick(TObject *Sender)
{
    AnsiString LotText;
    int SelectedRow;

    if(sgLotList==NULL)
        return;

    SelectedRow=sgLotList->Row;
    if(SelectedRow<1 || SelectedRow>=sgLotList->RowCount)
        return;

    LotText=sgLotList->Cells[0][SelectedRow].Trim();
    if(LotText=="")
        return;

    //AI(general) 20260610 : remove from LotRegistry, then reproject the grid.
    LotRegistry.RemoveLot(LotText);
    RefreshLotListFromRegistry();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sgLotListClick(TObject *Sender)
{
    int SelectedRow;

    if(sgLotList==NULL || edLotNo==NULL)
        return;

    SelectedRow=sgLotList->Row;
    if(SelectedRow<1 || SelectedRow>=sgLotList->RowCount)
        return;

    if(sgLotList->Cells[0][SelectedRow].Trim()!="")
        edLotNo->Text=sgLotList->Cells[0][SelectedRow];
}
//---------------------------------------------------------------------------

