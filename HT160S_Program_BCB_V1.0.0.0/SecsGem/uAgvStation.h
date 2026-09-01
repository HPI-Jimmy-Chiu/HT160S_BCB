//---------------------------------------------------------------------------
// uAgvStation.h
// AI(ht160s-agv) 20260615 : E87/AGV station coordinator.
// Owns the P1-P9 station table (PIndex = AutoNo + 3), the SVID-bound snapshot
// data (carrier id / tray & device counts / P-bitmaps), and the FSM-free
// per-station handshake state. Spec:
//   docs/AGV/HT160S_E87_AGV_Communication_Draft_20260527.md
// Layering: this object OWNS the AGV concern; HT160Gem only registers pointers
// to the snapshot members here (SetSVDataPointer). No FSM framework (HT160 rule):
// the handshake is a plain per-station status byte advanced by a switch.
//---------------------------------------------------------------------------
#ifndef uAgvStationH
#define uAgvStationH

#include <vcl.h>   // AnsiString

class THGem;        // AI(ht160s-agv) 20260615 : SECS transport for firing S6F11

#define AGV_STATION_COUNT 9
#define AGV_AUTO_COUNT    6

// station kind (table dispatch key)
enum eAgvStationKind { ASK_LOADER = 0, ASK_EMPTY = 1, ASK_COLOR = 2, ASK_AUTO = 3 };

// per-station latched handshake status (replaces an FSM with a small enum)
enum eAgvHandshake { AGV_IDLE = 0, AGV_CALLED = 1, AGV_PREP = 2, AGV_READY = 3, AGV_FINISH = 4 };

// AI(ht160s-agv-identity2d) 20260714 : SINGLE change-point for the identity-tray carrier SVID/station.
// The identity 2D is uploaded via CEID275 with AgvStation[AMR_IDENTITY_CARRIER_INDEX].SvidCarrierID -
// both the stamped CarrierID[] index and report 7's SVID derive from this one constant, so a single
// edit keeps them locked together. 0 = Loader (P1, SVID 38202), 2 = Color (P3, SVID 38204).
//AI(secs-identity2d-38202) 20260805 : moved 2 -> 0 on the customer's ruling. It was 2 because Color
// is the station that PHYSICALLY reads the 2D, but KYEC wants the value on HT9045's number: their
// EAP's RPTID 2000 binds 38202 "Load port carrier ID" as its first field, so with the value on 38204
// that field had always come back <A[0]>. Alignment with the 9045 dictionary outranks matching our
// own mechanical layout here; the mechanical fact (Color CCD does the reading) is documented in the
// customer workbook's SVID sheet instead. aColor still calls ReportLoaderIdentity with this constant,
// so the reading station is unchanged - only the SVID the value is published on moves.
// NOTE the consequence: SVID 38204 Color Carrier ID now has no writer at all. That is intended.
#define AMR_IDENTITY_CARRIER_INDEX 0

struct TAgvStationDesc
{
    int         PIndex;        // 1..9
    int         Kind;          // eAgvStationKind
    int         AutoIndex;     // 0..5 for ASK_AUTO, else -1
    unsigned    SvidCarrierID; // 38202/38203/38204 + Auto1-3 38205-38207 (810) + Auto4-6 38199-38201 (899)
    unsigned    SvidTrayCount; // draft section 6 (non-contiguous past Auto3)
    unsigned    SvidDeviceCnt; // ditto
    unsigned    SvidBinSet;    // ASCII bin setting (Auto only; 0 if none)
    //AI(amr-lane-lotno) 20260831 : lot number this lane sorts FOR (Auto only; 0 = no SVID).
    // Customer re-opened the private 66xxx band at 66040 for it - see the uHGemHT160 tombstone.
    unsigned    SvidLotNo;     // 66040-66045 (Auto1-6); 0 on P1-P3
    const char *Name;          // "Loader"/"Empty"/"Color"/"AUTO1".."AUTO6"
};

