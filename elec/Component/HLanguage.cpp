//---------------------------------------------------------------------------
#include <vcl.h>
#include <string.h>
#pragma hdrstop

#include "HLanguage.h"
#include "inifiles.hpp"
#include "assert.h"
#pragma package(smart_init)


//---------------------------------------------------------------------------
static TList       *HKList = NULL;                  // 記錄目前所有使用中的物件串列
static HKFLanguage  NowLanguage = hkChineseTaiwan;  // 目前的語系

//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//
static inline void ValidCtrCheck(HLanguage *)
{
    new HLanguage(NULL);
}

//---------------------------------------------------------------------------
//  建構子
//---------------------------------------------------------------------------
__fastcall HLanguage::HLanguage(TComponent* Owner)
    : TComponent(Owner)
{
    FirstRun = true;

    // 找出父視窗的指標 ###############################
    ParentForm = NULL;
    TComponent *P = Owner;

    while (true) {
        if (P == NULL)
            break;
        ParentForm = dynamic_cast<TForm *>(P);
        if (ParentForm != NULL)
            break;
        P = P->Owner;
    }

    HIniFileName = ParentForm->Name;
    HNowLanguage = hkChineseTaiwan;

    try {
        if (HKList == NULL)
            HKList = new TList;

        HKList->Add((void *) this);
    }
    catch (...) {
        ShowMessage("HLanguage元件:無法建立HKList串列 .....");
    }
}

//---------------------------------------------------------------------------
//  解構子
//---------------------------------------------------------------------------
__fastcall HLanguage::~HLanguage()
{
    for (int iP = 0;iP < HKList->Count;iP ++) {
        if ((HLanguage *) HKList->Items[iP] == this) {
            HKList->Delete(iP);
        }
    }

    if (HKList->Count == 0) {
        delete HKList;
        HKList = NULL;
    }
}

//---------------------------------------------------------------------------
//  設定語系
//---------------------------------------------------------------------------
void __fastcall HLanguage::SetLanguage(HKFLanguage l)
{
    assert(l == hkChineseTaiwan ||
           l == hkChinese ||
           l == hkEnglish ||
           l == hkFrench  ||
           l == hkGerman  ||
           l == hkItalian ||
           l == hkSpanish ||
           l == hkSwedish ||
           l == hkThai);

    if (ComponentState.Contains(csDesigning) &&         // 如果是在設計模式
        !FirstRun) {
        AnsiString s;
        s = "是否將目前的屬性存成:" + GetSegName() + "語系.";
        if (Application->MessageBox(s.c_str(),"存檔?",MB_OKCANCEL) == IDOK)
            SaveAttribute(ParentForm);
    }
    if (FirstRun)
        FirstRun = false;
    HNowLanguage = l;
    ReadAttribute(ParentForm);
}


//---------------------------------------------------------------------------
//  取得段落名稱
//---------------------------------------------------------------------------
AnsiString HLanguage::GetSegName()
{
    AnsiString SegName;
    SegName = ParentForm->Name.UpperCase();

    switch (HNowLanguage)
    {
        case hkChineseTaiwan:
            SegName += "_CHINESE_TAIWAN";
            break;
        case hkChinese:
            SegName += "_CHINESE";
            break;
        case hkEnglish:
            SegName += "_ENGLISH";
            break;
        case hkFrench:
            SegName += "_FRENCH";
            break;
        case hkGerman:
            SegName += "_GERMAN";
            break;
        case hkItalian:
            SegName += "_ITALIAN";
            break;
        case hkSpanish:
            SegName += "_SPANISH";
            break;
        case hkSwedish:
            SegName += "_SWEDISH";
            break;
        case hkThai:
            SegName += "_THAI";
            break;
    }
    return SegName;
}

//---------------------------------------------------------------------------
//  取得ini檔名
//---------------------------------------------------------------------------
AnsiString HLanguage::GetIniFileName()
{
    AnsiString s;
    char cBuf[256];

  //  GetWindowsDirectory(cBuf,sizeof(cBuf));
    s=GetCurrentDir();
    strcpy(cBuf,s.c_str());
    strcat(cBuf,"\\");
    s = cBuf + HIniFileName;
    s = ChangeFileExt(s,".INI");

    return s;
}

