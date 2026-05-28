//---------------------------------------------------------------------------
#ifndef mysensorH
#define mysensorH
//---------------------------------------------------------------------------
#include "myio.h"
//---------------------------------------------------------------------------
bool GetTrayBuildState(int pos);
void SetTrayBuildState(int pos);
void ClrTrayBuildState(int pos);
//---------------------------------------------------------------------------
class TMySensor
{
public:
    __fastcall TMySensor();
    TMyIo *Input;

    AnsiString Name;
    AnsiString OnAlarmCode;
    AnsiString OffAlarmCode;
    AnsiString AlarmType;
    int Tag;
    AnsiString CardModal;
    AnsiString IOPos;
    int Card;
    int Port;
    int Bit;
    int Type;
    bool Enable;
    bool EnableAtDataBase;
    bool bErroHappen;
    int iStatus;

    bool Status();
    bool IsOn();
    bool IsOff();
};
//---------------------------------------------------------------------------
void CopySensor(TMySensor *Source, TMySensor *Target);
//---------------------------------------------------------------------------
#endif
