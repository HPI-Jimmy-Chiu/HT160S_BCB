//---------------------------------------------------------------------------
#ifndef languageH
#define languageH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
// HT160S bilingual UI engine (file-based, no BDE).
//
// Behavior is ported from the HT172 language.cpp component-tree walk, but the
// legacy Paradox/BDE TTable dictionary is replaced by a plain text file:
//     system\language.txt   (Big5/CP950 encoded)
//     one record per line, TAB delimited:  formName \t component \t EN \t ZH
//     lines starting with ';' or '#', and blank lines, are ignored.
//
// The dictionary doubles as the white-list: only components listed in it are
// touched, so runtime-driven captions (counts, state strings, value displays)
// are left alone automatically.
//
// Language selector reuses HSys.LastSet.iLanguageCountry : 0 = English (the
// design-time default), 1 = Traditional Chinese.
//---------------------------------------------------------------------------
class TfLan : public TForm
{
__published:
private:
    void __fastcall ApplyTreeCaptions(TWinControl *PCtrl, const AnsiString FormName);
    bool __fastcall LookupCaption(const AnsiString FormName, const AnsiString Component,
                                  AnsiString &EnText, AnsiString &ZhText);
public:
    __fastcall TfLan(TComponent* Owner);
    void __fastcall LoadDictionary();
    bool __fastcall IsDictionaryLoaded();
    void __fastcall ChangeLanguage(TForm *P);
};
//---------------------------------------------------------------------------
extern PACKAGE TfLan *fLan;
//---------------------------------------------------------------------------
// Runtime phrase translator for captions/text that code assigns at run time
// (counts excluded - they are language-neutral). Returns the English input
// unchanged when iLanguageCountry==0, or its Traditional-Chinese rendering
// (from system\language_phrases.txt, Big5, "EN<TAB>ZH") when ==1. Unknown
// phrases fall back to the English input, so wrapping a site is always safe.
//     ctrl->Caption = LangT("Real");        // literal
//     ctrl->Caption = LangT(Estr[i]);       // variable / table value
AnsiString LangT(const AnsiString &en);
void ReloadLanguagePhrases();
//---------------------------------------------------------------------------
#endif
