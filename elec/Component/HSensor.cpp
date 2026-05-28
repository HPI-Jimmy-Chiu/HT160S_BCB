//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "HSensor.h"
#include <assert.h>

#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(HSensor *)
{
    new HSensor(NULL);
}
//---------------------------------------------------------------------------
__fastcall HSensor::HSensor(TComponent* Owner)
    : TComponent(Owner)
{
    IOBitObj = NULL;
}
//---------------------------------------------------------------------------
__fastcall HSensor::~HSensor()
{
    delete IOBitObj;
}

//---------------------------------------------------------------------------
//  讀取字串,配置I/O PORT
//---------------------------------------------------------------------------
void __fastcall HSensor::SetIOPort(AnsiString s)
{
    int iBit,iPort;
    bool Read;
    try {
        if (GetPortAndBitNumber(s.c_str(),iPort,iBit,Read)) {
            if (IOBitObj != NULL)
                delete IOBitObj;
            IOBitObj = new IOBit(iPort,iBit,true);
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
//  目前Port狀態
//---------------------------------------------------------------------------
bool HSensor::Stat()
{
    assert(IOBitObj != NULL);

    return IOBitObj->In();

}
//---------------------------------------------------------------------------
namespace Hsensor
{
    void __fastcall PACKAGE Register()
    {
        TComponentClass classes[1] = {__classid(HSensor)};
        RegisterComponents("HungKai", classes, 0);
    }
}
//---------------------------------------------------------------------------
