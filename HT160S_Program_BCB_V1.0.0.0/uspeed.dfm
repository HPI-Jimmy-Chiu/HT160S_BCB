object fSpeed: TfSpeed
  Left = 200
  Top = 100
  Width = 640
  Height = 560
  Caption = 'Speed Setup'
  Color = 12761254
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel10: TPanel
    Left = 0
    Top = 462
    Width = 632
    Height = 90
    Align = alBottom
    BevelOuter = bvNone
    Color = 12761254
    TabOrder = 0
    object spbFullSpeed: TButton
      Left = 12
      Top = 10
      Width = 140
      Height = 35
      Caption = 'Full Speed'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -16
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnClick = spbFullSpeedClick
    end
    object spbAddSpeed: TButton
      Left = 162
      Top = 10
      Width = 140
      Height = 35
      Caption = 'Speed +'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -16
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = spbAddSpeedClick
    end
    object spbSubSpeed: TButton
      Left = 312
      Top = 10
      Width = 140
      Height = 35
      Caption = 'Speed -'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -16
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 2
      OnClick = spbSubSpeedClick
    end
    object spbExit: TButton
      Left = 472
      Top = 10
      Width = 140
      Height = 35
      Caption = 'EXIT'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -16
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 3
      OnClick = spbExitClick
    end
  end
  object ScrollBox1: TScrollBox
    Left = 0
    Top = 0
    Width = 632
    Height = 462
    Align = alClient
    Color = 12761254
    ParentColor = False
    TabOrder = 1
  end
end
