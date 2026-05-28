//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "uHGemHT160.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
struct ETypeStruct SECS_EVENT;
char GEM_MachineName[16] = "HT160S";
//---------------------------------------------------------------------------
HT160Gem::HT160Gem(AnsiString Path, THGem *HGemTmp)
    : HTGem(Path, HGemTmp)
{
    iControlState = 0;
    sSystemTime = Now().FormatString("yyyy/mm/dd hh:nn:ss");

    EventDescription[SECS_EVENT.HandlerStatus       ] = "1 Handler change status";
    EventDescription[SECS_EVENT.RecipeChange        ] = "2 Recipe Change";
    EventDescription[SECS_EVENT.ClearCount          ] = "3 Press Clear Count button";
    EventDescription[SECS_EVENT.PressStartWithoutIC ] = "4 Press Start button without IC inside handler";
    EventDescription[SECS_EVENT.PressStartWithIC    ] = "5 Press Start button with IC inside handler";
    EventDescription[SECS_EVENT.PressPause          ] = "6 Press Pause button";
    EventDescription[SECS_EVENT.PressHome           ] = "7 Press Home button";
    EventDescription[SECS_EVENT.PressOneCycle       ] = "8 Press One Cycle button";
    EventDescription[SECS_EVENT.PressCleanOut       ] = "9 Press Clean Out button";
    EventDescription[SECS_EVENT.PressTrayFeed       ] = "10 Press Tray Feed button";
    EventDescription[SECS_EVENT.PressLotStart       ] = "11 Press Lot Start button";
    EventDescription[SECS_EVENT.PressLotEnd         ] = "12 Press Lot End button";
    EventDescription[SECS_EVENT.PressExit           ] = "13 Press Exit button";
    EventDescription[SECS_EVENT.PressRetry          ] = "14 Press Retry button";
    EventDescription[SECS_EVENT.PressSkip           ] = "15 Press Skip button";
    EventDescription[SECS_EVENT.PressAlarmReset     ] = "16 Press Alarm Reset button";
    EventDescription[SECS_EVENT.ShowAlarm           ] = "17 Show Alarm";
    EventDescription[SECS_EVENT.ReleaseAlarm        ] = "18 Release Alarm";
    EventDescription[SECS_EVENT.ShowMessage         ] = "19 Show Message";
    EventDescription[SECS_EVENT.ReleaseMessage      ] = "20 Release Message";
    EventDescription[SECS_EVENT.ChangeUser          ] = "21 Switching User Level";
    EventDescription[SECS_EVENT.EnterSetup          ] = "22 Enter Setup Page";
    EventDescription[SECS_EVENT.EnterMaintenPage    ] = "23 Enter Maintenance Page";
    EventDescription[SECS_EVENT.EnterIOPage         ] = "24 Enter I/O Page";
    EventDescription[SECS_EVENT.EnterTeach          ] = "25 Enter Teach Page";
    EventDescription[SECS_EVENT.EnterSECSPage       ] = "26 Enter SECS GEM Page";
    EventDescription[SECS_EVENT.OneCycleOK          ] = "27 One Cycle Finish";
    EventDescription[SECS_EVENT.CleanOutOK          ] = "28 Clean Out Finish";
    EventDescription[SECS_EVENT.TrayFeedOK          ] = "29 Tray Feed Finish";
    EventDescription[SECS_EVENT.TimeEvent           ] = "30 Time Event";
    EventDescription[SECS_EVENT.RealDummy           ] = "31 Switching Real/Dummy Mode";
}
//---------------------------------------------------------------------------
HT160Gem::~HT160Gem()
{
}
//---------------------------------------------------------------------------
void HT160Gem::AddSV()
{
    sSystemTime = Now().FormatString("yyyy/mm/dd hh:nn:ss");
    if(HGemPtr==NULL)
        return;

    HGemPtr->SetSVDataPointer(1000, HType.ASCII_TYPE, "Machine Model", "", &HandlerPath, "HT160S handler model name");
    HGemPtr->SetSVDataPointer(1001, HType.ASCII_TYPE, "System Time", "", &sSystemTime, "HT160S current system time");
    HGemPtr->SetSVDataPointer(1002, HType.INT_4_TYPE, "Control State", "", &iControlState, "HT160S SECS control state placeholder");
}
//---------------------------------------------------------------------------
void HT160Gem::AddEC()
{
}
//---------------------------------------------------------------------------
void HT160Gem::AddAlarmList()
{
}
//---------------------------------------------------------------------------
void HT160Gem::AddCEID()
{
    int EquDefault = 1;
    unsigned ReportID[1];
    ReportID[0] = 1;

    if(HGemPtr==NULL)
        return;

    for(int i=SECS_EVENT.HandlerStatus; i<SECS_EVENT.TotalEvent; i++)
    {
        HGemPtr->SetCEIDContent(i, EventDescription[i], 1, ReportID, EquDefault);
    }
}
//---------------------------------------------------------------------------
void HT160Gem::AddReprot()
{
    int EquDefault = 1;
    unsigned ReportIDContent[3];

    if(HGemPtr==NULL)
        return;

    ReportIDContent[0] = 1000;
    ReportIDContent[1] = 1001;
    ReportIDContent[2] = 1002;
    HGemPtr->SetReportIDContent(1, 3, ReportIDContent, EquDefault);
    HGemPtr->SaveEventReportData();
}
//---------------------------------------------------------------------------
int HT160Gem::S2F15_CheckNewEquipmentConstant()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S2F15 EC update is disabled for HT160 framework skeleton");
    return 1;
}
//---------------------------------------------------------------------------
int HT160Gem::S2F15_UpdateNewEquipmentConstant()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S2F15 EC update is disabled for HT160 framework skeleton");
    return 1;
}
//---------------------------------------------------------------------------
int HT160Gem::S2F42_Host_Command_Acknowledge()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S2F42 remote command is disabled for HT160 framework skeleton");
    return 1;
}
//---------------------------------------------------------------------------
void HT160Gem::S5F6_ListAlarmData()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S5F6 alarm list is empty in HT160 framework skeleton");
}
//---------------------------------------------------------------------------
int HT160Gem::S7F2_ProcessProgramLoadGrant()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S7F2 recipe load is disabled for HT160 framework skeleton");
    return 1;
}
//---------------------------------------------------------------------------
void HT160Gem::S7F4_ProcessProgramAcknowledge()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S7F4 recipe acknowledge is disabled for HT160 framework skeleton");
}
//---------------------------------------------------------------------------
void HT160Gem::S7F6_ProcessProgramData()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S7F6 recipe upload is disabled for HT160 framework skeleton");
}
//---------------------------------------------------------------------------
void HT160Gem::S7F6_ProcessProgramData(AnsiString FileName)
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S7F6 recipe upload is disabled for HT160 framework skeleton: " + FileName);
}
//---------------------------------------------------------------------------
void HT160Gem::ProcessS14F1_GetAttrRequest(AnsiString asTrayID)
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S14F1 tray attribute request is disabled for HT160 framework skeleton: " + asTrayID);
}
//---------------------------------------------------------------------------
unsigned char HT160Gem::ProcessS14F2_GetAttrData()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S14F2 tray attribute data is disabled for HT160 framework skeleton");
    return 1;
}
//---------------------------------------------------------------------------
void HT160Gem::ReloadParameter()
{
}
//---------------------------------------------------------------------------
void HT160Gem::LookForFile()
{
}
//---------------------------------------------------------------------------
