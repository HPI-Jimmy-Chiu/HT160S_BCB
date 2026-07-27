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
	int PassBin;   //AI(ht160s-bin-passfail) 20260708 : bin# that counts as PASS (0 = feature off)

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
	int GetPassBin();
	void SetPassBin(int Bin);
	// AI(ht160s-lotpassfail) 20260709 : single classifier shared by the CCD-scan freeze
	// (aLoader), the place-time PASS/FAIL log and the By Lot+PassFail routing read, so all
	// three agree. Returns 0 = no class (error bin, or PassBin feature off) -> Error Auto /
	// blank log ; 1 = PASS (Bin==PassBin) ; 2 = FAIL (any other real bin).
	int GetPassFailClass(int Bin);
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
	// Color-station 2D reader enable (mirrors bUse2DBinMap for the Top CCD).
	// Loaded from system\General.ini [ColorCCD] Enable; gates the Color CCD
	// connect/shot trigger in aColor and is toggled by the maintenance page.
	bool bUseColorCcd;
	// Top CCD (Loader IC 2D) reader enable; mirrors bUseColorCcd. [TopCCD] Enable
	// in General.ini; OFF -> simulated 2D (REALLY only; HAS_TRAY/DUMMY always sim).
	bool bUseTopCcd;
	//AI(ht160s-kyec) 20260722 : routing bin source used to be selectable (SBin/HBin);
	//it is now permanently HBin (see LoadFromJsonString). The knob was removed so it can
	//never be mis-set. HBin/SBin are both still recorded per-IC for the Soter report.
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
	AnsiString sKyecLotID;         // KYEC batch id (Soter col7 / file token); from SET_LOT_INFO, "" until supplied
	AnsiString sSourceMachine;     // which upstream machine sent this Lot
	AnsiString sDeviceName;        // product / device (optional)
	int        iSource;            // HT160_LOT_SOURCE_OFFLINE / _SECS
	int        iPlanQty;           // expected IC count (lot-table item count)
	int        iSortedQty;         // IC already sorted for this Lot
	int        iBinCount[TEST_MAX_BIN]; // per-Bin sorted count for this Lot
	TDateTime  dtFirstSeen;        // first IC of this Lot scanned
	TDateTime  dtLastSeen;         // last IC of this Lot scanned
	bool       bActive;            // still being sorted on the machine

	// Backup meta from the customer 2DIDHistory format (not used for routing,
	// kept for traceability / UI / SECS report). Cleared by Clear().
	AnsiString sSubstage;          // 2DIDHistory[].Substage
	AnsiString sProductCode;       // 2DIDHistory[].ProductCode

	void Clear();
} TLotRunInfo;
//---------------------------------------------------------------------------
// Per-IC backup record (customer 2DIDHistory ICIInfo[] item). The hot sorting
// path only needs (LotIndex, Bin) packed in the reverse index; this struct
// stores ALL extra customer fields for traceability / UI / SECS, keyed by the
// IC's unique 2D code. Allocated on the heap and freed wholesale on Clear().
typedef struct
{
	AnsiString sCode2D;       // QRCodeID (unique key)
	AnsiString sLotID;        // owning Lot = KYEC lot (machine identity)
	int        iBin;          // routing bin actually used (always HBin)
	int        iHBin;         // customer HBin (backup)
	int        iSBin;         // customer SBin (backup)
	AnsiString sRetestCode;   // customer RetestCode (backup)
	AnsiString sDiePass;      // customer DiePass (backup)
	//AI(ht160s-kyec) 20260722 : dual lot identity. These three are PER-IC because one
	//KYEC lot can map to several customer lots (1:N). The Soter report reads them per die
	//(col6 Cust lot / col4 ProductCode / col5 Substage); col7 = owning KYEC lot (sLotID).
	AnsiString sCustLotID;    // customer lot (WebAPI QRCodeIDHis[].LOTID) -> Soter col6
	AnsiString sProductCode;  // per-IC ProductCode -> Soter col4
	AnsiString sSubstage;     // per-IC Substage -> Soter col5
} TLotIcInfo;
//---------------------------------------------------------------------------
// Layer 2 : the Lot registry + the global 2D-code reverse index.
// Reverse index key = Code2D alone (unique); value packs (LotIndex, Bin).
class THT160LotRegistry
{
private:
	TLotRunInfo  m_Lots[HT160_MAX_LOT];
	int          m_LotCount;
	TStringList *m_Code2DIndex;    // Sorted; name=Code2D; Objects=(LotIndex*1000000+Bin)
	TStringList *m_Code2DInfo;     // Sorted; name=Code2D; Objects=TLotIcInfo* (backup)
	AnsiString   m_LastDupCode;    // last duplicate 2D code seen on load
	//AI(ht160s-kyec) 20260722 : # of already-present 2D codes UPDATED in place by the
	//last LoadFromJsonString upsert pass (WebAPI re-pull latest-wins). Reset each parse.
	int          m_RefreshCount;

