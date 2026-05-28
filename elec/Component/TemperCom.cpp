//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TemperCom.h"
#pragma link "SPComm"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TTemperCom *)
{
    new TTemperCom(NULL);
}
//---------------------------------------------------------------------------
__fastcall TTemperCom::TTemperCom(TComponent* Owner)
    : TComm(Owner)
{
}
//---------------------------------------------------------------------------
namespace Tempercom
{
    void __fastcall PACKAGE Register()
    {
         TComponentClass classes[1] = {__classid(TTemperCom)};
         RegisterComponents("HonTech", classes, 0);
    }
}
//---------------------------------------------------------------------------
