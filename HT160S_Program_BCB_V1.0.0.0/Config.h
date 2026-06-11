//---------------------------------------------------------------------------
#ifndef ConfigH
#define ConfigH
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
// Config tier : features already shipped that the CUSTOMER turns on/off.
// Storage : config\config.ini ONLY (never recipe folder / system / machine_option).
// Backing field set is the legacy TFunction tFunction (cprod.h), which Config
// loads at startup and saves when the customer changes a UI checkbox.
// See skill ht160s-config-tiers.
//---------------------------------------------------------------------------
class THT160Config
{
private:
	AnsiString GetConfigIniFileName();

public:
	__fastcall THT160Config();

	void SetDefault();
	void Load();
	void Save();
};
//---------------------------------------------------------------------------
extern THT160Config Config;
//---------------------------------------------------------------------------
#endif
