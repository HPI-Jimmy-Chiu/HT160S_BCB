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
    //AI(ht160s-secsgem) 20260611 : SV snapshot members refreshed just before each
    // S6F11 / S1F4 serialize, so SetSVDataPointer can bind a stable address while
    // the value still tracks live machine data (avoids binding bool/enum/form ptr).
    int        svRunMode;        // (int)HSys.Sys.RunMode 0Normal/1Home/2OneCycle/3CleanOut/4TrayFeed
    int        svSystemRunning;  // HSys.Sys.SystemStart 0/1
    int        svAlarmActive;    // fNote->fShow 0/1
    int        svAlarmCode;      // fNote->Code
    int        svTotalIC;        // tRunData.TotalIC
    int        svTotalSorted;    // MachineRun.iTotalSorted
    int        svUPH;            // tRunData.UPH
    int        svLotCount;       // LotRegistry.GetLotCount()
    AnsiString svCurrentLot;     // first registered Lot ID ("" if none)
    AnsiString svSoftwareVersion;// application software version (constant, 9045 SVID 1003)
    //AI(ht160s-secsgem) 20260611 : EC snapshot member. Recipe name has no single
    // stable address (lives behind RecipeManager getter/normalizer), so S2F14
    // refreshes this before serialize. Tray-form ECs bind THT160TrayForm directly.
    AnsiString ecRecipeName;     // RecipeManager.GetCurrentRecipeName()
protected:
    AnsiString EventDescription[SECS_EVENT.TotalEvent];
public:
    HT160Gem(AnsiString Path, THGem *HGemTmp);
    virtual ~HT160Gem();

    virtual void RefreshSVData();
    virtual void RefreshSecsBadge();   //AI(ht160s-secsgem) 20260612 : 1s tick -> sync main-screen SECS badge to HSMS state
    virtual void AddSV();
    virtual void AddEC();
    virtual void AddAlarmList();
    virtual void AddCEID();
    virtual void AddReprot();
    virtual void S1F4_SelectedStatusReply();
    virtual void S1F12_StatusVariableNamelistReply();//AI(ht160s-secsgem) 20260611 : S1F11->S1F12 SV namelist
    //AI(ht160s-secsgem) 20260611 : GUI EC editor - same idle/range guard as S2F16.
    //  0=ok, 1=ECID not host-settable, 2=busy, 3=range/convert error.
    int GuiWriteTrayEC(unsigned ECID, AnsiString sValue);
    virtual int  S2F15_CheckNewEquipmentConstant();
    virtual int  S2F15_UpdateNewEquipmentConstant();
    virtual void S2F16_NewEquipmentConstantSendAcknowledge();//AI(ht160s-secsgem) 20260611 : S2F15->S2F16 EC write
    virtual void S2F14_EquipmentConstanData();
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
