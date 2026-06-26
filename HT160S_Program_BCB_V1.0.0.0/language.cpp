//---------------------------------------------------------------------------
#include <vcl.h>
#include <StdCtrls.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#pragma hdrstop

#include "language.h"
#include "database.h"   // SYSTEM_MODULAR HSys : CurrentDir, LastSet.iLanguageCountry
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfLan *fLan;

// One dictionary row : design-time English caption + Traditional Chinese.
struct TLangEntry
{
    AnsiString En;
    AnsiString Zh;
};

// key = formName + "|" + component ; Sorted so IndexOf is a binary search.
// Objects[] holds the matching TLangEntry*.
static TStringList *gDictKey = NULL;

// Runtime phrase map for code-assigned captions : Sorted English key ;
// Objects[] holds a heap AnsiString* with the Traditional-Chinese rendering.
static TStringList *gPhraseKey = NULL;

//---------------------------------------------------------------------------
__fastcall TfLan::TfLan(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
static void FreeDict()
{
    if(gDictKey != NULL)
    {
        for(int i=0;i<gDictKey->Count;i++)
        {
            TLangEntry *e = (TLangEntry *)gDictKey->Objects[i];
            if(e != NULL) delete e;
        }
        delete gDictKey;
        gDictKey = NULL;
    }
}
//---------------------------------------------------------------------------
static AnsiString DictFileName()
{
    return HSys.CurrentDir + AnsiString("\\system\\language.txt");
}
//---------------------------------------------------------------------------
// Assign a caption to whichever caption-bearing VCL class P actually is.
static void SetCtrlCaption(TControl *P, const AnsiString Cap)
{
    { TLabel       *q = dynamic_cast<TLabel *>(P);       if(q){ q->Caption=Cap; return; } }
    { TSpeedButton *q = dynamic_cast<TSpeedButton *>(P); if(q){ q->Caption=Cap; return; } }
    { TBitBtn      *q = dynamic_cast<TBitBtn *>(P);      if(q){ q->Caption=Cap; return; } }
    { TButton      *q = dynamic_cast<TButton *>(P);      if(q){ q->Caption=Cap; return; } }
    { TCheckBox    *q = dynamic_cast<TCheckBox *>(P);    if(q){ q->Caption=Cap; return; } }
    { TRadioButton *q = dynamic_cast<TRadioButton *>(P); if(q){ q->Caption=Cap; return; } }
    { TRadioGroup  *q = dynamic_cast<TRadioGroup *>(P);  if(q){ q->Caption=Cap; return; } }
    { TGroupBox    *q = dynamic_cast<TGroupBox *>(P);    if(q){ q->Caption=Cap; return; } }
    { TTabSheet    *q = dynamic_cast<TTabSheet *>(P);    if(q){ q->Caption=Cap; return; } }
    { TStaticText  *q = dynamic_cast<TStaticText *>(P);  if(q){ q->Caption=Cap; return; } }
    { TPanel       *q = dynamic_cast<TPanel *>(P);       if(q){ q->Caption=Cap; return; } }
}
//---------------------------------------------------------------------------
bool __fastcall TfLan::IsDictionaryLoaded()
{
    return (gDictKey != NULL && gDictKey->Count > 0);
}
//---------------------------------------------------------------------------
void __fastcall TfLan::LoadDictionary()
{
    FreeDict();
    gDictKey = new TStringList;
    gDictKey->Sorted = false;          // build first, sort afterwards

    AnsiString fn = DictFileName();
    if(!FileExists(fn))
    {
        gDictKey->Sorted = true;       // graceful no-op when dictionary is absent
        return;
    }

    TStringList *Lines = new TStringList;
    try
    {
        Lines->LoadFromFile(fn);
        for(int i=0;i<Lines->Count;i++)
        {
            AnsiString L = Lines->Strings[i];
            if(L.Length()==0)               continue;
            if(L[1]==';' || L[1]=='#')      continue;

            // split into 4 TAB-delimited fields : form, component, EN, ZH
            int p1 = L.Pos("\t");
            if(p1<=0) continue;
            AnsiString form = L.SubString(1, p1-1);
            AnsiString rest = L.SubString(p1+1, L.Length()-p1);

            int p2 = rest.Pos("\t");
            if(p2<=0) continue;
            AnsiString comp  = rest.SubString(1, p2-1);
            AnsiString rest2 = rest.SubString(p2+1, rest.Length()-p2);

            int p3 = rest2.Pos("\t");
            AnsiString en, zh;
            if(p3<=0) { en = rest2; zh = ""; }
            else      { en = rest2.SubString(1, p3-1);
                        zh = rest2.SubString(p3+1, rest2.Length()-p3); }

            if(form.Length()==0 || comp.Length()==0) continue;

            TLangEntry *e = new TLangEntry;
            e->En = en;
            e->Zh = zh;
            gDictKey->AddObject(form + AnsiString("|") + comp, (TObject *)e);
        }
        gDictKey->Sorted = true;       // enable IndexOf binary search
    }
    __finally
    {
        delete Lines;
    }
}
//---------------------------------------------------------------------------
bool __fastcall TfLan::LookupCaption(const AnsiString FormName, const AnsiString Component,
                                     AnsiString &EnText, AnsiString &ZhText)
{
    if(gDictKey == NULL)      return false;
    if(Component.Length()==0) return false;
    int idx = gDictKey->IndexOf(FormName + AnsiString("|") + Component);
    if(idx < 0) return false;
    TLangEntry *e = (TLangEntry *)gDictKey->Objects[idx];
    if(e == NULL) return false;
    EnText = e->En;
    ZhText = e->Zh;
    return true;
}
//---------------------------------------------------------------------------
void __fastcall TfLan::ApplyTreeCaptions(TWinControl *PCtrl, const AnsiString FormName)
{
    if(PCtrl == NULL) return;
    int lang = HSys.LastSet.iLanguageCountry;   // 0 = English, 1 = Chinese
    AnsiString en, zh;

    for(int iP=0; iP<PCtrl->ControlCount; iP++)
    {
        TControl *P = PCtrl->Controls[iP];

        if(LookupCaption(FormName, P->Name, en, zh))
        {
            AnsiString cap = (lang==1) ? zh : en;
            if(cap.Length()>0) SetCtrlCaption(P, cap);
        }

        // recurse into containers (Panel / GroupBox / PageControl / TabSheet ...)
        TWinControl *W = dynamic_cast<TWinControl *>(P);
        if(W != NULL)
            ApplyTreeCaptions(W, FormName);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfLan::ChangeLanguage(TForm *P)
{
    if(P == NULL) return;
    if(gDictKey == NULL) LoadDictionary();
    if(gDictKey == NULL || gDictKey->Count==0) return;

    AnsiString FormName = P->Name;
    int lang = HSys.LastSet.iLanguageCountry;
    AnsiString en, zh;

    // the form's own caption is keyed under (formName, formName)
    if(LookupCaption(FormName, FormName, en, zh))
    {
        AnsiString cap = (lang==1) ? zh : en;
        if(cap.Length()>0) P->Caption = cap;
    }

    ApplyTreeCaptions(P, FormName);
}
//---------------------------------------------------------------------------
static void FreePhrases()
{
    if(gPhraseKey != NULL)
    {
        for(int i=0;i<gPhraseKey->Count;i++)
        {
            AnsiString *z = (AnsiString *)gPhraseKey->Objects[i];
            if(z != NULL) delete z;
        }
        delete gPhraseKey;
        gPhraseKey = NULL;
    }
}
//---------------------------------------------------------------------------
static AnsiString PhraseFileName()
{
    return HSys.CurrentDir + AnsiString("\\system\\language_phrases.txt");
}
//---------------------------------------------------------------------------
// Decode \n \t \r \\ escapes so multi-line dialog messages can live on one
// line in the phrase file. Big5-aware : a lead byte (>=0x81) plus its trail
// byte are copied as a unit, so a Big5 trail byte of 0x5C ('\') is never
// mistaken for an escape introducer.
static AnsiString DecodeEscapes(const AnsiString &s)
{
    AnsiString out = "";
    int i = 1;
    int n = s.Length();
    while(i <= n)
    {
        unsigned char c = (unsigned char)s[i];
        if(c >= 0x81 && i < n)            // Big5 double-byte char : copy whole
        {
            out += s[i];
            out += s[i+1];
            i += 2;
            continue;
        }
        if(c == '\\' && i < n)
        {
            char d = s[i+1];
            if(d == 'n') { out += AnsiString("\n"); i += 2; continue; }
            if(d == 't') { out += AnsiString("\t"); i += 2; continue; }
            if(d == 'r') { out += AnsiString("\r"); i += 2; continue; }
            if(d == '\\'){ out += AnsiString("\\"); i += 2; continue; }
        }
        out += s[i];
        i++;
    }
    return out;
}
//---------------------------------------------------------------------------
static void LoadPhrases()
{
    FreePhrases();
    gPhraseKey = new TStringList;
    gPhraseKey->Sorted = false;

    AnsiString fn = PhraseFileName();
    if(!FileExists(fn))
    {
        gPhraseKey->Sorted = true;     // graceful no-op when phrase file is absent
        return;
    }

    TStringList *Lines = new TStringList;
    try
    {
        Lines->LoadFromFile(fn);
        for(int i=0;i<Lines->Count;i++)
        {
            AnsiString L = Lines->Strings[i];
            if(L.Length()==0)          continue;
            if(L[1]==';' || L[1]=='#') continue;

            int p1 = L.Pos("\t");      // EN <TAB> ZH
            if(p1<=0) continue;
            AnsiString en = DecodeEscapes(L.SubString(1, p1-1));
            AnsiString zh = DecodeEscapes(L.SubString(p1+1, L.Length()-p1));
            if(en.Length()==0) continue;

            int idx = gPhraseKey->IndexOf(en);   // not yet sorted : linear, but de-dup
            if(idx>=0) continue;
            gPhraseKey->AddObject(en, (TObject *) new AnsiString(zh));
        }
        gPhraseKey->Sorted = true;
    }
    __finally
    {
        delete Lines;
    }
}
//---------------------------------------------------------------------------
void ReloadLanguagePhrases()
{
    LoadPhrases();
}
//---------------------------------------------------------------------------
AnsiString LangT(const AnsiString &en)
{
    if(HSys.LastSet.iLanguageCountry == 0)   // English : pass through
        return en;
    if(gPhraseKey == NULL)
        LoadPhrases();
    if(gPhraseKey == NULL)
        return en;
    int idx = gPhraseKey->IndexOf(en);
    if(idx < 0)
        return en;                            // unknown phrase : safe fallback
    AnsiString *z = (AnsiString *)gPhraseKey->Objects[idx];
    if(z == NULL || z->Length()==0)
        return en;
    return *z;
}
//---------------------------------------------------------------------------
