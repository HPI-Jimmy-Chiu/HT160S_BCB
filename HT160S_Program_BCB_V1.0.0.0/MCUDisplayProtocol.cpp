//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "MCUDisplayProtocol.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
AnsiString __fastcall THT160MCUDisplayProtocol::BuildBinCodeCommand(int Address,
    AnsiString CodeText, bool SymbolType)
{
    if(Address < 0)
        return "";

    return BuildCommand(IntToStr(Address), "BC", SymbolType ? "S" : "I", CodeText);
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160MCUDisplayProtocol::BuildBinLightCommand(int Address, int LightValue)
{
    if(Address < 0)
        return "";

    AnsiString LightText;
    if(LightValue == 0)
        LightText = "0002";
    else if(LightValue == 3)
        LightText = "0003";
    else
        LightText = "0001";

    return BuildCommand(IntToStr(Address), "BL", "I", LightText);
}
//---------------------------------------------------------------------------
bool __fastcall THT160MCUDisplayProtocol::BuildBinDisplayCommands(int Address,
    AnsiString Text, AnsiString ColorText, AnsiString &CodeCommand, AnsiString &LightCommand)
{
    CodeCommand = "";
    LightCommand = "";

    if(Address < 0)
        return false;
    if(Text == "")
        Text = "9";

    char DisplayChar = Text[1];
    if((DisplayChar >= '0') && (DisplayChar <= '9'))
        CodeCommand = BuildBinCodeCommand(Address, Text.SubString(1, 1), false);
    else if(((DisplayChar >= 'A') && (DisplayChar <= 'Z')) || ((DisplayChar >= 'a') && (DisplayChar <= 'z')))
        CodeCommand = BuildBinCodeCommand(Address, Text.SubString(1, 1), true);
    else
        CodeCommand = BuildBinCodeCommand(Address, "9", false);

    AnsiString ColorUpper = ColorText.UpperCase();
    if(ColorUpper == "GREEN" || ColorUpper == "PASS")
        LightCommand = BuildBinLightCommand(Address, 0);
    else
        LightCommand = BuildBinLightCommand(Address, 3);

    return true;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160MCUDisplayProtocol::BuildCommand(AnsiString AddressText,
    AnsiString CommandText, AnsiString TypeText, AnsiString DataText)
{
    AnsiString SendText = "HM";
    SendText += AdjustString(AddressText, 2, '0');
    SendText += AdjustString(CommandText, 2, '0');
    SendText += AdjustString(TypeText, 1, ' ');
    SendText += AdjustString(DataText, 4, '0');
    SendText += (char)CalculateCheckSum(SendText, 3);
    SendText += (char)0x0D;
    return SendText;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160MCUDisplayProtocol::AdjustString(AnsiString Text,
    int MaxLength, char FillChar)
{
    if(MaxLength <= 0)
        return "";
    if(Text.Length() > MaxLength)
        return Text.SubString(Text.Length() - MaxLength + 1, MaxLength);

    AnsiString ResultText = "";
    int FillCount = MaxLength - Text.Length();
    for(int FillIndex = 0; FillIndex < FillCount; FillIndex++)
        ResultText += FillChar;
    ResultText += Text;
    return ResultText;
}
//---------------------------------------------------------------------------
unsigned char __fastcall THT160MCUDisplayProtocol::CalculateCheckSum(AnsiString Text, int StartIndex)
{
    unsigned char CheckSum = 0x00;
    for(int CharIndex = StartIndex; CharIndex <= Text.Length(); CharIndex++)
    {
        unsigned char DataByte = (unsigned char)Text[CharIndex];
        CheckSum += DataByte / 0x10;
        CheckSum += DataByte % 0x10;
    }

    CheckSum = 0xFF - CheckSum + 0x01;
    return CheckSum;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160MCUDisplayProtocol::CommandToHex(AnsiString CommandText)
{
    AnsiString ResultText = "";
    for(int CharIndex = 1; CharIndex <= CommandText.Length(); CharIndex++)
    {
        if(ResultText != "")
            ResultText += " ";
        ResultText += IntToHex((int)(unsigned char)CommandText[CharIndex], 2);
    }
    return ResultText;
}
//---------------------------------------------------------------------------