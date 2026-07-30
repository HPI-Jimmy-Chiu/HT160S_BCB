//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "uHGemHT160.h"
//AI(ht160s-secsgem) 20260610 : includes for SET_LOT_INFO handler
#include "main.h"          // fMain, edLotNo
#include "database.h"      // HSys.Sys.SystemStart / RunMode
#include "csystem.h"       // HasICUnderMachine()
#include "CosFunction.h"   // LotRegistry, HT160_LOT_SOURCE_SECS, HT160_MAX_LOT
//AI(ht160s-secsgem) 20260611 : includes for live SV snapshot sources
#include "cprod.h"         // tRunData (TotalIC/UPH), MachineRun (iTotalSorted)
#include "cSoterOutput.h"
#include "note.h"          // fNote (alarm dialog : fShow / Code)
#include "cmydef.h"        // SoftStop (S2F42 PAUSE host command)
#include "uAgvStation.h"   // AI(ht160s-agv) 20260615 : E87/AGV station table + AgvCoord
#include "uAmrInject.h"   // AI(ht160s-agv) 20260708 : AMR manual-inject alert (HCACK!=0 surfacing)
#include "UsecegemMainFrom.h" // AI(ht160s-secsgem) 20260715 : ComputeAlarmAlid (S5 ALID SSOT)
#include "GeneralSetting.h" // AI(ht160s-whitelist) 20260715 : IsWhiteListSortMode()
#include "maintenance.h"    // AI(ht160s-whitelist) 20260716 : SyncSortModeSelectorFromSetting (SECS SORTMODE UI sync)
#include "cEventLog.h"      // AI(secs-msggap) 20260728 : g_EventLog (S10F3/S10F5 host terminal text)
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

    //AI(ht160s-secsgem) 20260611 : init SV snapshot members (refreshed on demand)
    svRunMode       = 0;
    svSystemRunning = 0;
    svAlarmActive   = 0;
    svAlarmCode     = 0;
    svTotalIC       = 0;
    svTotalSorted   = 0;
    svUPH           = 0;
    svLotCount      = 0;
    svCurrentLot    = "";
    svLotStartTime  = "";            //AI(secs-lotstarttime) 20260730 : latched by NoteLotStartTime, not by RefreshSVData
    svSoftwareVersion = "1.0.0.0";   // keep in step with ht160s.cpp GemInitial("HT160S","1.0.0.0")
    ecRecipeName    = "";

    //AI(ht160s-secsgem) 20260610 : wire transport->logic dispatch back-pointer
    if(HGemPtr!=NULL)
        HGemPtr->SetGemLogic(this);

    //AI(secs-ceid-align9045) 20260729 : EventDescription is a VERBATIM copy of HT9045
    //  uHGemHT9045.cpp HT9045Gem::HT9045Gem (272 assignments, CEID 1-275). 142/143/144 are
    //  left unassigned exactly as HT9045 leaves them, so AddCEID registers them with an
    //  empty alias and the .def dump matches the HT9045 firmware dump column-for-column.
    //  Strings are HT9045's, including its own typos - do NOT "improve" them, the host
    //  matches on them. See uHGemHT160.h ETypeStruct for the numbering contract.
    EventDescription[SECS_EVENT.DoStart                 ] = "1 Start Pressed";
    EventDescription[SECS_EVENT.DoPause                 ] = "2 Pause Pressed";
    EventDescription[SECS_EVENT.DoOneCycle              ] = "3 OneCycle Pressed";
    EventDescription[SECS_EVENT.DoCleanOut              ] = "4 CleanOut Pressed";
    EventDescription[SECS_EVENT.DoClearCount            ] = "5 ClearCount Pressed";
    EventDescription[SECS_EVENT.DoLotStart              ] = "6 Lot Start";
    EventDescription[SECS_EVENT.DoLot                   ] = "7 Lot";
    EventDescription[SECS_EVENT.DoLotEnd                ] = "8 Lot End";
    EventDescription[SECS_EVENT.SwitchRunMode           ] = "9 Switch Real Dummy Mode";
    EventDescription[SECS_EVENT.SwitchTesterMode        ] = "10 Switch Tester Online";
    EventDescription[SECS_EVENT.SwitchProduction        ] = "11 Switch Production Mode";
    EventDescription[SECS_EVENT.SwitchEngineer          ] = "12 Switch Engineer Mode";
    EventDescription[SECS_EVENT.SwitchTemperature       ] = "13 Switch Temperature Mode";
    EventDescription[SECS_EVENT.SwitchStartMode         ] = "14 Switch StartMode";
    EventDescription[SECS_EVENT.SwitchSetupFile         ] = "15 Switch Setup File";
    EventDescription[SECS_EVENT.SwitchUser              ] = "16 Switch UserLevel";
    EventDescription[SECS_EVENT.EnterTool               ] = "17 Enter Tool Page";
    EventDescription[SECS_EVENT.EnterConfig             ] = "18 Enter Maintenance Page";
    EventDescription[SECS_EVENT.EnterOffset             ] = "19 Enter Offset Page";
    EventDescription[SECS_EVENT.EnterSpeed              ] = "20 Enter Speed Page";
    EventDescription[SECS_EVENT.EnterIO                 ] = "21 Enter IO Page";
    EventDescription[SECS_EVENT.EnterMessage            ] = "22 Enter Message Page";
    EventDescription[SECS_EVENT.EnterDebug              ] = "23 Enter Debug Page";
    EventDescription[SECS_EVENT.DoExit                  ] = "24 Exit Pressed";
    EventDescription[SECS_EVENT.DoHome                  ] = "25 Home Pressed";
    EventDescription[SECS_EVENT.GetTestResult           ] = "26 Get Test Result";
    EventDescription[SECS_EVENT.RunStatus               ] = "27 Change Machine State";
    EventDescription[SECS_EVENT.DoRetry                 ] = "28 Retry Pressed";
    EventDescription[SECS_EVENT.DoSkip                  ] = "29 Skip Pressed";
    EventDescription[SECS_EVENT.DoAlarmReset            ] = "30 Alarm Reset Pressed";
    EventDescription[SECS_EVENT.DoTrayEnd               ] = "31 Tray End Pressed";
    EventDescription[SECS_EVENT.DoTrayFeed              ] = "32 Tray Feed Pressed";
    EventDescription[SECS_EVENT.DoReset                 ] = "33 Reset Pressed";
    EventDescription[SECS_EVENT.DoAutoClean             ] = "34 Auto Clean Start";
    EventDescription[SECS_EVENT.Auto1Full               ] = "35 Auto1 Full";
    EventDescription[SECS_EVENT.Auto2Full               ] = "36 Auto2 Full";
    EventDescription[SECS_EVENT.Auto3Full               ] = "37 Auto3 Full";
    EventDescription[SECS_EVENT.Fix1Full                ] = "38 Fix1 Full";
    EventDescription[SECS_EVENT.Fix2Full                ] = "39 Fix2 Full";
    EventDescription[SECS_EVENT.Fix3Full                ] = "40 Fix3 Full";
    EventDescription[SECS_EVENT.OneCycleFinish          ] = "41 One Cycle Finish";
    EventDescription[SECS_EVENT.CleanOutFinish          ] = "42 Clean Out Finish";
    EventDescription[SECS_EVENT.DownloadRecipe          ] = "43 DownLoadRecipe";
    EventDescription[SECS_EVENT.SiteOnOff               ] = "44 Site On Off";
    EventDescription[SECS_EVENT.ArmOnOff                ] = "45 Arm On Off";
    EventDescription[SECS_EVENT.SwitchTempData          ] = "46 Change Temp Defaultand Soak Time";
    EventDescription[SECS_EVENT.SwitchSpeed             ] = "47 Change HandlerSpeed";
    EventDescription[SECS_EVENT.ChangeEC                ] = "48 Change EC";
    EventDescription[SECS_EVENT.TrayFeedFinish          ] = "49 Tray Feed Finish";
    EventDescription[SECS_EVENT.AutoCleanFinish         ] = "50 Auto Clean Finish";
    EventDescription[SECS_EVENT.SiteMappingStart        ] = "51 Site Mapping Start";
    EventDescription[SECS_EVENT.SiteMappingEnd          ] = "52 Site Mapping End";
    EventDescription[SECS_EVENT.UPHRecordStart          ] = "53 UPH Record Start";
    EventDescription[SECS_EVENT.UPHRecordEnd            ] = "54 UPH Record End";
    EventDescription[SECS_EVENT.InitialArtStart         ] = "55 Initial ART Start";
    EventDescription[SECS_EVENT.TesterFT                ] = "56 Change Tester Program to FT";
    EventDescription[SECS_EVENT.TesterRT                ] = "57 Change Tester Program to RT";
    EventDescription[SECS_EVENT.ReadyForArt             ] = "58 Ready for ART";
    EventDescription[SECS_EVENT.ArtReceiveTrayOK        ] = "59 ART Receive Tray OK";
    EventDescription[SECS_EVENT.ArtReceiveTraySTART     ] = "60 ART Receive Tray START";
    EventDescription[SECS_EVENT.ArtRTFinish             ] = "61 RT Finish";
    EventDescription[SECS_EVENT.ArtTrayFeedFinish       ] = "62 ART Finish";
    EventDescription[SECS_EVENT.ArtFTFinish             ] = "63 FT Finish";
    EventDescription[SECS_EVENT.DownLoadRecipeByFTPOK   ] = "64 DownLoad Recipe by FTP OK";
    EventDescription[SECS_EVENT.DownLoadRecipeByFTPNG   ] = "65 DownLoad Recipe by FTP NG";
    EventDescription[SECS_EVENT.LoadTrayFinish          ] = "66 Load Tray Finish";
    EventDescription[SECS_EVENT.TrayTestFinish          ] = "67 Tray Test Finish";
    EventDescription[SECS_EVENT.AutoCleanClearCount     ] = "68 Auto Clean Clear Count";
    EventDescription[SECS_EVENT.SiteMappingStop         ] = "69 Site Mapping Stop";
    EventDescription[SECS_EVENT.BarcodeReaderEnter      ] = "70 Barcode Reader Enter";
    EventDescription[SECS_EVENT.OTDLock                 ] = "71 OTD Lock";
    EventDescription[SECS_EVENT.OTDUnLock               ] = "72 OTD UnLock";
    EventDescription[SECS_EVENT.MymessboxOK             ] = "73 Mymessbox OK";
    EventDescription[SECS_EVENT.RemoteProgramClose      ] = "74";
    EventDescription[SECS_EVENT.ChangeTesterPrgToEQC    ] = "75";
    EventDescription[SECS_EVENT.DoStartHasIC            ] = "76 Start Pressed HasIC";
    EventDescription[SECS_EVENT.ReadCurrentESDData      ] = "77 Read Current ESD Data";
    EventDescription[SECS_EVENT.JamSkipICCount          ] = "78 Jam Skip IC Count";
    EventDescription[SECS_EVENT.REVERSED79              ] = "79 ";
    EventDescription[SECS_EVENT.ReadNowHandlerData      ] = "80 Read Now Handler Data";
    EventDescription[SECS_EVENT.ReadATCTemperature      ] = "81 Read ATC Temperature";
    EventDescription[SECS_EVENT.ReadATCRefTemperature   ] = "82 Read ATC Ref Temperature";
    EventDescription[SECS_EVENT.ReadNowEPPenconder      ] = "83 Read Now EP Penconder";
    EventDescription[SECS_EVENT.RunStatus_FT            ] = "84 Run Status FT";
    EventDescription[SECS_EVENT.RunStatus_RT            ] = "85 Run Status RT";
    EventDescription[SECS_EVENT.MapNoArmHasIC           ] = "86 Map No Device Arm Has Device";
    EventDescription[SECS_EVENT.MapHasICArmRetry        ] = "87 Map Has Device Arm Error Retry";
    EventDescription[SECS_EVENT.MapHasICArmSkip         ] = "88 Map Has Device Arm Error Skip";
    EventDescription[SECS_EVENT.PreAlarmMessage         ] = "89 Pre Alarm Message";
    EventDescription[SECS_EVENT.GetTestResultAndBarcode ] = "90 Get TestResult And Barcode";
    EventDescription[SECS_EVENT.SECSOffline             ] = "91 SECS/GEM Offline";
    EventDescription[SECS_EVENT.SECSOnline              ] = "92 SECS/GEM Online";
    EventDescription[SECS_EVENT.SECSOnlineRemote        ] = "93 SECS/GEM Online Remote";
    EventDescription[SECS_EVENT.TransferBlocked         ] = "94 Transfer Blocked";
    EventDescription[SECS_EVENT.CassetteLoadComplete    ] = "95 Cassette Load Complete";
    EventDescription[SECS_EVENT.CassetteIDReadComplete  ] = "96 Cassette ID Read Complete";
    EventDescription[SECS_EVENT.ReadyToProcessComplete  ] = "97 Ready To Process Complete";
    EventDescription[SECS_EVENT.ReadyToCarrierOutLot    ] = "98 Ready To Carrier Out Lot";
    EventDescription[SECS_EVENT.CassetteOutComplete     ] = "99 Cassette Out Complete";
    EventDescription[SECS_EVENT.CassetteUnclamped       ] = "100 Cassette Unclamped";
    EventDescription[SECS_EVENT.ReadyToUnload           ] = "101 Ready To Unload";
    EventDescription[SECS_EVENT.UnloadComplete          ] = "102 Unload Complete";
    EventDescription[SECS_EVENT.ReadyToCarrierOutTray   ] = "103 Ready To Carrier Out Tray";
    EventDescription[SECS_EVENT.ReadyToCombinePass      ] = "104 Ready To Combine Pass";
    EventDescription[SECS_EVENT.ReadyToCombineFail      ] = "105 Ready To Combine Fail";
    EventDescription[SECS_EVENT.MachineNoStart          ] = "106 Machine No Start";
    EventDescription[SECS_EVENT.ReadyToCombinePassLotEnd] = "107 Ready To Combine Pass Lot End";
    EventDescription[SECS_EVENT.DoCSTLotStart           ] = "108 Cassette Lot Start";
    EventDescription[SECS_EVENT.DieCountFailMessageClose] = "109 Die Count Fail Message Close";
    EventDescription[SECS_EVENT.CleanOutTrayFeedFinish  ] = "110 Clean Out Tray Feed Finish";
    EventDescription[SECS_EVENT.MapNoICArmAutoSkip      ] = "111 Map No Device Arm Auto Skip";
    EventDescription[SECS_EVENT.MRRunModeChange         ] = "112 MR Run Mode Change";
    EventDescription[SECS_EVENT.AccessModeChange        ] = "113 Access Mode Change";
    EventDescription[SECS_EVENT.SoftwareBin             ] = "114 Software Bin";
    EventDescription[SECS_EVENT.TrayIDChange            ] = "115 Tray ID Change";
    EventDescription[SECS_EVENT.ReadyToLoadNoLot        ] = "116 Ready To Load No Lot";
    EventDescription[SECS_EVENT.ReadyToLoadNoTray       ] = "117 Ready To Load No Tray";
    EventDescription[SECS_EVENT.ReadyToLoadNoCassette   ] = "118 Ready To Load No Cassette";
    EventDescription[SECS_EVENT.ART_SRQKIND2_FTLOTSTART ] = "119 ART SRQKIND2 FT LOTSTART";
    EventDescription[SECS_EVENT.ART_SRQKIND4_RTLOTSTART ] = "120 ART SRQKIND4 RT LOTSTART";
    EventDescription[SECS_EVENT.ART_SRQKIND8_LOTEND     ] = "121 ART SRQKIND6 LOTEND";
    EventDescription[SECS_EVENT.ART_SRQKIND10_FINALLOTEND] = "122 FINAL LOTEND";
    EventDescription[SECS_EVENT.SafeDoorOnOff           ] = "123 Safe Door On Off";
    EventDescription[SECS_EVENT.SaveRecipe              ] = "124 Save Recipe";
    EventDescription[SECS_EVENT.EESUGOffestSelect       ] = "125 EESUG Offest Select";
    EventDescription[SECS_EVENT.EESUGOffestModify       ] = "126 EESUG Offest Modify";
    EventDescription[SECS_EVENT.Backtonormal            ] = "127 Back To Normal";
    EventDescription[SECS_EVENT.TestStart               ] = "128 Test Start";
    EventDescription[SECS_EVENT.TestFinish              ] = "129 Test Finish";
    EventDescription[SECS_EVENT.MaterialReceive         ] = "130 Material Receive";
    EventDescription[SECS_EVENT.SlotMapCountOK          ] = "131 Slot Map Count OK";
    EventDescription[SECS_EVENT.CHECK_IN                ] = "132 CHECK IN";
    EventDescription[SECS_EVENT.CHECK_OUT               ] = "133 CHECK OUT";
    EventDescription[SECS_EVENT.ReadyToCombineFailLotEnd] = "134 Ready To Combine Fail Lot End";
    EventDescription[SECS_EVENT.ReadyToOHTLotEnd        ] = "135 Ready To OHT Lot End";
    EventDescription[SECS_EVENT.Auto1Unloadtray         ] = "136 Auto 1 Unloading tray";
    EventDescription[SECS_EVENT.Auto2Unloadtray         ] = "137 Auto 2 Unloading tray";
    EventDescription[SECS_EVENT.Auto3Unloadtray         ] = "138 Auto 3 Unloading tray";
    EventDescription[SECS_EVENT.DoVisualSortLotStart    ] = "139 Click lot start button for visual sort mode";
    EventDescription[SECS_EVENT.PreLoadTray             ] = "140 Prepare Load Tray";
    EventDescription[SECS_EVENT.GemControlStateChange   ] = "141 GEM Control State Change";
    EventDescription[SECS_EVENT.Auto4Unloadtray         ] = "145 Auto 4 Unloading tray";
    EventDescription[SECS_EVENT.Auto5Unloadtray         ] = "146 Auto 5 Unloading tray";
    EventDescription[SECS_EVENT.Auto6Unloadtray         ] = "147 Auto 6 Unloading tray";
    EventDescription[SECS_EVENT.Auto4Full               ] = "148 Auto 4 Full";
    EventDescription[SECS_EVENT.Auto5Full               ] = "149 Auto 5 Full";
    EventDescription[SECS_EVENT.Auto6Full               ] = "150 Auto 6 Full";
    EventDescription[SECS_EVENT.Fix4Full                ] = "151 Fix 4 Full";
    EventDescription[SECS_EVENT.Fix5Full                ] = "152 Fix 5 Full";
    EventDescription[SECS_EVENT.Fix6Full                ] = "153 Fix 6 Full";
    EventDescription[SECS_EVENT.LoadNoTray              ] = "154 Loader hasn't tray";
    EventDescription[SECS_EVENT.LoadFullTray            ] = "155 Loader full of tray";
    EventDescription[SECS_EVENT.LoadOnlyOneTray         ] = "156 Loader only one tray";
    EventDescription[SECS_EVENT.Loader_ReadyToUnload    ] = "157 Loader ready to unload";
    EventDescription[SECS_EVENT.Loader_FinishUnload     ] = "158 Loader finish unload";
    EventDescription[SECS_EVENT.Empty_PreLoadTray       ] = "159 Empty Prepare Load Tray";
    EventDescription[SECS_EVENT.EmptyOnlyOneTray        ] = "160 Empty only one tray";
    EventDescription[SECS_EVENT.EmptyNoTray             ] = "161 Empty hasn't tray";
    EventDescription[SECS_EVENT.EmptyFullTray           ] = "162 Empty full of tray";
    EventDescription[SECS_EVENT.Color_PreLoadTray       ] = "163 Color Prepare Load Tray";
    EventDescription[SECS_EVENT.ColorOnlyOneTray        ] = "164 Color only one tray";
    EventDescription[SECS_EVENT.ColorNoTray             ] = "165 Color hasn't tray";
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto1    ] = "166 Empty put tray to Auto1";
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto2    ] = "167 Empty put tray to Auto2";
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto3    ] = "168 Empty put tray to Auto3";
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto4    ] = "169 Empty put tray to Auto4";
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto5    ] = "170 Empty put tray to Auto5";
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto6    ] = "171 Empty put tray to Auto6";
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto1   ] = "172 Empty put cover to Auto1";
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto2   ] = "173 Empty put cover to Auto2";
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto3   ] = "174 Empty put cover to Auto3";
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto4   ] = "175 Empty put cover to Auto4";
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto5   ] = "176 Empty put cover to Auto5";
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto6   ] = "177 Empty put cover to Auto6";
    EventDescription[SECS_EVENT.Color_PutTrayToAuto1    ] = "178 Color put tray to Auto1";
    EventDescription[SECS_EVENT.Color_PutTrayToAuto2    ] = "179 Color put tray to Auto2";
    EventDescription[SECS_EVENT.Color_PutTrayToAuto3    ] = "180 Color put tray to Auto3";
    EventDescription[SECS_EVENT.Color_PutTrayToAuto4    ] = "181 Color put tray to Auto4";
    EventDescription[SECS_EVENT.Color_PutTrayToAuto5    ] = "182 Color put tray to Auto5";
    EventDescription[SECS_EVENT.Color_PutTrayToAuto6    ] = "183 Color put tray to Auto6";
    EventDescription[SECS_EVENT.Color_PutCoverToAuto1   ] = "184 Color put cover to Auto1";
    EventDescription[SECS_EVENT.Color_PutCoverToAuto2   ] = "185 Color put cover to Auto2";
    EventDescription[SECS_EVENT.Color_PutCoverToAuto3   ] = "186 Color put cover to Auto3";
    EventDescription[SECS_EVENT.Color_PutCoverToAuto4   ] = "187 Color put cover to Auto4";
    EventDescription[SECS_EVENT.Color_PutCoverToAuto5   ] = "188 Color put cover to Auto5";
    EventDescription[SECS_EVENT.Color_PutCoverToAuto6   ] = "189 Color put cover to Auto6";
    EventDescription[SECS_EVENT.Auto1_LoadTrayFinish    ] = "190 Auto1 load tray finish";
    EventDescription[SECS_EVENT.Auto2_LoadTrayFinish    ] = "191 Auto2 load tray finish";
    EventDescription[SECS_EVENT.Auto3_LoadTrayFinish    ] = "192 Auto3 load tray finish";
    EventDescription[SECS_EVENT.Auto4_LoadTrayFinish    ] = "193 Auto4 load tray finish";
    EventDescription[SECS_EVENT.Auto5_LoadTrayFinish    ] = "194 Auto5 load tray finish";
    EventDescription[SECS_EVENT.Auto6_LoadTrayFinish    ] = "195 Auto6 load tray finish";
    EventDescription[SECS_EVENT.Auto1_ReadyToUnload     ] = "196 Auto1 ready to unload";
    EventDescription[SECS_EVENT.Auto2_ReadyToUnload     ] = "197 Auto2 ready to unload";
    EventDescription[SECS_EVENT.Auto3_ReadyToUnload     ] = "198 Auto3 ready to unload";
    EventDescription[SECS_EVENT.Auto4_ReadyToUnload     ] = "199 Auto4 ready to unload";
    EventDescription[SECS_EVENT.Auto5_ReadyToUnload     ] = "200 Auto5 ready to unload";
    EventDescription[SECS_EVENT.Auto6_ReadyToUnload     ] = "201 Auto6 ready to unload";
    EventDescription[SECS_EVENT.Auto1NoTray             ] = "202 Auto1 hasn't tray";
    EventDescription[SECS_EVENT.Auto2NoTray             ] = "203 Auto2 hasn't tray";
    EventDescription[SECS_EVENT.Auto3NoTray             ] = "204 Auto3 hasn't tray";
    EventDescription[SECS_EVENT.Auto4NoTray             ] = "205 Auto4 hasn't tray";
    EventDescription[SECS_EVENT.Auto5NoTray             ] = "206 Auto5 hasn't tray";
    EventDescription[SECS_EVENT.Auto6NoTray             ] = "207 Auto6 hasn't tray";
    EventDescription[SECS_EVENT.ColorFullTray           ] = "208 Color full of tray";
    EventDescription[SECS_EVENT.TrayEndFinish           ] = "209 Tray End Finish";
    EventDescription[SECS_EVENT.Empty_FinishUnload      ] = "210 Empty finish unload";
    EventDescription[SECS_EVENT.Color_FinishUnload      ] = "211 Color finish unload";
    EventDescription[SECS_EVENT.PowerSavingStart        ] = "212 Energy Saving Start";
    EventDescription[SECS_EVENT.PowerSavingEnd          ] = "213 Energy Saving End";
    EventDescription[SECS_EVENT.Reserved_03             ] = "214 Reserved_03";
    EventDescription[SECS_EVENT.Reserved_04             ] = "215 Reserved_04";
    EventDescription[SECS_EVENT.Reserved_05             ] = "216 Reserved_05";
    EventDescription[SECS_EVENT.Reserved_06             ] = "217 Reserved_06";
    EventDescription[SECS_EVENT.Reserved_07             ] = "218 Reserved_07";
    EventDescription[SECS_EVENT.Reserved_08             ] = "219 Reserved_08";
    EventDescription[SECS_EVENT.Reserved_09             ] = "220 Reserved_09";
    EventDescription[SECS_EVENT.Reserved_10             ] = "221 Reserved_10";
    EventDescription[SECS_EVENT.Reserved_11             ] = "222 Reserved_11";
    EventDescription[SECS_EVENT.Reserved_12             ] = "223 Reserved_12";
    EventDescription[SECS_EVENT.Reserved_13             ] = "224 Reserved_13";
    EventDescription[SECS_EVENT.Reserved_14             ] = "225 Reserved_14";
    EventDescription[SECS_EVENT.Reserved_15             ] = "226 Reserved_15";
    EventDescription[SECS_EVENT.Reserved_16             ] = "227 Reserved_16";
    EventDescription[SECS_EVENT.Reserved_17             ] = "228 Reserved_17";
    EventDescription[SECS_EVENT.Reserved_18             ] = "229 Reserved_18";
    EventDescription[SECS_EVENT.Reserved_19             ] = "230 Reserved_19";
    EventDescription[SECS_EVENT.Reserved_20             ] = "231 Reserved_20";
    EventDescription[SECS_EVENT.Reserved_21             ] = "232 Reserved_21";
    EventDescription[SECS_EVENT.Reserved_22             ] = "233 Reserved_22";
    EventDescription[SECS_EVENT.Reserved_23             ] = "234 Reserved_23";
    EventDescription[SECS_EVENT.Reserved_24             ] = "235 Reserved_24";
    EventDescription[SECS_EVENT.Reserved_25             ] = "236 Reserved_25";
    EventDescription[SECS_EVENT.Reserved_26             ] = "237 Reserved_26";
    EventDescription[SECS_EVENT.Reserved_27             ] = "238 Reserved_27";
    EventDescription[SECS_EVENT.Reserved_28             ] = "239 Reserved_28";
    EventDescription[SECS_EVENT.Reserved_29             ] = "240 Reserved_29";
    EventDescription[SECS_EVENT.Reserved_30             ] = "241 Reserved_30";
    EventDescription[SECS_EVENT.Reserved_31             ] = "242 Reserved_31";
    EventDescription[SECS_EVENT.Reserved_32             ] = "243 Reserved_32";
    EventDescription[SECS_EVENT.Reserved_33             ] = "244 Reserved_33";
    EventDescription[SECS_EVENT.Reserved_34             ] = "245 Reserved_34";
    EventDescription[SECS_EVENT.Reserved_35             ] = "246 Reserved_35";
    EventDescription[SECS_EVENT.Reserved_36             ] = "247 Reserved_36";
    EventDescription[SECS_EVENT.Reserved_37             ] = "248 Reserved_37";
    EventDescription[SECS_EVENT.Reserved_38             ] = "249 Reserved_38";
    EventDescription[SECS_EVENT.DoStartAutoHeight       ] = "250 START Auto contact height";
    EventDescription[SECS_EVENT.DoSecsGemIndexFail      ] = "251 SECS GEM consecutive failure";
    EventDescription[SECS_EVENT.Reserved_39             ] = "252 Reserved_39";
    EventDescription[SECS_EVENT.Reserved_40             ] = "253 Reserved_40";
    EventDescription[SECS_EVENT.Reserved_41             ] = "254 Reserved_41";
    EventDescription[SECS_EVENT.Reserved_42             ] = "255 Reserved_42";
    EventDescription[SECS_EVENT.Reserved_43             ] = "256 Reserved_43";
    EventDescription[SECS_EVENT.Reserved_44             ] = "257 Reserved_44";
    EventDescription[SECS_EVENT.Reserved_45             ] = "258 Reserved_45";
    EventDescription[SECS_EVENT.Reserved_46             ] = "259 Reserved_46";
    EventDescription[SECS_EVENT.Reserved_47             ] = "260 Reserved_47";
    EventDescription[SECS_EVENT.Reserved_48             ] = "261 Reserved_48";
    EventDescription[SECS_EVENT.Reserved_49             ] = "262 Reserved_49";
    EventDescription[SECS_EVENT.Reserved_50             ] = "263 Reserved_50";
    EventDescription[SECS_EVENT.Reserved_51             ] = "264 Reserved_51";
    EventDescription[SECS_EVENT.Reserved_52             ] = "265 Reserved_52";
    EventDescription[SECS_EVENT.Reserved_53             ] = "266 Reserved_53";
    EventDescription[SECS_EVENT.Reserved_54             ] = "267 Reserved_54";
    EventDescription[SECS_EVENT.Reserved_55             ] = "268 Reserved_55";
    EventDescription[SECS_EVENT.Reserved_56             ] = "269 Reserved_56";
    EventDescription[SECS_EVENT.Reserved_57             ] = "270 Reserved_57";
    EventDescription[SECS_EVENT.Reserved_58             ] = "271 Reserved_58";
    EventDescription[SECS_EVENT.AGVSupplement           ] = "272 AMR Supplement";
    EventDescription[SECS_EVENT.AGVLDUnLDStatus         ] = "273 AMR LDUnLD Status";
    EventDescription[SECS_EVENT.AGVLDUnLDFinish         ] = "274 AMR LDUnLD Finish";
    EventDescription[SECS_EVENT.AGVLdID                 ] = "275 AMR LD ID";
}
//---------------------------------------------------------------------------
HT160Gem::~HT160Gem()
{
    //AI(secs-audit-fix) 20260729 : un-wire the transport->logic back-pointer set in the ctor.
    //THGem::OnPeerDisconnected() calls GemLogic->OnCommunicationLost(), and THGem outlives us
    //whenever the global HSys (which owns HSys.MyGem == this) is destroyed before Application
    //(which owns HGem). Static-dtor order is not guaranteed here, so both ends clear the link :
    //~THGem nulls GemLogic before it stops its sockets, and we null it from this side too.
    if(HGemPtr!=NULL)
        HGemPtr->SetGemLogic(NULL);
}
//---------------------------------------------------------------------------
void HT160Gem::AddSV()
{
    sSystemTime = Now().FormatString("yyyy/mm/dd hh:nn:ss");
    if(HGemPtr==NULL)
        return;

    //AI(ht160s-secsgem) 20260612 : SVID numbering follows HT9045 AddSV for the
    // common identity/time/throughput band (1001 Machine Model, 1003 Software
    // Version, 1021 UPH, 1027 System Time) so a shared host SVID table works across
    // machines. HT160-specific run/alarm/output/lot SVs are relocated to a high
    // 66000+ band (9045 max SVID 65095 + 1000, rounded) so they can never collide
    // with 9045's 1010-1190 count region or any future 9045 SVID growth.
    // -- 9045-aligned common band --
    HGemPtr->SetSVDataPointer(1001, HType.ASCII_TYPE, "Machine Model", "", &HandlerPath, "machine model name (9045 SVID 1001)");
    HGemPtr->SetSVDataPointer(1003, HType.ASCII_TYPE, "Software Version", "", &svSoftwareVersion, "application software version (9045 SVID 1003)");
    HGemPtr->SetSVDataPointer(1021, HType.INT_4_TYPE, "UPH", "pcs/hr", &svUPH, "units per hour (9045 SVID 1021)");
    HGemPtr->SetSVDataPointer(1027, HType.ASCII_TYPE, "System Time", "", &sSystemTime, "current system time (9045 SVID 1027)");

    //AI(secs-gem-std) 20260727 : SVID 1518 Real/Dummy, 9045-aligned. Bound DIRECTLY to the live
    // HSys.LastSet.iRealDummy (int; DUMMY=0 / HAS_TRAY=1 / REALLY=2 already match the 9045
    // 0:Dummy / 1:Tray Only / 2:Real encoding) so a host RPTID-502 read gets the true mode
    // with no snapshot. The KYEC CJ_EAP host references SVID 1518 in report RPTID 502.
    HGemPtr->SetSVDataPointer(1518, HType.INT_4_TYPE, "Real/Dummy", "", &HSys.LastSet.iRealDummy, "0=Dummy 1=Tray Only 2=Real (9045 SVID 1518)");

    //AI(ht160s-secsgem) 20260612 : HT160 custom high band (66000+). Values are
    // snapshotted into the sv* members by RefreshSVData() right before each
    // serialize, so the bound address stays valid while the reported value tracks
    // live data. Sub-grouped with gaps (66000 state / 66010 alarm / 66020 output /
    // 66030 lot) so each group can grow without renumbering. These are HT160-only
    // semantics; 9045 has no equivalent SVID here.
    // -- Machine state (66000-66009) --
    HGemPtr->SetSVDataPointer(66000, HType.INT_4_TYPE, "Run Mode", "", &svRunMode, "0=Normal 1=Home 2=OneCycle 3=CleanOut 4=TrayFeed");
    HGemPtr->SetSVDataPointer(66001, HType.INT_4_TYPE, "System Running", "", &svSystemRunning, "1=machine started/running, 0=stopped");
    HGemPtr->SetSVDataPointer(66002, HType.INT_4_TYPE, "Control State", "", &iControlState, "GEM control state mirror (4=Local 5=Remote)");
    // -- Alarm state (66010-66019) --
    HGemPtr->SetSVDataPointer(66010, HType.INT_4_TYPE, "Alarm Active", "", &svAlarmActive, "1=alarm dialog showing, 0=no alarm");
    HGemPtr->SetSVDataPointer(66011, HType.INT_4_TYPE, "Alarm Code", "", &svAlarmCode, "current active alarm code (0=none)");
    // -- Output / production (66020-66029) --
    HGemPtr->SetSVDataPointer(66020, HType.INT_4_TYPE, "Total IC", "pcs", &svTotalIC, "total IC processed this lot/run");
    HGemPtr->SetSVDataPointer(66021, HType.INT_4_TYPE, "Total Sorted", "pcs", &svTotalSorted, "total IC sorted into a bin");
    // -- Current Lot (66030-66039) --
    HGemPtr->SetSVDataPointer(66030, HType.INT_4_TYPE, "Active Lot Count", "", &svLotCount, "lots currently loaded on the machine");
    HGemPtr->SetSVDataPointer(66031, HType.ASCII_TYPE, "Current Lot ID", "", &svCurrentLot, "first registered lot id (empty if none)");
    //AI(secs-lotstarttime) 20260730 : customer checklist "vendor lot start time". Latched at
    // Lot Start (manual button OR SECS LOTSTART), cleared at Lot End. Same text format as SVID
    // 1027 System Time. Deliberately NOT added to firmware report 1 : that report is the default
    // payload of EVERY CEID and its 13-SV shape is published in the customer interface spec, so
    // widening it would change every event on the wire. A host that wants the start time inside
    // an event binds 66033 into its own report with S2F33 + S2F35; otherwise S1F3 reads it back
    // at any time during the lot.
    HGemPtr->SetSVDataPointer(66033, HType.ASCII_TYPE, "Lot Start Time", "", &svLotStartTime, "yyyy/mm/dd hh:nn:ss when the current work order started (empty between lots)");
    //AI(ht160s-whitelist) 20260716 : Q6 host read-back of the active sort mode. Bound to the
    // live config int (stable global address, read at serialize time) so a SORTMODE switch via
    // S2F41 LOTSTART is confirmable by S1F3. No RefreshSVData mirror : not a per-cycle snapshot.
    //AI(ht160s-whitelist-override) 20260717 : report the EFFECTIVE mode (base + WhiteList overlay),
    // not the raw base, so the host reads WHITELIST during a WhiteList lot and the base between lots.
    //AI(ht160s-whitelist) 20260727 : WhiteList is NOT a 4th entry of the maintenance Sort Mode
    // selector (rgSortMode stays 3-way : 0/1/2). Value 3 means "the per-lot WhiteList overlay is
    // armed", whatever the base is; between lots the base value comes back. Matching semantic on
    // the command side : SORTMODE=NORMAL means "no overlay, keep the operator's base selector",
    // NOT "set base to 0", so a NORMAL lot can legitimately read back 1 or 2 here.
    HGemPtr->SetSVDataPointer(66032, HType.INT_4_TYPE, "Sort Mode", "", &GeneralSetting.iEffectiveSortMode, "effective: 0/1/2=base Normal/LotBin/LotPassFail, 3=WhiteList overlay armed");

    //AI(ht160s-agv) 20260615 : E87/AGV SVIDs (draft 38202-38245), bound to the
    // AgvCoord snapshot block (stable addresses). Bitmaps 38219-38221 are written
    // by the coordinator right before each AGV S6F11; carrier id / counts are
    // refreshed by the coordinator before the relevant events. SVIDs are not all
    // contiguous past Auto3, so they come from the station table, not a base+offset.
    HGemPtr->SetSVDataPointer(38219, HType.ASCII_TYPE, "Supplement Bin",     "", &AgvCoord.SupplementBitmap, "AGVSupplement P1-P9 bitmap");
    HGemPtr->SetSVDataPointer(38220, HType.ASCII_TYPE, "LD UnLD Check AGV",  "", &AgvCoord.StatusBitmap,     "AGVLDUnLDStatus P1-P9 bitmap");
    HGemPtr->SetSVDataPointer(38221, HType.ASCII_TYPE, "LD UnLD Finish AGV", "", &AgvCoord.FinishBitmap,     "AGVLDUnLDFinish P1-P9 bitmap");
    for(int ai = 0; ai < AGV_STATION_COUNT; ai++)
    {
        const TAgvStationDesc *d = &AgvStation[ai];
        AnsiString cName, tName, vName;
        cName.sprintf("%s Carrier ID", d->Name);
        tName.sprintf("AMR %s Tray Count", d->Name);
        vName.sprintf("AMR %s Device Count", d->Name);
        HGemPtr->SetSVDataPointer(d->SvidCarrierID, HType.ASCII_TYPE, cName, "",      &AgvCoord.CarrierID[ai],   "AGV carrier id");
        HGemPtr->SetSVDataPointer(d->SvidTrayCount, HType.INT_4_TYPE,  tName, "trays", &AgvCoord.TrayCount[ai],   "AGV tray count");
        HGemPtr->SetSVDataPointer(d->SvidDeviceCnt, HType.INT_4_TYPE,  vName, "pcs",   &AgvCoord.DeviceCount[ai], "AGV device count");
        if(d->SvidBinSet != 0 && d->AutoIndex >= 0)
        {
            AnsiString bName;
            bName.sprintf("AMR %s Bin Setting", d->Name);
            HGemPtr->SetSVDataPointer(d->SvidBinSet, HType.ASCII_TYPE, bName, "", &AgvCoord.BinSetting[d->AutoIndex], "AGV bin setting");
        }
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : snapshot live machine data into SV members.
// Called from THGem::EventReport (before S6F11) and S1F4_SelectedStatusReply.
// Kept NULL-safe (fNote may not exist yet) and type-safe (no bool/enum binding).
void HT160Gem::RefreshSVData()
{
    sSystemTime = Now().FormatString("yyyy/mm/dd hh:nn:ss");

    svRunMode       = (int)HSys.Sys.RunMode;
    svSystemRunning = HSys.Sys.SystemStart ? 1 : 0;

    svAlarmActive   = (fNote!=NULL && fNote->fShow) ? 1 : 0;
    svAlarmCode     = (fNote!=NULL) ? fNote->Code : 0;

    svTotalIC       = tRunData.TotalIC;
    svTotalSorted   = MachineRun.iTotalSorted;
    svUPH           = tRunData.UPH;

    svLotCount      = LotRegistry.GetLotCount();
    if(svLotCount>0)
    {
        TLotRunInfo *Lot = LotRegistry.GetLot(0);
        svCurrentLot = (Lot!=NULL) ? Lot->sLotID : AnsiString("");
    }
    else
        svCurrentLot = "";
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260612 : 1s tick from THGem::Timer1Timer. Sync the main-
// screen SECS status badge to the live HSMS link state. NULL-safe (fMain may not
// exist during very early boot / shutdown). The actual edge-trigger + repaint is
// inside TfMain::UpdateSecsFeatureBadge() so this stays a thin forwarder.
void HT160Gem::RefreshSecsBadge()
{
    if(fMain!=NULL)
        fMain->UpdateSecsFeatureBadge();
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260615 : 1s tick from THGem::Timer1Timer. Drive the E87/AGV
// coordinator : Phase B poll (Auto car full -> CEID272 AGVSupplement) + Phase D
// handshake service. AgvCoord fires S6F11 through HGemPtr only while SELECTED, so
// this is safe to call every tick regardless of link state.
void HT160Gem::ServiceAgv()
{
    //AI(ht160s-agv-binsetting) 20260713 : keep SVID 38234-45 (per-Auto bin setting) live
    // for host S1F3. Config-derived + RunMode/link-independent, so it runs BEFORE the
    // RunMode-gated PollAndCall (a host reads bin setting at config time, machine idle).
    AgvCoord.RefreshBinSettings();
    AgvCoord.PollAndCall(HGemPtr);
    AgvCoord.ServiceHandshake(HGemPtr);
}
//---------------------------------------------------------------------------
void HT160Gem::AddEC()
{
    if(HGemPtr==NULL)
        return;

    //AI(ht160s-secsgem) 20260611 : Equipment Constant catalog (modeled on
    // HT9045Gem::AddEC). Two binding styles on purpose:
    //  - Tray-form geometry is bound DIRECTLY to the live THT160TrayForm struct
    //    members (persistent + type-matched: double->FT_8, int->INT_4). This is
    //    the 9045 "&USE_RFID_READER" style: the host always reads the current
    //    value with no snapshot, and a future S2F15 write lands straight on the
    //    struct that Loader/SortArm/Monitor already read from.
    //  - Recipe name has no single stable address (it lives behind
    //    RecipeManager's getter/normalizer), so it is reported from the
    //    ecRecipeName snapshot refreshed in S2F14_EquipmentConstanData().
    // -- Recipe selection --
    HGemPtr->SetECDataPointer(1501, HType.ASCII_TYPE, "Recipe Name",     "",   &ecRecipeName,      "", "", "Default", "current recipe (Setup File) name");
    //AI(ht160s-secsgem) 20260612 : Tray Form geometry aligned to HT9045 "Type 1
    //  Tray" band (9045 ECID 2758-2763) instead of the old 2011-2016 (which collided
    //  with 9045's 2011 X-Dimension / 2012 Y-Dimension / 2013 Kit Diameter contact-
    //  force band). HT160 currently exposes a single tray geometry, bound directly to
    //  the live THT160TrayForm struct (persistent + type-matched).
    //  data\<recipe>\setup.ini [TrayForm]
    HGemPtr->SetECDataPointer(2760, HType.FT_8_TYPE,  "Tray X Start",    "mm", &TrayForm.XStart,   "", "", "0", "tray X start pos (9045 ECID 2760 Type1 Start Position X)");
    HGemPtr->SetECDataPointer(2758, HType.FT_8_TYPE,  "Tray X Pitch",    "mm", &TrayForm.XPitch,   "", "", "0", "tray X pitch (9045 ECID 2758 Type1 Pitch X)");
    HGemPtr->SetECDataPointer(2761, HType.FT_8_TYPE,  "Tray Y Start",    "mm", &TrayForm.YStart,   "", "", "0", "tray Y start pos (9045 ECID 2761 Type1 Start Position Y)");
    HGemPtr->SetECDataPointer(2759, HType.FT_8_TYPE,  "Tray Y Pitch",    "mm", &TrayForm.YPitch,   "", "", "0", "tray Y pitch (9045 ECID 2759 Type1 Pitch Y)");
    HGemPtr->SetECDataPointer(2762, HType.INT_4_TYPE, "Tray X Division", "",   &TrayForm.XDivision,"", "", "0", "tray columns / X count (9045 ECID 2762 Type1 Division X)");
    HGemPtr->SetECDataPointer(2763, HType.INT_4_TYPE, "Tray Y Division", "",   &TrayForm.YDivision,"", "", "0", "tray rows / Y count (9045 ECID 2763 Type1 Division Y)");
    //AI(ht160s-secsgem) 20260612 : RESERVED tray-type slots aligned to HT9045 for
    //  future HT160 multi-tray support. NOT registered yet (HT160 has no Type2/3
    //  data source / values today). When HT160 gains Type 2/3 tray geometries,
    //  register them on these reserved IDs and widen the S2F16 / GuiWriteTrayEC
    //  settable range accordingly:
    //    Type 2 -> 2771 Pitch X / 2772 Pitch Y / 2773 Start X / 2774 Start Y / 2775 Div X / 2776 Div Y
    //    Type 3 -> 2784 Pitch X / 2785 Pitch Y / 2786 Start X / 2787 Start Y / 2788 Div X / 2789 Div Y
}
//---------------------------------------------------------------------------
void HT160Gem::AddAlarmList()
{
    //AI(ht160s-secsgem) 20260715 : HT160 has no alarm StringGrid and needs no pre-built
    // store - the S5F6/S5F8 catalog is emitted LIVE from HSys.mapAlarmCodeList (the SSOT
    // populated by CreateSystemAlarmCode). Nothing to build here; kept for framework parity.
}
//---------------------------------------------------------------------------
void HT160Gem::AddCEID()
{
    int EquDefault = 1;
    unsigned ReportID[1];
    ReportID[0] = 1;

    if(HGemPtr==NULL)
        return;

    //AI(secs-ceid-align9045) 20260729 : register the FULL HT9045 dictionary (CEID 1-275),
    // mirroring HT9045Gem::AddCEID. Two deliberate departures from HT9045, both at the
    // REPORT-LINK layer only (the CEID numbers and aliases are byte-identical):
    //   1) HT9045 links CEID i -> RPTID i and only ever defines RPTID 1 = {1027}, so every
    //      id except 1 ships an empty L[0] until the host provisions S2F33/S2F35. HT160S
    //      keeps its own report 1 (13 machine-context SVs, see AddReprot) on every id, so a
    //      host that has NOT provisioned still gets usable data. A host that does provision
    //      overwrites the link anyway, so this cannot diverge once S2F35 has run.
    //   2) CEID i -> RPTID i would collide head-on with HT160S reports 2-7, which carry the
    //      AMR P1-P9 bitmaps / tray+device counts / identity-tray 2D. CEID 2 (Pause) would
    //      start shipping the AMR supplement bitmap and the 272-275 handshake would break.
    // Ids whose mechanism HT160S does not have are registered but have no emit site, so
    // they are inert : the dictionary matches HT9045, the traffic stays HT160S's.
    for(int i=SECS_EVENT.DoStart; i<SECS_EVENT.TotalEvent; i++)
    {
        HGemPtr->SetCEIDContent(i, EventDescription[i], 1, ReportID, EquDefault);
    }

    //AI(ht160s-agv) 20260615 : E87/AGV events mapped to DEDICATED reports (2/3/4/5)
    // NOT report 1, so the host receives the P-bitmap / carrier id, not the 13
    // machine-status SVs that every other CEID carries. Report content is defined
    // in AddReprot() (runs after AddCEID; EventReport resolves report->SV at send).
    //AI(ht160s-agv-devicecount) 20260713 : 272 (call) and 274 (finish) additionally
    // carry report 6 (all-station Tray/Device Count, see AddReprot) so the host gets
    // the real closing tray+IC numbers on the SAME S6F11 as the bitmap, no follow-up
    // S1F3 needed. 273 (Ready) stays bitmap-only : nothing has been counted yet.
    unsigned rptSup[2]; rptSup[0] = 2; rptSup[1] = 6;
    unsigned rptSta[1]; rptSta[0] = 3;
    unsigned rptFin[2]; rptFin[0] = 4; rptFin[1] = 6;
    unsigned rptCid[1]; rptCid[0] = 7;   //AI(ht160s-agv-identity2d) 20260713 : CEID275 -> dedicated report 7 ([38202] only), NOT the 9-wide report 5 (AGVLdID must not ship 8 stale carrier ids)
    HGemPtr->SetCEIDContent(272, "AGVSupplement",   2, rptSup, EquDefault);
    HGemPtr->SetCEIDContent(273, "AGVLDUnLDStatus", 1, rptSta, EquDefault);
    HGemPtr->SetCEIDContent(274, "AGVLDUnLDFinish", 2, rptFin, EquDefault);
    HGemPtr->SetCEIDContent(275, "AGVLdID",         1, rptCid, EquDefault);

    //AI(secs-ceid-align9045) 20260729 : the explicit Auto-Full block that used to live here
    // (35/36/37/148/149/150) is gone - the 1-275 loop above now registers those ids straight
    // from the HT9045 dictionary, with HT9045's own aliases ("35 Auto1 Full" ...) instead of
    // the locally sprintf'd ones. Same for the Unloadtray ids 136-138/145-147, which used to
    // be left unregistered on purpose : they are now registered like every other id, so they
    // ship report 1 instead of an empty L[0]. Emit sites are unchanged (uAgvStation PollAndCall
    // for Full, aAuto1To6 DoDischargeTray for Unloadtray).
}
//---------------------------------------------------------------------------
void HT160Gem::AddReprot()
{
    int EquDefault = 1;
    //AI(ht160s-secsgem) 20260612 : report 1 carries the real machine-data SVs on
    // every event, using the 9045-aligned numbering (1001/1003/1021/1027) plus the
    // HT160 custom high band (66000+).
    unsigned ReportIDContent[16];

    if(HGemPtr==NULL)
        return;

    ReportIDContent[ 0] = 1001;  // Machine Model     (9045-aligned)
    ReportIDContent[ 1] = 1003;  // Software Version  (9045-aligned)
    ReportIDContent[ 2] = 1021;  // UPH               (9045-aligned)
    ReportIDContent[ 3] = 1027;  // System Time       (9045-aligned)
    ReportIDContent[ 4] = 66000; // Run Mode          (HT160 custom band)
    ReportIDContent[ 5] = 66001; // System Running
    ReportIDContent[ 6] = 66002; // Control State
    ReportIDContent[ 7] = 66010; // Alarm Active
    ReportIDContent[ 8] = 66011; // Alarm Code
    ReportIDContent[ 9] = 66020; // Total IC
    ReportIDContent[10] = 66021; // Total Sorted
    ReportIDContent[11] = 66030; // Active Lot Count
    ReportIDContent[12] = 66031; // Current Lot ID
    HGemPtr->SetReportIDContent(1, 13, ReportIDContent, EquDefault);

    //AI(ht160s-agv) 20260615 : dedicated AGV reports. 2/3/4 each carry a single
    // P1-P9 bitmap SV (supplement/status/finish); 5 carries all nine carrier IDs.
    unsigned rSup[1]; rSup[0] = 38219; HGemPtr->SetReportIDContent(2, 1, rSup, EquDefault);
    unsigned rSta[1]; rSta[0] = 38220; HGemPtr->SetReportIDContent(3, 1, rSta, EquDefault);
    unsigned rFin[1]; rFin[0] = 38221; HGemPtr->SetReportIDContent(4, 1, rFin, EquDefault);
    unsigned rCid[AGV_STATION_COUNT];
    for(int ci = 0; ci < AGV_STATION_COUNT; ci++)
        rCid[ci] = AgvStation[ci].SvidCarrierID;
    HGemPtr->SetReportIDContent(5, AGV_STATION_COUNT, rCid, EquDefault);

    //AI(ht160s-agv-devicecount) 20260713 : report 6 = all nine stations' Tray Count
    // then all nine Device Count SVIDs (same fixed P1-P9 order as report 5's carrier
    // ids), attached to CEID272/274 (AddCEID) so the host reads the real per-station
    // tray+IC numbers off the same S6F11 as the bitmap.
    unsigned rCnt[AGV_STATION_COUNT * 2];
    for(int ni = 0; ni < AGV_STATION_COUNT; ni++)
        rCnt[ni] = AgvStation[ni].SvidTrayCount;
    for(int ni = 0; ni < AGV_STATION_COUNT; ni++)
        rCnt[AGV_STATION_COUNT + ni] = AgvStation[ni].SvidDeviceCnt;
    HGemPtr->SetReportIDContent(6, AGV_STATION_COUNT * 2, rCnt, EquDefault);

    //AI(ht160s-agv-identity2d) 20260714 : report 7 = ONLY the identity carrier SVID
    // (AgvStation[AMR_IDENTITY_CARRIER_INDEX].SvidCarrierID = Color P3 / SVID 38204). CEID275
    // (AGVLdID) links to THIS (AddCEID rptCid[0]=7), so the AGVLdID event carries just the
    // freshly-scanned identity-tray 2D and does NOT ship the 8 other per-station carrier ids
    // (which may be stale). The Auto-stack carrier ids stay host-pollable via S1F3; if the host
    // wants them evented it can link report 5 via S2F35. Matches HT9045 (AGVLdID = load id; Auto
    // carrier ids poll-only). SVID derived from the single change-point constant so the stamped
    // CarrierID[] index and the reported SVID stay locked to one edit.
    unsigned rLdId[1]; rLdId[0] = AgvStation[AMR_IDENTITY_CARRIER_INDEX].SvidCarrierID;
    HGemPtr->SetReportIDContent(7, 1, rLdId, EquDefault);

    //AI(secs-reportdef) 20260724 : premature save removed - persistence loads at boot via ReadEventReportData and saves on host S2F33/F37 only
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
//AI(ht160s-secsgem) 20260611 : S2F13 Equipment Constant Request -> S2F14.
//  Mirror of S1F3/S1F4 but over the EC registry. Request body = L,n { ECID };
//  n==0 means "report all EC". Reply body = L,m { <EC live value> }. Tray-form
//  ECs read straight from THT160TrayForm; the recipe-name EC is refreshed from
//  RecipeManager into ecRecipeName first (no stable address to bind).
//---------------------------------------------------------------------------
void HT160Gem::S2F14_EquipmentConstanData()
{
    int n, len, i;
    unsigned char Type;
    AnsiString S;
    unsigned reqList[512];
    int reqCount = 0;

    if(HGemPtr==NULL)
        return;

    HGemPtr->ResetReturnCode();

    //AI(ht160s-secsgem) 20260611 : refresh snapshot-backed EC value (recipe name)
    ecRecipeName = RecipeManager.GetCurrentRecipeName();

    if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
    {
        // malformed request -> reply empty list
        HGemPtr->InitLocalHead(2, 14, 0);
        HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
        HGemPtr->SendLocalData();
        return;
    }

    if(n<=0)
    {
        // report every registered EC
        int total = HGemPtr->GetECCount();
        HGemPtr->InitLocalHead(2, 14, 0);
        HGemPtr->DataItemOut(total, HType.LIST_TYPE, NULL);
        for(i=0; i<total; i++)
            HGemPtr->DataItemOutECValue(HGemPtr->GetECIDByIndex(i));
        HGemPtr->SendLocalData();
        return;
    }

    // read requested ECIDs first (this consumes the receive buffer)
    for(i=0; i<n && reqCount<512; i++)
    {
        if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
            break;
        S = "";
        if(HGemPtr->DataItemIn(len, Type, S)==1)
            reqList[reqCount++] = (unsigned)StrToIntDef(S, 0);
        else
            reqList[reqCount++] = 0;     // unreadable -> empty item
    }

    HGemPtr->InitLocalHead(2, 14, 0);
    HGemPtr->DataItemOut(reqCount, HType.LIST_TYPE, NULL);
    for(i=0; i<reqCount; i++)
        HGemPtr->DataItemOutECValue(reqList[i]);
    HGemPtr->SendLocalData();
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : S1F3 Selected Equipment Status Request -> S1F4.
//  Request body = L,n { SVID ... }; n==0 means "report all SV". Reply body =
//  L,m { <SV live value> ... } in request order (or registry order for all).
//  SVIDs are read via the AnsiString numeric overload so U1/U2/U4/I1/I2/I4 all
//  parse uniformly; unknown SVID encodes an empty L[0] item.
//---------------------------------------------------------------------------
void HT160Gem::S1F4_SelectedStatusReply()
{
    int n, len, i;
    unsigned char Type;
    AnsiString S;
    unsigned reqList[512];
    int reqCount = 0;

    if(HGemPtr==NULL)
        return;

    HGemPtr->ResetReturnCode();

    //AI(ht160s-secsgem) 20260611 : snapshot live machine data before replying
    RefreshSVData();

    if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
    {
        // malformed request -> reply empty list
        HGemPtr->InitLocalHead(1, 4, 0);
        HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
        HGemPtr->SendLocalData();
        return;
    }

    if(n<=0)
    {
        // report every registered SV
        int total = HGemPtr->GetSVCount();
        HGemPtr->InitLocalHead(1, 4, 0);
        HGemPtr->DataItemOut(total, HType.LIST_TYPE, NULL);
        for(i=0; i<total; i++)
            HGemPtr->DataItemOutSVValue(HGemPtr->GetSVIDByIndex(i));
        HGemPtr->SendLocalData();
        return;
    }

    // read requested SVIDs first (this consumes the receive buffer)
    for(i=0; i<n && reqCount<512; i++)
    {
        if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
            break;
        S = "";
        if(HGemPtr->DataItemIn(len, Type, S)==1)
            reqList[reqCount++] = (unsigned)StrToIntDef(S, 0);
        else
            reqList[reqCount++] = 0;     // unreadable -> empty item
    }

    HGemPtr->InitLocalHead(1, 4, 0);
    HGemPtr->DataItemOut(reqCount, HType.LIST_TYPE, NULL);
    for(i=0; i<reqCount; i++)
        HGemPtr->DataItemOutSVValue(reqList[i]);
    HGemPtr->SendLocalData();
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : S1F11 Status Variable Namelist Request -> S1F12.
//  Request  L,n { U4 SVID }        n<=0 -> every registered SV.
//  Reply    L,n { L,3 { U4 SVID, A SVNAME, A UNITS } }.
//  Read-only: touches no machine state. Unknown SVID -> empty name+unit per E5.
//---------------------------------------------------------------------------
void HT160Gem::S1F12_StatusVariableNamelistReply()
{
    int n, len, i;
    unsigned char Type;
    AnsiString S;
    unsigned reqList[512];
    int reqCount = 0;

    if(HGemPtr==NULL)
        return;

    HGemPtr->ResetReturnCode();

    if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
    {
        // malformed request -> reply empty list
        HGemPtr->InitLocalHead(1, 12, 0);
        HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
        HGemPtr->SendLocalData();
        return;
    }

    if(n<=0)
    {
        int total = HGemPtr->GetSVCount();
        for(i=0; i<total && reqCount<512; i++)
            reqList[reqCount++] = HGemPtr->GetSVIDByIndex(i);
    }
    else
    {
        // read requested SVIDs first (this consumes the receive buffer)
        for(i=0; i<n && reqCount<512; i++)
        {
            if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
                break;
            S = "";
            if(HGemPtr->DataItemIn(len, Type, S)==1)
                reqList[reqCount++] = (unsigned)StrToIntDef(S, 0);
            else
                reqList[reqCount++] = 0;
        }
    }

    HGemPtr->InitLocalHead(1, 12, 0);
    HGemPtr->DataItemOut(reqCount, HType.LIST_TYPE, NULL);
    for(i=0; i<reqCount; i++)
    {
        unsigned SVID = reqList[i];
        AnsiString nm = HGemPtr->GetSVName(SVID);
        AnsiString un = HGemPtr->GetSVUnit(SVID);
        HGemPtr->DataItemOut(3, HType.LIST_TYPE, NULL);
        HGemPtr->DataItemOut(1, HType.UINT_4_TYPE, &SVID);
        HGemPtr->DataItemOut(HType.ASCII_TYPE, nm);
        HGemPtr->DataItemOut(HType.ASCII_TYPE, un);
    }
    HGemPtr->SendLocalData();
}
//---------------------------------------------------------------------------
//AI(secs-namelist) 20260730 : S1F23 Collection Event Namelist Request -> S1F24.
//  Request  L,n { U4 CEID }        n<=0 -> EVERY registered CEID.
//  Reply    L,n { L,3 { U4 CEID, A CENAME, L,m { U4 VID } } }.
//  Read-only : touches no machine state, changes no report link, enables no event.
//  Unknown CEID -> { CEID, "", L,0 } so the top-level shape stays constant (HT9045
//  answers the same way; see its S1F24 "no such CEID" branch).
//
//  FULL DUMP is deliberate. AddCEID registers all 275 HT9045 ids, and only 52 of them
//  have an emit site on HT160S - this reply lists all 275 anyway:
//    - HT9045 dumps its whole dictionary too, so a host that diffs the two machines'
//      namelists sees one dictionary, which is the entire point of the 20260729 align.
//    - The namelist is NOT a subscription. The host binds what it wants with S2F33 +
//      S2F35 and arms it with S2F37, so a wider namelist cannot make an unemitted CEID
//      start arriving. Trimming to the 52 would instead hide ids the host may legitimately
//      pre-define reports against (e.g. for a later firmware that does emit them).
//  Size: 275 x (name + report 1's 13 VIDs) is ~35 KB against the 1 MB encode buffer, and
//  DataItemOut's overflow backstop poisons the message rather than truncating it.
//---------------------------------------------------------------------------
void HT160Gem::S1F24_CollectionEventNamelist()
{
    int n, len, i;
    unsigned char Type;
    AnsiString S;
    unsigned reqList[512];
    int reqCount = 0;

    if(HGemPtr==NULL)
        return;

    HGemPtr->ResetReturnCode();

    if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
    {
        // malformed request -> reply empty list (same policy as S1F12)
        HGemPtr->InitLocalHead(1, 24, 0);
        HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
        HGemPtr->SendLocalData();
        return;
    }

    if(n<=0)
    {
        int total = HGemPtr->GetCEIDCount();
        for(i=0; i<total && reqCount<512; i++)
            reqList[reqCount++] = HGemPtr->GetCEIDByIndex(i);
        if(total > reqCount)
        {
            AnsiString sCap;
            sCap.sprintf("[SECS][TX] S1F24 CEID list capped at %d of %d registered", reqCount, total);
            HGemPtr->StringOut(sCap);
        }
    }
    else
    {
        // read requested CEIDs first (this consumes the receive buffer)
        for(i=0; i<n && reqCount<512; i++)
        {
            if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
                break;
            S = "";
            if(HGemPtr->DataItemIn(len, Type, S)==1)
                reqList[reqCount++] = (unsigned)StrToIntDef(S, 0);
            else
                reqList[reqCount++] = 0;
        }
    }

    HGemPtr->InitLocalHead(1, 24, 0);
    HGemPtr->DataItemOut(reqCount, HType.LIST_TYPE, NULL);
    for(i=0; i<reqCount; i++)
    {
        unsigned CEID = reqList[i];
        AnsiString nm = HGemPtr->GetCEIDAlias(CEID);
        //AI(secs-namelist) 20260730 : 1024 covers 32 linked reports of the firmware-shaped
        // sizes with headroom; a host-defined monster report set can still overrun it, so
        // say so in the log instead of shipping a silently short VID list.
        unsigned vidList[1024];
        int vidCount = HGemPtr->GetCEIDVidList(CEID, vidList, 1024);
        if(vidCount >= 1024)
        {
            AnsiString sTrunc;
            sTrunc.sprintf("[SECS][TX] S1F24 CEID %u VID list TRUNCATED at %d", CEID, vidCount);
            HGemPtr->StringOut(sTrunc);
        }

        HGemPtr->DataItemOut(3, HType.LIST_TYPE, NULL);
        HGemPtr->DataItemOut(1, HType.UINT_4_TYPE, &CEID);
        HGemPtr->DataItemOut(HType.ASCII_TYPE, nm);
        HGemPtr->DataItemOut(vidCount, HType.LIST_TYPE, NULL);
        for(int v=0; v<vidCount; v++)
        {
            unsigned VID = vidList[v];
            HGemPtr->DataItemOut(1, HType.UINT_4_TYPE, &VID);
        }
    }
    HGemPtr->SendLocalData();
}
//---------------------------------------------------------------------------
//AI(secs-namelist) 20260730 : S2F29 Equipment Constant Namelist Request -> S2F30.
//  Request  L,n { U4 ECID }        n<=0 -> EVERY registered EC.
//  Reply    L,n { L,6 { U4 ECID, A ECNAME, ECMIN, ECMAX, ECDEF, A ECUNITS } }.
//  ECMIN / ECMAX / ECDEF are encoded in the EC's OWN declared type (HT9045 shape), so a
//  FT_8 tray dimension reports FT_8 limits, not ASCII. An EC that declares no limit ships
//  a ZERO-LENGTH item of that type - E5's "no limit declared" - which is what HT9045 emits
//  when its Min/Max/Default pointer is NULL. Today only 1501 declares a default ("Default")
//  and the six tray-form ECs declare a default of "0"; no EC declares min/max yet, so most
//  rows carry three empty items. That is a registration-data gap, not a protocol gap: fill
//  the limits in AddEC and this reply follows with no code change.
//  Unknown ECID -> L,6 { ECID, "", <0-len>, <0-len>, <0-len>, "" } (GetECType answers ASCII
//  for an unknown id, so the empty items are well-typed).
//  Read-only : reports the registry only. Host EC WRITES still go through S2F15, which keeps
//  its idle-gate and its "tray geometry only" whitelist - S2F30 does not widen what is settable.
//---------------------------------------------------------------------------
void HT160Gem::S2F30_EquipmentConstantNamelistReply()
{
    int n, len, i;
    unsigned char Type;
    AnsiString S;
    unsigned reqList[512];
    int reqCount = 0;

    if(HGemPtr==NULL)
        return;

    HGemPtr->ResetReturnCode();

    if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
    {
        // malformed request -> reply empty list (same policy as S1F12 / S1F24)
        HGemPtr->InitLocalHead(2, 30, 0);
        HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
        HGemPtr->SendLocalData();
        return;
    }

    if(n<=0)
    {
        int total = HGemPtr->GetECCount();
        for(i=0; i<total && reqCount<512; i++)
            reqList[reqCount++] = HGemPtr->GetECIDByIndex(i);
        if(total > reqCount)
        {
            AnsiString sCap;
            sCap.sprintf("[SECS][TX] S2F30 EC list capped at %d of %d registered", reqCount, total);
            HGemPtr->StringOut(sCap);
        }
    }
    else
    {
        // read requested ECIDs first (this consumes the receive buffer)
        for(i=0; i<n && reqCount<512; i++)
        {
            if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
                break;
            S = "";
            if(HGemPtr->DataItemIn(len, Type, S)==1)
                reqList[reqCount++] = (unsigned)StrToIntDef(S, 0);
            else
                reqList[reqCount++] = 0;
        }
    }

    HGemPtr->InitLocalHead(2, 30, 0);
    HGemPtr->DataItemOut(reqCount, HType.LIST_TYPE, NULL);
    for(i=0; i<reqCount; i++)
    {
        unsigned ECID = reqList[i];
        unsigned char ecType = HGemPtr->GetECType(ECID);
        AnsiString nm = HGemPtr->GetECName(ECID);
        AnsiString un = HGemPtr->GetECUnit(ECID);

        HGemPtr->DataItemOut(6, HType.LIST_TYPE, NULL);
        HGemPtr->DataItemOut(1, HType.UINT_4_TYPE, &ECID);
        HGemPtr->DataItemOut(HType.ASCII_TYPE, nm);
        HGemPtr->DataItemOutTypedText(ecType, HGemPtr->GetECMinValue(ECID));
        HGemPtr->DataItemOutTypedText(ecType, HGemPtr->GetECMaxValue(ECID));
        HGemPtr->DataItemOutTypedText(ecType, HGemPtr->GetECDefaultValue(ECID));
        HGemPtr->DataItemOut(HType.ASCII_TYPE, un);
    }
    HGemPtr->SendLocalData();
}
//---------------------------------------------------------------------------
//AI(secs-lotstarttime) 20260730 : latch / clear SVID 66033 Lot Start Time.
//  Called from the manual Lot Start + Lot End buttons (main.cpp) and from the SECS
//  LOTSTART accept path, so the host reads the same value whichever way the lot began.
//  Format matches SVID 1027 System Time ("yyyy/mm/dd hh:nn:ss") so a host can compare
//  the two without reformatting.
//  NOT persisted: a power cycle mid-lot leaves it empty until the next Lot Start. The
//  event stream is the durable record (CEID 6 carries the moment); this SV exists so a
//  host that connected late, or that wants to re-read, can still ask.
//---------------------------------------------------------------------------
void HT160Gem::NoteLotStartTime(bool bStarted)
{
    if(bStarted)
        svLotStartTime = Now().FormatString("yyyy/mm/dd hh:nn:ss");
    else
        svLotStartTime = "";
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : S2F15 New Equipment Constant Send -> S2F16.
//  Request  L,n { L,2 { ECID, ECV } }.   Reply  S2F16  B EAC.
//  EAC: 0=ok, 1=one+ ECID unknown / not host-settable, 2=busy, 3=range.
//  SAFETY: only the tray-form geometry ECs (2011-2016) are host-settable, and
//  only while the machine is idle (SystemStart==false && no IC under machine).
//  Writing tray geometry while running could shift sort/place coordinates, so a
//  busy machine rejects the whole request (EAC=2, nothing written). Recipe-name
//  (1501) and any other ECID are rejected (EAC=1) to keep host EC-set out of
//  recipe switching. A successful tray write is persisted via TrayForm.Save().
//  NOTE: writes are applied incrementally; on a mixed batch the accepted items
//  are already written when a later item fails. Host should send tray ECs alone.
//---------------------------------------------------------------------------
void HT160Gem::S2F16_NewEquipmentConstantSendAcknowledge()
{
    int n, inner, len, i;
    unsigned char Type;
    unsigned char EAC = 0;
    AnsiString sECID, sVal, sLog;
    unsigned ECID;
    bool bWroteTrayForm = false;

    if(HGemPtr==NULL)
        return;

    HGemPtr->ResetReturnCode();

    bool bBusy = (HSys.Sys.SystemStart==true || HasICUnderMachine()==true);

    if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
    {
        EAC = 1;        // malformed request
        n = 0;
    }

    for(i=0; i<n; i++)
    {
        // inner L,2 { ECID, ECV }
        if(HGemPtr->GetDataItemLenAndTypeAndDelete(inner, HType.LIST_TYPE)!=1)
        {
            EAC = 1;
            break;
        }
        sECID = "";
        if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
            HGemPtr->DataItemIn(len, Type, sECID);
        sVal = "";
        if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
            HGemPtr->DataItemIn(len, Type, sVal);

        ECID = (unsigned)StrToIntDef(sECID, 0);

        if(bBusy)
        {
            EAC = 2;                 // busy -> reject, write nothing
            continue;
        }
        if(ECID>=2758 && ECID<=2763)  //AI(ht160s-secsgem) 20260612 : 9045 Type1 tray band
        {
            if(HGemPtr->WriteECValueByString(ECID, sVal)==0)
                bWroteTrayForm = true;
            else
                EAC = 3;             // unconvertible / out of range
        }
        else
        {
            EAC = 1;                 // not a host-settable constant
        }
    }

    if(bWroteTrayForm && !bBusy)
        TrayForm.Save(RecipeManager.GetCurrentRecipeName());

    //AI(secs-ceid-align9045) 20260729 : CEID 48 "Change EC". HT9045 reports this from its own
    //S2F15 commit loop (uHGemEquipment.cpp:4021), once the new value has actually been taken.
    //HT160S only ever commits the tray-form geometry band, so bWroteTrayForm is the exact
    //"an EC really changed" condition. EAC!=0 (rejected / busy / out of range) writes nothing
    //and therefore reports nothing.
    if(bWroteTrayForm && !bBusy && EAC == 0)
        HGemPtr->EventReport(1, SECS_EVENT.ChangeEC);

    // S2F16 reply : B EAC
    HGemPtr->InitLocalHead(2, 16, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &EAC);
    HGemPtr->SendLocalData();

    sLog.sprintf("[SECS] S2F16 EAC=%u busy=%d trayWritten=%d",
                 (unsigned)EAC, (int)bBusy, (int)bWroteTrayForm);
    HGemPtr->StringOut(sLog);
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260612 : local GUI EC editor write path. Enforces the
//  exact same policy as the host S2F16: only the tray-form geometry ECs
//  (9045 Type1 band 2758-2763) are settable, only while the machine is idle, and
//  a successful write is persisted to the active recipe's setup.ini. Returns 0=ok,
//  1=not settable / unknown, 2=busy, 3=convert/range error.
int HT160Gem::GuiWriteTrayEC(unsigned ECID, AnsiString sValue)
{
    if(HGemPtr==NULL)
        return 1;

    bool bBusy = (HSys.Sys.SystemStart==true || HasICUnderMachine()==true);
    if(bBusy)
        return 2;

    if(ECID<2758 || ECID>2763)
        return 1;        // not a host/GUI-settable constant

    if(HGemPtr->WriteECValueByString(ECID, sValue)!=0)
        return 3;        // unconvertible value

    TrayForm.Save(RecipeManager.GetCurrentRecipeName());
    HGemPtr->StringOut("[SECS][GUI] EC " + AnsiString((int)ECID) + " = " + sValue + " (tray form saved)");
    return 0;
}
//---------------------------------------------------------------------------
//AI(secs-rcmd-9045) 20260729 : HT9045 host commands that exist in its S2F42 dispatch but
//describe TESTER mechanisms HT-160S does not have - test/retest flow (ART / MRT / AQL / FT-RT
//program switching), socket cleaning, site mapping, device-temperature offsets and EESUG
//offsets, and yield-fail handling. Every name below was verified present in
//D:/HT9045/HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP/SECSGEM/uHGemHT9045.cpp.
//
//These answer HCACK=2 "recognised, cannot perform" rather than falling through to the
//unknown-command HCACK=1. Reason: to a host audit, 1 reads as "this equipment has never
//heard of the command" (suggesting a typo or a version mismatch) whereas 2 reads as "known
//command, not available on this machine" - which is the true statement. This is the same
//answer the existing ENERGY_SAVING branch gives, and the same answer KYEC's own HT9045 gave
//to ENERGY_SAVING 23/23 times on 2026-06-08 without the host escalating.
//
//NEVER 4 here : SEMI E5 HCACK=4 is a POSITIVE ack promising a later completion event, and
//none of these will ever produce one.
//
//Exact match on the trimmed name, NOT the AnsiPos prefix match the other branches use,
//because several of these names are prefixes of each other or of live commands
//(INITIAL_START_ART vs INITIAL_START, START_AQL vs START_AGV). A prefix test here could
//swallow a command HT-160S actually implements.
static bool IsTesterOnlyRcmd(AnsiString S)
{
    static const char *Names[] = {
        "AUTO_RETEST",
        "CONTINUE_RETEST_ART",
        "CONTINUE_START_ART",
        "CONTINUE_START_MRT",
        "INITIAL_START_ART",
        "INITIAL_START_MRT",
        "RETEST_MRT",
        "SWITCH_TO_FT",
        "SWITCH_TO_RT",
        "START_AQL",
        "DEVTEMPOFFSETADJUST",
        "TESTTEMPSETTING",
        "EESUG_OFFSET",
        "AUTOSITEMAP",
        "AUTO_CLEAN",
        "YIELD_FAIL"
    };
    AnsiString T = S.Trim();
    int Count = (int)(sizeof(Names)/sizeof(Names[0]));
    for(int i=0; i<Count; i++)
    {
        if(T==AnsiString(Names[i]))
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260610 : S2F41 SET_LOT_INFO variable-length multi-Lot.
//  Reads outer L[2]{ A "SET_LOT_INFO", L[n]{ A lotID ... } }, refills
//  LotRegistry (overwrite, D1), backfills first lot to edLotNo (D2), rejects
//  while producing / IC under machine (D3, HCACK=4). HCACK: 0=ok,1=format,
//  2=param,4=busy (D4); cap HT160_MAX_LOT=64, over -> HCACK=2 (D5).
//  S2F42 reply: L[2]{ B HCACK, L[0] }.
//---------------------------------------------------------------------------
int HT160Gem::S2F42_Host_Command_Acknowledge()
{
    unsigned char HCACK = 1;        // default = format error
    int len, n, i;
    unsigned char Type;
    char CommandStr[256];
    char str[256];
    AnsiString S;
    AnsiString sRxDetail = "";   //AI(ht160s-agv) 20260720 : parsed-content echo for AMR-tab host-cmd feedback

    if(HGemPtr==NULL)
        return 1;

    ZeroMemory(CommandStr, sizeof(CommandStr));
    ZeroMemory(str, sizeof(str));
    HGemPtr->ResetReturnCode();

    // Outer L[2]
    if(HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
    {
        // Command name (ASCII)
        HGemPtr->GetDataItemLenAndType(len, Type);
        if(Type==HType.ASCII_TYPE && len>0 && len<(int)sizeof(CommandStr))
        {
            if(HGemPtr->DataItemIn(len, HType.ASCII_TYPE, CommandStr)==1)
                S = CommandStr;
            else
                S = "";
        }
        else
        {
            S = "";
        }
        S = S.UpperCase();

        if(S.AnsiPos("SET_LOT_INFO")==1)
        {
            // Inner L[n] : n = number of Lots (variable)
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)==1)
            {
                sRxDetail = "lots=" + IntToStr(n);
                if(n==0)
                {
                    HCACK = 2;                                   // empty list -> param error
                }
                else if(n>HT160_MAX_LOT)
                {
                    HCACK = 2;                                   // one packet cannot exceed capacity
                }
                //AI(secs-lot-additive) 20260730 : the blanket "producing / IC inside -> HCACK=4"
                // guard and the ArchiveDiscardedWorkOrder gate that used to sit here are BOTH gone.
                // Both existed because SET_LOT_INFO was a destructive OVERWRITE (Clear + refill).
                // It is now purely ADDITIVE, so it destroys nothing: appending a lot to a live
                // multi-lot order is safe (AddLot appends or reuses a freed slot, existing slot
                // INDICES never move, and tray cells / SortArm slots hold those indices as raw
                // ints), and there is nothing left to archive. The one case that still destroys
                // data - re-declaring an existing Lot under a DIFFERENT KYEC batch, which retires
                // that Lot's 2D->Bin list - keeps a busy guard, applied below AFTER the parse so
                // it only fires for packets that actually retire something.
                else
                {
                    //AI(ht160s-ftp) 20260721 : parse into local buffers FIRST, commit atomically
                    // ONLY after the whole list parses cleanly. A mid-list reject
                    // (HCACK!=0) then leaves the prior work order untouched, so the host's "rejected"
                    // belief matches the machine. Mirrors the LOTSTART buffer-then-commit path; the
                    // previous code Cleared before the loop and committed incrementally, leaving a
                    // partial lot set live after a reject.
                    AnsiString bufCust[HT160_MAX_LOT];
                    AnsiString bufKyec[HT160_MAX_LOT];
                    int nBuf = 0;
                    HCACK = 0;
                    for(i=0; i<n; i++)
                    {
                        //AI(ht160s-ftp) 20260721 : each SET_LOT_INFO item is EITHER
                        //   A  "custLot"                     (legacy : Cust lot only, no KYEC batch)
                        //   L[2]{ A "custLot", A "kyecLot" }  (KYEC : Cust lot + Kyec batch id, Soter col7)
                        // Backward compatible : an old host sending bare ASCII still works unchanged; a
                        // new host sends the pair. The L[2] READ idiom mirrors the proven LOTSTART
                        // SORTMODE-pair reader below, so it cannot mis-consume the stream. **The pair
                        // SHAPE is OUR PROPOSAL pending KYEC confirmation of the SET_LOT_INFO SML**
                        // (docs/plan/ftp-kyec-upload-plan-20260721) -- if KYEC carries the KYEC lot a
                        // different way, ONLY this L[2] branch changes; the ASCII branch is the shipped path.
                        AnsiString custLot = "";
                        AnsiString kyecLot = "";
                        if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
                        {
                            HCACK = 2;                           // truncated list -> param error
                            break;
                        }
                        if(Type==HType.LIST_TYPE)
                        {
                            int pairLen = 0;
                            if(HGemPtr->GetDataItemLenAndTypeAndDelete(pairLen, HType.LIST_TYPE)!=1
                               || pairLen<1 || pairLen>2)
                            {
                                HCACK = 2;                       // malformed (custLot[,kyecLot]) pair
                                break;
                            }
                            if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                                HGemPtr->DataItemIn(len, Type, custLot);
                            if(pairLen>=2 && HGemPtr->GetDataItemLenAndType(len, Type)==1)
                                HGemPtr->DataItemIn(len, Type, kyecLot);
                            if(custLot.Trim()=="")
                            {
                                HCACK = 2;                       // pair carried no Cust lot -> param error
                                break;
                            }
                        }
                        else if(Type==HType.ASCII_TYPE && len>0 && len<(int)sizeof(str))
                        {
                            if(HGemPtr->DataItemIn(len, HType.ASCII_TYPE, str)!=1)
                            {
                                HCACK = 2;                       // read failure -> param error
                                break;
                            }
                            custLot = AnsiString(str);
                        }
                        else
                        {
                            HCACK = 2;                           // type mismatch -> param error
                            break;
                        }

                        if(nBuf>=HT160_MAX_LOT)
                        {
                            HCACK = 2;                           // more items than capacity -> param error
                            break;
                        }
                        bufCust[nBuf] = custLot;
                        bufKyec[nBuf] = kyecLot;
                        nBuf++;
                    }

                    //AI(secs-lot-additive) 20260730 : admission checks on the PARSED list, before
                    // anything is committed. Two things the old overwrite semantics made
                    // impossible and additive semantics make mandatory:
                    //  1) real capacity. The per-packet n>HT160_MAX_LOT test above is not enough
                    //     once lots accumulate; AddLot returns -1 when the registry is full and
                    //     the old commit loop ignored that, so the host got HCACK=0 for lots the
                    //     machine had silently dropped. Count the genuinely NEW ids and refuse the
                    //     whole packet if they would not fit.
                    //  2) a KYEC batch change on an EXISTING lot. That retires the lot's old
                    //     2D->Bin list (see the commit loop), which must never happen with that
                    //     lot's material still in the machine.
                    if(HCACK==0)
                    {
                        int nNewLots = 0;
                        int nRetire  = 0;
                        for(int j=0; j<nBuf; j++)
                        {
                            int iExist = LotRegistry.FindLotIndex(bufCust[j]);
                            if(iExist<0)
                            {
                                nNewLots++;
                                continue;
                            }
                            TLotRunInfo *pOld = LotRegistry.GetLot(iExist);
                            if(pOld!=NULL
                               && bufKyec[j].Trim()!=""
                               && pOld->sKyecLotID.Trim()!=""
                               && pOld->sKyecLotID.Trim()!=bufKyec[j].Trim())
                                nRetire++;
                        }
                        if(LotRegistry.GetLotCount()+nNewLots > HT160_MAX_LOT)
                        {
                            HCACK = 2;                           // registry would overflow -> param error
                            RecordProcess("SECS SET_LOT_INFO refused : lot registry full ("
                                          +IntToStr(LotRegistry.GetLotCount())+"+"+IntToStr(nNewLots)
                                          +" > "+IntToStr(HT160_MAX_LOT)+")");
                        }
                        else if(nRetire>0 && HasICUnderMachine()==true)
                        {
                            HCACK = 4;                           // would retire live 2D data -> busy
                            RecordProcess("SECS SET_LOT_INFO refused : KYEC batch change with IC still under the machine");
                        }
                    }

                    // Commit atomically only on a clean parse + clean admission. On any reject the
                    // prior work order is left intact, matching the host's HCACK!=0 belief.
                    if(HCACK==0)
                    {
                        //AI(secs-lot-additive) 20260730 : ADDITIVE. No LotRegistry.Clear() and no
                        // LotBinBinding.Clear() here any more - SET_LOT_INFO declares lots, it does
                        // not open or close a work order. Lot END (DoLotEndProcess, reachable from
                        // the operator button, the AMR CleanOut-finish path and the new
                        // CLEAR_LOT_INFO host command) remains the ONLY thing that clears them.
                        // AddLot is already dedupe-keep-existing (CosFunction.cpp:978-994), so a
                        // re-sent lot id keeps its slot, its index and its per-bin counters.
                        for(int j=0; j<nBuf; j++)
                        {
                            //AI(secs-lot-additive) 20260730 : resolve "did this lot already exist"
                            // BEFORE AddLot, otherwise AddLot has already created it and every lot
                            // looks new.
                            bool bExisted = (LotRegistry.FindLotIndex(bufCust[j])>=0);
                            int iLotIdx = LotRegistry.AddLot(bufCust[j], HT160_LOT_SOURCE_SECS, "", "");
                            if(iLotIdx<0)
                            {
                                // Unreachable after the capacity check above; never fail silently.
                                HCACK = 2;
                                RecordProcess("SECS SET_LOT_INFO : AddLot rejected '"+bufCust[j]+"'");
                                break;
                            }
                            TLotRunInfo *pLot = LotRegistry.GetLot(iLotIdx);
                            if(pLot==NULL)
                                continue;

                            AnsiString sNewKyec = bufKyec[j].Trim();
                            AnsiString sOldKyec = pLot->sKyecLotID.Trim();
                            //AI(secs-lot-additive) 20260730 : a CHANGED KYEC batch id under the same
                            // customer lot means a different physical batch, so the previous batch's
                            // 2D->Bin list must stop routing - additive merging would otherwise leave
                            // codes that are absent from the new list still routable on their old bin.
                            // Unchanged (or newly supplied) batch id = a refresh: keep the data.
                            if(bExisted && sNewKyec!="" && sOldKyec!="" && sNewKyec!=sOldKyec)
                            {
                                int nDropped = LotRegistry.ClearLotItems(bufCust[j]);
                                RecordProcess("SECS SET_LOT_INFO : lot "+bufCust[j]+" KYEC batch "
                                              +sOldKyec+" -> "+sNewKyec+", retired "
                                              +IntToStr(nDropped)+" 2D items");
                            }
                            //AI(secs-lot-additive) 20260730 : only OVERWRITE the stored KYEC batch id
                            // when the host actually supplied one. It used to be assigned
                            // unconditionally, so a legacy bare-ASCII re-send of an existing lot wiped
                            // the batch identity and persisted "" into WorkOrder.json. Mirrors the JSON
                            // parser, which also only writes the field when it is present.
                            if(sNewKyec!="")
                                pLot->sKyecLotID = sNewKyec;
                        }
                    }
                }
            }
            else
            {
                HCACK = 1;                                       // bad list format
            }
            //AI(general) 20260610 : project LotRegistry onto the on-screen Lot list
            // (sgLotList) so SECS-pushed lots become visible, using the single shared
            // refresh that Simu / JSON / manual paths also use. SECS runs on the VCL
            // main thread (stNonBlocking OnClientRead), so this UI call is safe.
            if(HCACK==0 && fMain!=NULL)
            {
                fMain->RefreshLotListFromRegistry();
                //AI(ht160s-2dbin-manual) 20260628 : persist the SECS-pushed lot list.
                fMain->SaveWorkOrder();
            }
        }
        else if(S.AnsiPos("PAUSE")==1)
        {
            //AI(ht160s-secsgem) 20260611 : PAUSE host command. Consume the (usually empty)
            // parameter list, then pause via the shared MachinePause() choke point
            // (csystem.cpp) : drop SystemStart, DecStop the motors, raise SoftStop, and
            // RecordProcess-log "MACHINE PAUSE by secs-remote". EventReport stays out of
            // the GEM layer (existing boundary; it is operator-specific).
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            MachinePause(trigSecsRemote);
            HCACK = 0;
        }
        else if(S.AnsiPos("CLEARCOUNT")==1)
        {
            //AI(ht160s-lot-reset) 20260706 : CLEARCOUNT host command (HT172 SECS
            // ClearCount parity). Zero the per-run production counters and persist to
            // lastdata. Refused while producing so a running total is never wiped.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            if(HSys.Sys.SystemStart==true)
            {
                HCACK = 4;                                       // producing -> busy
            }
            else
            {
                ResetPerLotProductionCounters();
                WriteLastDataIni();
                HCACK = 0;
            }
        }
        else if(S.AnsiPos("CLEAR_LOT_INFO")==1)
        {
            //AI(secs-lot-additive) 20260730 : the host-side LOT END, ported from HT9045
            // (uHGemHT9045.cpp:2431-2454), which refuses while running (HCACK=1) or with material
            // inside (HCACK=2) and otherwise clears the lot info + the 2D list. HT160S routes it
            // through DoLotEndProcess() - the SAME body the operator's Lot End button and the AMR
            // CleanOut-finish path use - so host and operator close a work order through ONE path
            // (UPH record, CEID 8, LotStory archive, LotRegistry.Clear, LotBinBinding.Clear,
            // WhiteList overlay revert, in-flight WebAPI pull cancel).
            //
            // This command is REQUIRED now that SET_LOT_INFO is additive: it is the only thing that
            // retires lots, so without it the registry would grow to HT160_MAX_LOT and stay there.
            // DoLotEndProcess shows no modal, so it is safe on the HSMS receive path.
            // Not sent by KYEC on 2026-06-08 (that line ends lots from the panel), so this is a
            // capability, not a regression risk.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            if(HSys.Sys.SystemStart==true)
            {
                HCACK = 1;                                       // 9045 parity : running -> 1
                RecordProcess("SECS CLEAR_LOT_INFO refused : machine is running");
            }
            else if(HasICUnderMachine()==true)
            {
                HCACK = 2;                                       // 9045 parity : IC inside -> 2
                RecordProcess("SECS CLEAR_LOT_INFO refused : IC still under the machine");
            }
            else if(fMain==NULL)
            {
                HCACK = 2;                                       // no UI context
            }
            else
            {
                RecordProcess("SECS CLEAR_LOT_INFO : lot end by host");
                fMain->DoLotEndProcess();
                HCACK = 0;
            }
        }
        else if(S.AnsiPos("CLEAN_AUTO_SORT_COUNT")==1)
        {
            //AI(secs-rcmd-9045) 20260729 : HT9045 RCMD, ported. KYEC sent it TWICE on
            // 2026-06-08, so this is a command the host really uses. HT9045
            // (uHGemHT9045.cpp:1145) refuses while material is in the machine, then logs the
            // per-bin counts and clears them. Same shape here.
            // NOT the same as HT160S's own CLEARCOUNT : that one wipes the machine-level
            // totals too (TotalIC / UPH / LoaderIC / JamCount). See ResetAutoSortCounters().
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            if(HasICUnderMachine())
            {
                HCACK = 2;                                       // material still inside -> cannot perform
                RecordProcess("SECS CLEAN_AUTO_SORT_COUNT refused : IC still under the machine");
            }
            else
            {
                RecordProcess("SECS CLEAN_AUTO_SORT_COUNT clearing : "+DescribeAutoSortCounters());
                ResetAutoSortCounters();
                WriteLastDataIni();
                HCACK = 0;
            }
        }
        else if(S.AnsiPos("CLEAN_OUT")==1)
        {
            //AI(secs-rcmd-9045) 20260729 : HT9045 RCMD, ported. HT9045
            // (uHGemHT9045.cpp:1460) just calls its own Clean Out button handler and answers 0.
            // HT160S routes through TfMain::CleanOutCore() (the modal-free body shared with
            // sbCleanOut1Click) so operator and host arm the drain through ONE path.
            // Departure from HT9045 : it answers 0 unconditionally. HT160S's Clean Out only
            // arms from Run_Normal, and silently answering 0 when nothing was armed would tell
            // the host a drain started that never will, so a refused arm answers 2.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            if(fMain==NULL)
                HCACK = 2;                                       // no UI context
            else if(fMain->CleanOutCore())
            {
                RecordProcess("SECS CLEAN_OUT : clean-out armed by host");
                HCACK = 0;
            }
            else
            {
                HCACK = 2;                                       // not in Run_Normal -> nothing armed
                RecordProcess("SECS CLEAN_OUT refused : machine is not in Normal run mode");
            }
        }
        else if(S.AnsiPos("HALT")==1)
        {
            //AI(secs-rcmd-9045) 20260729 : HT9045 RCMD, ported. HT9045
            // (uHGemHT9045.cpp:1794) clears its SoftStart run latch - a SOFT stop, no motor
            // stop command - and answers 2 when its remote-start precondition is not armed.
            // So HALT maps onto HT160S's MachinePause() choke point (decelerating stop), the
            // same one RCMD PAUSE uses, NOT MachineStop() (which hard-stops the motors and is
            // what RCMD STOP is for). HT9045 itself aliases PAUSE and STOP into one branch
            // (uHGemHT9045.cpp:1081), so answering HALT with the soft path is in keeping.
            // Gate mirrors HT9045's intent : do not claim to have halted a machine that was
            // not running.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            if(HSys.Sys.SystemStart==false)
            {
                HCACK = 2;                                       // not running -> nothing to halt
                RecordProcess("SECS HALT refused : machine is not running");
            }
            else
            {
                MachinePause(trigSecsRemote);
                HCACK = 0;
            }
        }
        else if(S.AnsiPos("ONLINE_REMOTE")==1 || S=="ONLINE")
        {
            //AI(ht160s-secsgem) 20260611 : ONLINE (remote) host command. Consume the
            // parameter list and move the local control-state mirror to Online Remote(5).
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            iControlState = 5;
            HCACK = 0;
        }
        else if(S.AnsiPos("ONLINE_LOCAL")==1)
        {
            //AI(ht160s-secsgem) 20260611 : ONLINE_LOCAL host command. Consume the
            // parameter list and move the control-state mirror to Online Local(4).
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            iControlState = 4;
            HCACK = 0;
        }
        else if(S.AnsiPos("LOTSTART")==1)
        {
            //AI(secs-lot-additive) 20260730 : LOTSTART realigned to HT9045 (uHGemHT9045.cpp:2081-2087,
            // which simply presses its own Lot Start button and answers HCACK=0 unconditionally, with
            // no busy guard and no parameters). The KYEC host sends L[0] EVERY time - verified on the
            // whole 2026-06-08 floor log, SECSGEM_TextLog_16.txt:526, _17.txt:2165, _18.txt:8277 - and
            // never sends SET_LOT_INFO at all, so the old "inner L[n] of lot ids, reject if empty"
            // shape answered HCACK=2 to every real host LOTSTART and the batch could not start.
            //
            // Contract now:
            //  * carries NO lot identity. SET_LOT_INFO is the ONLY lot-info setter; any ASCII item
            //    here is consumed and IGNORED (counted + logged), never registered.
            //  * answers HCACK=0. The ONLY non-zero answers come from HT160S's own SORTMODE
            //    extension, which HT9045 does not have - so a 9045-shaped message ALWAYS gets 0.
            //  * may be repeated freely : HT160S is a multi-lot machine and this is how the host
            //    refreshes 2D/Bin data mid-lot.
            //  * branches on the lot lifecycle state:
            //        lot CLOSED -> full per-lot init (LotStartCore) + 2D/Bin exchange
            //        lot OPEN   -> NO init, 2D/Bin exchange only
            //    "open" is MachineRun.bRunning (cprod.h:194 : set only by a Lot Start, cleared only
            //    by DoLotEndProcess - pause / alarm / stop / HOME never touch it) OR'd with
            //    HasICUnderMachine(). The OR is a safety widening, not a nicety: if material is in
            //    the machine the lot is de-facto open, and running the full init would clear the
            //    (Lot,Bin)->Auto bindings and per-lot counters under that material - mis-routing it
            //    and mislabelling its trace.
            int  nIgnoredLots = 0;
            int  pairLen;
            AnsiString PendingSortMode = "";      // "" = no SORTMODE pair present
            bool bLotOpen = (MachineRun.bRunning==true || HasICUnderMachine()==true);
            HCACK = 0;                                           // 9045 parity : unconditional accept
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)==1)
            {
                for(i=0; i<n; i++)
                {
                    if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
                        break;                                   // truncated : stop reading, still ACK
                    if(Type==HType.LIST_TYPE)
                    {
                        //AI(ht160s-whitelist) 20260716 : the optional L[2]{ A"SORTMODE", A value }
                        // pair (NORMAL|WHITELIST), HT160S's own extension. Kept because it is the
                        // shipped WhiteList contract with the customer.
                        if(HGemPtr->GetDataItemLenAndTypeAndDelete(pairLen, HType.LIST_TYPE)!=1 || pairLen!=2)
                        {
                            HCACK = 2;                           // malformed pair : cannot be honoured
                            break;
                        }
                        AnsiString cpName="", cpVal="";
                        if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                            HGemPtr->DataItemIn(len, Type, cpName);
                        if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                            HGemPtr->DataItemIn(len, Type, cpVal);
                        if(cpName.Trim().UpperCase()!="SORTMODE")
                        {
                            HCACK = 2;                           // only SORTMODE is recognized
                            break;
                        }
                        AnsiString sMode = cpVal.Trim().UpperCase();
                        if(sMode!="NORMAL" && sMode!="WHITELIST")
                        {
                            HCACK = 2;                           // value out of domain
                            break;
                        }
                        PendingSortMode = sMode;                 // duplicate pair -> last one wins
                    }
                    else if(Type==HType.ASCII_TYPE && len>0 && len<(int)sizeof(str))
                    {
                        //AI(secs-lot-additive) 20260730 : lot ids are NO LONGER registered from here.
                        // Consume the item so the stream stays aligned, count it for the log/echo.
                        if(HGemPtr->DataItemIn(len, HType.ASCII_TYPE, str)!=1)
                            break;
                        nIgnoredLots++;
                    }
                    else
                    {
                        break;                                   // unknown item : stop reading, still ACK
                    }
                }
            }
            sRxDetail = "ignored_lotids=" + IntToStr(nIgnoredLots)
                      + " sortmode=" + (PendingSortMode==""?AnsiString("-"):PendingSortMode)
                      + " lot_open=" + AnsiString(bLotOpen?"Y":"N");
            if(nIgnoredLots>0)
                RecordProcess("SECS LOTSTART : "+IntToStr(nIgnoredLots)
                              +" lot id(s) in the packet ignored - use SET_LOT_INFO to declare lots");

            //AI(ht160s-whitelist) 20260716 : a sort-mode change is only safe with the lot closed.
            // Reject the whole packet rather than switch the classification model mid-lot.
            if(HCACK==0 && PendingSortMode!="" && bLotOpen==true)
                HCACK = 4;
            //AI(secs-lot-additive) 20260730 : the mid-lot "exchange" in WhiteList mode would be
            // LoadWhiteListFile(), and that Clear()s the WHOLE registry before it even checks the
            // file exists (main.cpp, deliberate : the local file is authoritative at lot open). Doing
            // that with a lot open empties the routing table under live material - and permanently if
            // the file is missing or invalid. Refuse this one case instead. The WebAPI path is a
            // non-destructive merge and stays allowed.
            if(HCACK==0 && bLotOpen==true && GeneralSetting.IsWhiteListSortMode())
            {
                HCACK = 4;
                RecordProcess("SECS LOTSTART refused : WhiteList mode cannot re-load its file with a lot open");
            }

            if(HCACK==0 && fMain!=NULL)
            {
                //AI(ht160s-whitelist-override) 20260717 : WhiteList is a per-lot OVERLAY.
                //AI(secs-lot-additive) 20260730 : apply it ONLY when the pair is actually present.
                // It used to be written on EVERY accepted LOTSTART, so a bare repeat (which the host
                // now sends routinely just to refresh 2D data) silently DISARMED a whitelist lot
                // mid-run and flipped the 2D source and the reject semantics with it.
                if(PendingSortMode!="")
                {
                    GeneralSetting.SetWhiteListActive(PendingSortMode=="WHITELIST");
                    GeneralSetting.SaveWhiteListOverlay();
                    if(fMaintenance!=NULL)
                        fMaintenance->SyncSortModeSelectorFromSetting();
                    fMain->UpdateSortModeFeatureBadge();
                }

                if(bLotOpen==false)
                {
                    //---- lot CLOSED : open it. Full per-lot init + 2D/Bin exchange. ----
                    // LotStartCore is the SAME body the operator's Lot Start button runs (counters,
                    // UPH folder, Soter buffer, product-info, (Lot,Bin) bindings, bRunning, work-order
                    // save, SVID 66033 latch, CEID 6) and it also performs the 2D/Bin exchange.
                    // It does NOT register lots - SET_LOT_INFO already did that.
                    AnsiString FirstLot = "";
                    int SlotCount = LotRegistry.GetLotSlotCount();
                    for(int k=0; k<SlotCount; k++)
                    {
                        TLotRunInfo *pLot = LotRegistry.GetLot(k);
                        if(pLot!=NULL && pLot->sLotID.Trim()!="")
                        {
                            FirstLot = pLot->sLotID.Trim();
                            break;
                        }
                    }
                    if(FirstLot=="")
                    {
                        //AI(secs-lot-additive) 20260730 : nothing declared yet. Still HCACK=0 (9045
                        // parity - it presses its Lot Start button whatever the state), but there is
                        // no lot to open and nothing to exchange. Idempotent : the next LOTSTART
                        // after a SET_LOT_INFO opens the lot normally.
                        RecordProcess("SECS LOTSTART : no lot declared (send SET_LOT_INFO first) - nothing started");
                    }
                    else
                    {
                        fMain->LotStartCore(FirstLot, "by secs-remote");
                        fMain->RefreshLotListFromRegistry();
                    }
                }
                else
                {
                    //---- lot OPEN : exchange only, no initialisation. ----
                    // Deliberately does NOT touch counters, the UPH folder, the Soter buffer, the
                    // (Lot,Bin) bindings or bRunning. StartLotWebApiPullAll folds into a sweep that
                    // is already in flight rather than restarting the cursor.
                    RecordProcess("SECS LOTSTART : lot already open - 2D/Bin exchange only, no re-init");
                    fMain->RefreshLotListFromRegistry();
                    fMain->StartLotWebApiPullAll();              // async, no modal
                }
            }
        }
        else if(S=="START")
        {
            //AI(machine-command-layer) 20260625 : production START host command. Route
            // through the single MachineStart() gate (csystem.cpp) -- the SAME entry the
            // operator Start button uses, so the lot/2D gate, the SystemStart raise, the
            // per-IC trace batch and home-first-when-unhomed all run identically. The gate
            // now lives inside MachineStart (not duplicated here); map its result to HCACK.
            // MachineStart never pops a modal, so the HSMS receive path is not stalled.
            // Exact "START" compare (NOT AnsiPos) so it does not swallow "START_AGV".
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            {
                AnsiString Reason;
                switch(MachineStart(trigSecsRemote, Reason))
                {
                    case msStarted:      HCACK = 0; break;   // armed (homes first if unhomed)
                    case msRejBusy:      HCACK = 4; break;   // already running
                    case msRejNoContext: HCACK = 2; break;   // no UI context
                    default:             HCACK = 2; break;   // msRejNotReady : lot/2D not ready
                }
            }
        }
        else if(S.AnsiPos("START_AGV")==1)
        {
            //AI(ht160s-agv) 20260615 : Phase C START_AGV. Inner L[n] of
            // L[2]{ A cpName, <cpValue> } pairs (spec 4.1). Station names
            // (Loader/Empty/Color/AUTO1..6) record a per-station AGV-handoff prep
            // intent via AgvCoord.BeginPrep; LoaderTrayCount sets the expected loader
            // tray count (SVID 38222). Records intent + ACKs ONLY : no motion is
            // driven and no Ready (CEID273) is faked - that is Phase D + in-place sensor.
            //AI(secs-kyec-startagv) 20260728 : refuse START_AGV outright when AMR is switched off.
            // BeginPrep/ReassertLocks have no bUseAMR gate, but EVERY release path does -
            // PollAndCall (uAgvStation.cpp:229), ServiceHandshake (:360) and the WAR0962 timeout
            // sweep (csystem.cpp:78) all early-return when bUseAMR==false. So a START_AGV received
            // with AMR off would lock a module with no code path left that can ever unlock it.
            // HCACK=2 also matches the documented host-simulator contract
            // (D:\AI_Area\Tool\HT160S_SECS_Simulator\code\ht160s_presets.py "2=param (unknown CP or non-AMR)").
            int innerLen;
            if(GeneralSetting.bUseAMR==false)
            {
                HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
                HGemPtr->StringOut("[SECS] START_AGV refused : AMR is disabled (bUseAMR=0)");
                HCACK = 2;
            }
            else if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)==1)
            {
                if(n<=0)
                {
                    HCACK = 2;                                   // empty list -> param error
                }
                else
                {
                    HCACK = 0;
                    for(i=0; i<n; i++)
                    {
                        if(HGemPtr->GetDataItemLenAndTypeAndDelete(innerLen, HType.LIST_TYPE)!=1)
                        {
                            HCACK = 1;                           // pair not a list -> format error
                            break;
                        }
                        AnsiString cpName="", cpVal="";
                        if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                            HGemPtr->DataItemIn(len, Type, cpName);
                        if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                            HGemPtr->DataItemIn(len, Type, cpVal);

                        //AI(ht160s-agv-binsetting) 20260713 : ONLY the Loader has a
                        // host tray-count CP. There is deliberately NO EmptyTrayCount /
                        // ColorTrayCount CP : SVID 38223/38224 (AMR Empty/Color Tray
                        // Count) stay reserved 0. HT9045/KYEC drive Empty/Color purely by
                        // sensor+TrayArm with zero SECS (docs/AGV/HT9045_vs_HT160_SECS_Diff)
                        // and HT160 has no stack-depth counting hardware. Loader is host-
                        // supplied because only it consumes the value (tray-kind tagging).
                        if(sRxDetail!="") sRxDetail = sRxDetail + " ";
                        sRxDetail = sRxDetail + cpName.Trim() + "=" + cpVal.Trim();
                        AnsiString cpN = cpName.Trim().UpperCase();
                        AnsiString cpV = cpVal.Trim().UpperCase();
                        if(cpN=="LOADERTRAYCOUNT")
                            AgvCoord.TrayCount[0] = StrToIntDef(cpVal, 0);  // P1 Loader expected trays
                        //AI(secs-kyec-startagv) 20260728 : LoaderICCount is a REAL KYEC CP, not an
                        // unknown one - the field host sends it on every START_AGV upload packet
                        // (KYEC log 2026-06-08 SECSGEM_TextLog_15.txt:544). It used to fall through
                        // to BeginPrep -> HCACK=2 and the whole handoff was refused. Park it in the
                        // P1 Loader device slot (SVID 38228 AMR Loader Device Count), which had no
                        // writer at all and always reported 0. Latched, not consume-once: unlike
                        // TrayCount[0] (consumed by EnqueueTrip, uAgvStation.cpp:85) nothing acts on
                        // it, it is purely the host-declared incoming IC count for read-back.
                        else if(cpN=="LOADERICCOUNT")
                            AgvCoord.DeviceCount[0] = StrToIntDef(cpVal, 0);
                        else if(AgvCoord.LookupByName(cpName) < 0)
                            HCACK = 2;                           // unknown CP name -> param error
                        //AI(secs-kyec-startagv) 20260728 : the KYEC host sends the FULL station
                        // vector every time, exactly one station carrying "Action" and all the rest
                        // "NA" (SECSGEM_TextLog_15.txt:507-546). "NA" means "listed for vector
                        // completeness, do nothing". HT160 used to ignore the value and BeginPrep
                        // EVERY named station, so one Loader dispatch put up to 8 innocent stations
                        // into AGV_PREP and locked their mechanisms - and the lock has no cheap
                        // release (RetryStation keeps it, AgvCoord.Reset has no runtime caller), so
                        // it ends in WAR0962 = DecStopAllMotor. HT9045 gates on the same literal
                        // (uHGemHT9045.cpp:1676-1709 requires S2=="Action"). Match it.
                        else if(cpV=="ACTION")
                            AgvCoord.BeginPrep(cpName);
                        else if(cpV!="NA")
                        {
                            //AI(secs-kyec-startagv) 20260728 : a known station with a verb we do not
                            // model yet (customer may later add LOAD/UNLOAD/SUPPLY routing, see
                            // docs\AGV\HT160S_E87_AGV_Communication_Draft_20260527.md:299). Do NOT
                            // act and do NOT reject - 9045 would silently no-op it and rejecting the
                            // packet would break a host we cannot re-spec. Log it so the field tells
                            // us what the real verb set is.
                            AnsiString sUnk;
                            sUnk.sprintf("[SECS] START_AGV %s: unmodelled CP value '%s' - no action taken",
                                         cpName.Trim().c_str(), cpVal.Trim().c_str());
                            HGemPtr->StringOut(sUnk);
                            RecordProcess("AGV: START_AGV "+cpName.Trim()+" value '"+cpVal.Trim()+"' not modelled - ignored");
                        }
                    }
                }
            }
            else
            {
                HCACK = 1;                                       // bad list format
            }
        }
        else if(S=="STOP")
        {
            //AI(machine-command-layer) 20260625 : STOP host command (S2F41 RCMD "STOP").
            // Hard stop / remote abort. Route through the single MachineStop() choke point
            // (csystem.cpp) so "SystemStart=false always stops the motors" holds : it
            // clears SystemStart, StopAllMotor(), raises SoftStop and RecordProcess-logs
            // the source. No modal, no UI context needed. (PAUSE only soft-pauses.)
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            MachineStop(trigSecsRemote);
            HCACK = 0;
        }
        else if(S=="HOME")
        {
            //AI(machine-command-layer) 20260625 : HOME host command (S2F41 RCMD "HOME"),
            // equivalent to the operator Home button. Calls the shared TfMain::HomeCore()
            // (main.cpp) -- the sbHome1Click body WITHOUT the "Confirm home?" modal, which
            // would stall the HSMS receive path. Runs on the VCL main thread; the home
            // engine is non-blocking. Gate on fMain (UI context) + not-already-running.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            if(fMain==NULL)
                HCACK = 2;                                       // no UI context -> param error
            else if(HSys.Sys.SystemStart==true)
                HCACK = 4;                                       // already running/homing -> busy
            else
            {
                fMain->HomeCore();
                HCACK = 0;
            }
        }
        else if(S=="ONE_CYCLE")
        {
            //AI(secs-kyec-rcmd4) 20260728 : KYEC host ONE_CYCLE. Field body 2026-06-08,
            // invariant across all 11 occurrences : L[2]{ A[9] "ONE_CYCLE", L[0] } - the
            // parameter list is ALWAYS empty, so it is consumed and ignored.
            // ORDERING NOTE : these four new branches use EXACT == (never AnsiPos) and sit
            // after HOME, immediately before the catch-all else. Verified that no earlier
            // AnsiPos prefix can swallow them - in particular "ONE_CYCLE" does NOT begin with
            // "ONLINE" (O-N-E vs O-N-L). Keep PP_SIGNALTOWER ahead of PP_MUSIC so the longer
            // of the shared "PP_" pair matches first if these are ever converted to AnsiPos.
            // Routed through the factored TfMain::OneCycleCore() (main.cpp) : the
            // sbOneCycle1Click body WITHOUT its two ShowMyMessage modals, which would stall
            // the HSMS receive path. Same precedent as the HOME branch (HomeCore) and the
            // START branch (MachineStart -> HCACK) above.
            // bRequireRunning=true is the SECS-ONLY stale-arm guard : arming Run_OneCycle on
            // a stopped machine is a LATCH nothing clears until the next Start (ProcessMotion
            // early-returns while SystemStart==false), which would silently turn the
            // operator's next Start into one cycle plus an automatic stop.
            //AI(secs-msggap-fix) 20260729 : recounted from the KYEC logs, and the guard is on
            // FIRMER ground than the first draft said. The 11 commands split 3 ACCEPTED /
            // 8 SWALLOWED (not 2/9), and the third acceptance is the smoking gun : ONE_CYCLE at
            // 19:00:19.281 armed (9045 CEID 3 at 19:00:19.306) on a machine that went HALT
            // 0.85 s later, so the arm LATCHED - exactly the failure this guard prevents.
            // 9045 clears its own BtnOneCycle->Down latch at exactly one site, inside the
            // finish handler that then never ran. The retry burst is 9 commands over
            // 8 min 00.3 s at an exact 60.0 s cadence (7 unambiguously in HALT, one on the
            // ART->HALT boundary, the last in Alarm) - "nine times in nine minutes" was loose.
            // DELIBERATE DEVIATION from HT9045, which answers HCACK=0 unconditionally and
            // swallows every rejection silently - its HCACK=0 means "message received", not
            // "cycle accepted". HT160 tells the truth.
            //AI(secs-msggap-fix) 20260729 : do NOT claim this is field-proven host-safe. The
            // "host tolerates refusals and keeps its cadence" evidence is ENERGY_SAVING's
            // (23/23 HCACK=2, zero S9Fx all day) and does not transfer : ONE_CYCLE got HCACK=0
            // on all 11 field occurrences, so a non-zero answer to it has never been on the
            // wire. Treat the host's reaction to HCACK=2/4 here as an ON-MACHINE WATCH ITEM;
            // reproduce the 60 s retry storm on the simulator first.
            //AI(secs-msggap-fix) 20260729 : the empty parameter list IS consumed below, which
            // 9045 does not do at all. Harmless while the body stays L[0] (11/11) and it keeps
            // the token stream clean if a host ever attaches parameters.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            if(fMain==NULL)
            {
                HCACK = 2;                                       // no UI context -> param error
            }
            else
            {
                AnsiString ocReason;
                switch(fMain->OneCycleCore(true, ocReason))
                {
                    case ocStarted:      HCACK = 0; break;   // armed on a running machine
                    case ocRejBusy:      HCACK = 4; break;   // a cycle is already armed and running
                    //AI(secs-audit-fix) 20260729 : machine stopped -> 2 "cannot perform now",
                    //never 4. SEMI E5 HCACK=4 is a POSITIVE ack ("will be performed, completion
                    //signalled later by an event"), and this same change made that promise
                    //checkable by emitting CEID 41 on cycle finish - a stopped machine will
                    //never emit it, so 4 would leave the host waiting forever. This was the
                    //dominant field case: 8 of the 11 KYEC ONE_CYCLE commands on 2026-06-08
                    //arrived while the equipment was reporting HALT.
                    case ocRejStopped:   HCACK = 2; break;   // stopped -> cannot perform now
                    default:             HCACK = 2; break;   // ocRejMode / ocRejNotReady
                }
                sRxDetail = ocReason;
                if(HCACK==0)
                    RecordProcess("ONE CYCLE by secs-remote");
                else
                    RecordProcess("SECS ONE_CYCLE refused : "+ocReason);
            }
        }
        else if(S=="ENERGY_SAVING")
        {
            //AI(secs-kyec-rcmd4) 20260728 : KYEC host ENERGY_SAVING. Field body 2026-06-08,
            // invariant across all 23 occurrences :
            //   L[2]{ A[13] "ENERGY_SAVING", L[1]{ L[2]{ A[5] "STATE", U4[1] 0 } } }
            // STATE always 0 (= leave power save; 1 never seen).
            //AI(secs-msggap-fix) 20260729 : "a ~30-minute heartbeat" is only half the story -
            // 22 beats run 00:03:24 to 10:33:54 at 28.9-30.8 min spacing, then the host STOPS
            // for 4 h 59.6 min, sends one orphan at 15:33:32, and nothing for the last 3 h 49.
            // So do not treat the arrival of this command as a liveness signal.
            // HT-160S HAS NO POWER-SAVE SUBSYSTEM to enter or leave : no heater
            // (DoTemptureControl() is an empty stub), no ATC, no motor-current idle-off.
            // So : VALIDATE the STATE syntax (a malformed packet is still reported as a format
            // error) and then REFUSE with HCACK=2. Deliberately NOT accept-and-no-op - replying
            // 0 would tell the host the machine changed a power state that does not exist.
            // This is byte-identical to what KYEC's OWN HT9045 already answers : 23/23 HCACK=2,
            // after which the host neither escalated (zero S9Fx all day) nor changed cadence.
            // The refusal is field-proven host-safe. It is also strictly better than the
            // unknown-command path : HCACK=1 for a well-formed command reads to a host audit as
            // "command not recognised" rather than "known, not available".
            //AI(secs-msggap-fix) 20260729 : the HCACK=2 itself is directly observed, but which
            // 9045 rung produced it is NOT observable in a SECS log - it is narrowed by
            // elimination to one of two config-level Function Disable rungs (the IC / Contact /
            // ATC-online gates are time-varying, while HCACK stayed 2 across 15.5 h and states
            // SLEEP / Alarm / HALT / Running). No HT160 behaviour depends on which one.
            //AI(secs-kyec-rcmd4-fix) 20260728 : an EMPTY parameter list is a well-formed
            // ENERGY_SAVING, so it must get the same "known command, not available" answer as
            // any other well-formed one. Returning 1 here contradicted this branch's own
            // rationale two paragraphs up (1 = not recognised) and disagreed with the sibling
            // PP_SIGNALTOWER / PP_MUSIC branches, which both accept n<=0 as a valid shape.
            // Only a genuinely unreadable list stays HCACK=1.
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
            {
                HCACK = 1;                                       // body is not a list -> format error
            }
            else if(n<=0)
            {
                HCACK = 2;                                       // well-formed but empty -> known command, not available
            }
            else
            {
                int esPairLen;
                HCACK = 2;                                       // syntax good so far -> "known command, not available"
                for(i=0; i<n; i++)
                {
                    if(HGemPtr->GetDataItemLenAndTypeAndDelete(esPairLen, HType.LIST_TYPE)!=1 || esPairLen!=2)
                    {
                        HCACK = 1;                               // pair is not an L[2] -> format error
                        break;
                    }
                    AnsiString esName="", esValStr="";
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, esName);
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, esValStr);
                    if(sRxDetail!="") sRxDetail = sRxDetail + " ";
                    sRxDetail = sRxDetail + esName.Trim() + "=" + esValStr.Trim();
                    int esState = StrToIntDef(esValStr.Trim(), -1);
                    if(esName.Trim().UpperCase()!="STATE" || esState<0 || esState>1)
                    {
                        HCACK = 1;                               // unknown CP name / value out of domain -> format error
                        break;
                    }
                }
                if(HCACK==2)
                    RecordProcess("SECS ENERGY_SAVING refused : HT-160S has no power-save subsystem");
            }
        }
        else if(S=="PP_SIGNALTOWER")
        {
            //AI(secs-kyec-rcmd4) 20260728 : KYEC host PP_SIGNALTOWER - a LATCHED host override
            // of the per-RunState tower-lamp table (SetSecsTowerOverride / DoSystemMessage,
            // csystem.cpp). Field bodies 2026-06-08, 15 occurrences :
            //   SET   L[2]{ A[14] "PP_SIGNALTOWER", L[3]{ L[2]{A "RED",U4 v},
            //                    L[2]{A "GREEN",U4 v}, L[2]{A "YELLOW",U4 v} } }   (6x)
            //   CLEAR L[2]{ A[14] "PP_SIGNALTOWER", L[0] }                          (9x)
            // Values seen are only 0 and 2, never 1; documented domain 0=off / 1=on / 2=blink.
            // MORE clears than sets, so the clear must be idempotent. Paired with PP_MUSIC
            // ~0.3 s apart (0.256-0.361 s, PP_MUSIC always first).
            //AI(secs-msggap-fix) 20260729 : two corrections to the field reading above.
            // (1) NOT all six SETs are 2/2/2 - five are, and one (19:07:50.526) is
            //     RED=2 GREEN=0 YELLOW=0. The host does use non-uniform per-colour states, so
            //     never collapse the three colours into one value. Per-colour handling below is
            //     load-bearing, not defensive.
            // (2) The trigger is NOT specifically "tester is IDLE, priority lot waiting" - only
            //     3 of the 6 SET bursts follow that text; the others follow "has no schedule.",
            //     "MES_Status Changed toSetUp", and an S10F3 "[ART]User manually abort ART
            //     process.". And two of the nine CLEARs arrive 0.3 s after the equipment's OWN
            //     alarm event report. So this is a GENERIC host attention annunciator that the
            //     host does also use around alarm-ish conditions. The alarm-suppression gates in
            //     csystem.cpp stand on their own safety argument (machine-derived red must win) -
            //     they must NOT be justified by "the host never uses this for alarms".
            // Buffer-then-commit (LOTSTART / SET_LOT_INFO idiom) : an unknown CP name or an
            // out-of-domain value rejects the WHOLE packet and applies NOTHING. HT9045 partially
            // applies and latches the flag anyway - not ported. A colour the host does not name
            // keeps its previous value (-1), matching 9045's write-only-what-is-present.
            // No busy/idle pre-gate : a lamp command is safe in every machine state.
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
            {
                HCACK = 1;                                       // bad list format
            }
            else if(n<=0)
            {
                ClearSecsTowerOverride();                        // empty list -> release (idempotent : 9 clears for 6 sets)
                sRxDetail = "clear";
                RecordProcess("SECS PP_SIGNALTOWER : tower override released (empty parameter list)");
                HCACK = 0;
            }
            else
            {
                int stPairLen;
                int stRed=-1, stYellow=-1, stGreen=-1;           // -1 = colour not named -> keep previous
                HCACK = 0;
                for(i=0; i<n; i++)
                {
                    if(HGemPtr->GetDataItemLenAndTypeAndDelete(stPairLen, HType.LIST_TYPE)!=1 || stPairLen!=2)
                    {
                        HCACK = 1;                               // pair is not an L[2] -> format error
                        break;
                    }
                    AnsiString stName="", stValStr="";
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, stName);
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, stValStr);
                    if(sRxDetail!="") sRxDetail = sRxDetail + " ";
                    sRxDetail = sRxDetail + stName.Trim() + "=" + stValStr.Trim();
                    AnsiString stN = stName.Trim().UpperCase();
                    int stVal = StrToIntDef(stValStr.Trim(), -1);
                    if(stVal<0 || stVal>2)
                    {
                        HCACK = 2;                               // value outside 0..2 -> param error, apply nothing
                        break;
                    }
                    if(stN=="RED")
                        stRed=stVal;
                    else if(stN=="YELLOW")
                        stYellow=stVal;
                    else if(stN=="GREEN")
                        stGreen=stVal;
                    else
                    {
                        HCACK = 2;                               // unknown CP name -> param error, apply nothing
                        break;
                    }
                }
                if(HCACK==0)
                {
                    SetSecsTowerOverride(stRed, stYellow, stGreen);
                    RecordProcess("SECS PP_SIGNALTOWER : tower override armed ("+sRxDetail+")");
                }
            }
        }
        else if(S=="PP_MUSIC")
        {
            //AI(secs-kyec-rcmd4) 20260728 : KYEC host PP_MUSIC - a LATCHED host override of the
            // per-RunState buzzer selection (SetSecsMusicOverride / DoSystemMessage,
            // csystem.cpp). Field bodies 2026-06-08, 15 occurrences :
            //   SET   L[2]{ A[8] "PP_MUSIC", L[1]{ L[2]{ A[0] "", U4[1] 1 } } }   (6x)
            //   CLEAR L[2]{ A[8] "PP_MUSIC", L[0] }                               (9x)
            // *** THE CP NAME IS A ZERO-LENGTH ASCII ITEM A[0] "". *** It is READ - to keep the
            // token stream in sync - and then DISCARDED WITHOUT COMPARISON, exactly as HT9045
            // does. Adding a name check here would reject every real KYEC PP_MUSIC packet; this
            // is the single easiest way to get this command wrong.
            // The A[0] read is proven safe in HT160's own tokenizer : ProcessSML stores three
            // tokens for a zero-length ASCII item, the peek returns 1 at len=0 and DataItemInSub
            // passes the l>len check and deletes all three.
            // Only the VALUE is validated : 1..4 -> SwMusic1..SwMusic4 (database.h:453-456).
            // HT9045 does SW[SwMusic1+CLASS-1].On() with NO range check, so CLASS=0 drives the
            // switch immediately BEFORE SwMusic1 - that latent bug is deliberately NOT ported;
            // out-of-range -> HCACK=2 and SetSecsMusicOverride clamps again defensively.
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
            {
                HCACK = 1;                                       // bad list format
            }
            else if(n<=0)
            {
                ClearSecsMusicOverride();                        // empty list -> release (idempotent)
                sRxDetail = "clear";
                RecordProcess("SECS PP_MUSIC : music override released (empty parameter list)");
                HCACK = 0;
            }
            else
            {
                int pmPairLen;
                int pmClass = -1;
                HCACK = 0;
                for(i=0; i<n; i++)
                {
                    if(HGemPtr->GetDataItemLenAndTypeAndDelete(pmPairLen, HType.LIST_TYPE)!=1 || pmPairLen!=2)
                    {
                        HCACK = 1;                               // pair is not an L[2] -> format error
                        break;
                    }
                    AnsiString pmName="", pmValStr="";
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, pmName);   // A[0] "" at KYEC : read to stay in sync, NEVER compared
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, pmValStr);
                    if(sRxDetail!="") sRxDetail = sRxDetail + " ";
                    sRxDetail = sRxDetail + "class=" + pmValStr.Trim();
                    pmClass = StrToIntDef(pmValStr.Trim(), -1);
                    if(pmClass<1 || pmClass>4)
                    {
                        HCACK = 2;                               // class outside SwMusic1..4 -> param error, apply nothing
                        break;
                    }
                }
                if(HCACK==0 && pmClass>=1)
                {
                    SetSecsMusicOverride(pmClass);
                    RecordProcess("SECS PP_MUSIC : music override armed (class="+IntToStr(pmClass)+")");
                }
            }
        }
        else if(IsTesterOnlyRcmd(S))
        {
            //AI(secs-rcmd-9045) 20260729 : a real HT9045 command for a mechanism HT-160S does
            // not have. Consume the parameter list (so the receive buffer stays in step) and
            // answer 2 "recognised, cannot perform". See IsTesterOnlyRcmd() for the reasoning
            // and for why this is an exact-match list rather than a prefix test.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            HCACK = 2;
            RecordProcess("SECS "+S.Trim()+" refused : tester-only HT9045 command, HT-160S is a sorter");
            HGemPtr->StringOut("[SECS] RCMD "+S.Trim()+" is a tester-only HT9045 command - HCACK=2 (known, not available)");
        }
        else
        {
            HCACK = 1;                                           // unknown command
        }
    }
    else
    {
        HCACK = 1;                                               // bad outer format
    }

    // S2F42 reply : L[2]{ B HCACK, L[0] }
    HGemPtr->InitLocalHead(2, 42, 0);
    HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
    HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
    HGemPtr->SendLocalData();

    {
        AnsiString sLog;
        sLog.sprintf("[SECS] S2F42 cmd=%s HCACK=%u Lots=%d", S.c_str(), (unsigned)HCACK,
                     (int)LotRegistry.GetLotCount());
        HGemPtr->StringOut(sLog);
    }
    AmrInject.NoteHostCommand(S, HCACK, sRxDetail);   // AI(ht160s-agv) 20260720 : every host cmd -> AMR tab (accept/reject + reason + content)
    return 1;
}
//---------------------------------------------------------------------------
void HT160Gem::S1F2_OnLineData()
{
    //AI(ht160s-secsgem) 20260625 : host S1F1 Are-You-There -> reply S1F2 On Line Data.
    //  L[2]{ A:MDLN, A:SOFTREV }. The sMachineType/sSoftwareVersion fields are private on
    //  THGem (no public getter), so emit fixed ASCII literals here - same idiom HT172
    //  uses at its GemInitial call site (literal model string). No clock/side effects.
    if(HGemPtr==NULL)
        return;
    HGemPtr->InitLocalHead(1, 2, 0);
    HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
    HGemPtr->DataItemOut(HType.ASCII_TYPE, AnsiString("HT-160S"));
    HGemPtr->DataItemOut(HType.ASCII_TYPE, AnsiString("1.0.0.0"));
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S1F2 on-line data sent (MDLN=HT-160S)");
}
//---------------------------------------------------------------------------
void HT160Gem::S1F14_ConnectRequestAcknowledge()
{
    //AI(ht160s-secsgem) 20260625 : host S1F13 Establish Communications Request -> reply
    //  S1F14 L[2]{ B:COMMACK, L[2]{ A:MDLN, A:SOFTREV } }. COMMACK=0 (always accept),
    //  matching HT172. MDLN/SOFTREV are ASCII literals (sMachineType/sSoftwareVersion are
    //  private on THGem). No inbound parse needed - HT160 has no abort-on-format option.
    if(HGemPtr==NULL)
        return;
    unsigned char COMMACK = 0;
    HGemPtr->InitLocalHead(1, 14, 0);
    HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &COMMACK);
    HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
    HGemPtr->DataItemOut(HType.ASCII_TYPE, AnsiString("HT-160S"));
    HGemPtr->DataItemOut(HType.ASCII_TYPE, AnsiString("1.0.0.0"));
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S1F14 establish-comm acknowledged (COMMACK=0)");
}
//---------------------------------------------------------------------------
void HT160Gem::S1F18_ONLINEAcknowledge()
{
    //AI(secs-online) 20260724 : host S1F17 Request ONLINE -> reply S1F18 <B ONLACK>. ONLACK=0
    //  (accepted). Also lift the GEM control-state mirror to Online-Remote(5), matching the
    //  existing "ONLINE" host command (S2F41) semantics at S2F42 dispatch. Previously unhandled
    //  -> the S1 dispatch fell through to S9F3 (Unrecognized), so a GEM host could never bring
    //  the tool online via S1F17/F18. Send idiom mirrors S1F14_ConnectRequestAcknowledge.
    if(HGemPtr==NULL)
        return;
    unsigned char ONLACK = 0;
    HGemPtr->InitLocalHead(1, 18, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &ONLACK);
    HGemPtr->SendLocalData();
    iControlState = 5;   //Online-Remote (same target as the ONLINE host command)
    HGemPtr->StringOut("[SECS] S1F18 ONLINE acknowledged (ONLACK=0, control state -> Online-Remote 5)");
}
//---------------------------------------------------------------------------
void HT160Gem::S1F16_OFFLINEAcknowledge()
{
    //AI(secs-offline) 20260727 : host S1F15 Request OFF-LINE -> reply S1F16 <B OFLACK>. OFLACK=0
    //  (accepted). Mirror of S1F18_ONLINEAcknowledge: also drop the GEM control-state mirror to
    //  Off-Line(1). Single-binary reply idiom (InitLocalHead + DataItemOut(BINARY) + SendLocalData).
    //  Previously S1F15 had no Dispatch case -> fell to S9F3, so a GEM host could not take the
    //  tool OFF-LINE. Pairs with the S1F17/F18 online path (commit 21be26a).
    if(HGemPtr==NULL)
        return;
    unsigned char OFLACK = 0;
    HGemPtr->InitLocalHead(1, 16, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &OFLACK);
    HGemPtr->SendLocalData();
    iControlState = 1;   //Equipment Off-Line (GEM control state)
    //AI(secs-kyec-rcmd4-fix) 20260728 : going OFF-LINE means the host has handed the machine back
    //  to the operator, so it must not leave the tower/buzzer driven by a stale host override.
    if(IsSecsPanelOverrideActive())
    {
        ClearSecsPanelOverride();
        HGemPtr->StringOut("[SECS] S1F16 OFF-LINE : host panel override (tower/buzzer) released");
        RecordProcess("SECS: OFF-LINE released host panel override");
    }
    HGemPtr->StringOut("[SECS] S1F16 OFF-LINE acknowledged (OFLACK=0, control state -> Off-Line 1)");
}
//---------------------------------------------------------------------------
//AI(secs-kyec-rcmd4-fix) 20260728 : HSMS link lost (peer disconnect / socket error /
//  Separate.req / watchdog DropConnection). KYEC arms PP_SIGNALTOWER + PP_MUSIC and clears them
//  seconds later with an empty parameter list; if the link drops in between, the operator is left
//  with a tower and buzzer nobody can release except the panel ALARM RESET key - which arrives
//  over the Pad RS232 link (SnFKAlarmReset/SnRKAlarmReset are COMM_PAD), so a Pad outage would
//  mean no escape at all. Release the latch with the host that set it.
void HT160Gem::OnCommunicationLost()
{
    if(IsSecsPanelOverrideActive()==false)
        return;
    ClearSecsPanelOverride();
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] link lost : host panel override (tower/buzzer) released");
    RecordProcess("SECS: link lost released host panel override");
}
//---------------------------------------------------------------------------
void HT160Gem::S2F34_DefineReportAcknowledge()
{
    //AI(secs-reportdef) 20260724 : Find* are private on THGem -> body lives there; delegate.
    if(HGemPtr!=NULL)
        HGemPtr->ProcessDefineReport_S2F33();
}
//---------------------------------------------------------------------------
void HT160Gem::S2F36_LinkEventReportAcknowledge()
{
    if(HGemPtr!=NULL)
        HGemPtr->ProcessLinkEventReport_S2F35();
}
//---------------------------------------------------------------------------
void HT160Gem::S2F38_EnableDisableEventReportAcknowledge()
{
    if(HGemPtr!=NULL)
        HGemPtr->ProcessEnableDisableEventReport_S2F37();
}
//---------------------------------------------------------------------------
void HT160Gem::S2F32_DateAndTimeAcknowledge()
{
    //AI(ht160s-secsgem) 20260625 : host S2F31 Date and Time Set Request -> reply S2F32 TIACK.
    //  TIACK=0 (accepted). MINIMAL port: acknowledge ONLY - deliberately does NOT write the
    //  equipment OS clock (HT172 settime/setdate is excluded; setting the PC clock from a host
    //  message is a side-effect that needs an explicit product decision + on-machine verify).
    //  Single-byte reply via the proven S5F4 triad (HT160 has no LocalAcknowledge wrapper).
    if(HGemPtr==NULL)
        return;
    unsigned char TIACK = 0;
    HGemPtr->InitLocalHead(2, 32, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &TIACK);
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S2F32 date/time acknowledged (TIACK=0, clock NOT set)");
}
//---------------------------------------------------------------------------
void HT160Gem::S2F18_DateandTimeData()
{
    //AI(secs-gem-std) 20260727 : host S2F17 Date and Time Request -> reply S2F18 <A TIME>.
    //  16-char SEMI E5 TIME "YYYYMMDDhhmmsscc" (cc=centiseconds, fixed "00") from the equipment
    //  clock. READ-ONLY : reports current time, sets nothing (the write side stays S2F31->S2F32
    //  ack-only). Previously routed to the base SendUnsupported stub -> host S2F17 T3-timed-out.
    if(HGemPtr==NULL)
        return;
    AnsiString sTime = Now().FormatString("yyyymmddhhnnss") + "00";
    HGemPtr->InitLocalHead(2, 18, 0);
    HGemPtr->DataItemOut(HType.ASCII_TYPE, sTime);
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S2F18 date/time data sent (" + sTime + ")");
}
//---------------------------------------------------------------------------
void HT160Gem::S2F26_DiagnosticLoopbackData()
{
    //AI(secs-gem-std) 20260727 : host S2F25 Loopback Diagnostic Request -> reply S2F26 echoing the
    //  received binary (ABS) back unchanged - the standard HSMS link diagnostic. Mirrors HT9045
    //  S2F26. HT160 has no S9F7 primitive, so a malformed request is logged (no reply) rather than
    //  an on-wire S9F7. Previously routed to the base SendUnsupported stub -> host S2F25 T3-timeout.
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    int len = 0;
    unsigned char Type = 0;
    if(HGemPtr->GetDataItemLenAndType(len, Type)==1 && Type==HType.BINARY_TYPE && len>=0 && len<=4096)
    {
        unsigned char *Temp = new unsigned char[len+1];
        if(HGemPtr->DataItemIn(len, Type, Temp)==1)
        {
            HGemPtr->InitLocalHead(2, 26, 0);
            HGemPtr->DataItemOut(len, HType.BINARY_TYPE, Temp);
            HGemPtr->SendLocalData();
            HGemPtr->StringOut("[SECS] S2F26 loopback echoed (len=" + IntToStr(len) + ")");
        }
        else
            HGemPtr->StringOut("[SECS] S2F25 loopback read failed - no echo");
        delete[] Temp;
    }
    else
        HGemPtr->StringOut("[SECS] S2F25 loopback format error (not BINARY) - no echo");
}
//---------------------------------------------------------------------------
void HT160Gem::S5F4_EnableDisableAlarmAcknowledge()
{
    //AI(ht160s-secsgem) 20260625 : host S5F3 Enable/Disable Alarm Send -> reply S5F4
    //  ACKC5=0 so the host's W-bit alarm-enable request does not T3-timeout. HT160
    //  keeps alarms always-enabled for now (per-ALID enable/disable is a follow-up).
    if(HGemPtr==NULL)
        return;
    unsigned char ACKC5 = 0;
    HGemPtr->InitLocalHead(5, 4, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &ACKC5);
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S5F4 alarm enable/disable acknowledged (ACKC5=0)");
}
//---------------------------------------------------------------------------
void HT160Gem::S5F6_ListAlarmData()
{
    //AI(ht160s-secsgem) 20260715 : reply to S5F5 (List Alarm Request) with the real alarm
    // catalog built live from HSys.mapAlarmCodeList (the SSOT). See EmitAlarmCatalog.
    EmitAlarmCatalog(6);
}
//---------------------------------------------------------------------------
void HT160Gem::S5F8_ListEnableAlarmAcknowledge()
{
    //AI(ht160s-secsgem) 20260715 : reply to S5F7 (List Enabled Alarm Request) with S5F8.
    // HT160 keeps every registered alarm enabled (no per-ALID enable/disable), so the
    // enabled-alarm list is identical to the full catalog.
    EmitAlarmCatalog(8);
}
//---------------------------------------------------------------------------
void HT160Gem::EmitAlarmCatalog(int Func)
{
    //AI(ht160s-secsgem) 20260715 : emit an alarm catalog as S5F<Func> from the SSOT map.
    // Body: L[n]{ L[3]{ B ALCD, U4 ALID, A ALTX } }. ALCD = alarm category (AlarmType
    // low 7 bits; the set/clear bit 7 is 0 for a definition listing); ALID = ComputeAlarmAlid()
    // so it matches the S5F1 report ALID; ALTX = "<code> <english message>".
    // NOTE: the S5F5/S5F7 ALID filter is NOT applied - the full catalog is always returned
    // (a safe superset; catalog requests normally send L,0 = all).
    if(HGemPtr==NULL)
        return;
    int n = (int)HSys.mapAlarmCodeList.size();
    HGemPtr->InitLocalHead(5, Func, 0);
    HGemPtr->DataItemOut(n, HType.LIST_TYPE, NULL);
    std::map<AnsiString, MyAlarmCodeStruct>::iterator it;
    for(it=HSys.mapAlarmCodeList.begin(); it!=HSys.mapAlarmCodeList.end(); it++)
    {
        unsigned char alcd  = (unsigned char)(it->second.AlarmType & 0x7F);
        unsigned      uAlid = ComputeAlarmAlid(it->second.AlarmCode);
        AnsiString    altx  = it->second.AlarmCode;
        if(it->second.E_ErrMessage!="")
            altx = it->second.AlarmCode + " " + it->second.E_ErrMessage;
        HGemPtr->DataItemOut(3, HType.LIST_TYPE, NULL);
        HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &alcd);
        HGemPtr->DataItemOut(1, HType.UINT_4_TYPE, &uAlid);
        HGemPtr->DataItemOut(HType.ASCII_TYPE, altx);
    }
    HGemPtr->SendLocalData();
    AnsiString msg;
    msg.sprintf("[SECS][TX] S5F%d alarm catalog sent (%d entries)", Func, n);
    HGemPtr->StringOut(msg);
}
//---------------------------------------------------------------------------
void HT160Gem::S6F16_EventReportData()
{
    //AI(secs-msggap) 20260728 : host S6F15 Event Report Request -> reply S6F16.
    //  KYEC sends a BARE <U4[1] CEID> (verified in all 3 occurrences), not an L,1 wrapper,
    //  so read one scalar directly. An optional L,1 wrapper is tolerated for other hosts.
    //  ResetReturnCode() FIRST : iReturnCode is sticky (GetDataItemLenAndType only ever
    //  lowers it), so a previous message's read failure would otherwise poison this one.
    //  Same first line as S2F26 / S2F33 / S2F37.
    //  DATAID is 1 : hardcoded in HT9045, matches the wire, and matches the global
    //  EventReport(unsigned) glue which calls HGem->EventReport(1, Ceid).
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    int len = 0;
    unsigned char Type = 0;
    AnsiString sTmp;
    //AI(secs-msggap-fix) 20260728 : NEVER return without sending. A silent return is the exact
    //  failure this whole patch exists to remove - the host just T3s again. Every parse failure
    //  now falls through to EmitEventReportBody with CEID 0, which is a well-formed
    //  L,3{ DATAID, CEID=0, L,0 } (FindCEIDItem returns NULL -> reportCount 0), i.e. "no such
    //  event". Reachable without a malformed host: DataItemIn(AnsiString&) has no BINARY /
    //  BOOLEAN / U8 / F4 / F8 branch and rejects Len!=1 for numerics, so a host encoding the
    //  CEID as <U8[1]> or <U4[2]> would otherwise get total silence.
    unsigned iCeid = 0;
    if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
    {
        HGemPtr->StringOut("[SECS] S6F15 format error (empty body) - replying S6F16 CEID=0");
    }
    else
    {
        bool bReadable = true;
        if(Type==HType.LIST_TYPE)
        {
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(len, HType.LIST_TYPE)!=1 ||
               HGemPtr->GetDataItemLenAndType(len, Type)!=1)
            {
                HGemPtr->StringOut("[SECS] S6F15 format error (bad list wrapper) - replying S6F16 CEID=0");
                bReadable = false;
            }
        }
        if(bReadable)
        {
            if(HGemPtr->DataItemIn(len, Type, sTmp)!=1)
                HGemPtr->StringOut("[SECS] S6F15 format error (CEID unreadable) - replying S6F16 CEID=0");
            else
                iCeid = (unsigned)strtoul(sTmp.c_str(), NULL, 10);   //saturates at ULONG_MAX; atoi would wrap a U4 >= 2^31
        }
    }
    HGemPtr->EmitEventReportBody(16, 1, iCeid);
}
//---------------------------------------------------------------------------
void HT160Gem::S6F20_IndividualReportData()
{
    //AI(secs-msggap) 20260728 : host S6F19 Individual Report Request -> reply S6F20.
    //  KYEC sends a BARE <U4[1] RPTID> (verified in all 8 occurrences: 700 x3, 600 x3,
    //  506 x2). The answer is a flat list of that report's SV values read from the LIVE
    //  registry, so a host that deletes and redefines the RPTID mid-campaign - KYEC does
    //  exactly this to 506, L[5] in one session and L[6] in another - gets the new shape.
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    int len = 0;
    unsigned char Type = 0;
    AnsiString sTmp;
    //AI(secs-msggap-fix) 20260728 : same rule as S6F16 - never return without sending. RPTID 0
    //  is not defined, so EmitIndividualReport answers the well-formed empty L,0 that S6F20
    //  already uses for an unknown report. See the S6F16 comment for why this is reachable
    //  without a malformed host.
    unsigned uRpt = 0;
    if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
    {
        HGemPtr->StringOut("[SECS] S6F19 format error (empty body) - replying S6F20 L,0");
    }
    else
    {
        bool bReadable = true;
        if(Type==HType.LIST_TYPE)
        {
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(len, HType.LIST_TYPE)!=1 ||
               HGemPtr->GetDataItemLenAndType(len, Type)!=1)
            {
                HGemPtr->StringOut("[SECS] S6F19 format error (bad list wrapper) - replying S6F20 L,0");
                bReadable = false;
            }
        }
        if(bReadable)
        {
            if(HGemPtr->DataItemIn(len, Type, sTmp)!=1)
                HGemPtr->StringOut("[SECS] S6F19 format error (RPTID unreadable) - replying S6F20 L,0");
            else
                uRpt = (unsigned)strtoul(sTmp.c_str(), NULL, 10);   //saturates; atoi would wrap a U4 >= 2^31
        }
    }
    HGemPtr->EmitIndividualReport(uRpt);
}
//---------------------------------------------------------------------------
//AI(secs-msggap) 20260728 : shared sink for host terminal text (S10F3 and S10F5).
//  Writes one SECS log line, one EventLog CSV row, and queues the raw text on
//  HTGem::SecsAlarmMessage - a TStringList that already exists (allocated in all three
//  HTGem constructors, freed in the dtor) and until now had ZERO consumers.
//  It NEVER pops a dialog. The caller runs inside the HSMS receive callback
//  (THGem::HandleDataMessage -> GemLogic->Dispatch), and every HT160 message box is a
//  ShowModal that suspends MainProc, so a popup here would stall SECS itself.
//  Deliberately NOT classified by severity: the host text cannot be judged in firmware.
//  Substring matching is provably broken on this very corpus - the purely informational
//  "[MIScheduleSetAlarm]|HP93K-1042EXA is IDLE now." contains the word "Alarm", while the
//  real action text "[OMS1] Criteria Check Alarm" arrives with TID=0x00 in one capture and
//  TID=0x01 in another, and informational texts also arrive with TID=0x01.
//AI(secs-msggap-fix) 20260729 : TID is not a severity LEVEL, but it is not meaningless either -
//  9045 consumes it as iSECSMessageCanCloseByOperator and its message box keys the dismissal
//  gate off it (0 = normal flow, 1 = operator may close directly, 2 = employee-ID check). Read
//  that way this host's usage is mostly coherent with one off-pattern text. So : it is a
//  DISMISSAL-POLICY flag, not a severity rank, and this host's use of it is not fully
//  self-consistent - which is why nothing here branches on it. Whoever implements the phase-2
//  operator popup must honour it as a dismissal gate, not sort by it.
//  Everything is logged identically; the operator reads it.
//  CR/LF are flattened for the log/CSV copy (one row per message); the queued copy keeps
//  its line breaks for a future popup.
//---------------------------------------------------------------------------
static void SinkHostTerminalText(THGem *Gem, TStringList *Queue, int F,
                                 unsigned char TID, AnsiString sText, unsigned char ACKC10)
{
    AnsiString sFlat = sText;
    for(int i=1; i<=sFlat.Length(); i++)
    {
        if(sFlat[i]=='\r' || sFlat[i]=='\n')
            sFlat[i] = ' ';
    }
    AnsiString sMsg;
    sMsg.sprintf("S10F%d TID=%u ACKC10=%u : %s",
                 F, (unsigned)TID, (unsigned)ACKC10, sFlat.c_str());
    if(Gem!=NULL)
    {
        Gem->StringOut("[SECS] host terminal text " + sMsg);
        Gem->FlushSecsLogToFile();
    }
    g_EventLog.Log("SECS_TERM", sMsg, "");
    if(Queue!=NULL && sText!="")
    {
        Queue->Add(sText);
        while(Queue->Count > 50)   // bounded : nothing drains this in phase 1
            Queue->Delete(0);
    }
}
//---------------------------------------------------------------------------
void HT160Gem::S10F4_TerminalDisplaySingleAcknowledge()
{
    //AI(secs-msggap) 20260728 : host S10F3 Terminal Display Single -> reply S10F4 <B ACKC10>.
    //  Body: L,2{ B[1] TID, A[n] TEXT }. Bare binary reply, no list wrapper - same single-byte
    //  triad as S5F4 / S1F16 / S1F18 / S2F32 (HT160 has no LocalAcknowledge wrapper).
    //  The Dispatch case already existed but landed on the base SendUnsupported stub, which
    //  only writes a log line - that is why the host T3s today.
    //  ACKC10 per SEMI E5: 0=accepted, 1=will not be displayed, 2=terminal not available.
    //  0 on a clean parse (the text IS accepted - logged, CSV'd and queued), 1 on any format
    //  error. 2 is not used: HT9045 answered 0 every single time at KYEC, so 0 is the
    //  field-proven answer.
    //  TEXT READ - safety critical: the literal HT9045 form for THIS function is
    //  `char str[1024]` plus DataItemIn(1024, ASCII_TYPE, str). That OVERFLOWS on HT160, because
    //  DataItemInSub only guards `if(l>len) return -2;` and then does strncpy(temp, src, len+1)
    //  - 1025 bytes into a 1024 buffer. The safe form is to peek the REAL length with
    //  GetDataItemLenAndType and hand it to the AnsiString overload, which allocates
    //  new char[Len+4] and null-terminates at [Len]. No fixed buffer at all.
    //AI(secs-msggap-fix) 20260729 : do NOT generalise that 1-byte-overrun description to S10F6.
    //  9045's S10F6 is WORSE : it peeks the request's own length and passes it straight to
    //  DataItemIn with no 1024 clamp at all, so a verbatim port there is an unbounded stack
    //  smash, not an off-by-one. Both HT160 handlers use the AnsiString overload, so neither is
    //  exposed - but a maintainer reading only the paragraph above would "fix" S10F6 by capping
    //  at 1024, which is not even the defect.
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    unsigned char TID    = 0;
    unsigned char ACKC10 = 0;
    int len = 0;
    unsigned char Type = 0;
    AnsiString sText;
    if(HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL)!=1)
        ACKC10 = 1;
    else if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
        ACKC10 = 1;
    else if(Type!=HType.BINARY_TYPE && Type!=HType.UINT_1_TYPE)
        ACKC10 = 1;
    else if(HGemPtr->DataItemIn(1, Type, &TID)!=1)
        ACKC10 = 1;
    else if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
        ACKC10 = 1;
    else if(Type!=HType.ASCII_TYPE || len<0 || len>4096)
        ACKC10 = 1;
    else if(HGemPtr->DataItemIn(len, Type, sText)!=1)
        ACKC10 = 1;

    HGemPtr->InitLocalHead(10, 4, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &ACKC10);
    HGemPtr->SendLocalData();

    SinkHostTerminalText(HGemPtr, SecsAlarmMessage, 3, TID, sText, ACKC10);
}
//---------------------------------------------------------------------------
void HT160Gem::S10F6_TerminalDisplayMultiBlockAcknowledge()
{
    //AI(secs-msggap) 20260728 : host S10F5 Terminal Display Multi-block -> reply S10F6.
    //  Body: L,2{ B[1] TID, L,n{ A TEXT ... } }, n observed 1..3 across the 7 KYEC captures.
    //  Lines are joined with CRLF exactly like HT9045, but read via the AnsiString overload
    //  (see the buffer-overflow note in S10F4). Reply is the same bare <B ACKC10>.
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    unsigned char TID    = 0;
    unsigned char ACKC10 = 0;
    int n = 0, len = 0, i = 0;
    unsigned char Type = 0;
    AnsiString sLine;
    AnsiString sText;
    if(HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL)!=1)
        ACKC10 = 1;
    else if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
        ACKC10 = 1;
    else if(Type!=HType.BINARY_TYPE && Type!=HType.UINT_1_TYPE)
        ACKC10 = 1;
    else if(HGemPtr->DataItemIn(1, Type, &TID)!=1)
        ACKC10 = 1;
    else if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1 || n<0 || n>64)
        ACKC10 = 1;
    else
    {
        for(i=0; i<n; i++)
        {
            if(HGemPtr->GetDataItemLenAndType(len, Type)!=1 ||
               Type!=HType.ASCII_TYPE || len<0 || len>4096 ||
               HGemPtr->DataItemIn(len, Type, sLine)!=1)
            {
                ACKC10 = 1;
                break;
            }
            if(sText.Length() < 8192)
                sText = sText + sLine + "\r\n";
        }
    }

    HGemPtr->InitLocalHead(10, 6, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &ACKC10);
    HGemPtr->SendLocalData();

    SinkHostTerminalText(HGemPtr, SecsAlarmMessage, 5, TID, sText, ACKC10);
}
//---------------------------------------------------------------------------
void HT160Gem::S125F2_EnableDisableECDataAcknowledge()
{
    //AI(secs-msggap) 20260728 : host S125F1 Enable/Disable EC Data Send (KYEC private stream)
    //  -> reply exactly ONE S125F2 <B ACK>. Body: L,2{ B[1] ALED, L,n{ U4 ECID ... } }.
    //  KYEC sends a pair per session: ALED=0x01 with L,0 (disable all), then ALED=0x80 with
    //  L,45. ALED is a BIT-7 flag on this host, not a boolean - HT9045 tests `T & 0x80`, which
    //  is why 0x01 means DISABLE despite looking like a true.
    //  Nothing is stored. The only thing the EC-enable list gates on HT9045 is whether an EC
    //  change fires CEID 48, and HT160 registers no such CEID and has no EC-enable table, so
    //  an enable table here would be dead weight. The referenced ECIDs are logged instead,
    //  the same referenced-set discovery logging S2F33 does for unknown SVIDs.
    //  ACK 0=Acknowledge, 1=Denied. Exactly ONE reply per request - see the Dispatch comment
    //  for why HT9045's 45-acks-per-request loop is a defect and is not ported.
    //AI(secs-msggap-fix) 20260729 : ACK=1 is also returned when the list names an ECID this
    //  machine does not register, so a 0 never claims an enable that cannot happen. See the
    //  in-loop comment for the 9045 and KYEC-wire evidence.
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    unsigned char ALED = 0;
    unsigned char ACK  = 0;
    int n = 0, len = 0, i = 0;
    int nUnknownEc = 0;                  //AI(secs-msggap-fix) 20260729 : ECIDs this machine does not register
    unsigned char Type = 0;
    AnsiString sTmp;
    AnsiString sIds;
    AnsiString sUnknownIds;
    if(HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL)!=1)
        ACK = 1;
    else if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
        ACK = 1;
    else if(Type!=HType.BINARY_TYPE && Type!=HType.UINT_1_TYPE)
        ACK = 1;
    else if(HGemPtr->DataItemIn(1, Type, &ALED)!=1)
        ACK = 1;
    else if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1 || n<0 || n>256)
        ACK = 1;
    else
    {
        for(i=0; i<n; i++)
        {
            if(HGemPtr->GetDataItemLenAndType(len, Type)!=1 ||
               HGemPtr->DataItemIn(len, Type, sTmp)!=1)
            {
                ACK = 1;
                break;
            }
            if(i<32)
            {
                if(sIds!="")
                    sIds = sIds + ",";
                sIds = sIds + sTmp;
            }
            //AI(secs-msggap-fix) 20260729 : do not ACK=0 an ECID this machine does not have.
            //A 0 tells the host "EC-change reporting is now enabled for that ECID" - it is not,
            //and with no EC-enable table and no EC-change event on HT160 it never will be.
            //HT9045 answers 1 per unregistered ECID (uHGemClass.cpp:2650-2653), wire-proven at
            //KYEC by the four 0x01 acks at list positions 7/8/10/45 = ECIDs 2101/2102/2004/8506,
            //the exact four missing from the whole 9045 tree. HT160 sends ONE ack per request, so
            //the honest compression is : any unregistered ECID -> ACK=1 for the request, and name
            //the offenders in the log. KYEC's list is 45 ECIDs of which HT160 owns exactly one
            //(1501), so this normally answers 1 - which is the truth, where 0 was a lie.
            //GetECName() is used instead of the private FindECItem(): it returns "" for an
            //unregistered ECID, and all seven ECs this machine registers have a non-empty name.
            if(HGemPtr->GetECName((unsigned)strtoul(sTmp.c_str(), NULL, 10))=="")
            {
                nUnknownEc++;
                if(nUnknownEc<=32)
                {
                    if(sUnknownIds!="")
                        sUnknownIds = sUnknownIds + ",";
                    sUnknownIds = sUnknownIds + sTmp;
                }
            }
        }
        if(ACK==0 && nUnknownEc>0)
            ACK = 1;                                 // list references ECIDs this machine has no EC for
    }

    HGemPtr->InitLocalHead(125, 2, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &ACK);
    HGemPtr->SendLocalData();

    const char *pMode = "disable";
    if((ALED & 0x80)!=0)
        pMode = "enable";
    AnsiString sLog;
    sLog.sprintf("[SECS] S125F2 ACK=%u ALED=0x%02X (%s) ECIDcount=%d notRegistered=%d",
                 (unsigned)ACK, (unsigned)ALED, pMode, n, nUnknownEc);
    HGemPtr->StringOut(sLog);
    if(sIds!="")
        HGemPtr->StringOut("[SECS] S125F1 referenced ECIDs (first 32): " + sIds);
    //AI(secs-msggap-fix) 20260729 : the not-registered set is the discovery list - it is what the
    //host expects this machine to report EC changes for and what a future EC port would add.
    if(sUnknownIds!="")
        HGemPtr->StringOut("[SECS] S125F1 ECIDs NOT registered here (first 32): " + sUnknownIds);
    HGemPtr->FlushSecsLogToFile();
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
