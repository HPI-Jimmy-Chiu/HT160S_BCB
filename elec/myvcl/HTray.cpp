//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "HTray.h"
#pragma package(smart_init)

//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TTMyTray *)
{
    new TTMyTray(NULL);
}
//---------------------------------------------------------------------------
__fastcall TTMyTray::TTMyTray(TComponent* Owner)
    : TCustomControl(Owner)
{
    int i, j;
    TColor TC[MAX_COLOR_INDEX]=
    {
        clWhite,clGreen,clYellow,clLime,clRed,
        (TColor)0x00FF8080, (TColor)0x00DC34BE, clBlue, (TColor)0x00FF80FF, (TColor)0x00FF0080, (TColor)0x0000FF80,
    };

    Width=100;
    Height=200;
    FXItem=2;
    FYItem=2;

    FXVisibleItem=2;
    FYVisibleItem=2;

    FXBlock=1;
    FYBlock=1;
    FXBlockWidth=1;
	FYBlockWidth=1;
    FLineWidth=1;
    FEdgeWidth=1;          // Ãä¦J¼e«×
    FShowFont=true;
    for(i=0; i<MAX_COLOR_INDEX; i++)
        ColorMap[i]=TC[i];
    for(i=0; i<MAX_ITEM; i++)
    {
        for(j=0; j<MAX_ITEM; j++)
        {
            CellColorIndex[i][j]=0;
            CellText[i][j]="5";
        }
        bXVisible[i]=true;
        bYVisible[i]=true;
    }
    FTrayColor=clWhite;
    FFrameColor=clBlack;    // ¦JÃC¦â
    FTrayDirect=csLeftTop ;
    FDirectWidth=10;
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::CreateWnd()
{
    if (ComponentState.Contains(csDestroying))
        return;
    TCustomControl::CreateWnd();
    CaculateTrayParameter();
    ClearCell();
    /*
    if (ComponentState.Contains(csDesigning))
    {
        CaculateTrayParameter();
        ClearCell();
    }
    */
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::WndProc(TMessage& Message)
{
    TCustomControl::WndProc(Message);
    if (Message.Msg == WM_PAINT || Message.Msg == WM_SIZE)
    {
        CaculateTrayParameter();
        DrawTray();
    }
    else if(Message.Msg == CM_FONTCHANGED)
    {
        DrawTray();
    }
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::ClearCell()
{
    for(int i=0; i<MAX_ITEM; i++)
    {
        for(int j=0; j<MAX_ITEM; j++)
        {
            CellColorIndex[i][j]=0;
            CellText[i][j]="";
        }
    }
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetXItem(int value)
{
    if(value<=0)
        value=1;
    if(value>=MAX_ITEM)
        value=MAX_ITEM-1;

    FXItem=value;
    FXVisibleItem=FXItem;

    for(int i=0; i<MAX_ITEM; i++)
    {
        bXVisible[i]=true;
    }

    CaculateTrayParameter();
    ClearCell();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetYItem(int value)
{
    if(value<=0)
        value=1;
    if(value>=MAX_ITEM)
        value=MAX_ITEM-1;
    FYItem=value;
    FYVisibleItem=FYItem;

    for(int i=0; i<MAX_ITEM; i++)
    {
        bXVisible[i]=true;
    }

    CaculateTrayParameter();
    ClearCell();
}
//---------------------------------------------------------------------------
//2014-03-04    Dell    for SPIL WLP Add Tray Block
void __fastcall TTMyTray::SetBlockXItem(int value)
{
    if(value<=0)
        value=1;
    if(value>=2)
        value=2;
    if(value>=FXItem)
        FXItem=value;

    FXVisibleItem=FXItem;

    for(int i=0; i<MAX_ITEM; i++)
    {
        bXVisible[i]=true;
    }

    FXBlock=value;
    CaculateTrayParameter();
    ClearCell();
}
//---------------------------------------------------------------------------
//2014-03-04    Dell    for SPIL WLP Add Tray Block
void __fastcall TTMyTray::SetBlockYItem(int value)
{
    if(value<=0)
        value=1;
    if(value>=5)
        value=5;
    if(value>=FYItem)
        FYItem=value;

    FYVisibleItem=FYItem;

    for(int i=0; i<MAX_ITEM; i++)
    {
        bXVisible[i]=true;
    }

    FYBlock=value;
    CaculateTrayParameter();
    ClearCell();
}
//---------------------------------------------------------------------------
//2014-03-04    Dell    for SPIL WLP Add Tray Block
void __fastcall TTMyTray::SetXBlockWidth(int value)
{
    if(value<=0)
        value=1;
    FXBlockWidth=value;
    CaculateTrayParameter();
    ClearCell();
}
//---------------------------------------------------------------------------
//2014-03-04    Dell    for SPIL WLP Add Tray Block
void __fastcall TTMyTray::SetYBlockWidth(int value)
{
    if(value<=0)
        value=1;
    FYBlockWidth=value;
    CaculateTrayParameter();
    ClearCell();
}
//---------------------------------------------------------------------------
void TTMyTray::SetFrameColor(TColor Color)
{
    FFrameColor=Color;
    DrawTray();
}
//---------------------------------------------------------------------------
void TTMyTray::SetTrayColor(TColor Color)
{
    FTrayColor=Color;
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::DrawLine(int X, int Y)
{
    if(Visible==false || bXVisible[X]==false || bYVisible[Y]==false)      //Steven 20160503 : Add Visible
        return;
    DrawTray();
    Canvas->Pen->Color=clRed;
    Canvas->MoveTo(0, Y);
    Canvas->LineTo(Width-1, Y);
    Canvas->MoveTo(X, 0);
    Canvas->LineTo(X, Height-1);
}
//---------------------------------------------------------------------------
  void __fastcall TTMyTray::DrawSingleIC(int X, int Y)
{
    if(Visible==false)      //Steven 20160503 : Add Visible
        return;

    if(X>=FXItem || Y>=FYItem)
        return;

    if(bXVisible[X]==false || bYVisible[Y]==false)
        return;

    int iBX = X / (FXItem / FXBlock)*FXBlockWidth;   //2014-03-04    Dell    for SPIL WLP Add Tray Block
    int iBY = Y / (FYItem / FYBlock)*FYBlockWidth;   //2014-03-04    Dell    for SPIL WLP Add Tray Block

    TRect NewRect;
    NewRect.Left=iStartX+X*iPitchX+iBX; //2014-03-04    Dell    for SPIL WLP Add Tray Block
    NewRect.Top =iStartY+Y*iPitchY+iBY; //2014-03-04    Dell    for SPIL WLP Add Tray Block
    NewRect.Right=NewRect.Left+iXWidth;
    NewRect.Bottom=NewRect.Top+iYWidth;
    Canvas->Brush->Color=ColorMap[CellColorIndex[X][Y]];
    if(FShape)
        Canvas->Ellipse(NewRect.Left, NewRect.Top, NewRect.Right, NewRect.Bottom);
    else
        Canvas->Rectangle(NewRect.Left, NewRect.Top, NewRect.Right, NewRect.Bottom);

    if(FShowFont)
    {
        DrawText(Canvas->Handle, CellText[X][Y].c_str(), CellText[X][Y].Length(),
                 &NewRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
    }
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::CaculateTrayParameter()
{
    iXWidth=(Width-2-(FXVisibleItem-1)*FLineWidth-2*FEdgeWidth-(FXBlock-1)*FXBlockWidth)/FXVisibleItem;      //2014-03-04    Dell    for SPIL WLP Add Tray Block
    iYWidth=(Height-2-(FYVisibleItem-1)*FLineWidth-2*FEdgeWidth-(FYBlock-1)*FYBlockWidth)/FYVisibleItem;     //2014-03-04    Dell    for SPIL WLP Add Tray Block
    iStartX=(Width-iXWidth*FXVisibleItem-(FXVisibleItem-1)*FLineWidth-(FXBlock-1)*FXBlockWidth)/2;
    iStartY=(Height-iYWidth*FYVisibleItem-(FYVisibleItem-1)*FLineWidth-(FYBlock-1)*FYBlockWidth)/2;
    iPitchX=(iXWidth+FLineWidth);
    iPitchY=(iYWidth+FLineWidth);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::DrawTray()
{
    if(Visible==false)      //Steven 20160503 : Add Visible
        return;

    int x, y, i, sx, sy;
    TRect NewRect       =Rect(Left, Top, Left+Width, Top+Height);
    Canvas->Font        =Font;
    Canvas->Brush->Color=FTrayColor;
    Canvas->Pen->Color  =FFrameColor;
    Canvas->Rectangle(0, 0, Width, Height);
    Canvas->Pen->Color=FFrameColor;
    if(FTrayDirect==csLeftTop)
    {
        sx=0;
        sy=0;
        x=sx;
        y=sy;
        for(i=0; i<DirectWidth; i++)
        {
            Canvas->MoveTo(x, sy);
            Canvas->LineTo(sx, y);
            x++;
            y++;
        }
    }
    else if(FTrayDirect==csLeftBottom)
    {
        sx=0;
        sy=Height;
        x=sx;
        y=sy;
        for(i=0; i<DirectWidth; i++)
        {
            Canvas->MoveTo(sx, y);
            Canvas->LineTo(x, sy);
            x++;
            y--;
        }
    }
    else if(FTrayDirect==csRightTop)
    {
        sx=Width;
        sy=0;
        x=sx;
        y=sy;

        for(i=0; i<DirectWidth; i++)
        {
            Canvas->MoveTo(x, sy);
            Canvas->LineTo(sx, y);
            x--;
            y++;
        }
    }
    else if(FTrayDirect==csRightBottom)
    {
        sx=Width;
        sy=Height;
        x=sx;
        y=sy;
        for(i=0; i<DirectWidth; i++)
        {
            Canvas->MoveTo(sx, y);
            Canvas->LineTo(x, sy);
            x--;
            y--;
        }
    }
    for(x=0;x<FXItem;x++)
        for(y=0;y<FYItem;y++)
            DrawSingleIC(x,y);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetLineWidth(int value)
{
    FLineWidth=value;
    CaculateTrayParameter();
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetEdgeWidth(int value)
{
    FEdgeWidth=value;
    CaculateTrayParameter();
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetDirectWidth(int value)
{
    if(value<0 || value >Width || value>Height)
        return;
    FDirectWidth=value;
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetTrayDirect(TTrayDirectStyle value)
{
    FTrayDirect=value;
    DrawTray();
}
//---------------------------------------------------------------------------
void TTMyTray::SetColorMap(int Index, TColor Color)
{
    if(Index>=MAX_COLOR_INDEX || Index<0)
        return;
    ColorMap[Index]=Color;
    DrawTray();
}
//---------------------------------------------------------------------------
void TTMyTray::SetCellColorIndex(int X, int Y, int Index)
{
    if(X>=FXItem || Y>=FYItem || Index>=MAX_COLOR_INDEX || Index<0 || X<0 || Y<0)
        return;
    CellColorIndex[X][Y]=Index;
    DrawSingleIC(X, Y);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetShowFont(bool value)
{
    FShowFont=value;
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetShape(bool value)
{
    FShape=value;
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetCellNumber(int x, int y, char *s)
{
    if(x>=FXItem || y>=FYItem || x<0 || y<0)
        return;
    CellText[x][y]=s;
    DrawSingleIC(x, y);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetCellNumber(int x, int y, AnsiString s)
{
    if(x>=FXItem || y>=FYItem || x<0 || y<0)
        return;
    CellText[x][y]=s.c_str();
    DrawSingleIC(x, y);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetCellNumber(int x, int y, int s)
{
    char str[256];
    sprintf(str, "%d", s);
    SetCellNumber(x, y, str);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SetCellNumber(int x, int y, double c)
{
    AnsiString str;
    str.sprintf("%0.2f", c);
    SetCellNumber(x, y, str.c_str());
}
//---------------------------------------------------------------------------
int TTMyTray::GetCellData(int i,int j)
{
    return CellColorIndex[i][j];
}
//---------------------------------------------------------------------------
int TTMyTray::ConvertIndexCells(int &X,int &Y)
{
    int ix,iy,mx,my;

    X-=iStartX;
    Y-=iStartY;
    if(X<0 || Y<0)
        return -1;
    ix=X/iPitchX;
    iy=Y/iPitchY;
    mx=X%iPitchX;
    my=Y%iPitchY;
    if( mx>=iXWidth || my>=iYWidth || ix>=FXItem || iy>=FYItem)
        return -1;
    X=ix;
    Y=iy;
    return 1;
}
//---------------------------------------------------------------------------
AnsiString TTMyTray::GetCellText(int X, int Y)
{
    return CellText[X][Y];
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray::SaveCellTextToFile(AnsiString FileName)
{
    AnsiString Str;
    TStringList *strList = new TStringList;

    for(int i=0; i<FXItem; i++)
    {
        Str="";
        for(int j=0; j<FYItem; j++)
        {
            Str+=CellText[i][j]+", ";
        }
        strList->Add(Str);
    }

    strList->SaveToFile(FileName);
    delete strList;
}
//---------------------------------------------------------------------------
void TTMyTray::SetXVisible(int x, bool bVisible)
{
    if(x<FXItem)
    {
        bXVisible[x]=bVisible;
        FXVisibleItem=0;
        for(int i=0; i<FXItem; i++)
        {
            if(bXVisible[i])
                FXVisibleItem++;
        }
    }
}
//---------------------------------------------------------------------------
void TTMyTray::SetYVisible(int y, bool bVisible)
{
    if(y<FYItem)
    {
        bYVisible[y]=bVisible;
        FYVisibleItem=0;
        for(int i=0; i<FYItem; i++)
        {
            if(bYVisible[i])
                FYVisibleItem++;
        }
    }
}
//---------------------------------------------------------------------------
namespace Htray
{
    void __fastcall PACKAGE Register()
    {
         TComponentClass classes[1]={__classid(TTMyTray)};
         RegisterComponents("lee40", classes, 0);
    }
}
//---------------------------------------------------------------------------

