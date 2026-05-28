//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#include <windows.h>
#pragma hdrstop

#include "myio.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const int eIOBaseMotionNet = 0;
static const int MAX_MNET_LANE = 8;
static const int MAX_MNET_IP = 64;
static const int MAX_MNET_PORT = 16;
static const int MAX_MNET_BIT = 8;

typedef short (__stdcall *TMnGetPortBit)(BYTE, BYTE, BYTE, BYTE, BYTE*);
typedef short (__stdcall *TMnSetPortBit)(BYTE, BYTE, BYTE, BYTE, BYTE);
typedef short (__stdcall *TMnGetPortByte)(BYTE, BYTE, BYTE, BYTE*);
typedef short (__stdcall *TMnSetPortByte)(BYTE, BYTE, BYTE, BYTE);
typedef short (__stdcall *TMnGetDiByte)(BYTE, BYTE, BYTE, BYTE*);
typedef short (__stdcall *TMnSetDoByte)(BYTE, BYTE, BYTE, BYTE);
typedef short (__stdcall *TMnGetDoByte)(BYTE, BYTE, BYTE, BYTE*);

static bool gMN200ApiChecked = false;
static HINSTANCE gMN200Dll = NULL;
static TMnGetPortBit gMnGetPortBit = NULL;
static TMnSetPortBit gMnSetPortBit = NULL;
static TMnGetPortByte gMnGetPortByte = NULL;
static TMnSetPortByte gMnSetPortByte = NULL;
static TMnGetDiByte gMnGetDiByte = NULL;
static TMnSetDoByte gMnSetDoByte = NULL;
static TMnGetDoByte gMnGetDoByte = NULL;

static byte gOutPortData[MAX_MNET_LANE][MAX_MNET_IP][MAX_MNET_PORT];
static bool gOutPortValid[MAX_MNET_LANE][MAX_MNET_IP][MAX_MNET_PORT];
static byte gBitMask[MAX_MNET_BIT] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

