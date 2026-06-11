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
	bShowSortArmPlaceCheck=false;
	iLoaderYSafeDistance=10000;
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
	bShowSortArmPlaceCheck=Ini->ReadBool("Diagnostic", "ShowSortArmPlaceCheck", false);
	iLoaderYSafeDistance=Ini->ReadInteger("Safety", "LoaderYSafeDistance", 10000);
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
	Ini->WriteBool("Diagnostic", "ShowSortArmPlaceCheck", bShowSortArmPlaceCheck);
	Ini->WriteInteger("Safety", "LoaderYSafeDistance", iLoaderYSafeDistance);
	delete Ini;
}
//---------------------------------------------------------------------------
