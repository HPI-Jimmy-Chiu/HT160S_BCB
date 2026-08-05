//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
#include "cStateRecordHT160.h"
//---------------------------------------------------------------------------
USEFORM("main.cpp", fMain);
USEFORM("iosetview.cpp", fiosetview);
USEFORM("uteach.cpp", fTeach);
USEFORM("uMotorTest.cpp", fMotorTest);
USEFORM("uHome.cpp", fHome);
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
USEFORM("database.cpp", DataModule1);
USEFORM("ComPort.cpp", fComPort);
USEFORM("SecsGem\uHGemLogForm.cpp", fSecsGemLog);
USEFORM("uPadInterface.cpp", fPadInterface);
//---------------------------------------------------------------------------
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
#include "cSoterOutput.h"
#include "uFtpUploadThread.h"   //AI(ht160s-ftp) 20260721 : create background FTP upload worker at startup
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
#include <ComCtrls.hpp>  //AI(ht160s-initflow) 20260624 : TProgressBar for the startup splash
//---------------------------------------------------------------------------
//AI(ht160s-initflow) 20260624 : startup progress splash (ref HT9045 fShowMessage/
//UpdateForm). A code-built (no .dfm) frameless form shown during WinMain startup so the
//operator sees an advancing progress bar instead of a frozen/blank screen ("looks
//broken"). Paired with the whole-machine InitialOK flag (cmydef) so background handlers
//no-op until startup finishes. NOT shown under headless --selftest-home. Phase B will
//also drive UpdateInitProgress() from HSys.Initial() once that heavy init (currently in
//the static SYSTEM_MODULAR ctor, before any window exists) is relocated into WinMain.
static TForm        *gInitSplash = NULL;
static TProgressBar *gInitBar    = NULL;
static TLabel       *gInitLabel  = NULL;

static void CreateInitSplash()
{
#ifdef SOFT_SIMULATE
    if(g_SelfTestHome)
        return;                              // headless self-test: no UI
#endif
    try
    {
        gInitSplash = new TForm(Application);
        gInitSplash->BorderStyle = bsNone;
        gInitSplash->Position    = poScreenCenter;
        gInitSplash->Width       = 460;
        gInitSplash->Height      = 150;
        gInitSplash->Color       = clWhite;

        gInitLabel = new TLabel(gInitSplash);
        gInitLabel->Parent     = gInitSplash;
        gInitLabel->Left       = 30;
        gInitLabel->Top        = 34;
        gInitLabel->Font->Size = 12;
        gInitLabel->Caption    = "System initializing, please wait...";

        gInitBar = new TProgressBar(gInitSplash);
        gInitBar->Parent   = gInitSplash;
        gInitBar->Left     = 30;
        gInitBar->Top      = 80;
        gInitBar->Width    = 400;
        gInitBar->Height   = 24;
        gInitBar->Min      = 0;
        gInitBar->Max      = 100;
        gInitBar->Position = 0;

        gInitSplash->Show();
        Application->ProcessMessages();
    }
    catch(...)
    {
        //a splash failure must never block boot - run without it
        gInitSplash = NULL;
        gInitBar    = NULL;
        gInitLabel  = NULL;
    }
}

//AI(ht160s-initflow) 20260624 : advance the startup progress bar and pump the message
//queue so the splash repaints (window not flagged "Not Responding"). Safe no-op when the
//splash was not created (headless / construction failed).
void UpdateInitProgress(int iPercent)   //AI(ht160s-initflow) 20260624 : exported (cmydef.h) for HSys.Initial() in Phase B
{
    if(iPercent < 0)   iPercent = 0;
    if(iPercent > 100) iPercent = 100;
    try
    {
        if(gInitBar != NULL)
            gInitBar->Position = iPercent;
        if(gInitSplash != NULL)
            Application->ProcessMessages();
    }
    catch(...)
    {
    }
}