	int  PackRef(int LotIndex, int Bin);
	void UnpackRef(int Packed, int &LotIndex, int &Bin);
	void FreeAllIcInfo();          // delete every heap TLotIcInfo + clear list

public:
	__fastcall THT160LotRegistry();
	__fastcall ~THT160LotRegistry();

	void Clear();
	int  GetLotCount();
	int  GetLotSlotCount();                 // raw slot span (incl. freed gaps)
	int  GetItemCount();
	AnsiString GetLastDuplicateCode();
	//AI(ht160s-kyec) 20260722 : # of codes refreshed (upserted to latest) by the last parse.
	int  GetRefreshCount();

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

	// Extended add : same as AddItem but also stores the customer backup fields
	// (HBin/SBin/RetestCode/DiePass) plus the per-IC customer identity (CustLotID /
	// ProductCode / Substage). Bin is the routing bin actually used.
	//AI(ht160s-kyec) 20260722 : bUpsert=true (WebAPI re-pull) -> an already-present 2D code
	//is UPDATED in place to the latest data (owning lot kept, iPlanQty not bumped,
	//m_RefreshCount++) instead of being rejected. bUpsert=false keeps the legacy
	//reject-on-duplicate (manual editor / offline import).
	bool AddItemEx(AnsiString LotID, AnsiString Code2D, int Bin,
		int HBin, int SBin, AnsiString RetestCode, AnsiString DiePass,
		AnsiString CustLotID, AnsiString ProductCode, AnsiString Substage,
		AnsiString &DupExistingLot, bool bUpsert=false);

	// Remove one 2D->Bin item by its unique Code2D. Returns false if the code is
	// not present. Decrements the owning lot's iPlanQty. Used by the manual 2D/Bin
	// editor (delete a manually-entered IC).
	bool RemoveItem(AnsiString Code2D);

	// Serialize ALL non-blank lots to a JSON file in the 2DIDHistory schema that
	// LoadFromJsonString parses (round-trips). Returns false on I/O exception.
	bool SaveToJsonFile(AnsiString FileName);

	// Backup lookup : fetch the full per-IC record by 2D code. Returns false if
	// the code is unknown. Not on the hot sorting path.
	bool FindIcInfo(AnsiString Code2D, TLotIcInfo &Info);

	//AI(ht160s-lot-webapi) 20260612 : UI / traceability : enumerate every 2D IC
	// record that belongs to one Lot. Out is filled with one tab-separated line
	// per IC : Code2D \t Bin \t HBin \t SBin \t RetestCode \t DiePass.
	//AI(ht160s-kyec) 20260722 : appended \t CustLotID \t ProductCode \t Substage (per-IC).
	// Trailing fields are backward-compatible : all front-indexed readers ignore them.
	// Returns the record count (0 = this Lot has no 2D data loaded yet).
	int  GetLotIcList(AnsiString LotID, TStrings *Out);

	// The heart of HT160 sorting : reverse lookup by the IC's unique 2D code.
	bool FindByCode2D(AnsiString Code2D, AnsiString &LotID, int &Bin, int &LotIndex);

	// Production counting after a successful sort.
	void OnSorted(int LotIndex, int Bin);

