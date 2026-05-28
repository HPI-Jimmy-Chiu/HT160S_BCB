//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "myio_MN200.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const int MN200_IO_BASE_MOTIONNET = 0;
//---------------------------------------------------------------------------
__fastcall TMyMN200_IO::TMyMN200_IO()
    : TMyIo()
{
}
//---------------------------------------------------------------------------
void TMyMN200_IO::SetPortInformation(int Card, int Port, int Bit)
{
    TMyIo::SetPortInformation(Card, Port, Bit);
}
//---------------------------------------------------------------------------
void TMyMN200_IO::SetPortInformation(int Line, AnsiString IP, int Port, int Bit)
{
    TMyIo::SetPortInformation(Line, IP, Port, Bit);
}
//---------------------------------------------------------------------------
void TMyMN200_IO::SetPortInformation(AnsiString Card, AnsiString Port, AnsiString Bit)
{
    TMyIo::SetPortInformation(Card, Port, Bit);
}
//---------------------------------------------------------------------------
bool TMyMN200_IO::IsOn()
{
#ifdef SOFT_SIMULATE
    return bOutValue;
#else
    bool State=false;

    if(ISABase!=MN200_IO_BASE_MOTIONNET)
        return TMyIo::IsOn();
    if(MN200ReadBit(iBit, &State))
        return State;
    return false;
#endif
}
//---------------------------------------------------------------------------
bool TMyMN200_IO::IsOff()
{
    return !IsOn();
}
//---------------------------------------------------------------------------
void TMyMN200_IO::On()
{
    bOutValue=true;
#ifdef SOFT_SIMULATE
    return;
#else
    if(ISABase!=MN200_IO_BASE_MOTIONNET)
    {
        TMyIo::On();
        return;
    }
    MN200WriteBit(iBit, true);
#endif
}
//---------------------------------------------------------------------------
void TMyMN200_IO::Off()
{
    bOutValue=false;
#ifdef SOFT_SIMULATE
    return;
#else
    if(ISABase!=MN200_IO_BASE_MOTIONNET)
    {
        TMyIo::Off();
        return;
    }
    MN200WriteBit(iBit, false);
#endif
}
//---------------------------------------------------------------------------
void TMyMN200_IO::DO_Process(byte Value)
{
#ifdef SOFT_SIMULATE
    bOutValue=(Value!=0);
    return;
#else
    if(ISABase!=MN200_IO_BASE_MOTIONNET)
    {
        TMyIo::IOByteOut(iPort, Value);
        return;
    }
    MN200WriteByte(iPort, Value);
#endif
}
//---------------------------------------------------------------------------
byte TMyMN200_IO::DI_Process(bool bByte)
{
#ifdef SOFT_SIMULATE
    return bOutValue?1:0;
#else
    byte Value=0;
    bool State=false;

    if(ISABase!=MN200_IO_BASE_MOTIONNET)
        return TMyIo::IOInputByte(iPort);

    if(bByte)
    {
        if(MN200ReadInputByte(iPort, &Value))
            return Value;
        return 0;
    }

    if(MN200ReadBit(iBit, &State))
        return State?1:0;
    return 0;
#endif
}
//---------------------------------------------------------------------------
byte TMyMN200_IO::IOInputByte()
{
    return DI_Process(true);
}
//---------------------------------------------------------------------------
void TMyMN200_IO::IOByteOut(byte Byte)
{
    DO_Process(Byte);
}
//---------------------------------------------------------------------------
void TMyMN200_IO::InitialMyOutIOData()
{
    TMyIo::InitialMyOutIOData();
}
//---------------------------------------------------------------------------