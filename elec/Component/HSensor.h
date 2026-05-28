//---------------------------------------------------------------------------
#ifndef HSensorH
#define HSensorH

//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include "iobit.h"

//---------------------------------------------------------------------------
#define     ON      true
#define     OFF     false

//---------------------------------------------------------------------------
extern bool GetPortAndBitNumber(char *,int &,int &,bool &);

//---------------------------------------------------------------------------
class PACKAGE HSensor : public TComponent
{
protected:
	Classes::TNotifyEvent aOnCreate;
    IOBit       *IOBitObj;

    void virtual __fastcall SetIOPort(AnsiString s);

public:
    __fastcall HSensor(TComponent* Owner);
    __fastcall virtual ~HSensor();
    bool       Stat();

    operator   == (bool t) {              // PORTª¬ºA¹Bºâ¤l
        return Stat() == t;
    }

protected:
    AnsiString   sIOPort;

__published:
    __property Classes::TNotifyEvent AOnCreate = {read=aOnCreate, write=aOnCreate};
    __property  AnsiString IOPort =
        { read = sIOPort,write = SetIOPort } ;
};
//---------------------------------------------------------------------------
#endif
