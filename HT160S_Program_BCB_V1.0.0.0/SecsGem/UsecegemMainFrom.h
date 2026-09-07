//---------------------------------------------------------------------------
#ifndef UsecegemMainFromH
#define UsecegemMainFromH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "uHGemEquipment.h"
//---------------------------------------------------------------------------
extern int USE_SECS_GEM;
//---------------------------------------------------------------------------
class TFSECS : public TComponent
{
private:
    bool bInitialed;
public:
    __fastcall TFSECS(TComponent *Owner);
    __fastcall ~TFSECS();

    void GemInitial(AnsiString HandlerType, AnsiString SoftwareVersion);
    void SECS_SETData(THGem *Ptr);
    int MySFCode(int Stream, int Function);
    bool IsInitialed();
};
//---------------------------------------------------------------------------
extern TFSECS *FSECS;
void EventReport(unsigned Ceid);
//AI(secs-lotstarttime) 20260730 : latch (true) / clear (false) SVID 1009 Lot Start Time
//  from the production layer. Thin forwarder to HSys.MyGem so main.cpp keeps including
//  only this header, exactly like EventReport / AlarmReport.
//AI(secs-lotstarttime-persist) 20260730 : sWhen lets a caller supply the moment instead of
//  "now" - used by the power-on work-order restore, which must re-latch the ORIGINAL lot start
//  time, not the resume time. Empty (the default) keeps the previous stamp-now behaviour, so
//  every existing call site is unchanged.
void NoteLotStartTime(bool bStarted, AnsiString sWhen="");
//AI(ht160s-secsgem) 20260625 : S5F1 alarm report (set/clear) glue, mirrors EventReport.
void AlarmReport(AnsiString Code, AnsiString Message, bool bSet);
//AI(secs-alid-optiond) 20260902 : SSOT ALID shared by S5F1 and the S5F6/S5F8 alarm
//  catalog - a PURE function of the alarm code string (no map, no language, no machine
//  state), which is what makes the reported ALID equal the catalog ALID. Returns the
//  class-banded encoding Class*100000000 + Payload : 1=JAM 2=WAR 3=MES (payload = the
//  canonical 4- or 5-digit tail), 4=cylinder 5=motor 6=sucker 7=eRecordProcess 8=eOther
//  (payload = the whole 5-digit code), 9=unregistered free string (host: "not in the
//  catalog, read the code from the leading token of ALTX"). ALWAYS exactly 9 digits:
//  never 0, never above 999999999, so even a signed 32-bit host parse is safe. Amendment
//  2 in database.cpp CreateSystemAlarmCode() self-checks every registered code at startup.
unsigned ComputeAlarmAlid(AnsiString Code);
//---------------------------------------------------------------------------
#endif