static void DestroyInitSplash()
{
    try
    {
        if(gInitSplash != NULL)
        {
            gInitSplash->Close();
            delete gInitSplash;              // owns and frees the child label/bar
        }
    }
    catch(...)
    {
    }
    gInitSplash = NULL;
    gInitBar    = NULL;
    gInitLabel  = NULL;
}
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
         //AI(ht160s-initflow) 20260624 : bring up the startup progress splash now,
         //before the heavy CreateForm sequence + hardware init, so the operator sees
         //progress instead of a dead screen. Torn down just before Application->Run().
         CreateInitSplash();
         UpdateInitProgress(2);
         //AI(ht160s-initflow) 20260624 : Phase B - run the heavy hardware/config init
         //HERE (relocated from the static SYSTEM_MODULAR ctor) so OpenMN200Card + file
         //loads advance the splash instead of freezing a pre-window screen. MUST run
         //before every CreateForm (forms read HSys) and InitializeXxxModule (read HSys.VMot).
         HSys.Initial();
         UpdateInitProgress(48);
         //AI(general) 20260603 : create every form once at startup (ref HT172
         //HT172.cpp WinMain). fMain must be first so it becomes Application->
         //MainForm. HSys.Initial() above has loaded the hardware database, so form
         //constructors that read HSys are safe. Forms are shown later via Show/
         //ShowModal; their DFM Visible=False keeps them hidden.
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
         UpdateInitProgress(86);
         g_EventLog.Init();
         //AI(ht160s-statusbar) 20260624 : HT172 program-start version log (was logged
         //from the systools timer reading the version panel). Routed through the HT160
         //event-log path (RecordProcess -> g_EventLog). fNote already exists (created above).
         RecordProcess(AnsiString("Program Start with version ")+MainVersion);
         //AI(general) 20260617 : log retention (auto-prune old day/month folders).
         //Values from General.ini [LogRetention]; audit logs kept longer than the
         //high-volume comm/diagnostic channels. GeneralSetting is already loaded
         //(HSys.Initial -> InitialCosFunction -> GeneralSetting.Load).
         g_EventLog.SetRetentionDays(GeneralSetting.iLogRetentionEventDays);
         //AI(ht160s-ftp) 20260721 : create the background FTP upload worker now that
         //GeneralSetting (retention) and the log root are ready. LoadConfig reads
         //[Ftp] and the worker starts suspended->resumed inside Ensure. Enable/
         //UploadReport ship OFF, so nothing uploads until the maintenance screen
         //verifies the link; teardown is in TfMain::FormClose.
         EnsureFtpUploadThreadCreated();
         //AI(ht160s-maintainer) 20260615 : per-channel serial comm CSV logs,
         //same folder layout as EventLog. Pad + LED bin display, for tracing.
         g_PadCommLog.Init("PadLog");
         g_PadCommLog.SetRetentionDays(GeneralSetting.iLogRetentionCommDays);
         g_BinDispCommLog.Init("BindisplayLog");
         g_BinDispCommLog.SetRetentionDays(GeneralSetting.iLogRetentionCommDays);
         g_DeviceInfo.Init();
         //AI(ht160s-prodlog) 20260716 : per-day Production aggregate retention (General.ini [LogRetention] ProdDailyDays)
         g_DeviceInfo.SetDailyRetentionDays(GeneralSetting.iLogRetentionProdDailyDays);
         g_SoterOutput.Init();
         InitializeLoaderModule();
         InitializeEmptyModule();
         InitializeAutoModule();
         InitializeTrayArmModule();
         InitializeSortArmModule();
         InitializeColorModule();
         UpdateInitProgress(90);
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
                 //AI(ht160s-initflow) 20260624 : drop the splash before the fatal dialog.
                 DestroyInitSplash();
                 //AI(general) 20260608 : no Application->MessageBox - use ShowMyMessage.
                 ShowMyMessage("Wiring Check Failed\r\n" + WiringReport);
                 return 0;
             }
         }
         HGem = new THGem(Application);
         FSECS = new TFSECS(Application);
         HSys.MyGem = new HT160Gem("HT160S", HGem);
         FSECS->GemInitial("HT160S", HT160S_VERSION);   //AI(ht160s-version-ssot) 20260805 : version from cmydef.h (IncludeAllHeader.h pulls it in)
         UpdateInitProgress(94);
         InitializeHT160Automation(fMain);
         UpdateInitProgress(96);
         //AI(general) 20260601 : start the run-control thread (ref HT172
         //main.cpp TfMain::FormShow "MyThread=new TRunControl(false)").
         //Without this, TRunControl::Execute()'s Synchronize(MainProc) loop
         //never runs, so MainProc()/ProcessMotion()/DoAllProcess() are never
         //called and the machine logic stays dead. false=run immediately.
         //Teardown is in TfMain::FormClose (Terminate+WaitFor while the VCL
         //message loop is still alive) to avoid a Synchronize deadlock.
         //AI(ht160s-initflow) 20260624 : startup complete - release the whole-machine
         //init guard so the worker thread's MainProc (and the serial-comm Timer1) run
         //for real. Then tear the splash down right before Application->Run() shows the
         //main form (no message pump in between, so no blank flash).
         UpdateInitProgress(100);
         InitialOK = true;
         MyThread = new TRunControl(false);
         DestroyInitSplash();
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
