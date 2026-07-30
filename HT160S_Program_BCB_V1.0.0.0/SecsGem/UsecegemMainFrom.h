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
//AI(secs-lotstarttime) 20260730 : latch (true) / clear (false) SVID 66033 Lot Start Time
//  from the production layer. Thin forwarder to HSys.MyGem so main.cpp keeps including
//  only this header, exactly like EventReport / AlarmReport.
void NoteLotStartTime(bool bStarted);
//AI(ht160s-secsgem) 20260625 : S5F1 alarm report (set/clear) glue, mirrors EventReport.
void AlarmReport(AnsiString Code, AnsiString Message, bool bSet);
//AI(ht160s-secsgem) 20260715 : SSOT ALID hash shared by S5F1 + the S5F6/S5F8 alarm catalog.
unsigned ComputeAlarmAlid(AnsiString Code);
//---------------------------------------------------------------------------
#endif
