//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "MapArray.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//
static inline void ValidCtrCheck(TMapArray *)
{
    new TMapArray(NULL);
}
//---------------------------------------------------------------------------
__fastcall TMapArray::TMapArray(TComponent* Owner)
    : TWinControl(Owner)
{
    Width  = 100;//自己元件的寬/高
    Height = 100;
    nXItem = 2;
    nYItem = 2;
    nXSpace= 2;
    nYSpace= 2;
    map = new TImage(this);
    map->Parent= this;
    map->Top   = 0;
    map->Left  = 0;
    map->Width = Width;
    map->Height= Height;

    for(int x=0;x<100;x++)
    for(int y=0;y<100;y++)
        nMap[x][y]=0;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void __fastcall TMapArray::WndProc(TMessage& Message)
{
    TWinControl::WndProc(Message);
    if( Message.Msg == MK_LBUTTON )
    {
        ShowCell();
    }
}
//-------------------------------
void __fastcall TMapArray::CreateWnd()
{
    TWinControl::CreateWnd();
    //畫出預設
    ShowCell();
}
//------------------------------------------------------------------
void __fastcall TMapArray::Paint()
{
    ShowCell();
}
//---------------------------------------------------------------------------
void __fastcall TMapArray::Resize(void)
{
    Invalidate();
    TWinControl::Resize();
}
//--------------------------------------
void __fastcall TMapArray::OnChange()
{
    ShowCell();
}
//--------------------------------------
void __fastcall TMapArray::SetXItem(int a)
{
    nXItem = a;
    ShowCell();
}
//--------------------------------------
void __fastcall TMapArray::SetYItem(int a)
{
    nYItem = a;
    ShowCell();
}
//------------------------------------------------------------------
void TMapArray::ShowCell()
{
    int x,y;
    float fStepX =0.0;
    float fStepY =0.0;

    fStepX=(Width- (nXSpace*(nXItem+1)))/nXItem;
    fStepY=(Height-(nYSpace*(nYItem+1)))/nYItem;
    //先畫底
    map->Canvas->Brush->Color = 0x00c08080;
    R=Rect(0,0,Width,Height);
    map->Canvas->FillRect(R);
    //畫map
    for(int dwx=0;dwx<nXItem;dwx++)
    for(int dwy=0;dwy<nYItem;dwy++)
    {
        x = nXSpace+nXSpace*(dwx+1)+fStepX*dwx;
        y = nYSpace+nYSpace*(dwy+1)+fStepY*dwy;
        R = Rect(x,y,x+fStepX,y+fStepY);
        map->Canvas->Brush->Color=clBlack;
        map->Canvas->FillRect(R);
        switch( nMap[dwx][dwy] )
        {
            case 0:map->Canvas->Brush->Color=clGreen;break;
            case 1:map->Canvas->Brush->Color=clRed  ;break;
            case 2:map->Canvas->Brush->Color=clBtnFace;break;
            case 3:map->Canvas->Brush->Color=0x00c08080;break;
            case 4:map->Canvas->Brush->Color=clYellow;break;
            case 5:map->Canvas->Brush->Color=clWhite;break;
        }
        R=Rect(x+1,y+1,x+fStepX-1,y+fStepY-1);
        map->Canvas->FillRect(R);
    }
}
//---------------------------------------------------------------------------
namespace Maparray
{
    void __fastcall PACKAGE Register()
    {
         TComponentClass classes[1] = {__classid(TMapArray)};
         RegisterComponents("HungKai", classes, 0);
    }
}
//---------------------------------------------------------------------------
