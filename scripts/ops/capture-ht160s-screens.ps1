# capture-ht160s-screens.ps1
# Launch the HT160S sim exe and screenshot each operator screen for the manual.
# Best-effort: drives the GUI by clicking toolbar / Maintenance-hub buttons at
# coordinates derived from the .dfm layout, reads the foreground window title to
# verify which screen opened, captures it, and closes modals back to the parent.
# Whatever cannot be reached is simply skipped (a placeholder stays in the doc).
#
# Coordinates are CLIENT-relative (origin = form client top-left), computed from
# the .dfm as (designAbs - rootFormLeft/Top). They are resolved to screen pixels
# at runtime via ClientToScreen, so they survive the window being placed anywhere.
#
# Usage:  powershell -ExecutionPolicy Bypass -File scripts\ops\capture-ht160s-screens.ps1
param(
  [string]$ExePath = "D:\HT160S_BCB\EXE\ht160s.exe",
  [string]$WorkDir = "D:\HT160S_BCB\EXE",
  [string]$OutDir  = "D:\HT160S_BCB\docs\manual\screenshots",
  [int]$StartupWait = 9
)
$ErrorActionPreference = 'Stop'
if (-not (Test-Path $OutDir)) { New-Item -ItemType Directory -Force $OutDir | Out-Null }
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;using System.Text;using System.Collections.Generic;using System.Runtime.InteropServices;
public class W{
 public delegate bool EnumProc(IntPtr h,IntPtr p);
 [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb,IntPtr p);
 [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h,out uint pid);
 [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
 [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h,out R r);
 [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h,out R r);
 [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h,ref P p);
 [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
 [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
 [DllImport("user32.dll")] public static extern bool SetCursorPos(int x,int y);
 [DllImport("user32.dll")] public static extern void mouse_event(uint f,uint dx,uint dy,uint d,IntPtr e);
 [DllImport("user32.dll")] public static extern void keybd_event(byte vk,byte sc,uint f,IntPtr e);
 [DllImport("user32.dll")] public static extern IntPtr PostMessage(IntPtr h,uint m,IntPtr w,IntPtr l);
 [DllImport("user32.dll",CharSet=CharSet.Auto)] public static extern int GetWindowText(IntPtr h,StringBuilder s,int n);
 [StructLayout(LayoutKind.Sequential)] public struct R{public int Left,Top,Right,Bottom;}
 [StructLayout(LayoutKind.Sequential)] public struct P{public int X,Y;}
 public static List<IntPtr> Wins(uint pid){
   var l=new List<IntPtr>();
   EnumWindows((h,p)=>{uint q;GetWindowThreadProcessId(h,out q);if(q==pid&&IsWindowVisible(h)){l.Add(h);}return true;},IntPtr.Zero);
   return l;
 }
}
"@

function Title($h){ $sb=New-Object System.Text.StringBuilder 256; [W]::GetWindowText($h,$sb,256)|Out-Null; return $sb.ToString() }
function Area($h){ $r=New-Object W+R; [W]::GetWindowRect($h,[ref]$r)|Out-Null; return ($r.Right-$r.Left)*($r.Bottom-$r.Top) }

function Find-Main($procId){
  $best=[IntPtr]::Zero; $ba=0
  foreach($h in [W]::Wins([uint32]$procId)){ $a=Area $h; if($a -gt $ba){$ba=$a;$best=$h} }
  return $best
}
function Find-ByTitle($procId,$needle){
  foreach($h in [W]::Wins([uint32]$procId)){ if((Title $h) -like "*$needle*" -and (Area $h) -gt 1000){ return $h } }
  return [IntPtr]::Zero
}
function ClientOrigin($h){ $p=New-Object W+P; $p.X=0;$p.Y=0; [W]::ClientToScreen($h,[ref]$p)|Out-Null; return $p }

function Capture($h,$path){
  $r=New-Object W+R; [W]::GetWindowRect($h,[ref]$r)|Out-Null
  $w=$r.Right-$r.Left; $ht=$r.Bottom-$r.Top
  if($w -le 10 -or $ht -le 10){ return $false }
  $b=New-Object System.Drawing.Bitmap $w,$ht
  $g=[System.Drawing.Graphics]::FromImage($b)
  $g.CopyFromScreen($r.Left,$r.Top,0,0,$b.Size)
  $b.Save($path,[System.Drawing.Imaging.ImageFormat]::Png); $g.Dispose(); $b.Dispose()
  return $true
}
function ClickClient($h,$cx,$cy){
  [W]::SetForegroundWindow($h)|Out-Null; Start-Sleep -Milliseconds 350
  $o=ClientOrigin $h
  [W]::SetCursorPos($o.X+$cx,$o.Y+$cy)|Out-Null; Start-Sleep -Milliseconds 200
  [W]::mouse_event(0x02,0,0,0,[IntPtr]::Zero); [W]::mouse_event(0x04,0,0,0,[IntPtr]::Zero)
}
function CloseWin($h){
  [W]::SetForegroundWindow($h)|Out-Null; Start-Sleep -Milliseconds 200
  [W]::keybd_event(0x1B,0,0,[IntPtr]::Zero); [W]::keybd_event(0x1B,0,2,[IntPtr]::Zero)  # ESC
  Start-Sleep -Milliseconds 300
  [W]::PostMessage($h,0x0010,[IntPtr]::Zero,[IntPtr]::Zero)|Out-Null                     # WM_CLOSE
  Start-Sleep -Milliseconds 500
}

$manifest = @()
function Note($name,$file,$ok,$title){ $script:manifest += [pscustomobject]@{ name=$name; file=$file; ok=$ok; fgTitle=$title } }

# ----- launch -----
Write-Output "Launching $ExePath ..."
$proc = Start-Process -FilePath $ExePath -WorkingDirectory $WorkDir -PassThru
Start-Sleep -Seconds $StartupWait
$main = Find-Main $proc.Id
if($main -eq [IntPtr]::Zero){ Write-Output "ERROR: main window not found"; if(-not $proc.HasExited){$proc.Kill()}; return }
Write-Output ("Main window found: '{0}'" -f (Title $main))

# ----- main overview + lower tabs (in-place, no modal) -----
$null = Capture $main "$OutDir\main-overview.png"; Note "main-overview" "main-overview.png" $true (Title $main)

# Lower tab buttons (observed client coords in the 1264-wide main client)
$mainTabs = @(
  @{n="main-traystatus"; f="main-traystatus.png"; x=70;  y=458},
  @{n="main-logs";       f="main-logs.png";       x=197; y=458},
  @{n="main-timedata";   f="main-timedata.png";   x=327; y=458},
  @{n="main-maptray";    f="main-maptray.png";    x=453; y=458}
)
foreach($t in $mainTabs){
  try{
    ClickClient $main $t.x $t.y; Start-Sleep -Milliseconds 800
    $ok = Capture $main $("$OutDir\"+$t.f); Note $t.n $t.f $ok (Title $main)
  } catch { Note $t.n $t.f $false "exception" }
}

# ----- toolbar sub-screens from main (open modal -> capture -> close) -----
# client-relative centers (rootForm 309,54): Product=214, Offset=492, Speed=631, Tools=770, Message=909 ; all y=27
$toolbar = @(
  @{n="screen-product"; f="screen-product.png"; x=214; y=27},
  @{n="screen-offset";  f="screen-offset.png";  x=492; y=27},
  @{n="screen-speed";   f="screen-speed.png";   x=631; y=27},
  @{n="screen-tools";   f="screen-tools.png";   x=770; y=27},
  @{n="screen-message"; f="screen-message.png"; x=909; y=27}
)
foreach($t in $toolbar){
  try{
    ClickClient $main $t.x $t.y; Start-Sleep -Seconds 2
    $fg=[W]::GetForegroundWindow(); $ttl=Title $fg
    $ok = Capture $fg $("$OutDir\"+$t.f); Note $t.n $t.f $ok $ttl
    if($fg -ne $main){ CloseWin $fg }
    # ensure we are back on main
    $cur=[W]::GetForegroundWindow()
    if((Title $cur) -notlike "*HT160S*"){ if(-not $proc.HasExited){$proc.Kill()}; $proc=Start-Process -FilePath $ExePath -WorkingDirectory $WorkDir -PassThru; Start-Sleep -Seconds $StartupWait; $main=Find-Main $proc.Id }
  } catch { Note $t.n $t.f $false "exception" }
}

# ----- Maintenance hub -----
# open Maintenance (client 353,27 on main)
ClickClient $main 353 27; Start-Sleep -Seconds 2
$maint = Find-ByTitle $proc.Id "Maintance"
if($maint -ne [IntPtr]::Zero){
  Write-Output "Maintenance opened."
  # hub items: client x=1047, y = 33 + 56*index ; capture as the hub view OR the modal it opens
  $hub = @(
    @{n="screen-maintenance"; f="screen-maintenance.png"; y=33},   # Tower Light (default)
    @{n="screen-password";    f="screen-password.png";    y=89},
    @{n="screen-funcdef";     f="screen-funcdef.png";     y=201},
    @{n="screen-hardware";    f="screen-hardware.png";    y=257},
    @{n="screen-iosetview";   f="screen-iosetview.png";   y=313},   # IO Monitor
    @{n="screen-teach";       f="screen-teach.png";       y=369},
    @{n="screen-motortest";   f="screen-motortest.png";   y=425},
    @{n="screen-padcom";      f="screen-padcom.png";      y=481},
    @{n="screen-bindisplay";  f="screen-bindisplay.png";  y=537},
    @{n="screen-topccd";      f="screen-topccd.png";      y=593},
    @{n="screen-colorccd";    f="screen-colorccd.png";    y=649},
    @{n="screen-lotapi";      f="screen-lotapi.png";      y=705},
    @{n="secs-main";          f="secs-main.png";          y=761}    # SECS/GEM
  )
  foreach($it in $hub){
    try{
      ClickClient $maint 1047 $it.y; Start-Sleep -Seconds 2
      $fg=[W]::GetForegroundWindow(); $ttl=Title $fg
      $ok = Capture $fg $("$OutDir\"+$it.f); Note $it.n $it.f $ok $ttl
      # if a separate modal opened (not the Maintenance window), close it back to Maintenance
      if($fg -ne $maint -and ($ttl -notlike "*Maintance*")){
        CloseWin $fg; Start-Sleep -Milliseconds 400
        # re-acquire maintenance handle if needed
        $maint2 = Find-ByTitle $proc.Id "Maintance"
        if($maint2 -ne [IntPtr]::Zero){ $maint=$maint2 }
      }
    } catch { Note $it.n $it.f $false "exception" }
  }
} else {
  Write-Output "WARN: Maintenance window not found; hub screens skipped."
}

# ----- shutdown -----
Start-Sleep -Milliseconds 500
if(-not $proc.HasExited){ $proc.Kill() }
Write-Output "----- CAPTURE MANIFEST -----"
$manifest | Format-Table -AutoSize | Out-String | Write-Output
$manifest | ConvertTo-Json | Out-File -Encoding utf8 "$OutDir\_manifest.json"
$got = ($manifest | Where-Object { $_.ok }).Count
Write-Output ("Captured {0}/{1} screens -> {2}" -f $got, $manifest.Count, $OutDir)