//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#pragma hdrstop

#include "butPa1.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TBtnPanel *)
{
    new TBtnPanel(NULL);
}
//---------------------------------------------------------------------------
__fastcall TBtnPanel::TBtnPanel(TComponent* Owner)
     : TPanel(Owner)
{
    TColor  tcTrueColor=clBtnFace;
    TColor  tcFalseColor=clBtnFace;
    bDown=false;
    FStyle=tsButtons;                                                           //Steven 20230826 : for 新版GUI, 增加Flat Btn選項
    if(bDown)
    {
        Color=tcTrueColor;
        BevelInner=bvLowered;
        BevelOuter=bvLowered;
    }
    else
    {
        Color=tcFalseColor;
        if(FStyle==tsFlatButtons)                                               //Steven 20230826 : for 新版GUI, 增加Flat Btn選項
        {
            BevelInner=bvNone;
            BevelOuter=bvNone;
        }
        else
        {
            BevelInner=bvRaised;
            BevelOuter=bvRaised;
        }
    }
    sOutPort="";
    sOutBit="";
    sOutType="";
    OutPort=0;
    OutBit=0;
    OutType=0;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanel::SetTrueColor(TColor v)
{
    tcTrueColor=v;
    if(bDown)
        Color=v;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanel::SetFalseColor(TColor v)
{
    tcFalseColor=v;
    if(!bDown)
        Color=v;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanel::SetTrueFontColor(TColor v)
{
    tcTrueFontColor=v;
    if(bDown)
        Font->Color=v;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanel::SetFalseFontColor(TColor v)
{
    tcFalseFontColor=v;
    if(!bDown)
        Font->Color=v;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanel::SetPanelStatus(bool v)
{
    bDown=v;
    if(bDown)
    {
        Color=tcTrueColor;
        Font->Color=tcTrueFontColor;
        BevelInner=bvLowered;
        BevelOuter=bvLowered;
    }
    else
    {
        Color=tcFalseColor;
        Font->Color=tcFalseFontColor;
        if(FStyle==tsFlatButtons)                                               //Steven 20230826 : for 新版GUI, 增加Flat Btn選項
        {
            BevelInner=bvNone;
            BevelOuter=bvNone;
        }
        else
        {
            BevelInner=bvRaised;
            BevelOuter=bvRaised;
        }
    }
}
//---------------------------------------------------------------------------
//void __fastcall TBtnPanel::Click(TControl *P)
//{
//     bDown=!bDown;
//     SetPanelStatus(bDown);
//     TPanel::Click();
//}
//---------------------------------------------------------------------------
void __fastcall TBtnPanel::CutSpaceAtTail(char *S)
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
void __fastcall TBtnPanel::CutSpaceAtHead(char *S)
{
    char str2[256]={""};
    int i=0,pos=0;
    while(1)
    {
        if(S[i]!=' ')
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
void __fastcall TBtnPanel::WritePort(AnsiString S)
{
    int i,len,Scale=1,j;
    char c,str[256];
    strcpy(str,S.c_str());
    if(strlen(str)==0)
        OutPort=0;
    CutSpaceAtHead(str);
    CutSpaceAtTail(str);
    strcpy(str,strupr(str));
    len=strlen(str);
    OutPort=0;
    for(i=(len-1); i>=0; i--)
    {
        c=str[i];
        if((c>='0' && c<='9'))
            j=(int)(c-'0');
        else if((c>='A' && c<='F'))
            j=10+(int)(c-'A');
        else
            break;
        OutPort+=j*Scale;
        Scale*=16;
    }
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanel::WriteBit(AnsiString S)
{
    OutBit=atoi(S.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanel::WriteType(AnsiString S)
{
    OutType=atoi(S.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanel::WriteAlias(AnsiString S)
{
    sAlias=S;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TBtnPanel::ReadAlias()    //Steven 20191009 : 避免讀不到Alias
{
    return sAlias;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanel::WriteStyle(TTabStyle Value)                          //Steven 20230826 : for 新版GUI, 增加Flat Btn選項
{
    FStyle=Value;
    SetPanelStatus(bDown);
}
//---------------------------------------------------------------------------
namespace Butpa1
{
     void __fastcall PACKAGE Register()
     {
           TComponentClass classes[1] = {__classid(TBtnPanel)};
           RegisterComponents("MarcComp", classes, 0);
     }
}
//---------------------------------------------------------------------------

