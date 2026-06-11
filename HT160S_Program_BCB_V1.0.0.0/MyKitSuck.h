//---------------------------------------------------------------------------
#ifndef MyKitSuckH
#define MyKitSuckH
//---------------------------------------------------------------------------
#include <vcl.h>
#include "HTimer.h"
#include "myswitch.h"
#include "mysensor.h"
//---------------------------------------------------------------------------
#define MAX_SUCKER_ROW 1
#define MAX_SUCKER_COL 4
#define MAX_SUB_SUCKER_ITEM (MAX_SUCKER_ROW*MAX_SUCKER_COL)
#define MAXSUCK MAX_SUB_SUCKER_ITEM
//---------------------------------------------------------------------------
enum eSuckError{eSuckPickErr     =0,
                eSuckDestroyErr  =1,
                eSuckVacOffErr   =2,
                eSuckDropErr     =3,
                eSuckIniOffErr   =4,
                eSuckIniOnErr    =5,
                eSuckErrTotal
               };
//---------------------------------------------------------------------------
class TMyLed;
//---------------------------------------------------------------------------
// Lightweight per-sucker IC carrier stub. Production trace logging is handled
// centrally by TDeviceInfo (g_DeviceInfo) in deviceinfo.h.
class TMySuckerDeviceInfo
{
public:
    void Clear() {}
    void CopyFrom(TMySuckerDeviceInfo &Source) {}
    void MoveFrom(TMySuckerDeviceInfo &Source) {}
};
//---------------------------------------------------------------------------
class TMySucker
{
private:
    int Task;

public:
    __fastcall TMySucker();

    TMySensor Sensor;
    TMySwitch OnSw;
    TMySwitch OffSw;

    DWORD VacuumOnTime;
    DWORD VacuumOffTime;
    DWORD VacuumOnTimeBuffer[20];
    DWORD VacuumOffTimeBuffer[20];

    HTimer Delay;
    AnsiString SensorName;
    AnsiString OnPortName;
    AnsiString OffPortName;
    AnsiString SuckerName;
    AnsiString Alias;
    int iMyRow;
    int iMyCol;
    int Tag;
    bool Error;
    bool Enable;
    bool EnableAtDataBase;
    bool Status;
    AnsiString OnAlarmCode;
    AnsiString OffAlarmCode;
    int OnAlarmTime;
    int OffAlarmTime;
    int OnDelayTime;
    int OffDelayTime;
    int iManualOffTask;
    AnsiString TempString;
    bool RealTimeRefreshVacuumOnOffTime;
    TMySuckerDeviceInfo DeviceInfo;
    int Item;
    TMyLed *pLed;

    bool Suck();
    bool Destroy();
    void On();
    void Off();
    void OnSuck();
    void OffSuck();
    void OnDestroy();
    void OffDestroy();
    void Normal();
    void Reset();
    bool GetStatus();
    bool GetOnBit();
    bool GetOffBit();
    void ResetSuckTask();
    void SetRetryCount(int Count);
    void CheckIsFallDown();
    void PushOnTime();
    void PushOffTime();
};
//---------------------------------------------------------------------------
class TMyKitSuck
{
public:
    __fastcall TMyKitSuck();

    TMySucker Suck[MAX_SUCKER_ROW][MAX_SUCKER_COL];
    AnsiString Name;
    AnsiString FlushPanelName;
    AnsiString AlarmName[eSuckErrTotal];
    int MaxItem;
    int MaxItemR;
    int MaxItemC;
    int iWhichAuto[16];
    int iBinData[16];
    int Tag;
    char cBinInfor[16][16];
    int TrayData;
    bool SuckIC;
    bool Has_SuckIC;
    AnsiString sSuck_2D;
    AnsiString sRule;
    AnsiString sBin;

    void SetMyLed(int RowIndex, int ColIndex, TMyLed *ledPtr);
    void SetItemData(int RowIndex, int ColIndex, int Data);
    void SetItemAmount(int Count);
    void SetItemAmount(int RowCount, int ColCount);
    void initMyKitSuck(AnsiString sName, AnsiString sFlushPanel, int RowCount, int ColCount);
    bool NoIC();
    bool HasIC();
    bool HasType(int Data);
    int GetCountOfDeiveType(int Data);
    bool AllIs_HAS_NULL_IC();
    bool HasRealIC();
    void ClearSingle(int RowIndex, int ColIndex);
    void ClearAll();
    void SetAll(int Type);
    bool HasTestedDevice();
    bool HasDeviceNotTested();
    void MoveSingleItem(TMyKitSuck &Source, int RowIndex, int ColIndex);
    void CopySingleItem(TMyKitSuck &Source, int RowIndex, int ColIndex);
    void MoveAllItem(TMyKitSuck &Source);
    void CopyFrom(TMyKitSuck &Source);
    void MoveSuckData(TMyKitSuck &Source, int RowIndex, int ColIndex);
    void MoveSuckDataDiff(TMyKitSuck &Source, int SourceRowIndex, int SourceColIndex, int TargetRowIndex, int TargetColIndex);
    bool AllDeviceTested();
    void ResetAll();
    void ClearAllError();
    void CheckVaccumIsIniaialON(int RowIndex, int ColIndex, bool &Flag);
    bool FullIC();
    void ClearBinData();
};
//---------------------------------------------------------------------------
#endif
