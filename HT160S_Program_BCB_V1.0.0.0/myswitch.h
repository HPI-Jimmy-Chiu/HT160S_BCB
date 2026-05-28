//---------------------------------------------------------------------------
#ifndef myswitchH
#define myswitchH
//---------------------------------------------------------------------------
#include "myio.h"
//---------------------------------------------------------------------------
class TMySwitch
{
public:
    __fastcall TMySwitch();
    AnsiString Name;
    TMyIo *Output;

    int Tag;
    AnsiString CardModal;
    AnsiString IOPos;
    int Card;
    int Port;
    int Bit;
    bool OutValue;
    bool SetValue;
    int Type;
    bool Enable;
    bool EnableAtDataBase;

    void On();
    void Off();
    bool Status();
    void OnOff(bool Type);
};
//---------------------------------------------------------------------------
extern class TMySwitch SWBackup[8];
void CopySwitch(TMySwitch *Source, TMySwitch *Target);
//---------------------------------------------------------------------------
#endif
