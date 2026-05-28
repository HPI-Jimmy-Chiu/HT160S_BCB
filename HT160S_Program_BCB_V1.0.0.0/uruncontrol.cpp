//---------------------------------------------------------------------------

#include <vcl.h>
#include <mmsystem.h>
#pragma hdrstop

#include "uruncontrol.h"
#include "csystem.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
TRunControl *MyThread = NULL;
bool RunControlSystemStart = false;
//---------------------------------------------------------------------------
__fastcall TRunControl::TRunControl(bool CreateSuspended)
    : TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void __fastcall TRunControl::ThreadProcess(void)
{
    MainProc();
}
//---------------------------------------------------------------------------
void __fastcall TRunControl::Execute()
{
    static int ict = 0;

    timeBeginPeriod(1);
    do
    {
        Synchronize(ThreadProcess);
        if(RunControlSystemStart == false)
        {
            SleepEx(1, true);
        }
        else
        {
            ict++;
            if(ict > 2)
            {
                ict = 0;
            }
            else
            {
                SleepEx(1, true);
            }
        }
    } while(!Terminated);
}
//---------------------------------------------------------------------------
