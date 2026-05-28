//---------------------------------------------------------------------------
#ifndef HLanguageH
#define HLanguageH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <buttons.hpp>
//---------------------------------------------------------------------------
enum HKFLanguage {
    hkChineseTaiwan,
    hkChinese,
    hkEnglish,
    hkFrench,
    hkGerman,
    hkItalian,
    hkSpanish,
    hkSwedish,
    hkThai
};

class PACKAGE HLanguage : public TComponent
{
private:
    HKFLanguage HNowLanguage;
    TForm       *ParentForm;
    AnsiString  HIniFileName;
    bool        FirstRun;
protected:
    AnsiString      GetSegName();
    AnsiString      GetIniFileName();
    void __fastcall SaveAttribute(TWinControl *);
    void __fastcall ReadAttribute(TWinControl *);

public:
    __fastcall HLanguage(TComponent* Owner);
    __fastcall ~HLanguage();
    void __fastcall SetLanguage(HKFLanguage l);

__published:
    __property HKFLanguage Language =
        { read = HNowLanguage,write = SetLanguage,default = hkChinese };
    __property AnsiString IniFileName =
        { read = HIniFileName,write = HIniFileName };
};

//---------------------------------------------------------------------------
extern void PACKAGE SetLanguage(HKFLanguage l);     // 更換所有視窗的語系
extern void PACKAGE UpdateLanguage();               // 更新所有視窗的語系(在自行
                                                    // 建立了視窗後可CALL本函數
                                                    // 更新語系)
extern HKFLanguage PACKAGE GetLanguage();           // 取得目前的語系
#endif
