//---------------------------------------------------------------------------
#ifndef setupH
#define setupH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include "HTray.h"
#include <Graphics.hpp>
//---------------------------------------------------------------------------
enum
{
    MAX_SETUP_MENU_COUNT = 8
};
//---------------------------------------------------------------------------
enum TSetupMenuAction
{
    suShowPage = 0,
    suCloseForm
};
//---------------------------------------------------------------------------
class TfSetup : public TForm
{
__published:
    TPanel *pnlMenu;
    TSpeedButton *spbSetupRecipe;
    TSpeedButton *spbSetupTrayForm;
    TSpeedButton *spbSetupBinSetting;
    TSpeedButton *spbSetupExit;
    TPanel *pnlClient;
    TPanel *pnlTitle;
    TPageControl *pcSetup;
    TTabSheet *tsSetupRecipe;
    TPanel *pnlRecipeHeader;
    TPanel *pnlRecipeBody;
    TLabel *lblCurrentRecipeTitle;
    TLabel *lblCurrentRecipeValue;
    TLabel *lblRecipeDirTitle;
    TLabel *lblRecipeDirValue;
    TLabel *lblSetupFileTitle;
    TLabel *lblSetupFileValue;
    TLabel *lblBinAreaMapTitle;
    TLabel *lblBinAreaMapValue;
    TLabel *lblRecipeNote;
    TLabel *lblRecipeListTitle;
    TListBox *lstRecipe;
    TLabel *lblNewRecipeTitle;
    TEdit *edRecipeName;
    TSpeedButton *spbRecipeSave;
    TSpeedButton *spbRecipeSaveAs;
    TSpeedButton *spbRecipeUse;
    TSpeedButton *spbRecipeNewBlank;
    TSpeedButton *spbRecipeDelete;
    TSpeedButton *spbRecipeRefresh;
    TLabel *lblSetupFileStatusTitle;
    TLabel *lblSetupFileStatusValue;
    TLabel *lblManifestTitle;
    TLabel *lblManifestValue;
    TTabSheet *tsSetupTrayForm;
    TPanel *pnlTrayHeader;
    TPanel *pnlTrayBody;
    TTabSheet *tsSetupBinSetting;
    TPanel *pnlBinHeader;
    TPanel *pnlBinBody;
    TLabel *lblBinPlaceholder;
    TLabel *lblBinMapStatusTitle;
    TLabel *lblBinMapStatusValue;
    TStringGrid *grdBinAreaMap;
    TPanel *pnlBinCommand;
    TLabel *lblBinRecipeTitle;
    TLabel *lblBinRecipeValue;
    TLabel *lblBinFileTitle;
    TLabel *lblBinFileValue;
    TLabel *lblBinMappedTitle;
    TLabel *lblBinMappedValue;
    TLabel *lblBinColorTitle;
    TLabel *lblBinColorValue;
    TLabel *lblBinErrorTitle;
    TComboBox *cbbBinErrorArea;
    TSpeedButton *spbBinLoadMap;
    TSpeedButton *spbBinSaveMap;
    TSpeedButton *spbBinValidate;
    TSpeedButton *spbBinClear;
    TSpeedButton *spbBinDefault;
    TPanel *Panel27;
    TImage *Image3;
    TLabel *Label20;
    TLabel *Label39;
    TLabel *Label41;
    TLabel *Label42;
    TLabel *Label43;
    TLabel *Label44;
    TLabel *labTrayForm;
    TEdit *edXStart;
    TEdit *edXPitch;
    TEdit *edYStart;
    TEdit *edYPitch;
    TEdit *edXDivision;
    TEdit *edYDivision;
    TTMyTray *TMyTray1;
    void __fastcall spbSetupMenuClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall spbRecipeSaveClick(TObject *Sender);
    void __fastcall spbRecipeSaveAsClick(TObject *Sender);
    void __fastcall spbRecipeUseClick(TObject *Sender);
    void __fastcall spbRecipeNewBlankClick(TObject *Sender);
    void __fastcall spbRecipeDeleteClick(TObject *Sender);
    void __fastcall spbRecipeRefreshClick(TObject *Sender);
    void __fastcall lstRecipeClick(TObject *Sender);
    void __fastcall TrayFormEditChange(TObject *Sender);
    void __fastcall grdBinAreaMapExit(TObject *Sender);
    void __fastcall grdBinAreaMapSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);
    void __fastcall spbBinLoadMapClick(TObject *Sender);
    void __fastcall spbBinSaveMapClick(TObject *Sender);
    void __fastcall spbBinValidateClick(TObject *Sender);
    void __fastcall spbBinClearClick(TObject *Sender);
    void __fastcall spbBinDefaultClick(TObject *Sender);
    void __fastcall cbbBinErrorAreaChange(TObject *Sender);
