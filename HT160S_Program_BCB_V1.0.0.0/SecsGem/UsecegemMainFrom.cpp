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
    //AI(secs-strict-reportdef) 20260810 : 1 (DEFAULT) = E5-conformant strict S2F33/S2F35 validation -
    //  an SVID/CEID/RPTID we do not define rejects the whole packet (DRACK 0x04 / LRACK 0x04 / 0x05)
    //  and commits nothing, so the host learns immediately that the data will never come. 0 = the
    //  20260727 Path A tolerance. This is a COMMISSIONING escape hatch, not an operator toggle:
    //  leave it at 1 unless a host is stuck and production cannot wait for a corrected report def.
    int        iStrictReportValidation = 1;
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
            iStrictReportValidation = IniFile->ReadInteger("SECS", "StrictReportValidation", iStrictReportValidation);   //AI(secs-strict-reportdef) 20260810
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
    //AI(secs-strict-reportdef) 20260810 : push the strict/tolerant policy into the codec before
    //StartCommunication, so the very first S2F33 of a session is already judged by it.
    HGem->SetStrictReportValidation(iStrictReportValidation != 0);
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
//AI(secs-lotstarttime) 20260730 : SVID 1009 Lot Start Time latch.
//  Deliberately NOT gated on USE_SECS_GEM (unlike EventReport): stamping a string costs
//  nothing, and gating it would leave 1009 empty for the whole lot if the operator turns
//  SECS on after Lot Start. Only the NULL check is load-bearing.
void NoteLotStartTime(bool bStarted, AnsiString sWhen)
{
    if(HSys.MyGem==NULL)
        return;

    HSys.MyGem->NoteLotStartTime(bStarted, sWhen);
}
//---------------------------------------------------------------------------
//AI(secs-alid-optiond) 20260902 : the legacy 31-polynomial ALID hash. Retained on
// purpose, for three jobs :
//  (1) it is the class-9 payload source in ComputeAlarmAlid() below,
//  (2) every SECS log written BEFORE the Option D cutover recorded the value it
//      returns, so a post-mortem on a D:\HT160S_Log\SECS_GEM\ file older than the
//      cutover still needs this exact arithmetic, and
//  (3) it is the one-line rollback switch (see ROLLBACK in ComputeAlarmAlid).
// Byte-for-byte the pre-Option-D loop - do NOT "tidy" the (unsigned char) cast or the
// 32-bit wrap; both are part of the historical values.
// File-static on purpose : nothing outside this translation unit may mint an ALID.
static unsigned LegacyAlarmHash(AnsiString Code)
{
    unsigned h = 0;
    for(int i=1; i<=Code.Length(); i++)
        h = h*31u + (unsigned char)Code[i];
    return h;
}
//---------------------------------------------------------------------------
unsigned ComputeAlarmAlid(AnsiString Code)
{
    //AI(secs-alid-optiond) 20260902 : SSOT for the S5 ALID. Class-banded 9-digit
    // encoding, owner-ratified 20260902 (docs\plan\alid-option-d-ratified-spec-20260902.md).
    // Replaces the raw 31-polynomial hash, which could reach 10 digits and so broke the
    // KYEC "alarm code is at most 9 digits" requirement.
    //
    //   ALID = Class * 100000000 + Payload            ALWAYS exactly 9 digits
    //
    //   Class 1  "JAM" + canonical digit tail   Payload = tail value   JAM0913  ->100000913
    //   Class 2  "WAR" + canonical digit tail   Payload = tail value   WAR16120 ->200016120
    //   Class 3  "MES" + canonical digit tail   Payload = tail value   MES1421  ->300001421
    //   Class 4  exactly 5 digits, 1st digit 4  Payload = whole code   40000    ->400040000
    //   Class 5  exactly 5 digits, 1st digit 5  Payload = whole code   50000    ->500050000
    //   Class 6  exactly 5 digits, 1st digit 6  Payload = whole code   60000    ->600060000
    //   Class 7  exactly 5 digits, 1st digit 7  Payload = whole code   70000    ->700070000
    //   Class 8  exactly 5 digits, 1st digit 8  Payload = whole code   80000    ->800080000
    //   Class 9  anything else                  Payload = legacy hash mod 100000000
    //   Class 0  NEVER produced (it would emit fewer than 9 digits).
    //
    // Classes 4..8 mirror eAlarmType (database.h) : 4=eCynAlarm 5=eMotorAlarm
    // 6=eSuckAlarm 7=eRecordProcess 8=eOther. note.cpp ShowSystemError already reads the
    // FIRST CHARACTER of a code as that type digit and has branches for 7 and 8, so a
    // 7xxxx / 8xxxx code is a first-class citizen of the alarm machinery, not a
    // hypothetical - which is why 7 and 8 are LIVE numeric classes here and NOT
    // "reserved for two future 3-letter prefixes".
    //
    // Class 9 is the HOST SIGNAL : "this ALID is NOT in the S5F6/S5F8 catalog - read the
    // alarm code from the leading token of ALTX". It is where the ~47 unregistered
    // free-string alarms land (sensor names, "<alias>_MotOverLimitErr", SUC%03d%d codes,
    // and whole English sentences passed straight through by note.cpp ShowSystemError).
    //
    // HOST DECODE :  class = ALID/100000000 ; payload = ALID%100000000
    //   class 1/2/3 -> prefix + (payload<10000 ? "%04u" : "%u")   exact, see AMENDMENT 1b
    //   class 4..8  -> "%05u"                                     exact by construction
    //   class 9     -> not in the catalog
    //
    // WHY NOT the HT9046LS "%d%02d%06d" scheme : it puts 47 HT160S codes onto ALIDs the
    // customer's own HT9046LS machines already use, 22 of them for a DIFFERENT alarm
    // (HT160S MES1421 -> 314001421 = HT9046LS MES1421 "No tray on Color tray"). Their EAP
    // resolves ALIDs from an out-of-band dictionary, so one shared dictionary would render
    // HT160S alarms as 9046LS ones.
    //
    // PURE function of the code string ONLY - no map lookup, no language, no machine
    // state, no clock. That is what makes the S5F1 report ALID equal the S5F6/S5F8
    // catalog ALID (uHGemHT160.cpp EmitAlarmCatalog calls this same function).
    //
    // ROLLBACK : insert  return LegacyAlarmHash(Code);  as the first statement below.
    unsigned uClass   = 0;
    unsigned uPayload = 0;
    int      iLen     = Code.Length();

    //Classes 1/2/3 : a 3-letter family prefix plus a CANONICAL numeric tail of exactly
    //4 or 5 digits - so the whole code string is exactly 7 or 8 characters. Any other
    //length falls straight through to class 9. AnsiString is 1-BASED : Code[1]..Code[iLen].
    if(iLen==7 || iLen==8)
    {
        if     (Code[1]=='J' && Code[2]=='A' && Code[3]=='M') uClass = 1;
        else if(Code[1]=='W' && Code[2]=='A' && Code[3]=='R') uClass = 2;
        else if(Code[1]=='M' && Code[2]=='E' && Code[3]=='S') uClass = 3;
    }
    if(uClass!=0)
    {
        int      iTailLen = iLen-3;               //4 or 5, guaranteed by the gate above
        unsigned uTail    = 0;
        bool     bOk      = true;
        //Parse the tail BY HAND, digit by digit - deliberately NOT atoi() / ToIntDef().
        //atoi would accept "+1", " 12" and "12x", and it would silently equate "01421"
        //with "1421" - exactly the non-injectivity AMENDMENT 1b exists to remove. At most
        //5 digits are read, so uTail can never exceed 99999 and cannot overflow; the
        //length gate above is what makes that true.
        for(int i=4; i<=iLen; i++)
        {
            if(Code[i]<'0' || Code[i]>'9')
            {
                bOk = false;
                break;
            }
            uTail = uTail*10u + (unsigned)(Code[i]-'0');
        }
        //AMENDMENT 1b - TAIL CANONICALISATION. Exactly one string per payload value :
        //    4 digits AND value <  10000     "0913" "1421" "0920"   (100 per unit 01..99)
        //    5 digits AND value >= 10000     "16120"                (1000 per unit 10..99)
        //Anything else falls to class 9. Needed because payload = int(tail) discards
        //leading zeros, so MES1421 / MES01421 / MES001421 would all compose 300001421 -
        //three different strings, ONE ALID, and the host cannot render the right one back.
        //NOT "reject any leading zero" : that naive rule kills 11 IN-SERVICE codes, all of
        //them unit 01..09 four-digit tails (JAM0913, MES0920, MES0924, MES0925, WAR0154,
        //WAR0330, WAR0462, WAR0475, WAR0962, WAR0963, WAR0970).
        //MEASURED 20260902 over system\AlarmList.csv : of the 65 prefixed codes the tail
        //lengths are {4:63, 5:2}, none has a 5-digit tail starting with '0', and this rule
        //rejects ZERO of them - all 65 survive.
        //CONSEQUENCE for future numbering : units 01..09 are capped at 100 alarms each
        //("0UNN"); a 5-digit "0UNNN" tail is ILLEGAL, so a future WAR09120 is NOT allowed -
        //renumber that unit to >=10 instead. Units 10..99 may use "UUNN" or "UUNNN".
        if(bOk && iTailLen==4 && uTail>=10000u)
            bOk = false;                          //structurally unreachable, kept explicit
        if(bOk && iTailLen==5 && uTail< 10000u)
            bOk = false;                          //"MES01421", "WAR09120", ...
        //AMENDMENT 1 - the disjointness guard, payload ceiling 999999. Subsumed by 1b
        //today (a tail of at most 5 digits can never exceed 99999) and retained on
        //purpose : it is the RATIFIED invariant that keeps Option D disjoint from the
        //customer's own HT9046LS ALID space. All 2737 of their ALIDs are 9 digits with a
        //NON-ZERO unit in digits 2-3 (min 101000109, max 330003021); a class 1/2/3 value
        //reads "00" there only while the payload stays below 1000000. Without this ceiling
        //a code string "MES14001421" would compose 314001421 - exactly the 9046LS MES1421.
        //If 1b is ever relaxed to allow longer tails, THIS is the line that still holds.
        if(bOk && uTail>999999u)
            bOk = false;
        if(bOk)
            uPayload = uTail;
        else
            uClass = 0;                           //fall through to class 9
    }

    //AMENDMENT A5 - classes 4..8 : exactly 5 characters, all digits, first digit 4..8
    //(4=cylinder 5=motor 6=sucker 7=eRecordProcess 8=eOther). The WHOLE code IS the
    //payload, so it is capped at 89999 and AMENDMENT 1's ceiling is satisfied by the code
    //format itself. Band [700000000,899999999] is unused by the 9046LS space - their
    //leading digit is only 1, 2 or 3, measured over all 2738 rows - so opening classes 7
    //and 8 does not touch disjointness.
    if(uClass==0 && iLen==5 && Code[1]>='4' && Code[1]<='8')
    {
        unsigned uWhole = 0;
        bool     bOk    = true;
        for(int i=1; i<=5; i++)
        {
            if(Code[i]<'0' || Code[i]>'9')
            {
                bOk = false;
                break;
            }
            uWhole = uWhole*10u + (unsigned)(Code[i]-'0');
        }
        if(bOk)
        {
            uClass   = (unsigned)(Code[1]-'0');   //'4'..'8' -> 4..8
            uPayload = uWhole;
        }
    }

    //Class 9 : an unregistered free-string alarm. The legacy hash folded to 8 digits keeps
    //the value stable for a given string; the class digit tells the host not to look it up.
    //AMENDMENT 2 (database.cpp CreateSystemAlarmCode) asserts at every startup that no
    //REGISTERED code ever lands here.
    if(uClass==0)
    {
        uClass   = 9;
        uPayload = LegacyAlarmHash(Code) % 100000000u;
    }

    //uClass is now 1..9 and uPayload <= 99999999, so the result lies in
    //[100000000..999999999] : never 0, never fewer than 9 digits, never more, and well
    //inside SIGNED 32-bit too (max 999999999 < 2147483647) - the old "the host must parse
    //this as unsigned 32-bit" warning is void.
    return uClass*100000000u + uPayload;
}
//---------------------------------------------------------------------------
void AlarmReport(AnsiString Code, AnsiString Message, bool bSet)
{
    if(USE_SECS_GEM<=0 || HGem==NULL)
        return;
    //AI(secs-alid-optiond) 20260902 : HT160 self-defined standard S5F1. ALID =
    // ComputeAlarmAlid(Code) = the class-banded 9-digit encoding (Class*100000000 +
    // Payload; 1=JAM 2=WAR 3=MES 4=cylinder 5=motor 6=sucker 7=eRecordProcess 8=eOther,
    // 9=unregistered free string). The host gets a decodable numeric key, and the SAME
    // value appears in the S5F6/S5F8 catalog because that path calls this same pure
    // function - S5F1 ALID == catalog ALID by construction, not by convention.
    // ALTX = "<code> <message>", so the alarm code is always recoverable as the LEADING
    // TOKEN of ALTX - that is the documented recovery for a class-9 (not-in-catalog)
    // ALID. ALCD bit7 = alarm set(1)/clear(0), category bits 0 (unspecified); the ALCD
    // encoding is UNCHANGED by the Option D re-encoding. Mirrors the EventReport glue.
    unsigned alid = ComputeAlarmAlid(Code);
    unsigned char alcd = bSet ? 0x80 : 0x00;
    AnsiString altx = Code;
    if(Message!="")
        altx = Code + " " + Message;
    HGem->SendAlarmS5F1(alid, alcd, altx);
}
//---------------------------------------------------------------------------
