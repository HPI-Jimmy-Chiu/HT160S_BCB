//---------------------------------------------------------------------------
#ifndef GeneralSettingH
#define GeneralSettingH
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
// General tier : ship parameters + hardware-install state.
// Storage : system\General.ini (per-machine, NOT copied between machines).
// Edited only through the Maintenance (commissioning) form.
// See skill ht160s-config-tiers.
//---------------------------------------------------------------------------
class THT160GeneralSetting
{
private:
	AnsiString GetGeneralIniFileName();

public:
	__fastcall THT160GeneralSetting();

	// Hardware install : does this machine physically have the Color bin-area
	// hardware? This is a commissioning fact, not a paid feature.
	bool bColorBinAreaInstalled;

	// Hardware install : is this machine equipped with an AMR (autonomous
	// mobile robot) docking interface? Commissioning fact, not a paid feature.
	bool bUseAMR;

	// Diagnostic : when true, SortArm pops a modal message before each Auto Z-down
	// comparing actual vs expected motor positions. Per-machine engineering check,
	// default OFF. Toggle via [Diagnostic] ShowSortArmPlaceCheck in General.ini.
	bool bShowSortArmPlaceCheck;

	// Safety : minimum encoder gap (motor counts) the two Loader-Y cars must keep
	// from each other before either car is allowed to move, used only when the
	// opposite car is clamping a tray. Larger = more conservative. Per-machine
	// commissioning value. Set via [Safety] LoaderYSafeDistance in General.ini.
	int iLoaderYSafeDistance;

	void SetDefault();
	void Load();
	void Save();
};
//---------------------------------------------------------------------------
extern THT160GeneralSetting GeneralSetting;
//---------------------------------------------------------------------------
#endif
