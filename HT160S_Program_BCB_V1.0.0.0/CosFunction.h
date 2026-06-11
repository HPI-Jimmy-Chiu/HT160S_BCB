//---------------------------------------------------------------------------
#ifndef CosFunctionH
#define CosFunctionH
//---------------------------------------------------------------------------
#include <System.hpp>
#include <Classes.hpp>
#include "cmydef.h"
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
// AI(HT160S-Maintainer) 20260608 : Recipe Tray-Form data structure.
// In-memory single source of truth for the Setup "Tray Form" page values
// (tray geometry per recipe). Machine modules (Loader / SortArm / Monitor)
// MUST read these fields instead of reaching into the Setup form's UI Edit
// controls (fSetup->edXDivision ...). The UI is only a view and holds valid
// data only while the form has been loaded for the current recipe; reading it
// cross-module produced stale/default values when the form was not (re)loaded.
// Loaded from data\<recipe>\setup.ini [TrayForm] at boot (InitialCosFunction),
// on recipe change, and refreshed right after the operator saves the page.
class THT160TrayForm
{
public:
	double XStart;
	double XPitch;
	double YStart;
	double YPitch;
	int    XDivision;
	int    YDivision;

	__fastcall THT160TrayForm();
	void SetDefault();
	void Load(AnsiString RecipeName);   // read data\<recipe>\setup.ini [TrayForm]
	void Load();                        // read the current recipe
	void Save(AnsiString RecipeName);   // write data\<recipe>\setup.ini [TrayForm]
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
	// Paid customer-bound feature : use Bin->Area routing table.
	// Set at compile time by DoCustomerFunction() per CUSTOMER_CODE; no ini.
	bool bUseBinAreaMap;
	// Use the 2D-code -> Bin lookup map (THT160Bin2DMap) at Top CCD stage.
	bool bUse2DBinMap;
	// Paid customer-bound feature : SECS/GEM factory communication. Gates both
	// the engine boot (GemInitial) and the main-form SECS status badge/log view.
	bool bUseSecsGem;
} HT160S_CUSTOMER_FUNCTION;
//---------------------------------------------------------------------------
// 2D-code -> Bin lookup ("dui zhang ben").
// Customer supplies a JSON file per delivery (filename includes HHmmss) under
//   <CurrentDir>\HT160S_LotInfo\yyyymm\dd\<name>_HHmmss.json
// Each file may contain multiple Lots. Format:
//   { "Maps":[ { "LotNumber":"LOT1", "Items":[ {"Code2D":"ABC","Bin":1}, ... ] }, ... ] }
// Lookup key is (LotNumber, Code2D) -> Bin. Future source : SECS / customer cloud.
class THT160Bin2DMap
{
private:
	TStringList *m_List;          // name = LOT \x01 CODE2D ; Objects[i] = (TObject*)bin
	AnsiString m_LoadedFile;      // full path of the file last loaded (empty if none)

	AnsiString MakeKey(AnsiString LotNumber, AnsiString Code2D);

public:
	__fastcall THT160Bin2DMap();
	__fastcall ~THT160Bin2DMap();

	void Clear();
	bool AddEntry(AnsiString LotNumber, AnsiString Code2D, int Bin);
	bool Lookup(AnsiString LotNumber, AnsiString Code2D, int &Bin);
	int GetEntryCount();
	AnsiString GetLoadedFile();

