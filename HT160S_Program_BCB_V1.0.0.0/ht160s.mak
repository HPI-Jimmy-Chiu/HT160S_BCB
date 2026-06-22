# ---------------------------------------------------------------------------
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

# ---------------------------------------------------------------------------
# IDE SECTION
# ---------------------------------------------------------------------------
# The following section of the project makefile is managed by the BCB IDE.
# It is recommended to use the IDE to change any of the values in this
# section.
# ---------------------------------------------------------------------------

VERSION = BCB.06.00
# ---------------------------------------------------------------------------
PROJECT = ..\EXE\ht160s.exe
OBJFILES = ht160s.obj main.obj iosetview.obj uteach.obj uMotorTest.obj uHome.obj \
    uQwertyKey.obj language.obj setup.obj data.obj maintenance.obj uOffset.obj \
    uspeed.obj systools.obj mymessbox.obj note.obj cmydef.obj cprod.obj \
    CosFunction.obj Include\cJSON.obj GeneralSetting.obj Config.obj \
    UserRoleManager.obj cCsvDailyLog.obj cEventLog.obj cCommLog.obj \
    cSelfCheck.obj cStepTrace.obj cStateRecordHT160.obj database.obj \
    aLoader.obj aEmpty.obj aAuto1To6.obj aTrayArm.obj aSortArm.obj aColor.obj \
    csystem.obj uruncontrol.obj HTimer.obj myio.obj myio_MN200.obj \
    mysensor.obj myswitch.obj mycylin.obj MyKitSuck.obj deviceinfo.obj \
    MotorAndIO\HTMotor.obj MotorAndIO\MyMotor.obj MotorAndIO\mySMCmotor.obj \
    MotorAndIO\myMN200motor.obj MotorAndIO\MC88X1PLazyLoad.obj \
    MotorAndIO\myMC88X1motor.obj AutomationServer.obj TopCcdSocket.obj \
    ColorCcdSocket.obj LotWebApiClient.obj ComPort.obj MyBinDisp.obj \
    uPadInterface.obj SecsGem\uHGemEquipment.obj SecsGem\UsecegemMainFrom.obj \
    SecsGem\uHGemClass.obj SecsGem\uHGemHT160.obj SecsGem\uAgvStation.obj \
    SecsGem\uHGemLogForm.obj
RESFILES = ht160s.res
MAINSOURCE = ht160s.cpp
RESDEPEN = $(RESFILES) main.dfm iosetview.dfm uteach.dfm uMotorTest.dfm uHome.dfm \
    uQwertyKey.dfm language.dfm setup.dfm data.dfm maintenance.dfm uOffset.dfm \
    uspeed.dfm systools.dfm mymessbox.dfm note.dfm database.dfm ComPort.dfm \
    SecsGem\uHGemLogForm.dfm
LIBFILES = 
IDLFILES = 
IDLGENFILES = 
LIBRARIES = ws2_32.lib
PACKAGES = vcl.bpi rtl.bpi bcb2kaxserver.bpi dbrtl.bpi adortl.bpi vcldb.bpi vclx.bpi \
    bdertl.bpi vcldbx.bpi ibxpress.bpi dsnap.bpi cds.bpi bdecds.bpi qrpt.bpi \
    teeui.bpi teedb.bpi tee.bpi dss.bpi teeqr.bpi visualclx.bpi \
    visualdbclx.bpi dsnapcrba.bpi dsnapcon.bpi bcbsmp.bpi vclie.bpi xmlrtl.bpi \
    inet.bpi inetdbbde.bpi inetdbxpress.bpi inetdb.bpi nmfast.bpi webdsnap.bpi \
    bcbie.bpi websnap.bpi soaprtl.bpi dclocx.bpi dbexpress.bpi dbxcds.bpi \
    indy.bpi dclusr60.bpi
