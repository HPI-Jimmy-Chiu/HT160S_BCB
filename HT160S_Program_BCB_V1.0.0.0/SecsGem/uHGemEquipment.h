//---------------------------------------------------------------------------
#ifndef uHGemEquipmentH
#define uHGemEquipmentH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <ExtCtrls.hpp>
#include <SysUtils.hpp>
//---------------------------------------------------------------------------
struct HTypeStruct
{
    unsigned char LIST_TYPE;
    unsigned char ASCII_TYPE;
    unsigned char JIS_TYPE;
    unsigned char BINARY_TYPE;
    unsigned char BOOLEAN_TYPE;
    unsigned char UINT_1_TYPE;
    unsigned char UINT_2_TYPE;
    unsigned char UINT_4_TYPE;
    unsigned char UINT_8_TYPE;
    unsigned char INT_1_TYPE;
    unsigned char INT_2_TYPE;
    unsigned char INT_4_TYPE;
    unsigned char INT_8_TYPE;
    unsigned char FT_4_TYPE;
    unsigned char FT_8_TYPE;
    unsigned char VCL_TYPE;
};
//---------------------------------------------------------------------------
extern HTypeStruct HType;
//---------------------------------------------------------------------------
class THGem : public TComponent
{
private:
    int iTimeFormat;
    AnsiString sDefaultAddress;
    AnsiString sDefaultPort;
    AnsiString sDeviceId;
    AnsiString sRecipeDirectory;
    AnsiString sRecipeFileMask;
    int iRecipeDirectoryType;
    AnsiString sMachineType;
    AnsiString sSoftwareVersion;
    TStringList *LogList;
    void __fastcall Timer1Timer(TObject *Sender);
public:
    TTimer *Timer1;
    AnsiString Alias;
    AnsiString CurrentDirectory;

    __fastcall THGem(TComponent *Owner);
    __fastcall ~THGem();

    void SetTimeFormat(int Format);
    void SetDefaultAddressAndPort(char *Address, char *Port);
    void SetDefaultAddressAndPort(char *Address, char *Port, char *DeviceId);
    int SetReceipeDirectoryAndGlobalName(AnsiString Path, AnsiString FileMask, int Type);
    void SetMachineTypeAndSoftwarseVer(char *MachineType, char *SoftwareVersion);
    void SaveEventReportData();
    void EventReport(unsigned iDataID, unsigned iCeid);
    bool IsEnableEvent(unsigned iDataID, unsigned iCeid);
    void StringOut(AnsiString Text);

    void SetSVDataPointer(unsigned SVID, unsigned char Type, AnsiString Name, AnsiString Unit, void *DataPtr, AnsiString Description);
    void SetECDataPointer(unsigned ECID, unsigned char Type, AnsiString Name, AnsiString Unit, void *DataPtr, AnsiString MinValue, AnsiString MaxValue, AnsiString DefaultValue, AnsiString Description);
    void SetAlamData(int Index, AnsiString AlarmCode, AnsiString UnitName, AnsiString Message, AnsiString AlarmType);
    void ReadAlamData();
    void WriteAlamData();
    void ReadEventReportData();
    void SetCEIDContent(unsigned iCeid, unsigned iReportCount, unsigned *iReportIDData, int Mode);
    void SetCEIDContent(unsigned iCeid, AnsiString CeidAlias, unsigned iReportCount, unsigned *iReportIDData, int Mode);
    bool SetReportIDContent(unsigned iReportID, unsigned iReportCount, unsigned *iReportIDData, int Mode);

    void InitLocalHead(int Stream, int Function, int WaitBit);
    void DataItemOut(int Len, unsigned char Type, void *Value);
    void SendLocalData();
    int DataItemIn(int Len, unsigned char Type, void *Value);
    int GetDataItemLenAndType(int &Len, unsigned char &Type);
    int GetDataItemLenAndTypeAndDelete(int &Len, unsigned char Type);
    bool CheckSFFormatOnlyHead(AnsiString ErrStr);
};
//---------------------------------------------------------------------------
extern THGem *HGem;
//---------------------------------------------------------------------------
#endif
