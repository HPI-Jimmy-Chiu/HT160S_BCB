//---------------------------------------------------------------------------
#ifndef uHGemHT160H
#define uHGemHT160H
//---------------------------------------------------------------------------
#include "uHGemClass.h"
//---------------------------------------------------------------------------
//AI(secs-ceid-align9045) 20260729 : CEID dictionary is now a VERBATIM copy of HT9045.
//  Source of truth : D:/HT9045/HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP/SECSGEM/
//                    uHGemHT9045.h (enum) + uHGemHT9045.cpp (EventDescription).
//  All 275 ids are registered so the host dictionary matches HT9045 one-for-one; ids whose
//  mechanism does not exist on HT160S (tester / ART / site / temperature / E87 cassette /
//  OHT / Fix station / energy saving / Reserved) are registered but never emitted.
//  142/143/144 carry no alias in HT9045 either - kept unnamed on purpose.
//  Numbering is FIXED by HT9045. Do not renumber, do not insert, do not reuse a slot.
struct ETypeStruct
{
    enum
    {
        DoStart = 1                 ,   //   1 Start Pressed
        DoPause                     ,   //   2 Pause Pressed
        DoOneCycle                  ,   //   3 OneCycle Pressed
        DoCleanOut                  ,   //   4 CleanOut Pressed
        DoClearCount                ,   //   5 ClearCount Pressed
        DoLotStart                  ,   //   6 Lot Start
        DoLot                       ,   //   7 Lot
        DoLotEnd                    ,   //   8 Lot End
        SwitchRunMode               ,   //   9 Switch Real Dummy Mode
        SwitchTesterMode            ,   //  10 Switch Tester Online
        SwitchProduction            ,   //  11 Switch Production Mode
        SwitchEngineer              ,   //  12 Switch Engineer Mode
        SwitchTemperature           ,   //  13 Switch Temperature Mode
        SwitchStartMode             ,   //  14 Switch StartMode
        SwitchSetupFile             ,   //  15 Switch Setup File
        SwitchUser                  ,   //  16 Switch UserLevel
        EnterTool                   ,   //  17 Enter Tool Page
        EnterConfig                 ,   //  18 Enter Maintenance Page
        EnterOffset                 ,   //  19 Enter Offset Page
        EnterSpeed                  ,   //  20 Enter Speed Page
        EnterIO                     ,   //  21 Enter IO Page
        EnterMessage                ,   //  22 Enter Message Page
        EnterDebug                  ,   //  23 Enter Debug Page
        DoExit                      ,   //  24 Exit Pressed
        DoHome                      ,   //  25 Home Pressed
        GetTestResult               ,   //  26 Get Test Result
        RunStatus                   ,   //  27 Change Machine State
        DoRetry                     ,   //  28 Retry Pressed
        DoSkip                      ,   //  29 Skip Pressed
        DoAlarmReset                ,   //  30 Alarm Reset Pressed
        DoTrayEnd                   ,   //  31 Tray End Pressed
        DoTrayFeed                  ,   //  32 Tray Feed Pressed
        DoReset                     ,   //  33 Reset Pressed
        DoAutoClean                 ,   //  34 Auto Clean Start
        Auto1Full                   ,   //  35 Auto1 Full
        Auto2Full                   ,   //  36 Auto2 Full
        Auto3Full                   ,   //  37 Auto3 Full
        Fix1Full                    ,   //  38 Fix1 Full
        Fix2Full                    ,   //  39 Fix2 Full
        Fix3Full                    ,   //  40 Fix3 Full
        OneCycleFinish              ,   //  41 One Cycle Finish
        CleanOutFinish              ,   //  42 Clean Out Finish
        DownloadRecipe              ,   //  43 DownLoadRecipe
        SiteOnOff                   ,   //  44 Site On Off
        ArmOnOff                    ,   //  45 Arm On Off
        SwitchTempData              ,   //  46 Change Temp Defaultand Soak Time
        SwitchSpeed                 ,   //  47 Change HandlerSpeed
        ChangeEC                    ,   //  48 Change EC
        TrayFeedFinish              ,   //  49 Tray Feed Finish
        AutoCleanFinish             ,   //  50 Auto Clean Finish
        SiteMappingStart            ,   //  51 Site Mapping Start
        SiteMappingEnd              ,   //  52 Site Mapping End
        UPHRecordStart              ,   //  53 UPH Record Start
        UPHRecordEnd                ,   //  54 UPH Record End
        InitialArtStart             ,   //  55 Initial ART Start
        TesterFT                    ,   //  56 Change Tester Program to FT
        TesterRT                    ,   //  57 Change Tester Program to RT
        ReadyForArt                 ,   //  58 Ready for ART
        ArtReceiveTrayOK            ,   //  59 ART Receive Tray OK
        ArtReceiveTraySTART         ,   //  60 ART Receive Tray START
        ArtRTFinish                 ,   //  61 RT Finish
        ArtTrayFeedFinish           ,   //  62 ART Finish
        ArtFTFinish                 ,   //  63 FT Finish
        DownLoadRecipeByFTPOK       ,   //  64 DownLoad Recipe by FTP OK
        DownLoadRecipeByFTPNG       ,   //  65 DownLoad Recipe by FTP NG
        LoadTrayFinish              ,   //  66 Load Tray Finish
        TrayTestFinish              ,   //  67 Tray Test Finish
        AutoCleanClearCount         ,   //  68 Auto Clean Clear Count
        SiteMappingStop             ,   //  69 Site Mapping Stop
        BarcodeReaderEnter          ,   //  70 Barcode Reader Enter
        OTDLock                     ,   //  71 OTD Lock
        OTDUnLock                   ,   //  72 OTD UnLock
        MymessboxOK                 ,   //  73 Mymessbox OK
        RemoteProgramClose          ,   //  74 74
        ChangeTesterPrgToEQC        ,   //  75 75
        DoStartHasIC                ,   //  76 Start Pressed HasIC
        ReadCurrentESDData          ,   //  77 Read Current ESD Data
        JamSkipICCount              ,   //  78 Jam Skip IC Count
        REVERSED79                  ,   //  79 REVERSED79  (no alias in HT9045)
        ReadNowHandlerData          ,   //  80 Read Now Handler Data
        ReadATCTemperature          ,   //  81 Read ATC Temperature
        ReadATCRefTemperature       ,   //  82 Read ATC Ref Temperature
        ReadNowEPPenconder          ,   //  83 Read Now EP Penconder
        RunStatus_FT                ,   //  84 Run Status FT
        RunStatus_RT                ,   //  85 Run Status RT
        MapNoArmHasIC               ,   //  86 Map No Device Arm Has Device
        MapHasICArmRetry            ,   //  87 Map Has Device Arm Error Retry
        MapHasICArmSkip             ,   //  88 Map Has Device Arm Error Skip
        PreAlarmMessage             ,   //  89 Pre Alarm Message
        GetTestResultAndBarcode     ,   //  90 Get TestResult And Barcode
        SECSOffline                 ,   //  91 SECS/GEM Offline
        SECSOnline                  ,   //  92 SECS/GEM Online
        SECSOnlineRemote            ,   //  93 SECS/GEM Online Remote
        TransferBlocked             ,   //  94 Transfer Blocked
        CassetteLoadComplete        ,   //  95 Cassette Load Complete
        CassetteIDReadComplete      ,   //  96 Cassette ID Read Complete
        ReadyToProcessComplete      ,   //  97 Ready To Process Complete
        ReadyToCarrierOutLot        ,   //  98 Ready To Carrier Out Lot
        CassetteOutComplete         ,   //  99 Cassette Out Complete
        CassetteUnclamped           ,   // 100 Cassette Unclamped
        ReadyToUnload               ,   // 101 Ready To Unload
        UnloadComplete              ,   // 102 Unload Complete
        ReadyToCarrierOutTray       ,   // 103 Ready To Carrier Out Tray
        ReadyToCombinePass          ,   // 104 Ready To Combine Pass
        ReadyToCombineFail          ,   // 105 Ready To Combine Fail
        MachineNoStart              ,   // 106 Machine No Start
        ReadyToCombinePassLotEnd    ,   // 107 Ready To Combine Pass Lot End
        DoCSTLotStart               ,   // 108 Cassette Lot Start
        DieCountFailMessageClose    ,   // 109 Die Count Fail Message Close
        CleanOutTrayFeedFinish      ,   // 110 Clean Out Tray Feed Finish
        MapNoICArmAutoSkip          ,   // 111 Map No Device Arm Auto Skip
        MRRunModeChange             ,   // 112 MR Run Mode Change
        AccessModeChange            ,   // 113 Access Mode Change
        SoftwareBin                 ,   // 114 Software Bin
        TrayIDChange                ,   // 115 Tray ID Change
        ReadyToLoadNoLot            ,   // 116 Ready To Load No Lot
        ReadyToLoadNoTray           ,   // 117 Ready To Load No Tray
        ReadyToLoadNoCassette       ,   // 118 Ready To Load No Cassette
        ART_SRQKIND2_FTLOTSTART     ,   // 119 ART SRQKIND2 FT LOTSTART
        ART_SRQKIND4_RTLOTSTART     ,   // 120 ART SRQKIND4 RT LOTSTART
        ART_SRQKIND8_LOTEND         ,   // 121 ART SRQKIND6 LOTEND
        ART_SRQKIND10_FINALLOTEND   ,   // 122 FINAL LOTEND
        SafeDoorOnOff               ,   // 123 Safe Door On Off
        SaveRecipe                  ,   // 124 Save Recipe
        EESUGOffestSelect           ,   // 125 EESUG Offest Select
        EESUGOffestModify           ,   // 126 EESUG Offest Modify
        Backtonormal                ,   // 127 Back To Normal
        TestStart                   ,   // 128 Test Start
        TestFinish                  ,   // 129 Test Finish
        MaterialReceive             ,   // 130 Material Receive
        SlotMapCountOK              ,   // 131 Slot Map Count OK
        CHECK_IN                    ,   // 132 CHECK IN
        CHECK_OUT                   ,   // 133 CHECK OUT
        ReadyToCombineFailLotEnd    ,   // 134 Ready To Combine Fail Lot End
        ReadyToOHTLotEnd            ,   // 135 Ready To OHT Lot End
        Auto1Unloadtray             ,   // 136 Auto 1 Unloading tray
        Auto2Unloadtray             ,   // 137 Auto 2 Unloading tray
        Auto3Unloadtray             ,   // 138 Auto 3 Unloading tray
        DoVisualSortLotStart        ,   // 139 Click lot start button for visual sort mode
        PreLoadTray                 ,   // 140 Prepare Load Tray
        GemControlStateChange       ,   // 141 GEM Control State Change
        PickerCountWasCleared       ,   // 142 PickerCountWasCleared  (no alias in HT9045)
        UploadPickerCount           ,   // 143 UploadPickerCount  (no alias in HT9045)
        RequestPickerCount          ,   // 144 RequestPickerCount  (no alias in HT9045)
        Auto4Unloadtray             ,   // 145 Auto 4 Unloading tray
        Auto5Unloadtray             ,   // 146 Auto 5 Unloading tray
        Auto6Unloadtray             ,   // 147 Auto 6 Unloading tray
        Auto4Full                   ,   // 148 Auto 4 Full
        Auto5Full                   ,   // 149 Auto 5 Full
        Auto6Full                   ,   // 150 Auto 6 Full
        Fix4Full                    ,   // 151 Fix 4 Full
        Fix5Full                    ,   // 152 Fix 5 Full
        Fix6Full                    ,   // 153 Fix 6 Full
        LoadNoTray                  ,   // 154 Loader hasn't tray
        LoadFullTray                ,   // 155 Loader full of tray
        LoadOnlyOneTray             ,   // 156 Loader only one tray
        Loader_ReadyToUnload        ,   // 157 Loader ready to unload
        Loader_FinishUnload         ,   // 158 Loader finish unload
        Empty_PreLoadTray           ,   // 159 Empty Prepare Load Tray
        EmptyOnlyOneTray            ,   // 160 Empty only one tray
        EmptyNoTray                 ,   // 161 Empty hasn't tray
        EmptyFullTray               ,   // 162 Empty full of tray
        Color_PreLoadTray           ,   // 163 Color Prepare Load Tray
        ColorOnlyOneTray            ,   // 164 Color only one tray
        ColorNoTray                 ,   // 165 Color hasn't tray
        Empty_PutTrayToAuto1        ,   // 166 Empty put tray to Auto1
        Empty_PutTrayToAuto2        ,   // 167 Empty put tray to Auto2
        Empty_PutTrayToAuto3        ,   // 168 Empty put tray to Auto3
        Empty_PutTrayToAuto4        ,   // 169 Empty put tray to Auto4
        Empty_PutTrayToAuto5        ,   // 170 Empty put tray to Auto5
        Empty_PutTrayToAuto6        ,   // 171 Empty put tray to Auto6
        Empty_PutCoverToAuto1       ,   // 172 Empty put cover to Auto1
        Empty_PutCoverToAuto2       ,   // 173 Empty put cover to Auto2
        Empty_PutCoverToAuto3       ,   // 174 Empty put cover to Auto3
        Empty_PutCoverToAuto4       ,   // 175 Empty put cover to Auto4
        Empty_PutCoverToAuto5       ,   // 176 Empty put cover to Auto5
        Empty_PutCoverToAuto6       ,   // 177 Empty put cover to Auto6
        Color_PutTrayToAuto1        ,   // 178 Color put tray to Auto1
        Color_PutTrayToAuto2        ,   // 179 Color put tray to Auto2
        Color_PutTrayToAuto3        ,   // 180 Color put tray to Auto3
        Color_PutTrayToAuto4        ,   // 181 Color put tray to Auto4
        Color_PutTrayToAuto5        ,   // 182 Color put tray to Auto5
        Color_PutTrayToAuto6        ,   // 183 Color put tray to Auto6
        Color_PutCoverToAuto1       ,   // 184 Color put cover to Auto1
        Color_PutCoverToAuto2       ,   // 185 Color put cover to Auto2
        Color_PutCoverToAuto3       ,   // 186 Color put cover to Auto3
        Color_PutCoverToAuto4       ,   // 187 Color put cover to Auto4
        Color_PutCoverToAuto5       ,   // 188 Color put cover to Auto5
        Color_PutCoverToAuto6       ,   // 189 Color put cover to Auto6
        Auto1_LoadTrayFinish        ,   // 190 Auto1 load tray finish
        Auto2_LoadTrayFinish        ,   // 191 Auto2 load tray finish
        Auto3_LoadTrayFinish        ,   // 192 Auto3 load tray finish
        Auto4_LoadTrayFinish        ,   // 193 Auto4 load tray finish
        Auto5_LoadTrayFinish        ,   // 194 Auto5 load tray finish
        Auto6_LoadTrayFinish        ,   // 195 Auto6 load tray finish
        Auto1_ReadyToUnload         ,   // 196 Auto1 ready to unload
        Auto2_ReadyToUnload         ,   // 197 Auto2 ready to unload
        Auto3_ReadyToUnload         ,   // 198 Auto3 ready to unload
        Auto4_ReadyToUnload         ,   // 199 Auto4 ready to unload
        Auto5_ReadyToUnload         ,   // 200 Auto5 ready to unload
        Auto6_ReadyToUnload         ,   // 201 Auto6 ready to unload
        Auto1NoTray                 ,   // 202 Auto1 hasn't tray
        Auto2NoTray                 ,   // 203 Auto2 hasn't tray
        Auto3NoTray                 ,   // 204 Auto3 hasn't tray
        Auto4NoTray                 ,   // 205 Auto4 hasn't tray
        Auto5NoTray                 ,   // 206 Auto5 hasn't tray
        Auto6NoTray                 ,   // 207 Auto6 hasn't tray
        ColorFullTray               ,   // 208 Color full of tray
        TrayEndFinish               ,   // 209 Tray End Finish
        Empty_FinishUnload          ,   // 210 Empty finish unload
        Color_FinishUnload          ,   // 211 Color finish unload
        PowerSavingStart            ,   // 212 Energy Saving Start
        PowerSavingEnd              ,   // 213 Energy Saving End
        Reserved_03                 ,   // 214 Reserved_03
        Reserved_04                 ,   // 215 Reserved_04
        Reserved_05                 ,   // 216 Reserved_05
        Reserved_06                 ,   // 217 Reserved_06
        Reserved_07                 ,   // 218 Reserved_07
        Reserved_08                 ,   // 219 Reserved_08
        Reserved_09                 ,   // 220 Reserved_09
        Reserved_10                 ,   // 221 Reserved_10
        Reserved_11                 ,   // 222 Reserved_11
        Reserved_12                 ,   // 223 Reserved_12
        Reserved_13                 ,   // 224 Reserved_13
        Reserved_14                 ,   // 225 Reserved_14
        Reserved_15                 ,   // 226 Reserved_15
        Reserved_16                 ,   // 227 Reserved_16
        Reserved_17                 ,   // 228 Reserved_17
        Reserved_18                 ,   // 229 Reserved_18
        Reserved_19                 ,   // 230 Reserved_19
        Reserved_20                 ,   // 231 Reserved_20
        Reserved_21                 ,   // 232 Reserved_21
        Reserved_22                 ,   // 233 Reserved_22
        Reserved_23                 ,   // 234 Reserved_23
        Reserved_24                 ,   // 235 Reserved_24
        Reserved_25                 ,   // 236 Reserved_25
        Reserved_26                 ,   // 237 Reserved_26
        Reserved_27                 ,   // 238 Reserved_27
        Reserved_28                 ,   // 239 Reserved_28
        Reserved_29                 ,   // 240 Reserved_29
        Reserved_30                 ,   // 241 Reserved_30
        Reserved_31                 ,   // 242 Reserved_31
        Reserved_32                 ,   // 243 Reserved_32
        Reserved_33                 ,   // 244 Reserved_33
        Reserved_34                 ,   // 245 Reserved_34
        Reserved_35                 ,   // 246 Reserved_35
        Reserved_36                 ,   // 247 Reserved_36
        Reserved_37                 ,   // 248 Reserved_37
        Reserved_38                 ,   // 249 Reserved_38
        DoStartAutoHeight           ,   // 250 START Auto contact height
        DoSecsGemIndexFail          ,   // 251 SECS GEM consecutive failure
        Reserved_39                 ,   // 252 Reserved_39
        Reserved_40                 ,   // 253 Reserved_40
        Reserved_41                 ,   // 254 Reserved_41
        Reserved_42                 ,   // 255 Reserved_42
        Reserved_43                 ,   // 256 Reserved_43
        Reserved_44                 ,   // 257 Reserved_44
        Reserved_45                 ,   // 258 Reserved_45
        Reserved_46                 ,   // 259 Reserved_46
        Reserved_47                 ,   // 260 Reserved_47
        Reserved_48                 ,   // 261 Reserved_48
        Reserved_49                 ,   // 262 Reserved_49
        Reserved_50                 ,   // 263 Reserved_50
        Reserved_51                 ,   // 264 Reserved_51
        Reserved_52                 ,   // 265 Reserved_52
        Reserved_53                 ,   // 266 Reserved_53
        Reserved_54                 ,   // 267 Reserved_54
        Reserved_55                 ,   // 268 Reserved_55
        Reserved_56                 ,   // 269 Reserved_56
        Reserved_57                 ,   // 270 Reserved_57
        Reserved_58                 ,   // 271 Reserved_58
        AGVSupplement               ,   // 272 AMR Supplement
        AGVLDUnLDStatus             ,   // 273 AMR LDUnLD Status
        AGVLDUnLDFinish             ,   // 274 AMR LDUnLD Finish
        AGVLdID                     ,   // 275 AMR LD ID
        TotalEvent
    };
};
extern struct ETypeStruct SECS_EVENT;
//---------------------------------------------------------------------------
class HT160Gem : public HTGem
{
private:
    AnsiString sSystemTime;
    //AI(secs-bclass-0803) 20260803 : SVID 3 GemClock. SEPARATE member from sSystemTime on
    // purpose - 3 is the GEM-standard clock and HT9045 puts a 16-char SEMI E5 TIME
    // "YYYYMMDDhhmmsscc" on the wire (uHGemEquipment.cpp:325 with iTimeFormat=1), which the
    // KYEC host has read in RPTID 502 slot 4 all day. Aliasing sSystemTime here (as
    // docs/plan/secs-9045-porting-20260729/svid-ownership.md:103 proposed) would push a
    // 19-char "yyyy/mm/dd hh:nn:ss" into a slot the host parses as E5 TIME.
    AnsiString sGemClock;
    int iControlState;
    //AI(secs-controlstate) 20260803 : HT9045-numbered view of the same control state, published
    // as SVID 4 / 9. unsigned char + U1 on the wire because that is exactly what HT9045 does
    // (uHGemEquipment.cpp SetSVDataPointer(4, UINT_1_TYPE, ...) over an unsigned char member),
    // and the VALUE DOMAIN is 9045's, not GEM's : 1=Off-Line, 2=On-Line Local, 3=On-Line Remote.
    // iControlState above keeps the GEM-standard 1/4/5 domain it has always published on 66002 -
    // that number is already in the customer spec, so it is not re-encoded here.
    unsigned char svGemControlState;      // SVID 4 GemControlState
    unsigned char svGemControlPreState;   // SVID 9 PreviousGemControlState
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
    //AI(secs-onsite0731) 20260801 : the seven-of-eight empty slots of the KYEC host's
    // RPTID 502 were the operator's on-site notes 3 and 4. These three close the ones whose
    // data HT160S already owns. svActiveLot is SEPARATE from svCurrentLot on purpose:
    // svCurrentLot rides firmware report 1, whose 13-SV shape is published in the customer
    // interface spec and means "first registered lot"; 1006 must be the ACTIVE lot.
    AnsiString svActiveLot;      // TfMain::ActiveLotID()      -> SVID 1006 Lot ID
    AnsiString svMachineState;   // g_sMachineStateText mirror -> SVID 1011 Machine State
    int        svLoaderIC;       // tRunData.LoaderIC          -> SVID 1101 Loader Count
    //AI(secs-startmode) 20260802 : SVID 1517 Start Mode, in HT9045's NUMBERING, not ours.
    // HSys.LastSet.iStartMode is 0=Initial / 1=Continue (clamped 0..1 at main.cpp:84-85);
    // HT9045's eRunStartMode is 0=Continuous / 1=Initial. The two are exactly INVERTED, so
    // this member holds the translated value and the raw flag is never published directly.
    int        svStartMode;      // 9045-numbered start mode   -> SVID 1517 Start Mode
    //AI(secs-skipiccount) 20260802 : SVID 37010 "Enter Skip IC Count". NOT a live counter -
    // it is LATCHED to the number the operator typed at the last SKIP, and CEID 78 is fired
    // immediately after, exactly as HT9045 does (note.cpp:2185-2189, iJamSkipIC = the entered
    // value, then EventReport). Deliberately not refreshed in RefreshSVData: the host must
    // read "how many were removed at that alarm", not a running total.
    int        svSkipICCount;    // operator-entered removal   -> SVID 37010 Enter Skip IC Count
    //AI(secs-lotstarttime) 20260730 : the Lot Start latch. LATCHED by NoteLotStartTime,
    // deliberately NOT refreshed in RefreshSVData - it must answer "when did this lot start",
    // not "now".
    //AI(secs-svid-dedupe) 20260803 : this member no longer has an SVID of its own. It was
    // SVID 66033 until the customer ruled that duplicated information collapses onto the
    // HT9045 number; it now lives on as the INTERNAL latch, in slash format because that is
    // the shape main.cpp persists into the work-order meta file and re-feeds on inherit.
    AnsiString svLotStartTime;   // "yyyy/mm/dd hh:nn:ss" of Lot Start ("" between lots)
    //AI(secs-bclass-0803) 20260803 : SVID 1009 Lot Start Time - the latch above in HT9045's
    // wire format "yyyy-mm-dd hh:nn:ss" (9045 sprintf cprod.cpp:693-694; the KYEC 2026-06-08
    // S6F16 carried A[19] "2026-06-07 12:57:32" in RPTID 508 slot 3). Converted in RefreshSVData
    // rather than in the latch so the persisted slash format is never disturbed.
    AnsiString svLotStartTime9045;// "yyyy-mm-dd hh:nn:ss" of Lot Start ("" between lots)
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
    virtual void ServiceAgv();         //AI(ht160s-agv) 20260615 : 1s tick -> drive E87/AGV coordinator (Phase B/D)
    virtual void PollGemControlState();//AI(secs-controlstate) 20260803 : 1s tick -> SVID 4/9 + CEID 141/91/92/93 on a control-state edge
    virtual void OnCommunicationLost();//AI(secs-kyec-rcmd4-fix) 20260728 : HSMS link lost -> drop the latched PP_SIGNALTOWER/PP_MUSIC panel override
    virtual void NoteLotStartTime(bool bStarted, AnsiString sWhen="");//AI(secs-lotstarttime) 20260730 : latch/clear the Lot Start Time behind SVID 1009 (sWhen = restored stamp, "" = now)
    //AI(secs-skipiccount) 20260802 : latch SVID 37010 then fire CEID 78, in that order -
    // the event exists only to carry that number, so the value must be in place first.
    // Called from the alarm chokepoint in note.cpp when the operator clears with SKIP and
    // General.ini [SECS] AskSkipICCount is on.
    virtual void ReportSkipICCount(int iCount);
    virtual void AddSV();
    virtual void AddEC();
    virtual void AddAlarmList();
    virtual void AddCEID();
    virtual void AddReprot();
    virtual void S1F4_SelectedStatusReply();
    virtual void S1F12_StatusVariableNamelistReply();//AI(ht160s-secsgem) 20260611 : S1F11->S1F12 SV namelist
    //AI(secs-namelist) 20260730 : S1F23->S1F24 CEID namelist / S2F29->S2F30 EC namelist.
    virtual void S1F24_CollectionEventNamelist();
    virtual void S2F30_EquipmentConstantNamelistReply();
    //AI(ht160s-secsgem) 20260625 : S1F1 are-you-there -> S1F2 on-line data
    virtual void S1F2_OnLineData();
    //AI(ht160s-secsgem) 20260625 : S1F13 establish-comm -> S1F14 connect-request ack
    virtual void S1F14_ConnectRequestAcknowledge();
    //AI(secs-online) 20260724 : S1F17 Request ONLINE -> S1F18 ONLACK=0 + control state Online-Remote
    virtual void S1F18_ONLINEAcknowledge();
    //AI(secs-offline) 20260727 : S1F15 Request OFF-LINE -> S1F16 OFLACK=0 + control state Off-Line(1)
    virtual void S1F16_OFFLINEAcknowledge();
    //AI(secs-reportdef) 20260724 : S2F33/F35/F37 -> delegate to THGem processors
    virtual void S2F34_DefineReportAcknowledge();
    virtual void S2F36_LinkEventReportAcknowledge();
    virtual void S2F38_EnableDisableEventReportAcknowledge();
    //AI(ht160s-secsgem) 20260611 : GUI EC editor - same idle/range guard as S2F16.
    //  0=ok, 1=ECID not host-settable, 2=busy, 3=range/convert error.
    int GuiWriteTrayEC(unsigned ECID, AnsiString sValue);
    virtual int  S2F15_CheckNewEquipmentConstant();
    virtual int  S2F15_UpdateNewEquipmentConstant();
    virtual void S2F16_NewEquipmentConstantSendAcknowledge();//AI(ht160s-secsgem) 20260611 : S2F15->S2F16 EC write
    virtual void S2F14_EquipmentConstanData();
    virtual int  S2F42_Host_Command_Acknowledge();
    //AI(ht160s-secsgem) 20260625 : S2F31 date/time set -> S2F32 TIACK (ack-only, no clock write)
    virtual void S2F32_DateAndTimeAcknowledge();
    //AI(secs-gem-std) 20260727 : S2F17 Date/Time Request -> S2F18 TIME ; S2F25 Loopback -> S2F26 echo
    virtual void S2F18_DateandTimeData();
    virtual void S2F26_DiagnosticLoopbackData();
    //AI(ht160s-secsgem) 20260625 : S5F3 enable/disable alarm -> S5F4 ack
    virtual void S5F4_EnableDisableAlarmAcknowledge();
    virtual void S5F6_ListAlarmData();
    //AI(ht160s-secsgem) 20260715 : S5F7 List Enabled Alarm Request -> S5F8 data (all enabled)
    virtual void S5F8_ListEnableAlarmAcknowledge();
    void EmitAlarmCatalog(int Func);   // shared S5F6/S5F8 catalog emitter from mapAlarmCodeList
    //AI(secs-msggap) 20260728 : S6F15 -> S6F16 event report pull ; S6F19 -> S6F20 report pull.
    //Parsing only : the encoders live on THGem because FindCEIDItem/FindReportItem are private.
    virtual void S6F16_EventReportData();
    virtual void S6F20_IndividualReportData();
    virtual int  S7F2_ProcessProgramLoadGrant();
    virtual void S7F4_ProcessProgramAcknowledge();
    virtual void S7F6_ProcessProgramData();
    virtual void S7F6_ProcessProgramData(AnsiString FileName);
    virtual void ProcessS14F1_GetAttrRequest(AnsiString asTrayID);
    virtual unsigned char ProcessS14F2_GetAttrData();
    //AI(secs-msggap) 20260728 : S10F3/S10F5 host terminal text -> ACKC10 ack + log only.
    //NEVER routed to ShowMyMessage : that primitive stops all motors and clears SystemStart.
    virtual void S10F4_TerminalDisplaySingleAcknowledge();
    virtual void S10F6_TerminalDisplayMultiBlockAcknowledge();
    //AI(secs-msggap) 20260728 : S125F1 -> exactly one S125F2 <B ACK>. No EC-enable table on HT160.
    virtual void S125F2_EnableDisableECDataAcknowledge();
    virtual void ReloadParameter();
    virtual void LookForFile();
};
//---------------------------------------------------------------------------
extern char GEM_MachineName[16];
//---------------------------------------------------------------------------
#endif