	AnsiString GetMapFolder(TDateTime When);
	AnsiString FindLatestMapFile(TDateTime When);
	bool LoadFromFile(AnsiString FileName);
	bool LoadLatest();            // scan today's folder, load newest file
};
//---------------------------------------------------------------------------
// Lot run-info data layer ("RunInfo").  Source HT172 RUN_INFO concept, but HT160
// is a SORTING center : many upstream-machine Lots coexist on one machine at the
// same time, with no TrayID.  Each IC carries only its unique 2D code (its ID).
// The machine reads the 2D code and REVERSE-looks-up across ALL loaded Lots to
// find both (which Lot, which Bin).  2D code is globally unique (no duplicates);
// a duplicate found while loading the lot table is an operator error -> alarm.
//---------------------------------------------------------------------------
#define HT160_MAX_LOT 64                 // max Lots coexisting on the machine
#define HT160_LOT_SOURCE_OFFLINE 0       // operator built the lot data by hand
#define HT160_LOT_SOURCE_SECS    1       // remote host (SECS) pushed the lot data
//---------------------------------------------------------------------------
// Layer 1 : one production Lot instance currently held on the machine.
typedef struct
{
	AnsiString sLotID;             // Lot identity (key returned by reverse lookup)
	AnsiString sSourceMachine;     // which upstream machine sent this Lot
	AnsiString sDeviceName;        // product / device (optional)
	int        iSource;            // HT160_LOT_SOURCE_OFFLINE / _SECS
	int        iPlanQty;           // expected IC count (lot-table item count)
	int        iSortedQty;         // IC already sorted for this Lot
	int        iBinCount[TEST_MAX_BIN]; // per-Bin sorted count for this Lot
	TDateTime  dtFirstSeen;        // first IC of this Lot scanned
	TDateTime  dtLastSeen;         // last IC of this Lot scanned
	bool       bActive;            // still being sorted on the machine

	void Clear();
} TLotRunInfo;
//---------------------------------------------------------------------------
// Layer 2 : the Lot registry + the global 2D-code reverse index.
// Reverse index key = Code2D alone (unique); value packs (LotIndex, Bin).
class THT160LotRegistry
{
private:
	TLotRunInfo  m_Lots[HT160_MAX_LOT];
	int          m_LotCount;
	TStringList *m_Code2DIndex;    // Sorted; name=Code2D; Objects=(LotIndex*1000000+Bin)
	AnsiString   m_LastDupCode;    // last duplicate 2D code seen on load

	int  PackRef(int LotIndex, int Bin);
	void UnpackRef(int Packed, int &LotIndex, int &Bin);

public:
	__fastcall THT160LotRegistry();
	__fastcall ~THT160LotRegistry();

	void Clear();
	int  GetLotCount();
	int  GetItemCount();
	AnsiString GetLastDuplicateCode();

	// Simulation helper : fetch the Index-th registered 2D code (sorted order).
	// Returns "" if Index is out of range. Used to cycle virtual ICs when no
	// real Top CCD hardware is present (tSimuData.bRunSimulation).
	AnsiString GetCode2DByIndex(int Index);

	int  FindLotIndex(AnsiString LotID);   // -1 if not present
	TLotRunInfo* GetLot(int Index);        // NULL if out of range

	// Lot list management (offline manual UI path + remote path share these).
	int  AddLot(AnsiString LotID, int Source, AnsiString SourceMachine, AnsiString DeviceName);
	bool RemoveLot(AnsiString LotID);
	bool RenameLot(AnsiString OldLotID, AnsiString NewLotID);

	// Add one 2D->Bin item.  Returns false if Code2D already exists globally
	// (duplicate); DupExistingLot is set to the Lot that already owns the code.
	bool AddItem(AnsiString LotID, AnsiString Code2D, int Bin, AnsiString &DupExistingLot);

	// The heart of HT160 sorting : reverse lookup by the IC's unique 2D code.
	bool FindByCode2D(AnsiString Code2D, AnsiString &LotID, int &Bin, int &LotIndex);

	// Production counting after a successful sort.
	void OnSorted(int LotIndex, int Bin);

	// Bulk load (remote/file).  bHasDuplicate / FirstDupCode report 2D collisions.
	bool LoadFromJsonFile(AnsiString FileName, bool &bHasDuplicate, AnsiString &FirstDupCode);
	bool LoadLatest(bool &bHasDuplicate, AnsiString &FirstDupCode);
};
//---------------------------------------------------------------------------
extern HT160S_CUSTOMER_FUNCTION CosFunction;
extern THT160RecipeManager RecipeManager;
extern THT160BinAreaMap BinAreaMap;
extern THT160Bin2DMap Bin2DMap;
extern THT160LotRegistry LotRegistry;
extern THT160TrayForm TrayForm;
extern void InitialCosFunction();
//---------------------------------------------------------------------------
#endif