class TAgvCoordinator
{
public:
    // --- SVID-bound snapshot data (stable addresses for SetSVDataPointer) ---
    AnsiString SupplementBitmap;                 // 38219  set at CEID272 fire time
    AnsiString StatusBitmap;                     // 38220  set at CEID273 fire time
    AnsiString FinishBitmap;                     // 38221  set at CEID274 fire time
    AnsiString CarrierID[AGV_STATION_COUNT];     // 38202-38207 + 38199-38201 (index P-1; see AgvStation[])
    // AI(ht160s-agv-binsetting) 20260713 : TrayCount[0]=Loader is host-supplied
    // (START_AGV LoaderTrayCount CP) because only the Loader consumes it (tray-kind
    // boundary tagging). [1]/[2] (Empty/Color, SVID 38223/38224) stay RESERVED 0 :
    // AI(secs-comment-truth) 20260805 : the reserved-0 set is WIDER than the two SVIDs named
    // below - DeviceCount[1]/[2] (SVID 38229/38230) are reserved for the same reason, and
    // CarrierID[1] (SVID 38203 Empty Carrier ID) is never written either.
    //AI(secs-identity2d-38202) 20260805 : CarrierID[2] (SVID 38204 Color Carrier ID) joined that
    // list today - the identity tray's 2D moved to CarrierID[0] / SVID 38202 to match HT9045, so
    // nothing writes index 2 any more. Full list of registered-but-never-maintained AMR SVIDs:
    // 38203 / 38204 / 38223 / 38224 / 38229 / 38230.
    // HT9045/KYEC drive Empty/Color purely by sensor+TrayArm with zero SECS, and
    // HT160 has no stack-depth counting hardware (only a present/empty InputEnd
    // sensor). [3..8]=Auto are refreshed live from TMyCar. See uHGemHT160 START_AGV.
    int        TrayCount[AGV_STATION_COUNT];     // (index P-1)
    int        DeviceCount[AGV_STATION_COUNT];   // (index P-1)
    AnsiString BinSetting[AGV_AUTO_COUNT];       // Auto1..6 (index AutoIndex)
    //AI(amr-lane-lotno) 20260831 : 66040-66045, the lot number each Auto lane is sorting for.
    // Same value the main screen shows in plLotNumberAuto1..6, read from the same source
    // (LotBinBinding reverse lookup), NOT from the panel caption. Empty when the lane has no
    // lot bound (smNormal routes by bin only). Refreshed every tick, ungated by bUseAMR.
    AnsiString LotNumber[AGV_AUTO_COUNT];        // Auto1..6 (index AutoIndex)

    // --- per-station runtime handshake state (Phase B/C/D) ---
    unsigned char Handshake[AGV_STATION_COUNT];      // eAgvHandshake
    unsigned char PrepDone[AGV_STATION_COUNT];
    unsigned char ShortageLatch[AGV_STATION_COUNT];  // one-shot: fire CEID272 once
    int           ShortageDebounce[AGV_STATION_COUNT];  // AI(ht160s-agv) 20260625 : per-station PREP/READY age (ServiceHandshake watchdog ticks)
    unsigned char ReadyEntrySensor[AGV_STATION_COUNT]; // edge baseline for Finish
    //AI(amr-unmanned W3) 20260721 : one-shot per-station "AGV handshake timed out" latch.
    //Set by the ServiceHandshake/PollAndCall aging (CALLED+PREP+READY vs iAgvTimeoutSec)
    //for P2-P9 (Empty/Color supply + Auto collect; P1 Loader excluded - its timeout is the
    //S4 source-dry auto-CleanOut). CONSUMED by the MAIN control loop (csystem) which pops
    //WAR0962 there - never a modal on this SECS-timer path. Cleared by RetryStation/Reset.
    unsigned char TimeoutPending[AGV_STATION_COUNT];
    //AI(agv-linklost-hold) 20260819 : per-station HSMS-link-lost hold. A dropped link used to
    //release every in-progress lock outright; it now HOLDS them (a TCP drop is not evidence the
    //AMR left, and the machine has no AMR-presence input). LinkLostAge counts 1s coordinator
    //ticks while the link is down AND this station still holds its module lock; past
    //iAgvTimeoutSec it latches LinkLostPending once. CONSUMED by the MAIN loop (csystem), which
    //pops WAR0963 - never a modal on this SECS-timer path. This escape deliberately depends on
    //NOTHING that stops working while disconnected, so a held lock is always bounded.
    int           LinkLostAge[AGV_STATION_COUNT];
    unsigned char LinkLostPending[AGV_STATION_COUNT];

