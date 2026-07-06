//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "language.h"

#include "setup.h"
#include "CosFunction.h"
#include "GeneralSetting.h"
#include "aSortArm.h"   //AI(ht160s-pnp) 20260626 : SortArmModule global + SetPnPParameters for ApplyPnPToSortArm
#include "database.h"
#include "mymessbox.h"   //AI(general) 20260608 : ShowMyMessageBox_YES_NO instead of Application->MessageBox
#include "ComPort.h"   //AI(ht160s-bindisplay) 20260706 : fComPort + EnsureComPortCreated to repaint the bin panel on Save Map
#include <Dialogs.hpp>
#include <IniFiles.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "HTray"
#pragma resource "*.dfm"
TfSetup *fSetup;
//---------------------------------------------------------------------------
typedef struct
{
    TTabSheet *Page;
    TSpeedButton *Button;
    TSetupMenuAction Action;
    bool PinBottom;
} TSetupPageDef;
//---------------------------------------------------------------------------
static const char *SETUP_INI_GROUP="Setup";
static const char *TRAY_FORM_INI_GROUP="TrayForm";
static const char *PNP_INI_GROUP="PnP";   //AI(ht160s-pnp) 20260626 : SortArm pick/place tuning section in recipe setup.ini
static const int SORT_ARM_SUCKER_COUNT=4;   //AI(ht160s-pnp) 20260626 : SortArm nozzle count (mirrors GeneralSetting.bSuckerEnabled[4]); keep in sync with aSortArm.cpp
enum
{
    BIN_GRID_COL_AREA=0,
    BIN_GRID_COL_BIN=1,
    BIN_GRID_COL_STATUS=2,
    BIN_GRID_COL_NOTE=3,
    BIN_GRID_COL_COUNT=4,
    BIN_GRID_FIRST_AREA=eHT160BinAreaAuto1
};
//---------------------------------------------------------------------------
__fastcall TfSetup::TfSetup(TComponent* Owner)
    : TForm(Owner)
{
    int PageIndex;

    iSetupMenuCount=0;
    LastClickButton=NULL;
    bLoadingTrayForm=false;
    bLoadingBinGrid=false;
    bLoadingPnP=false;

    for(PageIndex=0; PageIndex<MAX_SETUP_MENU_COUNT; PageIndex++)
    {
        MenuButtons[PageIndex]=NULL;
        MenuPages[PageIndex]=NULL;
        MenuActions[PageIndex]=suShowPage;
        MenuBottomPins[PageIndex]=false;
    }

    RegisterSetupPages();
    LayoutSetupButtons();
    BindTrayFormEvents();
    BuildBinSettingUI();
    BuildPnPUI();
    SelectSetupPage(0);
}
//---------------------------------------------------------------------------
AnsiString __fastcall TfSetup::GetSetUpFileName()
{
    return RecipeManager.GetRecipeFileName("setup.ini");
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::OpenWorkFile()
{
    RecipeManager.LoadLastRecipeName();
    RecipeManager.EnsureCurrentRecipeDir();
    LoadTrayFormSettings(RecipeManager.GetCurrentRecipeName());
    LoadPnPSettings(RecipeManager.GetCurrentRecipeName());   //AI(ht160s-pnp) 20260626 : seed runtime SortArm PnP scalars from the active recipe
    BinAreaMap.LoadDefault();
    LoadBinMapToGrid();
    RefreshRecipeList();
    RefreshRecipeStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::SaveWorkFile(AnsiString S)
{
    if(S==AnsiString(""))
        S=GetSetUpFileName();

    RecipeManager.EnsureCurrentRecipeDir();
    WriteRecipeSetupFile(RecipeManager.GetCurrentRecipeName());
    SaveTrayFormSettings(RecipeManager.GetCurrentRecipeName());
    SavePnPSettings(RecipeManager.GetCurrentRecipeName());   //AI(ht160s-pnp) 20260626 : persist + re-apply SortArm PnP scalars
    SaveBinSettingMap(false);
    WriteRecipeManifest(RecipeManager.GetCurrentRecipeName(), AnsiString(""));
    RecipeManager.SaveLastRecipeName();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::RegisterSetupPages()
{
    TSetupPageDef PageDefs[]={
        {tsSetupRecipe,    spbSetupRecipe,    suShowPage,  false},
        {tsSetupTrayForm,  spbSetupTrayForm,  suShowPage,  false},
        {tsSetupBinSetting,spbSetupBinSetting,suShowPage,  false},
        {tsSetupPnP,       spbSetupPnP,       suShowPage,  false},
        {NULL,             spbSetupExit,      suCloseForm, true}
    };
    int PageIndex;
    int PageCount;

    PageCount=sizeof(PageDefs)/sizeof(PageDefs[0]);
    if(PageCount>MAX_SETUP_MENU_COUNT)
        PageCount=MAX_SETUP_MENU_COUNT;

    iSetupMenuCount=PageCount;
    for(PageIndex=0; PageIndex<iSetupMenuCount; PageIndex++)
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
            MenuButtons[PageIndex]->OnClick=spbSetupMenuClick;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::LayoutSetupButtons()
{
    const int ButtonLeft=8;
    const int ButtonTop=8;
    const int ButtonWidth=180;
    const int ButtonHeight=50;
    const int ButtonGap=6;
    int PageIndex;
    int TopPos;

    TopPos=ButtonTop;
    for(PageIndex=0; PageIndex<iSetupMenuCount; PageIndex++)
    {
        if(MenuButtons[PageIndex]==NULL)
            continue;

        if(MenuBottomPins[PageIndex])
            MenuButtons[PageIndex]->SetBounds(ButtonLeft, pnlMenu->ClientHeight-ButtonHeight-ButtonTop, ButtonWidth, ButtonHeight);
        else
        {
            MenuButtons[PageIndex]->SetBounds(ButtonLeft, TopPos, ButtonWidth, ButtonHeight);
            TopPos += ButtonHeight + ButtonGap;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::SelectSetupPage(int PageIndex)
{
    if(PageIndex<0 || PageIndex>=iSetupMenuCount)
        return;

    if(MenuActions[PageIndex]==suCloseForm)
    {
        Close();
        return;
    }

    if(pcSetup==NULL || MenuPages[PageIndex]==NULL || MenuButtons[PageIndex]==NULL)
        return;

    pcSetup->ActivePage=MenuPages[PageIndex];
    pnlTitle->Caption=MenuPages[PageIndex]->Caption;
    MenuButtons[PageIndex]->Down=true;
    LastClickButton=MenuButtons[PageIndex];
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::RefreshRecipeStatus()
{
    AnsiString SetupFileName;
    AnsiString BinAreaFileName;
    AnsiString ManifestFileName;

    SetupFileName=GetSetUpFileName();
    BinAreaFileName=BinAreaMap.GetDefaultIniFileName();
    ManifestFileName=RecipeManager.GetRecipeFileName("manifest.ini");

    lblCurrentRecipeValue->Caption=RecipeManager.GetCurrentRecipeName();
    lblRecipeDirValue->Caption=RecipeManager.GetRecipeDirName();
    lblSetupFileValue->Caption=SetupFileName;
    lblBinAreaMapValue->Caption=BinAreaFileName;
    lblSetupFileStatusValue->Caption=FileExists(SetupFileName)?AnsiString(LangT("Ready")):AnsiString(LangT("Not Created"));
    lblBinMapStatusValue->Caption=FileExists(BinAreaFileName)?AnsiString(LangT("Ready")):AnsiString(LangT("Not Created"));
    lblManifestValue->Caption=FileExists(ManifestFileName)?AnsiString(LangT("Ready")):AnsiString(LangT("Not Created"));
    RefreshBinSettingStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::RefreshRecipeList()
{
    TSearchRec Search;
    AnsiString CurrentRecipe;
    int FindResult;
    int Index;

    if(lstRecipe==NULL)
        return;

    RecipeManager.EnsureCurrentRecipeDir();
    CurrentRecipe=RecipeManager.GetCurrentRecipeName();
    lstRecipe->Items->BeginUpdate();
    lstRecipe->Items->Clear();
    FindResult=FindFirst(RecipeManager.GetDataRootPath()+AnsiString("\\*.*"), faAnyFile, Search);
    if(FindResult==0)
    {
        while(FindResult==0)
        {
            if((Search.Attr & faDirectory)!=0 && Search.Name!=AnsiString(".") && Search.Name!=AnsiString(".."))
                lstRecipe->Items->Add(Search.Name);
            FindResult=FindNext(Search);
        }
        FindClose(Search);
    }
    lstRecipe->Items->EndUpdate();

    SelectRecipeInList(CurrentRecipe);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::SelectRecipeInList(AnsiString RecipeName)
{
    AnsiString Name=RecipeManager.NormalizeRecipeName(RecipeName);
    int Index;

    if(lstRecipe==NULL)
        return;

    lstRecipe->ItemIndex=-1;
    for(Index=0; Index<lstRecipe->Items->Count; Index++)
    {
        if(lstRecipe->Items->Strings[Index].UpperCase()==Name.UpperCase())
        {
            lstRecipe->ItemIndex=Index;
            break;
        }
    }
    edRecipeName->Text=Name;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TfSetup::GetSelectedRecipeName()
{
    if(lstRecipe!=NULL && lstRecipe->ItemIndex>=0)
        return RecipeManager.NormalizeRecipeName(lstRecipe->Items->Strings[lstRecipe->ItemIndex]);
    return RecipeManager.NormalizeRecipeName(edRecipeName->Text);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::WriteRecipeSetupFile(AnsiString RecipeName)
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString Name=RecipeManager.NormalizeRecipeName(RecipeName);

    FileName=RecipeManager.GetRecipeFileName(Name, "setup.ini");
    ForceDirectories(ExtractFilePath(FileName));
    Ini=new TIniFile(FileName);
    Ini->WriteString(SETUP_INI_GROUP, "RecipeName", Name);
    Ini->WriteString(SETUP_INI_GROUP, "RecipeDir", RecipeManager.GetRecipeDirName(Name));
    Ini->WriteString(SETUP_INI_GROUP, "BinAreaMap", RecipeManager.GetRecipeFileName(Name, "BinAreaMap.ini"));
    delete Ini;
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::WriteRecipeManifest(AnsiString RecipeName, AnsiString SourceRecipeName)
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString Name=RecipeManager.NormalizeRecipeName(RecipeName);

    FileName=RecipeManager.GetRecipeFileName(Name, "manifest.ini");
    ForceDirectories(ExtractFilePath(FileName));
    Ini=new TIniFile(FileName);
    Ini->WriteString("Manifest", "RecipeName", Name);
    Ini->WriteString("Manifest", "SourceRecipe", SourceRecipeName.Trim()==AnsiString("")?AnsiString(""):RecipeManager.NormalizeRecipeName(SourceRecipeName));
    Ini->WriteString("Manifest", "LastUpdate", DateTimeToStr(Now()));
    delete Ini;
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::BindTrayFormEvents()
{
    TEdit *Edits[6];
    int Index;

    Edits[0]=edXStart;
    Edits[1]=edXPitch;
    Edits[2]=edYStart;
    Edits[3]=edYPitch;
    Edits[4]=edXDivision;
    Edits[5]=edYDivision;
    for(Index=0; Index<6; Index++)
    {
        if(Edits[Index]!=NULL)
        {
            Edits[Index]->OnChange=TrayFormEditChange;
            Edits[Index]->OnExit=TrayFormEditChange;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::LoadTrayFormSettings(AnsiString RecipeName)
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString Name=RecipeManager.NormalizeRecipeName(RecipeName);

    FileName=RecipeManager.GetRecipeFileName(Name, "setup.ini");
    bLoadingTrayForm=true;
    if(FileExists(FileName))
    {
        Ini=new TIniFile(FileName);
        edXStart->Text=FormatTrayDouble(Ini->ReadFloat(TRAY_FORM_INI_GROUP, "XStart", 0.0));
        edXPitch->Text=FormatTrayDouble(Ini->ReadFloat(TRAY_FORM_INI_GROUP, "XPitch", 1.0));
        edYStart->Text=FormatTrayDouble(Ini->ReadFloat(TRAY_FORM_INI_GROUP, "YStart", 0.0));
        edYPitch->Text=FormatTrayDouble(Ini->ReadFloat(TRAY_FORM_INI_GROUP, "YPitch", 1.0));
        edXDivision->Text=IntToStr(Ini->ReadInteger(TRAY_FORM_INI_GROUP, "XDivision", 1));
        edYDivision->Text=IntToStr(Ini->ReadInteger(TRAY_FORM_INI_GROUP, "YDivision", 1));
        delete Ini;
    }
    else
    {
        edXStart->Text="0.000";
        edXPitch->Text="1.000";
        edYStart->Text="0.000";
        edYPitch->Text="1.000";
        edXDivision->Text="1";
        edYDivision->Text="1";
    }
    bLoadingTrayForm=false;
    RefreshTrayFormPreview();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::SaveTrayFormSettings(AnsiString RecipeName)
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString Name=RecipeManager.NormalizeRecipeName(RecipeName);
    int XDivision;
    int YDivision;

    FileName=RecipeManager.GetRecipeFileName(Name, "setup.ini");
    ForceDirectories(ExtractFilePath(FileName));
    if(atoi(edXDivision->Text.c_str())>MAX_TRAY_X || atoi(edYDivision->Text.c_str())>MAX_TRAY_Y)
    {
        ShowMyOKMessageNoStop(Format(LangT("Tray division exceeds machine limit (X max=%d, Y max=%d). Value clamped."), ARRAYOFCONST((MAX_TRAY_X, MAX_TRAY_Y))));
    }
    XDivision=GetTrayEditInt(edXDivision, 1, 1, MAX_TRAY_X);
    YDivision=GetTrayEditInt(edYDivision, 1, 1, MAX_TRAY_Y);
    Ini=new TIniFile(FileName);
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "XStart", GetTrayEditDouble(edXStart, 0.0));
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "XPitch", GetTrayEditDouble(edXPitch, 1.0));
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "YStart", GetTrayEditDouble(edYStart, 0.0));
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "YPitch", GetTrayEditDouble(edYPitch, 1.0));
    Ini->WriteInteger(TRAY_FORM_INI_GROUP, "XDivision", XDivision);
    Ini->WriteInteger(TRAY_FORM_INI_GROUP, "YDivision", YDivision);
    delete Ini;

    //AI(HT160S-Maintainer) 20260608 : refresh the in-memory TrayForm structure
    //from the just-saved INI so Loader/SortArm/Auto/Monitor pick up the new
    //geometry immediately instead of reading stale Setup-form UI defaults.
    TrayForm.Load(Name);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::BuildPnPUI()
{
    //AI(ht160s-pnp) 20260626 : one-time PnP tab wiring. Size the sucker-enable grid to the SortArm
    //nozzle count (one row, N columns) and bind the grid-click + Use-Suck radio handlers in code so
    //the .dfm carries no event bindings.
    if(grdSuckEnable!=NULL)
    {
        grdSuckEnable->XItem=SORT_ARM_SUCKER_COUNT;
        grdSuckEnable->YItem=1;
        grdSuckEnable->OnMouseUp=grdSuckEnableMouseUp;
    }
    if(rgPnpUseSuck!=NULL)
        rgPnpUseSuck->OnClick=rgPnpUseSuckClick;
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::LoadPnPSettings(AnsiString RecipeName)
{
    //AI(ht160s-pnp) 20260626 : read the [PnP] scalar tuning from the recipe setup.ini into the edits,
    //then push to the runtime model. Mirrors LoadTrayFormSettings (TIniFile + FileExists guard).
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString Name=RecipeManager.NormalizeRecipeName(RecipeName);

    FileName=RecipeManager.GetRecipeFileName(Name, "setup.ini");
    bLoadingPnP=true;
    if(FileExists(FileName))
    {
        Ini=new TIniFile(FileName);
        edPnpPickDelay->Text=FormatTrayDouble(Ini->ReadFloat(PNP_INI_GROUP, "PickDelaySec", 0.0));
        edPnpPlaceDelay->Text=FormatTrayDouble(Ini->ReadFloat(PNP_INI_GROUP, "PlaceDelaySec", 0.0));
        edtDestroyCheckTime->Text=FormatTrayDouble(Ini->ReadFloat(PNP_INI_GROUP, "DestroyCheckTime", 0.3));
        delete Ini;
    }
    else
    {
        edPnpPickDelay->Text="0.000";
        edPnpPlaceDelay->Text="0.000";
        edtDestroyCheckTime->Text="0.300";
    }
    bLoadingPnP=false;
    ApplyPnPToSortArm();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::SavePnPSettings(AnsiString RecipeName)
{
    //AI(ht160s-pnp) 20260626 : write the [PnP] scalar tuning back to the recipe setup.ini, then
    //re-apply to the runtime model. Mirrors SaveTrayFormSettings (ForceDirectories + WriteFloat).
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString Name=RecipeManager.NormalizeRecipeName(RecipeName);

    FileName=RecipeManager.GetRecipeFileName(Name, "setup.ini");
    ForceDirectories(ExtractFilePath(FileName));
    Ini=new TIniFile(FileName);
    Ini->WriteFloat(PNP_INI_GROUP, "PickDelaySec", GetTrayEditDouble(edPnpPickDelay, 0.0));
    Ini->WriteFloat(PNP_INI_GROUP, "PlaceDelaySec", GetTrayEditDouble(edPnpPlaceDelay, 0.0));
    Ini->WriteFloat(PNP_INI_GROUP, "DestroyCheckTime", GetTrayEditDouble(edtDestroyCheckTime, 0.3));
    delete Ini;

    ApplyPnPToSortArm();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::ApplyPnPToSortArm()
{
    //AI(ht160s-pnp) 20260626 : push the [PnP] scalar tuning into the runtime SortArm model. Per-nozzle
    //enable is NOT pushed here - it lives in GeneralSetting.bSuckerEnabled[4] and aSortArm reads it
    //live each pick. SortArmModule can be NULL very early in startup; guard it.
    if(SortArmModule==NULL)
        return;
    SortArmModule->SetPnPParameters(
        GetTrayEditDouble(edPnpPickDelay, 0.0),
        GetTrayEditDouble(edPnpPlaceDelay, 0.0),
        GetTrayEditDouble(edtDestroyCheckTime, 0.3));
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::LoadSuckEnable()
{
    //AI(ht160s-pnp) 20260626 : per-nozzle enable is machine-level (GeneralSetting / General.ini),
    //already loaded at startup. Reflect it into the grid + the Use-Suck mode selector. All-enabled
    //-> "Use All" (index 0, grid locked); otherwise "Use Select" (index 1, grid editable).
    int s;
    int iEnabledCount;

    bLoadingPnP=true;
    iEnabledCount=0;
    for(s=0; s<SORT_ARM_SUCKER_COUNT; s++)
        if(GeneralSetting.bSuckerEnabled[s])
            iEnabledCount++;
    if(rgPnpUseSuck!=NULL)
        rgPnpUseSuck->ItemIndex=(iEnabledCount==SORT_ARM_SUCKER_COUNT)?0:1;
    RefreshSuckGrid();
    if(rgPnpUseSuck!=NULL && grdSuckEnable!=NULL)
        grdSuckEnable->Enabled=(rgPnpUseSuck->ItemIndex==1);
    //AI(ht160s-pick-retry) 20260706 : reflect machine-level [SortArm] PickRetryCount into the edit.
    if(edSortArmPickRetry!=NULL)
        edSortArmPickRetry->Text=IntToStr(GeneralSetting.iSortArmPickRetryCount);
    bLoadingPnP=false;
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::SaveSuckEnable()
{
    //AI(ht160s-pnp) 20260626 : grid clicks already wrote GeneralSetting.bSuckerEnabled[]. Guarantee
    //at least one nozzle stays enabled (mirrors the maintenance.cpp invariant), then persist to
    //General.ini. aSortArm reads the array live, so no engine refresh is required.
    int s;
    int iEnabledCount;

    iEnabledCount=0;
    for(s=0; s<SORT_ARM_SUCKER_COUNT; s++)
        if(GeneralSetting.bSuckerEnabled[s])
            iEnabledCount++;
    if(iEnabledCount==0)
        GeneralSetting.bSuckerEnabled[0]=true;
    //AI(ht160s-pick-retry) 20260706 : persist the pick-retry budget (0..10; 0 = alarm on first fail).
    //aSortArm reads GeneralSetting.iSortArmPickRetryCount live, so no engine refresh is required.
    if(edSortArmPickRetry!=NULL)
        GeneralSetting.iSortArmPickRetryCount=GetTrayEditInt(edSortArmPickRetry, 3, 0, 10);
    GeneralSetting.Save();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::RefreshSuckGrid()
{
    //AI(ht160s-pnp) 20260626 : paint each nozzle cell green (enabled, color index 1) or white
    //(disabled, color index 0) from GeneralSetting.bSuckerEnabled[]; label cells 1..N.
    int s;

    if(grdSuckEnable==NULL)
        return;
    grdSuckEnable->XItem=SORT_ARM_SUCKER_COUNT;
    grdSuckEnable->YItem=1;
    for(s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        grdSuckEnable->SetCellNumber(s, 0, s+1);
        grdSuckEnable->SetCellColorIndex(s, 0, GeneralSetting.bSuckerEnabled[s]?1:0);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::grdSuckEnableMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    //AI(ht160s-pnp) 20260626 : map pixel (X,Y) to a nozzle cell via TTMyTray::ConvertIndexCells
    //(returns 1 on hit, rewriting X/Y to cell indices). Grid is one row so the nozzle index is X.
    //Toggle GeneralSetting.bSuckerEnabled[] and recolor; never disable the last enabled nozzle.
    int idx;
    int s;
    int iEnabledCount;

    (void)Sender;
    (void)Button;
    (void)Shift;
    if(grdSuckEnable==NULL)
        return;
    if(bLoadingPnP)
        return;
    if(rgPnpUseSuck!=NULL && rgPnpUseSuck->ItemIndex==0)
        return;
    if(grdSuckEnable->ConvertIndexCells(X, Y)!=1)
        return;
    idx=X;
    if(idx<0 || idx>=SORT_ARM_SUCKER_COUNT)
        return;
    if(GeneralSetting.bSuckerEnabled[idx])
    {
        iEnabledCount=0;
        for(s=0; s<SORT_ARM_SUCKER_COUNT; s++)
            if(GeneralSetting.bSuckerEnabled[s])
                iEnabledCount++;
        if(iEnabledCount<=1)
        {
            ShowMyOKMessageNoStop(LangT("At least one nozzle must stay enabled."));
            return;
        }
        GeneralSetting.bSuckerEnabled[idx]=false;
        grdSuckEnable->SetCellColorIndex(idx, 0, 0);
    }
    else
    {
        GeneralSetting.bSuckerEnabled[idx]=true;
        grdSuckEnable->SetCellColorIndex(idx, 0, 1);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::rgPnpUseSuckClick(TObject *Sender)
{
    //AI(ht160s-pnp) 20260626 : SortArm-only Use-Suck mode (the HT172 Mag Arm column is removed).
    //Index 0 = Use All : force every nozzle enabled + green, lock the grid. Index 1 = Use Select :
    //unlock the grid for per-nozzle clicking. bLoadingPnP suppresses the programmatic ItemIndex set.
    int s;

    (void)Sender;
    if(bLoadingPnP)
        return;
    if(rgPnpUseSuck==NULL)
        return;
    if(rgPnpUseSuck->ItemIndex==1)
    {
        if(grdSuckEnable!=NULL)
            grdSuckEnable->Enabled=true;
    }
    else
    {
        for(s=0; s<SORT_ARM_SUCKER_COUNT; s++)
            GeneralSetting.bSuckerEnabled[s]=true;
        RefreshSuckGrid();
        if(grdSuckEnable!=NULL)
            grdSuckEnable->Enabled=false;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::WriteDefaultTrayFormSettings(AnsiString RecipeName)
{
    TIniFile *Ini;
    AnsiString FileName;
    AnsiString Name=RecipeManager.NormalizeRecipeName(RecipeName);

    FileName=RecipeManager.GetRecipeFileName(Name, "setup.ini");
    ForceDirectories(ExtractFilePath(FileName));
    Ini=new TIniFile(FileName);
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "XStart", 0.0);
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "XPitch", 1.0);
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "YStart", 0.0);
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "YPitch", 1.0);
    Ini->WriteInteger(TRAY_FORM_INI_GROUP, "XDivision", 1);
    Ini->WriteInteger(TRAY_FORM_INI_GROUP, "YDivision", 1);
    delete Ini;
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::RefreshTrayFormPreview()
{
    int XDivision;
    int YDivision;
    int XIndex;
    int YIndex;
    int CellNo;

    if(TMyTray1==NULL)
        return;

    XDivision=GetTrayEditInt(edXDivision, 1, 1, MAX_TRAY_X);
    YDivision=GetTrayEditInt(edYDivision, 1, 1, MAX_TRAY_Y);
    TMyTray1->XItem=XDivision;
    TMyTray1->YItem=YDivision;
    CellNo=1;
    for(YIndex=0; YIndex<YDivision; YIndex++)
    {
        for(XIndex=0; XIndex<XDivision; XIndex++)
        {
            TMyTray1->SetCellNumber(XIndex, YIndex, CellNo);
            TMyTray1->SetCellColorIndex(XIndex, YIndex, 1);
            CellNo++;
        }
    }
}
//---------------------------------------------------------------------------
int __fastcall TfSetup::GetTrayEditInt(TEdit *Edit, int DefaultValue, int MinValue, int MaxValue)
{
    int Value;

    if(Edit==NULL)
        return DefaultValue;
    Value=atoi(Edit->Text.c_str());
    if(Value<MinValue)
        Value=MinValue;
    if(Value>MaxValue)
        Value=MaxValue;
    Edit->Text=IntToStr(Value);
    return Value;
}
//---------------------------------------------------------------------------
double __fastcall TfSetup::GetTrayEditDouble(TEdit *Edit, double DefaultValue)
{
    double Value;

    if(Edit==NULL || Edit->Text.Trim()==AnsiString(""))
        return DefaultValue;
    Value=atof(Edit->Text.c_str());
    return Value;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TfSetup::FormatTrayDouble(double Value)
{
    AnsiString Text;

    Text.sprintf("%.3f", Value);
    return Text;
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::BuildBinSettingUI()
{
    ConfigureBinSettingGrid();
    LoadBinMapToGrid();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::ConfigureBinSettingGrid()
{
    if(grdBinAreaMap==NULL)
        return;

    grdBinAreaMap->ColCount=BIN_GRID_COL_COUNT;
    grdBinAreaMap->RowCount=GetBinGridAreaCount()+1;
    grdBinAreaMap->FixedRows=1;
    grdBinAreaMap->FixedCols=0;
    grdBinAreaMap->DefaultRowHeight=24;
    grdBinAreaMap->FixedColor=clTeal;
    grdBinAreaMap->Color=clWhite;
    grdBinAreaMap->Font->Name="Arial";
    grdBinAreaMap->Font->Size=9;
    grdBinAreaMap->Options=grdBinAreaMap->Options << goEditing << goTabs << goColSizing;
    grdBinAreaMap->OnExit=grdBinAreaMapExit;
    grdBinAreaMap->OnSelectCell=grdBinAreaMapSelectCell;
    grdBinAreaMap->Cells[BIN_GRID_COL_AREA][0]=LangT("Area");
    grdBinAreaMap->Cells[BIN_GRID_COL_BIN][0]="Bin";
    grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][0]=LangT("Status");
    grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][0]=LangT("Note");
    grdBinAreaMap->ColWidths[BIN_GRID_COL_AREA]=120;
    grdBinAreaMap->ColWidths[BIN_GRID_COL_BIN]=80;
    grdBinAreaMap->ColWidths[BIN_GRID_COL_STATUS]=100;
    grdBinAreaMap->ColWidths[BIN_GRID_COL_NOTE]=300;
    if(spbBinDefault!=NULL)
        spbBinDefault->Caption=Format(LangT("Default 1-%d"),ARRAYOFCONST((GetBinGridAreaCount())));
    RefreshBinErrorAreaOptions();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::LoadBinMapToGrid()
{
    int Row;
    int Area;
    int Bin;

    if(grdBinAreaMap==NULL)
        return;

    bLoadingBinGrid=true;
    for(Row=1; Row<grdBinAreaMap->RowCount; Row++)
    {
        Area=GetBinGridAreaByRow(Row);
        Bin=BinAreaMap.GetBinByArea(Area);
        grdBinAreaMap->Cells[BIN_GRID_COL_AREA][Row]=BinAreaMap.GetAreaName(Area);
        grdBinAreaMap->Cells[BIN_GRID_COL_BIN][Row]=IntToStr(Bin);
        grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]="";
        grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]="";
    }
    RefreshBinErrorAreaOptions();
    SelectBinErrorArea(BinAreaMap.GetErrorBinArea());
    bLoadingBinGrid=false;
    ValidateBinSettingGrid(false);
    RefreshBinSettingStatus();
}
//---------------------------------------------------------------------------
bool __fastcall TfSetup::SaveBinSettingMap(bool ShowResultMessage)
{
    int Row;
    int Area;
    int Bin;
    bool ValidValue;

    if(grdBinAreaMap==NULL)
        return true;

    if(!ValidateBinSettingGrid(false))
    {
        if(ShowResultMessage)
            ShowMyOKMessageNoStop(LangT("Bin map setting has invalid rows."));
        return false;
    }

    BinAreaMap.Clear();
    for(Row=1; Row<grdBinAreaMap->RowCount; Row++)
    {
        Area=GetBinGridAreaByRow(Row);
        Bin=GetBinGridValue(Row, ValidValue);
        if(ValidValue && Bin>0)
            BinAreaMap.SetBinByArea(Bin, Area);
    }
    BinAreaMap.SetErrorBinArea(GetSelectedBinErrorArea());
    BinAreaMap.SaveDefault();
    //AI(ht160s-bindisplay) 20260706 : the Error Bin area just changed in the live
    //global BinAreaMap. Sort routing reads it live, but the physical bin display
    //color is only repainted by ApplyBinDisplayConfig (via ConfigureBinDisplay at
    //startup / maintenance Apply), so without this the panel red-Auto stayed stale
    //until restart. Re-push per-unit color now so the panel matches the saved map.
    //Lighter than ConfigureBinDisplay (no COM teardown); safe no-op when the
    //controller is absent (ApplyBinDisplayConfig early-returns on BinDisCtrl==NULL).
    EnsureComPortCreated(Application);
    if(fComPort!=NULL)
        fComPort->ApplyBinDisplayConfig();
    RefreshBinSettingStatus();
    RefreshRecipeStatus();
    if(ShowResultMessage)
        ShowMyOKMessageNoStop(LangT("Bin map saved."));
    return true;
}
//---------------------------------------------------------------------------
bool __fastcall TfSetup::ValidateBinSettingGrid(bool ShowResultMessage)
{
    THT160BinAreaMap TempMap;
    int Row;
    int Area;
    int Bin;
    int DuplicateArea;
    int ErrorArea;
    bool ValidValue;
    bool Result=true;
    AnsiString Message;

    if(grdBinAreaMap==NULL)
        return true;

    ErrorArea=GetSelectedBinErrorArea();
    for(Row=1; Row<grdBinAreaMap->RowCount; Row++)
    {
        ResetBinGridRow(Row);
        Area=GetBinGridAreaByRow(Row);
        Bin=GetBinGridValue(Row, ValidValue);
        if(!ValidValue)
        {
            grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]=LangT("Invalid");
            grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]=LangT("Bin must be 0-999. Error code starts from 1000.");
            Result=false;
            continue;
        }
        if(Bin==0)
        {
            if(Area==ErrorArea)
            {
                grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]=LangT("Error");
                grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]="Error bin collection.";
            }
            else
            {
                grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]=LangT("Empty");
                grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]=LangT("Not assigned.");
            }
            continue;
        }
        DuplicateArea=TempMap.GetAreaByBin(Bin);
        if(DuplicateArea!=eHT160BinAreaNotUse && DuplicateArea!=Area)
        {
            grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]=LangT("Duplicate");
            grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]=Format(LangT("Same bin as %s."),ARRAYOFCONST((BinAreaMap.GetAreaName(DuplicateArea))));
            Result=false;
            continue;
        }
        TempMap.SetBinByArea(Bin, Area);
        if(Area==ErrorArea)
        {
            grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]=LangT("OK/Error");
            grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]="Error bin collection.";
        }
        else
        {
            grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]="OK";
            grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]="";
        }
    }
    RefreshBinSettingStatus();
    if(ShowResultMessage)
    {
        Message=Result?AnsiString(LangT("Bin map setting is OK.")):AnsiString(LangT("Bin map setting has invalid rows."));
        ShowMyOKMessageNoStop(LangT(Message));
    }
    return Result;
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::RefreshBinSettingStatus()
{
    int Row;
    int Bin;
    int Count=0;
    bool ValidValue;
    AnsiString CountText;
    AnsiString FileName;

    FileName=BinAreaMap.GetDefaultIniFileName();
    if(grdBinAreaMap!=NULL)
    {
        for(Row=1; Row<grdBinAreaMap->RowCount; Row++)
        {
            Bin=GetBinGridValue(Row, ValidValue);
            if(ValidValue && Bin>0)
                Count++;
        }
    }
    else
        Count=BinAreaMap.GetTotalMappedCount();

    CountText.sprintf("%d / %d", Count, GetBinGridAreaCount());
    if(lblBinMapStatusValue!=NULL)
        lblBinMapStatusValue->Caption=(FileExists(FileName)?AnsiString(LangT("Ready ")):AnsiString(LangT("Not Created ")))+AnsiString("(")+CountText+AnsiString(")");
    if(lblBinRecipeValue!=NULL)
        lblBinRecipeValue->Caption=RecipeManager.GetCurrentRecipeName();
    if(lblBinFileValue!=NULL)
        lblBinFileValue->Caption=FileName;
    if(lblBinMappedValue!=NULL)
        lblBinMappedValue->Caption=CountText;
    if(lblBinColorValue!=NULL)
        lblBinColorValue->Caption=GeneralSetting.bColorBinAreaInstalled?AnsiString(LangT("Installed")):AnsiString(LangT("Not Installed"));
    if(cbbBinErrorArea!=NULL)
        SelectBinErrorArea(GetSelectedBinErrorArea());
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::RefreshBinErrorAreaOptions()
{
    int Area;
    int ErrorArea;
    bool OldLoading;

    if(lblBinColorValue!=NULL)
        lblBinColorValue->Caption=GeneralSetting.bColorBinAreaInstalled?AnsiString(LangT("Installed")):AnsiString(LangT("Not Installed"));
    if(cbbBinErrorArea==NULL)
        return;

    OldLoading=bLoadingBinGrid;
    bLoadingBinGrid=true;
    ErrorArea=GetSelectedBinErrorArea();
    cbbBinErrorArea->Items->Clear();
    for(Area=BIN_GRID_FIRST_AREA; Area<=GetBinGridLastArea(); Area++)
    {
        if(BinAreaMap.IsAreaEnabled(Area))
            cbbBinErrorArea->Items->Add(BinAreaMap.GetAreaName(Area));
    }
    SelectBinErrorArea(ErrorArea);
    bLoadingBinGrid=OldLoading;
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::SelectBinErrorArea(int Area)
{
    AnsiString AreaName;
    int Index;

    if(cbbBinErrorArea==NULL)
        return;

    if(!BinAreaMap.IsAreaEnabled(Area))
        Area=HT160_DEFAULT_ERROR_BIN_AREA;
    AreaName=BinAreaMap.GetAreaName(Area);
    for(Index=0; Index<cbbBinErrorArea->Items->Count; Index++)
    {
        if(cbbBinErrorArea->Items->Strings[Index]==AreaName)
        {
            if(cbbBinErrorArea->ItemIndex!=Index)
                cbbBinErrorArea->ItemIndex=Index;
            return;
        }
    }
    if(cbbBinErrorArea->Items->Count>0 && cbbBinErrorArea->ItemIndex!=0)
        cbbBinErrorArea->ItemIndex=0;
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::SetBinGridDefaultValues(bool SequentialDefault)
{
    int Row;
    int Area;

    if(grdBinAreaMap==NULL)
        return;

    bLoadingBinGrid=true;
    for(Row=1; Row<grdBinAreaMap->RowCount; Row++)
    {
        Area=GetBinGridAreaByRow(Row);
        grdBinAreaMap->Cells[BIN_GRID_COL_AREA][Row]=BinAreaMap.GetAreaName(Area);
        grdBinAreaMap->Cells[BIN_GRID_COL_BIN][Row]=SequentialDefault?IntToStr(Row):AnsiString("0");
        grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]="";
        grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]="";
    }
    bLoadingBinGrid=false;
    ValidateBinSettingGrid(false);
}
//---------------------------------------------------------------------------
int __fastcall TfSetup::GetBinGridAreaByRow(int Row)
{
    int Area=BIN_GRID_FIRST_AREA+Row-1;

    if(Area<BIN_GRID_FIRST_AREA || Area>GetBinGridLastArea())
        return eHT160BinAreaNotUse;
    return Area;
}
//---------------------------------------------------------------------------
int __fastcall TfSetup::GetBinGridAreaCount()
{
    return GetBinGridLastArea()-BIN_GRID_FIRST_AREA+1;
}
//---------------------------------------------------------------------------
int __fastcall TfSetup::GetBinGridLastArea()
{
    if(GeneralSetting.bColorBinAreaInstalled)
        return eHT160BinAreaColor;
    return eHT160BinAreaAuto6;
}
//---------------------------------------------------------------------------
int __fastcall TfSetup::GetSelectedBinErrorArea()
{
    int Area;

    if(cbbBinErrorArea==NULL || cbbBinErrorArea->Text.Trim()==AnsiString(""))
        return HT160_DEFAULT_ERROR_BIN_AREA;
    Area=BinAreaMap.GetAreaByName(cbbBinErrorArea->Text);
    if(!BinAreaMap.IsAreaEnabled(Area))
        Area=HT160_DEFAULT_ERROR_BIN_AREA;
    return Area;
}
//---------------------------------------------------------------------------
int __fastcall TfSetup::GetBinGridValue(int Row, bool &ValidValue)
{
    AnsiString Text;
    int Index;
    int Value;

    ValidValue=false;
    if(grdBinAreaMap==NULL || Row<=0 || Row>=grdBinAreaMap->RowCount)
        return 0;

    Text=grdBinAreaMap->Cells[BIN_GRID_COL_BIN][Row].Trim();
    if(Text==AnsiString(""))
    {
        ValidValue=true;
        return 0;
    }
    for(Index=1; Index<=Text.Length(); Index++)
    {
        if(Text[Index]<'0' || Text[Index]>'9')
            return 0;
    }
    Value=atoi(Text.c_str());
    if(Value<0 || Value>=HT160_BIN_AREA_NORMAL_MAX_BIN)
        return Value;
    ValidValue=true;
    return Value;
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::ResetBinGridRow(int Row)
{
    int Area;

    if(grdBinAreaMap==NULL || Row<=0 || Row>=grdBinAreaMap->RowCount)
        return;
    Area=GetBinGridAreaByRow(Row);
    grdBinAreaMap->Cells[BIN_GRID_COL_AREA][Row]=BinAreaMap.GetAreaName(Area);
    grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]="";
    grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]="";
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::grdBinAreaMapExit(TObject *Sender)
{
    (void)Sender;
    if(!bLoadingBinGrid)
        ValidateBinSettingGrid(false);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::grdBinAreaMapSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{
    (void)Sender;
    //AI(ht160s-lotbin) 20260615 : By Lot+Bin mode binds Auto<->Bin dynamically at run
    //time, so the static Auto->Bin assignment must NOT be edited here (only the Error
    //Bin selection stays usable). Block the Bin column from being focused/edited.
    if(GeneralSetting.bUseLotBinSortMode)
    {
        CanSelect=false;
        return;
    }
    CanSelect=(ARow==0 || ACol==BIN_GRID_COL_BIN);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbBinLoadMapClick(TObject *Sender)
{
    (void)Sender;
    BinAreaMap.LoadDefault();
    LoadBinMapToGrid();
    RefreshRecipeStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbBinSaveMapClick(TObject *Sender)
{
    (void)Sender;
    SaveBinSettingMap(true);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbBinValidateClick(TObject *Sender)
{
    (void)Sender;
    ValidateBinSettingGrid(true);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbBinClearClick(TObject *Sender)
{
    (void)Sender;
    SetBinGridDefaultValues(false);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbBinDefaultClick(TObject *Sender)
{
    (void)Sender;
    SetBinGridDefaultValues(true);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::cbbBinErrorAreaChange(TObject *Sender)
{
    (void)Sender;
    if(!bLoadingBinGrid)
        ValidateBinSettingGrid(false);
}
//---------------------------------------------------------------------------
bool __fastcall TfSetup::IsSystemRunning()
{
    return HSys.Sys.SystemStart;
}
//---------------------------------------------------------------------------
//AI(poka-yoke) 20260616 : run-state lock for recipe operations. While running,
//  disable Use/Delete recipe so the operator cannot change the live recipe
//  mid-run. Pure visual interlock; the existing ShowMessage guards inside the
//  click handlers stay as a harmless backstop (VCL ShowMessage does not stop the
//  machine). Called every cycle from UpdateRunControlFlag, so it self-heals when
//  the machine stops.
void __fastcall TfSetup::UpdateRunStateLock()
{
    bool bRunning;

    bRunning=HSys.Sys.SystemStart;
    if(spbRecipeUse!=NULL)
        spbRecipeUse->Enabled=(bRunning==false);
    if(spbRecipeDelete!=NULL)
        spbRecipeDelete->Enabled=(bRunning==false);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbSetupMenuClick(TObject *Sender)
{
    TSpeedButton *Button;

    Button=dynamic_cast<TSpeedButton *>(Sender);
    if(Button==NULL)
        return;

    SelectSetupPage(Button->Tag);
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbRecipeSaveClick(TObject *Sender)
{
    (void)Sender;
    SaveWorkFile(GetSetUpFileName());
    if(!SaveBinSettingMap(true))
        return;
    RefreshRecipeList();
    RefreshRecipeStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbRecipeSaveAsClick(TObject *Sender)
{
    AnsiString SourceName;
    AnsiString NewName;

    (void)Sender;
    if(edRecipeName->Text.Trim()==AnsiString(""))
    {
        ShowMyOKMessageNoStop(LangT("Please input new recipe name."));
        return;
    }

    SourceName=RecipeManager.GetCurrentRecipeName();
    NewName=RecipeManager.NormalizeRecipeName(edRecipeName->Text);
    if(RecipeManager.RecipeExists(NewName))
    {
        ShowMyOKMessageNoStop(LangT("Recipe already exists."));
        return;
    }

    SaveWorkFile(GetSetUpFileName());
    if(!SaveBinSettingMap(false))
    {
        ShowMyOKMessageNoStop(LangT("Bin map setting is invalid."));
        return;
    }
    if(!RecipeManager.CopyRecipe(SourceName, NewName))
    {
        ShowMyOKMessageNoStop(LangT("Save As recipe failed."));
        return;
    }

    WriteRecipeSetupFile(NewName);
    WriteRecipeManifest(NewName, SourceName);
    RefreshRecipeList();
    SelectRecipeInList(NewName);
    RefreshRecipeStatus();
    ShowMyOKMessageNoStop(LangT("Recipe saved as ")+NewName+AnsiString(LangT(".")));
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbRecipeUseClick(TObject *Sender)
{
    AnsiString Name;

    (void)Sender;
    if(IsSystemRunning())
    {
        ShowMyOKMessageNoStop(LangT("Can not change recipe while machine is running."));
        return;
    }

    Name=GetSelectedRecipeName();
    if(!RecipeManager.RecipeExists(Name))
    {
        ShowMyOKMessageNoStop(LangT("Recipe does not exist."));
        return;
    }

    SaveWorkFile(GetSetUpFileName());
    if(!SaveBinSettingMap(false))
    {
        ShowMyOKMessageNoStop(LangT("Bin map setting is invalid."));
        return;
    }
    RecipeManager.SetCurrentRecipeName(Name);
    RecipeManager.SaveLastRecipeName();
    OpenWorkFile();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbRecipeNewBlankClick(TObject *Sender)
{
    AnsiString Name;
    THT160BinAreaMap EmptyMap;

    (void)Sender;
    if(edRecipeName->Text.Trim()==AnsiString(""))
    {
        ShowMyOKMessageNoStop(LangT("Please input new recipe name."));
        return;
    }

    Name=RecipeManager.NormalizeRecipeName(edRecipeName->Text);
    if(!RecipeManager.CreateRecipe(Name))
    {
        ShowMyOKMessageNoStop(LangT("Create recipe failed or recipe already exists."));
        return;
    }

    WriteRecipeSetupFile(Name);
    WriteRecipeManifest(Name, AnsiString(""));
    WriteDefaultTrayFormSettings(Name);
    EmptyMap.SaveToIni(RecipeManager.GetRecipeFileName(Name, "BinAreaMap.ini"));
    RefreshRecipeList();
    SelectRecipeInList(Name);
    RefreshRecipeStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbRecipeDeleteClick(TObject *Sender)
{
    AnsiString Name;
    int Ret;

    (void)Sender;
    if(IsSystemRunning())
    {
        ShowMyOKMessageNoStop(LangT("Can not delete recipe while machine is running."));
        return;
    }

    Name=GetSelectedRecipeName();
    if(Name.UpperCase()==RecipeManager.GetCurrentRecipeName().UpperCase())
    {
        ShowMyOKMessageNoStop(LangT("Can not delete current recipe."));
        return;
    }

    //AI(general) 20260608 : no Application->MessageBox - use the project message tool.
    Ret=ShowMyMessageBox_YES_NO(AnsiString(LangT("Delete recipe "))+Name+AnsiString(LangT("?")));
    if(Ret!=TMyMessageBox::msgrtnYES)
        return;

    if(!RecipeManager.DeleteRecipe(Name))
    {
        ShowMyOKMessageNoStop(LangT("Delete recipe failed."));
        return;
    }
    RefreshRecipeList();
    RefreshRecipeStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbRecipeRefreshClick(TObject *Sender)
{
    (void)Sender;
    RefreshRecipeList();
    RefreshRecipeStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::lstRecipeClick(TObject *Sender)
{
    (void)Sender;
    edRecipeName->Text=GetSelectedRecipeName();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::TrayFormEditChange(TObject *Sender)
{
    (void)Sender;
    if(bLoadingTrayForm)
        return;
    RefreshTrayFormPreview();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::FormShow(TObject *Sender)
{
    (void)Sender;
    OpenWorkFile();
    LoadSuckEnable();   //AI(ht160s-pnp) 20260626 : reflect machine-level GeneralSetting.bSuckerEnabled[] into the PnP grid
    LayoutSetupButtons();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::FormClose(TObject *Sender, TCloseAction &Action)
{
    (void)Sender;
    (void)Action;
    SaveWorkFile(GetSetUpFileName());
    SaveSuckEnable();   //AI(ht160s-pnp) 20260626 : enforce >=1 nozzle + persist GeneralSetting.bSuckerEnabled[] to General.ini
}
//---------------------------------------------------------------------------
