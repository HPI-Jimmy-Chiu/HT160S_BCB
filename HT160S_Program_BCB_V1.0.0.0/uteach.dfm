object fTeach: TfTeach
  Left = 134
  Top = 33
  BorderIcons = []
  BorderStyle = bsSingle
  Caption = 'Teach'
  ClientHeight = 880
  ClientWidth = 1200
  Color = 12761254
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefault
  OnClose = FormClose
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object palMotorControl: TPanel
    Left = 839
    Top = 0
    Width = 361
    Height = 880
    Align = alRight
    BevelInner = bvLowered
    Color = 12761254
    TabOrder = 0
    object lblActiveMot: TLabel
      Left = 2
      Top = 2
      Width = 357
      Height = 34
      Alignment = taCenter
      AutoSize = False
      Caption = 'Activel Motor'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBlue
      Font.Height = -24
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentColor = False
      ParentFont = False
    end
    object lblNowPos: TLabel
      Left = 12
      Top = 102
      Width = 96
      Height = 20
      AutoSize = False
      Caption = 'Now Position'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clNavy
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblEncoder: TLabel
      Left = 12
      Top = 136
      Width = 96
      Height = 20
      AutoSize = False
      Caption = 'Encoder'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clNavy
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblSpeed: TLabel
      Left = 12
      Top = 170
      Width = 96
      Height = 20
      AutoSize = False
      Caption = 'Speed'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clNavy
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStep: TLabel
      Left = 12
      Top = 204
      Width = 96
      Height = 20
      AutoSize = False
      Caption = 'Step'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clNavy
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblTarget: TLabel
      Left = 12
      Top = 238
      Width = 96
      Height = 20
      AutoSize = False
      Caption = 'Move To'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clNavy
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus0: TLabel
      Left = 20
      Top = 382
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'CW'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus1: TLabel
      Left = 20
      Top = 406
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'HOME'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus2: TLabel
      Left = 20
      Top = 430
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'CCW'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus3: TLabel
      Left = 20
      Top = 454
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'EMG'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus4: TLabel
      Left = 20
      Top = 478
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'ALARM'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus5: TLabel
      Left = 20
      Top = 502
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'Soft CW'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus6: TLabel
      Left = 188
      Top = 382
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'Soft CCW'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus7: TLabel
      Left = 188
      Top = 406
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'Servo Alarm'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus8: TLabel
      Left = 188
      Top = 430
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'In Pos'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus9: TLabel
      Left = 188
      Top = 454
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'Z Phase'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus10: TLabel
      Left = 188
      Top = 478
      Width = 92
      Height = 18
      AutoSize = False
      Caption = 'Servo On'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblMotorList: TLabel
      Left = 20
      Top = 540
      Width = 110
      Height = 20
      AutoSize = False
      Caption = 'Motor List'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clNavy
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object palMotorName: TPanel
      Left = 12
      Top = 44
      Width = 337
      Height = 44
      BevelOuter = bvLowered
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
    end
    object edNowPos: TEdit
      Left = 116
      Top = 98
      Width = 126
      Height = 24
      ReadOnly = True
      TabOrder = 1
    end
    object edEncoder: TEdit
      Left = 116
      Top = 132
      Width = 126
      Height = 24
      ReadOnly = True
      TabOrder = 2
    end
    object edSpeed: TEdit
      Left = 116
      Top = 166
      Width = 126
      Height = 24
      TabOrder = 3
      Text = '10'
    end
    object edStep: TEdit
      Left = 116
      Top = 200
      Width = 126
      Height = 24
      TabOrder = 4
      Text = '1.00'
    end
    object edTarget: TEdit
      Left = 116
      Top = 234
      Width = 126
      Height = 24
      TabOrder = 5
      Text = '0.00'
    end
    object btnMotorSet: TButton
      Left = 252
      Top = 98
      Width = 96
      Height = 28
      Caption = 'SET'
      TabOrder = 6
    end
    object btnMove: TButton
      Left = 252
      Top = 232
      Width = 96
      Height = 30
      Caption = 'MOVE'
      TabOrder = 7
    end
    object btnJogP: TButton
      Left = 20
      Top = 282
      Width = 90
      Height = 34
      Caption = 'JOG +'
      TabOrder = 8
    end
    object btnJogN: TButton
      Left = 126
      Top = 282
      Width = 90
      Height = 34
      Caption = 'JOG -'
      TabOrder = 9
    end
    object btnStepP: TButton
      Left = 232
      Top = 282
      Width = 54
      Height = 34
      Caption = '+'
      TabOrder = 10
    end
    object btnStepN: TButton
      Left = 294
      Top = 282
      Width = 54
      Height = 34
      Caption = '-'
      TabOrder = 11
    end
    object btnHome: TButton
      Left = 20
      Top = 326
      Width = 96
      Height = 36
      Caption = 'HOME'
      TabOrder = 12
    end
    object btnStop: TButton
      Left = 132
      Top = 326
      Width = 96
      Height = 36
      Caption = 'STOP'
      TabOrder = 13
    end
    object btnRefresh: TButton
      Left = 244
      Top = 326
      Width = 96
      Height = 36
      Caption = 'REFRESH'
      TabOrder = 14
    end
    object ledStatus0: TALed
      Left = 118
      Top = 382
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object ledStatus1: TALed
      Left = 118
      Top = 406
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object ledStatus2: TALed
      Left = 118
      Top = 430
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object ledStatus3: TALed
      Left = 118
      Top = 454
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object ledStatus4: TALed
      Left = 118
      Top = 478
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object ledStatus5: TALed
      Left = 118
      Top = 502
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object ledStatus6: TALed
      Left = 286
      Top = 382
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object ledStatus7: TALed
      Left = 286
      Top = 406
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object ledStatus8: TALed
      Left = 286
      Top = 430
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object ledStatus9: TALed
      Left = 286
      Top = 454
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object ledStatus10: TALed
      Left = 286
      Top = 478
      Width = 22
      Height = 22
      LEDStyle = LEDSqLarge
    end
    object lstMotors: TListBox
      Left = 20
      Top = 562
      Width = 328
      Height = 260
      ItemHeight = 13
      TabOrder = 15
    end
  end
  object palClient: TPanel
    Left = 0
    Top = 0
    Width = 839
    Height = 880
    Align = alClient
    BevelOuter = bvNone
    Color = 12761254
    TabOrder = 1
    object palTitle: TPanel
      Left = 0
      Top = 0
      Width = 839
      Height = 58
      Align = alTop
      Caption = 'Teach'
      Color = 10263630
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clYellow
      Font.Height = -32
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 0
    end
    object palFunction: TPanel
      Left = 0
      Top = 58
      Width = 839
      Height = 52
      Align = alTop
      BevelOuter = bvLowered
      Color = 12761254
      TabOrder = 1
      object lblMessage: TLabel
        Left = 592
        Top = 15
        Width = 220
        Height = 22
        AutoSize = False
        Color = 12761254
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clNavy
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object btnSetTeach: TButton
        Left = 10
        Top = 10
        Width = 96
        Height = 30
        Caption = 'SET NOW'
        TabOrder = 0
      end
      object btnGoTeach: TButton
        Left = 114
        Top = 10
        Width = 86
        Height = 30
        Caption = 'GO'
        TabOrder = 1
      end
      object btnSave: TButton
        Left = 208
        Top = 10
        Width = 86
        Height = 30
        Caption = 'SAVE'
        TabOrder = 2
      end
      object btnReload: TButton
        Left = 302
        Top = 10
        Width = 86
        Height = 30
        Caption = 'RELOAD'
        TabOrder = 3
      end
      object btnIOForm: TButton
        Left = 396
        Top = 10
        Width = 86
        Height = 30
        Caption = 'IO TOOL'
        TabOrder = 4
      end
      object btnClose: TButton
        Left = 490
        Top = 10
        Width = 86
        Height = 30
        Caption = 'EXIT'
        TabOrder = 5
      end
    end
    object PageTeach: TPageControl
      Left = 0
      Top = 110
      Width = 839
      Height = 770
      ActivePage = tsEmptyTray
      Align = alClient
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 2
      object tsEmptyTray: TTabSheet
        Caption = 'Empty / Tray X'
        object grdEmptyTray: TStringGrid
          Left = 0
          Top = 0
          Width = 831
          Height = 742
          Align = alClient
          ColCount = 5
          DefaultRowHeight = 24
          FixedRows = 1
          RowCount = 2
          TabOrder = 0
        end
      end
      object tsLoaderSort: TTabSheet
        Caption = 'Loader / Sort X'
        ImageIndex = 1
        object grdLoaderSort: TStringGrid
          Left = 0
          Top = 0
          Width = 831
          Height = 742
          Align = alClient
          ColCount = 5
          DefaultRowHeight = 24
          FixedRows = 1
          RowCount = 2
          TabOrder = 0
        end
      end
      object tsAuto: TTabSheet
        Caption = 'Auto 1-6'
        ImageIndex = 2
        object grdAuto: TStringGrid
          Left = 0
          Top = 0
          Width = 831
          Height = 742
          Align = alClient
          ColCount = 5
          DefaultRowHeight = 24
          FixedRows = 1
          RowCount = 2
          TabOrder = 0
        end
      end
      object tsSortZ: TTabSheet
        Caption = 'Sort Z'
        ImageIndex = 3
        object grdSortZ: TStringGrid
          Left = 0
          Top = 0
          Width = 831
          Height = 742
          Align = alClient
          ColCount = 5
          DefaultRowHeight = 24
          FixedRows = 1
          RowCount = 2
          TabOrder = 0
        end
      end
      object tsOthers: TTabSheet
        Caption = 'Others'
        ImageIndex = 4
        object grdOthers: TStringGrid
          Left = 0
          Top = 0
          Width = 831
          Height = 742
          Align = alClient
          ColCount = 5
          DefaultRowHeight = 24
          FixedRows = 1
          RowCount = 2
          TabOrder = 0
        end
      end
    end
  end
  object tmrUpdate: TTimer
    Enabled = False
    Interval = 200
    Left = 664
    Top = 16
  end
end