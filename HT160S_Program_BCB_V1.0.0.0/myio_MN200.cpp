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
//AI(HT160S-Maintainer) 20260622 : HT172 myio_MN200.cpp parity - the IO layer is now
//HARDWARE-TRANSPARENT. Always attempt the real MN200 read/write; fall back to the cached
//bOutValue (reads) / silent no-op (writes) ONLY when the card call fails (no MN200DLL or
//card absent -> MN200ReadBit/WriteBit return false). The old code had a top-level
//"#ifdef SOFT_SIMULATE return bOutValue/return;" in every method that bypassed the card
//ENTIRELY in a sim build, so a SOFT_SIMULATE build on a real machine read/wrote garbage at
//runtime while IOsetview (which uses base TMyIo, no bypass) saw the real card - the exact
//"IOsetview correct / production sensor wrong" split. Card-absent dev/CI is unchanged (the
//calls just fail and fall back, same values as the old sim branch). HT172 uses the same
//attempt-first model; its #ifdef SOFT_SIMULATE only picks the on-failure value (mn ret<0).
//NOTE: a SOFT_SIMULATE build run on real hardware now drives real outputs too (matches
//HT172). Production ships with SOFT_SIMULATE off, where this path was always taken anyway.
bool TMyMN200_IO::IsOn()
{
    bool State=false;

    if(ISABase!=MN200_IO_BASE_MOTIONNET)
        return TMyIo::IsOn();
    if(MN200ReadBit(iBit, &State))
        return State;
    return bOutValue;
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
    if(ISABase!=MN200_IO_BASE_MOTIONNET)
    {
        TMyIo::On();
        return;
    }
    MN200WriteBit(iBit, true);
}
//---------------------------------------------------------------------------
void TMyMN200_IO::Off()
{
    bOutValue=false;
    if(ISABase!=MN200_IO_BASE_MOTIONNET)
    {
        TMyIo::Off();
        return;
    }
    MN200WriteBit(iBit, false);
}
//---------------------------------------------------------------------------
void TMyMN200_IO::DO_Process(byte Value)
{
    bOutValue=(Value!=0);
    if(ISABase!=MN200_IO_BASE_MOTIONNET)
    {
        TMyIo::IOByteOut(iPort, Value);
        return;
    }
    MN200WriteByte(iPort, Value);
}
//---------------------------------------------------------------------------
byte TMyMN200_IO::DI_Process(bool bByte)
{
    byte Value=0;
    bool State=false;

    if(ISABase!=MN200_IO_BASE_MOTIONNET)
        return TMyIo::IOInputByte(iPort);

    if(bByte)
    {
        if(MN200ReadInputByte(iPort, &Value))
            return Value;
        return bOutValue?1:0;
    }

    if(MN200ReadBit(iBit, &State))
        return State?1:0;
    return bOutValue?1:0;
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