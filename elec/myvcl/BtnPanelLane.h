//---------------------------------------------------------------------------
#ifndef BtnPanelLaneH
#define BtnPanelLaneH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class PACKAGE TBtnPanelLane : public TPanel
{
private:
    AnsiString sOutLane;
    AnsiString sOutIP;
    AnsiString sOutPort;
    AnsiString sOutBit;
    AnsiString sOutType;
    AnsiString sAlias;
    AnsiString sISA;
    TTabStyle FStyle;                                                           //Steven 20230826 : for 新版GUI, 增加Flat Btn選項
   
protected:
     TColor  tcTrueColor;
     TColor  tcFalseColor;
     TColor  tcTrueFontColor;
     TColor  tcFalseFontColor;

     bool    bDown;

     void __fastcall SetTrueColor(TColor v);
     void __fastcall SetFalseColor(TColor v);
     void __fastcall SetTrueFontColor(TColor v);
     void __fastcall SetFalseFontColor(TColor v);

     void __fastcall SetPanelStatus(bool v);
//     virtual void __fastcall Click(TControl *P);

    void __fastcall WritePort(AnsiString S);
    void __fastcall WriteBit(AnsiString S);
    void __fastcall WriteType(AnsiString S);
    void __fastcall WriteAlias(AnsiString S);
    AnsiString __fastcall ReadAlias();          //Steven 20191009 : 避免讀不到Alias
    void __fastcall SetRing(AnsiString Value);
    void __fastcall SetIP(AnsiString Value);
    void __fastcall WriteISA(AnsiString Value);
    void __fastcall WriteStyle(TTabStyle Value);                                //Steven 20230826 : for 新版GUI, 增加Flat Btn選項

public:
    int OutRing;
    int OutIP;
    int OutPort;
    int OutBit;
    int OutType;
    int ISABase;

     __fastcall TBtnPanelLane(TComponent* Owner);
__published:
    __property  TColor TrueColor      = {read = tcTrueColor ,    write=SetTrueColor};
    __property  TColor FalseColor     = {read = tcFalseColor,    write=SetFalseColor};
    __property  TColor TrueFontColor  = {read = tcTrueFontColor ,write=SetTrueFontColor};
    __property  TColor FalseFontColor = {read = tcFalseFontColor,write=SetFalseFontColor};

    __property  bool       Down ={read=bDown    , write=SetPanelStatus};

    __property  AnsiString Lane ={read=sOutLane , write=SetRing};
    __property  AnsiString IP   ={read=sOutIP   , write=SetIP};
    __property  AnsiString Port ={read=sOutPort , write=WritePort};
    __property  AnsiString Bit  ={read=sOutBit  , write=WriteBit};
    __property  AnsiString Type ={read=sOutType , write=WriteType};
    __property  AnsiString Alias={read=ReadAlias, write=WriteAlias};
    __property  AnsiString IsISA={read=sISA,      write=WriteISA};
    __property  TTabStyle  Style={read=FStyle,    write=WriteStyle};            //Steven 20230826 : for 新版GUI, 增加Flat Btn選項

};
//---------------------------------------------------------------------------
#endif
