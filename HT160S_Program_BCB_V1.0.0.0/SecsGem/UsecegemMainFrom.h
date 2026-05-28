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
//---------------------------------------------------------------------------
#endif
