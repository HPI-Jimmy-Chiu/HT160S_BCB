//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#pragma hdrstop

#include "MyLed.h"
#pragma link "ALed"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TMyLed *)
{
    new TMyLed(NULL);
}
//---------------------------------------------------------------------------
__fastcall TMyLed::TMyLed(TComponent* Owner)
    : TALed(Owner)
{
    LEDStyle=LEDSqLarge;
    sInPort="";
    sInBit="";
    sInType="";
    InPort=0;
    InBit=0;
    InType=0;
}
//---------------------------------------------------------------------------
void __fastcall TMyLed::CutSpaceAtTail(char *S)
{
    int len;
    len=strlen(S);
    while(len)
    {
        if(S[len]==' ')
        {
            S[len]='\0';
        }
        else if(S[len]!='\0')
        {
            return;
        }
        len--;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMyLed::CutSpaceAtHead(char *S)
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
void __fastcall TMyLed::WritePort(AnsiString S)
{
    int i,len,Scale=1,j;
    char c,str[256];
    strcpy(str, S.c_str());
    if(strlen(str)==0)
        InPort=0;
    CutSpaceAtHead(str);
    CutSpaceAtTail(str);
    strcpy(str, strupr(str));
    len=strlen(str);
    InPort=0;
    for(i=(len-1); i>=0; i--)
    {
        c=str[i];
        if((c>='0' && c<='9'))
            j=(int)(c-'0');
        else if((c>='A' && c<='F'))
            j=10+(int)(c-'A');
        else
            break;
        InPort+=j*Scale;
        Scale*=16;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMyLed::WriteBit(AnsiString S)
{
    InBit=atoi(S.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TMyLed::WriteType(AnsiString S)
{
    InType=atoi(S.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TMyLed::WriteAlias(AnsiString S)
{
    sAlias=S;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TMyLed::ReadAlias()       //Steven 20191009 : Á×§KÅª¤£¨ìAlias
{
    return sAlias;
}
//---------------------------------------------------------------------------
namespace Myled
{
    void __fastcall PACKAGE Register()
    {
        TComponentClass classes[1] = {__classid(TMyLed)};
        RegisterComponents("lee40", classes, 0);
    }
}
//---------------------------------------------------------------------------
