//---------------------------------------------------------------------------
#ifndef MyLedLaneH
#define MyLedLaneH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <aled.hpp>
//---------------------------------------------------------------------------
class PACKAGE TMyLedLane : public TALed
{
private:
    AnsiString sRing;
    AnsiString sIP;
    AnsiString sInPort;
    AnsiString sInBit;
    AnsiString sInType;
    AnsiString sAlias;
    AnsiString sISA;                         //Nickliu 20230316 add ISA Type
protected:
    void __fastcall WriteRing(AnsiString Value);
    void __fastcall WriteIP(AnsiString Value);
    void __fastcall WritePort(AnsiString S);
    void __fastcall WriteBit(AnsiString S);
    void __fastcall WriteType(AnsiString S);
    void __fastcall WriteAlias(AnsiString S);
    AnsiString __fastcall ReadAlias();          //Steven 20191009 : Á×§KÅª¤£¨ìAlias
    void __fastcall CutSpaceAtTail(char *S);
    void __fastcall CutSpaceAtHead(char *S);
    void __fastcall WriteISA(AnsiString S);    //Nickliu 20230316 add ISA Type
public:
    int InRing;
    int InIP;
    int InPort;
    int InBit;
    int InType;
    int ISABase;
    __fastcall TMyLedLane(TComponent* Owner);
__published:
    __property  AnsiString Ring ={read=sRing    , write=WriteRing};
    __property  AnsiString IP   ={read=sIP      , write=WriteIP};
    __property  AnsiString Port ={read=sInPort  , write=WritePort};
    __property  AnsiString Bit  ={read=sInBit   , write=WriteBit};
    __property  AnsiString Type ={read=sInType  , write=WriteType};
    __property  AnsiString Alias={read=ReadAlias, write=WriteAlias};
    __property  AnsiString IsISA={read=sISA,      write=WriteISA};      //Nickliu 20230316 add ISA Type
};
//---------------------------------------------------------------------------
#endif