	// Bulk load (remote/file).  bHasDuplicate / FirstDupCode report 2D collisions.
	bool LoadFromJsonFile(AnsiString FileName, bool &bHasDuplicate, AnsiString &FirstDupCode);
	//AI(ht160s-lot-webapi) 20260612 : Stage 4 : load directly from a JSON string
	// (e.g. a WebAPI HTTP response body), not a file.  LoadFromJsonFile delegates here.
	//AI(ht160s-kyec) 20260722 : StampKyecLotId != "" (WebAPI pull) registers EVERY IC in
	//the response under that KYEC lot (owning lot), keeping each response group's LOTID as
	//the per-IC customer lot, and enables upsert (latest-wins). Default "" (boot restore /
	//local import) keeps the legacy per-LOTID keying with no upsert.
	bool LoadFromJsonString(AnsiString Json, bool &bHasDuplicate, AnsiString &FirstDupCode,
		AnsiString StampKyecLotId="");
	bool LoadLatest(bool &bHasDuplicate, AnsiString &FirstDupCode);
	//AI(ht160s-whitelist) 20260727 : pre-flight for the customer WhiteList.json contract only
	//(KYEC-WhiteList-Interface-Spec 3.2). Loads nothing; returns false + an operator-facing
	//Reason when the file is not a usable whitelist. See the definition for why the shared
	//LoadFromJsonString cannot carry these rules.
	bool ValidateWhiteListJson(AnsiString Json, AnsiString &Reason);
};
//---------------------------------------------------------------------------
// Dynamic (Lot,Bin) -> Auto binding table for the "By Lot+Bin" sort mode
// (customer special request). In this mode the Auto<->Bin mapping is NOT a
// static recipe table (THT160BinAreaMap); instead each distinct (LotID,Bin)
// pair is bound to an Auto on a first-come-first-served basis as ICs are
// scanned at the Top CCD. The first scanned (Lot,Bin) takes the first free
// non-Error Auto, the next distinct pair takes the next, and so on. Once every
// non-Error Auto is bound, any further distinct pair routes to the Error Auto.
//
// Persistence is keyed by LotID (stable across restart), not LotIndex, so a
// mid-lot software restart can restore the table and keep each Auto tied to the
// same (Lot,Bin) -> no cross-lot mixing in one Auto. Stored in
//   <CurrentDir>\system\LotBinBinding.ini .
// The Error Auto comes from BinAreaMap.GetErrorBinArea() (set on the Bin page,
// only editable at Lot End), so the Error Auto is fixed during a lot.
class THT160LotBinBinding
{
private:
	TStringList *m_List;        // name = LotID \x01 Bin ; Objects[i] = (TObject*)(AutoIndex+1)

	AnsiString MakeKey(AnsiString LotID, int Bin);
	AnsiString GetIniFileName();
	int  GetAutoStationCount();     // number of Auto stations (Auto1..Auto6)
	int  GetErrorAutoIndex();       // 0-based Auto index of the Error Auto

public:
	__fastcall THT160LotBinBinding();
	__fastcall ~THT160LotBinBinding();

	void Clear();
	// Read-only lookup of an already-bound pair. Returns 0-based Auto index, or
	// -1 if (LotID,Bin) is not bound yet.
	int  FindAutoByLotID(AnsiString LotID, int Bin);
	int  FindAuto(int LotIndex, int Bin);   // resolve LotID via LotRegistry first
	// Is this Auto already taken by some non-Error (Lot,Bin) binding?
	bool IsAutoBound(int AutoIndex);
	// First-come-first-served allocation. Binds (LotID,Bin) to the first free
	// non-Error Auto (skips the Error Auto), persists, and returns its index.
	// When every non-Error Auto is bound, binds to the Error Auto. Idempotent :
	// an already-bound pair returns its existing Auto. LotIndex<0 (no lot) is
	// not bound here; the caller routes such ICs straight to the Error Auto.
	int  ResolveAuto(int LotIndex, int Bin);

	int  GetBindingCount();
	bool GetBindingByIndex(int Index, AnsiString &LotID, int &Bin, int &AutoIndex);

	void SaveToIni();
	void LoadFromIni();
};
//---------------------------------------------------------------------------
extern HT160S_CUSTOMER_FUNCTION CosFunction;
extern THT160RecipeManager RecipeManager;
extern THT160BinAreaMap BinAreaMap;
extern THT160Bin2DMap Bin2DMap;
extern THT160LotRegistry LotRegistry;
extern THT160LotBinBinding LotBinBinding;
extern THT160TrayForm TrayForm;
extern void InitialCosFunction();
//---------------------------------------------------------------------------
#endif