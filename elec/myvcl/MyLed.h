//---------------------------------------------------------------------------
#ifndef MyLedH
#define MyLedH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <aled.hpp>
//---------------------------------------------------------------------------
class PACKAGE TMyLed : public TALed
{
private:
    AnsiString sInPort;
    AnsiString sInBit;
    AnsiString sInType;
    AnsiString sAlias;
protected:
    void __fastcall WritePort(AnsiString S);
    void __fastcall WriteBit(AnsiString S);
    void __fastcall WriteType(AnsiString S);
    void __fastcall WriteAlias(AnsiString S);
    AnsiString __fastcall ReadAlias();      //Steven 20191009 : Á×§KÅª¤£¨ìAlias
    void __fastcall CutSpaceAtTail(char *S);
    void __fastcall CutSpaceAtHead(char *S);
public:
    int InPort;
    int InBit;
    int InType;
    __fastcall TMyLed(TComponent* Owner);
__published:
    __property  AnsiString Port ={read=sInPort  , write=WritePort};
    __property  AnsiString Bit  ={read=sInBit   , write=WriteBit};
    __property  AnsiString Type ={read=sInType  , write=WriteType};
    __property  AnsiString Alias={read=ReadAlias, write=WriteAlias};
__published:
};
//---------------------------------------------------------------------------
#endif
