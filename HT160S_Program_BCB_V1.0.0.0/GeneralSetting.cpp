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
void THT160GeneralSetting::SetDefault()
{
	bColorBinAreaInstalled=false;
	bUseAMR=false;
	for(int z=0;z<9;z++)
		iSimAmrMaxTray[z]=10;
	bUseLotBinSortMode=false;
	for(int a=0;a<6;a++)
		bAutoEnabled[a]=true;
	for(int s=0;s<4;s++)
		bSuckerEnabled[s]=true;
	bShowSortArmPlaceCheck=false;
	bUseTrayDatumModel=true;
	iLoaderYSafeDistance=10000;
	bFrontSeparateInterlock=true;
	bBinDisplayInstalled=false;
	sBinDispComPort="COM5";
	iBinDispDelaySec=5;
	iAmrFeedWaitSec=600;
	iAmrFullWaitSec=600;
	iAmrHandshakeWaitSec=240;
	sMachineModel="HT160S";
	sHandlerID="";
	sSerialNo="";
	iBinDispBaud=9600;
	iBinDispPanelType=0;
	bBinDispLogVerbose=false;
	bBinDispUseMyComm=false;
	iLogRetentionEventDays=365;   // EventLog, WebAPI : audit value, keep ~1 year
	iLogRetentionCommDays=90;     // comm/diagnostic logs : high volume, keep 90d
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
		return;

	Ini=new TIniFile(FileName);
	bColorBinAreaInstalled=Ini->ReadBool("HardwareInstall", "ColorBinAreaInstalled", false);
	bUseAMR=Ini->ReadBool("HardwareInstall", "UseAMR", false);
	for(int z=0;z<9;z++)
		iSimAmrMaxTray[z]=Ini->ReadInteger("SimAMR", "MaxTray"+IntToStr(z), 10);
	bUseLotBinSortMode=Ini->ReadBool("SortMode", "UseLotBinMode", false);
	for(int a=0;a<6;a++)
		bAutoEnabled[a]=Ini->ReadBool("SortMode", "AutoEnabled"+IntToStr(a), true);
	for(int s=0;s<4;s++)
		bSuckerEnabled[s]=Ini->ReadBool("HardwareInstall", "SuckerEnabled"+IntToStr(s), true);
	bShowSortArmPlaceCheck=Ini->ReadBool("Diagnostic", "ShowSortArmPlaceCheck", false);
	bUseTrayDatumModel=Ini->ReadBool("HardwareInstall", "UseTrayDatumModel", true);
	iLoaderYSafeDistance=Ini->ReadInteger("Safety", "LoaderYSafeDistance", 10000);
	bFrontSeparateInterlock=Ini->ReadBool("Safety", "FrontSeparateInterlock", true);
	bBinDisplayInstalled=Ini->ReadBool("BinDisplay", "Installed", false);
	sBinDispComPort=Ini->ReadString("BinDisplay", "ComPort", "COM5");
	iBinDispDelaySec=Ini->ReadInteger("BinDisplay", "DelaySec", 5);
	iAmrFeedWaitSec=Ini->ReadInteger("AGV", "AmrFeedWaitSec", 600);
	iAmrFullWaitSec=Ini->ReadInteger("AGV", "AmrFullWaitSec", 600);
	iAmrHandshakeWaitSec=Ini->ReadInteger("AGV", "AmrHandshakeWaitSec", 240);
	sMachineModel=Ini->ReadString("MachineIdentity", "Model", "HT160S");
	sHandlerID=Ini->ReadString("MachineIdentity", "HandlerID", "");
	sSerialNo=Ini->ReadString("MachineIdentity", "SerialNo", "");
	iBinDispBaud=Ini->ReadInteger("BinDisplay", "Baud", 9600);
	iBinDispPanelType=Ini->ReadInteger("BinDisplay", "PanelType", 0);
	bBinDispLogVerbose=Ini->ReadBool("BinDisplay", "LogVerbose", false);
	bBinDispUseMyComm=Ini->ReadBool("BinDisplay", "UseMyComm", false);
	iLogRetentionEventDays=Ini->ReadInteger("LogRetention", "EventDays", 365);
	iLogRetentionCommDays=Ini->ReadInteger("LogRetention", "CommDays", 90);
	for(int i=0;i<9;i++)
	{
		sBinDispText[i]=Ini->ReadString("BinDisplay", "Text"+IntToStr(i), sBinDispText[i]);
		iBinDispColor[i]=Ini->ReadInteger("BinDisplay", "Color"+IntToStr(i), iBinDispColor[i]);
	}
	// AI(ht160s-agv) clamp : HTimer::Off() returns true at 0 (instant alarm) and wraps
	// negative to ~49.7 days (never alarms). Force a positive lower bound on all AGV waits.
	if(iAmrFeedWaitSec      < 5) iAmrFeedWaitSec      = 5;
	if(iAmrFullWaitSec      < 5) iAmrFullWaitSec      = 5;
	if(iAmrHandshakeWaitSec < 5) iAmrHandshakeWaitSec = 5;
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
	Ini->WriteBool("SortMode", "UseLotBinMode", bUseLotBinSortMode);
	for(int a=0;a<6;a++)
		Ini->WriteBool("SortMode", "AutoEnabled"+IntToStr(a), bAutoEnabled[a]);
	for(int s=0;s<4;s++)
		Ini->WriteBool("HardwareInstall", "SuckerEnabled"+IntToStr(s), bSuckerEnabled[s]);
	Ini->WriteBool("Diagnostic", "ShowSortArmPlaceCheck", bShowSortArmPlaceCheck);
	Ini->WriteBool("HardwareInstall", "UseTrayDatumModel", bUseTrayDatumModel);
	Ini->WriteInteger("Safety", "LoaderYSafeDistance", iLoaderYSafeDistance);
	Ini->WriteBool("Safety", "FrontSeparateInterlock", bFrontSeparateInterlock);
	Ini->WriteBool("BinDisplay", "Installed", bBinDisplayInstalled);
	Ini->WriteString("BinDisplay", "ComPort", sBinDispComPort);
	Ini->WriteInteger("BinDisplay", "DelaySec", iBinDispDelaySec);
	Ini->WriteInteger("AGV", "AmrFeedWaitSec", iAmrFeedWaitSec);
	Ini->WriteInteger("AGV", "AmrFullWaitSec", iAmrFullWaitSec);
	Ini->WriteInteger("AGV", "AmrHandshakeWaitSec", iAmrHandshakeWaitSec);
	Ini->WriteString("MachineIdentity", "Model", sMachineModel);
	Ini->WriteString("MachineIdentity", "HandlerID", sHandlerID);
	Ini->WriteString("MachineIdentity", "SerialNo", sSerialNo);
	Ini->WriteInteger("BinDisplay", "Baud", iBinDispBaud);
	Ini->WriteInteger("BinDisplay", "PanelType", iBinDispPanelType);
	Ini->WriteBool("BinDisplay", "LogVerbose", bBinDispLogVerbose);
	Ini->WriteBool("BinDisplay", "UseMyComm", bBinDispUseMyComm);
	Ini->WriteInteger("LogRetention", "EventDays", iLogRetentionEventDays);
	Ini->WriteInteger("LogRetention", "CommDays", iLogRetentionCommDays);
	for(int i=0;i<9;i++)
	{
		Ini->WriteString("BinDisplay", "Text"+IntToStr(i), sBinDispText[i]);
		Ini->WriteInteger("BinDisplay", "Color"+IntToStr(i), iBinDispColor[i]);
	}
	delete Ini;
}
//---------------------------------------------------------------------------
