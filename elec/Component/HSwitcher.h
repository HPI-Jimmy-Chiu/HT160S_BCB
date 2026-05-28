//---------------------------------------------------------------------------
#ifndef HSwitcherH
#define HSwitcherH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include "hsensor.h"

//---------------------------------------------------------------------------
class PACKAGE HSwitcher : public HSensor
{
private:

protected:
    void __fastcall SetIOPort2(AnsiString s);

public:
    __fastcall HSwitcher(TComponent* Owner);
    void __fastcall SetValue(bool Var);
    bool __fastcall ReadValue();
    
    operator = (bool t) {               // ³]­È¥Î¨ç¼Æ
        SetValue(t);
    }
    __property  bool Value =
        { read=ReadValue,write=SetValue };

__published:
    __property  AOnCreate;
    __property  AnsiString IOPort =
        { read = sIOPort,write = SetIOPort2 } ;
};
//---------------------------------------------------------------------------
#endif
