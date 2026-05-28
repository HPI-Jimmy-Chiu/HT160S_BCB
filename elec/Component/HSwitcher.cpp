//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "HSwitcher.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(HSwitcher *)
{
    new HSwitcher(NULL);
}

//---------------------------------------------------------------------------
__fastcall HSwitcher::HSwitcher(TComponent* Owner)
    : HSensor(Owner)
{
}

//---------------------------------------------------------------------------
//  讀取字串,配置I/O PORT
//---------------------------------------------------------------------------
void __fastcall HSwitcher::SetIOPort2(AnsiString s)
{
    int iBit,iPort;
    bool Read;

    try {
        if (GetPortAndBitNumber(s.c_str(),iPort,iBit,Read)) {
            if (IOBitObj != NULL)
                delete IOBitObj;
            IOBitObj = new IOBit(iPort,iBit,false);
            sIOPort = s;
        }
        if (aOnCreate != NULL)
            aOnCreate(this);
    }
    catch (...) {
        AnsiString ms = "無法配置PORT:" + s;
        ShowMessage(ms);
    }
}

//---------------------------------------------------------------------------
//  設定PORT值
//---------------------------------------------------------------------------
void __fastcall HSwitcher::SetValue(bool Var)
{
    if (IOBitObj != NULL) {
        IOBitObj->Out(Var);
    }
    else {
        Application->MessageBox("請先設定I/O PORT","錯誤",MB_ICONINFORMATION | MB_OK);
    }
}

//---------------------------------------------------------------------------
//  取得PORT值
//---------------------------------------------------------------------------
bool __fastcall HSwitcher::ReadValue()
{
    if (IOBitObj != NULL) {
        return IOBitObj->In();
    }
    return false;
}

//---------------------------------------------------------------------------
namespace Hswitcher
{
    void __fastcall PACKAGE Register()
    {
        TComponentClass classes[1] = {__classid(HSwitcher)};
        RegisterComponents("HungKai", classes, 0);
    }
}
//---------------------------------------------------------------------------
 