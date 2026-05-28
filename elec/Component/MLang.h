//---------------------------------------------------------------------------
// 程式名稱:.H
// 程式設計:
// 版權所有:鴻鎧資訊有限公司 HungKai Co.LTD
// 建立日期:98/09/28 PM 04:30:57
//---------------------------------------------------------------------------
#ifndef MLangH
#define MLangH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "HLanguage.h"

//---------------------------------------------------------------------------
class PACKAGE TMLanguage : public TForm
{
__published: // IDE加入元件使用區
private:     // 使用者私用宣告區
    HLanguage *HLanguageSet;
    
    HKFLanguage __fastcall HGetLanguage();
    AnsiString  __fastcall HGetIniName();
    void __fastcall HSetLanguage(HKFLanguage l);
    void __fastcall HSetIniName(AnsiString s);

protected:   // 使用者保護宣告區
public:      // 使用者公用宣告區
    __fastcall TMLanguage(TComponent* Owner);
__published: // 使用者性質自定宣告區
    __property HKFLanguage Language =
        { read = HGetLanguage,write = HSetLanguage,default = hkChinese };
    __property AnsiString IniFileName =
        { read = HGetIniName,write = HSetIniName };
};
//---------------------------------------------------------------------------
extern PACKAGE TMLanguage *MLanguage;
//---------------------------------------------------------------------------
#endif