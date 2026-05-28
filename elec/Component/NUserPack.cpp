//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("NUserPack.res");
USEPACKAGE("vcl35.bpi");
USEUNIT("..\spcomm\Spcomm.pas");
USERES("..\spcomm\Spcomm.dcr");
USEUNIT("..\atcomm32\ATComm32.pas");
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
//   Package source.
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
    return 1;
}
//---------------------------------------------------------------------------
