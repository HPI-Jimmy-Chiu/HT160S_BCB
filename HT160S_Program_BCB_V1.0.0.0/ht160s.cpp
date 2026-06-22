//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
#include "cStateRecordHT160.h"
//---------------------------------------------------------------------------
USEFORM("main.cpp", fMain);

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
USEUNIT("cprod.cpp");
USEUNIT("CosFunction.cpp");
USEUNIT("UserRoleManager.cpp");
USEUNIT("cEventLog.cpp");
USEUNIT("cProductionLog.cpp");
USEUNIT("cSelfCheck.cpp");
USEUNIT("cStepTrace.cpp");
USEUNIT("cStateRecordHT160.cpp");
USEFORM("database.cpp", DataModule1);
USEUNIT("aLoader.cpp");
USEUNIT("aEmpty.cpp");
USEUNIT("aAuto1To6.cpp");
USEUNIT("aTrayArm.cpp");
USEUNIT("aSortArm.cpp");
USEUNIT("aColor.cpp");
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
USEFORM("ComPort.cpp", fComPort);
USEUNIT("uPadInterface.cpp");
USEUNIT("SecsGem\uHGemEquipment.cpp");
USEUNIT("SecsGem\UsecegemMainFrom.cpp");
USEUNIT("SecsGem\uHGemClass.cpp");
USEUNIT("SecsGem\uHGemHT160.cpp");
USEFORM("iosetview.cpp", fiosetview);
#include "AutomationServer.h"
#include "aLoader.h"
#include "aEmpty.h"
#include "aAuto1To6.h"
#include "aTrayArm.h"
#include "aSortArm.h"
#include "aColor.h"
#include "database.h"
#include "uruncontrol.h"
#include "cEventLog.h"
#include "cCommLog.h"
#include "GeneralSetting.h"
#include "deviceinfo.h"
#include "cSelfCheck.h"
#include "SecsGem\uHGemEquipment.h"
#include "SecsGem\UsecegemMainFrom.h"
#include "SecsGem\uHGemHT160.h"
#include "uHome.h"
//AI(general) 20260603 : form class definitions needed for eager CreateForm at
//startup (ref HT172 HT172.cpp). All forms are created once at power-on instead
//of the lazy "if(fXxx==NULL) fXxx=new TfXxx(this)" pattern, which left dialogs
//(e.g. fHome) unbuilt when first needed and caused mis-position faults.
#include "main.h"
#include "language.h"
#include "setup.h"
#include "data.h"
#include "maintenance.h"
#include "uOffset.h"
#include "uspeed.h"
#include "systools.h"
#include "note.h"
#include "uteach.h"
#include "uMotorTest.h"
#include "uQwertyKey.h"
#include "iosetview.h"
#include "ComPort.h"
#include "uPadInterface.h"
#include "mymessbox.h"   //AI(general) 20260608 : ShowMyMessage instead of Application->MessageBox
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
#ifdef SOFT_SIMULATE
         //AI(HT160S-Maintainer) 20260619 : --selftest-home headless self-test (sim only).
         //MainProc auto-runs one full-machine home and terminates with an exit code.
         for(int ai=1; ai<=ParamCount(); ai++)
             if(ParamStr(ai).LowerCase()=="--selftest-home")
                 g_SelfTestHome=true;
