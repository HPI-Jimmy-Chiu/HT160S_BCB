//---------------------------------------------------------------------------

#include "IncludeAllHeader.h"
#pragma hdrstop

#include <shellapi.h>   //AI(general) 20260608 : ShellExecute for Explorer /select on snapshot zip
#include <Clipbrd.hpp>   //AI(ht160s-2dbin-manual) 20260628 : clipboard paste of 2D/Bin rows
#include "main.h"
#include "database.h"
#include "cStateRecordHT160.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "GeneralSetting.h"   //AI(ht160s-agv) 20260615 : GeneralSetting.bUseAMR for the AMR status badge
#include "cCsvDailyLog.h"   //AI(ht160s-workorder-backup) 20260630 : PruneFolderTree for LotStory Discarded
#include "cprod.h"
#include "aAuto1To6.h"   //AI(ht160s-motion-view) 20260618 : AutoModule->GetWorkingTrayID for Unload Auto info
#include "UserRoleManager.h"
#include "uQwertyKey.h"   //AI(ht160s-password) 20260624 : on-screen keypad for login ID/password
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
#include "cSoterOutput.h"
#include "aLoader.h"
#include "aEmpty.h"    //AI(ht160s-agv) 20260624 : EmptyModule->GetCarTrayCount for PanelMain6 header
#include "aColor.h"    //AI(ht160s-agv) 20260624 : ColorModule->GetCarTrayCount for PanelMain6 header
#include "aSortArm.h"  //AI(ht160s-status) 20260703 : SortArmModule->GetStatus for the Module Status sheet
#include "aTrayArm.h"  //AI(ht160s-status) 20260703 : TrayArmModule->GetStatus for the Module Status sheet
#include "LotWebApiClient.h"   //AI(ht160s-lot-webapi) 20260612 : Stage 4 : machine-flow Lot data pull
#include "uFtpUploadThread.h"  //AI(ht160s-ftp) 20260721 : background FTP upload worker (lifecycle + result pump)
#include "cEventLog.h"         //AI(ht160s-ftp) 20260721 : g_EventLog.Log for FTP upload audit records
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
        //AI(ht160s-language) 20260626 : apply current UI language before showing.
        if(fLan != NULL)
            fLan->ChangeLanguage(FormPtr);
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
    iLastSortModeBadge = -1;    //AI(ht160s-whitelist-override) 20260717 : force the first sort-mode badge paint
    bLotApiPullActive = false;  //AI(ht160s-lot-webapi) 20260612 : Stage 4 : no pull in flight at startup
    sLotApiPullLot = "";
    bLotApiPullAll = false;     //AI(ht160s-lot-webapi) 20260612 : no "pull all lots" sweep at startup
    iLotApiPullCursor = 0;
    iLotApiRetryCount = 0;

    if(ComponentState.Contains(csDesigning))
        return;

    //AI(HT160S-Maintainer) 20260622 : re-front a ShowModal() sub-screen after a desktop
    //task-switch so it can't get stuck behind the (modally-disabled) main form. See AppActivate.
    Application->OnActivate = AppActivate;

    //AI(HT160S-Maintainer) 20260603 : create central alarm object before any module can raise an alarm
    if(Alarm==NULL)
        Alarm = new HAlarm(this);

    BuildFeatureStatusBadges();

    //AI(ht160s-statusbar) 20260624 : port of HT172 main.cpp:31-41 version-string set.
    //HT160 has no VerInfo()/GetSVNRev() and no __CODEGUARD__, so drop the SVN-rev call
    //and the .CG suffix; keep the optional .QC suffix (CUSTOMER_CODE==CC_HONPREC_QC).
    //MainVersion is the single version constant (cmydef). emsSim panel is filled ONLY
    //on a SOFT_SIMULATE build (compile-time, separate from the runtime Real/Dummy icon).
    if(stbMain!=NULL)
    {
        AnsiString VerText=MainVersion;
        if(CUSTOMER_CODE==CC_HONPREC_QC)
            VerText=MainVersion+".QC";
        stbMain->Panels->Items[emsVersion]->Text=VerText;
        #ifdef SOFT_SIMULATE
        stbMain->Panels->Items[emsSim]->Text="SIMULATE";
        #endif
    }

    LoadMainRunSettingsFromIni();
    LoadRunModePicture();
    LoadStartModePicture();
    ReadPassword();   //AI(ht160s-password) 20260624 : load system\login.txt user book (seed default if missing)
    UserRoleManager.InitializeByBuildMode();
    //AI(secs-operatorid) 20260803 : SVID/ECID 1007. GeneralSetting is already loaded here
    //(HSys.Initial -> InitialCosFunction -> GeneralSetting.Load, well before CreateForm), so
    //the field shows the persisted value from the first paint instead of the DFM placeholder.
    LoadOperatorIDToDisplay();

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
    Setup2DBinGrid();                                                           //AI(ht160s-2dbin-manual) 20260628 : init editable 2D/Bin grid
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
    RefreshEventLogView();   //AI(ht160s-eventlog-view) 20260708 : show today's EventLog tail
}
//---------------------------------------------------------------------------
void __fastcall TfMain::RefreshEventLogView()
{
    if(lstLog==NULL)
        return;
    AnsiString path=HSys.LogRootDir+"/EventLog/"+FormatDateTime("yyyy_mm", Now())+"/HT160S_"+FormatDateTime("yyyy_mm_dd", Now())+".csv";
    lstLog->Items->BeginUpdate();
    lstLog->Clear();
    if(FileExists(path)==false)
    {
        lstLog->Items->Add("(no EventLog today)");
        lstLog->Items->EndUpdate();
        return;
    }
    TFileStream *fs=NULL;
    TStringList *sl=NULL;
    char *buf=NULL;
    try
    {
        fs=new TFileStream(path, fmOpenRead | fmShareDenyNone);
        const int TAIL=8192;   //AI(ht160s-eventlog-view) 20260708 : read only the tail (~last lines) to stay cheap on a large daily log
        __int64 sz=fs->Size;
        __int64 start=(sz>TAIL)?(sz-TAIL):0;
        int len=(int)(sz-start);
        fs->Position=start;
        buf=new char[len+1];
        int got=(len>0)?fs->Read(buf, len):0;
        buf[got]=0;
        sl=new TStringList;
        sl->Text=AnsiString(buf, got);
        int first=(start>0 && sl->Count>0)?1:0;   //skip the partial first line when we seeked into the file
        int from=sl->Count-20;
        if(from<first)
            from=first;
        for(int i=from; i<sl->Count; i++)
            lstLog->Items->Add(sl->Strings[i]);
    }
    catch(...)
    {
        lstLog->Items->Add("(EventLog read error)");
    }
    if(buf!=NULL) delete [] buf;
    if(sl!=NULL) delete sl;
    if(fs!=NULL) delete fs;
    lstLog->Items->EndUpdate();
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

    //AI(ht160s-whitelist-override) 20260717 : effective sort-mode badge (Normal / By Lot+Bin /
    //  By Lot+PassFail / WhiteList). WhiteList is a customer special override shown in red.
    FeatureStatusPanels[eMainFeatureSortMode]      = pnlFeatureBadge4;
    FeatureStatusNameLabels[eMainFeatureSortMode]  = lblFeatureName4;
    FeatureStatusValueLabels[eMainFeatureSortMode] = lblFeatureValue4;

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

    //AI(ht160s-whitelist-override) 20260717 : seed the sort-mode badge from the effective mode.
    UpdateSortModeFeatureBadge();

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
        ShowMyOKMessageNoStop(LangT(
            "Main status badges exceed the grid (max "
            "3 cols x 2 rows = 6). The extra badges are hidden.\n"
            "Increase MAIN_FEATURE_BADGE_COLS / MAIN_FEATURE_BADGE_ROWS in "
            "main.h and widen pnlFeatureStatus to make room."));
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

    FeatureStatusValueLabels[BadgeIndex]->Caption = LangT(ValueText);
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
//AI(ht160s-whitelist-override) 20260717 : sync the main-screen sort-mode badge to the EFFECTIVE
//  sort mode (base production mode, or WhiteList while the per-lot overlay is armed). Edge-
//  triggered on the effective mode so repeated calls (mode change, Lot Start/End, host SORTMODE
//  switch, maintenance edit) are cheap. WhiteList is a customer special override, painted red so
//  the operator can see at a glance the machine is NOT in normal production.
void __fastcall TfMain::UpdateSortModeFeatureBadge()
{
    int Mode = GeneralSetting.GetEffectiveSortMode();
    if(Mode == iLastSortModeBadge)
        return;
    iLastSortModeBadge = Mode;

    switch(Mode)
    {
        case smLotBin:      SetFeatureStatusBadge(eMainFeatureSortMode, "LOT+BIN",   clGreen); break;
        case smLotPassFail: SetFeatureStatusBadge(eMainFeatureSortMode, "PASS/FAIL", clGreen); break;
        case smWhiteList:   SetFeatureStatusBadge(eMainFeatureSortMode, "WHITELIST", clRed);   break;
        default:            SetFeatureStatusBadge(eMainFeatureSortMode, "NORMAL",    clGray);  break;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbLaguageClick(TObject *Sender)
{
    //AI(ht160s-language) 20260626 : toggle EN/ZH and re-apply live, instead of
    //  opening the (now retired) empty fLan form. Blocked while running.
    if(HSys.Sys.SystemStart)
        return;
    if(HSys.LastSet.iLanguageCountry==0)
        HSys.LastSet.iLanguageCountry=1;
    else
        HSys.LastSet.iLanguageCountry=0;
    SaveMainRunSettingsToIni();
    if(fLan != NULL)
        fLan->ChangeLanguage(this);
    sbLaguage->Down=false;
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
//AI(secs-ceid-align9045) 20260729 : the four page-entry CEIDs are reported from the button
//handlers that OPEN each page, not from the forms' OnShow. Two reasons : the form units
//(uOffset/uspeed/systools) do not include the SECS headers, and a re-show caused by z-order
//or restore should not look like a fresh operator navigation. HT9045 reports these from
//main.cpp too (24657 Speed / 24679 Offset / 24931 IO / 24954 Maintenance).
//NOTE : HT160S's Setup page (fSetup) intentionally gets NO CEID - HT9045 has no Setup-page
//event, and its 22/23 (Enter Message Page / Enter Debug Page) are pages HT160S does not have.
void __fastcall TfMain::sbMaintanceClick(TObject *Sender)
{
    EventReport(SECS_EVENT.EnterConfig);        //18 Enter Maintenance Page
    ShowTopForm(fMaintenance, sbMaintance);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbOffsetClick(TObject *Sender)
{
    EventReport(SECS_EVENT.EnterOffset);        //19 Enter Offset Page
    ShowTopForm(fOffset, sbOffset);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbSpeedClick(TObject *Sender)
{
    EventReport(SECS_EVENT.EnterSpeed);         //20 Enter Speed Page
    ShowTopForm(fSpeed, sbSpeed);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbMessageClick(TObject *Sender)
{
    //AI(secs-ceid-align9045) 20260729 : CEID 22 "Enter Message Page" (HT9045 has the same
    //button). Reported from the button handler like the other page-entry ids.
    //AI(ht160s-toolbar) 20260804 : re-pointed from fNote to fData. fNote is the ALARM /
    //recovery form, not a page : showing it modally with no alarm pending sounded the buzzer
    //(note.cpp FormShow PlayAlarmBuzzer), handed the physical operator panel to
    //TfNote::ScanKey (main.cpp ScanPanelKeys early-returns while fNote->fShow) and let the
    //panel PAUSE key run DecStopAllMotor() from what the operator thinks is an info page.
    //fData is HT172's Message page (main.cpp:948 fData->ShowModal), still an empty stub here.
    EventReport(SECS_EVENT.EnterMessage);
    ShowTopForm(fData, sbMessage);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbExitClick(TObject *Sender)
{
    //AI(secs-ceid-align9045) 20260729 : CEID 24 "Exit Pressed" (HT9045 main.cpp:10234, which
    //also reports on the exit path before the form actually closes). Sent here rather than in
    //FormClose so it goes out while the SECS link is still up - FormClose tears the stack down.
    EventReport(SECS_EVENT.DoExit);
    Close();
}
//---------------------------------------------------------------------------
//AI(general) 20260601 : stop the run-control thread before the VCL message
//loop ends (ref HT172 main.cpp "MyThread->Terminate(); MyThread->WaitFor();").
//Must run here while messages are still pumped, otherwise the worker thread's
//Synchronize(MainProc) can never complete and WaitFor would deadlock.
void __fastcall TfMain::FormClose(TObject *Sender, TCloseAction &Action)
{
    //AI(ht160s-lot-reset) 20260706 : persist production counters on shutdown (HT172
    //main.cpp:578 parity) so a power cycle mid-lot resumes the count, not loses it.
    WriteLastDataIni();

    if(MyThread != NULL)
    {
        MyThread->Terminate();
        MyThread->WaitFor();
    }

    //AI(ht160s-ftp) 20260721 : stop the background FTP upload worker before the VCL
    //message loop ends. EndThread signals its events, WaitFor blocks until Execute
    //returns (the worker never Synchronizes, so no deadlock against this main-thread
    //WaitFor). Any in-flight upload is abandoned; a partially-sent lot is re-sent on
    //the next Lot End (the KYEC hand-off is idempotent, flag-committed).
    if(FtpUploadThd != NULL)
    {
        FtpUploadThd->EndThread();
        FtpUploadThd->WaitFor();
        delete FtpUploadThd;
        FtpUploadThd = NULL;
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

        //AI(ht160s-agv) 20260624 : refresh the per-zone "trays on the AMR car" header.
        //Motion View only, so gated here with the rest of the per-frame visible-page work.
        ShowCarTrayCount();
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260624 : format an AMR-car tray total as "identity/cover/other".
//Given the live total t and how many identity(id0)/cover(cv0) trays a full car holds,
//derive the per-kind split assuming normal trays drain first, then cover, then
//identity. Sum(id,cv,nm) always equals t so the shown total still matches the count.
//id0<0 means the whole car is identity trays (Color supply).
static AnsiString FmtCarKinds(int t, int id0, int cv0)
{
    int id, cv, nm;
    if(t<0) t=0;
    if(id0<0)                   // whole car is identity (Color)
        return IntToStr(t) + "/0/0";
    id = (t<id0) ? t : id0;     // identity reserved (drains last)
    cv = t - id;                // remaining after identity
    if(cv > cv0) cv = cv0;      // cover capped (drains after normal)
    if(cv < 0)   cv = 0;
    nm = t - id - cv;           // normal = remainder (drains first)
    return IntToStr(id) + "/" + IntToStr(cv) + "/" + IntToStr(nm);
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260624 : PanelMain6 Motion View header - per-zone "trays on the AMR
//car" count. Input zones (Loader/Empty/Color) show the sim supply-car remaining count
//(drains as trays feed); Auto1~6 show the output-car accumulated stack count. Display
//only and NULL-guarded; reads module accessors, never writes machine state. In real
//mode the input counts are sensor-driven and not maintained (they show the configured
//max); the Auto output counts are book-keeping and valid in both sim and real.
//AI(ht160s-uph) 20260707 : sgProductInfo row indices (mirror HT172 IncludeAllHeader
// e-enum: Lot Start / Lot End / Alarm Time / Pause Time / UPH). File-scope enum
// (this is a .cpp, not the form class body).
//AI(jamrate) 20260801 : PI_Jam added last so no existing row index moves.
enum { PI_LotStart=0, PI_LotEnd, PI_AlarmTime, PI_PauseTime, PI_UPH, PI_TotalTime, PI_AutoSkip, PI_Jam };
//AI(ht160s-uph) 20260708 : Lot End time + final UPH + UPH-denominator time are shown only
//AFTER a lot ends (blank during a running lot). false at Lot Start, true at Lot End finalize.
static bool bLotEnded=false;
//---------------------------------------------------------------------------
void __fastcall TfMain::ClearProductInfoAtLotStart()
{
    bLotEnded=false;
    if(sgProductInfo==NULL)
        return;
    sgProductInfo->Cells[1][PI_LotEnd   ]="";
    sgProductInfo->Cells[1][PI_UPH      ]="";
    sgProductInfo->Cells[1][PI_TotalTime]="";
}
//---------------------------------------------------------------------------
//AI(ht160s-uph) 20260709 : "Clear All" counts button (previously a dead DFM control).
//Zeroes the per-lot production counters (TotalIC/Bin/Tray/UPH/Loader/Jam + SECS
//Scanned/Sorted) via the shared reset, persists lastdata, and blanks the product-info
//panel. Blocked while running. This is the missing UI to clear a stale TotalIC that
//lastdata restored at boot, which would otherwise ride into the next run and spike UPH.
void __fastcall TfMain::btnClearCountClick(TObject *Sender)
{
    (void)Sender;
    if(HSys.Sys.SystemStart)
    {
        ShowMyMessage(LangT("Stop the machine before Clear Count."));
        return;
    }
    if(ShowMyMessageBox_YES_NO(LangT("Clear all production counts ?"))!=TMyMessageBox::msgrtnYES)
        return;
    //AI(secs-ceid-align9045) 20260729 : CEID 5 "ClearCount Pressed" (HT9045 cSortCT.cpp:609).
    //Reported AFTER the operator confirms, so a cancelled dialog sends nothing.
    EventReport(SECS_EVENT.DoClearCount);
    ResetPerLotProductionCounters();
    WriteLastDataIni();
    ClearProductInfoAtLotStart();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::FreezeProductInfoAtLotEnd()
{
    bLotEnded=true;
    if(sgProductInfo==NULL)
        return;
    sgProductInfo->Cells[1][PI_LotEnd   ]=FormatDateTime("hh:nn:ss", tRunData.LotEndTime);
    sgProductInfo->Cells[1][PI_UPH      ]=IntToStr(tRunData.UPH);
    sgProductInfo->Cells[1][PI_TotalTime]=(tRunData.LotEndTime-tRunData.StartTime-tUPH_PauseTime).FormatString("hh:nn:ss");
}
//---------------------------------------------------------------------------
// AI(ht160s-uph) 20260707 : live production-info panel (HT172 TfMain::ShowProductInfo
// parity). One-time header, then per-cycle fill of Lot Start/End/Alarm/Pause and a
// LIVE cumulative UPH (TotalIC / productive-hours, pause excluded) written to
// tRunData.UPH so the screen AND SECS SVID 1021 track running UPH. The frozen
// end-of-lot value is still (re)computed at Clean Out finish and End of Lot.
//AI(loadtotal) 20260801 : KYEC on-site note 7 "Load and total number is zero". The two
//panels above the Bin column were ported from HT172 as DFM objects but the two assignment
//lines at the tail of HT172 ShowBinCount() (HT172 main.cpp:2438-2439) were not, so both
//kept their design-time literal "0" forever - a repo-wide grep for palloadingCount /
//palUnloadingCount found only the DFM and the main.h declarations, no ->Caption= anywhere.
//The other half of the same note was tRunData.LoaderIC never being incremented, fixed at
//aSortArm.cpp TransferPickDataFromLoader.
//
//SEMANTICS, stated plainly because it matters to the operator: both counters live in
//tRunData and are zeroed by ResetPerLotProductionCounters, i.e. by Lot Start / SECS
//LOTSTART / the Clear All button - NOT by Lot End. So they read "since the last lot start
//or clear", and on a machine driven with START + LOT END only they carry across lots and
//across power cycles (tRunData is restored from lastdata.dat at boot, by explicit design).
//That is the same epoch SVID 1101 / 1102 already report to the host, so the screen
//and the host now agree. (66020 carried it too until the 66xxx band was retired 20260804.) Whether Lot End should also zero them is a separate, customer-
//visible decision recorded in docs/plan/onsite-0731-kyec-triage-20260801.md.
//
//Write-only-on-change, matching ShowUnloadAutoInfo, so the panels do not flicker. Explicit
//IntToStr rather than HT172's implicit AnsiString(int) into TPanel::Caption.
void __fastcall TfMain::ShowBinCntInfo()
{
    AnsiString sLoad =IntToStr(tRunData.LoaderIC);
    AnsiString sTotal=IntToStr(tRunData.TotalIC);
    if(palloadingCount  !=NULL && palloadingCount->Caption  !=sLoad ) palloadingCount->Caption  =sLoad;
    if(palUnloadingCount!=NULL && palUnloadingCount->Caption!=sTotal) palUnloadingCount->Caption=sTotal;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::ShowProductInfo()
{
    if(sgProductInfo==NULL)
        return;
    static bool bProdHdr=false;
    if(bProdHdr==false)
    {
        bProdHdr=true;
        sgProductInfo->Cells[0][PI_LotStart ]="Lot Start :";
        sgProductInfo->Cells[0][PI_LotEnd   ]="Lot End :";
        sgProductInfo->Cells[0][PI_AlarmTime]="Alarm Time :";
        sgProductInfo->Cells[0][PI_PauseTime]="Pause Time :";
        sgProductInfo->Cells[0][PI_UPH      ]="UPH :";
        sgProductInfo->Cells[2][PI_UPH      ]="Unit / Hr";
        sgProductInfo->Cells[0][PI_TotalTime]="UPH Time :";
        //AI(jamrate) 20260801 : 8 rows. The grid is Align=alLeft inside Panel1, so its
        //Height is overwritten with the parent client height at runtime and cannot be
        //raised from code; Panel1 itself cannot grow either - its parent Panel9 has only
        //~17 px of slack (alTop 4+141+141+291 = 577, alBottom 116+122 = 238, client 832).
        //So the row fits by trimming DefaultRowHeight 16 -> 14 in the DFM: 8*(14+1) = 120,
        //exactly the grid height, where 7*(16+1) = 119 was before.
        sgProductInfo->RowCount=8;
        sgProductInfo->Cells[0][PI_AutoSkip]="Auto Skip :";
        sgProductInfo->Cells[2][PI_AutoSkip]="pcs";
        sgProductInfo->Cells[0][PI_Jam]="Jam :";
    }
    sgProductInfo->Cells[1][PI_LotStart ]=FormatDateTime("hh:nn:ss", tRunData.StartTime);
    sgProductInfo->Cells[1][PI_LotEnd   ]=bLotEnded ? FormatDateTime("hh:nn:ss", tRunData.LotEndTime) : AnsiString("");
    sgProductInfo->Cells[1][PI_AlarmTime]=FormatDateTime("hh:nn:ss", tRunData.AlarmTime);
    sgProductInfo->Cells[1][PI_PauseTime]=tUPH_PauseTime.FormatString("hh:nn:ss");
    sgProductInfo->Cells[1][PI_AutoSkip]=IntToStr(tRunData.iAutoSkipCount);
    //AI(jamrate) 20260801 : the CALCULATE half. Recomputed every repaint rather than only
    //at each AddJamCount, so the rate also tracks the growing denominator between jams.
    //Shown unconditionally (not inside the SystemStart guard below) so the figure survives
    //a stop for the operator to read - it is a lot statistic, not a live-only reading.
    RefreshJamRate();
    sgProductInfo->Cells[1][PI_Jam]=IntToStr(tRunData.JamCount);
    sgProductInfo->Cells[2][PI_Jam]=AnsiString().sprintf("%.1f/%d", tRunData.JamRate, GetJamRateDenom());
    if(HSys.Sys.SystemStart && bFirstRun==false && tRunData.TotalIC>0)
    {
        // AI(ht160s-uph) 20260709 : small-sample warm-up guard. Early in a lot the
        // elapsed window is tiny, so GetCalculateUPH (TotalIC*3600/sec) explodes into a
        // bogus spike. Below the threshold, hide it: screen shows "--", tRunData.UPH=0 so
        // SECS SVID 1021 does not report the spike either. GeneralSetting.iUphMinSampleIC:
        // 0 = auto (one full tray from live TrayForm geometry); >0 = fixed IC count.
        int iUphMinN=GeneralSetting.iUphMinSampleIC;
        if(iUphMinN<=0)
            iUphMinN=(SortArmModule!=NULL) ? (SortArmModule->GetTrayXCount()*SortArmModule->GetTrayYCount()) : 0;
        // AI(secs-onsite0731) 20260801 : test the WINDOW count, not the cumulative one.
        // tRunData.TotalIC survives power cycles, so on any boot with carry-over this guard
        // was already satisfied by stale units (on site: 317 against a MinSampleIC of 0 ->
        // auto tray-cell count) and the first-minutes spike shipped to the screen and to
        // SVID 1021 anyway - it failed in precisely the case it was written for.
        if(iUphMinN>0 && (tRunData.TotalIC-g_iUphBaseIC)<iUphMinN)
        {
            tRunData.UPH=0;
            sgProductInfo->Cells[1][PI_UPH]="--";
        }
        else
        {
            tRunData.UPH=GetCalculateUPH(Now());
            sgProductInfo->Cells[1][PI_UPH]=IntToStr(tRunData.UPH);
        }
        sgProductInfo->Cells[1][PI_TotalTime]=(Now()-tRunData.StartTime-tUPH_PauseTime).FormatString("hh:nn:ss");
    }
}
//---------------------------------------------------------------------------
// AI(ht160s-uph) 20260707 : rolling per-tray UPH history + Avg (HT172
// MySortArmParameter::CalculateUPH / UPH_StringGrid parity). Renders the cprod ring
// (newest at row 1) only when a tray completed (g_UphRowsDirty), so it is cheap.
void __fastcall TfMain::ShowTrayUphHistory()
{
    if(UPH_StringGrid==NULL)
        return;
    static bool bUphHdr=false;
    if(bUphHdr==false)
    {
        bUphHdr=true;
        UPH_StringGrid->Cells[0][0]="Start Time";
        UPH_StringGrid->Cells[1][0]="End Time";
        UPH_StringGrid->Cells[2][0]="Pause Time";
        UPH_StringGrid->Cells[3][0]="UPH";
        g_UphRowsDirty=true;
    }
    if(g_UphRowsDirty==false)
        return;
    g_UphRowsDirty=false;

    int i, iTotalUph=0, iCnt=0;
    for(i=0; i<UPH_ROW_MAX; i++)
    {
        int r=i+1;   // grid data rows 1..10, newest first
        if(i<g_UphRecentCount)
        {
            UPH_StringGrid->Cells[0][r]=g_UphRecentRows[i].sStart;
            UPH_StringGrid->Cells[1][r]=g_UphRecentRows[i].sEnd;
            UPH_StringGrid->Cells[2][r]=g_UphRecentRows[i].sPause;
            UPH_StringGrid->Cells[3][r]=IntToStr(g_UphRecentRows[i].iUph);
            iTotalUph+=g_UphRecentRows[i].iUph;
            iCnt++;
        }
        else
        {
            UPH_StringGrid->Cells[0][r]="";
            UPH_StringGrid->Cells[1][r]="";
            UPH_StringGrid->Cells[2][r]="";
            UPH_StringGrid->Cells[3][r]="";
        }
    }
    UPH_StringGrid->Cells[2][12]="Avg UPH :";
    UPH_StringGrid->Cells[3][12]=IntToStr((iCnt>0)?(iTotalUph/iCnt):0);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::ShowCarTrayCount()
{
    if(LoaderModule!=NULL && lbCarTrayCount_Loader!=NULL)
        lbCarTrayCount_Loader->Caption=FmtCarKinds(LoaderModule->GetCarTrayCount(), GeneralSetting.iAmrIdentityTray[0], GeneralSetting.iAmrCoverTray[0]);
    if(EmptyModule!=NULL && lbCarTrayCount_Empty!=NULL)
        lbCarTrayCount_Empty->Caption=FmtCarKinds(EmptyModule->GetCarTrayCount(), GeneralSetting.iAmrIdentityTray[1], GeneralSetting.iAmrCoverTray[1]);
    if(ColorModule!=NULL && lbCarTrayCount_Color!=NULL)
        lbCarTrayCount_Color->Caption=FmtCarKinds(ColorModule->GetCarTrayCount(), GeneralSetting.iAmrIdentityTray[2], GeneralSetting.iAmrCoverTray[2]);

    if(AutoModule!=NULL)
    {
        TLabel *AutoLbl[6]={lbCarTrayCount_Auto1, lbCarTrayCount_Auto2, lbCarTrayCount_Auto3,
                            lbCarTrayCount_Auto4, lbCarTrayCount_Auto5, lbCarTrayCount_Auto6};
        for(int i=0; i<6; i++)
            if(AutoLbl[i]!=NULL)
                AutoLbl[i]->Caption=FmtCarKinds(AutoModule->GetCarTrayCount(i), GeneralSetting.iAmrIdentityTray[3+i], GeneralSetting.iAmrCoverTray[3+i]);
    }
}
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

    //AI(ht160s-lotpassfail) 20260709 : both dynamic modes reverse-look-up the binding table.
    //The middle field is a Bin in Lot+Bin mode but a PASS/FAIL class (1/2) in Lot+PassFail
    //mode, so show "PASS"/"FAIL" text there instead of the raw class number.
    bool bDynamicMode=GeneralSetting.IsDynamicBindingMode();
    bool bPassFailMode=GeneralSetting.IsLotPassFailSortMode();
    //AI(ht160s-lotpassfail) 20260730 : in By Lot+PassFail mode the binding's middle key IS
    //the PASS/FAIL class frozen at CCD scan (1=PASS, 2=FAIL), so compare against the class.
    //It used to be compared against BinAreaMap.GetPassBin() - a BIN number - which agreed
    //with the class only by luck, when the configured Pass Bin happened to be 1. That
    //setting is gone; PASS/FAIL is customer data (per-IC DiePass) now.

    for(int i=0; i<6; i++)
    {
        AnsiString sBin="0";
        AnsiString sLot="";

        if(bDynamicMode)
        {
            int BindCount=LotBinBinding.GetBindingCount();
            for(int j=0; j<BindCount; j++)
            {
                AnsiString BindLotID;
                int BindBin=0, BindAuto=-1;
                if(LotBinBinding.GetBindingByIndex(j, BindLotID, BindBin, BindAuto) && BindAuto==i)
                {
                    if(bPassFailMode)
                    {
                        if(BindBin==1)
                            sBin=AnsiString("PASS");
                        else
                            sBin=AnsiString("FAIL");
                    }
                    else
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
        sgMotorStatus->Cells[ 0][0]=LangT("Motor Name");
        sgMotorStatus->Cells[ 1][0]=LangT("Target");
        sgMotorStatus->Cells[ 2][0]=LangT("Position");
        sgMotorStatus->Cells[ 3][0]=LangT("Encoder");
        sgMotorStatus->Cells[ 4][0]=LangT("CW");
        sgMotorStatus->Cells[ 5][0]=LangT("HOME");
        sgMotorStatus->Cells[ 6][0]=LangT("CCW");
        sgMotorStatus->Cells[ 7][0]=LangT("Emg");
        sgMotorStatus->Cells[ 8][0]=LangT("Alarm");
        sgMotorStatus->Cells[ 9][0]=LangT("SoftCW");
        sgMotorStatus->Cells[10][0]=LangT("SoftCCW");
        sgMotorStatus->Cells[11][0]=LangT("ServoAlarm");
        sgMotorStatus->Cells[12][0]=LangT("InPos");
        sgMotorStatus->Cells[13][0]=LangT("Z Phase");
        sgMotorStatus->Cells[14][0]=LangT("ServoOn");
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

    //AI(ht160s-motion-view-tray-display) 20260624 : Color column carrier position-binds
    // to MColorY (front<->rear). //AI(ht160s-tray-source) 20260625 : Color now drives the
    // MMColorY content motor so the presented identity tray shows on MotionView (frame =
    // has-tray; identity grid is empty, no IC). Visibility is fHasTray-driven per frame.
    BindMovingTrayPanel(HSys.Mot.MColorY, HSys.VMot.MMColorY, plColorTrayWork, mtColorTrayWork);

    // Loader has two physical Y lanes: MLoaderY_1 (Left) and MLoaderY_2 (Right).
    BindMovingTrayPanel(HSys.Mot.MLoaderY_1, HSys.VMot.MMLoaderY_1, plLoaderLTrayWork, mtLoaderLTrayWork);
    BindMovingTrayPanel(HSys.Mot.MLoaderY_2, HSys.VMot.MMLoaderY_2, plLoaderRTrayWork, mtLoaderRTrayWork);

    //AI(ht160s-loader-2d-panel) 20260708 : mirror each Loader lane tray onto the static
    //"Loader 2D Left/Right" grids on the Tray Status page - the SUB panel of the same
    //content motor (TTrayMotor::Refresh paints main + sub). Display only, no gating.
    if(HSys.VMot.MMLoaderY_1 != NULL && mtLoaderL != NULL)
        HSys.VMot.MMLoaderY_1->SetSubHTrayPanel(mtLoaderL);
    if(HSys.VMot.MMLoaderY_2 != NULL && mtLoaderR != NULL)
        HSys.VMot.MMLoaderY_2->SetSubHTrayPanel(mtLoaderR);

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
    ApplyTrayDivisionToPanel(mtColorTrayWork, HSys.VMot.MMColorY, X, Y);

    ApplyTrayDivisionToPanel(mtLoaderLTrayWork, HSys.VMot.MMLoaderY_1, X, Y);
    ApplyTrayDivisionToPanel(mtLoaderRTrayWork, HSys.VMot.MMLoaderY_2, X, Y);

    //AI(ht160s-loader-2d-panel) 20260708 : size the static Loader 2D Left/Right grids too.
    ApplyTrayDivisionToPanel(mtLoaderL, HSys.VMot.MMLoaderY_1, X, Y);
    ApplyTrayDivisionToPanel(mtLoaderR, HSys.VMot.MMLoaderY_2, X, Y);
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
//AI(ht160s-agv) 20260623 : write the sim AMR per-zone max-tray grid back to GeneralSetting
//[SimAMR] and persist. Sim-only test parameter; takes effect at the next AMR refill (or
//restart). Index 0=Loader 1=Empty 2=Color 3..8=Auto1..6.
void __fastcall TfMain::btnSaveSimMaxClick(TObject *Sender)
{
    (void)Sender;
    if(sgSimMaxTray==NULL)
        return;
    for(int i=0;i<9;i++)
    {
        int v=StrToIntDef(sgSimMaxTray->Cells[1][i+1], 10);
        if(v<1) v=1;
        if(v>999) v=999;
        GeneralSetting.iSimAmrMaxTray[i]=v;
    }
    GeneralSetting.Save();
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
            //AI(ht160s-lotpassfail) 20260730 : stamp a DiePass so By Lot+PassFail stays
            //testable on the laptop. Mirrors the customer pattern where HBin 1 is the pass
            //bin : Bin 1 -> DiePass "1" (PASS), everything else "0" (FAIL). Without it every
            //simulated IC would classify FAIL (a missing DiePass counts as FAIL) and a single
            //Auto would swallow the whole simulated run.
            AnsiString SimDiePass;
            if(Bin==1)
                SimDiePass="1";
            else
                SimDiePass="0";
            LotRegistry.AddItemEx(LotID, Code, Bin, Bin, Bin, "", SimDiePass,
                "", "", "", DupExistingLot);
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
        ShowMyMessage(LangT("Recipe name cannot be empty."));
        return;
    }

    if(HSys.Sys.SystemStart)
    {
        cb_WorkFile->Text = CurrentRecipe;
        ShowMyMessage(LangT("Can not change recipe while machine is running."));
        return;
    }

    SelectedRecipe = RecipeManager.NormalizeRecipeName(cb_WorkFile->Text);
    if(!RecipeManager.RecipeExists(SelectedRecipe))
    {
        cb_WorkFile->Text = CurrentRecipe;
        ShowMyMessage(LangT("Recipe does not exist."));
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
    if(fOffset != NULL)   //AI 20260623 : load new recipe offset and re-fold effective Teach
        fOffset->OpenWorkFile();

    //AI(HT160S-Maintainer) 20260608 : refresh Monitor Col/Row grid for new recipe
    SyncMonitorTrayDivision();

    LogText = AnsiString("Change Recipe to ")+RecipeManager.GetCurrentRecipeName();
    RecordProcess(LogText);
    EventReport(SECS_EVENT.SwitchSetupFile);
}
//---------------------------------------------------------------------------
#ifndef SOFT_SIMULATE   //AI(ht160s-password) 20260624 : login keypad helper (real build only; sim auto-grants)
// Prompt one text value via the on-screen QWERTY keypad using a hidden scratch
// edit (parented so the VCL handle is valid). Returns false if operator cancels.
static bool PromptLoginInput(TWinControl *AParent, AnsiString ATitle, int AFunc, AnsiString &AValue)
{
    TEdit *Scratch;
    bool bOK;

    if(fQwertyKey==NULL || AParent==NULL)
        return false;

    Scratch=new TEdit(AParent);
    bOK=false;
    try
    {
        Scratch->Parent=AParent;
        Scratch->Visible=false;
        Scratch->Text=AValue;
        bOK=fQwertyKey->ShowQwertyKey(Scratch, AFunc, 0, false, 0, 0, ATitle);
        if(bOK)
            AValue=Scratch->Text;
    }
    __finally
    {
        delete Scratch;
    }
    return bOK;
}
#endif
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
        AnsiString sLoginID="";
        AnsiString sLoginPass="";
        if(PromptLoginInput(this, "Login User ID", N_NO_SPACE, sLoginID)==false ||
           sLoginID.Trim()==AnsiString(""))
        {
            bUpdatingMainSelections = true;
            RefreshMainUserSelect();
            bUpdatingMainSelections = false;
            return;
        }
        if(PromptLoginInput(this, "Login Password", N_PASSWORD|N_NO_SPACE, sLoginPass)==false)
        {
            bUpdatingMainSelections = true;
            RefreshMainUserSelect();
            bUpdatingMainSelections = false;
            return;
        }
        if(UserRoleManager.Login(RoleLevel, sLoginID, sLoginPass)==false)
        {
            ShowMyMessage(LangT("User ID or password is incorrect."));
            bUpdatingMainSelections = true;
            RefreshMainUserSelect();
            bUpdatingMainSelections = false;
            return;
        }
        #endif
    }

    bUpdatingMainSelections = true;
    RefreshMainUserSelect();
    bUpdatingMainSelections = false;

    if(OldRoleLevel != UserRoleManager.GetLevel())
    {
        LogText = AnsiString("Change User to ")+UserRoleManager.GetLevelName();
        RecordProcess(LogText);
        EventReport(SECS_EVENT.SwitchUser);
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
        pnRealDummy->Caption = LangT("Real");
        pnRealDummy->Font->Color = clRed;
    }
    else if(HSys.LastSet.iRealDummy == HAS_TRAY)
    {
        pnRealDummy->Caption = LangT("HasTray");
        pnRealDummy->Font->Color = clBlack;
    }
    else
    {
        pnRealDummy->Caption = LangT("Dummy");
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
        pnStartMode->Caption = LangT("Initial");
    else
        pnStartMode->Caption = LangT("Continue");
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

    //AI(secs-ceid-align9045) 20260729 : CEID 14 "Switch StartMode". HT9045 reports this from
    //every start-mode / run-mode change point (main.cpp:835, 1012, 21328 - "Run Mode 改變都需要
    //上報SECS GEM EVEN"). HT160S has exactly one toggle for it, this handler, and it is already
    //gated on a stopped machine above, so one site covers the whole event.
    EventReport(SECS_EVENT.SwitchStartMode);

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
    EventReport(SECS_EVENT.SwitchRunMode);
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

        ProbeStage = "Message:create";
        if(fData == NULL)
            fData = new TfData(this);
        ProbeStage = "Message:show";
        SmokeShowTopForm(fData);
        OpenedForms += ",Message";

        //AI(ht160s-toolbar) 20260804 : fNote has no toolbar button (it is the alarm form),
        //but keep it in the smoke probe - it is the most safety-critical screen we own.
        ProbeStage = "Note:create";
        if(fNote == NULL)
            fNote = new TfNote(this);
        ProbeStage = "Note:show";
        SmokeShowTopForm(fNote);
        OpenedForms += ",Note";
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



//AI(machine-command-layer) 20260625 : shared full-machine HOME sequence, extracted from
//sbHome1Click so the Home button AND the SECS HOME host command run the SAME steps (no
//drift). Excludes the "Confirm home?" modal and the button cosmetics -- the button
//handler keeps those; the SECS path must not pop a modal on the receive thread.
void TfMain::HomeCore()
{
    if(fHome==NULL)   //AI(machine-command-layer) 20260625 : SECS HOME reaches here via fMain->HomeCore; guard fHome like the kernel (csystem.cpp) does. fHome is created right after fMain at startup, so this is defensive.
        return;
    fHome->lstHomeMsg->Clear();
    fHome->lstHomeMsg->Items->Insert(0, "Starting home procedure....");
    fHome->Show();

    RecordProcess("HOME pressed");
    EventReport(SECS_EVENT.DoHome);
    ChangeRunMode(Run_Home);
    HSys.Sys.SystemStart=true;
    fHome->MarkSeenStart();

    fAllMotorHome=false;
    ArmMotorHome();
    SoftStart=true;
    bHomeByStart=false;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbHome1Click(TObject *Sender)
{
    //AI(HT160S-Maintainer) 20260602 : HT172-style home. Press Home starts the
    //  full-machine home (reuse Run_Home engine) and shows the Home monitor
    //  non-modally; the monitor auto-closes when homing finishes (see uHome).
#ifndef SOFT_SIMULATE
    int ret=ShowMyMessageBox_YES_NO(LangT("Confirm home?"));
    if(ret==TMyMessageBox::msgrtnYES)
#endif
    {
        HomeCore();   //AI(machine-command-layer) 20260625 : shared with SECS HOME
    }
#ifndef SOFT_SIMULATE
    else
        return;
#endif
    sbHome1->Down=false;
}
//---------------------------------------------------------------------------

//AI(secs-kyec-rcmd4) 20260728 : factored One Cycle entry point - the same split the machine
//command layer already uses for Start (MachineStart) and Home (HomeCore). The BUTTON keeps
//its ShowMyMessage modals; the SECS S2F41 "ONE_CYCLE" branch calls this core and maps the
//result to HCACK. A modal on the SECS path would stall the HSMS receive thread.
//bRequireRunning is the SECS-ONLY stale-arm guard : ChangeRunMode(Run_OneCycle) on a stopped
//machine is a LATCH that nothing clears until the next Start, because ProcessMotion
//early-returns while SystemStart==false so the OneCycle-finish dispatcher never runs, and
//DoStartArm only resets RunMode on the UNHOMED path. On a homed, stopped machine a host
//command accepted at idle would silently turn the operator's NEXT Start into one cycle
//followed by an automatic stop. The operator button passes false and keeps "arm one cycle,
//then Start".
//AI(secs-msggap-fix) 20260729 : the field evidence, recounted, is stronger than "observed, not
//theoretical" - it is an actual stranded arm, not just a retry burst. KYEC's ONE_CYCLE at
//19:00:19.281 was ACCEPTED and armed (9045 CEID 3 at 19:00:19.306) on a machine that went HALT
//0.85 s later, and 9045 clears that latch at exactly one site inside the finish handler, which
//then never ran. The retry burst around it is 9 commands over 8 min 00.3 s at an exact 60.0 s
//cadence (7 unambiguously in HALT, one on the ART->HALT boundary, the last in Alarm) - the
//earlier "nine times in nine minutes" was loose. See docs/plan/secs-9045-claim-crosscheck-20260729.md.
eOneCycleResult TfMain::OneCycleCore(bool bRequireRunning, AnsiString &Reason)
{
    Reason = "";
    //AI(secs-audit-fix) 20260729 : the STOPPED test now runs FIRST, ahead of the already-armed
    //test. Order matters for the host answer : a stopped machine that still carries a stale
    //Run_OneCycle latch used to answer "already armed" -> HCACK=4 (a positive ack promising a
    //completion event) when the truthful answer is "cannot perform now" -> HCACK=2. With the
    //stopped test first, a stopped machine ALWAYS answers 2 regardless of the latch, so the host
    //can never be told a cycle is under way while the machine sits still. Operator button path
    //is unchanged : it passes bRequireRunning=false, so this test is skipped entirely and
    //arm-at-idle-then-Start still works.
    if(bRequireRunning && HSys.Sys.SystemStart==false)
    {
        Reason = "One Cycle from host requires the machine to be running.";
        return ocRejStopped;
    }
    //Checked before the mode gate so an already-armed cycle reports busy (HCACK=4) rather
    //than the generic mode message. HT9045 silently no-ops this case, which is what made
    //the KYEC retry storm invisible.
    if(HSys.Sys.RunMode==Run_OneCycle)
    {
        Reason = "One Cycle is already armed.";
        return ocRejBusy;
    }
    if(HSys.Sys.RunMode!=Run_Normal && HSys.Sys.RunMode!=Run_CleanOut)
    {
        Reason = "One Cycle is only allowed in Normal / Clean Out mode.";
        return ocRejMode;
    }
    //Deliberately NO HasICUnderMachine() gate : One Cycle is a place-what-you-hold-then-stop
    //operation that is DESIGNED to leave material under the machine, so gating on IC would
    //make the command permanently unusable mid-lot - its only real use.
    if(CheckLotDataReady(Reason)==false)
        return ocRejNotReady;
    EventReport(SECS_EVENT.DoOneCycle);
    ChangeRunMode(Run_OneCycle);
    Reason = "armed";
    return ocStarted;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbOneCycle1Click(TObject *Sender)
{
    //AI(poka-yoke) 20260616 : was a silent no-op when mode/data was wrong. One
    //  Cycle runs at idle, so ShowMyMessage here is safe (does not stop a running
    //  machine). Tell the operator why, and require the same lot/2D data as Start.
    //AI(secs-kyec-rcmd4) 20260728 : gates factored into OneCycleCore() so the SECS S2F41
    //  "ONE_CYCLE" branch can share them without these modals. The operator path passes
    //  bRequireRunning=false, so "arm one cycle then press Start" behaves exactly as before.
    AnsiString Reason;
    if(OneCycleCore(false, Reason)!=ocStarted)
    {
        ShowMyMessage(LangT(Reason));
        return;
    }
    RecordProcess("ONE CYCLE pressed");
}
//---------------------------------------------------------------------------

//AI(secs-rcmd-9045) 20260729 : shared Clean Out body, factored out of sbCleanOut1Click so the
//SECS S2F41 "CLEAN_OUT" host command can arm the same drain (HT9045 uHGemHT9045.cpp:1460 calls
//its own BtnCleanOutClick for exactly this reason). Mirrors the OneCycleCore/HomeCore pattern:
//no modal, no UI assumption, returns whether it armed so the caller can map the result (operator
//-> silent no-op as before, SECS -> HCACK). The Run_Normal gate is the original one and is what
//makes a second CLEAN_OUT while already draining a no-op rather than a restart.
bool TfMain::CleanOutCore()
{
    if(HSys.Sys.RunMode!=Run_Normal)
        return false;
    EventReport(SECS_EVENT.DoCleanOut);
    HSys.Sys.bCleanOut=true;
    ChangeRunMode(Run_CleanOut);
    return true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbCleanOut1Click(TObject *Sender)
{
    if(CleanOutCore())
        RecordProcess("CLEAN OUT pressed");
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbStart1Click(TObject *Sender)
{
    Start();        //JerryYang 20250106 : Run check    
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbPause1Click(TObject *Sender)
{
    //AI(machine-command-layer) 20260625 : route the operator Pause through the single
    //MachinePause() choke point (csystem.cpp) so the SAME RecordProcess + SystemStart-drop
    //+ DecStop + SoftStop runs for the button, the panel key and SECS PAUSE. EventReport
    //(PressPause) stays here : it is operator-specific and MachinePause is shared with the
    //safety/EMG stop paths that must not raise a Pause event.
    if(HSys.Sys.SystemStart==true)
        EventReport(SECS_EVENT.DoPause);
    MachinePause(trigOperator);
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
        ShowMyMessage(LangT("State Record snapshot failed (check 7-Zip / disk)."));
}
//---------------------------------------------------------------------------
//AI(ht160s-whitelist) 20260727 : last WhiteList.json load failure, in operator words.
//  Set by LoadWhiteListFile (missing / unreadable / bad JSON / missing KYECLotID),
//  cleared on a valid load, read by CheckLotDataReady so the blocked Start says WHY
//  the file was refused instead of the generic "missing or empty".
static AnsiString gWhiteListLoadError="";
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
    //AI(ht160s-whitelist) 20260715 : WhiteList mode loads its 2D->Bin list from the local
    // file at Lot Start (LoadWhiteListFile). Give a mode-specific reason before the generic
    // 2D-data gates below so a missing/empty whitelist file is obvious.
    //AI(ht160s-whitelist) 20260727 : prefer the exact refusal recorded by LoadWhiteListFile
    // (e.g. a lot with no KYECLotID) - the generic text sends the FE hunting for a missing
    // file that is actually present but invalid.
    //AI(secs-lot-multilot) 20260730 : test the RECORDED refusal on its own, ahead of the
    // "registry is empty" test below. Every previous whitelist refusal happened inside
    // LoadWhiteListFile, which Clear()s the registry first, so GetItemCount()<=0 was always
    // true and the two conditions were effectively one. The new multi-lot refusal deliberately
    // does NOT clear (it must not destroy the declared lots), so tying the reason to an empty
    // registry would have logged it and thrown it away, letting Start proceed in WhiteList mode
    // with whatever map happened to be loaded and WhiteList.json never read. gWhiteListLoadError
    // is cleared at the top of every LoadWhiteListFile attempt and at Lot End, so it is only
    // non-empty when the LAST attempt genuinely failed.
    if(GeneralSetting.IsWhiteListSortMode() && gWhiteListLoadError!=AnsiString(""))
    {
        Reason=gWhiteListLoadError;
        return false;
    }
    if(GeneralSetting.IsWhiteListSortMode() && LotRegistry.GetItemCount()<=0)
    {
        Reason="WhiteList mode is ON but HT160S_WhiteList\\WhiteList.json is missing or empty !";
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
    //AI(machine-command-layer) 20260625 : the global GetItemCount() above only proves SOME
    //lot has 2D data, so the per-lot proof below is what actually protects the run.
    //AI(secs-lot-multilot) 20260730 : that per-lot proof used to test ONE lot - whatever
    //edLotNo happened to hold. Now that SET_LOT_INFO is additive and several lots coexist,
    //that was wrong in both directions:
    //  * FALSE PASS - the far worse one. Host declares LOT_A + LOT_B, LOT_B's WebAPI pull
    //    fails its retries, edLotNo is LOT_A, so Start is ALLOWED. Every physical LOT_B unit
    //    then misses FindByCode2D in aLoader and is either dumped SILENTLY into the Error
    //    Auto (WhiteList / skip-unknown-2D armed) or raises a blocking WAR0475 modal per IC.
    //    The only trace was MachineRun.iUnknown2D, which nothing displays.
    //  * FALSE BLOCK - a single click on the Lot list retargets edLotNo (see sgLotListClick),
    //    so inspecting another lot mid-run could refuse the next Start / One Cycle.
    //Test EVERY registered lot instead, and name the offender. Deliberately STRICTER than
    //before: a declared lot with no 2D map cannot be sorted, so starting with one is never
    //right. The reason text separates "still downloading" from "gave up" so the operator
    //knows whether to wait or to fix the data.
    {
        AnsiString MissingLot="";
        int Missing=LotRegistry.CountLotsWithoutItems(MissingLot);
        if(Missing>0)
        {
            Reason="No 2D data for lot "+MissingLot;
            if(Missing>1)
                Reason=Reason+" (and "+IntToStr(Missing-1)+" more)";
            if(bLotApiPullAll==true || bLotApiPullActive==true)
                Reason=Reason+" : Lot WebAPI pull still in progress, wait for it to finish !";
            else
                Reason=Reason+" : load this lot's 2D/Bin before Start !";
            RecordProcess("Start refused : "+Reason);
            return false;
        }
    }
    //AI(ht160s-lotbin) 20260701 : By Lot+Bin binds (Lot,Bin)->Auto dynamically at Top
    // CCD scan time (ResolveAuto in aLoader), so a fresh work order legitimately starts
    // with ZERO bindings and self-binds first-come-first-served as ICs scan. The old
    // poka-yoke that blocked Start on GetBindingCount()<=0 contradicted this model (a
    // fresh order could never satisfy it -- binds are only made while running) and is
    // removed; the lot/2D-data gates above already guarantee routable data before Start.
    //AI(ht160s-lotpassfail) 20260730 : the old "no Pass Bin is set" gate is GONE together with
    // the operator setting it guarded. By Lot+PassFail classifies on the customer per-IC
    // DiePass now, and an IC whose DiePass is missing counts as FAIL, so no config state is
    // left that could make every IC unclassifiable. The 2D-data gates above still apply.
    return true;
}
//---------------------------------------------------------------------------
void TfMain::Start()
{
    //AI(machine-command-layer) 20260625 : operator Start button -> single MachineStart
    //gate. The modal lives on the caller side so the kernel/SECS reuse of MachineStart
    //never pops a dialog on a non-UI / receive-thread path.
    AnsiString Reason;
    if(MachineStart(trigOperator, Reason)==msRejNotReady)
        ShowMyMessage(LangT(Reason));
}
//---------------------------------------------------------------------------
//AI(machine-command-layer) 20260625 : the arm half of the old Start(). Called ONLY by
//MachineStart (csystem.cpp) after the lot/2D gate + trigger log pass. Sets the run
//latches, opens the per-IC trace batch, and home-firsts an unhomed machine (bHomeByStart
//kept : START on an unhomed machine homes then auto-runs, per 2026-06-25 decision).
void TfMain::DoStartArm()
{
//        CheckContinusStartIsReady();                                            //Sam 20240710 : StartMode exception handling
//
//        RecordProcess moved into MachineStart() : logs "MACHINE START by <trig>"
        tSimuData.bRunSimulation=cbEnableSimulation->Checked;                   //sync simulation IC flag from UI at lot start
//
        //AI(secs-ceid-align9045) 20260729 : re-enabled. HT9045 main.cpp:28819-28828 splits the
        //start event by whether the machine still holds material : CEID 76 "Start Pressed HasIC"
        //when HasICUnderMachine(), else CEID 1 "Start Pressed". The KYEC host binds 12 reports to
        //each of those two ids and subscribes x7, so leaving this commented meant the host's
        //start anchor never arrived. HasICUnderMachine() is real on HT160S (csystem.cpp:1859,
        //scans every VMotor's HasIC()). This is the arm path, so a start rejected by the lot/2D
        //gate still reports nothing - same as HT9045, which also fires from the arm half only.
        if(HasICUnderMachine())
            EventReport(SECS_EVENT.DoStartHasIC);
        else
            EventReport(SECS_EVENT.DoStart);
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
            //AI(secs-lot-multilot) 20260730 : label with the LATCHED active lot, not the live
            //edLotNo TEdit. These four run on EVERY Start including a pause/resume, and
            //g_DeviceInfo.OnLotStart is NOT idempotent (it re-derives the Production_Log file
            //name and ClearBatch()es), so a drifted edLotNo used to split the trace into a
            //second, wrongly-named CSV on resume and discard in-flight per-IC records.
            g_DeviceInfo.OnLotStart(ActiveLotID(), Now());                           //AI(HT160S-Maintainer) 20260603 : start per-IC production trace batch
            TrayUphLog_EnsureActive(ActiveLotID());   //AI(ht160s-uph) 20260708 : 172-aligned arm-on-run (idempotent; skips if Lot Start/SECS already armed or on resume)
            g_SoterOutput.EnsureActive(ActiveLotID());
            if(LoaderModule!=NULL)
                LoaderModule->SetCurrentLotNumber(ActiveLotID());                   //AI(HT160S-Maintainer) 20260604 : P3 2D->Bin lookup keyed by lot number
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
            fHome->MarkSeenStart();   //AI(HT160S-Maintainer) 20260624 : SystemStart already true above; latch start (see uHome.cpp) for reliable kernel-side monitor close on a fault drop
        }
//        flagOneCycleTrayEnd=false;
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
    //AI(secs-kyec-rcmd4) 20260728 : operator escape for the SECS host panel override
    //(S2F41 PP_SIGNALTOWER / PP_MUSIC). MANDATORY, not optional : HT160 has NO global ALARM
    //RESET handler - the key is read only inside TfNote::ScanKey and TMyMessageBox::ScanKey,
    //both dialog-scoped - and the KYEC trigger fires while the machine is merely IDLE with no
    //dialog up. Without this rung an operator would have no way to silence a host-driven
    //buzzer.
    //Placed BEFORE the screen/interlock guard block below ON PURPOSE : those guards return
    //early for fNote / MyMessageBox (both ShowModal, so their own ScanKey owns the key and
    //also releases the override), but ALSO for fiosetview, for fHome->Visible - a NON-modal
    //Show - and for IsSafeLock(). The last two keep MainProc spinning and would keep the
    //override sounding with no escape.
    //AI(secs-audit-fix) 20260729 : the fiosetview case is NOT reachable from here at all and the
    //old note claiming "MainProc already returned, override inert" had it backwards - MainProc's
    //early return skips this whole function AND DoSystemMessage, so the SwMusic outputs kept
    //their last value and an armed override stayed audible with no escape. That is now handled
    //at the source : MainProc releases the override and calls CloseBuzzerOff() when it suspends
    //the spin for the IO Set View (csystem.cpp).
    //Fires ONLY when an override is actually armed, so with no override the behaviour is
    //bit-identical to today (this key does nothing in ScanPanelKeys at present).
    //MainProc calls ScanPanelKeys() immediately before DoSystem() -> DoSystemMessage(), so
    //the release is honoured on this same scan.
    static bool bWasSecsReset=false;
    bool bSecsReset = HSys.Sen.SnFKAlarmReset.IsOn() || HSys.Sen.SnRKAlarmReset.IsOn();
    if(bSecsReset && bWasSecsReset==false && IsSecsPanelOverrideActive())
    {
        ClearSecsPanelOverride();
        CloseBuzzerOff();
        RecordProcess("ALARM RESET pressed : SECS host panel override released");
        EventReport(SECS_EVENT.DoAlarmReset);
    }
    bWasSecsReset=bSecsReset;

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
        if(ShowMyMessageBox_YES_NO(LangT("Confirm Clean Out?"))==TMyMessageBox::msgrtnYES)
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

//AI(HT160S-Maintainer) 20260622 : VCL modal z-order fix. The config/Teach/Maintenance
//sub-screens are shown with ShowModal() (see ShowTopForm). When the operator switches to
//the desktop and back, Windows raises the main form above the open modal child; because
//the main form is modally disabled, the child is hidden behind it and nothing is
//clickable - it looks hung. On app re-activation, re-front the active (modal) form so it
//returns above the main window. No-op when no modal child is up (ActiveForm==fMain).
//HT172 shares the same ShowModal pattern with no such guard; this is HT160 hardening.
void __fastcall TfMain::AppActivate(TObject *Sender)
{
    (void)Sender;
    if(Screen->ActiveForm != NULL && Screen->ActiveForm != this)
        Screen->ActiveForm->BringToFront();
}
//---------------------------------------------------------------------------
//AI(ht160s-statusbar) 20260624 : owner-draw the emsSim panel in red. Only the SIM
//panel is psOwnerDraw (set in main.dfm), so this fires for it alone; default text
//color is used for every other (psText) panel. Compile-time SIMULATE indicator -
//the panel Text is set ONLY under #ifdef SOFT_SIMULATE (ctor), so on a real build
//the panel is empty and this draws nothing.
void __fastcall TfMain::stbMainDrawPanel(TStatusBar *StatusBar, TStatusPanel *Panel,
    const TRect &Rect)
{
    if(StatusBar==NULL || Panel==NULL)
        return;
    StatusBar->Canvas->Font->Color=clRed;
    StatusBar->Canvas->Font->Style=TFontStyles()<<fsBold;
    StatusBar->Canvas->TextRect(Rect, Rect.Left+4, Rect.Top+2, Panel->Text);
}
//---------------------------------------------------------------------------
//AI(ht160s-statusbar) 20260624 : HT172 MyFunctionB::Update() analog. Source the
//identity from GeneralSetting (single source of truth), mirror into the cmydef as*
//globals for HT172 API parity, then write stbMain panels 1-3. Defined here (not
//cmydef.cpp) so the low-level cmydef TU need not pull main.h's include graph.
void UpdateMachineIdentity()
{
    asModel=GeneralSetting.sMachineModel;
    asHandlerID=GeneralSetting.sHandlerID;
    asSerialNo=GeneralSetting.sSerialNo;
    if(fMain!=NULL && fMain->stbMain!=NULL)
    {
        fMain->stbMain->Panels->Items[emsModel]->Text=asModel;
        fMain->stbMain->Panels->Items[emsHandlerID]->Text=asHandlerID;
        fMain->stbMain->Panels->Items[emsSerialNo]->Text=asSerialNo;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMain::FormShow(TObject *Sender)
{
    (void)Sender;
    //AI(ht160s-statusbar) 20260624 : port of HT172 main.cpp:315 - register the clock
    //panel with the time-string subsystem so TFormSysTools::RefreshMyTimeString()
    //(driven 1 Hz off TDataModule1::Timer1) fills it. Also push current identity into
    //panels 1-3 (HT172 MyFunctionB::Update analog). Registration de-dups by pointer.
    if(stbMain!=NULL && FormSysTools!=NULL)
        FormSysTools->AddMyTimeStringShow((TObject *)stbMain->Panels->Items[emsTime], 0);
    UpdateMachineIdentity();
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

    //AI(ht160s-agv) 20260623 : populate the sim AMR per-zone max-tray grid from GeneralSetting.
    if(sgSimMaxTray!=NULL)
    {
        const char *ZoneName[9]={"Loader","Empty","Color","Auto1","Auto2","Auto3","Auto4","Auto5","Auto6"};
        sgSimMaxTray->ColWidths[0]=110;
        sgSimMaxTray->ColWidths[1]=70;
        sgSimMaxTray->Cells[0][0]=LangT("Zone");
        sgSimMaxTray->Cells[1][0]=LangT("MaxTray");
        for(int i=0;i<9;i++)
        {
            sgSimMaxTray->Cells[0][i+1]=LangT(ZoneName[i]);
            sgSimMaxTray->Cells[1][i+1]=IntToStr(GeneralSetting.iSimAmrMaxTray[i]);
        }
    }

    //AI(ht160s-language) 20260626 : load dictionary + apply persisted UI language.
    if(fLan != NULL)
    {
        fLan->LoadDictionary();
        fLan->ChangeLanguage(this);
    }
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
        ShowMyMessage(LangT("Please add at least one Lot to the list !"));
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

    LotStartCore(FirstLot, "pressed");
}
//---------------------------------------------------------------------------
//AI(secs-lot-additive) 20260730 : modal-free Lot-Start body, shared by the operator
//button above and the SECS S2F41 LOTSTART handler (SecsGem\uHGemHT160.cpp). Everything
//from "a lot is now open" onward lives here so the two entries cannot drift : the SECS
//path used to hand-roll a SUBSET of this list (no LotBinBinding clear, no Lot-Start-time
//latch, no CEID 6). Lot REGISTRATION deliberately stays with the caller - the button
//pushes the grid rows, and on the SECS side SET_LOT_INFO is the only lot-identity setter,
//so LOTSTART starts whatever the registry already holds. Never shows a modal : the SECS
//path runs on the HSMS receive thread and a popup there stalls the link.
//AI(secs-lot-multilot) 20260730 : the active lot, for everything that needs ONE work-order
//label. Prefers the value latched at Lot Start; falls back to edLotNo so paths that run
//before any Lot Start (a bare machine Start, a boot restore whose latch is empty) keep the
//old behaviour instead of suddenly labelling with "".
//---------------------------------------------------------------------------
//AI(secs-operatorid) 20260803 : SVID/ECID 1007 Operator ID - slot 2 of the KYEC host's
//RPTID 502. GeneralSetting.sOperatorID is the CANONICAL store and SVID/ECID 1007 bind
//straight to it (uHGemHT160.cpp), the same no-snapshot shape as sHandlerID / SVID 1002.
//Nothing in the SECS layer reads this TEdit; these two helpers only keep the visible
//field and the store agreed in both directions.
//  LoadOperatorIDToDisplay() : store -> edit. Boot, and after a host S2F15 write, so the
//                              operator SEES what CJ_EAP pushed (KYEC writes "AGV").
//  CommitOperatorID()        : edit -> store + General.ini. A blank field collapses back
//                              to "Operator" so the host never reads A[0] "".
void __fastcall TfMain::LoadOperatorIDToDisplay()
{
    if(GeneralSetting.sOperatorID.Trim()==AnsiString(""))
        GeneralSetting.sOperatorID="Operator";
    if(edtSysOperatorID==NULL)
        return;
    if(edtSysOperatorID->Text!=GeneralSetting.sOperatorID)
        edtSysOperatorID->Text=GeneralSetting.sOperatorID;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::CommitOperatorID()
{
    AnsiString sNew;

    if(edtSysOperatorID==NULL)
        return;
    sNew=edtSysOperatorID->Text.Trim();
    if(sNew==AnsiString(""))
        sNew="Operator";
    if(edtSysOperatorID->Text!=sNew)
        edtSysOperatorID->Text=sNew;
    if(GeneralSetting.sOperatorID==sNew)
        return;                     // unchanged -> no disk write, no log line
    GeneralSetting.sOperatorID=sNew;
    GeneralSetting.Save();
    RecordProcess(AnsiString("Operator ID set to ")+sNew);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::edtSysOperatorIDExit(TObject *Sender)
{
    (void)Sender;
    CommitOperatorID();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::edtSysOperatorIDMouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
    (void)Sender;
    (void)Button;
    (void)Shift;
    (void)X;
    (void)Y;
    //The panel has no physical keyboard, so the field is filled through the same
    //on-screen QWERTY the account page uses (maintenance.cpp edPwId). HT9045 hooks
    //its own edtSysOperatorID on OnMouseUp the same way (uLotInfo.dfm).
    if(fQwertyKey==NULL || edtSysOperatorID==NULL)
        return;
    if(fQwertyKey->ShowQwertyKey(edtSysOperatorID, N_NO_SPACE, 0, false, 0, 0,
           LangT("Operator ID"))==false)
        return;
    CommitOperatorID();
}
//---------------------------------------------------------------------------
AnsiString __fastcall TfMain::ActiveLotID()
{
    if(m_sActiveLot.Trim()!=AnsiString(""))
        return m_sActiveLot;
    return (edLotNo!=NULL) ? edLotNo->Text.Trim() : AnsiString("");
}
//---------------------------------------------------------------------------
void __fastcall TfMain::LotStartCore(AnsiString FirstLot, AnsiString Origin)
{
    //AI(ht160s-lotbin) 20260615 : By Lot+Bin mode. A fresh Lot Start clears all
    //dynamic (Lot,Bin)->Auto bindings so Autos are re-bound first-come-first-served
    //for this work order. (Mid-lot RESTART resumes via the machine START button,
    //which does NOT clear - bindings are restored in RestoreLastWorkOrder. The
    //future "inherit last record?" prompt will gate this clear for that path.)
    LotBinBinding.Clear();
    LotBinBinding.SaveToIni();
    //AI(ht160s-whitelist-override) 20260717 : persist the per-lot WhiteList overlay with the work
    //order so a mid-lot restart resumes in the same mode (mirrors the LotBinBinding lifecycle). The
    //overlay was armed/cleared beforehand via the maintenance panel or a SECS SORTMODE pair.
    GeneralSetting.SaveWhiteListOverlay();
    UpdateSortModeFeatureBadge();

    //AI(ht160s-lot-reset) 20260706 : a fresh Lot Start zeroes the per-run production
    //counters (Auto Cnt display, UPH, SECS Scanned/Sorted/TotalIC) so they represent
    //THIS work order instead of accumulating across lots. Machine-total cumulative
    //fields are untouched. Must run before bRunning/iActiveLotCount are set below.
    ResetPerLotProductionCounters();
    //AI(ht160s-uph) 20260706 : open this work order's per-tray/lot UPH log folder.
    TrayUphLog_OnLotStart(FirstLot);
    g_SoterOutput.OnLotStart(FirstLot);
    ClearProductInfoAtLotStart();

    MachineRun.bRunning=true;
    MachineRun.iActiveLotCount=LotRegistry.GetLotCount();

    //AI(secs-lot-additive) 20260730 : NULL-guarded now that the SECS LOTSTART path calls this.
    //This is the one place the ACTIVE lot is (re)targeted - a lot list refresh must not do it.
    //AI(secs-lot-multilot) 20260730 : LATCH it. Everything that needs one work-order label reads
    //ActiveLotID() from here on, so a later click / keystroke in edLotNo cannot move the running
    //work order. edLotNo is still set for the operator and for the 2D editor.
    m_sActiveLot=FirstLot;
    if(edLotNo!=NULL)
        edLotNo->Text=FirstLot;
    //AI(ht160s-lot-webapi) 20260612 : Stage 4 : at lot start, pull EVERY lot's
    // 2D/Bin data from the customer WebAPI (async, no modal). Shared helper so the
    // SECS S2F42 LOTSTART handler pulls every lot too (not just the first).
    //AI(ht160s-whitelist) 20260715 : WhiteList mode substitutes the local WhiteList.json
    // for the WebAPI pull (same LotRegistry, same downstream). See LoadWhiteListFile.
    //AI(ht160s-whitelist) 20260727 : a REJECTED whitelist file leaves the registry empty ON
    // PURPOSE. Saving that would overwrite WorkOrder.json with an empty order and destroy the
    // power-on restore point over a fixable typo in WhiteList.json. LoadWhiteListFile already
    // saves on the valid path, so skip the unconditional save when it refused the file.
    bool bWhiteListOk=true;
    if(GeneralSetting.IsWhiteListSortMode())
    {
        //AI(secs-lot-multilot) 20260730 : WhiteList is a SINGLE-lot overlay and its file is
        //authoritative - LoadWhiteListFile Clear()s the whole registry before it even checks the
        //file exists, and it stamps EVERY IC in the file with ONE expected KYEC lot (ActiveLotID).
        //With SET_LOT_INFO now additive, a host that declared several lots and armed the overlay
        //would have the extra lots wiped with no log line, or have a perfectly valid file rejected
        //because its per-lot KYECLotID cannot equal one stamp. Refuse BEFORE the Clear, with a
        //reason CheckLotDataReady will show, instead of silently discarding declared work. This
        //mirrors the mid-lot refusal the SECS LOTSTART handler already answers HCACK=4 for.
        if(LotRegistry.GetLotCount()>1)
        {
            gWhiteListLoadError="WhiteList mode supports ONE lot ; "
                                +IntToStr(LotRegistry.GetLotCount())
                                +" lots are registered. Remove the extra lots or turn WhiteList off.";
            g_EventLog.Log("WRN_WHITELIST_REJECT", gWhiteListLoadError);
            RecordProcess("WhiteList: "+gWhiteListLoadError);
            bWhiteListOk=false;
        }
        else
            bWhiteListOk=LoadWhiteListFile();
    }
    else
        StartLotWebApiPullAll();
    //AI(HT160S-Maintainer) 20260608 : need1 : persist the started work order so
    //the next power-on can restore it (see RestoreLastWorkOrder / FormShow).
    if(bWhiteListOk)
        SaveWorkOrder();
    RecordProcess("LOT START "+Origin);

    //AI(secs-lotstarttime) 20260730 : latch SVID 1009 Lot Start Time BEFORE the event, so a
    // host that answers CEID 6 with an immediate S1F3 already reads THIS lot's start time.
    //AI(secs-lotstarttime-persist) 20260730 : format the stamp ONCE and use the same string for
    // the SV and for the work-order meta file, so the two can never disagree. Persisting it is
    // what lets an inherited work order answer 1009 after a power cycle instead of "".
    AnsiString sLotStartStamp=Now().FormatString("yyyy/mm/dd hh:nn:ss");
    NoteLotStartTime(true, sLotStartStamp);
    SaveWorkOrderLotStartTime(sLotStartStamp);
    //AI(ht160s-secsgem) 20260714 : notify host a new lot has started (S6F11 CEID 6 Lot Start).
    // Pairs with the Lot End event in DoLotEndProcess. Report 1 carries the new Current
    // Lot ID. Registry is already populated (GetLotListCount()==0 returned early above),
    // so no guard is needed. EventReport self-gates on USE_SECS_GEM + HSMS SELECTED.
    //AI(secs-ceid-align9045) 20260730 : this comment said "CEID 11" and named btnLotEndClick
    // before the 20260729 HT9045 renumber. SECS_EVENT.DoLotStart has been 6 ever since, and the
    // Lot End button routes through DoLotEndProcess. Code was always right; the comment was stale.
    EventReport(SECS_EVENT.DoLotStart);
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
//AI(ht160s-whitelist) 20260715 : WhiteList mode 2D->Bin source. In place of the WebAPI pull,
// load HT160S_WhiteList\WhiteList.json into LotRegistry through the SAME parser the WebAPI uses
//AI(ht160s-whitelist) 20260727 : the file must be in the CUSTOMER schema (root QRCodeIDHis /
// 2DIDHistory, per-lot LOTID + KYECLotID) - ValidateWhiteListJson refuses anything else,
// including the legacy "Maps" import shape this comment used to name.
// (LoadFromJsonString). Clear() FIRST so the local file is AUTHORITATIVE : only
// listed codes become routable, and any boot-restored / stale WorkOrder 2D data cannot leak
// through in WhiteList mode. On success mirror PollLotDataWebApi (RefreshLotListFromRegistry +
// SaveWorkOrder). Runs on the same VCL thread as the WebAPI helpers; never shows a modal.
bool __fastcall TfMain::LoadWhiteListFile()
{
    AnsiString fn = HSys.CurrentDir + "\\HT160S_WhiteList\\WhiteList.json";
    ForceDirectories(ExtractFilePath(fn));   // guide the FE : create the folder if absent
    gWhiteListLoadError="";                  // re-armed below on every failure path
    //AI(ht160s-whitelist) 20260715 : Clear FIRST - unconditionally, BEFORE the file checks.
    // WhiteList is authoritative: if the file is missing/unreadable the registry MUST end up
    // EMPTY so CheckLotDataReady blocks Start with the right reason, and NO boot-restored /
    // stale WorkOrder 2D data can survive to be mis-sorted as if whitelisted. (Only reached in
    // WhiteList mode at Lot Start, where clearing the registry is the intended fresh-lot action.)
    LotRegistry.Clear();
    if(!FileExists(fn))
    {
        gWhiteListLoadError="WhiteList file not found : "+fn;
        //AI(ht160s-whitelist-kyec) 20260727 : a rejected WhiteList file is a HANDLED,
        // operator-actionable outcome (production Start is blocked with this exact reason),
        // NOT a software fault. Record every rejection in the EventLog so the FE/operator sees
        // it was detected and handled, never mistaken for the software silently misbehaving.
        g_EventLog.Log("WRN_WHITELIST_REJECT", gWhiteListLoadError);
        RecordProcess("WhiteList: file missing - "+fn);
        RefreshLotListFromRegistry();         // reflect the now-empty registry in the UI
        return false;
    }
    AnsiString text="";
    TStringList *raw = new TStringList;
    try { raw->LoadFromFile(fn); text = raw->Text; }
    catch(...)
    {
        delete raw;
        gWhiteListLoadError="WhiteList file cannot be read : "+fn;
        g_EventLog.Log("WRN_WHITELIST_REJECT", gWhiteListLoadError);
        RecordProcess("WhiteList: read failed - "+fn);
        RefreshLotListFromRegistry();
        return false;
    }
    delete raw;

    //AI(ht160s-whitelist) 20260727 : pre-flight the CUSTOMER contract before the shared parser
    // touches anything. KYECLotID (and LOTID) are mandatory per lot - KYECLotID is the KYEC
    // batch identity the customer file declares for the run, so a whitelist lot without it is
    // an unidentifiable batch. The shared parser cannot carry this rule (it also serves the
    // WebAPI pull and the boot restore, where the field is legitimately absent), and it returns
    // true for any parseable JSON - including a typo'd root key, which would otherwise log
    // "loaded" with an empty registry. All-or-nothing, matching the authoritative-list semantic
    // above : the first offending lot rejects the WHOLE file and the registry stays empty, so
    // Start is blocked with the exact reason instead of running a partially-identified batch.
    //AI(ht160s-whitelist-kyec) 20260727 : the KYEC lot the registry keys on (Soter col7) is the
    // Lot Start lot held in edLotNo (SECS LOTSTART / manual Lot Start, both set before this runs).
    // Capture it up here so (a) the pre-flight validator can reject a file whose per-lot KYECLotID
    // disagrees with it - otherwise a manual operator who typed the customer lot would silently sort
    // under it and drop the KYEC identity from the report - and (b) the loader below stamps it as
    // the registry key, exactly like the WebAPI pull (LoadFromJsonString(Body, ..., sLotApiPullLot)).
    //AI(secs-lot-multilot) 20260730 : the LATCHED active lot, not the live TEdit. This value
    //becomes the registry KEY for every IC in the file (LoadFromJsonString stamp) AND the
    //expected-KYEC value the validator compares each lot against, so a drifted edLotNo would
    //re-key the whole routing table or reject a valid file outright.
    AnsiString sKyecStamp = ActiveLotID();
    AnsiString vReason="";
    if(!LotRegistry.ValidateWhiteListJson(text, vReason, sKyecStamp))
    {
        gWhiteListLoadError=vReason;
        g_EventLog.Log("WRN_WHITELIST_REJECT", gWhiteListLoadError);
        RecordProcess("WhiteList: "+vReason+" - "+fn);
        RefreshLotListFromRegistry();
        return false;
    }

    // sKyecStamp (the KYEC lot) was captured + cross-checked against every lot's KYECLotID above ;
    // stamp it so the registry keys on the KYEC lot (Lot->sLotID = Soter col7) while each IC's
    // sCustLotID = the file's customer LOTID (col6) - the 1-KYEC-lot : N-customer-lot shape the
    // WebAPI pull already produces.
    bool bDup=false; AnsiString dupCode="";
    bool ok = LotRegistry.LoadFromJsonString(text, bDup, dupCode, sKyecStamp);
    RefreshLotListFromRegistry();
    if(ok)
    {
        gWhiteListLoadError="";
        SaveWorkOrder();                       // persist only a VALID whitelist load
        RecordProcess("WhiteList loaded: "+fn+"  (2D="+IntToStr(LotRegistry.GetItemCount())+")");
        if(bDup)
            RecordProcess("WhiteList duplicate 2D ignored: "+dupCode);
    }
    else
    {
        gWhiteListLoadError="WhiteList file is not valid JSON : "+fn;
        g_EventLog.Log("WRN_WHITELIST_REJECT", gWhiteListLoadError);
        RecordProcess("WhiteList: JSON parse failed - "+fn);
    }
    return ok;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::StartLotWebApiPullAll()
{
    EnsureLotWebApiClientCreated();
    if(LotWebApiClient!=NULL && LotWebApiClient->GetUsePull())
    {
        //AI(secs-lot-additive) 20260730 : do NOT restart a sweep that is still running. The host
        //may now send LOTSTART repeatedly (that is how it refreshes 2D/Bin data mid-lot), and each
        //call used to slam the cursor back to 0 while a request was in flight : StartNextLotApiPull
        //would target slot 0, RequestLotDataFromWebApi would find the single-request client busy and
        //drop it with NO retry armed, and the in-flight response would then advance the cursor past
        //slot 0 - so that lot silently finished the sweep with zero 2D items and every one of its
        //units would route to the Error Auto. A sweep already in flight will visit every slot anyway.
        if(bLotApiPullAll==true)
        {
            RecordProcess("Lot WebAPI pull-all already in flight; new request folded into it");
            return;
        }
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
        int iLotsBefore=LotRegistry.GetLotCount(); /*AI(ht160s-lot-webapi) 20260715: snapshot before parse*/ if(LotRegistry.LoadFromJsonString(Body, bDuplicate, DupCode, sLotApiPullLot)) /*AI(ht160s-kyec) 20260722: stamp KYEC lot -> ICs register under it, customer lot kept per-IC + upsert latest-wins*/
        {
            RefreshLotListFromRegistry();
            SaveWorkOrder();
            if(LotRegistry.GetLotCount()==iLotsBefore) RecordProcess("Lot WebAPI parsed 0 lots (schema mismatch?): "+sLotApiPullLot); else RecordProcess("Lot WebAPI data loaded: "+sLotApiPullLot); /*AI(ht160s-lot-webapi) 20260715: break silent-empty parse*/
            if(bDuplicate==true)
                RecordProcess("Lot WebAPI duplicate 2D ignored: "+DupCode);
            //AI(ht160s-kyec) 20260722 : re-pull latest-wins : loud record of how many existing
            //2D codes were refreshed in place to the newest WebAPI data.
            if(LotRegistry.GetRefreshCount()>0)
                RecordProcess("Lot WebAPI refreshed "+IntToStr(LotRegistry.GetRefreshCount())+" existing 2D codes to latest data: "+sLotApiPullLot);
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
//AI(ht160s-ftp) 20260721 : pump background FTP upload results into the EventLog.
// Called every MainProc cycle (VCL main thread via TRunControl::Synchronize), the
// same safe spot as PollLotDataWebApi. Cheap no-op when the queue is empty. Only
// LOT_PUBLISH ok / give-up messages reach here; maintenance test-button results
// stay on the maintenance screen. The per-cycle guard caps the drain so a flood of
// queued results cannot stall one MainProc tick.
void __fastcall TfMain::PollFtpUploadResults()
{
    if(FtpUploadThd == NULL)
        return;
    AnsiString sMsg;
    int iGuard = 0;
    while(iGuard < 20 && FtpUploadThd->FetchResult(sMsg))
    {
        g_EventLog.Log("FTP_UPLOAD", sMsg, "");
        iGuard++;
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-2dbin-manual) 20260628 : forward decl : the WorkOrder.json path helper
//is defined further down (near GetLastLotListFileName) but used here on Lot End.
static AnsiString GetWorkOrderFileName();
//---------------------------------------------------------------------------
void __fastcall TfMain::btnLotEndClick(TObject *Sender)
{
    //AI(ht160s-overcount-tripqueue D3) 20260721 : thin shell. The Lot-End body moved to
    //DoLotEndProcess() so the CleanOut-finish path (csystem) can auto-run the identical
    //sequence (CEID12 + work-order / LotBinBinding clear). Button behaviour byte-unchanged.
    (void)Sender;
    DoLotEndProcess();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::DoLotEndProcess()
{
    //AI(HT160S-Maintainer) 20260604 : P1 stop the sort run (HT172 LotEnd analog).
    if(HSys.Sys.SystemStart==true)
    {
        RecordProcess("LOT END pressed");
        HSys.Sys.SystemStart=false;
    }
    SoftStop=true;
    MachineRun.bRunning=false;

    //AI(ht160s-whitelist-override) 20260717 : revert the per-lot WhiteList overlay to the base sort
    //mode as the lot closes. Placed EARLY (right after bRunning clears, before any file I/O or CEID
    //emit that could pump the message loop) so a host LOTSTART(WHITELIST) for the NEXT lot is not
    //clobbered by this revert - arm(N+1) then lands after disarm(N). Persist cleared + refresh badge.
    GeneralSetting.SetWhiteListActive(false);
    GeneralSetting.SaveWhiteListOverlay();
    UpdateSortModeFeatureBadge();
    //AI(ht160s-whitelist) 20260727 : drop the last file-refusal reason with the lot. It
    //described THIS lot's WhiteList.json; keeping it would let a later Start quote a
    //refusal for a file the FE has since fixed (the next Lot Start re-reads the file and
    //sets a fresh reason if it is still bad).
    gWhiteListLoadError="";

    //AI(ht160s-uph) 20260706 : record this lot's total UPH (HT172 parity : aggregate
    //TotalIC / productive-hours) to the EventLog + per-lot UPH summary before the work
    //order is cleared, then persist the final counts (item 7 lastdata).
    tRunData.LotEndTime=Now();
    tRunData.UPH=GetCalculateUPH(tRunData.LotEndTime);
    //AI(secs-lot-multilot) 20260730 : ActiveLotID(), so the closing record names the lot the
    //work order was OPENED with even if the operator has since typed in / clicked another lot.
    //NOTE the TotalIC / UPH numbers are work-order-wide (tRunData.TotalIC counts every placed
    //IC with no lot in scope), so with several lots this row credits them all to the opening
    //lot. That is pre-existing and log-only - no reader in the tree - and per-lot accounting
    //would be a new feature, not part of this fix.
    RecordProcess("End of Lot: Lot="+ActiveLotID()+", TotalIC="+IntToStr(tRunData.TotalIC)+", UPH="+IntToStr(tRunData.UPH));
    TrayUphLog_OnLotEnd(ActiveLotID(), tRunData.TotalIC, tRunData.UPH);
    g_SoterOutput.OnLotEnd();
    FreezeProductInfoAtLotEnd();
    WriteLastDataIni();

    //AI(ht160s-secsgem) 20260714 : notify host this lot has ended (S6F11 CEID 8 Lot End).
    // Fire BEFORE LotRegistry.Clear() below so report 1's snapshot still carries the
    // ending lot (Current Lot ID / Total IC / UPH). EventReport self-gates on
    // USE_SECS_GEM + HSMS SELECTED (no-op when SECS off or link down). The lot-count
    // guard mirrors HT9045's bLotStart gate : an empty Lot End press sends nothing.
    //AI(secs-ceid-align9045) 20260730 : this comment said "CEID 12" before the 20260729 HT9045
    // renumber; SECS_EVENT.DoLotEnd has been 8 ever since. Code unchanged, comment corrected.
    if(LotRegistry.GetLotCount()>0)
        EventReport(SECS_EVENT.DoLotEnd);
    //AI(secs-lotstarttime) 20260730 : clear SVID 1009 AFTER the Lot End event, so the lot's
    // start time stays readable right up to the moment the work order closes. Unconditional
    // (NOT under the lot-count guard): an empty Lot End press must not leave a stale timestamp.
    NoteLotStartTime(false);

    //AI(ht160s-lot-webapi) 20260612 : stop any in-flight "pull all lots" sweep so
    // it does not walk the registry we are about to clear.
    bLotApiPullAll=false;
    iLotApiPullCursor=0;
    iLotApiRetryCount=0;

    //AI(ht160s-lot-webapi) 20260722 : also abort a SINGLE pull already on the wire.
    // Clearing bLotApiPullAll (above) is not enough : PollLotDataWebApi's own guard is
    // bLotApiPullActive, not bLotApiPullAll, so a sweep pull still in flight when Lot End
    // fires would land in the lower half, LoadFromJsonString the response into the registry
    // we Clear() just below, and SaveWorkOrder re-write WorkOrder.json - re-adding the lot the
    // operator just ended. Drop the active flag AND Cancel() the client (closes the socket,
    // returns iState to IDLE) so the stale response is never consumed, and so the next Lot
    // Start's pull is not blocked by a still-"busy" client. Guarded on bLotApiPullActive : the
    // maintenance manual fetch (bLotApiResultPending) uses the same single-request client but is
    // display-only (never touches LotRegistry), and cannot be in flight at the same time as a
    // production pull, so this leaves that diagnostic path alone.
    //AI(secs-lot-additive) 20260730 : same hunk as fix/lot-end-inflight-webapi-cancel (22d1ac1),
    // brought onto this branch because the new host lot lifecycle makes the window ROUTINE rather
    // than an operator slip : LOTSTART is now the host's mid-lot 2D refresh and CLEAR_LOT_INFO is
    // the host lot end, so "refresh then clear" inside the client's 8 s request window is a normal
    // host sequence - and the host would get HCACK=0 for a clear that then silently un-did itself.
    if(bLotApiPullActive==true)
    {
        bLotApiPullActive=false;
        if(LotWebApiClient!=NULL)
            LotWebApiClient->Cancel();
    }

    //AI(ht160s-lot-webapi) 20260612 : Lot End clears the whole work order so the
    // next lot starts clean. Clear() drops every Lot slot, the 2D-code index and
    // all per-IC 2D/Bin records. Then blank the active Lot No, repaint the grid
    // (RefreshLotListFromRegistry blanks every row when the registry is empty),
    // and overwrite system\LastLotList.ini with the now-empty list so a restart
    // does NOT restore the finished lots.
    ArchiveWorkOrderToLotStory();
    LotRegistry.Clear();
    //AI(ht160s-lotbin) 20260615 : drop all (Lot,Bin)->Auto bindings on Lot End so the
    //next work order starts with a clean dynamic table (also persisted empty).
    LotBinBinding.Clear();
    LotBinBinding.SaveToIni();
    //AI(secs-lot-multilot) 20260730 : release the active-lot latch with the work order, so the
    //next Lot Start re-latches instead of inheriting the finished lot's name.
    m_sActiveLot="";
    if(edLotNo!=NULL)
        edLotNo->Text="";
    RefreshLotListFromRegistry();
    DeleteFile(GetWorkOrderFileName());
    //AI(secs-lotstarttime-persist) 20260730 : the meta file follows WorkOrder.json exactly, so a
    //finished lot's start time cannot be re-latched by a later restore.
    ClearWorkOrderMeta();
    RecordProcess("Lot data cleared (Lot End)");
}
//---------------------------------------------------------------------------
//AI(ht160s-overcount-tripqueue D3) 20260721 : emit S6F11 CEID42 "Clean Out Finish"
//(was defined but never sent). Called from csystem's CleanOut-finish BEFORE the Lot End
//AI(secs-comment-truth) 20260805 : was "CleanOutOK(28) then PressLotEnd(12)". Both numbers
//were from the pre-alignment dictionary. It is Clean Out Finish = CEID 42 then Lot End =
//CEID 8; in the HT9045-aligned dictionary we now use, 28 is Retry Pressed and 12 is Switch
//Engineer Mode.
//so the host sees Clean Out Finish (42) then Lot End (8). EventReport self-gates on
//USE_SECS_GEM + HSMS SELECTED, so this is a no-op when SECS is off / link is down.
void __fastcall TfMain::EmitCleanOutOK()
{
    EventReport(SECS_EVENT.CleanOutFinish);
}
//---------------------------------------------------------------------------
//AI(secs-kyec-rcmd4) 20260728 : emit S6F11 CEID41 "One Cycle Finish", the OneCycle twin of
//EmitCleanOutOK above. The id was 27 until the 20260729 HT9045 alignment renumbered it to 41.
//loop but sent from nowhere, so a host driving S2F41 ONE_CYCLE never learned the cycle ended.
//Called from the OneCycle-finish dispatcher in csystem.cpp.
//EventReport self-gates on USE_SECS_GEM + HSMS SELECTED, so this is a no-op when SECS is off.
void __fastcall TfMain::EmitOneCycleOK()
{
    EventReport(SECS_EVENT.OneCycleFinish);
}
//---------------------------------------------------------------------------
//AI(secs-ceid-align9045) 20260729 : emit S6F11 CEID27 "Change Machine State". Port of
//HT9045 main.cpp:18486, which fires whenever palMainStatus->Caption changes. HT160S has the
//same single status writer (csystem.cpp SetMainStatus), so the change edge is detected there
//and routed here (csystem.cpp cannot see EventReport). This was the single highest-traffic
//event on the KYEC floor (406 S6F11/day on the 9045) and HT160S had no emit site for it.
//EventReport self-gates on USE_SECS_GEM + HSMS SELECTED, so this is a no-op when SECS is off.
void __fastcall TfMain::EmitRunStatusChange()
{
    EventReport(SECS_EVENT.RunStatus);
}
//---------------------------------------------------------------------------
//AI(secs-ceid-align9045) 20260729 : emit S6F11 CEID123 "Safe Door On Off". Port of HT9045
//csystem.cpp:2775, which fires on ANY safety-door sensor edge (open or close), independent
//of whether the machine is running. Edge tracking lives in csystem ReportSafeDoorChangeToSecs.
void __fastcall TfMain::EmitSafeDoorChange()
{
    EventReport(SECS_EVENT.SafeDoorOnOff);
}
//---------------------------------------------------------------------------
//AI(secs-ceid-align9045) 20260729 : CEID 21 "Enter IO Page" (HT9045 main.cpp:24931). The
//HT160S I/O view is opened from maintenance.cpp, which does not include the SECS headers,
//so the report is routed through here like EmitCleanOutOK/EmitOneCycleOK already are.
void __fastcall TfMain::EmitEnterIOPage()
{
    EventReport(SECS_EVENT.EnterIO);
}
//---------------------------------------------------------------------------
//AI(secs-ceid-align9045) 20260729 : CEID 73 "Mymessbox OK" (HT9045 mymessbox.cpp:368). HT9045
//only reports it for SECS-originated alarm boxes; HT160S has no such flag on TMyMessageBox, so
//this fires on EVERY message-box dismissal - i.e. the "message released" notification the KYEC
//host subscribed to (it saw 7 of these on the 9045 that day). Called from mymessbox FormClose
//so every dismissal path (Yes / No / Pause / Esc) is covered by one site.
void __fastcall TfMain::EmitMessageBoxClosed()
{
    EventReport(SECS_EVENT.MymessboxOK);
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
    sgLotList->ColWidths[0]=150;
    sgLotList->ColWidths[1]=70;
    sgLotList->ColWidths[2]=70;
    sgLotList->ColWidths[3]=80;
    sgLotList->Cells[0][0]=LangT("Lot No.");
    sgLotList->Cells[1][0]=LangT("Src");
    sgLotList->Cells[2][0]="2D";
    sgLotList->Cells[3][0]=LangT("Sorted");

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
//AI(ht160s-2dbin-manual) 20260628 : unified work-order persistence. WorkOrder.json
//is the single active in-execution work order (lots + their full 2D items in the
//2DIDHistory schema). It supersedes the older system\LastLotList.ini, which is now
//only a migration fallback (lots only, no 2D items) read when WorkOrder.json is
//absent. Saved on every work-order mutation, archived to LotStory on Lot End.
//---------------------------------------------------------------------------
static AnsiString GetWorkOrderFileName()
{
    return HSys.CurrentDir + AnsiString("\\HT160S_LotInfo\\WorkOrder.json");
}
//---------------------------------------------------------------------------
//AI(secs-lotstarttime-persist) 20260730 : work-order-scoped metadata that is NOT per-lot and
//therefore has no home in WorkOrder.json (whose schema is the customer 2DIDHistory shape,
//shared with the WebAPI and WhiteList parsers - adding our own root key there would leak into
//a customer-facing contract). Lives beside WorkOrder.json so it shares its exact lifecycle:
//written at Lot Start, deleted wherever WorkOrder.json is deleted, read only when the operator
//chooses to INHERIT. Currently one key : the Lot Start time behind SVID 1009.
static AnsiString GetWorkOrderMetaFileName()
{
    return HSys.CurrentDir + AnsiString("\\HT160S_LotInfo\\WorkOrder.meta.ini");
}
//---------------------------------------------------------------------------
void __fastcall TfMain::SaveWorkOrderLotStartTime(AnsiString sWhen)
{
    AnsiString fn=GetWorkOrderMetaFileName();
    ForceDirectories(ExtractFilePath(fn));
    TIniFile *Ini=new TIniFile(fn);
    try
    {
        try
        {
            // WriteString, never WriteDateTime : the value is the already-formatted
            // "yyyy/mm/dd hh:nn:ss" stamp the SV answers, and a locale short-date would reshape it.
            Ini->WriteString("WorkOrder", "LotStartTime", sWhen);
        }
        catch(...)
        {
            //AI(secs-lotstarttime-persist) 20260730 : a METADATA write must never abort a Lot
            //Start. TIniFile::WriteString raises when the folder/file is read-only, the disk is
            //full or the file is locked, and this call sits between the 1009 latch and the
            //CEID 6 emit : on the SECS path LotStartCore runs inside the S2F41 handler, so an
            //escaping exception would leave the S2F42 reply unsent (host T3 timeout) with the
            //lot already open, and on the button path it would show a raw VCL error instead of
            //a Lot Start. Degrade to "not persisted", which the restore side already tolerates
            //(no stored stamp -> 1009 stays empty).
            RecordProcess("WorkOrder meta write failed ; SVID 1009 will not survive a power cycle");
        }
    }
    __finally
    {
        delete Ini;
    }
}
//---------------------------------------------------------------------------
AnsiString __fastcall TfMain::LoadWorkOrderLotStartTime()
{
    AnsiString fn=GetWorkOrderMetaFileName();
    if(!FileExists(fn))
        return AnsiString("");
    AnsiString sWhen="";
    TIniFile *Ini=new TIniFile(fn);
    try
    {
        try
        {
            sWhen=Ini->ReadString("WorkOrder", "LotStartTime", "");
        }
        catch(...)
        {
            //AI(secs-lotstarttime-persist) 20260730 : same reasoning as the write side - this
            //runs inside the startup restore, and an unreadable meta file must degrade to
            //"no stored stamp", never abort the work-order restore.
            sWhen="";
            RecordProcess("WorkOrder meta read failed ; SVID 1009 stays empty");
        }
    }
    __finally
    {
        delete Ini;
    }
    return sWhen.Trim();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::ClearWorkOrderMeta()
{
    DeleteFile(GetWorkOrderMetaFileName());
}
//---------------------------------------------------------------------------
void __fastcall TfMain::SaveWorkOrder()
{
    AnsiString fn=GetWorkOrderFileName();
    ForceDirectories(ExtractFilePath(fn));
    LotRegistry.SaveToJsonFile(fn);
}
//---------------------------------------------------------------------------
bool __fastcall TfMain::LoadWorkOrder()
{
    AnsiString fn=GetWorkOrderFileName();
    if(!FileExists(fn))
        return false;
    bool dup=false;
    AnsiString code="";
    if(LotRegistry.LoadFromJsonFile(fn, dup, code))
    {
        RefreshLotListFromRegistry();
        return (LotRegistry.GetLotCount()>0);
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(ht160s-2dbin-manual) 20260628 : on Lot End, snapshot the finished work order
//into the monthly LotStory archive before it is cleared. Folder layout mirrors
//cCsvDailyLog exactly : HSys.LogRootDir\LotStory\<yyyy_mm> . File name is the
//first non-blank lot id (filesystem-sanitized) + a yyyymmdd_hhnnss timestamp.
void __fastcall TfMain::ArchiveWorkOrderToLotStory()
{
    if(LotRegistry.GetLotCount()<=0)
        return;

    AnsiString LotName="";
    int SlotCount=LotRegistry.GetLotSlotCount();
    for(int Index=0; Index<SlotCount; Index++)
    {
        TLotRunInfo *Lot=LotRegistry.GetLot(Index);
        if(Lot!=NULL && Lot->sLotID.Trim()!=AnsiString(""))
        {
            LotName=Lot->sLotID.Trim();
            break;
        }
    }
    if(LotName==AnsiString(""))
        return;

    // Filesystem-safe lot name : replace any of  \ / : * ? " < > |  with '_'.
    AnsiString SafeLot="";
    for(int i=1;i<=LotName.Length();i++)
    {
        char c=LotName[i];
        if(c=='\\' || c=='/' || c==':' || c=='*' || c=='?' ||
           c=='"' || c=='<' || c=='>' || c=='|')
            SafeLot+='_';
        else
            SafeLot+=c;
    }

    AnsiString Folder=HSys.LogRootDir + AnsiString("\\LotStory\\") +
        FormatDateTime("yyyy_mm", Now());
    ForceDirectories(Folder);
    AnsiString File=Folder + AnsiString("\\") + SafeLot + AnsiString("_") +
        FormatDateTime("yyyymmdd_hhnnss", Now()) + AnsiString(".json");
    LotRegistry.SaveToJsonFile(File);
}
//---------------------------------------------------------------------------
//AI(ht160s-workorder-backup) 20260630 : back up the current work order to a
//dedicated LotStory Discarded subfolder BEFORE a NON-Lot-End path clears it
//(startup fresh-start, SECS SET_LOT_INFO overwrite, manual Remove Lot). Mirrors
//ArchiveWorkOrderToLotStory but tags the file with a reason so a mis-click or host
//overwrite never loses the in-progress 2D/Bin trace.
//Return value = "safe for the caller to destroy" : true if the work order was
//archived OR there was nothing to archive; false ONLY when a real snapshot write
//failed while data existed (caller must then NOT destroy). LotBinBinding bindings
//are derived, recoverable runtime state (not persisted trace), out of scope here.
bool __fastcall TfMain::ArchiveDiscardedWorkOrder(AnsiString Reason)
{
    if(LotRegistry.GetLotCount()<=0)
        return true;                 // nothing to archive : nothing to lose

    AnsiString LotName="";
    int SlotCount=LotRegistry.GetLotSlotCount();
    for(int Index=0; Index<SlotCount; Index++)
    {
        TLotRunInfo *Lot=LotRegistry.GetLot(Index);
        if(Lot!=NULL && Lot->sLotID.Trim()!=AnsiString(""))
        {
            LotName=Lot->sLotID.Trim();
            break;
        }
    }
    if(LotName==AnsiString(""))
        return true;                 // no identifiable lot : nothing to lose

    // Filesystem-safe lot name : replace any of  \ / : * ? " < > |  with '_'.
    AnsiString SafeLot="";
    for(int i=1;i<=LotName.Length();i++)
    {
        char c=LotName[i];
        if(c=='\\' || c=='/' || c==':' || c=='*' || c=='?' ||
           c=='"' || c=='<' || c=='>' || c=='|')
            SafeLot+='_';
        else
            SafeLot+=c;
    }

    AnsiString Tag="";
    if(Reason.Trim()!=AnsiString(""))
        Tag=AnsiString("_")+Reason.Trim();
    //AI(ht160s-workorder-backup) 20260630 : bucket Discarded backups by month
    //(yyyy_mm), mirroring ArchiveWorkOrderToLotStory and the cCsvDailyLog log
    //folders, so the folder no longer grows unbounded and PruneFolderTree can
    //age whole months out.
    AnsiString Folder=HSys.LogRootDir + AnsiString("\\LotStory\\Discarded\\") +
        FormatDateTime("yyyy_mm", Now());
    ForceDirectories(Folder);
    AnsiString File=Folder + AnsiString("\\") + SafeLot + AnsiString("_") +
        FormatDateTime("yyyymmdd_hhnnsszzz", Now()) + Tag + AnsiString(".json");
    return LotRegistry.SaveToJsonFile(File);
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
        //AI(secs-lot-multilot) 20260730 : persist the LATCHED active lot. It used to save
        //whatever edLotNo held, so a mid-run click on the Lot list changed which lot a later
        //restore came back as.
        Ini->WriteString("LotList", "ActiveLot", ActiveLotID());
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
    //AI(secs-lot-additive) 20260730 : do NOT retarget an active lot. This used to assign
    //FirstLotID unconditionally, which is invisible with one lot but wrong the moment lots
    //accumulate : SET_LOT_INFO is now ADDITIVE, so every later refresh snapped edLotNo back
    //to the OLDEST registry slot - and edLotNo keys the pre-Start 2D check
    //(CheckLotDataReady), the Production_Log batch, the Soter arming, the Loader lot key and
    //the WhiteList KYEC stamp, so all five silently pointed at the wrong lot. Fill it only
    //when it is unset or has gone stale (its lot is no longer registered, e.g. after a Lot
    //End clear), which keeps every legacy single-lot caller behaving exactly as before.
    if(edLotNo!=NULL && FirstLotID!=AnsiString(""))
    {
        AnsiString Active=edLotNo->Text.Trim();
        if(Active==AnsiString("") || LotRegistry.FindLotIndex(Active)<0)
            edLotNo->Text=FirstLotID;
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-2dbin-manual) 20260628 : manual 2D/Bin editor (ts2DBinManual tab).
//Operator edits the per-Lot 2D->Bin item list by hand : add/del/paste/import
//rows then Commit into LotRegistry. Editing is locked while a lot is running
//(Is2DEditLocked) so the live sort table is never mutated mid-run.
//---------------------------------------------------------------------------
void __fastcall TfMain::Setup2DBinGrid()
{
    if(sg2DBinEdit==NULL)
        return;
    sg2DBinEdit->ColCount=2;
    sg2DBinEdit->FixedCols=0;
    sg2DBinEdit->FixedRows=1;
    sg2DBinEdit->ColWidths[0]=360;
    sg2DBinEdit->ColWidths[1]=120;
    sg2DBinEdit->Cells[0][0]=LangT("2D Code");
    sg2DBinEdit->Cells[1][0]="Bin";
    sg2DBinEdit->RowCount=2;
    sg2DBinEdit->Cells[0][1]="";
    sg2DBinEdit->Cells[1][1]="";
}
//---------------------------------------------------------------------------
void __fastcall TfMain::Refresh2DBinHeader()
{
    int DataCount=0;
    if(sg2DBinEdit!=NULL)
    {
        for(int RowIndex=1; RowIndex<sg2DBinEdit->RowCount; RowIndex++)
        {
            if(sg2DBinEdit->Cells[0][RowIndex].Trim()!=AnsiString(""))
                DataCount++;
        }
    }
    AnsiString LotText=(edLotNo!=NULL)?edLotNo->Text:AnsiString("");
    if(lblTargetLot2D!=NULL)
        lblTargetLot2D->Caption=LangT("Target Lot:")+AnsiString(" ")+LotText;
    if(lbl2DCount!=NULL)
        lbl2DCount->Caption=LangT("Items:")+AnsiString(" ")+IntToStr(DataCount);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::Reload2DBinGridFromRegistry()
{
    AnsiString TargetLot;
    int RecordCount;
    if(sg2DBinEdit==NULL || edLotNo==NULL)
        return;
    TargetLot=edLotNo->Text.Trim();
    TStringList *List=new TStringList;
    try
    {
        RecordCount=LotRegistry.GetLotIcList(TargetLot, List);
        int Rows=1+RecordCount;
        if(Rows<2)
            Rows=2;
        sg2DBinEdit->RowCount=Rows;
        // each List line : Code2D \t Bin \t HBin \t SBin \t RetestCode \t DiePass
        for(int Index=0; Index<RecordCount; Index++)
        {
            AnsiString Line=List->Strings[Index];
            AnsiString Code2D="";
            AnsiString BinStr="";
            int Tab1=Line.Pos("\t");
            if(Tab1>0)
            {
                Code2D=Line.SubString(1, Tab1-1);
                AnsiString Rest=Line.SubString(Tab1+1, Line.Length()-Tab1);
                int Tab2=Rest.Pos("\t");
                if(Tab2>0)
                    BinStr=Rest.SubString(1, Tab2-1);
                else
                    BinStr=Rest;
            }
            else
            {
                Code2D=Line;
            }
            sg2DBinEdit->Cells[0][1+Index]=Code2D;
            sg2DBinEdit->Cells[1][1+Index]=BinStr;
        }
        for(int Blank=1+RecordCount; Blank<sg2DBinEdit->RowCount; Blank++)
        {
            sg2DBinEdit->Cells[0][Blank]="";
            sg2DBinEdit->Cells[1][Blank]="";
        }
    }
    __finally
    {
        delete List;
    }
    Refresh2DBinHeader();
}
//---------------------------------------------------------------------------
bool __fastcall TfMain::Is2DEditLocked()
{
    if(MachineRun.bRunning)
    {
        ShowMyMessage(LangT("Locked: a lot is running. Edit 2D/Bin only when stopped."));
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btn2DAddRowClick(TObject *Sender)
{
    (void)Sender;
    if(Is2DEditLocked())
        return;
    if(sg2DBinEdit==NULL)
        return;
    sg2DBinEdit->RowCount=sg2DBinEdit->RowCount+1;
    sg2DBinEdit->Cells[0][sg2DBinEdit->RowCount-1]="";
    sg2DBinEdit->Cells[1][sg2DBinEdit->RowCount-1]="";
    Refresh2DBinHeader();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btn2DDelRowClick(TObject *Sender)
{
    (void)Sender;
    if(Is2DEditLocked())
        return;
    if(sg2DBinEdit==NULL)
        return;
    int Row=sg2DBinEdit->Row;
    if(Row<1)
        return;
    AnsiString Code=sg2DBinEdit->Cells[0][Row].Trim();
    if(Code!=AnsiString(""))
    {
        AnsiString Lot;
        int Bin;
        int Idx;
        if(LotRegistry.FindByCode2D(Code, Lot, Bin, Idx))
            LotRegistry.RemoveItem(Code);
    }
    // shift rows up to delete Row, keep RowCount>=2
    for(int RowIndex=Row; RowIndex<sg2DBinEdit->RowCount-1; RowIndex++)
    {
        sg2DBinEdit->Cells[0][RowIndex]=sg2DBinEdit->Cells[0][RowIndex+1];
        sg2DBinEdit->Cells[1][RowIndex]=sg2DBinEdit->Cells[1][RowIndex+1];
    }
    if(sg2DBinEdit->RowCount>2)
        sg2DBinEdit->RowCount=sg2DBinEdit->RowCount-1;
    else
    {
        sg2DBinEdit->Cells[0][1]="";
        sg2DBinEdit->Cells[1][1]="";
    }
    RefreshLotListFromRegistry();
    SaveWorkOrder();
    Refresh2DBinHeader();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btn2DCommitClick(TObject *Sender)
{
    (void)Sender;
    if(Is2DEditLocked())
        return;
    if(sg2DBinEdit==NULL || edLotNo==NULL)
        return;
    AnsiString TargetLot=edLotNo->Text.Trim();
    if(TargetLot==AnsiString(""))
    {
        ShowMyMessage(LangT("Please select or enter a Lot first !"));
        return;
    }
    int AddedCount=0;
    int DupCount=0;
    for(int Row=1; Row<sg2DBinEdit->RowCount; Row++)
    {
        AnsiString Code=sg2DBinEdit->Cells[0][Row].Trim();
        if(Code==AnsiString(""))
            continue;
        int Bin=StrToIntDef(sg2DBinEdit->Cells[1][Row].Trim(), 0);
        AnsiString ExistLot;
        int ExistBin;
        int ExistIdx;
        if(LotRegistry.FindByCode2D(Code, ExistLot, ExistBin, ExistIdx))
        {
            if(ExistLot==TargetLot)
            {
                if(ExistBin!=Bin)
                {
                    LotRegistry.RemoveItem(Code);
                    AnsiString DupLot;
                    if(LotRegistry.AddItem(TargetLot, Code, Bin, DupLot))
                        AddedCount++;
                }
            }
            else
            {
                DupCount++;
            }
        }
        else
        {
            AnsiString DupLot;
            if(LotRegistry.AddItem(TargetLot, Code, Bin, DupLot))
                AddedCount++;
            else
                DupCount++;
        }
    }
    RefreshLotListFromRegistry();
    Reload2DBinGridFromRegistry();
    SaveWorkOrder();
    ShowMyMessage(Format(LangT("Committed: %d added, %d duplicate skipped"), ARRAYOFCONST((AddedCount, DupCount))));
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btn2DClearClick(TObject *Sender)
{
    (void)Sender;
    if(Is2DEditLocked())
        return;
    if(edLotNo==NULL)
        return;
    if(ShowMyMessageBox_YES_NO(LangT("Clear ALL 2D data for this Lot ?"))!=TMyMessageBox::msgrtnYES)
        return;
    AnsiString TargetLot=edLotNo->Text.Trim();
    TStringList *List=new TStringList;
    try
    {
        LotRegistry.GetLotIcList(TargetLot, List);
        for(int Index=0; Index<List->Count; Index++)
        {
            AnsiString Line=List->Strings[Index];
            AnsiString Code;
            int Tab1=Line.Pos("\t");
            if(Tab1>0)
                Code=Line.SubString(1, Tab1-1);
            else
                Code=Line;
            Code=Code.Trim();
            if(Code!=AnsiString(""))
                LotRegistry.RemoveItem(Code);
        }
    }
    __finally
    {
        delete List;
    }
    RefreshLotListFromRegistry();
    Reload2DBinGridFromRegistry();
    SaveWorkOrder();
}
//---------------------------------------------------------------------------
//AI(ht160s-2dbin-import) 20260714 : RFC-4180 field unquote. A field wrapped in
//double quotes may contain a comma/space; a doubled "" inside is one literal
//quote. An unquoted field is Trim()'d (legacy behavior); a quoted field keeps
//its inner spaces verbatim. 2D codes never contain CR/LF (stripped at CCD
//capture) so a single-line tokenizer suffices - embedded newlines are out of scope.
static AnsiString CsvUnquoteField(AnsiString Field)
{
    Field=Field.Trim();
    int Len=Field.Length();
    if(Len>=2 && Field[1]=='"' && Field[Len]=='"')
    {
        AnsiString Inner=Field.SubString(2, Len-2);
        AnsiString Out="";
        int Pos=1;
        int InnerLen=Inner.Length();
        while(Pos<=InnerLen)
        {
            char Ch=Inner[Pos];
            if(Ch=='"' && Pos<InnerLen && Inner[Pos+1]=='"')
            {
                Out+=Ch;
                Pos+=2;
            }
            else
            {
                Out+=Ch;
                Pos++;
            }
        }
        return Out;
    }
    return Field;
}
//---------------------------------------------------------------------------
//AI(ht160s-2dbin-manual) 20260628 : split one pasted/imported line into code+bin.
//AI(ht160s-2dbin-import) 20260714 : now RFC-4180 quote-aware so a 2D code may
//contain a comma/space when wrapped in double quotes. Splits on the FIRST
//UNQUOTED comma; if none, falls back to the first unquoted tab (legacy .tab.txt).
//Staging only (operator reviews, then Commit). Returns false for a blank line.
static bool Split2DBinLine(AnsiString Line, AnsiString &Code, AnsiString &Bin)
{
    Line=Line.Trim();
    if(Line==AnsiString(""))
        return false;
    int Len=Line.Length();
    bool InQuotes=false;
    int SepPos=0;
    int TabPos=0;
    for(int Pos=1; Pos<=Len; Pos++)
    {
        char Ch=Line[Pos];
        if(Ch=='"')
        {
            if(InQuotes && Pos<Len && Line[Pos+1]=='"')
            {
                Pos++;
                continue;
            }
            InQuotes=!InQuotes;
        }
        else if(!InQuotes && Ch==',')
        {
            SepPos=Pos;
            break;
        }
        else if(!InQuotes && Ch=='\t' && TabPos==0)
        {
            TabPos=Pos;
        }
    }
    if(SepPos==0 && TabPos!=0)
        SepPos=TabPos;
    if(SepPos>0)
    {
        Code=CsvUnquoteField(Line.SubString(1, SepPos-1));
        Bin=CsvUnquoteField(Line.SubString(SepPos+1, Line.Length()-SepPos));
    }
    else
    {
        Code=CsvUnquoteField(Line);
        Bin="";
    }
    return (Code!=AnsiString(""));
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btn2DPasteClick(TObject *Sender)
{
    (void)Sender;
    if(Is2DEditLocked())
        return;
    if(sg2DBinEdit==NULL)
        return;
    AnsiString Text=Clipboard()->AsText;
    TStringList *Lines=new TStringList;
    try
    {
        Lines->Text=Text;
        for(int Index=0; Index<Lines->Count; Index++)
        {
            AnsiString Code;
            AnsiString Bin;
            if(!Split2DBinLine(Lines->Strings[Index], Code, Bin))
                continue;
            sg2DBinEdit->RowCount=sg2DBinEdit->RowCount+1;
            sg2DBinEdit->Cells[0][sg2DBinEdit->RowCount-1]=Code;
            sg2DBinEdit->Cells[1][sg2DBinEdit->RowCount-1]=Bin;
        }
    }
    __finally
    {
        delete Lines;
    }
    Refresh2DBinHeader();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::btn2DImportClick(TObject *Sender)
{
    (void)Sender;
    if(Is2DEditLocked())
        return;
    if(sg2DBinEdit==NULL)
        return;
    TOpenDialog *Dlg=new TOpenDialog(this);
    TStringList *Lines=new TStringList;
    try
    {
        Dlg->Filter="2D Import|*.csv;*.txt;*.json|CSV/Text|*.csv;*.txt|JSON|*.json|All|*.*";
        if(Dlg->Execute())
        {
            //AI(ht160s-2dbin-import) 20260714 : .json bypasses grid staging and
            //loads straight into the registry via the existing cJSON parser
            //(comma/space/quote in a code are handled). Replace semantics : Clear()
            //first so re-import is idempotent (confirmed when work order non-empty).
            AnsiString Ext=ExtractFileExt(Dlg->FileName).LowerCase();
            if(Ext==AnsiString(".json"))
            {
                if(LotRegistry.GetLotCount()>0)
                {
                    if(ShowMyMessageBox_YES_NO(LangT("Import JSON will REPLACE the current work order. Continue ?"))!=TMyMessageBox::msgrtnYES)
                        return;
                }
                //AI(ht160s-2dbin-import) 20260714 : work-order destroy point -
                //back up to LotStory Discarded first (mirror Remove Lot / fresh
                //start) and abort if the backup write fails.
                if(ArchiveDiscardedWorkOrder("JSON_IMPORT")==false)
                {
                    ShowMyMessage(LangT("Backup failed : import aborted. Check disk / log path."));
                    RecordProcess("JSON import aborted : work-order backup failed");
                    return;
                }
                LotRegistry.Clear();
                bool HasDup=false;
                AnsiString FirstDup="";
                bool bJsonOk=LotRegistry.LoadFromJsonFile(Dlg->FileName, HasDup, FirstDup);
                if(bJsonOk==false || LotRegistry.GetLotCount()<=0)
                {
                    //AI(ht160s-2dbin-import) 20260714 : LoadFromJsonString returns
                    //true for ANY parseable JSON, even one with no Maps/2DIDHistory
                    //(0 lots). Do NOT persist that empty state - restore the prior
                    //order from the still-intact on-disk WorkOrder.json.
                    ShowMyMessage(LangT("JSON load produced no lots (wrong format or empty). Work order was NOT changed."));
                    LotRegistry.Clear();
                    LoadWorkOrder();
                    RefreshLotListFromRegistry();
                    if(LotRegistry.GetLotCount()<=0 && edLotNo!=NULL)
                        edLotNo->Text="";
                    Reload2DBinGridFromRegistry();
                }
                else
                {
                    RefreshLotListFromRegistry();
                    Reload2DBinGridFromRegistry();
                    SaveWorkOrder();
                    if(HasDup)
                        ShowMyMessage(Format(LangT("Duplicate 2D code across lots (first): %s"), ARRAYOFCONST((FirstDup))));
                }
            }
            else
            {
                Lines->LoadFromFile(Dlg->FileName);
                for(int Index=0; Index<Lines->Count; Index++)
                {
                    AnsiString Code;
                    AnsiString Bin;
                    if(!Split2DBinLine(Lines->Strings[Index], Code, Bin))
                        continue;
                    sg2DBinEdit->RowCount=sg2DBinEdit->RowCount+1;
                    sg2DBinEdit->Cells[0][sg2DBinEdit->RowCount-1]=Code;
                    sg2DBinEdit->Cells[1][sg2DBinEdit->RowCount-1]=Bin;
                }
            }
        }
    }
    __finally
    {
        delete Lines;
        delete Dlg;
    }
    Refresh2DBinHeader();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::pgcWorkOrderChange(TObject *Sender)
{
    (void)Sender;
    if(pgcWorkOrder!=NULL && pgcWorkOrder->ActivePage==ts2DBinManual)
        Reload2DBinGridFromRegistry();
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

    //AI(ht160s-workorder-backup) 20260630 : prune aged LotStory Discarded backups
    //once per startup. Retention from General.ini [LogRetention] DiscardedDays
    //(0 = keep forever). Only the Discarded safety backups are aged out here; the
    //LotStory monthly production archive is traceability data and is NOT pruned.
    cCsvDailyLog::PruneFolderTree(
        HSys.LogRootDir + AnsiString("\\LotStory\\Discarded"),
        GeneralSetting.iLogRetentionDiscardedDays);
    //AI(ht160s-uph) 20260706 : age out old per-lot UPH log folders (same policy).
    TrayUphLog_PruneOld();

    //B) auto-load today's newest delivered work order (2D->Bin JSON). When a
    //   fresh lot table exists, drive the Lot list display from the registry.
    bDuplicate=false;
    DupCode="";
    bLoaded=false;
    //AI(ht160s-2dbin-manual) 20260628 : unified store first. WorkOrder.json carries
    //the full work order (lots + 2D items); if present it wins over LoadLatest and
    //the LastLotList.ini migration fallback below.
    if(LoadWorkOrder())
    {
        LotBinBinding.LoadFromIni();
        bLoaded=true;
    }
    else if(LotRegistry.LoadLatest(bDuplicate, DupCode))
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
        //C) no JSON work order : restore the last manually-used Lot list (migration
        //   fallback only : LastLotList.ini predates WorkOrder.json).
        LoadLastLotList();
        LotBinBinding.LoadFromIni();   //AI(ht160s-lotbin) 20260615 : restore dynamic bindings (see above)
        bLoaded=(LotRegistry.GetLotCount()>0 || LotBinBinding.GetBindingCount()>0);
    }

    //AI(ht160s-whitelist-override) 20260717 : restore the per-lot WhiteList overlay with the work
    //order (mirrors LotBinBinding.LoadFromIni above). Only when a work order was actually loaded;
    //otherwise force the overlay OFF and persist it so a stale overlay file cannot arm a machine
    //with no work order. The Yes/No prompt below then keeps (Yes) or clears (No) it.
    if(bLoaded)
        GeneralSetting.LoadWhiteListOverlay();
    else
    {
        GeneralSetting.SetWhiteListActive(false);
        GeneralSetting.SaveWhiteListOverlay();
    }
    UpdateSortModeFeatureBadge();

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
        Msg=Format(LangT("Inherit last work order ? (%d lots, %d bindings)   Yes = resume,  No = start fresh"), ARRAYOFCONST((LotCnt, BindCnt)));
        if(ShowMyMessageBox_YES_NO(Msg)!=TMyMessageBox::msgrtnYES)
        {
            //AI(ht160s-workorder-backup) 20260630 : archive the work order BEFORE
            //clearing, so a mis-click never loses the in-progress trace. Only delete
            //WorkOrder.json if the snapshot was written; on archive failure keep the
            //on-disk file so the next boot re-prompts instead of losing data.
            bool bArchived=ArchiveDiscardedWorkOrder("STARTUP_FRESH");
            LotRegistry.Clear();
            if(bArchived)
                DeleteFile(GetWorkOrderFileName());
            LotBinBinding.Clear();
            LotBinBinding.SaveToIni();
            //AI(secs-lotstarttime-persist) 20260730 : a fresh start is not an open lot - drop the
            //stored Lot Start time with the rest of the work order so 1009 stays empty.
            ClearWorkOrderMeta();
            //AI(ht160s-whitelist-override) 20260717 : fresh start also clears the WhiteList overlay.
            GeneralSetting.SetWhiteListActive(false);
            GeneralSetting.SaveWhiteListOverlay();
            UpdateSortModeFeatureBadge();
            if(edLotNo!=NULL)
                edLotNo->Text="";
            RefreshLotListFromRegistry();
            SaveLastLotList();
            RecordProcess(bArchived
                ? "Startup: fresh start; last work order archived to LotStory Discarded then cleared"
                : "Startup: fresh start; ARCHIVE FAILED, WorkOrder.json kept, registry cleared");
        }
        else
        {
            //AI(secs-lot-additive) 20260730 : an inherited work order is an OPEN lot, so
            //re-latch the lot-lifecycle flag. MachineRun.bRunning is in-memory only and used
            //to come back false after a power cycle, which meant (a) the 2D/Bin editor and
            //the hardware/sort-mode page unlocked mid-lot, and (b) - now that the SECS
            //LOTSTART branch keys off it - a host LOTSTART would classify a resumed lot as
            //"Lot End state" and run the FULL init, wiping the very LotBinBinding table and
            //per-lot counters this restore just brought back. Only the resume answer sets it;
            //the "start fresh" branch above cleared the registry and must stay Lot-End.
            if(LotRegistry.GetLotCount()>0)
            {
                MachineRun.bRunning=true;
                MachineRun.iActiveLotCount=LotRegistry.GetLotCount();
                //AI(secs-lot-multilot) 20260730 : re-latch the active lot too. LoadWorkOrder /
                //LoadLastLotList have already put the stored active lot into edLotNo, so adopt it
                //as the latch; the work order is open again and ActiveLotID() must not fall back
                //to a box the operator is free to retype.
                m_sActiveLot=(edLotNo!=NULL) ? edLotNo->Text.Trim() : AnsiString("");
                //AI(secs-lotstarttime-persist) 20260730 : an inherited lot is still OPEN, so
                //SVID 1009 must answer the ORIGINAL start time rather than "". Re-latch the
                //stored stamp verbatim. If nothing was stored (work order predates this, or the
                //meta file was lost) leave it empty - the documented "between lots" value beats a
                //confidently wrong timestamp derived from Now() or tRunData.StartTime.
                AnsiString sStoredStart=LoadWorkOrderLotStartTime();
                if(sStoredStart!=AnsiString(""))
                {
                    NoteLotStartTime(true, sStoredStart);
                    RecordProcess("Startup: restored Lot Start Time (SVID 1009) = "+sStoredStart);
                }
                else
                    RecordProcess("Startup: no stored Lot Start Time ; SVID 1009 stays empty");
            }
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
        ShowMyMessage(LangT("Please Enter LotID !"));
        return;
    }

    //AI(general) 20260610 : add through LotRegistry (single source of truth),
    // then reproject the grid. AddLot is idempotent (existing LotID re-activates),
    // matching the SECS / Simu paths. <0 means the 64-lot registry is full.
    LotIndex=LotRegistry.AddLot(LotText, HT160_LOT_SOURCE_OFFLINE, "", "");
    if(LotIndex<0)
    {
        ShowMyMessage(LangT("Lot list is full !"));
        return;
    }
    RefreshLotListFromRegistry();
    SaveWorkOrder();
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
        ShowMyMessage(LangT("Please Enter LotID !"));
        return;
    }
    if(OldLot=="" || OldLot==NewLot)
        return;

    //AI(general) 20260610 : rename in LotRegistry, then reproject the grid.
    LotRegistry.RenameLot(OldLot, NewLot);
    RefreshLotListFromRegistry();
    SaveWorkOrder();
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
    //AI(ht160s-workorder-backup) 20260630 : Remove Lot had NO confirm and NO archive
    //- a single mis-click dropped a whole lot's 2D/Bin trace. Confirm first, then
    //snapshot the whole work order (captures the lot being removed) before the drop.
    if(ShowMyMessageBox_YES_NO(LangT("Remove this Lot from the work order ?"))!=TMyMessageBox::msgrtnYES)
        return;
    //AI(ht160s-workorder-backup) 20260630 : abort the removal if the snapshot did not
    //write (real data + disk/log-path failure), so the lot's trace is never lost.
    if(ArchiveDiscardedWorkOrder("REMOVE_LOT")==false)
    {
        ShowMyMessage(LangT("Backup failed : Lot NOT removed. Check disk / log path."));
        RecordProcess("Remove Lot aborted : work-order backup failed");
        return;
    }
    LotRegistry.RemoveLot(LotText);
    RefreshLotListFromRegistry();
    SaveWorkOrder();
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
    {
        //AI(secs-lot-multilot) 20260730 : this assignment is deliberately left alone. It used to
        //be a hazard - edLotNo doubled as the ACTIVE-lot pointer, so clicking a row just to LOOK
        //at another lot's 2D list (the documented double-click workflow, and the single click
        //fires first) silently retargeted the running work order. The active lot is now LATCHED
        //at Lot Start in m_sActiveLot / ActiveLotID(), so edLotNo is once again only the manual
        //Add/Edit-Lot field and the 2D-editor target, and following the click here is correct.
        edLotNo->Text=sgLotList->Cells[0][SelectedRow];
        Reload2DBinGridFromRegistry();   //AI(ht160s-2dbin-manual) 20260628 : picking a lot refreshes the 2D grid
    }
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
            ShowMyMessage(Format(LangT("Lot \"%s\" has no 2D data loaded yet.\n(Work-order JSON not downloaded for this Lot.)"), ARRAYOFCONST((LotID))));
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
            Grid->Cells[0][0]=LangT("2D Code");
            Grid->Cells[1][0]="Bin";
            Grid->Cells[2][0]="HBin";
            Grid->Cells[3][0]="SBin";
            Grid->Cells[4][0]=LangT("RetestCode");
            Grid->Cells[5][0]=LangT("DiePass");

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

//---------------------------------------------------------------------------
//AI(ht160s-status) 20260703 : Module Status diagnostic sheet (pgcDiagnostic /
//tsModuleStatus). One row per module showing the unified-status name (approved design,
//docs/plan/module-status-enum-design-20260703.md). Called on every DoSystemMessage
//pass; throttled to ~300ms and skipped while the sheet is not the active diagnostic
//page, so the machine spin never pays for a hidden grid.
void __fastcall TfMain::RefreshModuleStatusGrid()
{
    static DWORD dwLast=0;
    static const char *SasName[4]={"IDLE","PICKING","PLACING","RECOVERY"};
    static const char *TasName[4]={"IDLE","PICKING","CARRYING","PLACING"};
    static const char *LsName[6]={"IDLE","FEEDING","CCD_SCAN","READY_SORT","SORTING","TO_REAR"};
    static const char *EsName[5]={"IDLE","DESTACK","FEEDING","REAR_READY","RETURNING"};
    static const char *CsName[5]={"IDLE","DESTACK","FEEDING","REAR_READY","RETURNING"};
    static const char *AsName[7]={"IDLE","REAR_STAGED","LOADING","SORTING","FULL","DISCHARGING","CLEANOUT_DONE"};
    DWORD dwNow;
    int St;
    int Row;

    if(sgModuleStatus==NULL || pgcDiagnostic==NULL || tsModuleStatus==NULL)
        return;
    if(pgcDiagnostic->ActivePage!=tsModuleStatus)
        return;
    dwNow=GetTickCount();
    if(dwLast!=0 && (int)(dwNow-dwLast)<300)
        return;
    dwLast=dwNow;

    sgModuleStatus->Cells[0][0]="Module";
    sgModuleStatus->Cells[1][0]="Status";
    sgModuleStatus->Cells[2][0]="Note";

    Row=1;
    St=(SortArmModule!=NULL) ? SortArmModule->GetStatus() : 0;
    sgModuleStatus->Cells[0][Row]="SortArm";
    sgModuleStatus->Cells[1][Row]=(St>=0 && St<4) ? SasName[St] : "?";
    sgModuleStatus->Cells[2][Row]=(SortArmModule!=NULL && SortArmModule->HasHoldingIC()) ? "holding IC" : "";
    Row++;
    St=(TrayArmModule!=NULL) ? TrayArmModule->GetStatus() : 0;
    sgModuleStatus->Cells[0][Row]="TrayArm";
    sgModuleStatus->Cells[1][Row]=(St>=0 && St<4) ? TasName[St] : "?";
    sgModuleStatus->Cells[2][Row]=(TrayArmModule!=NULL && TrayArmModule->HasTray()) ? "tray in hand" : "";
    Row++;
    for(int n=1; n<=2; n++)
    {
        St=(LoaderModule!=NULL) ? LoaderModule->GetLoaderStatus(n) : 0;
        sgModuleStatus->Cells[0][Row]=AnsiString("Loader")+IntToStr(n);
        sgModuleStatus->Cells[1][Row]=(St>=0 && St<6) ? LsName[St] : "?";
        sgModuleStatus->Cells[2][Row]="";
        Row++;
    }
    St=(EmptyModule!=NULL) ? EmptyModule->GetStatus() : 0;
    sgModuleStatus->Cells[0][Row]="Empty";
    sgModuleStatus->Cells[1][Row]=(St>=0 && St<5) ? EsName[St] : "?";
    sgModuleStatus->Cells[2][Row]="";
    Row++;
    St=(ColorModule!=NULL) ? ColorModule->GetStatus() : 0;
    sgModuleStatus->Cells[0][Row]="Color";
    sgModuleStatus->Cells[1][Row]=(St>=0 && St<5) ? CsName[St] : "?";
    sgModuleStatus->Cells[2][Row]="";
    Row++;
    for(int n=0; n<6; n++)
    {
        St=(AutoModule!=NULL) ? AutoModule->GetStationStatus(n) : 0;
        sgModuleStatus->Cells[0][Row]=AnsiString("Auto")+IntToStr(n+1);
        sgModuleStatus->Cells[1][Row]=(St>=0 && St<7) ? AsName[St] : "?";
        sgModuleStatus->Cells[2][Row]="";
        Row++;
    }
}
