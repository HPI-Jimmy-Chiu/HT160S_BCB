//---------------------------------------------------------------------------

#include "IncludeAllHeader.h"
#pragma hdrstop

#include <shellapi.h>   //AI(general) 20260608 : ShellExecute for Explorer /select on snapshot zip
#include "main.h"
#include "database.h"
#include "cStateRecordHT160.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "GeneralSetting.h"   //AI(ht160s-agv) 20260615 : GeneralSetting.bUseAMR for the AMR status badge
#include "cprod.h"
#include "aAuto1To6.h"   //AI(ht160s-motion-view) 20260618 : AutoModule->GetWorkingTrayID for Unload Auto info
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
#include "LotWebApiClient.h"   //AI(ht160s-lot-webapi) 20260612 : Stage 4 : machine-flow Lot data pull
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
//AI(ht160s-secsgem) 20260616 : badge name captions (SECS/SAFE/AMR) now live in
//  main.dfm, so the former GetFeatureStatusName() helper was removed as dead code.
static AnsiString GetFeatureStatusDefaultValue(int BadgeIndex)
{
    switch(BadgeIndex)
    {
        case eMainFeatureSECS:     return "OFF";
        case eMainFeatureSafeDoor: return "NORMAL";
        case eMainFeatureAMR:      return "OFF";
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
        case eMainFeatureAMR:      return clGray;
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
    bFeatureBadgeOverflowWarned = false;   //AI(ht160s-mainui) 20260617 : badge-grid overflow warning not shown yet
    iLastSecsBadgeState = -1;   //AI(ht160s-secsgem) 20260612 : force the first periodic tick to paint the real HSMS state
    bLotApiPullActive = false;  //AI(ht160s-lot-webapi) 20260612 : Stage 4 : no pull in flight at startup
    sLotApiPullLot = "";
    bLotApiPullAll = false;     //AI(ht160s-lot-webapi) 20260612 : no "pull all lots" sweep at startup
    iLotApiPullCursor = 0;
    iLotApiRetryCount = 0;

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
        pgcMonitor->ActivePage = tsMotionView;
    }

    //AI 20260619 : hide the pgcLog tab row (Tray Status/Logs/Time Data/Map Tray) and
    //drive page switching from the four header buttons (spbTrayStatus/sbTimeData/
    //apbLogs/btnTrayMap OnClick -> pgcLog->ActivePage), matching HT172
    //SetInitialWindowFrame. Without this the redundant tab row showed and the four
    //buttons did nothing (their OnClick was never wired).
    if(pgcLog != NULL)
    {
        for(int PageIndex = 0; PageIndex < pgcLog->PageCount; PageIndex++)
            pgcLog->Pages[PageIndex]->TabVisible = false;
        pgcLog->ActivePage = tsTrayStatus;
    }

    if(sbMotionView != NULL)
        sbMotionView->Down = true;

    SetupLotListGrid();                                                         //AI(HT160S-Maintainer) 20260604 : init multi-lot manual list grid
    if(sgLotList != NULL)
        sgLotList->OnDblClick = sgLotListDblClick;                              //AI(ht160s-lot-webapi) 20260612 : double-click a Lot row -> 2D detail viewer
}
//---------------------------------------------------------------------------
//AI 20260619 : four header buttons that switch the pgcLog page (HT172 port). The
//pgcLog tab row is hidden (SetInitialWindowFrame), so these buttons are the only
//way to switch Tray Status / Time Data / Logs / Map Tray.
void __fastcall TfMain::spbTrayStatusClick(TObject *Sender)
{
    if(pgcLog != NULL)
        pgcLog->ActivePage = tsTrayStatus;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbTimeDataClick(TObject *Sender)
{
    if(pgcLog != NULL)
        pgcLog->ActivePage = tsTimeData;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::apbLogsClick(TObject *Sender)
{
    if(pgcLog != NULL)
        pgcLog->ActivePage = tsLogs;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btnTrayMapClick(TObject *Sender)
{
    if(pgcLog != NULL)
        pgcLog->ActivePage = tsMapTray;
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
//AI(ht160s-secsgem) 20260616 : the SECS/SAFE/AMR badges now live in main.dfm as
//  static layout (panel geometry, fonts, name caption, default value color). This
//  routine no longer creates VCL objects - it only binds the array slots to the
//  designer components, seeds the live value text/color, and applies the dynamic
//  gating (SECS visibility + click-to-open-log, AMR ON/OFF). The arrays are already
//  NULL-initialised in the constructor, and the unused Reserve2..6 slots stay NULL.
void __fastcall TfMain::BuildFeatureStatusBadges()
{
    if(pnlFeatureStatus == NULL)
        return;

    FeatureStatusPanels[eMainFeatureSECS]      = pnlFeatureBadge1;
    FeatureStatusNameLabels[eMainFeatureSECS]  = lblFeatureName1;
    FeatureStatusValueLabels[eMainFeatureSECS] = lblFeatureValue1;

    FeatureStatusPanels[eMainFeatureSafeDoor]      = pnlFeatureBadge2;
    FeatureStatusNameLabels[eMainFeatureSafeDoor]  = lblFeatureName2;
    FeatureStatusValueLabels[eMainFeatureSafeDoor] = lblFeatureValue2;

    FeatureStatusPanels[eMainFeatureAMR]      = pnlFeatureBadge3;
    FeatureStatusNameLabels[eMainFeatureAMR]  = lblFeatureName3;
    FeatureStatusValueLabels[eMainFeatureAMR] = lblFeatureValue3;

    //AI(ht160s-secsgem) 20260616 : seed the static-default value text/color for the
    //  two badges that carry one (SECS, SAFE); AMR is seeded from its live flag below.
    SetFeatureStatusBadge(eMainFeatureSECS, GetFeatureStatusDefaultValue(eMainFeatureSECS), GetFeatureStatusDefaultColor(eMainFeatureSECS));
    SetFeatureStatusBadge(eMainFeatureSafeDoor, GetFeatureStatusDefaultValue(eMainFeatureSafeDoor), GetFeatureStatusDefaultColor(eMainFeatureSafeDoor));

    //AI(ht160s-secsgem) 20260611 : make the SECS status badge open the GEM log.
    //  Only wire/show it when the SECS/GEM paid feature is enabled; otherwise hide
    //  the badge entirely (the engine is also not booted in that case).
    if(pnlFeatureBadge1 != NULL)
    {
        if(CosFunction.bUseSecsGem)
        {
            pnlFeatureBadge1->OnClick  = FeatureBadgeSecsClick;
            lblFeatureName1->OnClick   = FeatureBadgeSecsClick;
            lblFeatureValue1->OnClick  = FeatureBadgeSecsClick;
            pnlFeatureBadge1->Cursor   = crHandPoint;
            lblFeatureName1->Cursor    = crHandPoint;
            lblFeatureValue1->Cursor   = crHandPoint;
            pnlFeatureBadge1->ShowHint = true;
            pnlFeatureBadge1->Hint     = "Open SECS/GEM Log";
        }
        else
        {
            pnlFeatureBadge1->Visible = false;   // feature not purchased -> no badge
        }
    }

    //AI(ht160s-agv) 20260615 : seed the AMR badge from the live AMR mode flag so the
    //  operator can read AMR ON/OFF at a glance (GetFeatureStatusDefaultValue only
    //  carries a static "OFF" placeholder).
    UpdateAmrFeatureBadge();

    //AI(ht160s-mainui) 20260617 : arrange the badges into the COLS x ROWS grid now
    //  that every badge's final Visible state is known (SECS may be hidden above).
    LayoutFeatureBadges();
}
//---------------------------------------------------------------------------
//AI(ht160s-mainui) 20260617 : auto-arrange the visible status badges into a grid of
//  at most MAIN_FEATURE_BADGE_COLS columns x MAIN_FEATURE_BADGE_ROWS rows inside
//  pnlFeatureStatus. The home screen is cramped on the machine, so badges wrap to a
//  second row instead of running off the panel. A hidden badge (e.g. SECS when the
//  feature is off) does not consume a grid cell - the rest pack up to fill the gap.
//  Defensive reminder (req: "future when adding more, remind"): if the visible badge
//  count exceeds the grid capacity, the extras are hidden and a one-time warning
//  dialog tells the developer to bump COLS/ROWS (and widen the panel).
void __fastcall TfMain::LayoutFeatureBadges()
{
    const int GapX = 6;   // horizontal gap between badge cells
    const int GapY = 6;   // vertical gap between badge rows
    int  VisibleIndex;
    int  Col;
    int  Row;
    bool bOverflow;
    TPanel *Badge;

    if(pnlFeatureStatus == NULL)
        return;

    VisibleIndex = 0;
    bOverflow = false;

    for(int BadgeIndex = 0; BadgeIndex < MAIN_FEATURE_STATUS_COUNT; BadgeIndex++)
    {
        Badge = FeatureStatusPanels[BadgeIndex];
        if(Badge == NULL)
            continue;             // unused slot - no DFM panel bound
        if(!Badge->Visible)
            continue;             // hidden badge takes no grid cell

        if(VisibleIndex >= MAIN_FEATURE_BADGE_CAPACITY)
        {
            Badge->Visible = false;   // grid full - drop the extra and flag a reminder
            bOverflow = true;
            continue;
        }

        Col = VisibleIndex % MAIN_FEATURE_BADGE_COLS;
        Row = VisibleIndex / MAIN_FEATURE_BADGE_COLS;
        Badge->Left = Col * (Badge->Width  + GapX);
        Badge->Top  = Row * (Badge->Height + GapY);
        VisibleIndex++;
    }

    if(bOverflow && !bFeatureBadgeOverflowWarned)
    {
        bFeatureBadgeOverflowWarned = true;
        ShowMyOKMessageNoStop(
            "Main status badges exceed the grid (max "
            "3 cols x 2 rows = 6). The extra badges are hidden.\n"
            "Increase MAIN_FEATURE_BADGE_COLS / MAIN_FEATURE_BADGE_ROWS in "
            "main.h and widen pnlFeatureStatus to make room.");
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260615 : sync the main-screen AMR badge to GeneralSetting.bUseAMR.
//  Shown as a green "ON" when AMR/AGV mode is enabled, grey "OFF" otherwise. Safe to
//  call any time (e.g. after a maintenance change) - it is a cheap label repaint.
void __fastcall TfMain::UpdateAmrFeatureBadge()
{
    AnsiString ValueText = GeneralSetting.bUseAMR ? "ON" : "OFF";
    TColor     ValueColor = GeneralSetting.bUseAMR ? clGreen : clGray;
    SetFeatureStatusBadge(eMainFeatureAMR, ValueText, ValueColor);
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
    if(pgcMonitor != NULL && tsMotorView != NULL)
        pgcMonitor->ActivePage = tsMotorView;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbMotionViewClick(TObject *Sender)
{
    if(pgcMonitor != NULL && tsMotionView != NULL)
        pgcMonitor->ActivePage = tsMotionView;
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
                    pgcMonitor != NULL && pgcMonitor->ActivePage == tsMotionView);

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
//---------------------------------------------------------------------------
//AI(ht160s-motion-view) 20260618 : fill the Unload-area Auto1~6 info panels.
// Bin/Lot from the routing core : Normal mode = BinAreaMap static Bin<-Auto reverse;
// By Lot+Bin mode = LotBinBinding dynamic (Lot,Bin)<-Auto reverse. ID = 2D TrayID at
// the Auto working position. Cnt = per-Auto sorted IC count (tRunData.TrayICCnt).
void __fastcall TfMain::ShowUnloadAutoInfo()
{
    TPanel *BinPanel[6]={palAuto01Bin, palAuto02Bin, palAuto03Bin, palAuto04Bin, palAuto05Bin, palAuto06Bin};
    TPanel *IDPanel[6] ={palAuto01ID,  palAuto02ID,  palAuto03ID,  palAuto04ID,  palAuto05ID,  palAuto06ID};
    TPanel *CntPanel[6]={palAuto01Cnt, palAuto02Cnt, palAuto03Cnt, palAuto04Cnt, palAuto05Cnt, palAuto06Cnt};
    TPanel *LotPanel[6]={plLotNumberAuto1, plLotNumberAuto2, plLotNumberAuto3, plLotNumberAuto4, plLotNumberAuto5, plLotNumberAuto6};

    bool bLotBinMode=GeneralSetting.bUseLotBinSortMode;

    for(int i=0; i<6; i++)
    {
        AnsiString sBin="0";
        AnsiString sLot="";

        if(bLotBinMode)
        {
            int BindCount=LotBinBinding.GetBindingCount();
            for(int j=0; j<BindCount; j++)
            {
                AnsiString BindLotID;
                int BindBin=0, BindAuto=-1;
                if(LotBinBinding.GetBindingByIndex(j, BindLotID, BindBin, BindAuto) && BindAuto==i)
                {
                    sBin=IntToStr(BindBin);
                    sLot=BindLotID;
                    break;
                }
            }
        }
        else
        {
            int Bin=BinAreaMap.GetBinByArea(eHT160BinAreaAuto1+i);
            if(Bin>0)
                sBin=IntToStr(Bin);
        }

        AnsiString sID="";
        if(AutoModule!=NULL)
            sID=AutoModule->GetWorkingTrayID(i);

        AnsiString sCnt=IntToStr(tRunData.TrayICCnt[eAuto1+i]);

        if(BinPanel[i]!=NULL && BinPanel[i]->Caption!=sBin) BinPanel[i]->Caption=sBin;
        if(LotPanel[i]!=NULL && LotPanel[i]->Caption!=sLot) LotPanel[i]->Caption=sLot;
        if(IDPanel[i] !=NULL && IDPanel[i]->Caption !=sID ) IDPanel[i]->Caption =sID;
        if(CntPanel[i]!=NULL && CntPanel[i]->Caption!=sCnt) CntPanel[i]->Caption=sCnt;
    }
}
//---------------------------------------------------------------------------
// Motor View page (tsMotorView) live status grid. Ported from HT172 0420
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
    if(pgcMonitor->ActivePage!=tsMotorView)                                       // only update when Motor View is shown //
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
            sgMotorStatus->Cells[0][i+1]=HSys.MotPtr[i]->NumberAlias;
        }
    }

    if(pgcMain==NULL || pgcMain->ActivePage!=tsMonitorView)
        return;

    for(int i=0; i<HSys.iTotalMotor; i++)
    {
//        if(HSys.MotPtr[i]==NULL || HSys.MotPtr[i]->GetEnable()==false)
//            continue;

        if(HSys.Sys.SystemStart==false)
            HSys.MotPtr[i]->ReadPos();
        sgMotorStatus->Cells[1][i+1]=HSys.MotPtr[i]->TargetPosition;
        sgMotorStatus->Cells[2][i+1]=HSys.MotPtr[i]->Position;
        sgMotorStatus->Cells[3][i+1]=HSys.MotPtr[i]->EncoderPosition;
        HSys.MotPtr[i]->ScanMotorStatus();
        for(int j=0; j<iMotLedTotalCnt; j++)
        {
            R=sgMotorStatus->CellRect(4+j, i+1);
            R.Top++;
            R.Bottom-=1;
            R.Left++;
            R.Right-=2;

            if(HSys.MotPtr[i]->Led[j])
            {
                if(j==iHomeLed)
                {
                    // AI 20260622 : green only after a REAL full home completed for this axis
                    // (fAllMotorHome && bHomeFinish), not merely because it sits on its home
                    // sensor -- an axis parked on the sensor at power-up used to read as homed
                    // before any home ran. Diverges from HT172 on-sensor=green by design.
                    sgMotorStatus->Canvas->Brush->Color=
                        (fAllMotorHome && HSys.MotPtr[i]->bHomeFinish)?clGreen:clRed;
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
            HSys.Mot.MSuckZ_1->GetSoftLimitN(), HSys.Mot.MSuckZ_1->GetSoftLimitP(), 14, 7);
    if(HSys.Mot.MSuckZ_2 != NULL && ledSortArm1ZB != NULL)
        HSys.Mot.MSuckZ_2->SetSimulateCompoment(ledSortArm1ZB, akLeft,
            HSys.Mot.MSuckZ_2->GetSoftLimitN(), HSys.Mot.MSuckZ_2->GetSoftLimitP(), 14, 7);
    if(HSys.Mot.MSuckZ_3 != NULL && ledSortArm1ZE != NULL)
        HSys.Mot.MSuckZ_3->SetSimulateCompoment(ledSortArm1ZE, akLeft,
            HSys.Mot.MSuckZ_3->GetSoftLimitN(), HSys.Mot.MSuckZ_3->GetSoftLimitP(), 14, 7);
    if(HSys.Mot.MSuckZ_4 != NULL && ledSortArm1ZF != NULL)
        HSys.Mot.MSuckZ_4->SetSimulateCompoment(ledSortArm1ZF, akLeft,
            HSys.Mot.MSuckZ_4->GetSoftLimitN(), HSys.Mot.MSuckZ_4->GetSoftLimitP(), 14, 7);

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
//AI(HT160S-Maintainer) 20260622 : status icons now load on demand from
// picture\status\*.bmp. One SpeedButton per group (sbRealIcon / sbStartIcon)
// whose Glyph is swapped per state, replacing the old stacked always-resident
// SpeedButtons (sbDummyIcon / sbContinue) that had to be moved together.
static AnsiString GetStatusIconDir()
{
    return HSys.CurrentDir + AnsiString("\\picture\\status\\");
}
//---------------------------------------------------------------------------
static void SetIconGlyph(TSpeedButton *Btn, AnsiString FileName)
{
    if(Btn == NULL)
        return;
    if(!FileExists(FileName))
        return;                 // file missing: keep current glyph, never crash
    try
    {
        Btn->Glyph->LoadFromFile(FileName);
    }
    catch(...)
    {
    }
}
//---------------------------------------------------------------------------
void TfMain::LoadRunModePicture()
{
    NormalizeMainRunSettings();

    if(sbRealIcon != NULL)
    {
        if(HSys.LastSet.iRealDummy == REALLY)
            SetIconGlyph(sbRealIcon, GetStatusIconDir() + AnsiString("real.bmp"));
        else
            SetIconGlyph(sbRealIcon, GetStatusIconDir() + AnsiString("dummy.bmp"));
    }

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

    if(sbStartIcon != NULL)
    {
        if(HSys.LastSet.iStartMode == 0)
            SetIconGlyph(sbStartIcon, GetStatusIconDir() + AnsiString("initial.bmp"));
        else
            SetIconGlyph(sbStartIcon, GetStatusIconDir() + AnsiString("continue.bmp"));
    }

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
    //AI(poka-yoke) 20260616 : was a silent no-op when mode/data was wrong. One
    //  Cycle runs at idle, so ShowMyMessage here is safe (does not stop a running
    //  machine). Tell the operator why, and require the same lot/2D data as Start.
    if(HSys.Sys.RunMode!=Run_Normal && HSys.Sys.RunMode!=Run_CleanOut)
    {
        ShowMyMessage("One Cycle is only allowed in Normal / Clean Out mode.");
        return;
    }
    AnsiString Reason;
    if(CheckLotDataReady(Reason)==false)
    {
        ShowMyMessage(Reason);
        return;
    }
    RecordProcess("ONE CYCLE pressed");
    EventReport(SECS_EVENT.PressOneCycle);
    ChangeRunMode(Run_OneCycle);
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
//AI(poka-yoke) 20260616 : shared start-precondition guard. Returns true when
//  lot/2D data is ready; otherwise fills Reason and the caller warns + aborts.
//  Centralizes what Start() checked inline so OneCycle uses the SAME rules. Only
//  ever called at idle (SystemStart==false), so the caller's ShowMyMessage does
//  not interrupt a running machine.
bool TfMain::CheckLotDataReady(AnsiString &Reason)
{
    if(edLotNo->Text=="")                                                       //Steven 20240625 : block start without lot ID
    {
        Reason="Please Enter LotID !";
        return false;
    }
    //AI(ht160s-lot-webapi) 20260612 : Start safety gate. Refuse to start the
    // machine unless (a) at least one Lot is registered AND (b) at least one
    // 2D/Bin record is loaded. A lot name alone (e.g. SECS SET_LOT_INFO that
    // only registers names) is NOT enough to sort by, so block and warn.
    if(LotRegistry.GetLotCount()<=0)
    {
        Reason="No Lot data : add at least one Lot before Start !";
        return false;
    }
    if(LotRegistry.GetItemCount()<=0)
    {
        Reason="No 2D data : load lot 2D/Bin data before Start !";
        return false;
    }
    //AI(poka-yoke) 20260616 : By Lot+Bin mode needs at least one (Lot,Bin)->Auto
    // binding, otherwise nothing can be routed. Block start until bindings exist.
    if(GeneralSetting.bUseLotBinSortMode && LotBinBinding.GetBindingCount()<=0)
    {
        Reason="By Lot+Bin mode is ON but no binding is set. Set bindings first !";
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void TfMain::Start()
{
    if(HSys.Sys.SystemStart==false)
    {
        AnsiString Reason;
        if(CheckLotDataReady(Reason)==false)
        {
            ShowMyMessage(Reason);
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
            // AI 20260622 : START on an un-homed machine must HOME first WITH the monitor
            // shown. The home engine already runs from the kernel (ProcessMotion sees
            // fAllMotorHome==false) but nothing called fHome->Show(), so it homed silently
            // and the run mode never switched to Home. Mirror the HOME button (sbHome1Click):
            // re-arm every axis, enter Run_Home, show the monitor non-modally -- matching
            // HT172/HT9045 where the home sequence owns the screen. Keep bHomeByStart=true so
            // the kernel auto-continues into production after home (vs the HOME button stop).
            bHomeByStart=true;
            ChangeRunMode(Run_Home);
            ArmMotorHome();
            fHome->lstHomeMsg->Clear();
            fHome->lstHomeMsg->Items->Insert(0, "Starting home procedure....");
            fHome->Show();
        }
//        flagOneCycleTrayEnd=false;
    }
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260617 : physical operator-panel key dispatch. Ported
//from HT172 ScanPannelKey()+TfMain::ScanKey() in procedural form (HT160 sensors
//carry no Tag, so no Tag table). The Pad hooks added 20260616 fill the scan
//buffer (uPadInterface::DoScanPanelLed), but nothing consumed it, so the physical
//Start/Home/Pause/One Cycle/Clean Out keys did nothing. Front and Rear keys share
//one action. Each key is rising-edge latched so a held button fires once. The
//existing screen handlers already log + EventReport + show prompts (e.g. Home's
//"Confirm home?"), so we just call them - pressing physical Home now prompts too.
void TfMain::ScanPanelKeys()
{
    static bool bWasStart=false;
    static bool bWasHome=false;
    static bool bWasPause=false;
    static bool bWasOneCycle=false;
    static bool bWasCleanOut=false;

    //suspend while a teach/IO-test/home/dialog window owns the panel (HT172 ScanKey
    //guard set), and while the machine is safety-interlocked.
    if(fNote!=NULL && fNote->fShow)
        return;
    if(fiosetview!=NULL && fiosetview->fShow)
        return;
    if(fHome!=NULL && fHome->Visible)
        return;
    if(MyMessageBox!=NULL && MyMessageBox->fShow)
        return;
    if(IsSafeLock())
        return;

    bool bStart    = HSys.Sen.SnFKStart.IsOn()    || HSys.Sen.SnRKStart.IsOn();
    bool bHome     = HSys.Sen.SnFKHome.IsOn()     || HSys.Sen.SnRKHome.IsOn();
    bool bPause    = HSys.Sen.SnFKPause.IsOn()    || HSys.Sen.SnRKPause.IsOn();
    bool bOneCycle = HSys.Sen.SnFKOneCycle.IsOn() || HSys.Sen.SnRKOneCycle.IsOn();
    bool bCleanOut = HSys.Sen.SnFKCleanOut.IsOn() || HSys.Sen.SnRKCleanOut.IsOn();

    //Pause first (operator stop has priority); each handler self-guards on state.
    if(bPause && bWasPause==false)
        sbPause1Click(this);
    else if(bStart && bWasStart==false)
        Start();
    else if(bHome && bWasHome==false)
        sbHome1Click(this);
    else if(bOneCycle && bWasOneCycle==false)
        sbOneCycle1Click(this);
    else if(bCleanOut && bWasCleanOut==false)
    {
        //AI(HT160S-Maintainer) 20260619 : HT172 ScanKey wraps the physical
        //CLEAN OUT key in a YES/NO confirm before acting. The touchscreen
        //handler stays prompt-free; only the panel key asks. SOFT_SIMULATE
        //skips the prompt (matches HT172).
        #ifndef SOFT_SIMULATE
        if(ShowMyMessageBox_YES_NO("Confirm Clean Out?")==TMyMessageBox::msgrtnYES)
        #endif
            sbCleanOut1Click(this);
    }

    bWasStart=bStart;
    bWasHome=bHome;
    bWasPause=bPause;
    bWasOneCycle=bOneCycle;
    bWasCleanOut=bCleanOut;
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

    //AI(ht160s-lotbin) 20260615 : By Lot+Bin mode. A fresh Lot Start clears all
    //dynamic (Lot,Bin)->Auto bindings so Autos are re-bound first-come-first-served
    //for this work order. (Mid-lot RESTART resumes via the machine START button,
    //which does NOT clear - bindings are restored in RestoreLastWorkOrder. The
    //future "inherit last record?" prompt will gate this clear for that path.)
    LotBinBinding.Clear();
    LotBinBinding.SaveToIni();

    MachineRun.bRunning=true;
    MachineRun.iActiveLotCount=LotRegistry.GetLotCount();

    edLotNo->Text=FirstLot;
    //AI(ht160s-lot-webapi) 20260612 : Stage 4 : at lot start, pull EVERY lot's
    // 2D/Bin data from the customer WebAPI (async, no modal). Shared helper so the
    // SECS S2F42 LOTSTART handler pulls every lot too (not just the first).
    StartLotWebApiPullAll();
    //AI(HT160S-Maintainer) 20260608 : need1 : persist the started work order so
    //the next power-on can restore it (see RestoreLastWorkOrder / FormShow).
    SaveLastLotList();
    RecordProcess("LOT START pressed");
}
//---------------------------------------------------------------------------
//AI(ht160s-lot-webapi) 20260612 : total attempts allowed per lot during a pull-all
// sweep (1 initial try + (LOT_API_MAX_RETRY-1) retries). Guards against transient
// network blips / a momentarily slow customer WebAPI server dropping a lot.
static const int LOT_API_MAX_RETRY = 3;
//---------------------------------------------------------------------------
//AI(ht160s-lot-webapi) 20260612 : advance the "pull all lots" sweep. Walk the raw
// registry slots from iLotApiPullCursor, skipping freed (blank) slots, and kick off
// a pull for the lot at the cursor. The cursor only moves past a real lot once that
// lot succeeds or its retries are used up (see PollLotDataWebApi), so a slow/failed
// lot is retried instead of being silently dropped, and the sweep never stalls.
void __fastcall TfMain::StartNextLotApiPull()
{
    TLotRunInfo *Lot;
    int SlotCount;

    if(bLotApiPullAll==false)
        return;

    SlotCount=LotRegistry.GetLotSlotCount();
    while(iLotApiPullCursor<SlotCount)
    {
        Lot=LotRegistry.GetLot(iLotApiPullCursor);
        if(Lot==NULL || Lot->sLotID.Trim()=="")
        {
            iLotApiPullCursor++;                   // skip freed/blank slots
            continue;
        }
        //real lot at the cursor : pull it. Do NOT advance the cursor here -
        //PollLotDataWebApi advances it on success, or re-calls us (same cursor)
        //to retry on failure, or advances it once the retries are used up.
        RequestLotDataFromWebApi(Lot->sLotID);
        return;                                    // one in flight ; Poll will call us again
    }

    //no more lots to pull : end the sweep
    bLotApiPullAll=false;
    iLotApiRetryCount=0;
    RecordProcess("Lot WebAPI pull-all sweep complete");
}
//---------------------------------------------------------------------------
//AI(ht160s-lot-webapi) 20260612 : arm a "pull all lots" sweep over the WHOLE
// registry, one lot at a time (the WebAPI client is single-request). Shared by the
// manual LotStart button and the SECS S2F42 LOTSTART handler so both fetch every
// lot's 2D/Bin data, not just the first lot. Non-blocking, no modal (the SECS path
// runs on the HSMS/VCL receive thread). Gated by the Lot WebAPI "UsePull" toggle.
void __fastcall TfMain::StartLotWebApiPullAll()
{
    EnsureLotWebApiClientCreated();
    if(LotWebApiClient!=NULL && LotWebApiClient->GetUsePull())
    {
        bLotApiPullAll=true;
        iLotApiPullCursor=0;
        iLotApiRetryCount=0;
        StartNextLotApiPull();
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-lot-webapi) 20260612 : Stage 4 : start a non-blocking Lot WebAPI pull.
// Used by both the manual LotStart path and the SECS S2F42 LOTSTART handler, so it
// must NEVER show a modal dialog (the SECS path runs on the HSMS/VCL receive thread
// and a popup would stall factory communication). On any problem we just log.
void __fastcall TfMain::RequestLotDataFromWebApi(AnsiString LotID)
{
    AnsiString Lot;

    Lot=LotID.Trim();
    if(Lot=="")
        return;

    EnsureLotWebApiClientCreated();
    if(LotWebApiClient==NULL)
        return;

    if(LotWebApiClient->IsBusy())
    {
        //a pull is already running : do not stack requests (non-blocking, no modal)
        RecordProcess("Lot WebAPI pull skipped (client busy): "+Lot);
        return;
    }

    if(LotWebApiClient->StartLotRequest(Lot))
    {
        bLotApiPullActive=true;
        sLotApiPullLot=Lot;
        RecordProcess("Lot WebAPI pull started: "+Lot);
    }
    else
    {
        //start failed (bad URL etc) : log only, never block the caller
        RecordProcess("Lot WebAPI pull start failed: "+LotWebApiClient->GetLastError());
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-lot-webapi) 20260612 : Stage 4 : drive the in-flight pull. Called every
// MainProc cycle (VCL main thread via Synchronize). Cheap no-op when idle. When the
// response arrives it is parsed straight into LotRegistry and the on-screen Lot list
// is reprojected. NO modal dialog here either.
void __fastcall TfMain::PollLotDataWebApi()
{
    AnsiString Body;
    bool bOk;
    int HttpStatus;
    bool bDuplicate;
    AnsiString DupCode;

    if(bLotApiPullActive==false)
        return;
    if(LotWebApiClient==NULL)
    {
        bLotApiPullActive=false;
        return;
    }

    LotWebApiClient->Poll();
    if(!LotWebApiClient->GetResult(Body, bOk, HttpStatus))
        return;     // still running

    bLotApiPullActive=false;        // one-shot consume
    bool bAttemptOk=false;          //AI(ht160s-lot-webapi) 20260612 : did THIS attempt succeed?
    if(bOk==true && Body.Trim()!="")
    {
        bDuplicate=false;
        DupCode="";
        if(LotRegistry.LoadFromJsonString(Body, bDuplicate, DupCode))
        {
            RefreshLotListFromRegistry();
            RecordProcess("Lot WebAPI data loaded: "+sLotApiPullLot);
            if(bDuplicate==true)
                RecordProcess("Lot WebAPI duplicate 2D ignored: "+DupCode);
            bAttemptOk=true;
        }
        else
        {
            RecordProcess("Lot WebAPI JSON parse failed: "+sLotApiPullLot);
        }
    }
    else
    {
        RecordProcess("Lot WebAPI pull failed (HTTP "+IntToStr(HttpStatus)+"): "+sLotApiPullLot);
    }
    sLotApiPullLot="";

    //AI(ht160s-lot-webapi) 20260612 : drive the "pull all lots" sweep. On success
    // advance to the next lot. On failure retry the SAME lot up to LOT_API_MAX_RETRY
    // times (network blips / slow server) before giving up and moving on, so one
    // flaky lot never stalls the sweep and never silently drops a lot on the first
    // hiccup.
    if(bLotApiPullAll==true)
    {
        if(bAttemptOk==true)
        {
            iLotApiPullCursor++;        // this lot done : move to the next slot
            iLotApiRetryCount=0;
            StartNextLotApiPull();
        }
        else
        {
            iLotApiRetryCount++;
            if(iLotApiRetryCount<LOT_API_MAX_RETRY)
            {
                RecordProcess("Lot WebAPI retry "+IntToStr(iLotApiRetryCount)+"/"+
                              IntToStr(LOT_API_MAX_RETRY-1)+" for slot "+IntToStr(iLotApiPullCursor));
                StartNextLotApiPull();  // same cursor : re-pull this lot
            }
            else
            {
                RecordProcess("Lot WebAPI giving up slot "+IntToStr(iLotApiPullCursor)+
                              " after "+IntToStr(LOT_API_MAX_RETRY-1)+" retries");
                iLotApiPullCursor++;    // skip this lot and continue the sweep
                iLotApiRetryCount=0;
                StartNextLotApiPull();
            }
        }
    }
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

    //AI(ht160s-lot-webapi) 20260612 : stop any in-flight "pull all lots" sweep so
    // it does not walk the registry we are about to clear.
    bLotApiPullAll=false;
    iLotApiPullCursor=0;
    iLotApiRetryCount=0;

    //AI(ht160s-lot-webapi) 20260612 : Lot End clears the whole work order so the
    // next lot starts clean. Clear() drops every Lot slot, the 2D-code index and
    // all per-IC 2D/Bin records. Then blank the active Lot No, repaint the grid
    // (RefreshLotListFromRegistry blanks every row when the registry is empty),
    // and overwrite system\LastLotList.ini with the now-empty list so a restart
    // does NOT restore the finished lots.
    LotRegistry.Clear();
    //AI(ht160s-lotbin) 20260615 : drop all (Lot,Bin)->Auto bindings on Lot End so the
    //next work order starts with a clean dynamic table (also persisted empty).
    LotBinBinding.Clear();
    LotBinBinding.SaveToIni();
    if(edLotNo!=NULL)
        edLotNo->Text="";
    RefreshLotListFromRegistry();
    SaveLastLotList();
    RecordProcess("Lot data cleared (Lot End)");
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260604 : Lot Manual Edit list helpers (multi-lot queue, UI layer)
void __fastcall TfMain::SetupLotListGrid()
{
    if(sgLotList==NULL)
        return;

    //AI(ht160s-lot-webapi) 20260612 : multi-column so each Lot's loaded data is
    // confirmable at a glance : LotID | source | 2D-code count | sorted count.
    // Column 0 stays the LotID (every other helper still reads Cells[0][row]).
    sgLotList->ColCount=4;
    sgLotList->FixedCols=0;
    sgLotList->FixedRows=1;
    if(sgLotList->RowCount<2)
        sgLotList->RowCount=2;
    sgLotList->ColWidths[0]=300;
    sgLotList->ColWidths[1]=70;
    sgLotList->ColWidths[2]=70;
    sgLotList->ColWidths[3]=80;
    sgLotList->Cells[0][0]="Lot No.";
    sgLotList->Cells[1][0]="Src";
    sgLotList->Cells[2][0]="2D";
    sgLotList->Cells[3][0]="Sorted";

    for(int RowIndex=1; RowIndex<sgLotList->RowCount; RowIndex++)
    {
        sgLotList->Cells[0][RowIndex]="";
        sgLotList->Cells[1][RowIndex]="";
        sgLotList->Cells[2][RowIndex]="";
        sgLotList->Cells[3][RowIndex]="";
    }
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

    //AI(ht160s-lot-webapi) 20260612 : RemoveLot leaves a freed (blank) slot in
    // place to keep packed 2D-ref indices valid, so we must walk the RAW slot
    // span and skip blanks. Emitting only non-blank lots into consecutive rows
    // shifts the list up after a middle delete (no gap, no dropped last lot).
    sgLotList->RowCount=1+LotCount;
    int OutRow=0;
    AnsiString FirstLotID="";
    int SlotCount=LotRegistry.GetLotSlotCount();
    for(int Index=0; Index<SlotCount; Index++)
    {
        Lot=LotRegistry.GetLot(Index);
        if(Lot==NULL || Lot->sLotID.Trim()==AnsiString(""))
            continue;
        //AI(ht160s-lot-webapi) 20260612 : col0=LotID, col1=source, col2=2D-code
        // count (iPlanQty grows as the work-order JSON loads), col3=sorted qty.
        sgLotList->Cells[0][1+OutRow]=Lot->sLotID;
        sgLotList->Cells[1][1+OutRow]=(Lot->iSource==HT160_LOT_SOURCE_SECS)?AnsiString("SECS"):AnsiString("OFF");
        sgLotList->Cells[2][1+OutRow]=IntToStr(Lot->iPlanQty);
        sgLotList->Cells[3][1+OutRow]=IntToStr(Lot->iSortedQty);
        if(FirstLotID==AnsiString(""))
            FirstLotID=Lot->sLotID;
        OutRow++;
    }
    if(sgLotList->RowCount<2)
        sgLotList->RowCount=2;
    if(edLotNo!=NULL && FirstLotID!=AnsiString(""))
        edLotNo->Text=FirstLotID;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::RestoreLastWorkOrder()
{
    bool bDuplicate;
    AnsiString DupCode;
    bool bLoaded;
    int LotCnt;
    int BindCnt;
    AnsiString Msg;

    //A) restore production run-data counters (ref HT172 ReadLastDataIni). These are
    //   cumulative production stats and are KEPT even on a fresh start (user choice).
    ReadLastDataIni();

    //B) auto-load today's newest delivered work order (2D->Bin JSON). When a
    //   fresh lot table exists, drive the Lot list display from the registry.
    bDuplicate=false;
    DupCode="";
    bLoaded=false;
    if(LotRegistry.LoadLatest(bDuplicate, DupCode))
    {
        RefreshLotListFromRegistry();
        //AI(ht160s-lotbin) 20260615 : restore the dynamic (Lot,Bin)->Auto table after
        //the registry is populated, so a mid-lot restart keeps each Auto's binding
        //(table is keyed by LotID, stable across restart).
        LotBinBinding.LoadFromIni();
        bLoaded=true;
    }
    else
    {
        //C) no JSON work order : restore the last manually-used Lot list.
        LoadLastLotList();
        LotBinBinding.LoadFromIni();   //AI(ht160s-lotbin) 20260615 : restore dynamic bindings (see above)
        bLoaded=(LotRegistry.GetLotCount()>0 || LotBinBinding.GetBindingCount()>0);
    }

    //AI(ht160s-lotbin) 20260615 : "inherit last record?" gate. When a previous work
    //order was restored above, ask the operator whether to resume it or start fresh.
    //NO clears the WHOLE work order (registry + LastLotList.ini + (Lot,Bin)->Auto
    //bindings, persisted empty), matching Lot End semantics; the cumulative production
    //counters from ReadLastDataIni are kept regardless. This is the gate that decides
    //resume-keeps-bindings vs fresh-start-clears-them (mid-lot restart -> Yes resumes).
    if(bLoaded)
    {
        LotCnt=LotRegistry.GetLotCount();
        BindCnt=LotBinBinding.GetBindingCount();
        Msg="Inherit last work order ? ("+IntToStr(LotCnt)+" lots, "+IntToStr(BindCnt)+
            " bindings)   Yes = resume,  No = start fresh";
        if(ShowMyMessageBox_YES_NO(Msg)!=TMyMessageBox::msgrtnYES)
        {
            LotRegistry.Clear();
            LotBinBinding.Clear();
            LotBinBinding.SaveToIni();
            if(edLotNo!=NULL)
                edLotNo->Text="";
            RefreshLotListFromRegistry();
            SaveLastLotList();
            RecordProcess("Startup: operator chose fresh start, last work order cleared");
        }
        else
        {
            RecordProcess("Startup: operator chose to inherit last work order");
        }
    }
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
//AI(ht160s-lot-webapi) 20260612 : double-click a Lot row to view its 2D / Bin
// data. This is the operator's confirmation that the work-order JSON (WebAPI /
// file / SECS) actually downloaded : an empty list means no 2D data is loaded
// for that Lot yet. User-initiated, so a modal popup is fine here.
void __fastcall TfMain::sgLotListDblClick(TObject *Sender)
{
    int SelectedRow;
    AnsiString LotText;

    (void)Sender;
    if(sgLotList==NULL)
        return;
    SelectedRow=sgLotList->Row;
    if(SelectedRow<1 || SelectedRow>=sgLotList->RowCount)
        return;
    LotText=sgLotList->Cells[0][SelectedRow].Trim();
    if(LotText=="")
        return;
    ShowLotDetail(LotText);
}
//---------------------------------------------------------------------------
//AI(ht160s-lot-webapi) 20260612 : pop a read-only grid of every 2D code mapped
// to LotID (Code2D / Bin / HBin / SBin / RetestCode / DiePass). Empty -> tell the
// operator no work-order JSON has been loaded for this Lot.
void __fastcall TfMain::ShowLotDetail(AnsiString LotID)
{
    TStringList *Lines;
    TForm *Dlg;
    TStringGrid *Grid;
    int Count;
    int i;
    int c;

    Lines=new TStringList;
    try
    {
        Count=LotRegistry.GetLotIcList(LotID, Lines);
        if(Count<=0)
        {
            ShowMyMessage("Lot \""+LotID+"\" has no 2D data loaded yet.\n"
                          "(Work-order JSON not downloaded for this Lot.)");
            return;
        }

        Dlg=new TForm((TComponent*)this);
        try
        {
            Dlg->Caption="Lot 2D Detail : "+LotID+"   ("+IntToStr(Count)+" codes)";
            Dlg->Width=660;
            Dlg->Height=480;
            Dlg->Position=poScreenCenter;

            Grid=new TStringGrid(Dlg);
            Grid->Parent=Dlg;
            Grid->Align=alClient;
            Grid->ColCount=6;
            Grid->FixedCols=0;
            Grid->FixedRows=1;
            Grid->RowCount=1+Count;
            Grid->ColWidths[0]=240;
            Grid->ColWidths[1]=50;
            Grid->ColWidths[2]=55;
            Grid->ColWidths[3]=55;
            Grid->ColWidths[4]=120;
            Grid->ColWidths[5]=100;
            Grid->Cells[0][0]="2D Code";
            Grid->Cells[1][0]="Bin";
            Grid->Cells[2][0]="HBin";
            Grid->Cells[3][0]="SBin";
            Grid->Cells[4][0]="RetestCode";
            Grid->Cells[5][0]="DiePass";

            for(i=0; i<Count; i++)
            {
                AnsiString Rest=Lines->Strings[i];
                for(c=0; c<6; c++)
                {
                    AnsiString Field;
                    int Tab=Rest.Pos("\t");
                    if(Tab>0)
                    {
                        Field=Rest.SubString(1, Tab-1);
                        Rest=Rest.SubString(Tab+1, Rest.Length());
                    }
                    else
                    {
                        Field=Rest;
                        Rest="";
                    }
                    Grid->Cells[c][1+i]=Field;
                }
            }
            Dlg->ShowModal();
        }
        __finally
        {
            delete Dlg;
        }
    }
    __finally
    {
        delete Lines;
    }
}
//---------------------------------------------------------------------------

