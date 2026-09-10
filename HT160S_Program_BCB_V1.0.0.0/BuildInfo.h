//---------------------------------------------------------------------------
// BuildInfo.h
// AI(ht160s-buildstamp) 20260910 : the build identity, so an on-site exe can be tied to a
// known code state. Both KYEC machines reported "HT160S 1.0.0.0" on 2026-09-09 while machine
// 2 demonstrably ran an older exe (it still carried the TrayPos sensors removed by 16984c8),
// which made every log conclusion "the repo does X", never "the machine does X".
//
// GENERATED: scripts/ops/build-ht160s.ps1 overwrites this file with the git short SHA plus a
// build timestamp for the duration of a build, then restores this placeholder in its finally
// block, so the working tree never carries a machine-specific stamp. A build started from the
// BCB6 IDE (which does not run the script) simply compiles the placeholder below - that is why
// the placeholder is COMMITTED and why cmydef.h also keeps an #ifndef fallback.
//
// Do NOT edit by hand and do NOT put anything else in here: cmydef.h includes it, so this file
// reaches every translation unit and any change forces a full rebuild.
//---------------------------------------------------------------------------
#ifndef BuildInfoH
#define BuildInfoH
#define HT160S_BUILD_ID "dev"
#endif
//---------------------------------------------------------------------------
