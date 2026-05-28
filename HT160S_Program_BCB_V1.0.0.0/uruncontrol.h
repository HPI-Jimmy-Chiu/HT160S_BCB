//---------------------------------------------------------------------------
#ifndef uruncontrolH
#define uruncontrolH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class TRunControl : public TThread
{
private:
protected:
    void __fastcall Execute();
public:
    __fastcall TRunControl(bool CreateSuspended);
    void __fastcall ThreadProcess(void);
};
//---------------------------------------------------------------------------
extern TRunControl *MyThread;
extern bool RunControlSystemStart;
//---------------------------------------------------------------------------
#endif
