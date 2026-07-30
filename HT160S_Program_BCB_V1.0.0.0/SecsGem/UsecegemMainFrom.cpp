//---------------------------------------------------------------------------
#include <vcl.h>
#include <IniFiles.hpp>
#pragma hdrstop

#include "UsecegemMainFrom.h"
#include "database.h"
#include "uHGemClass.h"
#include "CosFunction.h"   //AI(ht160s-secsgem) 20260611 : paid-feature gate (bUseSecsGem)
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TFSECS *FSECS = NULL;
int USE_SECS_GEM = 1;
//---------------------------------------------------------------------------
__fastcall TFSECS::TFSECS(TComponent *Owner)
    : TComponent(Owner)
{
    bInitialed = false;
}
//---------------------------------------------------------------------------
__fastcall TFSECS::~TFSECS()
{
}
//---------------------------------------------------------------------------
void TFSECS::GemInitial(AnsiString HandlerType, AnsiString SoftwareVersion)
{
    if(HGem==NULL || HSys.MyGem==NULL)
        return;

    //AI(ht160s-secsgem) 20260611 : hard paid-feature gate. If the customer has
    //  not bought SECS/GEM, the whole stack stays off (no socket, no badge).
    if(!CosFunction.bUseSecsGem)
    {
        USE_SECS_GEM = 0;
        return;
    }

    //AI(ht160s-secsgem) 20260610 : read HSMS endpoint from system\General.ini [SECS].
    //  Ship + hardware install tier (same pattern as [ColorCCD]/[TopCCD]).
    //  Enable=0 disables the whole GEM stack; ActiveMode=1 dials out (host),
    //  ActiveMode=0 (default) listens as equipment.
    AnsiString sAddress = "127.0.0.1";
    int        iPort     = 5098;
    int        iDeviceID = 0;
    int        iActive   = 0;
    int        iEnable   = 1;
    int        iReconnect = 5;   //AI(ht160s-secsgem) 20260611 : reconnect interval (s), 0=off
    int        iLinktest  = 10;  //AI(ht160s-secsgem) 20260611 : Linktest heartbeat (s), 0=off
    int        iT6        = 6;   //AI(ht160s-secsgem) 20260611 : T6 wait for Linktest.rsp (s)
    int        iLogToFile = 1;   //AI(ht160s-secsgem) 20260611 : 1=persist SECS log to disk
    int        iLogLinktest = 0; //AI(ht160s-secsgem) 20260612 : 1=show routine Linktest in log (default quiet)
    int        iLogSmlBody = 1;  //AI(ht160s-secsgem) 20260716 : 1=dump full SECS-II body as SML tree (RX+TX)
    {
        AnsiString ConfigPath = HSys.CurrentDir + AnsiString("\\system\\General.ini");
        TIniFile *IniFile = new TIniFile(ConfigPath);
        try
        {
            iEnable   = IniFile->ReadInteger("SECS", "Enable",     iEnable);
            sAddress  = IniFile->ReadString ("SECS", "Address",    sAddress);
            iPort     = IniFile->ReadInteger("SECS", "Port",       iPort);
            iDeviceID = IniFile->ReadInteger("SECS", "DeviceID",   iDeviceID);
            iActive   = IniFile->ReadInteger("SECS", "ActiveMode", iActive);
            iReconnect= IniFile->ReadInteger("SECS", "ReconnectInterval", iReconnect);
            iLinktest = IniFile->ReadInteger("SECS", "LinktestInterval",  iLinktest);
            iT6       = IniFile->ReadInteger("SECS", "T6Timeout",         iT6);
            iLogToFile= IniFile->ReadInteger("SECS", "LogToFile",         iLogToFile);
            iLogLinktest= IniFile->ReadInteger("SECS", "LogLinktest",     iLogLinktest);
            iLogSmlBody = IniFile->ReadInteger("SECS", "LogSmlBody",      iLogSmlBody);
        }
        __finally
        {
            delete IniFile;
        }
    }
    if(iPort <= 0 || iPort > 65535)
        iPort = 5098;
    if(iReconnect < 0)
        iReconnect = 0;
    USE_SECS_GEM = (iEnable > 0) ? 1 : 0;

    AnsiString sPort, sDeviceID;
    sPort.sprintf("%d", iPort);
    sDeviceID.sprintf("%d", iDeviceID);

    HGem->SetTimeFormat(1);
    HGem->SetLogToFile(iLogToFile != 0);   //AI(ht160s-secsgem) 20260611 : disk log on/off
    HGem->SetLogLinktest(iLogLinktest != 0); //AI(ht160s-secsgem) 20260612 : quiet heartbeat by default
    HGem->SetLogSmlBody(iLogSmlBody != 0);   //AI(ht160s-secsgem) 20260716 : full SML body dump on by default
    HGem->SetDefaultAddressAndPort(sAddress.c_str(), sPort.c_str(), sDeviceID.c_str());
    HGem->SetReceipeDirectoryAndGlobalName("..\\data\\", "*.*", 0);
    HGem->SetMachineTypeAndSoftwarseVer(HandlerType.c_str(), SoftwareVersion.c_str());
    SECS_SETData(HGem);
    HGem->ReadEventReportData();   //AI(secs-reportdef) 20260724 : overlay host report defs AFTER firmware baseline (was SaveEventReportData)
    HGem->Timer1->Enabled = true;

    //AI(ht160s-secsgem) 20260610 : open the HSMS socket (listen or connect).
    if(USE_SECS_GEM > 0)
    {
        HGem->SetReconnectInterval(iReconnect);   //AI(ht160s-secsgem) 20260611
        HGem->SetLinktestInterval(iLinktest);     //AI(ht160s-secsgem) 20260611
        HGem->SetT6Timeout(iT6);                  //AI(ht160s-secsgem) 20260611
        HGem->SetHsmsMode(iActive != 0);
        HGem->StartCommunication();
    }
    bInitialed = true;
}
//---------------------------------------------------------------------------
void TFSECS::SECS_SETData(THGem *Ptr)
{
    if(Ptr==NULL || HSys.MyGem==NULL)
        return;

    HSys.MyGem->AddSV();
    HSys.MyGem->AddEC();
    HSys.MyGem->AddAlarmList();
    HSys.MyGem->AddCEID();
    HSys.MyGem->AddReprot();
}
//---------------------------------------------------------------------------
int TFSECS::MySFCode(int Stream, int Function)
{
    if(HSys.MyGem==NULL)
        return 1;

    if(Stream==2 && Function==41)
        return HSys.MyGem->S2F42_Host_Command_Acknowledge();

    return 1;
}
//---------------------------------------------------------------------------
bool TFSECS::IsInitialed()
{
    return bInitialed;
}
//---------------------------------------------------------------------------
void EventReport(unsigned Ceid)
{
    if(USE_SECS_GEM<=0 || HGem==NULL)
        return;

    HGem->EventReport(1, Ceid);
}
//---------------------------------------------------------------------------
//AI(secs-lotstarttime) 20260730 : SVID 66033 Lot Start Time latch.
//  Deliberately NOT gated on USE_SECS_GEM (unlike EventReport): stamping a string costs
//  nothing, and gating it would leave 66033 empty for the whole lot if the operator turns
//  SECS on after Lot Start. Only the NULL check is load-bearing.
void NoteLotStartTime(bool bStarted)
{
    if(HSys.MyGem==NULL)
        return;

    HSys.MyGem->NoteLotStartTime(bStarted);
}
//---------------------------------------------------------------------------
unsigned ComputeAlarmAlid(AnsiString Code)
{
    //AI(ht160s-secsgem) 20260715 : SSOT for the S5 ALID. 31-polynomial hash of the alarm
    // code string, shared by S5F1 (AlarmReport) and the S5F6/S5F8 catalog so a reported
    // alarm's ALID always equals its catalog-entry ALID. Registered and free-string codes
    // hash identically, so no map lookup is needed to stay consistent.
    unsigned alid = 0;
    for(int i=1; i<=Code.Length(); i++)
        alid = alid*31u + (unsigned char)Code[i];
    return alid;
}
//---------------------------------------------------------------------------
void AlarmReport(AnsiString Code, AnsiString Message, bool bSet)
{
    if(USE_SECS_GEM<=0 || HGem==NULL)
        return;
    //AI(ht160s-secsgem) 20260625 : HT160 self-defined standard S5F1. ALID = stable
    // 31-polynomial hash of the unique alarm code string (host gets a stable numeric
    // key; the readable code+message rides in ALTX). ALCD bit7 = alarm set(1)/clear(0),
    // category bits 0 (unspecified). Mirrors the global EventReport glue above.
    unsigned alid = ComputeAlarmAlid(Code);
    unsigned char alcd = bSet ? 0x80 : 0x00;
    AnsiString altx = Code;
    if(Message!="")
        altx = Code + " " + Message;
    HGem->SendAlarmS5F1(alid, alcd, altx);
}
//---------------------------------------------------------------------------
