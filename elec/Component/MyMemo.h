//---------------------------------------------------------------------------

#ifndef MyMemoH
#define MyMemoH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
//---------------------------------------------------------------------------
enum TSaveType
{
    TByMaxLineCount =0,
    TByHour         =1,
    TBy2Hour        =2,
    TBy4Hour        =3,
    TBy6Hour        =4,
    TBy8Hour        =5,
    TBy12Hour       =6,
    TByDay          =7,
    TByMonth        =8,
    TByYear         =9,
    TByMin          =10         //Steven 20210520 : 加入By分鐘存檔
};
//---------------------------------------------------------------------------
class PACKAGE TMyMemo : public TMemo
{
private:
    AnsiString  HTPath;
    AnsiString  HTFileName;
    AnsiString  HTFirstRow;
    int         HTMaxLineCount;
    TSaveType   HTSaveType;
    bool        HTAutoSave;
    void __fastcall SetPath(AnsiString P);
    void __fastcall SetFirstRow(AnsiString P);
    void __fastcall SetMaxLineCount(int Cnt);
    void __fastcall SetSaveType(TSaveType Type);
    void __fastcall SetFileName(AnsiString P);
    void __fastcall SetAutoSave(bool P);

    Word SystemHour, SystemMin, SystemSec, SystemMSec;
    Word SystemYear, SystemMonth, SystemDate;
    void GetTimeInfo();
protected:
public:
    __fastcall TMyMemo(TComponent* Owner);
    __fastcall ~TMyMemo();
    void __fastcall AddText(AnsiString Msg);
    void __fastcall AddTextWithDateTime(AnsiString Msg);
    void SaveToFile();
    void __fastcall AddTextWithDateTimeIm(AnsiString Msg,bool bimmediately); //kevin 20180816 需立即存資料
    TStringList *MyList;    //因為建構跟解構時,還沒有畫面,用到VCL得東西會死翹翹,所以用StringList做中介   

__published:
    __property AnsiString   Path        ={read=HTPath,          write=SetPath,          default=NULL};
    __property AnsiString   FileName    ={read=HTFileName,      write=SetFileName,      default=NULL};
    __property AnsiString   FirstRow    ={read=HTFirstRow,      write=SetFirstRow,      default=NULL};
    __property int          MaxLineCount={read=HTMaxLineCount,  write=SetMaxLineCount,  default=1000};
    __property TSaveType    SaveType    ={read=HTSaveType,      write=SetSaveType,      default=TByDay};
    __property bool         AutoSave    ={read=HTAutoSave,      write=SetAutoSave,      default=true};
};
//---------------------------------------------------------------------------
#endif
