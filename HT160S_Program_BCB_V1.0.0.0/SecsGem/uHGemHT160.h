//---------------------------------------------------------------------------
#ifndef uHGemHT160H
#define uHGemHT160H
//---------------------------------------------------------------------------
#include "uHGemClass.h"
//---------------------------------------------------------------------------
struct ETypeStruct
{
    enum
    {
        HandlerStatus       = 1,
        RecipeChange        = 2,
        ClearCount          = 3,
        PressStartWithoutIC = 4,
        PressStartWithIC    = 5,
        PressPause          = 6,
        PressHome           = 7,
        PressOneCycle       = 8,
        PressCleanOut       = 9,
        PressTrayFeed       =10,
        PressLotStart       =11,
        PressLotEnd         =12,
        PressExit           =13,
        PressRetry          =14,
        PressSkip           =15,
        PressAlarmReset     =16,
        ShowAlarm           =17,
        ReleaseAlarm        =18,
        ShowMessage         =19,
        ReleaseMessage      =20,
        ChangeUser          =21,
        EnterSetup          =22,
        EnterMaintenPage    =23,
        EnterIOPage         =24,
        EnterTeach          =25,
        EnterSECSPage       =26,
        OneCycleOK          =27,
        CleanOutOK          =28,
        TrayFeedOK          =29,
        TimeEvent           =30,
        RealDummy           =31,
        TotalEvent
    };
};
extern struct ETypeStruct SECS_EVENT;
//---------------------------------------------------------------------------
class HT160Gem : public HTGem
{
private:
    AnsiString sSystemTime;
    int iControlState;
protected:
    AnsiString EventDescription[SECS_EVENT.TotalEvent];
public:
    HT160Gem(AnsiString Path, THGem *HGemTmp);
    virtual ~HT160Gem();

    virtual void AddSV();
    virtual void AddEC();
    virtual void AddAlarmList();
    virtual void AddCEID();
    virtual void AddReprot();
    virtual int  S2F15_CheckNewEquipmentConstant();
    virtual int  S2F15_UpdateNewEquipmentConstant();
    virtual int  S2F42_Host_Command_Acknowledge();
    virtual void S5F6_ListAlarmData();
    virtual int  S7F2_ProcessProgramLoadGrant();
    virtual void S7F4_ProcessProgramAcknowledge();
    virtual void S7F6_ProcessProgramData();
    virtual void S7F6_ProcessProgramData(AnsiString FileName);
    virtual void ProcessS14F1_GetAttrRequest(AnsiString asTrayID);
    virtual unsigned char ProcessS14F2_GetAttrData();
    virtual void ReloadParameter();
    virtual void LookForFile();
};
//---------------------------------------------------------------------------
extern char GEM_MachineName[16];
//---------------------------------------------------------------------------
#endif
