//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "TMapArray.h"
#include "stdio.h"
#include "io.h"
#pragma package(smart_init)
//---------------------------------------------------------------
static inline void ValidCtrCheck(TArrayMap *)
{
    new TArrayMap(NULL);
}
//---------------------------------------------------------------
__fastcall TArrayMap::TArrayMap(TComponent* Owner)
    : TImage(Owner)
{

    Width  = 200;//じン糴/蔼
    Height = 200;
    nXItem = 2;
    nYItem = 2;
    MapArrayReset();
}
//-----------------------------------------------------------------------
void TArrayMap::MapArrayReset()
{
    nFirstPos=0;      //0:オ  1:オ 2: 3:
    nSearchMode=0;    //0;X Dir 1:Y Dir
    nContinousMode=0; //0:Yes   1:No
    nXPos=0;
    nYPos=0;
    for(int x=0;x<100;x++)
    for(int y=0;y<100;y++)
        nMap[x][y]=0;
    ShowMap();
}
//--------------------------------------------------------------------
namespace Tmaparray
{
    void __fastcall PACKAGE Register()
    {
         TComponentClass classes[1] = {__classid(TArrayMap)};
         RegisterComponents("HungKai", classes, 0);
    }
}
//---------------------------------------------------------------------------
void __fastcall TArrayMap::Paint(void)
{
    TImage::Resize();
//    ShowMap();
//    Invalidate();
    TImage::Paint();
}
//---------------------------------------------------------------------------
void __fastcall TArrayMap::MouseDown(TMouseButton Button, TShiftState Shift, int X, int Y)
{

    int i,j,x,y;
    int SY    = Top;
    int SX    = Left;
    float fStepX = 1.0;
    float fStepY = 1.0;

    fStepX = Width /nXItem;
    fStepY = Height/nYItem;

    bOnManualSet = false;

    bool flag=false;
    for(i=0;i<nXItem;i++)
    {
        for(j=0;j<nYItem;j++)
        {
            x=fStepX*i;
            y=fStepY*j;
            if( X>=x && X<=(x+fStepX) && Y>=y && Y<=(y+fStepY) )
            {
                flag=true;
                break;
            }
        }
        if( flag )
            break;
    }
    //-----------------------------------------------------
    if( flag )
    {
        bOnManualSet = true;
    }
}
//---------------------------------------------------------------------------
void __fastcall TArrayMap::MouseUp(TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if( !bOnManualSet )
        return;

    bOnManualSet = false;
    int i,j,x,y;
    int SY    = Top;
    int SX    = Left;
    float fStepX = 0.0;
    float fStepY = 0.0;

    fStepX = Width /nXItem;
    fStepY = Height/nYItem;

    bOnManualSet = false;

    bool flag=false;

    for(i=0;i<nXItem;i++)
    {
        for(j=0;j<nYItem;j++)
        {
            x=fStepX*i;
            y=fStepY*j;
            if( X>=x && X<=(x+fStepX) && Y>=y && Y<=(y+fStepY) )
            {
                flag=true;
                break;
            }
        }
        if( flag )
            break;
    }
    //--------------------------------------------------------
    if( flag )
    {
        nSelectX = i;
        nSelectY = j;
        if( nMap[nSelectX][nSelectY] == 1)
            nMap[nSelectX][nSelectY]=0;
        else
            nMap[nSelectX][nSelectY]=1;
        ShowCell(nSelectX,nSelectY);
    }
}
//---------------------------------------------------------------------------
void __fastcall TArrayMap::Resize(void)
{
    Invalidate();
    TImage::Resize();
    TImage::Paint();
    ShowMap();
}
//---------------------------------------------------------------------------
void __fastcall TArrayMap::SetXItem(int a)
{
    nXItem = a;
    ShowMap();
}
//---------------------------------------------------------------------------
void __fastcall TArrayMap::SetYItem(int a)
{
    nYItem = a;
    ShowMap();
}
//--------------------------------------------------------------------
void TArrayMap::ShowCell(int XPos,int YPos)
{
    int XStart,YStart;
    float fStepX =0.0;
    float fStepY =0.0;

    fStepX=(Width- (nXSpace*(nXItem+1)))/nXItem;
    fStepY=(Height-(nYSpace*(nYItem+1)))/nYItem;

    //礶cell
    XStart = nXSpace+nXSpace*(XPos+1)+fStepX*XPos;
    YStart = nYSpace+nYSpace*(YPos+1)+fStepY*YPos;
    R = Rect(XStart,YStart,XStart+fStepX,YStart+fStepY);
    Canvas->Brush->Color=clBlack;
    Canvas->FillRect(R);
    switch( nMap[XPos][YPos] )
    {
        case 0:Canvas->Brush->Color=clGreen;break;
        case 1:Canvas->Brush->Color=clRed  ;break;
        case 2:Canvas->Brush->Color=clBtnFace;break;
        case 3:Canvas->Brush->Color=(TColor)0x00c08080;break;
        case 4:Canvas->Brush->Color=clYellow;break;
        case 5:Canvas->Brush->Color=clWhite;break;
        default:Canvas->Brush->Color=clWhite;break;
    }
    R=Rect(XStart+1,YStart+1,XStart+fStepX-1,YStart+fStepY-1);
    Canvas->FillRect(R);
}
//--------------------------------------------------------------------
void TArrayMap::ShowMap()
{
    int x,y;
    float fStepX =0.0;
    float fStepY =0.0;
    fStepX=(Width- (nXSpace*(nXItem+1)))/nXItem;
    fStepY=(Height-(nYSpace*(nYItem+1)))/nYItem;
/*
    TRect rc  = GetClientRect();
    int a=rc.Top;
    int b=rc.Left;
    int c=rc.Right;
    int d=rc.Bottom;
    int e=Top;
    int f=Left;
    int g=Width;
    int h=Height;
*/

    //礶┏(穝)
    R=Rect(0,0,Width,Height);
    Canvas->Brush->Color =(TColor)0x00c08080;
    Canvas->FillRect(R);
    //礶map
    for(int dwx=0;dwx<nXItem;dwx++)
    for(int dwy=0;dwy<nYItem;dwy++)
        ShowCell(dwx,dwy);
    //礶嘿
    Canvas->Brush->Color= clBtnFace;
    Canvas->Font->Color = clBlue;
    Canvas->Font->Size  = 9;
    char cName[64];
    sprintf(cName,"%-s",Name);
    Canvas->TextOut(Top-20,Left+20,cName);
}
//--------------------------------------------------------------------
void __fastcall TArrayMap::WndProc(TMessage& Message)
{

    TImage::WndProc(Message);
    if( Message.Msg == WM_SIZING )
    {
        ShowMap();
    }
    if( Message.Msg == CM_CTL3DCHANGED)//:SHOWINGCHANGED)//CM_CHANGED )
    {
        ShowMap();
    }
}
//--------------------------------------------------------------------
// Search map next position
//--------------------------------------------------------------------
bool TArrayMap::SearchNextPos()
{
    bool bFlag = true;
    //------------------------------------------------
    //  材Ω
    if( nXPos==0 && nYPos==0 )
    {
        switch( nFirstPos )
        {
            case 0:
                nXPos = 1;
                nYPos = 1;
                break;
            case 1:
                nXPos = 1;
                nYPos = nYItem;
                break;
            case 2:
                nXPos = nXItem;
                nYPos = 1;
                break;
            case 3:
                nXPos = nXItem;
                nYPos = nYItem;
                break;
        }
    }
    else
    {
        //璸衡币﹍翴
        switch( nFirstPos )
        {
            case 0://L-Up
                if( nSearchMode )//0;X Dir 1:Y Dir
                {
                    if( nXPos%2 )//虫计
                        nYPos++;
                    else        //案计
                        nYPos--;
                    if( nYPos<1 || nYPos>nYItem )
                    {
                        nXPos++;
                        if( nXPos>nXItem )
                            bFlag=false;
                        else
                        {
                            if( nXPos%2 )
                                nYPos=1;
                            else
                                nYPos=nYItem;
                        }
                    }
                }
                else
                {
                    if( nYPos%2 )//虫计
                        nXPos++;
                    else        //案计
                        nXPos--;
                    if( nXPos<1 || nXPos>nXItem )
                    {
                        nYPos++;
                        if( nYPos>nYItem )
                            bFlag=false;
                        else
                        {
                            if( nYPos%2 )
                                nXPos=1;
                            else
                                nXPos=nXItem;
                        }
                    }
                }
                break;
            case 1://L-Down
                if( nSearchMode )//0;X Dir 1:Y Dir
                {
                    if( nXPos%2 )//虫计
                        nYPos--;
                    else        //案计
                        nYPos++;
                    if( nYPos<1 || nYPos>nYItem )
                    {
                        nXPos++;
                        if( nXPos>nXItem )
                            bFlag=false;
                        else
                        {
                            if( nXPos%2 )
                                nYPos=nYItem;
                            else
                                nYPos=1;
                        }
                    }
                }
                else
                {
                    if( nYPos%2 )//虫计
                        nXPos--;
                    else        //案计
                        nXPos++;
                    if( nXPos<1 || nXPos>nXItem )
                    {
                        nYPos--;
                        if( nYPos<1 )
                            bFlag=false;
                        else
                        {
                            if( nYPos%2 )
                                nXPos=nXItem;
                            else
                                nXPos=1;
                        }
                    }
                }
                break;
            case 2://R-Up
                if( nSearchMode )//0;X Dir 1:Y Dir
                {
                    if( nXPos%2 )//虫计
                        nYPos--;
                    else        //案计
                        nYPos++;
                    if( nYPos<1 || nYPos>nYItem )
                    {
                        nXPos--;
                        if( nXPos<1 )
                            bFlag=false;
                        else
                        {
                            if( nXPos%2 )
                                nYPos=nYItem;
                            else
                                nYPos=1;
                        }
                    }
                }
                else
                {
                    if( nYPos%2 )//虫计
                        nXPos--;
                    else        //案计
                        nXPos++;
                    if( nXPos<1 || nXPos>nXItem )
                    {
                        nYPos++;
                        if( nYPos>nYItem )
                            bFlag=false;
                        else
                        {
                            if( nYPos%2 )
                                nXPos=nXItem;
                            else
                                nXPos=1;//nXItem;
                        }
                    }
                }
                break;
            case 3://R-Down
                if( nSearchMode )//0;X Dir 1:Y Dir
                {
                    if( nXPos%2 )//虫计
                        nYPos++;
                    else        //案计
                        nYPos--;
                    if( nYPos<1 || nYPos>nYItem )
                    {
                        nXPos--;
                        if( nXPos<1 )
                            bFlag=false;
                        else
                        {
                            if( nXPos%2 )
                                nYPos=1;
                            else
                                nYPos=nYItem;
                        }
                    }
                }
                else
                {
                    if( nYPos%2 )//虫计
                        nXPos++;
                    else        //案计
                        nXPos--;
                    if( nXPos<1 || nXPos>nXItem )
                    {
                        nYPos--;
                        if( nYPos<1 )
                            bFlag=false;
                        else
                        {
                            if( nYPos%2 )
                                nXPos=1;//nXItem;//1;
                            else
                                nXPos=nXItem;
                        }
                    }
                }
                break;
        }
    }
    return bFlag;
}
//-------------------------------------------------------------------
// Map戈郎
// 1:OK
// 0:Fail/Not test
//-------------------------------------------------------------------
bool TArrayMap::fnMapSave2File(AnsiString FileName)
{

    FILE *fout;
    fout=fopen(FileName.c_str(),"w+");
    if (access(FileName.c_str(),0) != 0)
    {
        fclose(fout);
        return false;
    }
    int XPos;
    int YPos;
    char TempChar;
    for( YPos=1;YPos<=nYItem;YPos++ )
    {
        for (XPos=1;XPos<=nXItem;XPos++)
        {
            if (nMap[XPos-1][YPos-1] == 0 )
                putc('1',fout);
            else
                putc('0',fout);
        }
        putc('\n',fout);
    }
    fclose(fout);
    return true;
}
//-------------------------------------------------------------------
bool TArrayMap::fnMapSave2RePort(AnsiString FileName)
{

    AnsiString Fail,Recovery,Mask,XItem,YItem,TestXPos;
    AnsiString UsedFileName;
    AnsiString OperatorID;
    AnsiString TestBIBID;
    AnsiString TestBIBDate;
    AnsiString TestBIBTime;

    int TestCNT=0;
    int PassCNT=0;
    int FailCNT=0;
    int RecoveryCNT=0;
    int MaskCNT=0;
    int XPos;
    int YPos;
    char TempBuffer[64];
    char str[256];

    FILE *fout;
    fout=fopen(FileName.c_str(),"w+");
    if (access(FileName.c_str(),0) != 0)
    {
        fclose(fout);
        return false;
    }

    for(YPos=0;YPos<=nYItem;YPos++)
    {
        for(XPos=0;XPos<=nXItem;XPos++)
        {
            if( nMap[XPos][YPos]== 0)
            {
                PassCNT++;
                TestCNT++;
            }
            else if (nMap[XPos][YPos]== 1)
            {
                FailCNT++;
                TestCNT++;
            }
            else if (nMap[XPos][YPos]!=0)
                RecoveryCNT++;
            else if (nMap[XPos][YPos]==5)
                MaskCNT++;
        }
    }

    Recovery     = RecoveryCNT;
    Mask         = MaskCNT;
    XItem  = nXItem;
    YItem  = nYItem;

    putc('\n',fout);
    fputs("                  Hon.Tech SYSTEM",fout);putc('\n',fout);
    putc('\n',fout);
    fputs("               GoogWell TESTER SYSTEM",fout);putc('\n',fout);
    putc('\n',fout);
    fputs("BOARD TEST REPORT",fout);
    putc('\n',fout);
    putc('\n',fout);
    fputs("Board Name :",fout);

    sprintf(str,"%-*.*s",UsedFileName.Length(),
                         UsedFileName.Length(),
                         UsedFileName.c_str());
    fputs(str,fout);
    putc('\n',fout);

    fputs("Operator ID:",fout);
    sprintf(str,"%-*.*s",OperatorID.Length(),
                         OperatorID.Length(),
                         OperatorID.c_str());
    fputs(str,fout);
    putc('\n',fout);

    fputs("BIBID      :",fout);
    sprintf(str,"%-*.*s",TestBIBID.Length(),
                         TestBIBID.Length(),
                         TestBIBID.c_str());
    fputs(str,fout);
    putc('\n',fout);

    fputs("Date       :",fout);
    sprintf(str,"%-*.*s",TestBIBDate.Length(),
                         TestBIBDate.Length(),
                         TestBIBDate.c_str());
    fputs(str,fout);
    putc('\n',fout);

    fputs("Time       :",fout);
    sprintf(str,"%-*.*s",TestBIBTime.Length(),
                         TestBIBTime.Length(),
                         TestBIBTime.c_str());
    fputs(str,fout);
    putc('\n',fout);

    putc('\n',fout);
    int ip=0;
    fputs("     ",fout);
    for(XPos=0;XPos<nXItem;XPos++)
    {
        if (XPos<9)
            sprintf(str,"%d  ",XPos+1);
        else
            sprintf(str,"%d ",XPos+1);
        fputs(str,fout);
    }
    putc('\n',fout);
    int RealPos;
    for( YPos=0;YPos<nYItem;YPos++ )
    {
        RealPos = nYItem-YPos;
        if (RealPos >=10)
            sprintf(str,"%d   " ,RealPos);
        else
            sprintf(str,"%d    ",RealPos);

        fputs(str,fout);
        for( XPos=0;XPos<nXItem;XPos++ )
        {
            if (nMap[XPos][YPos]==5)
            {
                fputs("-  ",fout);
            }
            else if (nMap[XPos][YPos] ==0)
            {
                fputs("P  ",fout);
            }
            else if (nMap[XPos][YPos]==1)
            {
                fputs("X  ",fout);
            }
            else if (nMap[XPos][YPos]==2)
            {
                fputs("-  ",fout);
            }
            else if (nMap[XPos][YPos]==3)
            {
                fputs("-  ",fout);
            }
            ip++;
        }
        putc('\n',fout);
    }
    putc('\n',fout);
    putc('\n',fout);
    fputs("P = Pass",fout);
    putc('\n',fout);
    fputs("X = Failed",fout);
    putc('\n',fout);
    fputs("- = Not Tested",fout);
    putc('\n',fout);

    sprintf(str,"%03d    sockets tested",TestCNT);
    fputs(str,fout);
    putc('\n',fout);
    sprintf(str,"%03d    sockets passed",PassCNT);
    fputs(str,fout);
    putc('\n',fout);
    sprintf(str,"%03d    sockets failed",FailCNT);
    fputs(str,fout);
    putc('\n',fout);
    fclose(fout);
    return true;
}
