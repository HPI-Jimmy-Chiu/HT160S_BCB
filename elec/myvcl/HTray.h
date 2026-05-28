//---------------------------------------------------------------------------
#ifndef HTrayH
#define HTrayH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------

#define MAX_COLOR_INDEX 32
#define MAX_ITEM        100 //эノHTray256.cpp

enum TTrayDirectStyle { csNull,csLeftTop,csLeftBottom,csRightTop,csRightBottom };

class PACKAGE TTMyTray : public TCustomControl
{
private:
    int iStartX;
    int iStartY;
    int iPitchX;
    int iPitchY;
    int iXWidth;
    int iYWidth;

    int FXItem;              // X よ计秖
    int FYItem;              // Y よ计秖

    int FXVisibleItem;       // X よ计秖
    int FYVisibleItem;       // Y よ计秖

    int FLineWidth;          // 絬糴
    int FEdgeWidth;          // 娩糴
    int FXBlock;             //2014-03-04    Dell    for SPIL WLP Add Tray Block
    int FYBlock;             //2014-03-04    Dell    for SPIL WLP Add Tray Block
    int FXBlockWidth;        //2014-03-04    Dell    for SPIL WLP Add Tray Block
	int FYBlockWidth;        //2014-03-04    Dell    for SPIL WLP Add Tray Block
    int FDirectWidth;
    bool FShowFont;
    bool FShape;
    TTrayDirectStyle FTrayDirect;

    TColor  ColorMap[MAX_COLOR_INDEX];    //
    int     CellColorIndex[MAX_ITEM][MAX_ITEM];
    AnsiString     CellText[MAX_ITEM][MAX_ITEM];
    bool bXVisible[MAX_ITEM];
    bool bYVisible[MAX_ITEM];

    TColor  FFrameColor;     // IC edge color
    TColor  FTrayColor;     // IC edge color
    void __fastcall SetXItem(int value);
    void __fastcall SetYItem(int value);
    void __fastcall SetBlockXItem(int value);   //2014-03-04    Dell    for SPIL WLP Add Tray Block
    void __fastcall SetBlockYItem(int value);   //2014-03-04    Dell    for SPIL WLP Add Tray Block
    void __fastcall SetXBlockWidth(int value);   //2014-03-04    Dell    for SPIL WLP Add Tray Block
	void __fastcall SetYBlockWidth(int value);   //2014-03-04    Dell    for SPIL WLP Add Tray Block

    void __fastcall SetDirectWidth(int value);
    void __fastcall SetTrayDirect(TTrayDirectStyle value);
    void __fastcall SetLineWidth(int value);
    void __fastcall SetEdgeWidth(int value);
    void __fastcall SetShowFont(bool value);
    void __fastcall SetShape(bool value);
    void __fastcall CaculateTrayParameter();

protected:
    virtual void __fastcall WndProc(Messages::TMessage& Message);
    virtual void __fastcall CreateWnd();
    void __fastcall DrawSingleIC(int X,int Y);


public:
    __fastcall TTMyTray(TComponent* Owner);
    void __fastcall DrawTray();
    void    SetColorMap(int Index,TColor Color);
    void    SetCellColorIndex(int X, int Y, int Index);
    void    SetFrameColor(TColor Color);
    void    SetTrayColor(TColor Color);
    AnsiString GetCellText(int X, int Y);
    void __fastcall ClearCell();
    void __fastcall SetCellNumber(int x, int y, char *c);
    void __fastcall SetCellNumber(int x, int y, AnsiString c);
    void __fastcall SetCellNumber(int x, int y, int c);
    void __fastcall SetCellNumber(int x, int y, double c);
    void __fastcall SaveCellTextToFile(AnsiString FileName);
//    virtual void  __fastcall MouseDown(TMouseButton *P,TShiftState *S,int X,int T);
    int ConvertIndexCells(int &X, int &Y);
    int GetCellData(int i, int j);
    void SetXVisible(int x, bool bVisible);
    void SetYVisible(int y, bool bVisible);
    void __fastcall DrawLine(int X,int Y);
__published:
    __property Color;  // 笆玻ネ Color Font 妮┦
    __property Font;   // base class properties redeclared
    __property int XItem  =         { read = FXItem,       write = SetXItem, 		default = 2 };
    __property int YItem  =         { read = FYItem,       write = SetYItem, 		default = 2 };
    __property int XBlockItem  =    { read = FXBlock,      write = SetBlockXItem, 	default = 1 }; //2014-03-04    Dell    for SPIL WLP Add Tray Block
    __property int YBlockItem  =    { read = FYBlock,      write = SetBlockYItem, 	default = 1 }; //2014-03-04    Dell    for SPIL WLP Add Tray Block
    __property int XBlockWidth =    { read = FXBlockWidth, write = SetXBlockWidth, 	default = 1 }; //2014-03-04    Dell    for SPIL WLP Add Tray Block
	__property int YBlockWidth =    { read = FYBlockWidth, write = SetYBlockWidth, 	default = 1 }; //2014-03-04    Dell    for SPIL WLP Add Tray Block
    __property int LineWidth  =     { read = FLineWidth,   write = SetLineWidth, 	default = 1 };
    __property int EdgeWidth  =     { read = FEdgeWidth,   write = SetEdgeWidth,    default = 1 };
    __property int DirectWidth  =   { read = FDirectWidth, write = SetDirectWidth,	default = 10 };
    __property bool ShowFont     =  { read = FShowFont    ,write = SetShowFont,		default = true };
    __property bool Shape        =  { read = FShape       ,write = SetShape,		default = false };
    __property TTrayDirectStyle DirectStyle={ read = FTrayDirect ,write = SetTrayDirect,  default = csLeftTop };

    __property OnMouseDown;
    __property OnMouseUp;
    __property OnMouseMove;
};
//---------------------------------------------------------------------------
#endif