/* *******************************************
    C++ Builder的 VCL元件從屬關係,所以如果
    要讀取或寫入屬性,必需判斷是否為主物件,
    或是附屬物件,下列程式中,如果是判斷該物
    件為主物件者,除了記錄或讀取該物件的屬性
    外,函數會再CALL自已(遞迴呼叫)來處理其
    它的附屬物件,所以程式會由視窗物件開始找
    出所有的附屬物件.

    如果需要加上其它物件者,必需以上列方式
    處理,否則可能會有轉換不完整的現象.

    PS.所有物件的型別確認,使用C++的RTTI機
       制所提供的dynamic_cast<>作指標轉換
       dynamic_cast<>如果可以成功轉換者,會
       傳回該物件的指標,否則會傳回NULL.
 ********************************************* */

//---------------------------------------------------------------------------
//  屬性存檔
//      PCtrl:  父類別視窗(Form,Panel ...)
//---------------------------------------------------------------------------
void __fastcall HLanguage::SaveAttribute(TWinControl *PCtrl)
{
    assert(PCtrl != NULL);

    AnsiString SegName = GetSegName();

    TIniFile *IFile = new TIniFile(GetIniFileName());

    // 先將主物件的資料寫入 ......................
    TPanel    *PanelPtr  = dynamic_cast <TPanel *>(PCtrl);
    TTabSheet *ShtPtr    = dynamic_cast <TTabSheet *>(PCtrl);
    TForm     *FrmPtr    = dynamic_cast <TForm *>(PCtrl);
    TGroupBox *PGroupBox = dynamic_cast <TGroupBox *>(PCtrl);

    if (PanelPtr != NULL)
        IFile->WriteString(SegName,PanelPtr->Name,PanelPtr->Caption);
    if (ShtPtr != NULL)
        IFile->WriteString(SegName,ShtPtr->Name,ShtPtr->Caption);
    if (FrmPtr != NULL)
        IFile->WriteString(SegName,FrmPtr->Name,FrmPtr->Caption);
    if (PGroupBox  != NULL)
        IFile->WriteString(SegName,PGroupBox->Name,PGroupBox->Caption);

    // 再將附屬物件的資料寫入 .....................
    for (int iP = 0;iP < PCtrl->ControlCount;iP ++) {
        TControl *P = PCtrl->Controls[iP];

        if (dynamic_cast <TPanel *>(P) != NULL ||
            dynamic_cast <TPageControl *>(P) != NULL ||
            dynamic_cast <TTabSheet *>(P) != NULL ||
            dynamic_cast <TTabControl *>(P) != NULL ||
            dynamic_cast <TForm *>(P) != NULL ||
            dynamic_cast <THeader *>(P) != NULL ||
            dynamic_cast <TPage *>(P) != NULL ||
            dynamic_cast <TGroupBox*>(P) != NULL) {
            SaveAttribute((TWinControl *) P);       // 找該物件附屬的物件(遞迴)
        }

        TLabel *PLabel               = dynamic_cast <TLabel *>(P);
        TSpeedButton    *PSpeedButton = dynamic_cast <TSpeedButton *>(P);
        TButton         *PButton      = dynamic_cast <TButton *>(P);
        TCheckBox       *PCheckBox    = dynamic_cast <TCheckBox *>(P);
        TRadioButton    *PRadioButton = dynamic_cast <TRadioButton *>(P);
        TCustomEdit     *PCustomEdit  = dynamic_cast <TCustomEdit *>(P);

        if (PLabel != NULL)                 // 如果是TLabel型態
            IFile->WriteString(SegName,P->Name,PLabel->Caption);
        if (PSpeedButton != NULL)           // 如果是TSpeedButton型態
            IFile->WriteString(SegName,P->Name,PSpeedButton->Caption);
        if (PButton != NULL)                // 如果是TButton型態
            IFile->WriteString(SegName,P->Name,PButton->Caption);
        if (PCheckBox != NULL)              // 如果是TCheckBox型態
            IFile->WriteString(SegName,P->Name,PCheckBox->Caption);
        if (PRadioButton != NULL)           // 如果是TRadioButton型態
            IFile->WriteString(SegName,P->Name,PRadioButton->Caption);
        if (PCustomEdit != NULL)            // 如果是TCustomEdit型態
            IFile->WriteString(SegName,P->Name,PCustomEdit->Text);
    }
    delete IFile;
}

