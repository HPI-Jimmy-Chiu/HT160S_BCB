//---------------------------------------------------------------------------
// 程式名稱:.H
// 程式設計:
// 版權所有:鴻鎧資訊有限公司 HungKai Co.LTD
// 建立日期:98/09/29 PM 01:32:45
//---------------------------------------------------------------------------
#ifndef LangTstMainH
#define LangTstMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "MLang.h"
#include "HLanguage.h"
//---------------------------------------------------------------------------
class PACKAGE TMLanguage1 : public TMLanguage
{
__published: // IDE加入元件使用區
    TLabel *Label1;
private:     // 使用者私用宣告區
protected:   // 使用者保護宣告區
public:      // 使用者公用宣告區
    __fastcall TMLanguage1(TComponent* Owner);
__published: // 使用者性質自定宣告區
};
//---------------------------------------------------------------------------
extern PACKAGE TMLanguage1 *MLanguage1;
//---------------------------------------------------------------------------
#endif