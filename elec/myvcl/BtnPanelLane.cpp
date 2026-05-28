//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>

#pragma hdrstop
#include "BtnPanelLane.h"


#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TBtnPanelLane *)
{
    new TBtnPanelLane(NULL);
}
//---------------------------------------------------------------------------
__fastcall TBtnPanelLane::TBtnPanelLane(TComponent* Owner)
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
    sOutLane="";
    sOutIP="";
    sOutPort="";
    sOutBit="";
    sOutType="";
    sISA="";
    OutPort=0;
    OutBit=0;
    OutType=0;
    OutRing=0;
    OutIP=0;
    ISABase=0;
}
//---------------------------------------------------------------------------
namespace Btnpanellane
{
    void __fastcall PACKAGE Register()
    {
        TComponentClass classes[1] = {__classid(TBtnPanelLane)};
        RegisterComponents("MarcComp", classes, 0);
    }
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::SetTrueColor(TColor v)
{
    tcTrueColor=v;
    if(bDown)
        Color=v;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::SetFalseColor(TColor v)
{
    tcFalseColor=v;
    if(!bDown)
        Color=v;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::SetTrueFontColor(TColor v)
{
    tcTrueFontColor=v;
    if(bDown)
        Font->Color=v;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::SetFalseFontColor(TColor v)
{
    tcFalseFontColor=v;
    if(!bDown)
        Font->Color=v;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::SetPanelStatus(bool v)
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
//void __fastcall TBtnPanelLane::Click(TControl *P)
//{
//     bDown=!bDown;
//     SetPanelStatus(bDown);
//     TPanel::Click();
//}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::SetRing(AnsiString Value)
{
    OutRing=atoi(Value.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::SetIP(AnsiString Value)
{
    OutIP=atoi(Value.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::WritePort(AnsiString S)
{
    OutPort=atoi(S.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::WriteBit(AnsiString S)
{
    OutBit=atoi(S.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::WriteType(AnsiString S)
{
    OutType=atoi(S.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::WriteAlias(AnsiString S)
{
    sAlias=S;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::WriteISA(AnsiString S)
{
    sISA=S;
    ISABase=atoi(S.c_str());
}
//---------------------------------------------------------------------------
AnsiString __fastcall TBtnPanelLane::ReadAlias()        //Steven 20191009 : 避免讀不到Alias
{
    return sAlias;
}
//---------------------------------------------------------------------------
void __fastcall TBtnPanelLane::WriteStyle(TTabStyle Value)                      //Steven 20230826 : for 新版GUI, 增加Flat Btn選項
{
    FStyle=Value;
    SetPanelStatus(bDown);
}
//---------------------------------------------------------------------------
