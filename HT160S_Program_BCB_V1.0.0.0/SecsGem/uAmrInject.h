//---------------------------------------------------------------------------
// uAmrInject.h
// AI(ht160s-agv) 20260708 : AMR MANUAL-INJECT test facility (runtime, NOT SOFT_SIMULATE).
// Lets an engineer drive the full AMR/AGV SECS handshake by button when there is
// no real AMR (or the car-taken sensor SnAutoX_InputEnd is unwired). The inject is
// consumed ONLY at the AGV coordinator call sites (TAgvCoordinator::PollAndCall /
// ServiceHandshake in uAgvStation.cpp), OR-ed with the real *ForAmr predicate, e.g.
//   bFull = AutoModule->IsOutputCarFullForAmr(a) || AmrInject.AutoFull(a);
// The predicate BODIES are unchanged, so production / clean-out consumers of the
// same predicates see the real sensors only - the inject blast radius is the SECS
// handshake alone. The SECS command content (bitmap / CEID / SVID / HCACK) is built
// downstream by production code from real car/station state and is NOT affected.
//
// Latch semantics mirror the real sensor lifecycle:
//   FULL / SHORTAGE / DRAINED / READY : level (sticky) - stay asserted while armed,
//     toggle on/off by button; auto-cleared when the cycle finishes (Taken/Finish).
//     Read-only (never consumed). Injection is applied only at the coordinator, so
//     the DescribeAgvState() 'ready=' display shows the REAL predicate, not the inject.
//   TAKEN / FINISH : edge (one-shot) - fire CEID274 once, then clear the whole
//     station cycle so PollAndCall does not immediately re-call (272).
//
// Additive override : each predicate does "if(inject) return true; else <normal>".
// So with an edge un-armed the machine runs its real/sim path unchanged - the
// operator can let some edges run real (e.g. real drain) and inject only the ones
// a real AMR would satisfy (car-taken / refill-done).
//
// SAFETY: bTestMode defaults OFF, is NEVER persisted (OFF on every app start), and is
// cleared by any HOME/init (TDataModule1::InitialAllTask), by any machine start
// (MachineStart in csystem.cpp), and by unchecking the maintenance checkbox. Every
// read/consume no-ops while bTestMode is false, so nothing changes when the mode is off.
//---------------------------------------------------------------------------
#ifndef uAmrInjectH
#define uAmrInjectH

#include <vcl.h>

#define AMR_INJ_AUTO_COUNT  6
#define AMR_INJ_INPUT_COUNT 3

enum eAmrInjectEdge
{
    AIE_FULL = 0,
    AIE_DRAINED,
    AIE_TAKEN,
    AIE_SHORTAGE,
    AIE_READY,
    AIE_FINISH
};

class TAmrInject
{
private:
    bool bTestMode;
    bool bAutoFull[AMR_INJ_AUTO_COUNT];
    bool bAutoDrained[AMR_INJ_AUTO_COUNT];
    bool bAutoTaken[AMR_INJ_AUTO_COUNT];
    bool bInShort[AMR_INJ_INPUT_COUNT];
    bool bInReady[AMR_INJ_INPUT_COUNT];
    bool bInFinish[AMR_INJ_INPUT_COUNT];
    AnsiString sLog;

    bool AutoOOR(int a) { return (a < 0 || a >= AMR_INJ_AUTO_COUNT); }
    bool InOOR(int p)   { return (p < 0 || p >= AMR_INJ_INPUT_COUNT); }
    void Add(AnsiString Line)
    {
        sLog = sLog + Line + "\r\n";
        if(sLog.Length() > 3000)
            sLog = sLog.SubString(sLog.Length() - 2399, 2400);
    }

public:
    TAmrInject() { Reset(); }

    void Reset()
    {
        int i;
        bTestMode = false;
        for(i = 0; i < AMR_INJ_AUTO_COUNT; i++)
        {
            bAutoFull[i] = false;
            bAutoDrained[i] = false;
            bAutoTaken[i] = false;
        }
        for(i = 0; i < AMR_INJ_INPUT_COUNT; i++)
        {
            bInShort[i] = false;
            bInReady[i] = false;
            bInFinish[i] = false;
        }
    }

    bool IsTestMode() { return bTestMode; }
    void SetTestMode(bool bOn)
    {
        if(bOn == bTestMode)
            return;
        if(bOn == false)
            Reset();              // leaving test mode clears every latch
        bTestMode = bOn;
        Add(bOn ? AnsiString("== AMR TEST MODE ON ==")
                : AnsiString("== AMR TEST MODE OFF =="));
    }

