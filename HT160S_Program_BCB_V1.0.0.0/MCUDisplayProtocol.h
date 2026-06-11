//---------------------------------------------------------------------------
#ifndef MCUDisplayProtocolH
#define MCUDisplayProtocolH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class THT160MCUDisplayProtocol
{
private:
    static AnsiString __fastcall BuildCommand(AnsiString AddressText, AnsiString CommandText,
        AnsiString TypeText, AnsiString DataText);
    static AnsiString __fastcall AdjustString(AnsiString Text, int MaxLength, char FillChar);
    static unsigned char __fastcall CalculateCheckSum(AnsiString Text, int StartIndex);
public:
    static AnsiString __fastcall BuildBinCodeCommand(int Address, AnsiString CodeText, bool SymbolType);
    static AnsiString __fastcall BuildBinLightCommand(int Address, int LightValue);
    static bool __fastcall BuildBinDisplayCommands(int Address, AnsiString Text, AnsiString ColorText,
        AnsiString &CodeCommand, AnsiString &LightCommand);
    static AnsiString __fastcall CommandToHex(AnsiString CommandText);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------