//---------------------------------------------------------------------------
#ifndef CosFunctionH
#define CosFunctionH
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
#define HT160_BIN_AREA_NORMAL_MAX_BIN 1000
#define HT160_BIN_ERROR_2D_SCAN_FAIL 1000
#define HT160_BIN_ERROR_NO_BIN_SETTING 1001
#define HT160_BIN_ERROR_REASON_COUNT 2
#define HT160_BIN_AREA_MAX_BIN 1102
#define HT160_BIN_AREA_MAX_AREA 16
#define HT160_DEFAULT_ERROR_BIN_AREA eHT160BinAreaAuto6
//---------------------------------------------------------------------------
class THT160RecipeManager
{
private:
	AnsiString CurrentRecipeName;
	bool CopyDirectory(AnsiString SourceDir, AnsiString DestDir);
	bool DeleteDirectory(AnsiString DirName);

public:
	__fastcall THT160RecipeManager();

	AnsiString NormalizeRecipeName(AnsiString RecipeName);
	void SetCurrentRecipeName(AnsiString RecipeName);
	AnsiString GetCurrentRecipeName();
	AnsiString GetDataRootPath();
	AnsiString GetRecipeDirName();
	AnsiString GetRecipeDirName(AnsiString RecipeName);
	AnsiString GetRecipeFileName(AnsiString FileName);
	AnsiString GetRecipeFileName(AnsiString RecipeName, AnsiString FileName);
	AnsiString GetLastSetFileName();
	void EnsureCurrentRecipeDir();
	void LoadLastRecipeName();
	void SaveLastRecipeName();
	bool RecipeExists(AnsiString RecipeName);
	bool CreateRecipe(AnsiString RecipeName);
	bool CopyRecipe(AnsiString SourceRecipeName, AnsiString DestRecipeName);
	bool DeleteRecipe(AnsiString RecipeName);
};
//---------------------------------------------------------------------------
enum EHT160BinArea
{
	eHT160BinAreaNotUse=0,
	eHT160BinAreaEmpty=1,
	eHT160BinAreaLoader=2,
	eHT160BinAreaAuto1=3,
	eHT160BinAreaAuto2=4,
	eHT160BinAreaAuto3=5,
	eHT160BinAreaAuto4=6,
	eHT160BinAreaAuto5=7,
	eHT160BinAreaAuto6=8,
	eHT160BinAreaColor=9,
	eHT160BinAreaTotal=10
};
//---------------------------------------------------------------------------
class THT160BinAreaMap
{
private:
	int BinToArea[HT160_BIN_AREA_MAX_BIN];
	int AreaToBin[HT160_BIN_AREA_MAX_AREA];
	int ErrorBinToArea[HT160_BIN_ERROR_REASON_COUNT];
	int ErrorBinArea;

	bool IsValidBin(int Bin);
	bool IsValidArea(int Area);
	int GetErrorBinIndex(int Bin);

public:
	__fastcall THT160BinAreaMap();

	void Clear();
	bool AddBinArea(int Bin, int Area);
	bool SetBinByArea(int Bin, int Area);
	void RemoveBin(int Bin);
	void RemoveArea(int Area);
	int GetAreaByBin(int Bin);
	int GetBinByArea(int Area);
	int GetTotalMappedCount();
	bool IsAreaEnabled(int Area);
	bool IsErrorBin(int Bin);
	AnsiString GetErrorBinName(int Bin);
	int GetErrorBinByIndex(int Index);
	bool SetErrorBinAreaByBin(int Bin, int Area);
	void UseDefaultErrorBinArea(int Bin);
	int GetAreaByErrorBin(int Bin);
	bool SetErrorBinArea(int Area);
	int GetErrorBinArea();
	AnsiString GetAreaName(int Area);
	int GetAreaByName(AnsiString AreaName);
	AnsiString GetDefaultIniFileName();
	void LoadFromIni(AnsiString FileName);
	void SaveToIni(AnsiString FileName);
	void LoadDefault();
	void SaveDefault();
};
//---------------------------------------------------------------------------
typedef struct
{
	bool bUseBinAreaMap;
	bool bColorBinAreaInstalled;
} HT160S_CUSTOMER_FUNCTION;
//---------------------------------------------------------------------------
extern HT160S_CUSTOMER_FUNCTION CosFunction;
extern THT160RecipeManager RecipeManager;
extern THT160BinAreaMap BinAreaMap;
extern void LoadCosFunctionMachineOption();
extern void SaveCosFunctionMachineOption();
extern void InitialCosFunction();
//---------------------------------------------------------------------------
#endif