SPARELIBS = vcl.lib rtl.lib ws2_32.lib dclusr60.lib dbrtl.lib vcldb.lib bdertl.lib
DEFFILE = 
OTHERFILES = 
# ---------------------------------------------------------------------------
DEBUGLIBPATH = $(BCB)\lib\debug
RELEASELIBPATH = $(BCB)\lib\release
USERDEFINES = _DEBUG
SYSDEFINES = _RTLDLL;NO_STRICT;USEPACKAGES
INCLUDEPATH = Include;SecsGem;MotorAndIO;$(BCB)\Projects;D:\HT160S_BCB\elec\Component;$(BCB)\include;$(BCB)\include\vcl;D:\HT160S_BCB\elec\myvcl
LIBPATH = Include;SecsGem;MotorAndIO;$(BCB)\Projects;D:\HT160S_BCB\elec\myvcl;$(BCB)\Projects\Lib;$(BCB)\lib\obj;$(BCB)\lib;D:\HT160S_BCB\elec\Component
WARNINGS= -w-par
PATHCPP = .;Include;MotorAndIO;SecsGem
PATHASM = .;
PATHPAS = .;
PATHRC = .;
PATHOBJ = .;$(LIBPATH)
# ---------------------------------------------------------------------------
CFLAG1 = -Od -H=..\Obj\ht160s.csm -Hc -Vx -Ve -X- -r- -a8 -b- -k -y -v -vi- -c -tW \
    -tWM
IDLCFLAGS = -ISecsGem -IMotorAndIO -I$(BCB)\Projects -ID:\HT160S_BCB\elec\Component \
    -I$(BCB)\include -I$(BCB)\include\vcl -ID:\HT160S_BCB\elec\myvcl \
    -src_suffix cpp -D_DEBUG -boa
PFLAGS = -$YD -$W -$O- -$A8 -v -JPHNE -M
RFLAGS = 
AFLAGS = /mx /w2 /zd
LFLAGS = -D"" -aa -Tpe -x -Gn -v
# ---------------------------------------------------------------------------
ALLOBJ = c0w32.obj $(PACKAGES) Memmgr.Lib sysinit.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) $(LIBRARIES) import32.lib cp32mti.lib
# ---------------------------------------------------------------------------
!ifdef IDEOPTIONS

[Version Info]
IncludeVerInfo=0
AutoIncBuild=0
MajorVer=1
MinorVer=0
Release=0
Build=0
Debug=0
PreRelease=0
Special=0
Private=0
DLL=0

[Version Info Keys]
CompanyName=
FileDescription=
FileVersion=1.0.0.0
InternalName=
LegalCopyright=
LegalTrademarks=
OriginalFilename=
ProductName=
ProductVersion=1.0.0.0
Comments=

[Debugging]
DebugSourceDirs=$(BCB)\source\vcl

!endif





# ---------------------------------------------------------------------------
# MAKE SECTION
# ---------------------------------------------------------------------------
# This section of the project file is not used by the BCB IDE.  It is for
# the benefit of building from the command-line using the MAKE utility.
# ---------------------------------------------------------------------------

.autodepend
# ---------------------------------------------------------------------------
!if "$(USERDEFINES)" != ""
AUSERDEFINES = -d$(USERDEFINES:;= -d)
!else
AUSERDEFINES =
!endif

!if !$d(BCC32)
BCC32 = bcc32
!endif

!if !$d(CPP32)
CPP32 = cpp32
!endif

!if !$d(DCC32)
DCC32 = dcc32
!endif

!if !$d(TASM32)
TASM32 = tasm32
!endif

!if !$d(LINKER)
LINKER = ilink32
!endif

!if !$d(BRCC32)
BRCC32 = brcc32
!endif


# ---------------------------------------------------------------------------
!if $d(PATHCPP)
.PATH.CPP = $(PATHCPP)
.PATH.C   = $(PATHCPP)
!endif

!if $d(PATHPAS)
.PATH.PAS = $(PATHPAS)
!endif

!if $d(PATHASM)
.PATH.ASM = $(PATHASM)
!endif

!if $d(PATHRC)
.PATH.RC  = $(PATHRC)
!endif

!if $d(PATHOBJ)
.PATH.OBJ  = $(PATHOBJ)
!endif
# ---------------------------------------------------------------------------
$(PROJECT): $(OTHERFILES) $(IDLGENFILES) $(OBJFILES) $(RESDEPEN) $(DEFFILE)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) -L$(LIBPATH) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES)
!
# ---------------------------------------------------------------------------
.pas.hpp:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.pas.obj:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.cpp.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.cpp.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.asm.obj:
    $(BCB)\BIN\$(TASM32) $(AFLAGS) -i$(INCLUDEPATH:;= -i) $(AUSERDEFINES) -d$(SYSDEFINES:;= -d) $<, $@

.rc.res:
    $(BCB)\BIN\$(BRCC32) $(RFLAGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -fo$@ $<



# ---------------------------------------------------------------------------




