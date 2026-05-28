//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#pragma hdrstop

#include "MyLedLane.h"
#pragma link "ALed"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TMyLedLane *)
{
    new TMyLedLane(NULL);
}
//---------------------------------------------------------------------------
__fastcall TMyLedLane::TMyLedLane(TComponent* Owner)
    : TALed(Owner)
{
    LEDStyle=LEDSqLarge;
    sRing="";
    sIP="";
    sInPort="";
    sInBit="";
    sInType="";
    sISA="";                                                             //Nickliu 20230316 add ISA Type
    InRing=0;
    InIP=0;
    InPort=0;
    InBit=0;
    InType=0;
    ISABase=0;                                                           //Nickliu 20230316 add ISA Type
}
//---------------------------------------------------------------------------
void __fastcall TMyLedLane::CutSpaceAtTail(char *S)
{
    int len;
    len=strlen(S);
    while(len){
        if( S[len]==' ')
            S[len]='\0';
        else if( S[len]!='\0'){
            return;
        }
        len--;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMyLedLane::CutSpaceAtHead(char *S)
{
    char str2[256]={""};
    int i=0,pos=0;
    while(1)
    {
        if( S[i]!=' ')
            break;
        i++;
    }
    if(S[i]==0)
        return;
    while(1)
    {
        str2[pos]=S[i];
        if(S[i]==0)
            break;
        pos++;
        i++;
    }
    strcpy(S,str2);
}
//---------------------------------------------------------------------------
void __fastcall TMyLedLane::WriteRing(AnsiString Value)
{
    char str[2];
    strcpy(str,Value.c_str());
    str[1]='\0';
    InRing=atoi(str);
}
//---------------------------------------------------------------------------
void __fastcall TMyLedLane::WriteIP(AnsiString Value)
{
    char str[3];
    strcpy(str,Value.c_str());
    str[2]='\0';
    InIP=atoi(str);
}
//---------------------------------------------------------------------------
void __fastcall TMyLedLane::WritePort(AnsiString S)
{
    char str[2];
    strcpy(str,S.c_str());
    str[1]='\0';
    InPort=atoi(str);
}
//---------------------------------------------------------------------------
void __fastcall TMyLedLane::WriteBit(AnsiString S)
{
    char str[2];
    strcpy(str,S.c_str());
    str[1]='\0';
    InBit=atoi(str);
}
//---------------------------------------------------------------------------
void __fastcall TMyLedLane::WriteType(AnsiString S)
{
//    int i,len,j;
//    char c,str[256];
    InType=atoi(S.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TMyLedLane::WriteAlias(AnsiString S)
{
    sAlias=S;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TMyLedLane::ReadAlias()       //Steven 20191009 : Á×§KÅª¤£¨ìAlias
{
    return sAlias;
}
//---------------------------------------------------------------------------
void __fastcall TMyLedLane::WriteISA(AnsiString S)                                     //Nickliu 20230316 add ISA Type
{
    sISA=S; 
    ISABase=atoi(S.c_str());                            
}
//---------------------------------------------------------------------------
namespace Myledlane
{
    void __fastcall PACKAGE Register()
    {
         TComponentClass classes[1] = {__classid(TMyLedLane)};
         RegisterComponents("MarcComp", classes, 0);
    }
}
