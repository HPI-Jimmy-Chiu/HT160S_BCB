//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("main.cpp", fMain);
USEFORM("iosetview.cpp", fiosetview);
USEFORM("uteach.cpp", fTeach);
USEFORM("uMotorTest.cpp", fMotorTest);
USEFORM("uQwertyKey.cpp", fQwertyKey);
USEFORM("language.cpp", fLan);
USEFORM("setup.cpp", fSetup);
USEFORM("data.cpp", fData);
USEFORM("maintenance.cpp", fMaintenance);
USEFORM("uOffset.cpp", fOffset);
USEFORM("uspeed.cpp", fSpeed);
USEFORM("systools.cpp", FormSysTools);
USEFORM("mymessbox.cpp", MyMessageBox);
USEFORM("note.cpp", fNote);
USEUNIT("cmydef.cpp");
USEUNIT("CosFunction.cpp");
USEUNIT("UserRoleManager.cpp");
USEUNIT("database.cpp");
USEUNIT("csystem.cpp");
USEUNIT("uruncontrol.cpp");
USEUNIT("HTimer.cpp");
USEUNIT("myio.cpp");
USEUNIT("myio_MN200.cpp");
USEUNIT("mysensor.cpp");
USEUNIT("myswitch.cpp");
USEUNIT("mycylin.cpp");
USEUNIT("MyKitSuck.cpp");
USEUNIT("MotorAndIO\\HTMotor.cpp");
USEUNIT("MotorAndIO\\MyMotor.cpp");
USEUNIT("MotorAndIO\\mySMCmotor.cpp");
USEUNIT("MotorAndIO\\myMN200motor.cpp");
USEUNIT("MotorAndIO\\MC88X1PLazyLoad.cpp");
USEUNIT("MotorAndIO\\myMC88X1motor.cpp");
USEUNIT("AutomationServer.cpp");
USEUNIT("ComPort.cpp");
USEUNIT("uPadInterface.cpp");
USEUNIT("SecsGem\uHGemEquipment.cpp");
USEUNIT("SecsGem\UsecegemMainFrom.cpp");
USEUNIT("SecsGem\uHGemClass.cpp");
USEUNIT("SecsGem\uHGemHT160.cpp");
#include "AutomationServer.h"
#include "database.h"
#include "SecsGem\uHGemEquipment.h"
#include "SecsGem\UsecegemMainFrom.h"
#include "SecsGem\uHGemHT160.h"
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
         Application->Initialize();
         Application->CreateForm(__classid(TfMain), &fMain);
         HGem = new THGem(Application);
         FSECS = new TFSECS(Application);
         HSys.MyGem = new HT160Gem("HT160S", HGem);
         FSECS->GemInitial("HT160S", "1.0.0.0");
         InitializeHT160Automation(fMain);
         Application->Run();
         ShutdownHT160Automation();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    catch (...)
    {
         try
         {
             throw Exception("");
         }
         catch (Exception &exception)
         {
             Application->ShowException(&exception);
         }
    }
    return 0;
}
//---------------------------------------------------------------------------
