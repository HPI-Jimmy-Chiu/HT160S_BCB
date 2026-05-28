//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "setup.h"
#include "CosFunction.h"
#include "database.h"
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
    lblSetupFileStatusValue->Caption=FileExists(SetupFileName)?AnsiString("Ready"):AnsiString("Not Created");
    lblBinMapStatusValue->Caption=FileExists(BinAreaFileName)?AnsiString("Ready"):AnsiString("Not Created");
    lblManifestValue->Caption=FileExists(ManifestFileName)?AnsiString("Ready"):AnsiString("Not Created");
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
    XDivision=GetTrayEditInt(edXDivision, 1, 1, 99);
    YDivision=GetTrayEditInt(edYDivision, 1, 1, 99);
    Ini=new TIniFile(FileName);
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "XStart", GetTrayEditDouble(edXStart, 0.0));
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "XPitch", GetTrayEditDouble(edXPitch, 1.0));
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "YStart", GetTrayEditDouble(edYStart, 0.0));
    Ini->WriteFloat(TRAY_FORM_INI_GROUP, "YPitch", GetTrayEditDouble(edYPitch, 1.0));
    Ini->WriteInteger(TRAY_FORM_INI_GROUP, "XDivision", XDivision);
    Ini->WriteInteger(TRAY_FORM_INI_GROUP, "YDivision", YDivision);
    delete Ini;
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

    XDivision=GetTrayEditInt(edXDivision, 1, 1, 99);
    YDivision=GetTrayEditInt(edYDivision, 1, 1, 99);
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
    grdBinAreaMap->Cells[BIN_GRID_COL_AREA][0]="Area";
    grdBinAreaMap->Cells[BIN_GRID_COL_BIN][0]="Bin";
    grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][0]="Status";
    grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][0]="Note";
    grdBinAreaMap->ColWidths[BIN_GRID_COL_AREA]=120;
    grdBinAreaMap->ColWidths[BIN_GRID_COL_BIN]=80;
    grdBinAreaMap->ColWidths[BIN_GRID_COL_STATUS]=100;
    grdBinAreaMap->ColWidths[BIN_GRID_COL_NOTE]=300;
    if(spbBinDefault!=NULL)
        spbBinDefault->Caption=AnsiString("Default 1-")+IntToStr(GetBinGridAreaCount());
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
            ShowMessage("Bin map setting has invalid rows.");
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
    RefreshBinSettingStatus();
    RefreshRecipeStatus();
    if(ShowResultMessage)
        ShowMessage("Bin map saved.");
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
            grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]="Invalid";
            grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]="Bin must be 0-999. Error code starts from 1000.";
            Result=false;
            continue;
        }
        if(Bin==0)
        {
            if(Area==ErrorArea)
            {
                grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]="Error";
                grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]="Error bin collection.";
            }
            else
            {
                grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]="Empty";
                grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]="Not assigned.";
            }
            continue;
        }
        DuplicateArea=TempMap.GetAreaByBin(Bin);
        if(DuplicateArea!=eHT160BinAreaNotUse && DuplicateArea!=Area)
        {
            grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]="Duplicate";
            grdBinAreaMap->Cells[BIN_GRID_COL_NOTE][Row]=AnsiString("Same bin as ")+BinAreaMap.GetAreaName(DuplicateArea)+AnsiString(".");
            Result=false;
            continue;
        }
        TempMap.SetBinByArea(Bin, Area);
        if(Area==ErrorArea)
        {
            grdBinAreaMap->Cells[BIN_GRID_COL_STATUS][Row]="OK/Error";
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
        Message=Result?AnsiString("Bin map setting is OK."):AnsiString("Bin map setting has invalid rows.");
        ShowMessage(Message);
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
        lblBinMapStatusValue->Caption=(FileExists(FileName)?AnsiString("Ready "):AnsiString("Not Created "))+AnsiString("(")+CountText+AnsiString(")");
    if(lblBinRecipeValue!=NULL)
        lblBinRecipeValue->Caption=RecipeManager.GetCurrentRecipeName();
    if(lblBinFileValue!=NULL)
        lblBinFileValue->Caption=FileName;
    if(lblBinMappedValue!=NULL)
        lblBinMappedValue->Caption=CountText;
    if(lblBinColorValue!=NULL)
        lblBinColorValue->Caption=CosFunction.bColorBinAreaInstalled?AnsiString("Installed"):AnsiString("Not Installed");
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
        lblBinColorValue->Caption=CosFunction.bColorBinAreaInstalled?AnsiString("Installed"):AnsiString("Not Installed");
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
    if(CosFunction.bColorBinAreaInstalled)
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
        ShowMessage("Please input new recipe name.");
        return;
    }

    SourceName=RecipeManager.GetCurrentRecipeName();
    NewName=RecipeManager.NormalizeRecipeName(edRecipeName->Text);
    if(RecipeManager.RecipeExists(NewName))
    {
        ShowMessage("Recipe already exists.");
        return;
    }

    SaveWorkFile(GetSetUpFileName());
    if(!SaveBinSettingMap(false))
    {
        ShowMessage("Bin map setting is invalid.");
        return;
    }
    if(!RecipeManager.CopyRecipe(SourceName, NewName))
    {
        ShowMessage("Save As recipe failed.");
        return;
    }

    WriteRecipeSetupFile(NewName);
    WriteRecipeManifest(NewName, SourceName);
    RefreshRecipeList();
    SelectRecipeInList(NewName);
    RefreshRecipeStatus();
    ShowMessage("Recipe saved as "+NewName+AnsiString("."));
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::spbRecipeUseClick(TObject *Sender)
{
    AnsiString Name;

    (void)Sender;
    if(IsSystemRunning())
    {
        ShowMessage("Can not change recipe while machine is running.");
        return;
    }

    Name=GetSelectedRecipeName();
    if(!RecipeManager.RecipeExists(Name))
    {
        ShowMessage("Recipe does not exist.");
        return;
    }

    SaveWorkFile(GetSetUpFileName());
    if(!SaveBinSettingMap(false))
    {
        ShowMessage("Bin map setting is invalid.");
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
        ShowMessage("Please input new recipe name.");
        return;
    }

    Name=RecipeManager.NormalizeRecipeName(edRecipeName->Text);
    if(!RecipeManager.CreateRecipe(Name))
    {
        ShowMessage("Create recipe failed or recipe already exists.");
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
        ShowMessage("Can not delete recipe while machine is running.");
        return;
    }

    Name=GetSelectedRecipeName();
    if(Name.UpperCase()==RecipeManager.GetCurrentRecipeName().UpperCase())
    {
        ShowMessage("Can not delete current recipe.");
        return;
    }

    Ret=Application->MessageBox((AnsiString("Delete recipe ")+Name+AnsiString("?")).c_str(), "Recipe", MB_YESNO | MB_ICONQUESTION);
    if(Ret!=IDYES)
        return;

    if(!RecipeManager.DeleteRecipe(Name))
    {
        ShowMessage("Delete recipe failed.");
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
    LayoutSetupButtons();
}
//---------------------------------------------------------------------------
void __fastcall TfSetup::FormClose(TObject *Sender, TCloseAction &Action)
{
    (void)Sender;
    (void)Action;
    SaveWorkFile(GetSetUpFileName());
}
//---------------------------------------------------------------------------
