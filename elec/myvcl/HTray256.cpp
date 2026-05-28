//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "HTray256.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TTMyTray256 *)
{
    new TTMyTray256(NULL);
}
//---------------------------------------------------------------------------
__fastcall TTMyTray256::TTMyTray256(TComponent* Owner)
    : TCustomControl(Owner)
{
    int i,j;
    TColor TC[MAX_COLOR_INDEX_256]=
    {
        clWhite,clGreen,clYellow,clLime,clRed,
        (TColor)0x00FF8080,(TColor)0x00DC34BE,clBlue,(TColor)0x00FF80FF,(TColor)0x00FF0080,(TColor)0x0000FF80,
    };

    Width=100;
    Height=200;
    FXItem=2;
    FYItem=2;
    FLineWidth=1;
    FEdgeWidth=1;          // Ãä¦J¼e«×
    FShowFont=true;
    for(i=0;i<MAX_COLOR_INDEX_256;i++)
        ColorMap[i]=TC[i];
    for(i=0;i<MAX_ITEM_256;i++)
    {
        for(j=0;j<MAX_ITEM_256;j++)
        {
            CellColorIndex[i][j]=0;
            CellText[i][j]="5";
        }
    }
    FTrayColor=clWhite;
    FFrameColor=clBlack;    // ¦JÃC¦â
    FTrayDirect=csLeftTop256;
    FDirectWidth=10;
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::CreateWnd()
{
    if(ComponentState.Contains(csDestroying))
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
void __fastcall TTMyTray256::WndProc(TMessage& Message)
{
    TCustomControl::WndProc(Message);
    if (Message.Msg == WM_PAINT || Message.Msg == WM_SIZE)
    {
        CaculateTrayParameter();
        DrawTray();
    }
    else if (Message.Msg == CM_FONTCHANGED)
    {
        DrawTray();
    }
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::ClearCell()
{
    for(int i=0;i<MAX_ITEM_256;i++)
        for(int j=0;j<MAX_ITEM_256;j++)
        {
            CellColorIndex[i][j]=0;
            CellText[i][j]="";
        }
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetXItem(int value)
{
    if( value<=0)
        value=1;
    if(value>=MAX_ITEM_256)
        value=MAX_ITEM_256-1;

    FXItem=value;
    CaculateTrayParameter();
    ClearCell();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetYItem(int value)
{
    if( value<=0)
        value=1;
    if(value>=MAX_ITEM_256)
        value=MAX_ITEM_256-1;
    FYItem=value;
    CaculateTrayParameter();
    ClearCell();
}
//---------------------------------------------------------------------------
void TTMyTray256::SetFrameColor(TColor Color)
{
    FFrameColor=Color;
    DrawTray();
}
//---------------------------------------------------------------------------
void TTMyTray256::SetTrayColor(TColor Color)
{
    FTrayColor=Color;
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::DrawLine(int X,int Y)
{
    if(Visible==false)      //Steven 20160503 : Add Visible
        return;

    DrawTray();
    Canvas->Pen->Color=clRed;
    Canvas->MoveTo(0,Y);
    Canvas->LineTo(Width-1,Y);
    Canvas->MoveTo(X,0);
    Canvas->LineTo(X,Height-1);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::DrawSingleIC(int X,int Y)
{
    if(Visible==false)      //Steven 20160503 : Add Visible
        return;

    if(X>=FXItem || Y>=FYItem)
        return;
    TRect NewRect;
    NewRect.Left=iStartX+X*iPitchX;
    NewRect.Top =iStartY+Y*iPitchY;
    NewRect.Right=NewRect.Left+iXWidth;
    NewRect.Bottom=NewRect.Top+iYWidth;
    Canvas->Brush->Color=ColorMap[CellColorIndex[X][Y]];
    if(FShape)
        Canvas->Ellipse(NewRect.Left,NewRect.Top,NewRect.Right,NewRect.Bottom);
    else
        Canvas->Rectangle(NewRect.Left,NewRect.Top,NewRect.Right,NewRect.Bottom);

    if(FShowFont)
    {
        char str[256]={" "};
        //str[0]=CellText[X][Y];
        strcpy(str,CellText[X][Y].c_str());
        DrawText(Canvas->Handle, str, strlen(str),
        &NewRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
        //TextOut(Canvas->Handle,NewRect.Left,NewRect.Top,str,strlen(str));
    }
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::CaculateTrayParameter()
{
    iXWidth=(Width-2- (FXItem-1)*FLineWidth-2*FEdgeWidth)/FXItem;
    iYWidth=(Height-2-(FYItem-1)*FLineWidth-2*FEdgeWidth)/FYItem;
    iStartX=(Width-iXWidth*FXItem-(FXItem-1)*FLineWidth)/2;
    iStartY=(Height-iYWidth*FYItem-(FYItem-1)*FLineWidth)/2;
    iPitchX=(iXWidth+FLineWidth);
    iPitchY=(iYWidth+FLineWidth);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::DrawTray()
{
    if(Visible==false)      //Steven 20160503 : Add Visible
        return;

    int x,y,i,sx,sy;
    TRect NewRect = Rect(Left, Top,Left+Width ,Top+Height);
    Canvas->Font = Font;
    Canvas->Brush->Color = FTrayColor;
    Canvas->Pen->Color = FFrameColor;
    Canvas->Rectangle(0,0,Width,Height);
    Canvas->Pen->Color=FFrameColor;
     if(FTrayDirect==csLeftTop256)
    {
        sx=0;
        sy=0;
        x=sx;
        y=sy;
        for(i=0;i<DirectWidth;i++)
        {
            Canvas->MoveTo(x,sy);
            Canvas->LineTo(sx,y);
            x++;
            y++;
        }
    }
    else if(FTrayDirect==csLeftBottom256)
    {
        sx=0;
        sy=Height;
        x=sx;
        y=sy;
        for(i=0;i<DirectWidth;i++)
        {
            Canvas->MoveTo(sx,y);
            Canvas->LineTo(x,sy);
            x++;
            y--;
        }
    }
    else if(FTrayDirect==csRightTop256)
    {
        sx=Width;
        sy=0;
        x=sx;
        y=sy;

        for(i=0;i<DirectWidth;i++)
        {
            Canvas->MoveTo(x,sy);
            Canvas->LineTo(sx,y);
            x--;
            y++;
        }
    }
    else if(FTrayDirect==csRightBottom256)
    {
        sx=Width;
        sy=Height;
        x=sx;
        y=sy;
        for(i=0;i<DirectWidth;i++)
        {
            Canvas->MoveTo(sx,y);
            Canvas->LineTo(x,sy);
            x--;
            y--;
        }
    }
    for(x=0;x<FXItem;x++)
        for(y=0;y<FYItem;y++)
            DrawSingleIC(x,y);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetLineWidth(int value)
{
    FLineWidth=value;
    CaculateTrayParameter();
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetEdgeWidth(int value)
{
    FEdgeWidth=value;
    CaculateTrayParameter();
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetDirectWidth(int value)
{
    if(value<0 || value >Width || value>Height)
        return;
    FDirectWidth=value;
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetTrayDirect(TTray256DirectStyle value)
{
    FTrayDirect=value;
    DrawTray();
}
//---------------------------------------------------------------------------
void TTMyTray256::SetColorMap(int Index,TColor Color)
{
    if(Index>=MAX_COLOR_INDEX_256 || Index<0)
        return;
    ColorMap[Index]=Color;
    DrawTray();
}
//---------------------------------------------------------------------------
void TTMyTray256::SetCellColorIndex(int X,int Y,int Index)
{
    if(X>=FXItem || Y>=FYItem || Index>=MAX_COLOR_INDEX_256 || Index<0 || X<0 || Y<0)
        return;
    CellColorIndex[X][Y]=Index;
    DrawSingleIC(X,Y);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetShowFont(bool value)
{
    FShowFont=value;
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetShape(bool value)
{
    FShape=value;
    DrawTray();
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetCellNumber(int x, int y, char *s)
{
    if(x>=FXItem || y>=FYItem || x<0 || y<0)
        return;
    CellText[x][y]=s;
    DrawSingleIC(x,y);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetCellNumber(int x, int y, int s)
{
    char str[256];
    sprintf(str,"%d",s);
    SetCellNumber(x,y,str);
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SetCellNumber(int x, int y, double c)
{
    AnsiString str;
    str.sprintf("%0.2f", c);
    SetCellNumber(x, y, str.c_str());
}
//---------------------------------------------------------------------------
int TTMyTray256::GetCellData(int i,int j)
{
    return CellColorIndex[i][j];
}
//---------------------------------------------------------------------------
int TTMyTray256::ConvertIndexCells(int &X,int &Y)
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
AnsiString TTMyTray256::GetCellText(int X, int Y)
{
    return CellText[X][Y];
}
//---------------------------------------------------------------------------
void __fastcall TTMyTray256::SaveCellTextToFile(AnsiString FileName)
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
namespace Htray256
{
    void __fastcall PACKAGE Register()
    {
         TComponentClass classes[1] = {__classid(TTMyTray256)};
         RegisterComponents("lee40", classes, 0);
    }
}
//---------------------------------------------------------------------------