#endif
         Application->Initialize();
         //AI(general) 20260603 : iosetview.dfm uses custom components
         //(TMyLed/TBtnPanel/TALed) whose design packages are not installed in
         //this IDE, so they are not in VCL's runtime streaming class registry.
         //Register them (and the standard DFM classes) once here, before any
         //CreateForm, or CreateForm(Tfiosetview) throws "Class Txxx not found".
         //(HT172 does not need this because its IDE has those component
         //packages installed; HT160S_BCB uses local compatibility units.)
         RegisterIOViewStreamClasses();
         //AI(general) 20260603 : create every form once at startup (ref HT172
         //HT172.cpp WinMain). fMain must be first so it becomes Application->
         //MainForm. HSys is a global, so its hardware database (motors/sensors)
         //is already loaded before any form constructor runs. Forms are shown
         //later via Show/ShowModal; their DFM Visible=False keeps them hidden.
         Application->CreateForm(__classid(TfMain), &fMain);
         Application->CreateForm(__classid(TDataModule1), &DataModule1);
         Application->CreateForm(__classid(TMyMessageBox), &MyMessageBox);
         Application->CreateForm(__classid(TfNote), &fNote);
         Application->CreateForm(__classid(TfHome), &fHome);
         Application->CreateForm(__classid(Tfiosetview), &fiosetview);
         Application->CreateForm(__classid(TfSetup), &fSetup);
         Application->CreateForm(__classid(TfData), &fData);
         Application->CreateForm(__classid(TfMaintenance), &fMaintenance);
         Application->CreateForm(__classid(TfTeach), &fTeach);
         Application->CreateForm(__classid(TfMotorTest), &fMotorTest);
         Application->CreateForm(__classid(TfOffset), &fOffset);
         Application->CreateForm(__classid(TfSpeed), &fSpeed);
         Application->CreateForm(__classid(TFormSysTools), &FormSysTools);
         Application->CreateForm(__classid(TfQwertyKey), &fQwertyKey);
         Application->CreateForm(__classid(TfLan), &fLan);
         Application->CreateForm(__classid(TfComPort), &fComPort);
         Application->CreateForm(__classid(TfPadInterface), &fPadInterface);
         g_EventLog.Init();
         //AI(general) 20260617 : log retention (auto-prune old day/month folders).
         //Values from General.ini [LogRetention]; audit logs kept longer than the
         //high-volume comm/diagnostic channels. GeneralSetting is already loaded
         //(HSys.Initial -> InitialCosFunction -> GeneralSetting.Load).
         g_EventLog.SetRetentionDays(GeneralSetting.iLogRetentionEventDays);
         //AI(ht160s-maintainer) 20260615 : per-channel serial comm CSV logs,
         //same folder layout as EventLog. Pad + LED bin display, for tracing.
         g_PadCommLog.Init("PadLog");
         g_PadCommLog.SetRetentionDays(GeneralSetting.iLogRetentionCommDays);
         g_BinDispCommLog.Init("BindisplayLog");
         g_BinDispCommLog.SetRetentionDays(GeneralSetting.iLogRetentionCommDays);
         g_DeviceInfo.Init();
         InitializeLoaderModule();
         InitializeEmptyModule();
         InitializeAutoModule();
         InitializeTrayArmModule();
         InitializeSortArmModule();
         InitializeColorModule();
         //AI(general) 20260608 : create the State Record snapshot recorder
         //(no FSM). Default output folder D:\HT160S_StateRecord\. Sampling is
         //driven from DataModule1::DoAllProcess(); snapshot from the main-form
         //"Store Hangup" button (TfMain::sbStoreHangupClick).
         if(gStateRecord==NULL)
             gStateRecord = new cStateRecordHT160();
         //AI(general) 20260601 : startup wiring self-check (no FSM).
         //Names every module/action binding at power-on; aborts start if a
         //binding is missing instead of failing silently mid-cycle.
         {
             AnsiString WiringReport;
             if(ValidateWiring(WiringReport) == false)
             {
                 //AI(general) 20260608 : no Application->MessageBox - use ShowMyMessage.
                 ShowMyMessage("Wiring Check Failed\r\n" + WiringReport);
                 return 0;
             }
         }
         HGem = new THGem(Application);
         FSECS = new TFSECS(Application);
         HSys.MyGem = new HT160Gem("HT160S", HGem);
         FSECS->GemInitial("HT160S", "1.0.0.0");
         InitializeHT160Automation(fMain);
         //AI(general) 20260601 : start the run-control thread (ref HT172
         //main.cpp TfMain::FormShow "MyThread=new TRunControl(false)").
         //Without this, TRunControl::Execute()'s Synchronize(MainProc) loop
         //never runs, so MainProc()/ProcessMotion()/DoAllProcess() are never
         //called and the machine logic stays dead. false=run immediately.
         //Teardown is in TfMain::FormClose (Terminate+WaitFor while the VCL
         //message loop is still alive) to avoid a Synchronize deadlock.
         MyThread = new TRunControl(false);
         Application->Run();
         //AI(general) 20260601 : thread already Terminated+WaitFor in
         //TfMain::FormClose; safe to release here (ref HT172 "delete MyThread").
         if(MyThread != NULL)
         {
             delete MyThread;
             MyThread = NULL;
         }
         //AI(general) 20260608 : run thread is stopped; release State Record.
         if(gStateRecord != NULL)
         {
             delete gStateRecord;
             gStateRecord = NULL;
         }
         ShutdownColorModule();
         ShutdownSortArmModule();
         ShutdownTrayArmModule();
         ShutdownAutoModule();
         ShutdownEmptyModule();
         ShutdownLoaderModule();
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
#ifdef SOFT_SIMULATE
    if(g_SelfTestHome)
        return g_SelfTestExitCode;
#endif
    return 0;
}
//---------------------------------------------------------------------------
