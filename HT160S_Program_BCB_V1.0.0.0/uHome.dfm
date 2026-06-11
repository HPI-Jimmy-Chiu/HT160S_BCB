object fHome: TfHome
  Left = 182
  Top = 69
  BorderIcons = []
  BorderStyle = bsSingle
  Caption = 'Motor Home Monitor'
  ClientHeight = 681
  ClientWidth = 1217
  Color = 12761254
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel104: TPanel
    Left = 0
    Top = 0
    Width = 1217
    Height = 44
    Align = alTop
    BevelInner = bvLowered
    Caption = 'Motor Home Monitor'
    Color = 8421440
    Font.Charset = ANSI_CHARSET
    Font.Color = clWhite
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 0
  end
  object Panel1: TPanel
    Left = 0
    Top = 44
    Width = 1217
    Height = 455
    Align = alClient
    BevelInner = bvLowered
    Color = 12761254
    TabOrder = 1
  end
  object lstHomeMsg: TListBox
    Left = 0
    Top = 499
    Width = 1217
    Height = 121
    Align = alBottom
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ItemHeight = 16
    ParentFont = False
    TabOrder = 2
  end
  object Panel2: TPanel
    Left = 0
    Top = 620
    Width = 1217
    Height = 61
    Align = alBottom
    BevelOuter = bvNone
    Color = 12761254
    TabOrder = 3
    object SpeedButton1: TSpeedButton
      Left = 8
      Top = 8
      Width = 1201
      Height = 45
      Caption = 'Abort Home'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -21
      Font.Name = 'Trebuchet MS'
      Font.Style = [fsBold]
      ParentFont = False
      OnClick = SpeedButton1Click
    end
  end
  object Timer1: TTimer
    Interval = 100
    OnTimer = Timer1Timer
    Left = 1168
    Top = 8
  end
end
