//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "GeneralSetting.h"
#include "database.h"
#include <IniFiles.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
THT160GeneralSetting GeneralSetting;
//---------------------------------------------------------------------------
__fastcall THT160GeneralSetting::THT160GeneralSetting()
{
	SetDefault();
	//AI(ht160s-whitelist-override) 20260717 : the WhiteList overlay is per-work-order RUNTIME state,
	//NOT a General.ini config field, so its boot-clear lives HERE (ctor), not in SetDefault(). Load()
	//calls SetDefault() on every maintenance reopen / MCU reload; clearing the overlay inside
	//SetDefault() would silently disarm a running WhiteList lot (adversarial review 2026-07-17 BLOCKER).
	//RestoreLastWorkOrder restores the overlay from WhiteListOverlay.ini at boot.
	bWhiteListActive=false;
	RecomputeEffectiveSortMode();
}
//---------------------------------------------------------------------------
AnsiString THT160GeneralSetting::GetGeneralIniFileName()
{
	AnsiString RootPath=HSys.CurrentDir;
	if(RootPath==AnsiString(""))
		RootPath="..";
	return RootPath+AnsiString("\\system\\General.ini");
}
//---------------------------------------------------------------------------
//AI(ht160s-whitelist-override) 20260717 : the WhiteList overlay is per-work-order runtime state
//(NOT sticky machine config), so it lives in its own tiny ini and rides the work-order lifecycle
//(written at Lot Start / Lot End / fresh-clear, read on resume). Keeping it out of General.ini
//avoids the config-tier violation and the "Load() re-runs on maintenance reopen" stale-read trap.
static AnsiString GetWhiteListOverlayIniFileName()
{
	AnsiString RootPath=HSys.CurrentDir;
	if(RootPath==AnsiString(""))
		RootPath="..";
	return RootPath+AnsiString("\\system\\WhiteListOverlay.ini");
}
//---------------------------------------------------------------------------
void THT160GeneralSetting::SaveWhiteListOverlay()
{
	AnsiString fn=GetWhiteListOverlayIniFileName();
	ForceDirectories(ExtractFilePath(fn));
	TIniFile *Ini=new TIniFile(fn);
	try
	{
		Ini->WriteBool("WhiteList", "Active", bWhiteListActive);
	}
	__finally
	{
		delete Ini;
	}
}
//---------------------------------------------------------------------------
void THT160GeneralSetting::LoadWhiteListOverlay()
{
	AnsiString fn=GetWhiteListOverlayIniFileName();
	bool b=false;
	if(FileExists(fn))
	{
		TIniFile *Ini=new TIniFile(fn);
		try
		{
			b=Ini->ReadBool("WhiteList", "Active", false);
		}
		__finally
		{
			delete Ini;
		}
	}
	SetWhiteListActive(b);
}
//---------------------------------------------------------------------------
void THT160GeneralSetting::SetDefault()
{
	bColorBinAreaInstalled=false;
	bUseAMR=false;
	for(int z=0;z<9;z++)
		iSimAmrMaxTray[z]=10;
	// AI(ht160s-loader-worktray-count) 20260713 : header defaults match the previously
	// hardcoded FmtCarKinds args - Loader/Auto 1 cover + 1 identity, Empty 1 cover + 0
	// identity, Color whole-car identity (-1).
	for(int z=0;z<9;z++)
	{
		iAmrCoverTray[z]=(z==2)?0:1;
		iAmrIdentityTray[z]=(z==1)?0:((z==2)?-1:1);
	}
	iSortMode=smNormal;
	//AI(ht160s-whitelist-override) 20260717 : do NOT touch bWhiteListActive / iEffectiveSortMode here -
	//SetDefault() is also called from Load() (maintenance reopen / MCU reload) and must not disarm a
	//running lot. The boot-clear is in the ctor; every SetDefault() caller recomputes the mirror after.
	bUsePredictiveAutoSupply=false;
	bUseAmrRecoveryDivert=false;
	bSkipUnknown2DAlarm=false;   //AI(ht160s-whitelist) 20260727 : F1 default OFF (legacy WAR0475 modal)
	for(int a=0;a<6;a++)
		bAutoEnabled[a]=true;
	for(int s=0;s<4;s++)
		bSuckerEnabled[s]=true;
	bSuck2QuadVacuum=false;
	bShowSortArmPlaceCheck=false;
	iSortArmXDatumBias=-1000;
	iSortArmYDatumBias=-1000;
	iSortArmPickRetryCount=3;
	iSortArmZMoveGuardMs=8000;   //AI(bcb6-172align) 20260723 : conservative default (ms)
	bSortArmAutoSkipOnPickFail=false;
	iLoaderYSafeDistance=10000;
	iEmptyDestackSettleMs=500;
	iColorDestackSettleMs=500;
	iLoaderDestackSettleMs=1000;
	iAutoPushConfirmSettleMs=500;
	iAutoConcurrency=0;   //AI(auto-per-station) 20260802 : 0 = legacy single ladder (see header)
	bAskSkipICCount=false;   //AI(secs-skipiccount) 20260802 : no extra operator dialog by default
	//AI(secs-e30-gate) 20260803 : boot ON-LINE REMOTE. Off-Line by default would lock the host out
	//(E30 accepts S1F17 only from HOST OFF-LINE) - see the header for the field evidence.
	iInitialControlState=5;
	bAcceptHostOnlineRequest=true;
	iAutoDischargePostYSettleMs=500;
	iHomeReacquireOffsetCnt=100;   //AI(ht160s-home-resume-w3c) : +1mm default
	iHomeDrainTimeoutSec=15;
	iRise1SettleWaitSec=10;   //AI(ht160s-anti-ghost-d) 20260720
	iStuckSnapshotSec=300;   //AI(ht160s-obsv-p1) : 5 min
	iAutoFrontRiseDwellMs=500;
	iAutoCleanOutRiseDwellMs=500;
	iTrayArmClampSettleMs=300;
	iEmptyFeedClampSettleMs=500;
	iColorFeedClampSettleMs=500;
	bFrontSeparateInterlock=true;
	bBinDisplayInstalled=false;
	sBinDispComPort="COM5";
	iBinDispDelaySec=5;
	iAmrFeedWaitSec=600;
	iAmrHandshakeWaitSec=240;
	iAgvTimeoutSec=300;   //AI(amr-unmanned W1) 20260721 : unified AGV handshake timeout -> WAR0962
	sMachineModel="HT160S";
	sHandlerID="";
	sSerialNo="";
	sOperatorID="Operator";   //AI(secs-operatorid) 20260803 : SVID/ECID 1007 default, never blank
	iBinDispBaud=9600;
	iBinDispPanelType=0;
	bBinDispLogVerbose=false;
	bBinDispUseMyComm=false;
	iLogRetentionEventDays=365;   // EventLog, WebAPI : audit value, keep ~1 year
	iLogRetentionCommDays=90;     // comm/diagnostic logs : high volume, keep 90d
	iLogRetentionDiscardedDays=90; // LotStory Discarded work-order backups, keep 90d
	iLogRetentionUPHLogDays=180;   // UPHLog per-lot folders, keep ~6 months
	iLogRetentionProdDailyDays=365; // Production_Log Daily aggregate : keep ~1 year (per-lot files permanent)
	iUphMinSampleIC=0;             // 0 = auto (one full tray); hide UPH below this IC count
	// Defaults mirror old-160: Empty=E, Loader=L, Auto1..6=1..6, Color=C.
	{
		const char *DefText[9]={"E","L","1","2","3","4","5","6","C"};
		for(int i=0;i<9;i++)
		{
			sBinDispText[i]=DefText[i];
			iBinDispColor[i]=3;
		}
	}
}
//---------------------------------------------------------------------------
void THT160GeneralSetting::Load()
{
	AnsiString FileName=GetGeneralIniFileName();
	TIniFile *Ini;

	SetDefault();
	if(!FileExists(FileName))
	{
		RecomputeEffectiveSortMode();   // no ini : keep the effective mirror consistent (overlay preserved)
		return;
	}

	Ini=new TIniFile(FileName);
	bColorBinAreaInstalled=Ini->ReadBool("HardwareInstall", "ColorBinAreaInstalled", false);
	bUseAMR=Ini->ReadBool("HardwareInstall", "UseAMR", false);
	for(int z=0;z<9;z++)
		iSimAmrMaxTray[z]=Ini->ReadInteger("SimAMR", "MaxTray"+IntToStr(z), 10);
	// AI(ht160s-loader-worktray-count) 20260713 : per-zone AMR header composition. Defaults
	// preserve the previous hardcoded FmtCarKinds split so an ini without [AMR] is unchanged.
	for(int z=0;z<9;z++)
	{
		int cvDef=(z==2)?0:1;
		int idDef=(z==1)?0:((z==2)?-1:1);
		iAmrCoverTray[z]=Ini->ReadInteger("AMR", "CoverTray"+IntToStr(z), cvDef);
		iAmrIdentityTray[z]=Ini->ReadInteger("AMR", "IdentityTray"+IntToStr(z), idDef);
	}
	// AI(ht160s-lotpassfail) 20260709 : Mode-first back-compat. The new [SortMode] Mode
	// wins; legacy [SortMode] UseLotBinMode is only the default when Mode is absent (so an
	// upgraded machine that ran LotBin keeps LotBin, but an explicit Mode=2 is never
	// clobbered back to 1). Clamp so a hand-edited/corrupt value fails safe to Normal.
	{
		int LegacyMode=Ini->ReadBool("SortMode", "UseLotBinMode", false)?smLotBin:smNormal;
		iSortMode=Ini->ReadInteger("SortMode", "Mode", LegacyMode);
		// AI(ht160s-whitelist-override) 20260717 : base mode is now {Normal,LotBin,LotPassFail}
		// only - WhiteList is a per-lot overlay (bWhiteListActive), never a persisted base. A
		// stale Mode=3 from the old sticky WhiteList design migrates to Normal (same static
		// routing); the overlay is work-order state (restored by RestoreLastWorkOrder), never
		// read here, so WhiteList must be re-commanded at the next Lot Start. Do NOT reset
		// bWhiteListActive here : Load() also runs on every maintenance reopen, and clearing it
		// would silently disarm a running WhiteList lot.
		if(iSortMode<smNormal || iSortMode>smLotPassFail)
			iSortMode=smNormal;
	}
	RecomputeEffectiveSortMode();   // keep the effective-mode mirror in step with the reloaded base

	bUsePredictiveAutoSupply=Ini->ReadBool("SortMode", "UsePredictiveAutoSupply", false);
	bUseAmrRecoveryDivert=Ini->ReadBool("SortMode", "UseAmrRecoveryDivert", false);
	bSkipUnknown2DAlarm=Ini->ReadBool("SortMode", "SkipUnknown2DAlarm", false);   //AI(ht160s-whitelist) 20260727 : F1
	for(int a=0;a<6;a++)
		bAutoEnabled[a]=Ini->ReadBool("SortMode", "AutoEnabled"+IntToStr(a), true);
	for(int s=0;s<4;s++)
		bSuckerEnabled[s]=Ini->ReadBool("HardwareInstall", "SuckerEnabled"+IntToStr(s), true);
	bSuck2QuadVacuum=Ini->ReadBool("HardwareInstall", "Suck2QuadVacuum", false);
	if(bSuck2QuadVacuum)
	{
		// Quad mode has only the Suck2 nozzle installed : force the pick mask to
		// Nozzle2-only on EVERY load (maintenance reopens call Load() too) so a
		// hand-edited SuckerEnabled key can never route work to a nozzle that is
		// not there. Not written back to the ini here.
		for(int s=0;s<4;s++)
			bSuckerEnabled[s]=(s==1);
	}
	bShowSortArmPlaceCheck=Ini->ReadBool("Diagnostic", "ShowSortArmPlaceCheck", false);
	iSortArmXDatumBias=Ini->ReadInteger("SortArm", "XDatumBias", -1000);
	iSortArmYDatumBias=Ini->ReadInteger("SortArm", "YDatumBias", -1000);
	iSortArmPickRetryCount=Ini->ReadInteger("SortArm", "PickRetryCount", 3);
	iSortArmZMoveGuardMs=Ini->ReadInteger("SortArm", "ZMoveGuardMs", 8000);
	bSortArmAutoSkipOnPickFail=Ini->ReadBool("SortArm", "AutoSkipOnPickFail", false);
	if(iSortArmPickRetryCount<0)
		iSortArmPickRetryCount=0;
	if(iSortArmZMoveGuardMs<1000)
		iSortArmZMoveGuardMs=1000;
	iLoaderYSafeDistance=Ini->ReadInteger("Safety", "LoaderYSafeDistance", 10000);
	iEmptyDestackSettleMs=Ini->ReadInteger("SettleDelay", "EmptyDestackSettleMs", 500);
	iColorDestackSettleMs=Ini->ReadInteger("SettleDelay", "ColorDestackSettleMs", 500);
	iLoaderDestackSettleMs=Ini->ReadInteger("SettleDelay", "LoaderDestackSettleMs", 1000);
	iAutoPushConfirmSettleMs=Ini->ReadInteger("SettleDelay", "AutoPushConfirmSettleMs", 500);
	//AI(auto-per-station) 20260802 : clamped 0..6 so a typo cannot ask for a station that
	//does not exist. 0 keeps the legacy ladder, which is the on-site rollback value.
	iAutoConcurrency=Ini->ReadInteger("Auto", "Concurrency", 0);
	bAskSkipICCount=Ini->ReadBool("SECS", "AskSkipICCount", false);
	iInitialControlState=Ini->ReadInteger("SECS", "InitialControlState", 5);
	if(iInitialControlState!=1 && iInitialControlState!=4 && iInitialControlState!=5)
		iInitialControlState=5;
	bAcceptHostOnlineRequest=Ini->ReadBool("SECS", "AcceptHostOnlineRequest", true);
	if(iAutoConcurrency<0) iAutoConcurrency=0;
	if(iAutoConcurrency>6) iAutoConcurrency=6;
	iAutoDischargePostYSettleMs=Ini->ReadInteger("SettleDelay", "AutoDischargePostYSettleMs", 500);
	iHomeReacquireOffsetCnt=Ini->ReadInteger("HomeResume", "ReacquireOffsetCnt", 100);
	iHomeDrainTimeoutSec=Ini->ReadInteger("HomeResume", "DrainTimeoutSec", 15);
	iRise1SettleWaitSec=Ini->ReadInteger("HomeResume", "Rise1SettleWaitSec", 10);
	iStuckSnapshotSec=Ini->ReadInteger("Observability", "StuckSnapshotSec", 300);
	iAutoFrontRiseDwellMs=Ini->ReadInteger("SettleDelay", "AutoFrontRiseDwellMs", 500);
	iAutoCleanOutRiseDwellMs=Ini->ReadInteger("SettleDelay", "AutoCleanOutRiseDwellMs", 500);
	iTrayArmClampSettleMs=Ini->ReadInteger("SettleDelay", "TrayArmClampSettleMs", 300);
	iEmptyFeedClampSettleMs=Ini->ReadInteger("SettleDelay", "EmptyFeedClampSettleMs", 500);
	iColorFeedClampSettleMs=Ini->ReadInteger("SettleDelay", "ColorFeedClampSettleMs", 500);
	bFrontSeparateInterlock=Ini->ReadBool("Safety", "FrontSeparateInterlock", true);
	bBinDisplayInstalled=Ini->ReadBool("BinDisplay", "Installed", false);
	sBinDispComPort=Ini->ReadString("BinDisplay", "ComPort", "COM5");
	iBinDispDelaySec=Ini->ReadInteger("BinDisplay", "DelaySec", 5);
	iAmrFeedWaitSec=Ini->ReadInteger("AGV", "AmrFeedWaitSec", 600);
	iAmrHandshakeWaitSec=Ini->ReadInteger("AGV", "AmrHandshakeWaitSec", 240);
	iAgvTimeoutSec=Ini->ReadInteger("AGV", "AgvTimeoutSec", 300);   //AI(amr-unmanned W1) 20260721
	sMachineModel=Ini->ReadString("MachineIdentity", "Model", "HT160S");
	sHandlerID=Ini->ReadString("MachineIdentity", "HandlerID", "");
	sSerialNo=Ini->ReadString("MachineIdentity", "SerialNo", "");
	sOperatorID=Ini->ReadString("MachineIdentity", "OperatorID", "Operator");   //AI(secs-operatorid) 20260803 : SVID/ECID 1007
	iBinDispBaud=Ini->ReadInteger("BinDisplay", "Baud", 9600);
	iBinDispPanelType=Ini->ReadInteger("BinDisplay", "PanelType", 0);
	bBinDispLogVerbose=Ini->ReadBool("BinDisplay", "LogVerbose", false);
	bBinDispUseMyComm=Ini->ReadBool("BinDisplay", "UseMyComm", false);
	iLogRetentionEventDays=Ini->ReadInteger("LogRetention", "EventDays", 365);
	iLogRetentionCommDays=Ini->ReadInteger("LogRetention", "CommDays", 90);
	iLogRetentionDiscardedDays=Ini->ReadInteger("LogRetention", "DiscardedDays", 90);
	iLogRetentionUPHLogDays=Ini->ReadInteger("LogRetention", "UPHLogDays", 180);
	iLogRetentionProdDailyDays=Ini->ReadInteger("LogRetention", "ProdDailyDays", 365);
	iUphMinSampleIC=Ini->ReadInteger("UPH", "MinSampleIC", 0);
	for(int i=0;i<9;i++)
	{
		sBinDispText[i]=Ini->ReadString("BinDisplay", "Text"+IntToStr(i), sBinDispText[i]);
		iBinDispColor[i]=Ini->ReadInteger("BinDisplay", "Color"+IntToStr(i), iBinDispColor[i]);
	}
	// AI(ht160s-agv) clamp : HTimer::Off() returns true at 0 (instant alarm) and wraps
	// negative to ~49.7 days (never alarms). Force a positive lower bound on all AGV waits.
	if(iAmrFeedWaitSec      < 5) iAmrFeedWaitSec      = 5;
	if(iAmrHandshakeWaitSec < 5) iAmrHandshakeWaitSec = 5;
	if(iRise1SettleWaitSec  < 5) iRise1SettleWaitSec  = 5;   //AI(ht160s-anti-ghost-d) 20260720 : same HTimer 0=instant / negative=49.7d footgun as the AGV waits
	if(iAgvTimeoutSec       < 5) iAgvTimeoutSec       = 5;   //AI(amr-unmanned W1) 20260721 : same HTimer footgun
	delete Ini;
}
//---------------------------------------------------------------------------
void THT160GeneralSetting::Save()
{
	AnsiString FileName=GetGeneralIniFileName();
	TIniFile *Ini;

	ForceDirectories(ExtractFilePath(FileName));
	Ini=new TIniFile(FileName);
	Ini->WriteBool("HardwareInstall", "ColorBinAreaInstalled", bColorBinAreaInstalled);
	Ini->WriteBool("HardwareInstall", "UseAMR", bUseAMR);
	for(int z=0;z<9;z++)
		Ini->WriteInteger("SimAMR", "MaxTray"+IntToStr(z), iSimAmrMaxTray[z]);
	// AI(ht160s-loader-worktray-count) 20260713 : persist per-zone AMR header composition.
	for(int z=0;z<9;z++)
	{
		Ini->WriteInteger("AMR", "CoverTray"+IntToStr(z), iAmrCoverTray[z]);
		Ini->WriteInteger("AMR", "IdentityTray"+IntToStr(z), iAmrIdentityTray[z]);
	}
	Ini->WriteInteger("SortMode", "Mode", iSortMode);
	// AI(ht160s-lotpassfail) 20260709 : keep the legacy bool key in sync so an older exe
	// (which only reads UseLotBinMode) still lands on a sane mode - LotBin stays LotBin,
	// both Normal and LotPassFail map to false=Normal (safe downgrade).
	Ini->WriteBool("SortMode", "UseLotBinMode", iSortMode==smLotBin);
	Ini->WriteBool("SortMode", "UsePredictiveAutoSupply", bUsePredictiveAutoSupply);
	Ini->WriteBool("SortMode", "UseAmrRecoveryDivert", bUseAmrRecoveryDivert);
	Ini->WriteBool("SortMode", "SkipUnknown2DAlarm", bSkipUnknown2DAlarm);   //AI(ht160s-whitelist) 20260727 : F1
	for(int a=0;a<6;a++)
		Ini->WriteBool("SortMode", "AutoEnabled"+IntToStr(a), bAutoEnabled[a]);
	for(int s=0;s<4;s++)
		Ini->WriteBool("HardwareInstall", "SuckerEnabled"+IntToStr(s), bSuckerEnabled[s]);
	Ini->WriteBool("HardwareInstall", "Suck2QuadVacuum", bSuck2QuadVacuum);
	Ini->WriteBool("Diagnostic", "ShowSortArmPlaceCheck", bShowSortArmPlaceCheck);
	Ini->WriteInteger("SortArm", "XDatumBias", iSortArmXDatumBias);
	Ini->WriteInteger("SortArm", "YDatumBias", iSortArmYDatumBias);
	Ini->WriteInteger("SortArm", "PickRetryCount", iSortArmPickRetryCount);
	Ini->WriteInteger("SortArm", "ZMoveGuardMs", iSortArmZMoveGuardMs);
	Ini->WriteBool("SortArm", "AutoSkipOnPickFail", bSortArmAutoSkipOnPickFail);
	Ini->WriteInteger("Safety", "LoaderYSafeDistance", iLoaderYSafeDistance);
	Ini->WriteInteger("SettleDelay", "EmptyDestackSettleMs", iEmptyDestackSettleMs);
	Ini->WriteInteger("SettleDelay", "ColorDestackSettleMs", iColorDestackSettleMs);
	Ini->WriteInteger("SettleDelay", "LoaderDestackSettleMs", iLoaderDestackSettleMs);
	Ini->WriteInteger("SettleDelay", "AutoPushConfirmSettleMs", iAutoPushConfirmSettleMs);
	Ini->WriteInteger("Auto", "Concurrency", iAutoConcurrency);
	Ini->WriteBool("SECS", "AskSkipICCount", bAskSkipICCount);
	Ini->WriteInteger("SECS", "InitialControlState", iInitialControlState);
	Ini->WriteBool("SECS", "AcceptHostOnlineRequest", bAcceptHostOnlineRequest);
	Ini->WriteInteger("SettleDelay", "AutoDischargePostYSettleMs", iAutoDischargePostYSettleMs);
	Ini->WriteInteger("HomeResume", "ReacquireOffsetCnt", iHomeReacquireOffsetCnt);
	Ini->WriteInteger("HomeResume", "DrainTimeoutSec", iHomeDrainTimeoutSec);
	Ini->WriteInteger("HomeResume", "Rise1SettleWaitSec", iRise1SettleWaitSec);
	Ini->WriteInteger("Observability", "StuckSnapshotSec", iStuckSnapshotSec);
	Ini->WriteInteger("SettleDelay", "AutoFrontRiseDwellMs", iAutoFrontRiseDwellMs);
	Ini->WriteInteger("SettleDelay", "AutoCleanOutRiseDwellMs", iAutoCleanOutRiseDwellMs);
	Ini->WriteInteger("SettleDelay", "TrayArmClampSettleMs", iTrayArmClampSettleMs);
	Ini->WriteInteger("SettleDelay", "EmptyFeedClampSettleMs", iEmptyFeedClampSettleMs);
	Ini->WriteInteger("SettleDelay", "ColorFeedClampSettleMs", iColorFeedClampSettleMs);
	Ini->WriteBool("Safety", "FrontSeparateInterlock", bFrontSeparateInterlock);
	Ini->WriteBool("BinDisplay", "Installed", bBinDisplayInstalled);
	Ini->WriteString("BinDisplay", "ComPort", sBinDispComPort);
	Ini->WriteInteger("BinDisplay", "DelaySec", iBinDispDelaySec);
	Ini->WriteInteger("AGV", "AmrFeedWaitSec", iAmrFeedWaitSec);
	Ini->WriteInteger("AGV", "AmrHandshakeWaitSec", iAmrHandshakeWaitSec);
	Ini->WriteInteger("AGV", "AgvTimeoutSec", iAgvTimeoutSec);   //AI(amr-unmanned W1) 20260721
	Ini->WriteString("MachineIdentity", "Model", sMachineModel);
	Ini->WriteString("MachineIdentity", "HandlerID", sHandlerID);
	Ini->WriteString("MachineIdentity", "SerialNo", sSerialNo);
	Ini->WriteString("MachineIdentity", "OperatorID", sOperatorID);   //AI(secs-operatorid) 20260803 : SVID/ECID 1007
	Ini->WriteInteger("BinDisplay", "Baud", iBinDispBaud);
	Ini->WriteInteger("BinDisplay", "PanelType", iBinDispPanelType);
	Ini->WriteBool("BinDisplay", "LogVerbose", bBinDispLogVerbose);
	Ini->WriteBool("BinDisplay", "UseMyComm", bBinDispUseMyComm);
	Ini->WriteInteger("LogRetention", "EventDays", iLogRetentionEventDays);
	Ini->WriteInteger("LogRetention", "CommDays", iLogRetentionCommDays);
	Ini->WriteInteger("LogRetention", "DiscardedDays", iLogRetentionDiscardedDays);
	Ini->WriteInteger("LogRetention", "UPHLogDays", iLogRetentionUPHLogDays);
	Ini->WriteInteger("LogRetention", "ProdDailyDays", iLogRetentionProdDailyDays);
	Ini->WriteInteger("UPH", "MinSampleIC", iUphMinSampleIC);
	for(int i=0;i<9;i++)
	{
		Ini->WriteString("BinDisplay", "Text"+IntToStr(i), sBinDispText[i]);
		Ini->WriteInteger("BinDisplay", "Color"+IntToStr(i), iBinDispColor[i]);
	}
	delete Ini;
}
//---------------------------------------------------------------------------
