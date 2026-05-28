//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "CosFunction.h"
#include "database.h"
#include <IniFiles.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
HT160S_CUSTOMER_FUNCTION CosFunction;
THT160RecipeManager RecipeManager;
THT160BinAreaMap BinAreaMap;
//---------------------------------------------------------------------------
static AnsiString GetCosFunctionMachineOptionFileName()
{
	AnsiString RootPath=HSys.CurrentDir;
	if(RootPath==AnsiString(""))
		RootPath="..";
	return RootPath+AnsiString("\\system\\machine_option.ini");
}
//---------------------------------------------------------------------------
__fastcall THT160RecipeManager::THT160RecipeManager()
{
	CurrentRecipeName="Default";
}
//---------------------------------------------------------------------------
AnsiString THT160RecipeManager::NormalizeRecipeName(AnsiString RecipeName)
{
	AnsiString Result=RecipeName.Trim();
	const char *InvalidChars="\\/:*?\"<>|";
	int Index;
	int InvalidIndex;

	if(Result==AnsiString(""))
		Result="Default";

	for(Index=1; Index<=Result.Length(); Index++)
	{
		for(InvalidIndex=0; InvalidChars[InvalidIndex]!=0; InvalidIndex++)
		{
			if(Result[Index]==InvalidChars[InvalidIndex])
			{
				Result[Index]='_';
				break;
			}
		}
	}
	return Result;
}
//---------------------------------------------------------------------------
void THT160RecipeManager::SetCurrentRecipeName(AnsiString RecipeName)
{
	CurrentRecipeName=NormalizeRecipeName(RecipeName);
}
//---------------------------------------------------------------------------
AnsiString THT160RecipeManager::GetCurrentRecipeName()
{
	return NormalizeRecipeName(CurrentRecipeName);
}
//---------------------------------------------------------------------------
AnsiString THT160RecipeManager::GetDataRootPath()
{
	AnsiString RootPath=HSys.CurrentDir;
	if(RootPath==AnsiString(""))
		RootPath="..";
	return RootPath+AnsiString("\\data");
}
//---------------------------------------------------------------------------
AnsiString THT160RecipeManager::GetRecipeDirName()
{
	return GetRecipeDirName(GetCurrentRecipeName());
}
//---------------------------------------------------------------------------
AnsiString THT160RecipeManager::GetRecipeDirName(AnsiString RecipeName)
{
	return GetDataRootPath()+AnsiString("\\")+NormalizeRecipeName(RecipeName);
}
//---------------------------------------------------------------------------
AnsiString THT160RecipeManager::GetRecipeFileName(AnsiString FileName)
{
	return GetRecipeDirName()+AnsiString("\\")+FileName;
}
//---------------------------------------------------------------------------
AnsiString THT160RecipeManager::GetRecipeFileName(AnsiString RecipeName, AnsiString FileName)
{
	return GetRecipeDirName(RecipeName)+AnsiString("\\")+FileName;
}
//---------------------------------------------------------------------------
AnsiString THT160RecipeManager::GetLastSetFileName()
{
	AnsiString RootPath=HSys.CurrentDir;
	if(RootPath==AnsiString(""))
		RootPath="..";
	return RootPath+AnsiString("\\system\\lastset.ini");
}
//---------------------------------------------------------------------------
void THT160RecipeManager::EnsureCurrentRecipeDir()
{
	ForceDirectories(GetRecipeDirName());
}
//---------------------------------------------------------------------------
void THT160RecipeManager::LoadLastRecipeName()
{
	AnsiString FileName=GetLastSetFileName();
	AnsiString RecipeName="";
	TIniFile *Ini;

	if(FileExists(FileName))
	{
		Ini=new TIniFile(FileName);
		RecipeName=Ini->ReadString("LastSet", "RecipeName", "");
		if(RecipeName==AnsiString(""))
			RecipeName=Ini->ReadString("LastSet", "cob_MainWorkFile", "");
		delete Ini;
	}
	SetCurrentRecipeName(RecipeName);
}
//---------------------------------------------------------------------------
void THT160RecipeManager::SaveLastRecipeName()
{
	TIniFile *Ini;
	AnsiString FileName=GetLastSetFileName();

	ForceDirectories(ExtractFilePath(FileName));
	Ini=new TIniFile(FileName);
	Ini->WriteString("LastSet", "RecipeName", GetCurrentRecipeName());
	Ini->WriteString("LastSet", "cob_MainWorkFile", GetCurrentRecipeName());
	delete Ini;
}
//---------------------------------------------------------------------------
bool THT160RecipeManager::RecipeExists(AnsiString RecipeName)
{
	return DirectoryExists(GetRecipeDirName(RecipeName));
}
//---------------------------------------------------------------------------
bool THT160RecipeManager::CreateRecipe(AnsiString RecipeName)
{
	AnsiString Name=NormalizeRecipeName(RecipeName);

	if(RecipeExists(Name))
		return false;
	return ForceDirectories(GetRecipeDirName(Name));
}
//---------------------------------------------------------------------------
bool THT160RecipeManager::CopyRecipe(AnsiString SourceRecipeName, AnsiString DestRecipeName)
{
	AnsiString SourceName=NormalizeRecipeName(SourceRecipeName);
	AnsiString DestName=NormalizeRecipeName(DestRecipeName);
	AnsiString DestDir;

	if(SourceName.UpperCase()==DestName.UpperCase())
		return false;
	if(!RecipeExists(SourceName) || RecipeExists(DestName))
		return false;

	DestDir=GetRecipeDirName(DestName);
	if(!CopyDirectory(GetRecipeDirName(SourceName), DestDir))
	{
		DeleteDirectory(DestDir);
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------
bool THT160RecipeManager::DeleteRecipe(AnsiString RecipeName)
{
	AnsiString Name=NormalizeRecipeName(RecipeName);

	if(Name.UpperCase()==GetCurrentRecipeName().UpperCase())
		return false;
	if(!RecipeExists(Name))
		return false;
	return DeleteDirectory(GetRecipeDirName(Name));
}
//---------------------------------------------------------------------------
bool THT160RecipeManager::CopyDirectory(AnsiString SourceDir, AnsiString DestDir)
{
	TSearchRec Search;
	AnsiString SourceName;
	AnsiString DestName;
	bool Result=true;
	int FindResult;

	if(!DirectoryExists(SourceDir))
		return false;
	if(!ForceDirectories(DestDir))
		return false;

	FindResult=FindFirst(SourceDir+AnsiString("\\*.*"), faAnyFile, Search);
	if(FindResult==0)
	{
		while(FindResult==0)
		{
			if(Search.Name!=AnsiString(".") && Search.Name!=AnsiString(".."))
			{
				SourceName=SourceDir+AnsiString("\\")+Search.Name;
				DestName=DestDir+AnsiString("\\")+Search.Name;
				if((Search.Attr & faDirectory)!=0)
				{
					if(!CopyDirectory(SourceName, DestName))
					{
						Result=false;
						break;
					}
				}
				else
				{
					if(!CopyFile(SourceName.c_str(), DestName.c_str(), false))
					{
						Result=false;
						break;
					}
				}
			}
			FindResult=FindNext(Search);
		}
		FindClose(Search);
	}
	return Result;
}
//---------------------------------------------------------------------------
bool THT160RecipeManager::DeleteDirectory(AnsiString DirName)
{
	TSearchRec Search;
	AnsiString ItemName;
	bool Result=true;
	int FindResult;

	if(!DirectoryExists(DirName))
		return true;

	FindResult=FindFirst(DirName+AnsiString("\\*.*"), faAnyFile, Search);
	if(FindResult==0)
	{
		while(FindResult==0)
		{
			if(Search.Name!=AnsiString(".") && Search.Name!=AnsiString(".."))
			{
				ItemName=DirName+AnsiString("\\")+Search.Name;
				if((Search.Attr & faDirectory)!=0)
				{
					if(!DeleteDirectory(ItemName))
					{
						Result=false;
						break;
					}
				}
				else
				{
					if(!DeleteFile(ItemName))
					{
						Result=false;
						break;
					}
				}
			}
			FindResult=FindNext(Search);
		}
		FindClose(Search);
	}
	if(Result)
		Result=RemoveDir(DirName);
	return Result;
}
//---------------------------------------------------------------------------
__fastcall THT160BinAreaMap::THT160BinAreaMap()
{
	ErrorBinArea=HT160_DEFAULT_ERROR_BIN_AREA;
	Clear();
}
//---------------------------------------------------------------------------
bool THT160BinAreaMap::IsValidBin(int Bin)
{
	return (Bin>0 && Bin<HT160_BIN_AREA_NORMAL_MAX_BIN);
}
//---------------------------------------------------------------------------
bool THT160BinAreaMap::IsValidArea(int Area)
{
	return (Area>eHT160BinAreaNotUse && Area<eHT160BinAreaTotal && Area<HT160_BIN_AREA_MAX_AREA);
}
//---------------------------------------------------------------------------
int THT160BinAreaMap::GetErrorBinIndex(int Bin)
{
	if(Bin==HT160_BIN_ERROR_2D_SCAN_FAIL)
		return 0;
	if(Bin==HT160_BIN_ERROR_NO_BIN_SETTING)
		return 1;
	return -1;
}
//---------------------------------------------------------------------------
void THT160BinAreaMap::Clear()
{
	int Index;

	for(Index=0; Index<HT160_BIN_AREA_MAX_BIN; Index++)
		BinToArea[Index]=eHT160BinAreaNotUse;
	for(Index=0; Index<HT160_BIN_AREA_MAX_AREA; Index++)
		AreaToBin[Index]=0;
	for(Index=0; Index<HT160_BIN_ERROR_REASON_COUNT; Index++)
		ErrorBinToArea[Index]=0;
	ErrorBinArea=HT160_DEFAULT_ERROR_BIN_AREA;
}
//---------------------------------------------------------------------------
bool THT160BinAreaMap::AddBinArea(int Bin, int Area)
{
	int OldArea;
	int OldBin;

	if(!IsValidBin(Bin) || !IsValidArea(Area) || !IsAreaEnabled(Area))
		return false;

	OldArea=BinToArea[Bin];
	if(IsValidArea(OldArea))
		AreaToBin[OldArea]=0;

	OldBin=AreaToBin[Area];
	if(IsValidBin(OldBin))
		BinToArea[OldBin]=eHT160BinAreaNotUse;

	BinToArea[Bin]=Area;
	AreaToBin[Area]=Bin;
	return true;
}
//---------------------------------------------------------------------------
bool THT160BinAreaMap::SetBinByArea(int Bin, int Area)
{
	return AddBinArea(Bin, Area);
}
//---------------------------------------------------------------------------
void THT160BinAreaMap::RemoveBin(int Bin)
{
	int Area;

	if(!IsValidBin(Bin))
		return;

	Area=BinToArea[Bin];
	BinToArea[Bin]=eHT160BinAreaNotUse;
	if(IsValidArea(Area) && AreaToBin[Area]==Bin)
		AreaToBin[Area]=0;
}
//---------------------------------------------------------------------------
void THT160BinAreaMap::RemoveArea(int Area)
{
	int Bin;

	if(!IsValidArea(Area))
		return;

	Bin=AreaToBin[Area];
	AreaToBin[Area]=0;
	if(IsValidBin(Bin) && BinToArea[Bin]==Area)
		BinToArea[Bin]=eHT160BinAreaNotUse;
}
//---------------------------------------------------------------------------
int THT160BinAreaMap::GetAreaByBin(int Bin)
{
	if(IsErrorBin(Bin))
		return GetAreaByErrorBin(Bin);
	if(!IsValidBin(Bin))
		return eHT160BinAreaNotUse;
	return BinToArea[Bin];
}
//---------------------------------------------------------------------------
int THT160BinAreaMap::GetBinByArea(int Area)
{
	if(!IsValidArea(Area))
		return 0;
	return AreaToBin[Area];
}
//---------------------------------------------------------------------------
int THT160BinAreaMap::GetTotalMappedCount()
{
	int Area;
	int Count=0;

	for(Area=eHT160BinAreaAuto1; Area<eHT160BinAreaTotal; Area++)
	{
		if(IsAreaEnabled(Area) && GetBinByArea(Area)>0)
			Count++;
	}
	return Count;
}
//---------------------------------------------------------------------------
bool THT160BinAreaMap::IsAreaEnabled(int Area)
{
	if(Area>=eHT160BinAreaAuto1 && Area<=eHT160BinAreaAuto6)
		return true;
	if(Area==eHT160BinAreaColor)
		return CosFunction.bColorBinAreaInstalled;
	return false;
}
//---------------------------------------------------------------------------
bool THT160BinAreaMap::IsErrorBin(int Bin)
{
	return (GetErrorBinIndex(Bin)>=0);
}
//---------------------------------------------------------------------------
AnsiString THT160BinAreaMap::GetErrorBinName(int Bin)
{
	if(Bin==HT160_BIN_ERROR_2D_SCAN_FAIL)
		return "2DScanFail";
	if(Bin==HT160_BIN_ERROR_NO_BIN_SETTING)
		return "NoBinSetting";
	return "";
}
//---------------------------------------------------------------------------
int THT160BinAreaMap::GetErrorBinByIndex(int Index)
{
	if(Index==0)
		return HT160_BIN_ERROR_2D_SCAN_FAIL;
	if(Index==1)
		return HT160_BIN_ERROR_NO_BIN_SETTING;
	return 0;
}
//---------------------------------------------------------------------------
bool THT160BinAreaMap::SetErrorBinAreaByBin(int Bin, int Area)
{
	int Index=GetErrorBinIndex(Bin);

	if(Index<0)
		return false;
	if(Area==0)
	{
		ErrorBinToArea[Index]=0;
		return true;
	}
	if(!IsAreaEnabled(Area))
		return false;
	ErrorBinToArea[Index]=Area;
	return true;
}
//---------------------------------------------------------------------------
void THT160BinAreaMap::UseDefaultErrorBinArea(int Bin)
{
	int Index=GetErrorBinIndex(Bin);

	if(Index>=0)
		ErrorBinToArea[Index]=0;
}
//---------------------------------------------------------------------------
int THT160BinAreaMap::GetAreaByErrorBin(int Bin)
{
	int Index=GetErrorBinIndex(Bin);
	int Area;

	if(Index<0)
		return eHT160BinAreaNotUse;
	Area=ErrorBinToArea[Index];
	if(IsAreaEnabled(Area))
		return Area;
	return GetErrorBinArea();
}
//---------------------------------------------------------------------------
bool THT160BinAreaMap::SetErrorBinArea(int Area)
{
	if(!IsAreaEnabled(Area))
		Area=HT160_DEFAULT_ERROR_BIN_AREA;
	if(!IsAreaEnabled(Area))
		return false;
	ErrorBinArea=Area;
	return true;
}
//---------------------------------------------------------------------------
int THT160BinAreaMap::GetErrorBinArea()
{
	if(!IsAreaEnabled(ErrorBinArea))
		ErrorBinArea=HT160_DEFAULT_ERROR_BIN_AREA;
	return ErrorBinArea;
}
//---------------------------------------------------------------------------
AnsiString THT160BinAreaMap::GetAreaName(int Area)
{
	switch(Area)
	{
		case eHT160BinAreaNotUse: return "NotUse";
		case eHT160BinAreaEmpty:  return "Empty";
		case eHT160BinAreaLoader: return "Loader";
		case eHT160BinAreaAuto1:  return "Auto1";
		case eHT160BinAreaAuto2:  return "Auto2";
		case eHT160BinAreaAuto3:  return "Auto3";
		case eHT160BinAreaAuto4:  return "Auto4";
		case eHT160BinAreaAuto5:  return "Auto5";
		case eHT160BinAreaAuto6:  return "Auto6";
		case eHT160BinAreaColor:  return "Color";
	}
	return "";
}
//---------------------------------------------------------------------------
int THT160BinAreaMap::GetAreaByName(AnsiString AreaName)
{
	AnsiString Name=AreaName.Trim().UpperCase();
	int Area;

	for(Area=eHT160BinAreaNotUse; Area<eHT160BinAreaTotal; Area++)
	{
		if(GetAreaName(Area).UpperCase()==Name)
			return Area;
	}
	return eHT160BinAreaNotUse;
}
//---------------------------------------------------------------------------
AnsiString THT160BinAreaMap::GetDefaultIniFileName()
{
	return RecipeManager.GetRecipeFileName("BinAreaMap.ini");
}
//---------------------------------------------------------------------------
void THT160BinAreaMap::LoadFromIni(AnsiString FileName)
{
	TIniFile *Ini;
	int Area;
	int Bin;
	int ErrorArea;
	AnsiString ErrorText;

	Clear();
	if(FileName==AnsiString(""))
		FileName=GetDefaultIniFileName();
	if(!FileExists(FileName))
		return;

	Ini=new TIniFile(FileName);
	for(Area=eHT160BinAreaAuto1; Area<eHT160BinAreaTotal; Area++)
	{
		if(!IsAreaEnabled(Area))
			continue;
		Bin=Ini->ReadInteger("BinAreaMap", GetAreaName(Area), 0);
		if(Bin>0)
			AddBinArea(Bin, Area);
	}
	ErrorText=Ini->ReadString("BinAreaMap", "ErrorBinArea", GetAreaName(HT160_DEFAULT_ERROR_BIN_AREA));
	ErrorArea=GetAreaByName(ErrorText);
	if(ErrorArea==eHT160BinAreaNotUse && ErrorText.Trim()!=AnsiString(""))
	{
		ErrorArea=atoi(ErrorText.c_str());
		if(ErrorArea>=1 && ErrorArea<=6)
			ErrorArea=eHT160BinAreaAuto1+ErrorArea-1;
		else if(ErrorArea==7 && CosFunction.bColorBinAreaInstalled)
			ErrorArea=eHT160BinAreaColor;
		else
			ErrorArea=HT160_DEFAULT_ERROR_BIN_AREA;
	}
	SetErrorBinArea(ErrorArea);
	for(Bin=HT160_BIN_ERROR_2D_SCAN_FAIL; Bin<=HT160_BIN_ERROR_NO_BIN_SETTING; Bin++)
	{
		ErrorText=Ini->ReadString("ErrorBinAreaMap", GetErrorBinName(Bin), "Default");
		if(ErrorText.UpperCase()==AnsiString("DEFAULT") || ErrorText.Trim()==AnsiString(""))
		{
			UseDefaultErrorBinArea(Bin);
			continue;
		}
		ErrorArea=GetAreaByName(ErrorText);
		if(IsAreaEnabled(ErrorArea))
			SetErrorBinAreaByBin(Bin, ErrorArea);
		else
			UseDefaultErrorBinArea(Bin);
	}
	delete Ini;
}
//---------------------------------------------------------------------------
void THT160BinAreaMap::SaveToIni(AnsiString FileName)
{
	TIniFile *Ini;
	int Area;
	int Bin;
	int Index;
	int ErrorArea;

	if(FileName==AnsiString(""))
		FileName=GetDefaultIniFileName();
	ForceDirectories(ExtractFilePath(FileName));

	Ini=new TIniFile(FileName);
	Ini->EraseSection("BinAreaMap");
	for(Area=eHT160BinAreaAuto1; Area<eHT160BinAreaTotal; Area++)
	{
		if(IsAreaEnabled(Area))
			Ini->WriteInteger("BinAreaMap", GetAreaName(Area), GetBinByArea(Area));
	}
	Ini->WriteString("BinAreaMap", "ErrorBinArea", GetAreaName(GetErrorBinArea()));
	Ini->EraseSection("ErrorBinAreaMap");
	for(Index=0; Index<HT160_BIN_ERROR_REASON_COUNT; Index++)
	{
		Bin=GetErrorBinByIndex(Index);
		ErrorArea=ErrorBinToArea[Index];
		if(IsAreaEnabled(ErrorArea))
			Ini->WriteString("ErrorBinAreaMap", GetErrorBinName(Bin), GetAreaName(ErrorArea));
		else
			Ini->WriteString("ErrorBinAreaMap", GetErrorBinName(Bin), "Default");
		Ini->WriteInteger("ErrorBinAreaMap", GetErrorBinName(Bin)+AnsiString("Code"), Bin);
	}
	delete Ini;
}
//---------------------------------------------------------------------------
void THT160BinAreaMap::LoadDefault()
{
	AnsiString FileName=GetDefaultIniFileName();
	AnsiString RootPath=HSys.CurrentDir;
	AnsiString LegacyFileName;

	LoadFromIni(FileName);
	if(FileExists(FileName))
		return;

	if(RootPath==AnsiString(""))
		RootPath="..";
	LegacyFileName=RootPath+AnsiString("\\data\\BinAreaMap.ini");
	if(FileExists(LegacyFileName))
	{
		LoadFromIni(LegacyFileName);
		SaveToIni(FileName);
	}
}
//---------------------------------------------------------------------------
void THT160BinAreaMap::SaveDefault()
{
	SaveToIni(GetDefaultIniFileName());
}
//---------------------------------------------------------------------------
void LoadCosFunctionMachineOption()
{
	AnsiString FileName=GetCosFunctionMachineOptionFileName();
	TIniFile *Ini;

	CosFunction.bColorBinAreaInstalled=false;
	if(!FileExists(FileName))
		return;

	Ini=new TIniFile(FileName);
	CosFunction.bColorBinAreaInstalled=Ini->ReadBool("HardwareInstall", "ColorBinAreaInstalled", false);
	delete Ini;
}
//---------------------------------------------------------------------------
void SaveCosFunctionMachineOption()
{
	AnsiString FileName=GetCosFunctionMachineOptionFileName();
	TIniFile *Ini;

	ForceDirectories(ExtractFilePath(FileName));
	Ini=new TIniFile(FileName);
	Ini->WriteBool("HardwareInstall", "ColorBinAreaInstalled", CosFunction.bColorBinAreaInstalled);
	delete Ini;
}
//---------------------------------------------------------------------------
void InitialCosFunction()
{
	CosFunction.bUseBinAreaMap=true;
	LoadCosFunctionMachineOption();
	RecipeManager.LoadLastRecipeName();
	RecipeManager.EnsureCurrentRecipeDir();
	BinAreaMap.LoadDefault();
}
//---------------------------------------------------------------------------