// Deterministic replica of HT160S Loader AMR tray-count logic.
// Copies verbatim (modulo AnsiString->std::string) the three functions that
// decide the Motion-View "work盤" number, for OLD (pre-111b976) and NEW (post-fix)
// code, and runs the exact State Record 2026-07-13 15:58 scenario:
//   host SECS LoaderTrayCount = 8 (work-only per spec), cover=1, identity=1, feed 2.
// Expected: OLD shows work=4 (the bug), NEW shows work=6 (correct).
#include <cstdio>
#include <string>
#include <sstream>
using namespace std;

static string I(int v){ ostringstream o; o<<v; return o.str(); }

// ---- FmtCarKinds : identical in old and new (main.cpp:789) ----
static string FmtCarKinds(int t,int id0,int cv0){
    int id,cv,nm;
    if(t<0) t=0;
    if(id0<0) return I(t)+"/0/0";
    id=(t<id0)?t:id0;
    cv=t-id;
    if(cv>cv0) cv=cv0;
    if(cv<0) cv=0;
    nm=t-id-cv;
    return I(id)+"/"+I(cv)+"/"+I(nm);   // identity/cover/work
}

enum Kind{ NORMAL, IDENTITY, COVER };
static const char* KN(int k){ return k==IDENTITY?"IDENTITY":(k==COVER?"COVER":"work"); }

// ================= OLD (pre-111b976) =================
static int OLD_iCarTrayTotal(bool useAMR,int secs,int simMax){
    return (useAMR && secs>0) ? secs : simMax;      // BUG: SECS work-count treated as full total
}
static int OLD_GetFedTrayKind(int feedSerial,int total,bool useAMR){
    if(!useAMR) return NORMAL;
    if(feedSerial>=total)   return IDENTITY;
    if(feedSerial==total-1) return COVER;
    return NORMAL;
}
// display always called FmtCarKinds(count,1,1) for Loader

// ================= NEW (post-111b976) =================
static int NEW_iCarTrayTotal(bool useAMR,int secs,int simMax,int cover,int identity){
    if(useAMR && secs>0){
        int header = cover + ((identity>0)?identity:0);
        return secs + header;
    }
    return simMax;
}
static int NEW_GetFedTrayKind(int feedSerial,int total,bool useAMR,int cover,int identity){
    if(!useAMR) return NORMAL;
    int idCount=(identity>0)?identity:0;
    int cvCount=(cover>0)?cover:0;
    if(feedSerial > total-idCount)         return IDENTITY;
    if(feedSerial > total-idCount-cvCount) return COVER;
    return NORMAL;
}

int main(){
    // ---- exact snapshot inputs ----
    bool useAMR=true;
    int  secs=8;        // iSecsCarTrayCount (host START_AGV LoaderTrayCount)
    int  simMax=10;     // GeneralSetting.iSimAmrMaxTray[0]
    int  cover=1;       // iAmrCoverTray[0]     (General.ini [AMR] CoverTray0 / default)
    int  identity=1;    // iAmrIdentityTray[0]  (General.ini [AMR] IdentityTray0 / default)
    int  feeds=2;       // iFeedSerial at snapshot

    printf("=== Scenario: SECS=%d work-only, cover=%d, identity=%d, fed=%d ===\n\n",
           secs,cover,identity,feeds);

    // ---------- OLD ----------
    {
        int total=OLD_iCarTrayTotal(useAMR,secs,simMax);
        int infeed=total;
        printf("[OLD pre-111b976]  iCarTrayTotal=%d\n",total);
        printf("  feed sequence kinds: ");
        // replicate: iFeedSerial++ then GetFedTrayKind(iFeedSerial,total); drain iSimInfeedCount--
        int serial=0;
        for(int f=0; f<feeds; f++){
            serial++;
            int k=OLD_GetFedTrayKind(serial,total,useAMR);
            if(infeed>0) infeed--;
            printf("s%d=%s ",serial,KN(k));
        }
        string disp=FmtCarKinds(infeed,1,1);   // Loader hardcoded (1,1)
        printf("\n  after %d feeds: iSimInfeedCount=%d  Motion-View label=\"%s\"  => work=%s\n",
               feeds,infeed,disp.c_str(),disp.substr(disp.rfind('/')+1).c_str());
        // full drain to show where cover/identity land
        printf("  full car kind map (serial->kind): ");
        for(int s=1;s<=total;s++) printf("%d:%s ",s,KN(OLD_GetFedTrayKind(s,total,useAMR)));
        int work=0,cv=0,id=0;
        for(int s=1;s<=total;s++){int k=OLD_GetFedTrayKind(s,total,useAMR); if(k==NORMAL)work++; else if(k==COVER)cv++; else id++;}
        printf("\n  => car of %d = %d work + %d cover + %d identity\n\n",total,work,cv,id);
    }

    // ---------- NEW ----------
    {
        int total=NEW_iCarTrayTotal(useAMR,secs,simMax,cover,identity);
        int infeed=total;
        printf("[NEW post-111b976] iCarTrayTotal=%d  (= SECS %d + header %d)\n",
               total,secs,cover+((identity>0)?identity:0));
        printf("  feed sequence kinds: ");
        int serial=0;
        for(int f=0; f<feeds; f++){
            serial++;
            int k=NEW_GetFedTrayKind(serial,total,useAMR,cover,identity);
            if(infeed>0) infeed--;
            printf("s%d=%s ",serial,KN(k));
        }
        string disp=FmtCarKinds(infeed,identity,cover);   // display reads same config
        printf("\n  after %d feeds: iSimInfeedCount=%d  Motion-View label=\"%s\"  => work=%s\n",
               feeds,infeed,disp.c_str(),disp.substr(disp.rfind('/')+1).c_str());
        printf("  full car kind map (serial->kind): ");
        for(int s=1;s<=total;s++) printf("%d:%s ",s,KN(NEW_GetFedTrayKind(s,total,useAMR,cover,identity)));
        int work=0,cv=0,id=0;
        for(int s=1;s<=total;s++){int k=NEW_GetFedTrayKind(s,total,useAMR,cover,identity); if(k==NORMAL)work++; else if(k==COVER)cv++; else id++;}
        printf("\n  => car of %d = %d work + %d cover + %d identity\n\n",total,work,cv,id);
    }

    // ---------- assertions ----------
    int oldWork, newWork;
    { int total=OLD_iCarTrayTotal(useAMR,secs,simMax); int infeed=total; for(int f=0;f<feeds;f++) if(infeed>0) infeed--;
      string d=FmtCarKinds(infeed,1,1); oldWork=atoi(d.substr(d.rfind('/')+1).c_str()); }
    { int total=NEW_iCarTrayTotal(useAMR,secs,simMax,cover,identity); int infeed=total; for(int f=0;f<feeds;f++) if(infeed>0) infeed--;
      string d=FmtCarKinds(infeed,identity,cover); newWork=atoi(d.substr(d.rfind('/')+1).c_str()); }
    printf("=== RESULT ===\n");
    printf("OLD work = %d  (expect 4, the reported bug)  -> %s\n", oldWork, oldWork==4?"MATCH":"MISMATCH");
    printf("NEW work = %d  (expect 6, correct)           -> %s\n", newWork, newWork==6?"MATCH":"MISMATCH");
    printf("\nVERDICT: %s\n", (oldWork==4 && newWork==6) ? "CONFIRMED - fix turns 4 into 6" : "UNEXPECTED - re-examine");
    return 0;
}