private:
    int iSetupMenuCount;
    TSpeedButton *MenuButtons[MAX_SETUP_MENU_COUNT];
    TTabSheet *MenuPages[MAX_SETUP_MENU_COUNT];
    TSetupMenuAction MenuActions[MAX_SETUP_MENU_COUNT];
    bool MenuBottomPins[MAX_SETUP_MENU_COUNT];
    TSpeedButton *LastClickButton;
    bool bLoadingTrayForm;
    bool bLoadingBinGrid;

    void __fastcall RegisterSetupPages();
    void __fastcall LayoutSetupButtons();
    void __fastcall SelectSetupPage(int PageIndex);
    void __fastcall RefreshRecipeStatus();
    void __fastcall RefreshRecipeList();
    void __fastcall SelectRecipeInList(AnsiString RecipeName);
    AnsiString __fastcall GetSelectedRecipeName();
    void __fastcall WriteRecipeSetupFile(AnsiString RecipeName);
    void __fastcall WriteRecipeManifest(AnsiString RecipeName, AnsiString SourceRecipeName);
    void __fastcall BindTrayFormEvents();
    void __fastcall LoadTrayFormSettings(AnsiString RecipeName);
    void __fastcall SaveTrayFormSettings(AnsiString RecipeName);
    void __fastcall WriteDefaultTrayFormSettings(AnsiString RecipeName);
    void __fastcall RefreshTrayFormPreview();
    int __fastcall GetTrayEditInt(TEdit *Edit, int DefaultValue, int MinValue, int MaxValue);
    double __fastcall GetTrayEditDouble(TEdit *Edit, double DefaultValue);
    AnsiString __fastcall FormatTrayDouble(double Value);
    void __fastcall BuildBinSettingUI();
    void __fastcall ConfigureBinSettingGrid();
    void __fastcall LoadBinMapToGrid();
    bool __fastcall SaveBinSettingMap(bool ShowResultMessage);
    bool __fastcall ValidateBinSettingGrid(bool ShowResultMessage);
    void __fastcall RefreshBinSettingStatus();
    void __fastcall RefreshBinErrorAreaOptions();
    void __fastcall SelectBinErrorArea(int Area);
    void __fastcall SetBinGridDefaultValues(bool SequentialDefault);
    int __fastcall GetBinGridAreaByRow(int Row);
    int __fastcall GetBinGridAreaCount();
    int __fastcall GetBinGridLastArea();
    int __fastcall GetSelectedBinErrorArea();
    int __fastcall GetBinGridValue(int Row, bool &ValidValue);
    void __fastcall ResetBinGridRow(int Row);
    bool __fastcall IsSystemRunning();
public:
    __fastcall TfSetup(TComponent* Owner);
    AnsiString __fastcall GetSetUpFileName();
    void __fastcall OpenWorkFile();
    void __fastcall SaveWorkFile(AnsiString S);
    void __fastcall UpdateRunStateLock();
};
//---------------------------------------------------------------------------
extern PACKAGE TfSetup *fSetup;
//---------------------------------------------------------------------------
#endif
