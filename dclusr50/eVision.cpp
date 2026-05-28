//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("eVision.res");
USEPACKAGE("vcl50.bpi");
USEUNIT("d:\Program Files\Borland\CBuilder5\Imports\EDISPLAYACLLib_OCX.cpp");
USERES("d:\program files\borland\cbuilder5\Imports\EDISPLAYACLLib_OCX.dcr");
USEUNIT("d:\Program Files\Borland\CBuilder5\Imports\EPICOLOACLLib_OCX.cpp");
USERES("d:\program files\borland\cbuilder5\Imports\EPICOLOACLLib_OCX.dcr");
USEUNIT("d:\Program Files\Borland\CBuilder5\Imports\eVision_OCX.cpp");
USERES("d:\program files\borland\cbuilder5\Imports\eVision_OCX.dcr");
USEUNIT("d:\Program Files\Borland\CBuilder5\Imports\MULTICAMLib_OCX.cpp");
USERES("d:\program files\borland\cbuilder5\Imports\MULTICAMLib_OCX.dcr");
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Package source.
//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
    return 1;
}
//---------------------------------------------------------------------------
