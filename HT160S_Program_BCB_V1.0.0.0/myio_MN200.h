//---------------------------------------------------------------------------
#ifndef myio_MN200H
#define myio_MN200H
//---------------------------------------------------------------------------
#include "myio.h"
//---------------------------------------------------------------------------
class TMyMN200_IO : public TMyIo
{
protected:
    void DO_Process(byte Value);
    byte DI_Process(bool bByte=false);

public:
    __fastcall TMyMN200_IO();
    virtual void SetPortInformation(int Card, int Port, int Bit);
    virtual void SetPortInformation(int Line, AnsiString IP, int Port, int Bit);
    virtual void SetPortInformation(AnsiString Card, AnsiString Port, AnsiString Bit);
    virtual bool IsOn();
    virtual bool IsOff();
    virtual void On();
    virtual void Off();
    virtual AnsiString GetDriverName(){return "TMyMN200_IO";};

    byte IOInputByte();
    void IOByteOut(byte Byte);
    void InitialMyOutIOData();
};
//---------------------------------------------------------------------------
#endif