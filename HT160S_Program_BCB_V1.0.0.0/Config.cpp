//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Config.h"
#include "cprod.h"
#include "database.h"
#include <IniFiles.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
THT160Config Config;
//---------------------------------------------------------------------------
__fastcall THT160Config::THT160Config()
{
}
//---------------------------------------------------------------------------
AnsiString THT160Config::GetConfigIniFileName()
{
	AnsiString RootPath=HSys.CurrentDir;
	if(RootPath==AnsiString(""))
		RootPath="..";
	return RootPath+AnsiString("\\config\\config.ini");
}
//---------------------------------------------------------------------------
void THT160Config::SetDefault()
{
	tFunction.UseCCD=false;
	tFunction.RejectCCDfail=false;
	tFunction.UseHitCylinder=false;
	tFunction.HitRetry=0;
	tFunction.UsePreAlignment=false;
}
//---------------------------------------------------------------------------
void THT160Config::Load()
{
	AnsiString FileName=GetConfigIniFileName();
	TIniFile *Ini;

	SetDefault();
	if(!FileExists(FileName))
		return;

	Ini=new TIniFile(FileName);
	tFunction.UseCCD=Ini->ReadBool("Function", "UseCCD", tFunction.UseCCD);
	tFunction.RejectCCDfail=Ini->ReadBool("Function", "RejectCCDfail", tFunction.RejectCCDfail);
	tFunction.UseHitCylinder=Ini->ReadBool("Function", "UseHitCylinder", tFunction.UseHitCylinder);
	tFunction.HitRetry=Ini->ReadInteger("Function", "HitRetry", tFunction.HitRetry);
	tFunction.UsePreAlignment=Ini->ReadBool("Function", "UsePreAlignment", tFunction.UsePreAlignment);
	delete Ini;
}
//---------------------------------------------------------------------------
void THT160Config::Save()
{
	AnsiString FileName=GetConfigIniFileName();
	TIniFile *Ini;

	ForceDirectories(ExtractFilePath(FileName));
	Ini=new TIniFile(FileName);
	Ini->WriteBool("Function", "UseCCD", tFunction.UseCCD);
	Ini->WriteBool("Function", "RejectCCDfail", tFunction.RejectCCDfail);
	Ini->WriteBool("Function", "UseHitCylinder", tFunction.UseHitCylinder);
	Ini->WriteInteger("Function", "HitRetry", tFunction.HitRetry);
	Ini->WriteBool("Function", "UsePreAlignment", tFunction.UsePreAlignment);
	delete Ini;
}
//---------------------------------------------------------------------------
