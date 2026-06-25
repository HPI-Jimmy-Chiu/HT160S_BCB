//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <stdio.h>

#include "systools.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormSysTools *FormSysTools;
//---------------------------------------------------------------------------
__fastcall TFormSysTools::TFormSysTools(TComponent* Owner)
    : TForm(Owner)
{
    ShowTimeStringList=new TList;
    ShowTimeStringList->Clear();
}
//---------------------------------------------------------------------------
//AI(ht160s-statusbar) 20260624 : port of HT172 systools.cpp:996. Register a
//TStatusPanel/TControl pointer (de-duped by pointer) to be filled with the time.
void __fastcall TFormSysTools::AddMyTimeStringShow(TObject *RefPanelPtr, int DataType)
{
    MyTimeStringShowList *P2;
    MyTimeStringShowList *P;
    int i;

    if(ShowTimeStringList==NULL)
        return;
    for(i=0; i<ShowTimeStringList->Count; i++)
    {
        P2=(MyTimeStringShowList *)ShowTimeStringList->Items[i];
        if(P2->palPtr==RefPanelPtr)
        {
            ShowTimeStringList->Delete(i);
            delete P2;
            break;
        }
    }
    P=new MyTimeStringShowList;
    P->palPtr=RefPanelPtr;
    if(DataType<0 || DataType>1)
        DataType=0;
    P->dataType=DataType;
    ShowTimeStringList->Add((void *)P);
}
//---------------------------------------------------------------------------
//AI(ht160s-statusbar) 20260624 : port of HT172 systools.cpp:1018. Builds the time
//string and writes it into each registered panel/control. Self-throttles to once
//per changed second via a static last-second guard even though the host timer is
//100ms. HT160 has no System* globals, so it reads Now()/DecodeDateTime directly.
void __fastcall TFormSysTools::RefreshMyTimeString()
{
    static int OldSec=-1;
    char str[2][64];
    unsigned short Yr, Mo, Dy, Hr, Mn, Sc, Ms;
    int i;
    MyTimeStringShowList *P;
    TStatusPanel *pPanel;
    TControl *pControl;
    TDateTime Tnow;

    if(ShowTimeStringList==NULL || ShowTimeStringList->Count==0)
        return;

    Tnow=Now();
    DecodeDate(Tnow, Yr, Mo, Dy);
    DecodeTime(Tnow, Hr, Mn, Sc, Ms);

    if(OldSec!=(int)Sc)
        OldSec=(int)Sc;
    else
        return;

    sprintf(str[0],"%04d-%02d-%02d  %02d:%02d:%02d", Yr, Mo, Dy, Hr, Mn, Sc);
    if(Hr>=13)
        sprintf(str[1],"%04d-%02d-%02d  %02d:%02d:%02d PM", Yr, Mo, Dy, Hr-12, Mn, Sc);
    else
        sprintf(str[1],"%04d-%02d-%02d  %02d:%02d:%02d AM", Yr, Mo, Dy, Hr, Mn, Sc);

    for(i=0; i<ShowTimeStringList->Count; i++)
    {
        P=(MyTimeStringShowList *)ShowTimeStringList->Items[i];
        pPanel=dynamic_cast<TStatusPanel *>(P->palPtr);
        if(pPanel!=NULL)
            pPanel->Text=str[P->dataType];
        pControl=dynamic_cast<TControl *>(P->palPtr);
        if(pControl!=NULL)
            pControl->SetTextBuf(str[P->dataType]);
    }
}
//---------------------------------------------------------------------------
