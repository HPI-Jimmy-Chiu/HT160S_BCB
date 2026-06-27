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

struct TAgvStationDesc
{
    int         PIndex;        // 1..9
    int         Kind;          // eAgvStationKind
    int         AutoIndex;     // 0..5 for ASK_AUTO, else -1
    unsigned    SvidCarrierID; // 38202..38210
    unsigned    SvidTrayCount; // draft section 6 (non-contiguous past Auto3)
    unsigned    SvidDeviceCnt; // ditto
    unsigned    SvidBinSet;    // ASCII bin setting (Auto only; 0 if none)
    const char *Name;          // "Loader"/"Empty"/"Color"/"AUTO1".."AUTO6"
};

class TAgvCoordinator
{
public:
    // --- SVID-bound snapshot data (stable addresses for SetSVDataPointer) ---
    AnsiString SupplementBitmap;                 // 38219  set at CEID272 fire time
    AnsiString StatusBitmap;                     // 38220  set at CEID273 fire time
    AnsiString FinishBitmap;                     // 38221  set at CEID274 fire time
    AnsiString CarrierID[AGV_STATION_COUNT];     // 38202..38210 (index P-1)
    int        TrayCount[AGV_STATION_COUNT];     // (index P-1)
    int        DeviceCount[AGV_STATION_COUNT];   // (index P-1)
    AnsiString BinSetting[AGV_AUTO_COUNT];       // Auto1..6 (index AutoIndex)

    // --- per-station runtime handshake state (Phase B/C/D) ---
    unsigned char Handshake[AGV_STATION_COUNT];      // eAgvHandshake
    unsigned char PrepDone[AGV_STATION_COUNT];
    unsigned char ShortageLatch[AGV_STATION_COUNT];  // one-shot: fire CEID272 once
    int           ShortageDebounce[AGV_STATION_COUNT];  // AI(ht160s-agv) 20260625 : per-station PREP/READY age (ServiceHandshake watchdog ticks)
    unsigned char ReadyEntrySensor[AGV_STATION_COUNT]; // edge baseline for Finish

    TAgvCoordinator();
    void Reset();

    AnsiString BuildBitmap(int targetPIndex);    // "P1:0,...,Px:1,...,P9:0"
    int        LookupByName(AnsiString cpName);  // -> index 0..8, -1 if unknown

    // Phase B/C/D entry points (Gem = SECS transport used to fire S6F11):
    void PollAndCall(THGem *Gem);                // Phase B : shortage/full -> CEID272
    void ServiceHandshake(THGem *Gem);           // Phase D : drive CEID273 / CEID274
    bool BeginPrep(AnsiString cpName);           // Phase C : START_AGV -> prep

    // AI(ht160s-agv) 20260627 : station-side timeout release. Auto-full waited
    // iAmrFullWaitSec for the AGV; drop THIS Auto's handshake (lock + state) so
    // neither the watchdog nor PollAndCall touches it until the next clean edge.
    void AbortAutoHandshake(int Index);

    // AI(ht160s-agv) 20260625 : read-only multi-line dump of coordinator state for
    // the State Record snapshot + the AMR maintenance panel (header + one line per
    // P1..P9 : lock / handshake / live ready). No state change.
    AnsiString DescribeAgvState();
};

extern TAgvCoordinator AgvCoord;
extern const TAgvStationDesc AgvStation[AGV_STATION_COUNT];

#endif
//---------------------------------------------------------------------------
