//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "uHGemEquipment.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
THGem *HGem = NULL;
HTypeStruct HType = {0x00, 0x40, 0x44, 0x20, 0x24, 0xA4, 0xA8, 0xB0, 0xA0, 0x64, 0x68, 0x70, 0x60, 0x90, 0x80, 0xFF};
//---------------------------------------------------------------------------
__fastcall THGem::THGem(TComponent *Owner)
    : TComponent(Owner)
{
    iTimeFormat = 1;
    iRecipeDirectoryType = 0;
    Alias = "SECS";
    CurrentDirectory = "..\\SECS";
    LogList = new TStringList;
    Timer1 = new TTimer(this);
    Timer1->Enabled = false;
    Timer1->Interval = 1000;
    Timer1->OnTimer = Timer1Timer;
}
//---------------------------------------------------------------------------
__fastcall THGem::~THGem()
{
    if(LogList!=NULL)
    {
        LogList->Clear();
        delete LogList;
        LogList = NULL;
    }
}
//---------------------------------------------------------------------------
void __fastcall THGem::Timer1Timer(TObject *Sender)
{
}
//---------------------------------------------------------------------------
void THGem::SetTimeFormat(int Format)
{
    iTimeFormat = Format;
}
//---------------------------------------------------------------------------
void THGem::SetDefaultAddressAndPort(char *Address, char *Port)
{
    SetDefaultAddressAndPort(Address, Port, "0");
}
//---------------------------------------------------------------------------
void THGem::SetDefaultAddressAndPort(char *Address, char *Port, char *DeviceId)
{
    sDefaultAddress = Address;
    sDefaultPort = Port;
    sDeviceId = DeviceId;
}
//---------------------------------------------------------------------------
int THGem::SetReceipeDirectoryAndGlobalName(AnsiString Path, AnsiString FileMask, int Type)
{
    sRecipeDirectory = Path;
    sRecipeFileMask = FileMask;
    iRecipeDirectoryType = Type;
    return 0;
}
//---------------------------------------------------------------------------
void THGem::SetMachineTypeAndSoftwarseVer(char *MachineType, char *SoftwareVersion)
{
    sMachineType = MachineType;
    sSoftwareVersion = SoftwareVersion;
}
//---------------------------------------------------------------------------
void THGem::SaveEventReportData()
{
    StringOut("[SECS] SaveEventReportData placeholder");
}
//---------------------------------------------------------------------------
void THGem::EventReport(unsigned iDataID, unsigned iCeid)
{
    AnsiString Text;
    Text.sprintf("[SECS] EventReport DataID=%u CEID=%u", iDataID, iCeid);
    StringOut(Text);
}
//---------------------------------------------------------------------------
bool THGem::IsEnableEvent(unsigned iDataID, unsigned iCeid)
{
    return true;
}
//---------------------------------------------------------------------------
void THGem::StringOut(AnsiString Text)
{
    if(LogList!=NULL)
        LogList->Add(Text);
}
//---------------------------------------------------------------------------
void THGem::SetSVDataPointer(unsigned SVID, unsigned char Type, AnsiString Name, AnsiString Unit, void *DataPtr, AnsiString Description)
{
}
//---------------------------------------------------------------------------
void THGem::SetECDataPointer(unsigned ECID, unsigned char Type, AnsiString Name, AnsiString Unit, void *DataPtr, AnsiString MinValue, AnsiString MaxValue, AnsiString DefaultValue, AnsiString Description)
{
}
//---------------------------------------------------------------------------
void THGem::SetAlamData(int Index, AnsiString AlarmCode, AnsiString UnitName, AnsiString Message, AnsiString AlarmType)
{
}
//---------------------------------------------------------------------------
void THGem::ReadAlamData()
{
}
//---------------------------------------------------------------------------
void THGem::WriteAlamData()
{
}
//---------------------------------------------------------------------------
void THGem::ReadEventReportData()
{
}
//---------------------------------------------------------------------------
void THGem::SetCEIDContent(unsigned iCeid, unsigned iReportCount, unsigned *iReportIDData, int Mode)
{
}
//---------------------------------------------------------------------------
void THGem::SetCEIDContent(unsigned iCeid, AnsiString CeidAlias, unsigned iReportCount, unsigned *iReportIDData, int Mode)
{
}
//---------------------------------------------------------------------------
bool THGem::SetReportIDContent(unsigned iReportID, unsigned iReportCount, unsigned *iReportIDData, int Mode)
{
    return true;
}
//---------------------------------------------------------------------------
void THGem::InitLocalHead(int Stream, int Function, int WaitBit)
{
}
//---------------------------------------------------------------------------
void THGem::DataItemOut(int Len, unsigned char Type, void *Value)
{
}
//---------------------------------------------------------------------------
void THGem::SendLocalData()
{
}
//---------------------------------------------------------------------------
int THGem::DataItemIn(int Len, unsigned char Type, void *Value)
{
    return -1;
}
//---------------------------------------------------------------------------
int THGem::GetDataItemLenAndType(int &Len, unsigned char &Type)
{
    Len = 0;
    Type = HType.LIST_TYPE;
    return 0;
}
//---------------------------------------------------------------------------
int THGem::GetDataItemLenAndTypeAndDelete(int &Len, unsigned char Type)
{
    Len = 0;
    return 0;
}
//---------------------------------------------------------------------------
bool THGem::CheckSFFormatOnlyHead(AnsiString ErrStr)
{
    return true;
}
//---------------------------------------------------------------------------