static bool IsValidMotionNetAddress(int Lane, int IP, int Port, int Bit, bool CheckBit)
{
    if(Lane<0 || Lane>=MAX_MNET_LANE || IP<0 || IP>=MAX_MNET_IP || Port<0 || Port>=MAX_MNET_PORT)
        return false;
    if(CheckBit && (Bit<0 || Bit>=MAX_MNET_BIT))
        return false;
    return true;
}
//---------------------------------------------------------------------------
static FARPROC LoadMN200Proc(const char *Name, const char *StdCallName)
{
    FARPROC Proc = NULL;

    if(gMN200Dll==NULL)
        return NULL;

    Proc=GetProcAddress(gMN200Dll, Name);
    if(Proc==NULL && StdCallName!=NULL)
        Proc=GetProcAddress(gMN200Dll, StdCallName);
    return Proc;
}
//---------------------------------------------------------------------------
static bool LoadMN200Api()
{
    if(gMN200ApiChecked)
        return (gMN200Dll!=NULL);

    gMN200ApiChecked=true;
    gMN200Dll=LoadLibrary("MN200DLL.dll");
    if(gMN200Dll==NULL)
        gMN200Dll=LoadLibrary("MN200.dll");
    if(gMN200Dll==NULL)
        return false;

    gMnGetPortBit=(TMnGetPortBit)LoadMN200Proc("mn_get_port_bit", "_mn_get_port_bit@20");
    gMnSetPortBit=(TMnSetPortBit)LoadMN200Proc("mn_set_port_bit", "_mn_set_port_bit@20");
    gMnGetPortByte=(TMnGetPortByte)LoadMN200Proc("mn_get_port_byte", "_mn_get_port_byte@16");
    gMnSetPortByte=(TMnSetPortByte)LoadMN200Proc("mn_set_port_byte", "_mn_set_port_byte@16");
    gMnGetDiByte=(TMnGetDiByte)LoadMN200Proc("mn_get_di_byte", "_mn_get_di_byte@16");
    gMnSetDoByte=(TMnSetDoByte)LoadMN200Proc("mn_set_do_byte", "_mn_set_do_byte@16");
    gMnGetDoByte=(TMnGetDoByte)LoadMN200Proc("mn_get_do_byte", "_mn_get_do_byte@16");

    return true;
}
//---------------------------------------------------------------------------
static bool ReadMotionNetInputByte(int ModuleType, int Lane, int IP, int Port, byte *Value)
{
    BYTE Data = 0;
    short Ret = -1;

    if(Value==NULL || !IsValidMotionNetAddress(Lane, IP, Port, 0, false) || !LoadMN200Api())
        return false;

    if(ModuleType==0 && gMnGetDiByte!=NULL)
        Ret=gMnGetDiByte((BYTE)Lane, (BYTE)IP, (BYTE)Port, &Data);
    else if(gMnGetPortByte!=NULL)
        Ret=gMnGetPortByte((BYTE)Lane, (BYTE)IP, (BYTE)Port, &Data);

    if(Ret<0 && gMnGetPortByte!=NULL)
        Ret=gMnGetPortByte((BYTE)Lane, (BYTE)IP, (BYTE)Port, &Data);

    if(Ret<0)
        return false;

    *Value=(byte)Data;
    return true;
}
//---------------------------------------------------------------------------
static bool ReadMotionNetOutputByte(int ModuleType, int Lane, int IP, int Port, byte *Value)
{
    BYTE Data = 0;
    short Ret = -1;

    if(Value==NULL || !IsValidMotionNetAddress(Lane, IP, Port, 0, false) || !LoadMN200Api())
        return false;

    if(ModuleType==0 && gMnGetDoByte!=NULL)
        Ret=gMnGetDoByte((BYTE)Lane, (BYTE)IP, (BYTE)Port, &Data);
    else if(gMnGetPortByte!=NULL)
        Ret=gMnGetPortByte((BYTE)Lane, (BYTE)IP, (BYTE)Port, &Data);

    if(Ret<0 && gMnGetPortByte!=NULL)
        Ret=gMnGetPortByte((BYTE)Lane, (BYTE)IP, (BYTE)Port, &Data);

    if(Ret<0)
        return false;

    *Value=(byte)Data;
    return true;
}
//---------------------------------------------------------------------------
static bool WriteMotionNetByte(int ModuleType, int Lane, int IP, int Port, byte Value)
{
    short Ret = -1;

    if(!IsValidMotionNetAddress(Lane, IP, Port, 0, false) || !LoadMN200Api())
        return false;

    if(ModuleType==0 && gMnSetDoByte!=NULL)
        Ret=gMnSetDoByte((BYTE)Lane, (BYTE)IP, (BYTE)Port, (BYTE)Value);
    else if(gMnSetPortByte!=NULL)
        Ret=gMnSetPortByte((BYTE)Lane, (BYTE)IP, (BYTE)Port, (BYTE)Value);

    if(Ret<0 && gMnSetPortByte!=NULL)
        Ret=gMnSetPortByte((BYTE)Lane, (BYTE)IP, (BYTE)Port, (BYTE)Value);

    if(Ret>=0)
    {
        gOutPortData[Lane][IP][Port]=Value;
        gOutPortValid[Lane][IP][Port]=true;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
static bool WriteMotionNetBit(int ModuleType, int Lane, int IP, int Port, int Bit, bool Value)
{
    byte PortData;
    short Ret;

    if(!IsValidMotionNetAddress(Lane, IP, Port, Bit, true) || !LoadMN200Api())
        return false;

    if(gMnSetPortBit!=NULL)
    {
        Ret=gMnSetPortBit((BYTE)Lane, (BYTE)IP, (BYTE)Port, (BYTE)Bit, (BYTE)(Value?1:0));
        if(Ret>=0)
            return true;
    }

    if(!gOutPortValid[Lane][IP][Port])
    {
        PortData=0;
        ReadMotionNetOutputByte(ModuleType, Lane, IP, Port, &PortData);
        gOutPortData[Lane][IP][Port]=PortData;
        gOutPortValid[Lane][IP][Port]=true;
    }

    if(Value)
        gOutPortData[Lane][IP][Port]|=gBitMask[Bit];
    else
        gOutPortData[Lane][IP][Port]&=~gBitMask[Bit];

    return WriteMotionNetByte(ModuleType, Lane, IP, Port, gOutPortData[Lane][IP][Port]);
}
//---------------------------------------------------------------------------
bool TMyIo::MN200IsValidAddress(int Port, int Bit, bool CheckBit)
{
    return IsValidMotionNetAddress(iLane, iIP, Port, Bit, CheckBit);
}
//---------------------------------------------------------------------------
bool TMyIo::MN200ReadBit(int Bit, bool *State)
{
    BYTE Data;
    byte ByteData;

    if(State!=NULL)
        *State=false;
    if(State==NULL || !MN200IsValidAddress(iPort, Bit, true) || !LoadMN200Api())
        return false;

    if(gMnGetPortBit!=NULL)
    {
        if(gMnGetPortBit((BYTE)iLane, (BYTE)iIP, (BYTE)iPort, (BYTE)Bit, &Data)>=0)
        {
            *State=(Data!=0);
            return true;
        }
    }

    if(ReadMotionNetInputByte(iModuleType, iLane, iIP, iPort, &ByteData))
    {
        *State=((ByteData & gBitMask[Bit])!=0);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool TMyIo::MN200ReadInputByte(int Port, byte *Value)
{
    return ReadMotionNetInputByte(iModuleType, iLane, iIP, Port, Value);
}
//---------------------------------------------------------------------------
bool TMyIo::MN200ReadOutputByte(int Port, byte *Value)
{
    return ReadMotionNetOutputByte(iModuleType, iLane, iIP, Port, Value);
}
//---------------------------------------------------------------------------
bool TMyIo::MN200WriteByte(int Port, byte Value)
{
    return WriteMotionNetByte(iModuleType, iLane, iIP, Port, Value);
}
//---------------------------------------------------------------------------
bool TMyIo::MN200WriteBit(int Bit, bool Value)
{
    return WriteMotionNetBit(iModuleType, iLane, iIP, iPort, Bit, Value);
}
//---------------------------------------------------------------------------
byte TMyIo::MN200BitMask(int Bit)
{
    if(Bit<0 || Bit>=MAX_MNET_BIT)
        return 0;
    return gBitMask[Bit];
}
//---------------------------------------------------------------------------
__fastcall TMyIo::TMyIo()
{
    iCard=0;
    iLane=0;
    iIP=0;
    iPort=0;
    iBit=0;
    iModuleType=0;
    ISABase=0;
    IOHint="";
    bOutValue=false;
}
//---------------------------------------------------------------------------
void TMyIo::SetPortInformation(int Card, int Port, int Bit)
{
    iCard=Card;
    iLane=Card/100;
    iIP=Card%100;
    iPort=Port;
    iBit=Bit;
    iModuleType=0;
}
//---------------------------------------------------------------------------
void TMyIo::SetPortInformation(int Line, AnsiString IP, int Port, int Bit)
{
    iLane=Line;
    if(IP.AnsiCompare("A")>=0 && IP.AnsiCompare("Z")<=0)
    {
        char *Temp=IP.c_str();
        iIP=int(Temp[0]-'A')+10;
    }
    else if(IP=="")
    {
        iIP=-1;
    }
    else
    {
        iIP=atoi(IP.c_str());
    }
    iCard=iLane*100+iIP;
    iPort=Port;
    iBit=Bit;
    iModuleType=0;
}
//---------------------------------------------------------------------------
void TMyIo::SetPortInformation(AnsiString Card, AnsiString Port, AnsiString Bit)
{
    int iCardValue=atoi(Card.c_str());
    SetPortInformation(iCardValue, atoi(Port.c_str()), atoi(Bit.c_str()));
}
//---------------------------------------------------------------------------
bool TMyIo::IsOn()
{
    bool State=false;

    if(ISABase==eIOBaseMotionNet && MN200ReadBit(iBit, &State))
        return State;

    return bOutValue;
}
//---------------------------------------------------------------------------
bool TMyIo::IsOff()
{
    return !IsOn();
}
//---------------------------------------------------------------------------
void TMyIo::On()
{
    bOutValue=true;
    if(ISABase==eIOBaseMotionNet)
    MN200WriteBit(iBit, true);
}
//---------------------------------------------------------------------------
void TMyIo::Off()
{
    bOutValue=false;
    if(ISABase==eIOBaseMotionNet)
    MN200WriteBit(iBit, false);
}
//---------------------------------------------------------------------------
byte TMyIo::IOInputByte(int port)
{
    byte Value=0;
    int TargetPort=port;

    if(TargetPort<0)
        TargetPort=iPort;

    if(ISABase==eIOBaseMotionNet && MN200ReadInputByte(TargetPort, &Value))
        return Value;

    return 0;
}
//---------------------------------------------------------------------------
int TMyIo::IOInputLongByte(int port)
{
    return 0;
}
//---------------------------------------------------------------------------
void TMyIo::IOByteOut(int port, byte Byte)
{
    int TargetPort=port;

    if(TargetPort<0)
        TargetPort=iPort;

    if(ISABase==eIOBaseMotionNet)
        MN200WriteByte(TargetPort, Byte);
}
//---------------------------------------------------------------------------
void TMyIo::InitialMyOutIOData()
{
    int Lane;
    int IP;
    int Port;

    for(Lane=0; Lane<MAX_MNET_LANE; Lane++)
    {
        for(IP=0; IP<MAX_MNET_IP; IP++)
        {
            for(Port=0; Port<MAX_MNET_PORT; Port++)
            {
                gOutPortData[Lane][IP][Port]=0;
                gOutPortValid[Lane][IP][Port]=false;
            }
        }
    }
}
//---------------------------------------------------------------------------
int TMyIo::IOSetOutport(int port)
{
    if(port==0)
        return 999;
    return 0;
}
//---------------------------------------------------------------------------
bool TMyIo::IOSetInport(int port)
{
    return (port!=0);
}
//---------------------------------------------------------------------------
void TMyIo::SetHint(AnsiString Hint)
{
    if(Hint=="")
    {
        IOHint="00000";
    }
    else
    {
        while(Hint.Length()<5)
            Hint="0"+Hint;
        IOHint=Hint;
    }
}
//---------------------------------------------------------------------------
