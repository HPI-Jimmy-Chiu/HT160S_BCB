//---------------------------------------------------------------------------
#ifndef HTray256H
#define HTray256H
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------

#define MAX_COLOR_INDEX_256 32
#define MAX_ITEM_256        300 //kevin 20140307 for 256 bin

enum TTray256DirectStyle { csNull256,csLeftTop256,csLeftBottom256,csRightTop256,csRightBottom256 };

class PACKAGE TTMyTray256 : public TCustomControl
{
private:
    int iStartX;
    int iStartY;
    int iPitchX;
    int iPitchY;
    int iXWidth;
    int iYWidth;

    int FXItem;              // X 方向數量
    int FYItem;              // Y 方向數量
    int FLineWidth;          // 格線寬度
    int FEdgeWidth;          // 邊匡寬度
    int FDirectWidth;
    bool FShowFont;
    bool FShape;
    TTray256DirectStyle FTrayDirect;

    TColor  ColorMap[MAX_COLOR_INDEX_256];    //
    int     CellColorIndex[MAX_ITEM_256][MAX_ITEM_256];
    AnsiString     CellText[MAX_ITEM_256][MAX_ITEM_256];

    TColor  FFrameColor;     // IC edge color
    TColor  FTrayColor;     // IC edge color
    void __fastcall SetXItem(int value);
    void __fastcall SetYItem(int value);

    void __fastcall SetDirectWidth(int value);
    void __fastcall SetTrayDirect(TTray256DirectStyle value);
    void __fastcall SetLineWidth(int value);
    void __fastcall SetEdgeWidth(int value);
    void __fastcall SetShowFont(bool value);
    void __fastcall SetShape(bool value);
    void __fastcall CaculateTrayParameter();

protected:
    virtual void __fastcall WndProc(TMessage& Message);
    virtual void __fastcall CreateWnd();
    void __fastcall DrawSingleIC(int X,int Y);

public:
    __fastcall TTMyTray256(TComponent* Owner);
    void __fastcall DrawTray();
    void    SetColorMap(int Index,TColor Color);
    void    SetCellColorIndex(int X,int Y,int Index);
    void    SetFrameColor(TColor Color);
    void    SetTrayColor(TColor Color);
    AnsiString GetCellText(int X, int Y);
    void __fastcall ClearCell();
    void __fastcall SetCellNumber(int x, int y, char *c);
    void __fastcall SetCellNumber(int x, int y, int c);
    void __fastcall SetCellNumber(int x, int y, double c);
    void __fastcall SaveCellTextToFile(AnsiString FileName);
//    virtual void  __fastcall MouseDown(TMouseButton *P,TShiftState *S,int X,int T);
    int ConvertIndexCells(int &X,int &Y);
    int GetCellData(int i,int j);
    void __fastcall DrawLine(int X,int Y);
__published:
    __property Color;  // 自動產生 Color Font 屬性
    __property Font;   // base class properties redeclared
    __property int XItem  =         { read = FXItem,      write = SetXItem, default = 2 };
    __property int YItem  =         { read = FYItem,      write = SetYItem, default = 2 };
    __property int LineWidth  =     { read = FLineWidth,  write = SetLineWidth, default = 1 };
    __property int EdgeWidth  =     { read = FEdgeWidth,  write = SetEdgeWidth, default = 1 };
    __property int DirectWidth  =   { read = FDirectWidth,write = SetDirectWidth,default = 10 };
    __property bool ShowFont     =  { read = FShowFont    ,write = SetShowFont,default = true };
    __property bool Shape        =  { read = FShape       ,write = SetShape,default = false };
    __property TTray256DirectStyle DirectStyle={ read = FTrayDirect ,write = SetTrayDirect,  default = csLeftTop256 };

    __property OnMouseDown;
    __property OnMouseUp;
    __property OnMouseMove;
};
//---------------------------------------------------------------------------
#endif

