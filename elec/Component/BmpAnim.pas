{
  A bitmap animator: animates a bitmap consisting of multiple likesized bitmaps
  like the explorer logo in Internet Explorer / Mail.

  Copyright © by Peter Thörnqvist May 1997; all rights reserved.
  Contact me at NMA96PTH@LUSTUDAT.STUDENT.LU.SE
  You are permitted to use this component free of charge,
  but this is NOT freeware: I retain all rights to it


  Properties:
    AutoSize : sizes the window to the frame size
    Color : the color to use as transparentcolor
    Direction: specifies which direction the frames are animated.
               Note that changing direction will reset the animation
               to the starting / ending frame.
    Enabled : run / stop animation
    Image : the bitmap to get the frames from
    NumFrames : number of frames in Image
    Orientation : frames are placed left-to-right or top-to-bottom in Image
    Position : what frame to display when stopped (disabled)
    Speed : frames / sec 
    Min : first frame to display
    Max : last frame to display
    Transparent : true if you want Color replaced by transparency

  Bugs:
  1 AutoSize doesn't get saved when exiting.
  2 Displaying some bitmaps looks OK when designing but turns black
    when running (explorer strip notably). Don't know why. Seems
    to be more common with vertical bitmaps (???) with black background
  3 Sometimes when manipulating the bitmap at design time, it turns black (like 2 above).
    Saving and reloading the app. seems to solve this.
    Also, displays correctly when run.


  Limitations:
  1 Uses a timer (oouuh, that's AWFUL!!!).
  2 Flickers a bit sometimes. Placing the control on a panel with the same
    general color, reduces the visibility of this.
  3 Delphi 2.0X (3.0X too maybe?) only.

  Installing:
  1 You need BmpAnim.pas and BmpAnim.dcr to install.
  2 Go about it in the normal fashion ( some help, huh? )
  3 Make some movies!
}

unit BmpAnim;
interface
uses
  SysUtils, Windows, Messages, Classes, Graphics, Controls,  CommCtrl,
  ExtCtrls;

type
  TOrientation=(toHorizontal,toVertical);
  TDirection=(tdForward,tdBack,tdFwdBack,tdBackFwd);
  TCustomBmpAnimator = class(TGraphicControl)
  private
    { Private declarations }
    FImageList:TImageList;
    FTimer:TTimer;
    FIndex:integer;
    FImage:TBitmap;
    FEnabled:boolean;
    FWidth,FHeight:integer;
    FNumGlyphs:integer;
    FOrientation:TOrientation;
    FSpeed:integer;
    FTransparent:boolean;
    FAutoSize:boolean;
    FStart,FStop:integer;
    FPosition:integer;
    FColor:TColor;
    FDirection:TDirection;
    FGoingUp:boolean;
    FCenter:boolean;
    procedure SetCenter(Value:boolean);
    procedure SetDirection(Value:TDirection);
    procedure SetColor(Value:TColor);
    procedure SetPosition(Value:integer);
    procedure SetStart(Value:integer);
    procedure SetStop(Value:integer);
    procedure SetAutoSize(Value:boolean);
    procedure SetTransparent(Value:boolean);
    procedure SetImage(Value:TBitmap);
    procedure SetEnabled(Value:boolean);
    procedure SetNumGlyphs(Value:integer);
    procedure SetOrientation(Value:Torientation);
    procedure SetSpeed(Value:integer);
    procedure TimerEvent(Sender:TObject);
    procedure UpdateImages;
  protected
    { Protected declarations }
    procedure Paint; override;
    property  AutoSize:boolean read FAutoSize write SetAutoSize default False;
    property  Centered:boolean read FCenter write SetCenter;
    property  Color:TColor read FColor write SetColor default clBtnFace;
    property  Direction: TDirection read FDirection write SetDirection;
    property  Enabled:boolean read FEnabled write SetEnabled default False;
    property  Image:TBitmap read FImage write SetImage;
    property  NumFrames:integer read FNumGlyphs write SetNumGlyphs default 0;
    property  Orientation:TOrientation read FOrientation write SetOrientation default toHorizontal;
    property  Position:integer read FPosition write SetPosition default 0;
    property  Speed:integer read FSpeed write SetSpeed default 100;
    property  Min:integer read FStart write SetStart default 0;
    property  Max:integer read FStop write SetStop default 0;
    property  Transparent:boolean read FTransparent write SetTransparent default False;
  public
    { Public declarations }
    constructor Create(AOwner:TComponent);override;
    destructor Destroy; override;
  published
    { Published declarations }
  end;

  TBmpAnimator=class(TCustomBmpAnimator)
  private
  protected
  public
  published
    property Align;
    property AutoSize;
    property Centered;
    property Color;
    property Direction;
    property Enabled;
    property Height;
    property Image;
    property Left;
    property Name;
    property NumFrames;
    property Orientation;
    property Position;
    property Speed;
    property Min;
    property Max;
    property Tag;
    property Top;
    property Transparent;
    property Width;
    property OnClick;
    property OnMouseDown;
    property OnMouseMove;
    property OnMouseUp;
    property OnDragDrop;
    property OnEndDrag;
    property OnStartDrag;
    property OnDragOver;
  end;

procedure Register;

implementation

constructor TCustomBmpAnimator.Create(AOwner:TComponent);
begin
  inherited Create(AOwner);
  FImage := TBitmap.Create;
  FWidth := 60;
  FHeight := 60;
  Width := FWidth;
  Height := FHeight;
  FTransparent := False;
  FAutoSize := False;
  FSpeed := 15;
  FNumGlyphs := 0;
  FIndex := 0;
  FStart := 0;
  FStop := 0;
  FPosition := 0;
  FEnabled := False;
  FColor := clBtnFace;
  FOrientation := toHorizontal;
  FImageList := TImageList.CreateSize(FWidth,FHeight);
  FTimer := TTimer.Create(nil);
  FTimer.OnTimer := TimerEvent;
  FTimer.Enabled := FEnabled;
  FTimer.Interval := 100;
  FDirection := tdForward;
  FGoingUp := True;
end;

destructor TCustomBmpAnimator.Destroy;
begin
  FImage.Free;
  FImageList.Free;
  FTimer.Enabled := False;
  FTimer.Free;
  inherited Destroy;
end;

procedure TCustomBmpAnimator.TimerEvent(Sender:TObject);
var dX,dY:integer;
begin
  if not FEnabled then FIndex := FPosition else
  case FDirection of
    tdForward:
    begin
      Inc(FIndex);
      if (FIndex > FNumGlyphs) or (FIndex > FStop) then
        FIndex := FStart;
    end;
    tdBack:
    begin
      Dec(FIndex);
      if (FIndex < 0) or (FIndex < FStart) then
        FIndex := FStop;
    end;

    tdFwdBack,tdBackFwd:
    begin
      if FGoingUp then
      begin
        if (FIndex >= FStop) then
        begin
          FGoingUp := False;
          Dec(FIndex);
        end
        else
          Inc(FIndex);
      end
      else
      begin
        if (FIndex <= FStart) then
        begin
          FGoingUp := True;
          Inc(FIndex);
        end
        else
          Dec(Findex);
      end;
    end;
  end;

  if FCenter then
  begin
    dX := (Width - FImageList.Width) div 2;
    dY := (Height - FImageList.Height) div 2;
  end
  else
  begin
    dX := 0;
    dY := 0;
  end;

  if FTransparent then
    ImageList_DrawEx(FImageList.Handle, FIndex, Canvas.Handle, dX,dY,0,0,
      clNone, clNone, ILD_Transparent)
  else
  begin
    Canvas.FillRect(ClientRect);
    ImageList_DrawEx(FImageList.Handle, FIndex, Canvas.Handle, dX,dY,0,0,
      ColorToRGB(FColor), FColor, ILD_Normal);
  end;
end;


procedure TCustomBmpAnimator.UpdateImages;
var i:integer;Bmp:TBitmap;Dest,Source:TRect;
begin
  Canvas.Brush.Color := FColor;
  FImageList.Clear;

  if FImage.Empty then Exit;
  if FNumGlyphs = 0 then SetNumGlyphs(1);
  if FOrientation = toHorizontal then
  begin
    FWidth := FImage.Width div FNumGlyphs;
    FHeight := FImage.Height;
  end
  else
  begin
    FWidth := FImage.Width;
    FHeight := FImage.Height div FNumGlyphs;
  end;


  if (FWidth <> FImageList.Width) or (FHeight <> FImageList.Height) then
  begin
    FImageList.Width := FWidth;
    FImageList.Height := FHeight;
  end;


  Bmp := TBitmap.Create;
 try
  Bmp.Width := FWidth;
  Bmp.Height := FHeight;
  Dest := Rect(0,0,FWidth,FHeight);
  { create the imagelist }
  for i := 0 to FNumGlyphs - 1 do
  begin
    if FOrientation = toHorizontal then
      Source := Rect(i * FWidth,0,i * FWidth + FWidth,FHeight)
    else
      Source := Rect(0,i * FHeight,FWidth,i * FHeight + FHeight);
    Bmp.Canvas.CopyRect(Bmp.Canvas.ClipRect,FImage.Canvas,Source);
    FImageList.AddMasked(Bmp,Bmp.TransparentColor)
  end;
  if FAutoSize and (Align = alNone) then
    SetBounds(Left,Top,FWidth,FHeight);
 finally
  Bmp.Free;
 end;
  FImage.Dormant;
  Repaint;
end;

procedure TCustomBmpAnimator.SetStart(Value:integer);
begin
  if FStart <> Value then
  begin
    FStart := Value;
    if FStart > FStop then FStart := FStop;
    if FStart >= FNumGlyphs then FStart := FNumGlyphs-1;
    if FStart < 0 then FStart := 0;
  end;
end;

procedure TCustomBmpAnimator.SetStop(Value:integer);
begin
  if FStop <> Value then
  begin
    FStop := Value;
    if FStop < FStart then FStop := FStart;
    if FStop > FNumGlyphs then FStop := FNumGlyphs-1;
    if FStop < 0 then FStop := 0;
  end;
end;


procedure TCustomBmpAnimator.SetAutoSize(Value:boolean);
begin
  if FAutoSize <> Value then
  begin
    FAutoSize := Value;
    UpdateImages;
  end;
end;

procedure TCustomBmpAnimator.SetTransparent(Value:boolean);
begin
  if FTransparent <> Value then
  begin
    FTransparent := Value;
    if FTransparent then
      FImageList.DrawingStyle := dsTransparent
    else
      FImageList.DrawingStyle := dsNormal;
    UpdateImages;
  end;
end;

procedure TCustomBmpAnimator.SetImage(Value:TBitmap);
begin
  if FImage <> Value then
  with FImage do
  begin
   Assign(Value);
   if not FImage.Empty then
   begin
     if Width > Height then SetOrientation(toHorizontal)
     else SetOrientation(toVertical);
     if Width mod Height = 0 then
       SetNumGlyphs(Width div Height)
     else if Height mod Width = 0 then
       SetNumGlyphs(Height div Width)
     else if FNumGlyphs = 0 then
       SetNumGlyphs(1);
     SetStart(FStart);
     SetStop(FNumGlyphs-1);
   end;
   UpdateImages;
  end;
end;

procedure TCustomBmpAnimator.SetEnabled(Value:boolean);
begin
  if not Assigned(FimageList) then
    Value := False;
  if FEnabled <> Value then
  begin
    FEnabled := Value;
    FTimer.Enabled := FEnabled;
    FIndex := FStart;
  end;
  Repaint;
end;


procedure TCustomBmpAnimator.SetNumGlyphs(Value:integer);
begin
  if FNumGlyphs <> Value then
  begin
    FNumGlyphs := Value;
    SetStop(FNumGlyphs-1);
    UpdateImages;
  end;
end;

procedure TCustomBmpAnimator.SetOrientation(Value:Torientation);
begin
  if FOrientation <> Value then
  begin
    FOrientation := Value;
    UpdateImages;
  end;
end;

procedure TCustomBmpAnimator.SetSpeed(Value:integer);
begin
  if FSpeed <> Value then
  begin
    FSpeed := Value;
    FTimer.Interval := 1000 div FSpeed;
  end;
end;

procedure TCustomBmpAnimator.SetCenter(Value:boolean);
begin
  if FCenter <> value then
  begin
    FCenter := Value;
    Invalidate;
  end;
end;

procedure TCustomBmpAnimator.SetDirection(Value:TDirection);
begin
  if FDirection <> Value then
  begin
    FDirection := Value;
    case FDirection of
      tdForward,tdFwdBack: begin
        FGoingUp := True;
        FIndex := FStart;
      end;
      tdBack,tdBackFwd:
      begin
        FGoingUp := False;
        FIndex := FStop;
      end;
    end;
  end;
end;

procedure TCustomBmpAnimator.SetColor(Value:TColor);
begin
  if FColor <> Value then
  begin
    FColor := Value;
    UpdateImages;
  end;
end;

procedure TCustomBmpAnimator.SetPosition(Value:integer);
begin
  FPosition := Value;
  if FPosition > FNumGlyphs-1 then
    FPosition := FNumGlyphs-1;
  Invalidate;
end;

procedure TCustomBmpAnimator.Paint;
var dX,dY:integer;
begin
    if {(FImage.Empty) or }(csDesigning in ComponentState) then
    begin
      Canvas.Brush.Color := clBlack;
//      Canvas.Pen.Color := FColor;
      Canvas.FrameRect(GetClientRect);
      Canvas.Brush.Color := FColor;
      if not FTransparent then
        Canvas.FillRect(GetClientRect);
    end;
    if not (FImage.Empty) then
    begin
      if FCenter then
      begin
        dX := (Width - FImageList.Width) div 2;
        dY := (Height - FImageList.Height) div 2;
      end
      else
      begin
        dX := 0;
        dY := 0;
      end;
    
      if FTransparent then
        ImageList_DrawEx(FImageList.Handle, FPosition, Canvas.Handle, dX,dY,0,0,
          clNone, FColor, ILD_Transparent)
      else
      begin
        Canvas.FillRect(ClientRect);
        ImageList_DrawEx(FImageList.Handle, FPosition, Canvas.Handle, dX,dY,0,0,
          ColorToRGB(FColor), FColor, ILD_Normal);
      end;
    end;
end;

procedure Register;
begin
  RegisterComponents('Samples', [TBmpAnimator]);
end;

end.