    TAgvCoordinator();
    void Reset();
    void RetryStation(int si);   //AI(amr-unmanned W3) 20260721 : WAR0962 K_RETRY -> station back to IDLE (+unlock kept? no: keep lock semantics, see .cpp) so PollAndCall re-CALLs
    //AI(agv-linklost-hold) 20260819 : link-lost hold helpers. ServiceLinkLostHold ages the hold
    //(called only from the disconnected branch of PollAndCall); ClearLinkLostHold drops the
    //counters on reconnect WITHOUT touching lock or handshake; ReleaseStationByOperator is the
    //WAR0963 answer - the one path allowed to release a held lock without SECS evidence, and it
    //MUST also clear Handshake or ReassertLocks would silently re-couple the lock after a HOME.
    //IsStationLockHeld asks the module (not Handshake) so a RetryStation orphan is covered too.
    void ServiceLinkLostHold();
    void ClearLinkLostHold();
    void ReleaseStationByOperator(int si);
    bool IsStationLockHeld(int si);
    void ReleaseInfeedForCleanOut();   //shared by the Clean Out path, reachable while disconnected

    AnsiString BuildBitmap(int targetPIndex);    // "P1:0,...,Px:1,...,P9:0"
    int        LookupByName(AnsiString cpName);  // -> index 0..8, -1 if unknown

    // Phase B/C/D entry points (Gem = SECS transport used to fire S6F11):
    void PollAndCall(THGem *Gem);                // Phase B : shortage/full -> CEID272
    void ServiceHandshake(THGem *Gem);           // Phase D : drive CEID273 / CEID274
    void ReportLoaderIdentity(THGem *Gem, int stationIndex, AnsiString id2D); //AI(ht160s-agv-identity2d) 20260714 : S6F11 CEID275 AGVLdID; SVID = AgvStation[stationIndex].SvidCarrierID (identity-tray 2D)
    bool BeginPrep(AnsiString cpName);           // Phase C : START_AGV -> prep
    void ReassertLocks();                        //AI(ht160s-home-resume-w5) 20260711 : post-HOME lock re-assert (InitialAllTask tail)

    // AI(ht160s-agv-binsetting) 20260713 : per-Auto "bin setting" string for SVID
    // 38234-38236 / 38243-38245 (host reads via S1F3; not on any event report). Mirrors
    // HT9045 AMRUnloadBin : describes which sort-result bin(s)/category land in that
    // Auto's output car, so the AMR/host knows what grade the car it collects holds.
    // RefreshBinSettings() repopulates all six from the live routing config each tick.
    AnsiString DescribeAutoBins(int AutoIndex);
    void       RefreshBinSettings();
    //AI(amr-lane-lotno) 20260831 : per-Auto lane lot number for SVID 66040-66045 (host S1F3).
    AnsiString DescribeAutoLot(int AutoIndex);
    void       RefreshLotNumbers();

    //AI(cleanout-amr-collect) 20260901 : second AMR call reason - the CLEAN-OUT COLLECT.
    // True when this Auto has finished its clean-out drain (every working tray already
    // stacked into the output car) and the car still holds trays, so it must be taken away
    // before the lot ends even though it never reached the Full sensor. OR-ed with the
    // full-car trigger in PollAndCall; see the block comment on the definition.
    bool IsCleanOutCollectDueForAmr(int AutoIndex);

    // AI(ht160s-agv) 20260625 : read-only multi-line dump of coordinator state for
    // the State Record snapshot + the AMR maintenance panel (header + one line per
    // P1..P9 : lock / handshake / live ready). No state change.
    AnsiString DescribeAgvState();
};

extern TAgvCoordinator AgvCoord;
extern const TAgvStationDesc AgvStation[AGV_STATION_COUNT];

#endif
//---------------------------------------------------------------------------
