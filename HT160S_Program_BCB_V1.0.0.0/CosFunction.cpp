//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "CosFunction.h"
#include "database.h"
#include "GeneralSetting.h"
#include "Config.h"
#include "cmydef.h"
#include "MachineType.h"
#include "cJSON.h"
#include <IniFiles.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
HT160S_CUSTOMER_FUNCTION CosFunction;
THT160RecipeManager RecipeManager;
THT160BinAreaMap BinAreaMap;
THT160Bin2DMap Bin2DMap;
THT160LotRegistry LotRegistry;
THT160TrayForm TrayForm;
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
		return GeneralSetting.bColorBinAreaInstalled;
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
		else if(ErrorArea==7 && GeneralSetting.bColorBinAreaInstalled)
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
static void FUNC_CC_HONPREC_QC()
{
	// HonPrec QC : Bin->Area routing table enabled.
	CosFunction.bUseBinAreaMap=true;
	// HonPrec QC : SECS/GEM factory communication enabled (paid feature).
	CosFunction.bUseSecsGem=true;
}
//---------------------------------------------------------------------------
static void DoCustomerFunction()
{
	// Compile-time paid-feature gate per CUSTOMER_CODE (aligns with HT172).
	switch(CUSTOMER_CODE)
	{
		case CC_HONPREC_QC:
			FUNC_CC_HONPREC_QC();
			break;
		default:
			break;
	}
}
//---------------------------------------------------------------------------
// THT160Bin2DMap : 2D-code -> Bin lookup table ("dui zhang ben").
//---------------------------------------------------------------------------
__fastcall THT160Bin2DMap::THT160Bin2DMap()
{
	m_List=new TStringList;
	m_List->Sorted=true;
	m_List->Duplicates=dupIgnore;
	m_List->CaseSensitive=true;
	m_LoadedFile="";
}
//---------------------------------------------------------------------------
__fastcall THT160Bin2DMap::~THT160Bin2DMap()
{
	delete m_List;
}
//---------------------------------------------------------------------------
AnsiString THT160Bin2DMap::MakeKey(AnsiString LotNumber, AnsiString Code2D)
{
	// Combine (Lot, Code) into a single sortable key with a control separator.
	return LotNumber.Trim()+AnsiString((char)1)+Code2D.Trim();
}
//---------------------------------------------------------------------------
void THT160Bin2DMap::Clear()
{
	m_List->Clear();
	m_LoadedFile="";
}
//---------------------------------------------------------------------------
bool THT160Bin2DMap::AddEntry(AnsiString LotNumber, AnsiString Code2D, int Bin)
{
	AnsiString Key=MakeKey(LotNumber, Code2D);
	if(Key.Trim()==AnsiString(""))
		return false;
	int Index=m_List->IndexOf(Key);
	if(Index>=0)
		m_List->Objects[Index]=(TObject*)Bin;   // update existing
	else
		m_List->AddObject(Key, (TObject*)Bin);
	return true;
}
//---------------------------------------------------------------------------
bool THT160Bin2DMap::Lookup(AnsiString LotNumber, AnsiString Code2D, int &Bin)
{
	int Index=m_List->IndexOf(MakeKey(LotNumber, Code2D));
	if(Index<0)
		return false;
	Bin=(int)m_List->Objects[Index];
	return true;
}
//---------------------------------------------------------------------------
int THT160Bin2DMap::GetEntryCount()
{
	return m_List->Count;
}
//---------------------------------------------------------------------------
AnsiString THT160Bin2DMap::GetLoadedFile()
{
	return m_LoadedFile;
}
//---------------------------------------------------------------------------
AnsiString THT160Bin2DMap::GetMapFolder(TDateTime When)
{
	AnsiString RootPath=HSys.CurrentDir;
	if(RootPath==AnsiString(""))
		RootPath="..";
	return RootPath+AnsiString("\\HT160S_LotInfo\\")
		+FormatDateTime("yyyymm", When)+AnsiString("\\")
		+FormatDateTime("dd", When);
}
//---------------------------------------------------------------------------
AnsiString THT160Bin2DMap::FindLatestMapFile(TDateTime When)
{
	AnsiString Folder=GetMapFolder(When);
	AnsiString Mask=Folder+AnsiString("\\*.json");
	TSearchRec Sr;
	AnsiString LatestName="";
	int LatestTime=-1;
	if(FindFirst(Mask, faAnyFile, Sr)==0)
	{
		do
		{
			if((Sr.Attr & faDirectory)!=0)
				continue;
			if(Sr.Time>LatestTime)
			{
				LatestTime=Sr.Time;
				LatestName=Sr.Name;
			}
		}
		while(FindNext(Sr)==0);
		FindClose(Sr);
	}
	if(LatestName==AnsiString(""))
		return AnsiString("");
	return Folder+AnsiString("\\")+LatestName;
}
//---------------------------------------------------------------------------
bool THT160Bin2DMap::LoadFromFile(AnsiString FileName)
{
	if(!FileExists(FileName))
		return false;

	// Read whole file as text.
	AnsiString Text;
	TStringList *Raw=new TStringList;
	try
	{
		Raw->LoadFromFile(FileName);
		Text=Raw->Text;
	}
	catch(...)
	{
		delete Raw;
		return false;
	}
	delete Raw;

	cJSON *Root=cJSON_Parse(Text.c_str());
	if(Root==NULL)
		return false;

	Clear();

	cJSON *Maps=cJSON_GetObjectItem(Root, "Maps");
	if(Maps!=NULL && cJSON_IsArray(Maps))
	{
		cJSON *MapNode=Maps->child;
		while(MapNode!=NULL)
		{
			cJSON *LotNode=cJSON_GetObjectItem(MapNode, "LotNumber");
			AnsiString LotNumber="";
			if(LotNode!=NULL && cJSON_IsString(LotNode) && LotNode->valuestring!=NULL)
				LotNumber=AnsiString(LotNode->valuestring);

			cJSON *Items=cJSON_GetObjectItem(MapNode, "Items");
			if(Items!=NULL && cJSON_IsArray(Items))
			{
				cJSON *ItemNode=Items->child;
				while(ItemNode!=NULL)
				{
					cJSON *CodeNode=cJSON_GetObjectItem(ItemNode, "Code2D");
					cJSON *BinNode=cJSON_GetObjectItem(ItemNode, "Bin");
					if(CodeNode!=NULL && cJSON_IsString(CodeNode) && CodeNode->valuestring!=NULL
						&& BinNode!=NULL && cJSON_IsNumber(BinNode))
					{
						AddEntry(LotNumber, AnsiString(CodeNode->valuestring), (int)BinNode->valuedouble);
					}
					ItemNode=ItemNode->next;
				}
			}
			MapNode=MapNode->next;
		}
	}

	cJSON_Delete(Root);
	m_LoadedFile=FileName;
	return true;
}
//---------------------------------------------------------------------------
bool THT160Bin2DMap::LoadLatest()
{
	AnsiString FileName=FindLatestMapFile(Now());
	if(FileName==AnsiString(""))
		return false;
	return LoadFromFile(FileName);
}
//---------------------------------------------------------------------------
// TLotRunInfo / THT160LotRegistry : multi-Lot run-info + 2D reverse index.
//---------------------------------------------------------------------------
void TLotRunInfo::Clear()
{
	sLotID="";
	sSourceMachine="";
	sDeviceName="";
	iSource=HT160_LOT_SOURCE_OFFLINE;
	iPlanQty=0;
	iSortedQty=0;
	for(int i=0;i<TEST_MAX_BIN;i++)
		iBinCount[i]=0;
	dtFirstSeen=0;
	dtLastSeen=0;
	bActive=false;
}
//---------------------------------------------------------------------------
__fastcall THT160LotRegistry::THT160LotRegistry()
{
	m_Code2DIndex=new TStringList;
	m_Code2DIndex->Sorted=true;
	m_Code2DIndex->Duplicates=dupIgnore;
	m_Code2DIndex->CaseSensitive=true;
	m_LotCount=0;
	m_LastDupCode="";
	for(int i=0;i<HT160_MAX_LOT;i++)
		m_Lots[i].Clear();
}
//---------------------------------------------------------------------------
__fastcall THT160LotRegistry::~THT160LotRegistry()
{
	delete m_Code2DIndex;
}
//---------------------------------------------------------------------------
int THT160LotRegistry::PackRef(int LotIndex, int Bin)
{
	return LotIndex*1000000+Bin;
}
//---------------------------------------------------------------------------
void THT160LotRegistry::UnpackRef(int Packed, int &LotIndex, int &Bin)
{
	LotIndex=Packed/1000000;
	Bin=Packed%1000000;
}
//---------------------------------------------------------------------------
void THT160LotRegistry::Clear()
{
	m_Code2DIndex->Clear();
	m_LotCount=0;
	m_LastDupCode="";
	for(int i=0;i<HT160_MAX_LOT;i++)
		m_Lots[i].Clear();
}
//---------------------------------------------------------------------------
int THT160LotRegistry::GetLotCount()
{
	int Count=0;
	for(int i=0;i<m_LotCount;i++)
		if(m_Lots[i].sLotID.Trim()!=AnsiString(""))
			Count++;
	return Count;
}
//---------------------------------------------------------------------------
int THT160LotRegistry::GetItemCount()
{
	return m_Code2DIndex->Count;
}
//---------------------------------------------------------------------------
AnsiString THT160LotRegistry::GetCode2DByIndex(int Index)
{
	if(Index<0 || Index>=m_Code2DIndex->Count)
		return AnsiString("");
	return m_Code2DIndex->Strings[Index];
}
//---------------------------------------------------------------------------
AnsiString THT160LotRegistry::GetLastDuplicateCode()
{
	return m_LastDupCode;
}
//---------------------------------------------------------------------------
int THT160LotRegistry::FindLotIndex(AnsiString LotID)
{
	AnsiString Key=LotID.Trim();
	if(Key==AnsiString(""))
		return -1;
	for(int i=0;i<m_LotCount;i++)
		if(m_Lots[i].sLotID.Trim()==Key)
			return i;
	return -1;
}
//---------------------------------------------------------------------------
TLotRunInfo* THT160LotRegistry::GetLot(int Index)
{
	if(Index<0 || Index>=m_LotCount)
		return NULL;
	return &m_Lots[Index];
}
//---------------------------------------------------------------------------
int THT160LotRegistry::AddLot(AnsiString LotID, int Source, AnsiString SourceMachine, AnsiString DeviceName)
{
	AnsiString Key=LotID.Trim();
	if(Key==AnsiString(""))
		return -1;

	// Already present : update meta, keep stable index.
	int Index=FindLotIndex(Key);
	if(Index>=0)
	{
		m_Lots[Index].iSource=Source;
		if(SourceMachine.Trim()!=AnsiString(""))
			m_Lots[Index].sSourceMachine=SourceMachine;
		if(DeviceName.Trim()!=AnsiString(""))
			m_Lots[Index].sDeviceName=DeviceName;
		m_Lots[Index].bActive=true;
		return Index;
	}

	// Reuse a freed slot (sLotID empty) to keep packed indices stable.
	for(int i=0;i<m_LotCount;i++)
	{
		if(m_Lots[i].sLotID.Trim()==AnsiString(""))
		{
			m_Lots[i].Clear();
			m_Lots[i].sLotID=Key;
			m_Lots[i].iSource=Source;
			m_Lots[i].sSourceMachine=SourceMachine;
			m_Lots[i].sDeviceName=DeviceName;
			m_Lots[i].bActive=true;
			return i;
		}
	}

	// Append a new slot.
	if(m_LotCount>=HT160_MAX_LOT)
		return -1;
	Index=m_LotCount;
	m_Lots[Index].Clear();
	m_Lots[Index].sLotID=Key;
	m_Lots[Index].iSource=Source;
	m_Lots[Index].sSourceMachine=SourceMachine;
	m_Lots[Index].sDeviceName=DeviceName;
	m_Lots[Index].bActive=true;
	m_LotCount++;
	return Index;
}
//---------------------------------------------------------------------------
bool THT160LotRegistry::RemoveLot(AnsiString LotID)
{
	int Index=FindLotIndex(LotID);
	if(Index<0)
		return false;

	// Drop every reverse-index entry that points at this Lot.
	for(int i=m_Code2DIndex->Count-1;i>=0;i--)
	{
		int LotIndex, Bin;
		UnpackRef((int)m_Code2DIndex->Objects[i], LotIndex, Bin);
		if(LotIndex==Index)
			m_Code2DIndex->Delete(i);
	}

	// Free the slot (kept in place so other packed indices stay valid).
	m_Lots[Index].Clear();
	return true;
}
//---------------------------------------------------------------------------
bool THT160LotRegistry::RenameLot(AnsiString OldLotID, AnsiString NewLotID)
{
	AnsiString NewKey=NewLotID.Trim();
	if(NewKey==AnsiString(""))
		return false;
	if(FindLotIndex(NewKey)>=0)
		return false;                 // target name already exists
	int Index=FindLotIndex(OldLotID);
	if(Index<0)
		return false;
	m_Lots[Index].sLotID=NewKey;     // reverse index keyed by Code2D, no reindex
	return true;
}
//---------------------------------------------------------------------------
bool THT160LotRegistry::AddItem(AnsiString LotID, AnsiString Code2D, int Bin, AnsiString &DupExistingLot)
{
	DupExistingLot="";
	AnsiString Code=Code2D.Trim();
	if(Code==AnsiString(""))
		return true;                  // nothing to add, not an error

	int LotIndex=AddLot(LotID, HT160_LOT_SOURCE_OFFLINE, "", "");
	if(LotIndex<0)
		return false;                 // registry full

	// 2D code is globally unique : reject a code already owned by any Lot.
	int Exist=m_Code2DIndex->IndexOf(Code);
	if(Exist>=0)
	{
		int OldLotIndex, OldBin;
		UnpackRef((int)m_Code2DIndex->Objects[Exist], OldLotIndex, OldBin);
		if(OldLotIndex>=0 && OldLotIndex<m_LotCount)
			DupExistingLot=m_Lots[OldLotIndex].sLotID;
		m_LastDupCode=Code;
		return false;
	}

	m_Code2DIndex->AddObject(Code, (TObject*)PackRef(LotIndex, Bin));
	m_Lots[LotIndex].iPlanQty++;
	return true;
}
//---------------------------------------------------------------------------
bool THT160LotRegistry::FindByCode2D(AnsiString Code2D, AnsiString &LotID, int &Bin, int &LotIndex)
{
	LotID="";
	Bin=0;
	LotIndex=-1;
	AnsiString Code=Code2D.Trim();
	if(Code==AnsiString(""))
		return false;
	int Index=m_Code2DIndex->IndexOf(Code);
	if(Index<0)
		return false;
	int Idx, B;
	UnpackRef((int)m_Code2DIndex->Objects[Index], Idx, B);
	if(Idx<0 || Idx>=m_LotCount)
		return false;
	if(m_Lots[Idx].sLotID.Trim()==AnsiString(""))
		return false;                 // lot was removed
	LotIndex=Idx;
	Bin=B;
	LotID=m_Lots[Idx].sLotID;
	return true;
}
//---------------------------------------------------------------------------
void THT160LotRegistry::OnSorted(int LotIndex, int Bin)
{
	if(LotIndex<0 || LotIndex>=m_LotCount)
		return;
	TLotRunInfo *Lot=&m_Lots[LotIndex];
	if(Lot->iSortedQty<=0)
		Lot->dtFirstSeen=Now();
	Lot->iSortedQty++;
	Lot->dtLastSeen=Now();
	if(Bin>=0 && Bin<TEST_MAX_BIN)
		Lot->iBinCount[Bin]++;
}
//---------------------------------------------------------------------------
bool THT160LotRegistry::LoadFromJsonFile(AnsiString FileName, bool &bHasDuplicate, AnsiString &FirstDupCode)
{
	// Appends to the registry (multi-Lot : several files may coexist).
	// Caller decides when to Clear().  Same JSON shape as THT160Bin2DMap.
	bHasDuplicate=false;
	FirstDupCode="";
	if(!FileExists(FileName))
		return false;

	AnsiString Text;
	TStringList *Raw=new TStringList;
	try
	{
		Raw->LoadFromFile(FileName);
		Text=Raw->Text;
	}
	catch(...)
	{
		delete Raw;
		return false;
	}
	delete Raw;

	cJSON *Root=cJSON_Parse(Text.c_str());
	if(Root==NULL)
		return false;

	cJSON *Maps=cJSON_GetObjectItem(Root, "Maps");
	if(Maps!=NULL && cJSON_IsArray(Maps))
	{
		cJSON *MapNode=Maps->child;
		while(MapNode!=NULL)
		{
			cJSON *LotNode=cJSON_GetObjectItem(MapNode, "LotNumber");
			AnsiString LotNumber="";
			if(LotNode!=NULL && cJSON_IsString(LotNode) && LotNode->valuestring!=NULL)
				LotNumber=AnsiString(LotNode->valuestring);

			cJSON *MachNode=cJSON_GetObjectItem(MapNode, "SourceMachine");
			AnsiString SourceMachine="";
			if(MachNode!=NULL && cJSON_IsString(MachNode) && MachNode->valuestring!=NULL)
				SourceMachine=AnsiString(MachNode->valuestring);

			int LotIndex=AddLot(LotNumber, HT160_LOT_SOURCE_OFFLINE, SourceMachine, "");

			cJSON *Items=cJSON_GetObjectItem(MapNode, "Items");
			if(LotIndex>=0 && Items!=NULL && cJSON_IsArray(Items))
			{
				cJSON *ItemNode=Items->child;
				while(ItemNode!=NULL)
				{
					cJSON *CodeNode=cJSON_GetObjectItem(ItemNode, "Code2D");
					cJSON *BinNode=cJSON_GetObjectItem(ItemNode, "Bin");
					if(CodeNode!=NULL && cJSON_IsString(CodeNode) && CodeNode->valuestring!=NULL
						&& BinNode!=NULL && cJSON_IsNumber(BinNode))
					{
						AnsiString DupLot;
						if(!AddItem(LotNumber, AnsiString(CodeNode->valuestring),
							(int)BinNode->valuedouble, DupLot))
						{
							if(!bHasDuplicate)
								FirstDupCode=AnsiString(CodeNode->valuestring);
							bHasDuplicate=true;
						}
					}
					ItemNode=ItemNode->next;
				}
			}
			MapNode=MapNode->next;
		}
	}

	cJSON_Delete(Root);
	return true;
}
//---------------------------------------------------------------------------
bool THT160LotRegistry::LoadLatest(bool &bHasDuplicate, AnsiString &FirstDupCode)
{
	// Reuse Bin2DMap folder layout to find today's newest lot-table file.
	AnsiString FileName=Bin2DMap.FindLatestMapFile(Now());
	bHasDuplicate=false;
	FirstDupCode="";
	if(FileName==AnsiString(""))
		return false;
	Clear();
	return LoadFromJsonFile(FileName, bHasDuplicate, FirstDupCode);
}
//---------------------------------------------------------------------------
// AI(HT160S-Maintainer) 20260608 : THT160TrayForm - recipe tray-geometry
// structure. Single source of truth shared by Loader / SortArm / Monitor so
// they never read the Setup form UI controls cross-module.
//---------------------------------------------------------------------------
__fastcall THT160TrayForm::THT160TrayForm()
{
	SetDefault();
}
//---------------------------------------------------------------------------
void THT160TrayForm::SetDefault()
{
	XStart=0.0;
	XPitch=1.0;
	YStart=0.0;
	YPitch=1.0;
	XDivision=1;
	YDivision=1;
}
//---------------------------------------------------------------------------
void THT160TrayForm::Load(AnsiString RecipeName)
{
	TIniFile *Ini;
	AnsiString FileName;

	SetDefault();
	FileName=RecipeManager.GetRecipeFileName(
		RecipeManager.NormalizeRecipeName(RecipeName), "setup.ini");
	if(!FileExists(FileName))
		return;
	Ini=new TIniFile(FileName);
	try
	{
		XStart   =Ini->ReadFloat("TrayForm", "XStart", XStart);
		XPitch   =Ini->ReadFloat("TrayForm", "XPitch", XPitch);
		YStart   =Ini->ReadFloat("TrayForm", "YStart", YStart);
		YPitch   =Ini->ReadFloat("TrayForm", "YPitch", YPitch);
		XDivision=Ini->ReadInteger("TrayForm", "XDivision", XDivision);
		YDivision=Ini->ReadInteger("TrayForm", "YDivision", YDivision);
	}
	__finally
	{
		delete Ini;
	}
}
//---------------------------------------------------------------------------
void THT160TrayForm::Load()
{
	Load(RecipeManager.GetCurrentRecipeName());
}
//---------------------------------------------------------------------------
void THT160TrayForm::Save(AnsiString RecipeName)
{
	TIniFile *Ini;
	AnsiString FileName;

	FileName=RecipeManager.GetRecipeFileName(
		RecipeManager.NormalizeRecipeName(RecipeName), "setup.ini");
	ForceDirectories(ExtractFilePath(FileName));
	Ini=new TIniFile(FileName);
	try
	{
		Ini->WriteFloat("TrayForm", "XStart", XStart);
		Ini->WriteFloat("TrayForm", "XPitch", XPitch);
		Ini->WriteFloat("TrayForm", "YStart", YStart);
		Ini->WriteFloat("TrayForm", "YPitch", YPitch);
		Ini->WriteInteger("TrayForm", "XDivision", XDivision);
		Ini->WriteInteger("TrayForm", "YDivision", YDivision);
	}
	__finally
	{
		delete Ini;
	}
}
//---------------------------------------------------------------------------
void InitialCosFunction()
{
	// CosFunction tier (paid features) : defaults first, then customer switch.
	CosFunction.bUseBinAreaMap=true;
	CosFunction.bUse2DBinMap=true;
	CosFunction.bUseSecsGem=false;   // paid SECS/GEM off unless a CUSTOMER_CODE enables it
	DoCustomerFunction();
	// General tier (ship + hardware install) : system\General.ini.
	GeneralSetting.Load();
	// Config tier (customer self-toggle) : config\config.ini.
	Config.Load();
	// Recipe tier : data\<recipe>\.
	RecipeManager.LoadLastRecipeName();
	RecipeManager.EnsureCurrentRecipeDir();
	BinAreaMap.LoadDefault();
	// AI(HT160S-Maintainer) 20260608 : load the recipe Tray-Form geometry into
	// the in-memory structure so machine modules never need the Setup form UI.
	TrayForm.Load();
}
//---------------------------------------------------------------------------