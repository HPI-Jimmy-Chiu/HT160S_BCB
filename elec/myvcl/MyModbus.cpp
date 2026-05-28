//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MyModbus.h"
#pragma package(smart_init)
#pragma resource "MyModbus.res"
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TTMyModbus *)
{
    new TTMyModbus(NULL);
}
//---------------------------------------------------------------------------
__fastcall TTMyModbus::TTMyModbus(TComponent* Owner)
    : TCustomControl(Owner)
{
}
//---------------------------------------------------------------------------
namespace Mymodbus
{
    void __fastcall PACKAGE Register()
    {
         TComponentClass classes[1] = {__classid(TTMyModbus)};
         RegisterComponents("lee40", classes, 0);
    }
}
//---------------------------------------------------------------------------
 