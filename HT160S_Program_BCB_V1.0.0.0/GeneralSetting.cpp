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
	bUseLotBinSortMode=false;
	for(int a=0;a<6;a++)
		bAutoEnabled[a]=true;
	for(int s=0;s<4;s++)
		bSuckerEnabled[s]=true;
	bShowSortArmPlaceCheck=false;
	iLoaderYSafeDistance=10000;
	bBinDisplayInstalled=false;
	sBinDispComPort="COM5";
	iBinDispDelaySec=5;
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
	bUseLotBinSortMode=Ini->ReadBool("SortMode", "UseLotBinMode", false);
	for(int a=0;a<6;a++)
		bAutoEnabled[a]=Ini->ReadBool("SortMode", "AutoEnabled"+IntToStr(a), true);
	for(int s=0;s<4;s++)
		bSuckerEnabled[s]=Ini->ReadBool("HardwareInstall", "SuckerEnabled"+IntToStr(s), true);
	bShowSortArmPlaceCheck=Ini->ReadBool("Diagnostic", "ShowSortArmPlaceCheck", false);
	iLoaderYSafeDistance=Ini->ReadInteger("Safety", "LoaderYSafeDistance", 10000);
	bBinDisplayInstalled=Ini->ReadBool("BinDisplay", "Installed", false);
	sBinDispComPort=Ini->ReadString("BinDisplay", "ComPort", "COM5");
	iBinDispDelaySec=Ini->ReadInteger("BinDisplay", "DelaySec", 5);
	for(int i=0;i<9;i++)
	{
		sBinDispText[i]=Ini->ReadString("BinDisplay", "Text"+IntToStr(i), sBinDispText[i]);
		iBinDispColor[i]=Ini->ReadInteger("BinDisplay", "Color"+IntToStr(i), iBinDispColor[i]);
	}
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
	Ini->WriteBool("SortMode", "UseLotBinMode", bUseLotBinSortMode);
	for(int a=0;a<6;a++)
		Ini->WriteBool("SortMode", "AutoEnabled"+IntToStr(a), bAutoEnabled[a]);
	for(int s=0;s<4;s++)
		Ini->WriteBool("HardwareInstall", "SuckerEnabled"+IntToStr(s), bSuckerEnabled[s]);
	Ini->WriteBool("Diagnostic", "ShowSortArmPlaceCheck", bShowSortArmPlaceCheck);
	Ini->WriteInteger("Safety", "LoaderYSafeDistance", iLoaderYSafeDistance);
	Ini->WriteBool("BinDisplay", "Installed", bBinDisplayInstalled);
	Ini->WriteString("BinDisplay", "ComPort", sBinDispComPort);
	Ini->WriteInteger("BinDisplay", "DelaySec", iBinDispDelaySec);
	for(int i=0;i<9;i++)
	{
		Ini->WriteString("BinDisplay", "Text"+IntToStr(i), sBinDispText[i]);
		Ini->WriteInteger("BinDisplay", "Color"+IntToStr(i), iBinDispColor[i]);
	}
	delete Ini;
}
//---------------------------------------------------------------------------
