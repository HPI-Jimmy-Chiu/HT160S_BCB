//---------------------------------------------------------------------------
#ifndef TMapArrayH
#define TMapArrayH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class PACKAGE TArrayMap : public TImage
{
private:
    void __fastcall Paint(void);
    DYNAMIC void __fastcall Resize(void);
    DYNAMIC void __fastcall MouseDown(TMouseButton Button, TShiftState Shift, int X, int Y);
    DYNAMIC void __fastcall MouseUp  (TMouseButton Button, TShiftState Shift, int X, int Y);
    int nXItem;
    int nYItem;

__published:
    __property  int XItem =
        { read = nXItem,write = SetXItem,default = 2 };
    __property  int YItem =
        { read = nYItem,write = SetYItem,default = 2 };
    __property  bool MapManualSet =
        { read = bOnManualSet,write = bOnManualSet,default = true};

protected:
    virtual void __fastcall WndProc(TMessage& Message);
public:
    __fastcall TArrayMap(TComponent* Owner);
__published:
//------------------------------------------------------
// Map 內容定義
//
//  0:clGreen   :
//  1:clRed     :
//  2:clBtnFace :
//  3:0x00c08080:
//  4:clYellow  :
//  5:clWhite   :
//------------------------------------------------------
public:
    TRect R;
    //基本變數
    bool bOnManualSet;

    int nXSpace;
    int nYSpace;
    int nBackGroundColor;
    int nMap[100][100];
    int nSelectX;
    int nSelectY;
    int nXPos;
    int nYPos;
    int nXPitch;
    int nYPitch;

    //使用模式
    int nFirstPos;      //0:左上  1:左下 2:右上 3:右下
    int nSearchMode;    //0;X Dir 1:Y Dir
    int nContinousMode; //0:Yes   1:No


    //-----------------------
    // 外部可使用Function
    //-----------------------
    void MapArrayReset();
    void ShowCell(int x,int y);//one cell change
    void ShowMap();            //all map  change
    bool SearchNextPos();
    void __fastcall SetXItem(int a);
    void __fastcall SetYItem(int a);

    bool fnMapSave2File(AnsiString FileName);
    bool fnMapSave2RePort(AnsiString FileName);
};
//---------------------------------------------------------------------------
#endif