//---------------------------------------------------------------------------
//  屬性存檔
//---------------------------------------------------------------------------
void __fastcall HLanguage::ReadAttribute(TWinControl *PCtrl)
{
    assert(PCtrl != NULL);

    AnsiString SegName = GetSegName();

    TIniFile *IFile = new TIniFile(GetIniFileName());

    // 先將主物件的資料讀出 ...................
    TPanel      *PanelPtr  = dynamic_cast <TPanel *>(PCtrl);
    TTabSheet   *ShtPtr    = dynamic_cast <TTabSheet *>(PCtrl);
    TForm       *FrmPtr    = dynamic_cast <TForm *>(PCtrl);
    TGroupBox   *PGroupBox = dynamic_cast <TGroupBox *>(PCtrl);

    if (PanelPtr != NULL)
        PanelPtr->Caption = IFile->ReadString(SegName,PanelPtr->Name,PanelPtr->Caption);
    if (ShtPtr != NULL)
        ShtPtr->Caption = IFile->ReadString(SegName,ShtPtr->Name,ShtPtr->Caption);
    if (FrmPtr != NULL)
        FrmPtr->Caption = IFile->ReadString(SegName,FrmPtr->Name,FrmPtr->Caption);
    if (PGroupBox != NULL)
        PGroupBox->Caption = IFile->ReadString(SegName,PGroupBox->Name,PGroupBox->Caption);

    // 再將附屬物件的資料讀出 .....................
    for (int iP = 0;iP < PCtrl->ControlCount;iP ++) {
        TControl *P = PCtrl->Controls[iP];

        if (dynamic_cast <TPanel *>(P) != NULL ||
            dynamic_cast <TPageControl *>(P) != NULL ||
            dynamic_cast <TTabSheet *>(P) != NULL ||
            dynamic_cast <TTabControl *>(P) != NULL ||
            dynamic_cast <TForm *>(P) != NULL ||
            dynamic_cast <THeader *>(P) != NULL ||
            dynamic_cast <TPage *>(P) != NULL ||
            dynamic_cast <TGroupBox *>(P) != NULL) {

            ReadAttribute((TWinControl *) P);       // 找該物件附屬的物件(遞迴)
        }

        TLabel          *PLabel       = dynamic_cast <TLabel *>(P);
        TSpeedButton    *PSpeedButton = dynamic_cast <TSpeedButton *>(P);
        TButton         *PButton      = dynamic_cast <TButton *>(P);
        TCheckBox       *PCheckBox    = dynamic_cast <TCheckBox *>(P);
        TRadioButton    *PRadioButton = dynamic_cast <TRadioButton *>(P);
        TCustomEdit    *PCustomEdit   = dynamic_cast <TCustomEdit *>(P);

        if (PLabel != NULL)                 // 如果是TLabel型態
            PLabel->Caption = IFile->ReadString(SegName,P->Name,PLabel->Caption);
        if (PSpeedButton != NULL)           // 如果是TSpeedButton型態
            PSpeedButton->Caption = IFile->ReadString(SegName,P->Name,PSpeedButton->Caption);
        if (PButton != NULL)                // 如果是TButton型態
            PButton->Caption = IFile->ReadString(SegName,P->Name,PButton->Caption);
        if (PCheckBox != NULL)              // 如果是TCheckBox型態
            PCheckBox->Caption = IFile->ReadString(SegName,P->Name,PCheckBox->Caption);
        if (PRadioButton != NULL)           // 如果是TRadioButton型態
            PRadioButton->Caption = IFile->ReadString(SegName,P->Name,PRadioButton->Caption);
        if (PCustomEdit != NULL)            // 如果是TCustomEdit型態
            PCustomEdit->Text = IFile->ReadString(SegName,P->Name,PCustomEdit->Text);
    }
    delete IFile;
}

//---------------------------------------------------------------------------
//  更換所有視窗的語系
//      l:要設定的語系
//---------------------------------------------------------------------------
void SetLanguage(HKFLanguage l)
{
    NowLanguage = l;
    for (int iP = 0;iP < HKList->Count;iP ++) {
        HLanguage *P = (HLanguage *) HKList->Items[iP];
        P->SetLanguage(l);
    }
}

//---------------------------------------------------------------------------
//  更新所有視窗的語系(在自行建立了視窗後可CALL本函數更新語系)
//---------------------------------------------------------------------------
void UpdateLanguage()
{
    for (int iP = 0;iP < HKList->Count;iP ++) {
        HLanguage *P = (HLanguage *) HKList->Items[iP];
        P->SetLanguage(NowLanguage);
    }
}

//---------------------------------------------------------------------------
//  取得目前的語系
//---------------------------------------------------------------------------
HKFLanguage GetLanguage()
{
    return NowLanguage;
}

//---------------------------------------------------------------------------
namespace Hlanguage
{
    void __fastcall PACKAGE Register()
    {
        TComponentClass classes[1] = {__classid(HLanguage)};
        RegisterComponents("HungKai", classes, 0);
    }
}
//---------------------------------------------------------------------------
