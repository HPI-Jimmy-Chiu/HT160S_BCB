//---------------------------------------------------------------------------
#ifndef HCalenderH
#define HCalenderH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>

#include "ccalendr.h"
//---------------------------------------------------------------------------
class PACKAGE HCalender : public TCCalendar
{
private:

protected:
  virtual void __fastcall DrawCell(int ACol, int ARow, const TRect &ARect,
    TGridDrawState AState);

public:
    bool    HStat[32];


    __fastcall HCalender(TComponent* Owner);

};
//---------------------------------------------------------------------------
#endif
 