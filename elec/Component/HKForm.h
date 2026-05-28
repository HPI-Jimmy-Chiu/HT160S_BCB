//---------------------------------------------------------------------------
#ifndef HKFormH
#define HKFormH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <buttons.hpp>
//---------------------------------------------------------------------------
enum HKFLanguage {
    hkChinese,
    hkEnglish
};

class PACKAGE HKForm : public TComponent
{
private:
    HKFLanguage HLanguage;
    TForm       *ParentForm;
    AnsiString  HIniFileName;
    bool        FirstRun;
protected:
    AnsiString      GetSegName();
    AnsiString      GetIniFileName();
    void __fastcall SaveAttribute();
    void __fastcall ReadAttribute();

public:
    __fastcall HKForm(TComponent* Owner);
    __fastcall ~HKForm();
    void __fastcall SetLanguage(HKFLanguage l);

__published:
    __property HKFLanguage Language =
        { read = HLanguage,write = SetLanguage,default = hkChinese };
    __property AnsiString IniFileName =
        { read = HIniFileName,write = HIniFileName };
};

//---------------------------------------------------------------------------
extern void PACKAGE SetLanguage(HKFLanguage l);     // 更換所有視窗的語系
extern void PACKAGE UpdateLanguage();               // 更新所有視窗的語系(在自行
                                                    // 建立了視窗後可CALL本函數
                                                    // 更新語系)
#endif
