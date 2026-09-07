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
	delete Ini;
}
//---------------------------------------------------------------------------
