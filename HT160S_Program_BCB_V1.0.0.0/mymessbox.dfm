object MyMessageBox: TMyMessageBox
  Left = 328
  Top = 430
  BorderIcons = []
  BorderStyle = bsSingle
  Caption = 'Message'
  ClientHeight = 216
  ClientWidth = 474
  Color = 12761254
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = True
  Position = poDefault
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 8
    Top = 8
    Width = 457
    Height = 153
    BevelInner = bvLowered
    Color = 14670284
    Font.Charset = ANSI_CHARSET
    Font.Color = clNavy
    Font.Height = -16
    Font.Name = 'MS Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 0
    object Label1: TLabel
      Left = 8
      Top = 40
      Width = 441
      Height = 33
      Alignment = taCenter
      AutoSize = False
      Caption = 'Message1'
      Font.Charset = ANSI_CHARSET
      Font.Color = clNavy
      Font.Height = -16
      Font.Name = 'MS Serif'
      Font.Style = []
      ParentFont = False
    end
    object Label2: TLabel
      Left = 0
      Top = 102
      Width = 441
      Height = 33
      Alignment = taCenter
      AutoSize = False
      Caption = 'Message2'
      Font.Charset = ANSI_CHARSET
      Font.Color = clNavy
      Font.Height = -16
      Font.Name = 'MS Serif'
      Font.Style = []
      ParentFont = False
    end
  end
  object palPause: TPanel
    Left = 120
    Top = 178
    Width = 117
    Height = 33
    BevelWidth = 2
    Caption = 'Pause'
    Color = 16757009
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 1
    OnClick = palPauseClick
  end
  object btnOffBuzzer: TButton
    Left = 384
    Top = 178
    Width = 90
    Height = 33
    Caption = 'Off Buzzer'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 2
    OnClick = btnOffBuzzerClick
  end
  object palYes: TPanel
    Tag = 1
    Left = 4
    Top = 178
    Width = 117
    Height = 33
    BevelWidth = 2
    Caption = 'Yes'
    Color = 16757009
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    Visible = False
    OnClick = palYesClick
  end
  object palNo: TPanel
    Tag = 2
    Left = 236
    Top = 178
    Width = 117
    Height = 33
    BevelWidth = 2
    Caption = 'No'
    Color = 16757009
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 4
    Visible = False
    OnClick = palYesClick
  end
end
