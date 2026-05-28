//---------------------------------------------------------------------------

#include <vcl.h>

#pragma hdrstop

#include "MyMemo.h"

#include <stdio.h>
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TMyMemo *)
{
    new TMyMemo(NULL);
}
//---------------------------------------------------------------------------
__fastcall TMyMemo::TMyMemo(TComponent* Owner)
    : TMemo(Owner)
{
    MaxLineCount        =1000;
    Path                ="D:\\HandlerLog";
    FileName            =this->Name;
    FirstRow            ="";    
    SaveType            =TByDay;
    AutoSave            =true;
    this->ScrollBars    =ssBoth;
//    this->Text          ="";
//    this->Parent=(TWinControl *)Owner;
    MyList              =new TStringList();
//    this->Lines->Clear();
}
//---------------------------------------------------------------------------
__fastcall TMyMemo::~TMyMemo()
{
    SaveToFile();
    delete MyList;
}
//---------------------------------------------------------------------------
void __fastcall TMyMemo::SetPath(AnsiString P)
{
    HTPath=P;
}
//---------------------------------------------------------------------------
void __fastcall TMyMemo::SetFileName(AnsiString P)
{
    AnsiString Str="";
    if(this->Parent!=NULL)
    {
        TWinControl* PP=this->Parent;
        while(PP->Parent!=NULL)
        {
            PP=PP->Parent;
        };
        Str=PP->Name;
    }

    if(P=="")
    {
        if(Str=="")
            HTFileName=this->Name;
        else
            HTFileName=Str+"_"+this->Name;
    }
    else
        HTFileName=P;
}
//---------------------------------------------------------------------------
void __fastcall TMyMemo::SetFirstRow(AnsiString P)
{
    HTFirstRow=P;
}
//---------------------------------------------------------------------------
void __fastcall TMyMemo::SetMaxLineCount(int Cnt)
{
    HTMaxLineCount=Cnt;
}
//---------------------------------------------------------------------------
void __fastcall TMyMemo::SetSaveType(TSaveType Type)
{
    HTSaveType=Type;
}
//---------------------------------------------------------------------------
void __fastcall TMyMemo::SetAutoSave(bool P)
{
    HTAutoSave=P;
}
//---------------------------------------------------------------------------
void __fastcall TMyMemo::AddText(AnsiString Msg)
{
    GetTimeInfo();

    if(FileName=="")
        FileName="";   

    if(MyList->Count>HTMaxLineCount)
    {
        SaveToFile();
        this->Clear();
        MyList->Clear();
    }
    MyList->Add(Msg);
    this->Text=MyList->Text;
//    this->Lines->Add(Msg);
}
//---------------------------------------------------------------------------
void __fastcall TMyMemo::AddTextWithDateTime(AnsiString Msg)
{
    AnsiString Str;
    GetTimeInfo();
    Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, Msg);

    if(FileName=="")
        FileName="";   

    if(MyList->Count>HTMaxLineCount)
    {
        SaveToFile();
        this->Clear();
        MyList->Clear();
    }
    MyList->Add(Str);
    this->Text=MyList->Text;
//    this->Lines->Add(Str);
}
//---------------------------------------------------------------------------
void __fastcall TMyMemo::AddTextWithDateTimeIm(AnsiString Msg,bool bimmediately)
{
    AnsiString Str;
    GetTimeInfo();
    Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, Msg);

    if(FileName=="")
        FileName="";   

    if(MyList->Count>HTMaxLineCount || bimmediately) //kevin 20180816 需立即存資料
    {
        SaveToFile();
        this->Clear();
        MyList->Clear();
    }
    MyList->Add(Str);
    this->Text=MyList->Text;
//    this->Lines->Add(Str);
}
//---------------------------------------------------------------------------
void TMyMemo::GetTimeInfo()
{
    static TDateTime dtPresent;
    dtPresent= Now();
    DecodeDate(dtPresent, SystemYear, SystemMonth, SystemDate);
    DecodeTime(dtPresent, SystemHour, SystemMin, SystemSec, SystemMSec);
}
//---------------------------------------------------------------------------
void TMyMemo::SaveToFile()
{
    AnsiString sPathName;
    AnsiString sFileName;
    AnsiString Str;
    int iHour;
    FILE *pFile;    
    GetTimeInfo();

    if(HTAutoSave==false || MyList->Count==0)   //沒資料就不用存檔
        return;

    if(HTPath=="")
        Path="D:\\HandlerLog";

    if(FileName=="")
        FileName="";

    if(HTSaveType==TByMaxLineCount)    //kevin 20180815 add  by day save file
    {
        sPathName.sprintf("%s\\%04d\\%02d", HTPath, SystemYear, SystemMonth);
    }
    else if(HTSaveType>=TByMonth)    //使用年月存檔的話,就By年分類
    {
        sPathName.sprintf("%s\\%04d", HTPath, SystemYear);
    }
    else if(HTSaveType>=TByDay) //使用每日存檔的話,就By月分類
    {
        sPathName.sprintf("%s\\%04d\\%02d", HTPath, SystemYear, SystemMonth);
    }
    else                        //其他就每年存成365個資料夾
    {
           sPathName.sprintf("%s\\%04d\\%02d\\%02d", HTPath, SystemYear, SystemMonth, SystemDate);
    }

    if(DirectoryExists(sPathName)==false)
    {
        ForceDirectories(sPathName);
    }

    if(HTSaveType==TByMaxLineCount)
    {
        //sFileName.sprintf("%s\\%s_%04d%02d%02d %02d%02d%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        sFileName.sprintf("%s\\%s_%04d%02d%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate); //kevin 20180815 add
    }
    else
    {
        if(HTSaveType==TByDay)
        {
            sFileName.sprintf("%s\\%s_%04d%02d%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate);
        }
        else if(HTSaveType==TByMonth)
        {
            sFileName.sprintf("%s\\%s_%04d%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth);
        }
        else if(HTSaveType==TByYear)
        {
            sFileName.sprintf("%s\\%s_%04d.csv", sPathName, HTFileName, SystemYear);
        }
        else if(HTSaveType==TByHour)
        {
            sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour);
        }
        else if(HTSaveType==TBy2Hour)
        {
            iHour=SystemHour-SystemHour%2;
            sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy4Hour)
        {
            iHour=SystemHour-SystemHour%4;
            sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy6Hour)
        {
            iHour=SystemHour-SystemHour%6;
            sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy8Hour)
        {
            iHour=SystemHour-SystemHour%8;
            sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy12Hour)
        {
            iHour=SystemHour-SystemHour%12;
            sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else
        {
            sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
        }
    }

    if(FileExists(sFileName)==false && HTFirstRow!="")
    {
//        Str=HTFirstRow+"\r\n"+MyList->Text;
        Str.sprintf("%s\r\n%s", HTFirstRow, MyList->Text.c_str());
    }
    else
    {
        Str.sprintf("%s", MyList->Text.c_str());
    }

    Str=StringReplace(Str, "\r\n", "\n", TReplaceFlags()<<rfReplaceAll);
    pFile=fopen(sFileName.c_str(), "a");
    if(pFile!=NULL)
    {
        fputs(Str.c_str(), pFile);
        fclose(pFile);
    }

    MyList->Clear();
}
//---------------------------------------------------------------------------
namespace Mymemo
{
    void __fastcall PACKAGE Register()
    {
         TComponentClass classes[1] = {__classid(TMyMemo)};
         RegisterComponents("Extras", classes, 0);
    }
}
//---------------------------------------------------------------------------
 
