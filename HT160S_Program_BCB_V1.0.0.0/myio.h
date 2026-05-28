//---------------------------------------------------------------------------
#ifndef myioH
#define myioH
//---------------------------------------------------------------------------
#include <vcl.h>
//---------------------------------------------------------------------------
class TMyIo
{
public:
    int iCard;
    int iLane;
    int iIP;
    int iPort;
    int iBit;
    int iModuleType;
    int ISABase;
    AnsiString IOHint;

    __fastcall TMyIo();
    virtual ~TMyIo(){};
    virtual void SetPortInformation(int Card, int Port, int Bit);
    virtual void SetPortInformation(int Line, AnsiString IP, int Port, int Bit);
    virtual void SetPortInformation(AnsiString Card, AnsiString Port, AnsiString Bit);
    virtual bool IsOn();
    virtual bool IsOff();
    virtual void On();
    virtual void Off();
    virtual int GetCard(){return iCard;};
    virtual int GetLane(){return iLane;};
    virtual int GetIP(){return iIP;};
    virtual int GetPort(){return iPort;};
    virtual int GetBit(){return iBit;};
    virtual AnsiString GetDriverName(){return "TMyIo";};

    byte IOInputByte(int port);
    int IOInputLongByte(int port);
    void IOByteOut(int port, byte Byte);
    void InitialMyOutIOData();
    int IOSetOutport(int port);
    bool IOSetInport(int port);
    void SetHint(AnsiString Hint);
    AnsiString GetHint(){return IOHint;};

protected:
    bool bOutValue;
    bool MN200IsValidAddress(int Port, int Bit, bool CheckBit);
    bool MN200ReadBit(int Bit, bool *State);
    bool MN200ReadInputByte(int Port, byte *Value);
    bool MN200ReadOutputByte(int Port, byte *Value);
    bool MN200WriteByte(int Port, byte Value);
    bool MN200WriteBit(int Bit, bool Value);
    byte MN200BitMask(int Bit);
};
//---------------------------------------------------------------------------
#endif