    // ---- button side : arm (level = toggle, edge = one-shot) ----
    void RequestAuto(int a, int edge)
    {
        if(bTestMode == false || AutoOOR(a))
            return;
        if(edge == AIE_FULL)
        {
            bAutoFull[a] = !bAutoFull[a];
            Add("Auto" + IntToStr(a + 1) + " FULL=" + IntToStr(bAutoFull[a] ? 1 : 0));
        }
        else if(edge == AIE_DRAINED)
        {
            bAutoDrained[a] = !bAutoDrained[a];
            Add("Auto" + IntToStr(a + 1) + " DRAINED=" + IntToStr(bAutoDrained[a] ? 1 : 0));
        }
        else if(edge == AIE_TAKEN)
        {
            bAutoTaken[a] = true;
            Add("Auto" + IntToStr(a + 1) + " TAKEN armed");
        }
    }
    void RequestInput(int p, int edge)
    {
        if(bTestMode == false || InOOR(p))
            return;
        AnsiString nm = (p == 0) ? "Loader" : ((p == 1) ? "Empty" : "Color");
        if(edge == AIE_SHORTAGE)
        {
            bInShort[p] = !bInShort[p];
            Add(nm + " SHORTAGE=" + IntToStr(bInShort[p] ? 1 : 0));
        }
        else if(edge == AIE_READY)
        {
            bInReady[p] = !bInReady[p];
            Add(nm + " READY=" + IntToStr(bInReady[p] ? 1 : 0));
        }
        else if(edge == AIE_FINISH)
        {
            bInFinish[p] = true;
            Add(nm + " FINISH armed");
        }
    }

    // ---- predicate side : level reads (sticky, non-consuming) ----
    bool AutoFull(int a)    { return (bTestMode && !AutoOOR(a) && bAutoFull[a]); }
    bool AutoDrained(int a) { return (bTestMode && !AutoOOR(a) && bAutoDrained[a]); }
    bool InputShort(int p)  { return (bTestMode && !InOOR(p) && bInShort[p]); }
    bool InputReady(int p)  { return (bTestMode && !InOOR(p) && bInReady[p]); }

    // ---- predicate side : edge consumes (one-shot, clear the station cycle) ----
    bool AutoTaken(int a)
    {
        if(bTestMode == false || AutoOOR(a) || bAutoTaken[a] == false)
            return false;
        bAutoTaken[a] = false;
        bAutoFull[a] = false;
        bAutoDrained[a] = false;
        Add("Auto" + IntToStr(a + 1) + " TAKEN injected (274)");
        return true;
    }
    bool InputFinish(int p)
    {
        if(bTestMode == false || InOOR(p) || bInFinish[p] == false)
            return false;
        bInFinish[p] = false;
        bInShort[p] = false;
        bInReady[p] = false;
        Add("P" + IntToStr(p + 1) + " FINISH injected (274)");
        return true;
    }

    // ---- cycle finish : clear this station's injected level latches (sim-safe) ----
    // AI(ht160s-agv) 20260720 : under SOFT_SIMULATE the drained/taken predicate is
    // hard-true and short-circuits the AutoTaken/InputFinish consume above, so an
    // injected FULL/SHORTAGE level latch never auto-cleared and the handshake re-called
    // every tick (272->273->274 loop). The coordinator now calls these at the CEID274
    // fire point so one inject drives exactly one cycle, then stops. Real hardware is
    // unaffected (its finish predicate is sensor-driven and consumes the inject normally).
    void ClearAutoCycle(int a)
    {
        if(bTestMode == false || AutoOOR(a))
            return;
        if(bAutoFull[a] || bAutoDrained[a] || bAutoTaken[a])
            Add("Auto" + IntToStr(a + 1) + " cycle cleared (274)");
        bAutoFull[a] = false;
        bAutoDrained[a] = false;
        bAutoTaken[a] = false;
    }
    void ClearInputCycle(int p)
    {
        if(bTestMode == false || InOOR(p))
            return;
        if(bInShort[p] || bInReady[p] || bInFinish[p])
            Add("P" + IntToStr(p + 1) + " cycle cleared (274)");
        bInShort[p] = false;
        bInReady[p] = false;
        bInFinish[p] = false;
    }

    // ---- SECS S2F41 handler alert (bad / rejected host command) ----
    void NoteHostReject(AnsiString Cmd, unsigned Hcack)
    {
        Add("!! S2F41 cmd=" + Cmd + " HCACK=" + IntToStr((int)Hcack));
    }

    AnsiString GetLog() { return sLog; }

    AnsiString Describe()
    {
        int i;
        AnsiString s = "AmrInject: testMode=" + IntToStr(bTestMode ? 1 : 0);
        if(bTestMode)
        {
            s = s + " armed[";
            for(i = 0; i < AMR_INJ_AUTO_COUNT; i++)
            {
                if(bAutoFull[i])    s = s + " A" + IntToStr(i + 1) + "F";
                if(bAutoDrained[i]) s = s + " A" + IntToStr(i + 1) + "D";
                if(bAutoTaken[i])   s = s + " A" + IntToStr(i + 1) + "T";
            }
            for(i = 0; i < AMR_INJ_INPUT_COUNT; i++)
            {
                if(bInShort[i]) s = s + " P" + IntToStr(i + 1) + "S";
                if(bInReady[i]) s = s + " P" + IntToStr(i + 1) + "R";
                if(bInFinish[i]) s = s + " P" + IntToStr(i + 1) + "F";
            }
            s = s + " ]";
        }
        return s + "\r\n";
    }
};

extern TAmrInject AmrInject;

#endif
//---------------------------------------------------------------------------
