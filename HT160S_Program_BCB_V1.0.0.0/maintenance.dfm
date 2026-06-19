object fMaintenance: TfMaintenance
  Left = 375
  Top = 187
  BorderStyle = bsSingle
  Caption = 'Maintance'
  ClientHeight = 987
  ClientWidth = 1145
  Color = 11250603
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
  object pnlMenu: TPanel
    Left = 949
    Top = 0
    Width = 196
    Height = 987
    Align = alRight
    BevelOuter = bvNone
    Color = 12761254
    TabOrder = 0
    object spbMaintTowerLight: TSpeedButton
      Left = 8
      Top = 8
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Tower Light'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintPassword: TSpeedButton
      Left = 8
      Top = 64
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Password'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintSoftSimu: TSpeedButton
      Left = 8
      Top = 120
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Soft Simulate'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintFunctionDef: TSpeedButton
      Left = 8
      Top = 176
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Function Define'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintHardware: TSpeedButton
      Left = 8
      Top = 232
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Hardware Setup'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintIO: TSpeedButton
      Left = 8
      Top = 288
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'IO Monitor'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintTeach: TSpeedButton
      Left = 8
      Top = 344
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Teach'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintMotor: TSpeedButton
      Left = 8
      Top = 400
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Motor Test'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintCOM: TSpeedButton
      Left = 8
      Top = 456
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Pad COM Port'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintSECS: TSpeedButton
      Left = 8
      Top = 736
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'SECS/GEM'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintMCUDisplay: TSpeedButton
      Left = 8
      Top = 512
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Bin Display'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintTopCcd: TSpeedButton
      Left = 8
      Top = 568
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Top CCD'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintColorCcd: TSpeedButton
      Left = 8
      Top = 624
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Color CCD'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintLotApi: TSpeedButton
      Left = 8
      Top = 680
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Lot WebAPI'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
    object spbMaintExit: TSpeedButton
      Left = 8
      Top = 852
      Width = 180
      Height = 50
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Exit'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnClick = spbMaintenanceMenuClick
    end
  end
  object pnlClient: TPanel
    Left = 0
    Top = 0
    Width = 949
    Height = 987
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 1
    object pnlTitle: TPanel
      Left = 0
      Top = 0
      Width = 949
      Height = 44
      Align = alTop
      Caption = 'Maintenance'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -24
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 0
    end
    object pcMaintenance: TPageControl
      Left = 0
      Top = 44
      Width = 949
      Height = 943
      ActivePage = tsMaintCOM
      Align = alClient
      TabIndex = 9
      TabOrder = 1
      TabWidth = 90
      object tsMaintTowerLight: TTabSheet
        Caption = 'Tower Light'
        object Panel2: TPanel
          Left = 0
          Top = 0
          Width = 941
          Height = 31
          Align = alTop
          BevelInner = bvLowered
          Caption = 'Tower light and  music setup'
          Color = clTeal
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -16
          Font.Name = 'MS Sans Serif'
          Font.Style = []
          ParentFont = False
          TabOrder = 0
        end
        object Panel12: TPanel
          Left = 0
          Top = 31
          Width = 941
          Height = 884
          Align = alClient
          BevelInner = bvLowered
          Color = 12761254
          TabOrder = 1
          object RGB50: TALed
            Tag = 15
            Left = 152
            Top = 260
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB40: TALed
            Tag = 12
            Left = 152
            Top = 219
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB30: TALed
            Tag = 9
            Left = 152
            Top = 177
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB20: TALed
            Tag = 6
            Left = 152
            Top = 136
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB10: TALed
            Tag = 3
            Left = 152
            Top = 95
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB00: TALed
            Left = 152
            Top = 54
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB01: TALed
            Tag = 1
            Left = 208
            Top = 54
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB11: TALed
            Tag = 4
            Left = 208
            Top = 95
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB21: TALed
            Tag = 7
            Left = 208
            Top = 136
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB31: TALed
            Tag = 10
            Left = 208
            Top = 177
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB41: TALed
            Tag = 13
            Left = 208
            Top = 219
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB51: TALed
            Tag = 16
            Left = 208
            Top = 260
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB52: TALed
            Tag = 17
            Left = 264
            Top = 260
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB42: TALed
            Tag = 14
            Left = 264
            Top = 219
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB32: TALed
            Tag = 11
            Left = 264
            Top = 177
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB22: TALed
            Tag = 8
            Left = 264
            Top = 136
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB12: TALed
            Tag = 5
            Left = 264
            Top = 95
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object RGB02: TALed
            Tag = 2
            Left = 264
            Top = 54
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            OnClick = RGB00Click
          end
          object Label3: TLabel
            Left = 144
            Top = 18
            Width = 45
            Height = 20
            AutoSize = False
            Caption = 'Green'
          end
          object Label4: TLabel
            Left = 199
            Top = 18
            Width = 46
            Height = 20
            AutoSize = False
            Caption = 'Yellow'
          end
          object Label6: TLabel
            Left = 261
            Top = 18
            Width = 30
            Height = 20
            AutoSize = False
            Caption = 'Red'
          end
          object Label7: TLabel
            Left = 315
            Top = 19
            Width = 461
            Height = 21
            Alignment = taCenter
            AutoSize = False
            Caption = 'Music Select'
            Color = 4227072
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWhite
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentColor = False
            ParentFont = False
          end
          object Label43: TLabel
            Left = 288
            Top = 303
            Width = 74
            Height = 13
            Caption = 'Click to change'
          end
          object Label17: TLabel
            Left = 315
            Top = 392
            Width = 461
            Height = 21
            Alignment = taCenter
            AutoSize = False
            Caption = 'Music Test'
            Color = 4227072
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWhite
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentColor = False
            ParentFont = False
          end
          object Bevel1: TBevel
            Left = 156
            Top = 285
            Width = 2
            Height = 50
          end
          object Bevel2: TBevel
            Left = 211
            Top = 285
            Width = 2
            Height = 50
          end
          object Bevel3: TBevel
            Left = 266
            Top = 285
            Width = 2
            Height = 50
          end
          object Bevel4: TBevel
            Left = 157
            Top = 335
            Width = 129
            Height = 2
          end
          object Panel13: TPanel
            Left = 24
            Top = 44
            Width = 116
            Height = 31
            Caption = 'Running'
            Color = 8421440
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 0
          end
          object Panel14: TPanel
            Left = 24
            Top = 85
            Width = 116
            Height = 31
            Caption = 'Error/Jam'
            Color = 8421440
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 1
          end
          object Panel15: TPanel
            Left = 24
            Top = 126
            Width = 116
            Height = 31
            Caption = 'Pause'
            Color = 8421440
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 2
          end
          object Panel16: TPanel
            Left = 24
            Top = 168
            Width = 116
            Height = 31
            Caption = 'Message'
            Color = 8421440
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 3
          end
          object Panel17: TPanel
            Left = 24
            Top = 209
            Width = 116
            Height = 31
            Caption = 'Heating'
            Color = 8421440
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 4
          end
          object Panel18: TPanel
            Left = 24
            Top = 250
            Width = 116
            Height = 31
            Caption = 'Homing'
            Color = 8421440
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 5
          end
          object RadioGroup2: TRadioGroup
            Tag = 1
            Left = 314
            Top = 41
            Width = 462
            Height = 40
            Columns = 5
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -15
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ItemIndex = 0
            Items.Strings = (
              '[ 0 ] Mute'
              '[ 1 ] Music 1'
              '[ 2 ] Music 2'
              '[ 3 ] Music 3'
              '[ 4 ] Music 4')
            ParentFont = False
            TabOrder = 6
          end
          object RadioGroup3: TRadioGroup
            Tag = 2
            Left = 314
            Top = 83
            Width = 462
            Height = 40
            Columns = 5
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -15
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ItemIndex = 0
            Items.Strings = (
              '[ 0 ] Mute'
              '[ 1 ] Music 1'
              '[ 2 ] Music 2'
              '[ 3 ] Music 3'
              '[ 4 ] Music 4')
            ParentFont = False
            TabOrder = 7
          end
          object RadioGroup4: TRadioGroup
            Tag = 3
            Left = 314
            Top = 124
            Width = 462
            Height = 40
            Columns = 5
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -15
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ItemIndex = 0
            Items.Strings = (
              '[ 0 ] Mute'
              '[ 1 ] Music 1'
              '[ 2 ] Music 2'
              '[ 3 ] Music 3'
              '[ 4 ] Music 4')
            ParentFont = False
            TabOrder = 8
          end
          object RadioGroup5: TRadioGroup
            Tag = 4
            Left = 314
            Top = 166
            Width = 462
            Height = 40
            Columns = 5
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -15
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ItemIndex = 0
            Items.Strings = (
              '[ 0 ] Mute'
              '[ 1 ] Music 1'
              '[ 2 ] Music 2'
              '[ 3 ] Music 3'
              '[ 4 ] Music 4')
            ParentFont = False
            TabOrder = 9
          end
          object RadioGroup6: TRadioGroup
            Tag = 5
            Left = 314
            Top = 207
            Width = 462
            Height = 40
            Columns = 5
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -15
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ItemIndex = 0
            Items.Strings = (
              '[ 0 ] Mute'
              '[ 1 ] Music 1'
              '[ 2 ] Music 2'
              '[ 3 ] Music 3'
              '[ 4 ] Music 4')
            ParentFont = False
            TabOrder = 10
          end
          object RadioGroup7: TRadioGroup
            Tag = 6
            Left = 314
            Top = 249
            Width = 462
            Height = 40
            Columns = 5
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -15
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ItemIndex = 0
            Items.Strings = (
              '[ 0 ] Mute'
              '[ 1 ] Music 1'
              '[ 2 ] Music 2'
              '[ 3 ] Music 3'
              '[ 4 ] Music 4')
            ParentFont = False
            TabOrder = 11
          end
          object Panel19: TPanel
            Left = 315
            Top = 424
            Width = 461
            Height = 125
            BevelInner = bvSpace
            BevelOuter = bvLowered
            Color = 8421440
            TabOrder = 12
            object sbMusic1: TSpeedButton
              Tag = 1
              Left = 30
              Top = 16
              Width = 170
              Height = 39
              AllowAllUp = True
              GroupIndex = 2
              Caption = 'Music1'
              OnClick = sbMusic1Click
            end
            object sbMusic2: TSpeedButton
              Tag = 2
              Left = 30
              Top = 70
              Width = 170
              Height = 39
              AllowAllUp = True
              GroupIndex = 2
              Caption = 'Music2'
              OnClick = sbMusic1Click
            end
            object sbMusic3: TSpeedButton
              Tag = 3
              Left = 255
              Top = 16
              Width = 170
              Height = 39
              AllowAllUp = True
              GroupIndex = 2
              Caption = 'Music3'
              OnClick = sbMusic1Click
            end
            object sbMusic4: TSpeedButton
              Tag = 4
              Left = 255
              Top = 70
              Width = 170
              Height = 39
              AllowAllUp = True
              GroupIndex = 2
              Caption = 'Music4'
              OnClick = sbMusic1Click
            end
          end
        end
      end
      object tsMaintPassword: TTabSheet
        Caption = 'Password'
      end
      object tsMaintSoftSimu: TTabSheet
        Caption = 'Soft Simulate'
      end
      object tsMaintFunctionDef: TTabSheet
        Caption = 'Function Define'
        object pnlFunctionDefHeader: TPanel
          Left = 0
          Top = 0
          Width = 941
          Height = 31
          Align = alTop
          BevelInner = bvLowered
          Caption = 'Function define setup'
          Color = clTeal
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -16
          Font.Name = 'MS Sans Serif'
          Font.Style = []
          ParentFont = False
          TabOrder = 0
        end
        object pnlFunctionDefBody: TPanel
          Left = 0
          Top = 31
          Width = 941
          Height = 884
          Align = alClient
          BevelInner = bvLowered
          Color = 12761254
          TabOrder = 1
          object pgcFunctionDef: TPageControl
            Left = 2
            Top = 2
            Width = 937
            Height = 880
            ActivePage = tsNetwork
            Align = alClient
            TabIndex = 1
            TabOrder = 0
            object tsFunctionGeneral: TTabSheet
              Caption = 'G[General]'
            end
            object tsNetwork: TTabSheet
              Caption = 'N[Network]'
              ImageIndex = 1
              object Panel4: TPanel
                Left = 0
                Top = 0
                Width = 929
                Height = 40
                Align = alTop
                BevelInner = bvLowered
                TabOrder = 0
                Visible = False
              end
            end
          end
        end
      end
      object tsMaintHardware: TTabSheet
        Caption = 'Hardware Setup'
        object pnlHardwareHeader: TPanel
          Left = 0
          Top = 0
          Width = 941
          Height = 31
          Align = alTop
          BevelInner = bvLowered
          Caption = 'Hardware install setup'
          Color = clTeal
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -16
          Font.Name = 'MS Sans Serif'
          Font.Style = []
          ParentFont = False
          TabOrder = 0
        end
        object pnlHardwareBody: TPanel
          Left = 0
          Top = 31
          Width = 941
          Height = 884
          Align = alClient
          BevelInner = bvLowered
          Color = 12761254
          TabOrder = 1
          object PageControl1: TPageControl
            Left = 2
            Top = 2
            Width = 937
            Height = 880
            ActivePage = tsLoaderUnloader
            Align = alClient
            TabIndex = 0
            TabOrder = 0
            object tsLoaderUnloader: TTabSheet
              Caption = 'Loader/Unloader'
              object pnlHardwareOptionBox: TPanel
                Left = 0
                Top = 0
                Width = 827
                Height = 40
                Align = alTop
                BevelInner = bvLowered
                TabOrder = 0
                object lblHardwareColorHint: TLabel
                  Left = 220
                  Top = 12
                  Width = 479
                  Height = 16
                  AutoSize = False
                  Caption = 
                    'Enable this only when the Color bin area hardware is installed o' +
                    'n this machine.'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clNavy
                  Font.Height = -13
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                end
                object chkHardwareColorBinArea: TCheckBox
                  Left = 16
                  Top = 10
                  Width = 193
                  Height = 20
                  Caption = 'Color bin area installed'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 0
                  OnClick = chkHardwareColorBinAreaClick
                end
              end
              object Panel3: TPanel
                Left = 0
                Top = 40
                Width = 827
                Height = 40
                Align = alTop
                BevelInner = bvLowered
                TabOrder = 1
                object chkUseAMR: TCheckBox
                  Left = 16
                  Top = 10
                  Width = 260
                  Height = 20
                  Caption = 'Use AMR'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 0
                  OnClick = chkUseAMRClick
                end
              end
              object pnlSortModeBox: TPanel
                Left = 0
                Top = 80
                Width = 827
                Height = 64
                Align = alTop
                BevelInner = bvLowered
                TabOrder = 2
                object lblLotBinModeHint: TLabel
                  Left = 240
                  Top = 12
                  Width = 560
                  Height = 36
                  AutoSize = False
                  Caption =
                    'When ON, classify By Lot+Bin (each Lot+Bin pair binds to an Auto' +
                    ' dynamically). When OFF, Normal mode (static Bin->Auto table). C' +
                    'hanging this needs a software restart.'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clNavy
                  Font.Height = -13
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  WordWrap = True
                end
                object chkUseLotBinMode: TCheckBox
                  Left = 16
                  Top = 20
                  Width = 210
                  Height = 20
                  Caption = 'Sort By Lot+Bin'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 0
                  OnClick = chkUseLotBinModeClick
                end
              end
              object pnlAutoEnableBox: TPanel
                Left = 0
                Top = 144
                Width = 827
                Height = 80
                Align = alTop
                BevelInner = bvLowered
                TabOrder = 3
                object lblAutoEnableHint: TLabel
                  Left = 16
                  Top = 10
                  Width = 780
                  Height = 16
                  AutoSize = False
                  Caption =
                    'Per-Auto enable (By Lot+Bin mode only). Unchecked Autos are skip' +
                    'ped when binding new Lot+Bin pairs. Changing this needs a softwar' +
                    'e restart.'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clNavy
                  Font.Height = -13
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                end
                object chkAutoEnable1: TCheckBox
                  Left = 16
                  Top = 44
                  Width = 110
                  Height = 20
                  Caption = 'Auto1'
                  Checked = True
                  State = cbChecked
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 0
                  OnClick = chkAutoEnableClick
                end
                object chkAutoEnable2: TCheckBox
                  Left = 136
                  Top = 44
                  Width = 110
                  Height = 20
                  Caption = 'Auto2'
                  Checked = True
                  State = cbChecked
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 1
                  OnClick = chkAutoEnableClick
                end
                object chkAutoEnable3: TCheckBox
                  Left = 256
                  Top = 44
                  Width = 110
                  Height = 20
                  Caption = 'Auto3'
                  Checked = True
                  State = cbChecked
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 2
                  OnClick = chkAutoEnableClick
                end
                object chkAutoEnable4: TCheckBox
                  Left = 376
                  Top = 44
                  Width = 110
                  Height = 20
                  Caption = 'Auto4'
                  Checked = True
                  State = cbChecked
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 3
                  OnClick = chkAutoEnableClick
                end
                object chkAutoEnable5: TCheckBox
                  Left = 496
                  Top = 44
                  Width = 110
                  Height = 20
                  Caption = 'Auto5'
                  Checked = True
                  State = cbChecked
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 4
                  OnClick = chkAutoEnableClick
                end
                object chkAutoEnable6: TCheckBox
                  Left = 616
                  Top = 44
                  Width = 110
                  Height = 20
                  Caption = 'Auto6'
                  Checked = True
                  State = cbChecked
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 5
                  OnClick = chkAutoEnableClick
                end
              end
              object pnlSuckerEnableBox: TPanel
                Left = 0
                Top = 224
                Width = 827
                Height = 80
                Align = alTop
                BevelInner = bvLowered
                TabOrder = 4
                object lblSuckerEnableHint: TLabel
                  Left = 16
                  Top = 10
                  Width = 780
                  Height = 16
                  AutoSize = False
                  Caption =
                    'Per-nozzle enable (SortArm sucker). Unchecked nozzles are skippe' +
                    'd during pick/place so a broken nozzle can be taken out of servi' +
                    'ce. At least one must stay enabled.'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clNavy
                  Font.Height = -13
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                end
                object chkSuckEnable1: TCheckBox
                  Left = 16
                  Top = 44
                  Width = 110
                  Height = 20
                  Caption = 'Nozzle1'
                  Checked = True
                  State = cbChecked
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 0
                  OnClick = chkSuckEnableClick
                end
                object chkSuckEnable2: TCheckBox
                  Left = 136
                  Top = 44
                  Width = 110
                  Height = 20
                  Caption = 'Nozzle2'
                  Checked = True
                  State = cbChecked
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 1
                  OnClick = chkSuckEnableClick
                end
                object chkSuckEnable3: TCheckBox
                  Left = 256
                  Top = 44
                  Width = 110
                  Height = 20
                  Caption = 'Nozzle3'
                  Checked = True
                  State = cbChecked
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 2
                  OnClick = chkSuckEnableClick
                end
                object chkSuckEnable4: TCheckBox
                  Left = 376
                  Top = 44
                  Width = 110
                  Height = 20
                  Caption = 'Nozzle4'
                  Checked = True
                  State = cbChecked
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  TabOrder = 3
                  OnClick = chkSuckEnableClick
                end
              end
            end
            object tsErrorMag: TTabSheet
              Caption = 'ErrorMag'
              ImageIndex = 1
              object pnlHardwareErrorBinBox: TPanel
                Left = 0
                Top = 0
                Width = 827
                Height = 170
                Align = alTop
                BevelInner = bvLowered
                TabOrder = 0
                object lblHardwareErrorTitle: TLabel
                  Left = 20
                  Top = 18
                  Width = 175
                  Height = 20
                  Caption = 'Error Bin Code Frame'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clWindowText
                  Font.Height = -16
                  Font.Name = 'MS Sans Serif'
                  Font.Style = [fsBold]
                  ParentFont = False
                end
                object lblHardwareErrorCode1000: TLabel
                  Left = 40
                  Top = 58
                  Width = 4
                  Height = 16
                  Caption = '-'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clNavy
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                end
                object lblHardwareErrorCode1001: TLabel
                  Left = 40
                  Top = 88
                  Width = 4
                  Height = 16
                  Caption = '-'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clNavy
                  Font.Height = -15
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                end
                object lblHardwareErrorHint: TLabel
                  Left = 40
                  Top = 124
                  Width = 660
                  Height = 30
                  AutoSize = False
                  Caption = 
                    'Special error bins follow the recipe Error Bin area by default a' +
                    'nd can be extended to per-code area mapping later.'
                  Font.Charset = DEFAULT_CHARSET
                  Font.Color = clNavy
                  Font.Height = -13
                  Font.Name = 'MS Sans Serif'
                  Font.Style = []
                  ParentFont = False
                  WordWrap = True
                end
              end
            end
          end
          object Panel1: TPanel
            Left = 2
            Top = 882
            Width = 937
            Height = 0
            Align = alBottom
            Color = 12761254
            TabOrder = 1
            Visible = False
            object sbUpdateTray: TSpeedButton
              Left = 504
              Top = 9
              Width = 170
              Height = 36
              Caption = 'Save'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWindowText
              Font.Height = -19
              Font.Name = 'Arial'
              Font.Style = []
              Glyph.Data = {
                660F0000424D660F000000000000360000002800000024000000240000000100
                180000000000300F000001000000010000000000000000000000FAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAF9F9F9F9F9F9F8F8F8F8F8F8F8
                F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8
                F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F9F9F9F9F9F9FAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAF7F7F7EFEFEFE8
                E8E8E6E6E6E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5
                E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5E5
                E5E5E5E5E9E9E9F3F3F3F9F9F9FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAF8F8F8ECECECD6D6D6C8C8C8C5C5C5C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4
                C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4
                C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4C4D0D0D0E9E9E9F9F9F9FAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAF0EFEFC0A6A7A55B6CA96270A45E6C934556
                A6A3A3A4A1A1A19C9C9C9797979393948D8D8E88888A8383867F7D837A7A8079
                78807978994759994759994759994759994759994759994759994756C4C4C4E5
                E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAD2C1C1B28181
                AF6D79B8717FB7707DAA6472EFEFF1EEEEF1995D68B8717EB06B78E1E0E0DDDA
                DBD8D5D6D4D1D2CFCCCCCAC6C6DBD8D88F4B59A05565A05565BE848EB9727FB8
                727EB8717E994557C4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAAE6A7AAF6D79BA7481B97380B7737FAA6472E9E9EBECEBEF995D
                68B8717EB06B78E1DFE1DCDADDD7D5D6D2D0D1CDC9CAC8C3C3D9D6D68D49589F
                5464A05565BF848EB97380B9737FB9727F9B4457C4C4C4E5E5E5F8F8F8FAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7AC38D95BA7682BA7481B974
                80AA6472E5E4E7E9E9EB9A5D69B8737FB06B78E5E5E7E2E0E3DDDBDDD8D5D7D3
                D0D1CDCACBDDDADB8B48579D53639F5464BF858FBA7480BA7481BA74819B4558
                C4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A
                7AC48E96BA7783BA7682B97481AA6472E0DFE1E5E3E69B5F6BB97580B06B78E9
                EAECE6E5E8E2E1E4DDDCDED9D6D8D4D0D2E0DFDF8945549B50619D5363BF8590
                BA7481BA7481BA74819A4759C4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAAE6A7AC58F96BB7784BA7784BA7582AA6472DAD9DBE0
                DEE09B5F6BBA7582B06B78EDECEFEAE9EDE6E6E9E2E2E4DEDDDFDAD8DAE5E2E4
                874353994E5F9B5061C18690B97480BA7481BA74819C495CC4C4C4E5E5E5F8F8
                F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7AC58F98BC7886BB
                7785BB7783AA6472D5D2D4DBD8DA9C606C925864925864EAEAEDEDEDF0EAEAED
                E8E7E9E4E2E5DFDDDFE8E7E8864252974C5D994E5FC08691B9727FB97380B973
                809C4B5CC4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAAE6A7AC49098BC7986BD7985BC7784AA6472CFCCCCD4D1D3D9D7D9DFDCDF
                E2E1E4E7E5E8EAEAEDEDECF0EAEBEEE8E7EAE4E2E6ECEAEC8642528642528743
                53C28791B7727FB8727EB9727F9C4E5DC4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7AC79198BD7A87BD7987BC7985BB7785
                AA6472AA6472AA6472AA6472AA6472AA6472AA6472AA6472AA6472AA6472AA64
                72AA6472AA6472AA6472B77A84B76F7DB7707DB7707EB8717E9E4E5FC4C4C4E5
                E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7AC6929A
                BE7B88BE7B87BC7986BC7886BB7785BB7784BA7682B97481B97480B87380B871
                7EB7717EB76F7DB76D7CB76D7BB76D7BB66D7BB56C7AB56C7AB66D7BB76F7CB7
                6F7CB7707D9F4E60C4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAAE6A7AC6929ABE7B88BE7B87BC7986BC7886BB7785BB7784BA76
                82B97481B97480B87380B8717EB76F7DB76D7CB76D7CB76C7AB66C7AB66D7BB5
                6C7AB56C7AB56C7AB66C7AB66C7AB76D7C9F4F60C4C4C4E5E5E5F8F8F8FAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7AC6939BBE7E89BC7884A75E
                70A75E70A75E70A75E70A75E70A75E70A75E70A75E70A75E70A75E70A75E70A7
                5E70A75E70A75E70A75E70A75E70A75E70A75E70A75E70B66B7AB66C7B9F5162
                C4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A
                7AC7949CBF7F8AAF6F7BEBE1E1FEFFFEFEFFFEFEFFFEFEFFFEFEFFFEFEFFFEFE
                FEFDFDFEFDFDFDFCFCFCFCFBFCFBFBFBFBFAFAFAF9F9F9F9F9F9F8F8F8E8D1D1
                A75E70B76D7BB66C7A9F5262C4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAAE6A7AC8959DC0808BAF6F7BFEFFFEFCFCFBFBFBFAFA
                FBFAFAFAFAF9FAF9F9F9F9F9F9F9F9F9F8F8F8F8F8F8F7F7F7F7F7F7F7F7F7F7
                F7F7F7F7F6F6F6F6F6F9F9F9A75E70B96F7DB66B7AA05363C4C4C4E5E5E5F8F8
                F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7AC9959DC1818CAF
                6F7BFEFFFEE1E1E1DFDFDFDDDDDDDBDCDBDADAD9D9D9D9D7D7D6D5D6D5D4D4D3
                D2D3D2D1D1D1D0D0D0CFCFCFCFCFCFCFCFCFCFCFCFF9FAF9A75E70BA707EB66B
                79A15466C4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAAE6A7AC9969EC1818DAF6F7BFEFFFEFCFCFCFCFCFBFBFBFBFBFBFAFBFAFA
                FAFAFAF9F9F9F9F9F9F9F9F9F9F9F8F8F8F8F8F8F8F7F7F7F7F7F7F7F7F7F7F7
                F6FBFBFBA75E70B97480B56B79A25666C4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7AC9989FC2828EAF6F7BFEFFFEE4E4E4
                E3E3E3E2E2E1E0E0E0DEDDDEDCDCDCDADADAD9D9D9D7D8D8D6D5D5D4D4D4D2D3
                D3D1D2D1D0D0D0CFCFCFCFCFCFFCFCFBA75E70BA7581B56B79A45968C4C4C4E5
                E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7AC9989F
                C2838EAF6F7BFEFFFEFCFCFCFCFCFCFCFCFCFCFCFBFBFBFBFBFBFAFAFAFAFAFA
                F9F9F9F9F9F9F9F9F9F8F8F9F8F8F8F8F8F7F7F7F7F7F7F7F7FCFCFCA75E70BA
                7783B66C7AA55969C4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAAE6A7ACA98A0C2838EAF6F7BFEFFFEE6E6E6E6E5E6E4E4E4E3E3
                E3E1E2E2E0E0E0DEDEDEDCDCDCDBDBDBD9D9D9D8D8D8D6D6D6D4D5D5D3D3D3D1
                D1D1D0D0D0FDFDFCA75E70BC7783B76D7BA55D6CC4C4C4E5E5E5F8F8F8FAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7ACB98A0C2838EAF6F7BFEFF
                FEFDFDFDFDFDFCFCFDFCFCFCFCFCFCFBFBFCFBFBFBFBFBFBFAFAFAFAFAFAF9F9
                F9F9F9F9F9F9F9F9F8F8F8F8F8F8F7F7F7FDFEFDA75E70BD7985B76F7DA65F6E
                C4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A
                7ACB9BA1C2838EAF6F7BFEFFFEE6E6E6E6E6E6E6E6E6E5E5E6E4E5E5E4E3E3E2
                E2E2E1E0E0DFDFDFDDDDDDDBDBDCDADAD9D9D8D9D7D6D7D5D5D5D3D4D3FEFEFE
                A75E70BE7A86B7707DA6606EC4C4C4E5E5E5F8F8F8FAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAAE6A7ACB9BA1C2838EAF6F7BFEFFFEFEFEFDFEFEFDFD
                FDFCFDFDFCFCFDFCFCFCFCFCFCFBFCFCFBFBFBFAFBFBFAFAFAF9F9F9F9F9F9F9
                F9F9F9F9F9F8F8F8F8FEFFFEA75E70B38088B8717FA7606EC4C4C4E5E5E5F8F8
                F8FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7ACB9BA1C2838EAF
                6F7BFEFFFEE6E6E6E6E6E6E6E6E6E6E6E6E6E6E6E6E6E6E5E5E5E4E4E3E2E3E3
                E1E1E1E0DFDFDDDDDEDCDBDCDADADAD9D9D9D7D8D7FEFFFEA75E70996E759E59
                67A9606EC5C5C5E5E5E5F9F9F9FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAAE6A7ACB9BA1C2838EAF6F7BFEFFFEFEFFFEFEFEFEFEFEFDFEFEFDFDFDFC
                FDFDFCFCFDFCFCFCFCFCFCFBFBFBFBFBFBFAFAFAFAFAFAF9FAFAF9F9F9F9F9F9
                F9FEFFFEA75E70B0757F9E5A68AA6471C5C5C5E6E6E6F9F9F9FAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7ACA969EC2838EAF6F7BFEFFFEFEFFFE
                FEFFFEFEFFFEFEFFFEFEFFFEFEFFFEFEFFFEFEFFFEFEFFFEFEFFFEFEFFFEFEFF
                FEFEFFFEFEFFFEFEFFFEFEFFFEFEFFFEA75E70C77F8BC77F8BAA6572D1D1D1EB
                EBEBFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAAE6A7AAE6A7A
                AE6A7AAE6A7AD9D6D6D9D6D6D9D6D6D9D6D6D9D6D6D9D6D6D9D6D6D9D6D6D9D6
                D6D9D6D6D9D6D6D9D6D6D9D6D6D9D6D6D9D6D6D9D6D6D9D6D6D9D6D6A75E70A7
                5E70A75E70A75E70ECECECF6F6F6FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA
                FAFAFAFAFAFAFAFAFAFA}
              Margin = 20
              ParentFont = False
            end
            object sbtReloadTray: TSpeedButton
              Left = 82
              Top = 9
              Width = 170
              Height = 36
              AllowAllUp = True
              GroupIndex = 1
              Caption = 'Load Data'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clBlack
              Font.Height = -19
              Font.Name = 'Arial'
              Font.Style = []
              Glyph.Data = {
                360C0000424D360C000000000000360000002800000020000000200000000100
                180000000000000C0000C40E0000C40E00000000000000000000FFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBFBFBF2F2F2F0F0F0F9F9
                F9FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFCFCFCEAEAEAD0D0D0D1D1D1EEEE
                EEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFCFCFCEBEBEBCDCDCD38996C008E4EE9E9
                E9FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFCFCFCEBEBEBCDCDCD3E986E00C686008B4BD8D8
                D8E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9EBEBEBF1F1F1F8
                F8F8FEFEFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFCFCFCEBEBEBCDCDCD3E986D00BF8200E3A6008848B7B7
                B7BCBCBCBCBCBCBCBCBCBCBCBCBCBCBCBCBCBCBCBCBCBDBDBDC1C1C1CCCCCCDB
                DBDBEAEAEAF7F7F7FEFEFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFBFBFBEBEBEBCDCDCD3E986D00BA8100D8A000D9A10083420087
                46008847008847008847008847008847008847008947008948078B4D3C986D83
                AA98C4C4C4D9D9D9EEEEEEFBFBFBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFF2F2F2D1D1D13E986D00B88000D4A000D19C00D19C00D9A000DB
                A200DBA200DBA200DBA200DBA200DBA200DBA200D3A200D5A300C18B00AA7000
                8E4D058B4D85AB9ACCCCCCE7E7E7FAFAFAFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFEDEDED3F9C7100B78100D09F00CC9B00CB9A00CB9A00CE9B00CE
                9B00CE9B00CE9B00CE9B00CE9B00CE9B00CF9B00CB9B00CC9C00CD9D00CF9F00
                CD9C00AC750089472E9666C8C8C8E8E8E8FCFCFCFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFF2F2F2008A482CD9B807CBA100C89B00C89B00C89B00C89900C8
                9900C89900C89900C89900C89900C89900C89900C89B00C89B00C89B00C99C00
                CA9E00CDA100C293008E4D139157CECECEEFEFEFFEFEFEFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFBFBFB4CAA7E00B28162DFC700C39A00C29900C29800C19800C1
                9900C19900C19900C19900C19900C19900C19900C39800C39800C39900C49A00
                C59B00C69C00C8A000C093008D4D3F9C71DADADAF7F7F7FFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFCFCFC51AB7F00AE7F60DCC600BE9794E5D74DE6D151E8
                D452E8D552E8D552E8D551E8D57FE1D37DE1D37CE4D172E1CC49D5B906C59E00
                C29900C39B00C49C00C6A000BE93008A478EB5A3EBEBEBFEFEFEFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFCFCFC51AB7F00AB7F5AD9C482E3D600823B0086
                42008743008743008743008743008743008A49169E6626B1826CD8C175E0CD27
                CBAD00BE9800C09A00C19C00C4A100A975068E4DDDDDDDF9F9F9FFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFCFCFC51AC7F00A97F7BE3DA008944E9E9
                E9FFFFFFFFFFFFFFFFFFFFFFFFFEFEFEC8E5D8CBE5D969B791008A484BC19D81
                E2D328C8AC00BC9800BE9B00C09D00C09D008E4DA5C1B4F1F1F1FFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFCFCFC51AC7F00AB87008C4AEEEE
                EEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFEFEDEEDE61D92574C
                C09F85E1D30EBFA200B99A00BB9C00BEA100A17043A074EBEBEBFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBFBFB4CAD80008E4DF9F9
                F9FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEAEEEC00
                8A4775D8C758D3C100B69800B79A00B99D00AF8C078D4CEBEBEBFBFBFBF2F2F2
                EAEAEAE9E9E9E9E9E9E9E9E9E9E9E9EDEDEDF7F7F7FEFEFEFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF9F9F961
                B18B2DB18A8FE4DE7BDDD47DDED47EDFD683E3DD008B48F2F2F2F2F2F2D4D4D4
                C0C0C0BCBCBCBCBCBCBCBCBCBCBCBCC7C7C7E3E3E3F9F9F9FFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFEFEC0
                E0D1008B49008944008843008843008944008A474BAC7FFBFBFBEBEBEB3B9D70
                008949008747008747008746008746008A49A0C0B1F1F1F1FFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF9F9F9F0F0F0F2F2F2FBFBFBFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBEBEB008949
                11ECAE03E5A400E3A000E29E96FBDF2FBE89509F79E0E0E0F9F9F9FEFEFEFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEEEEEED1D1D1D0D0D0EAEAEAFC
                FCFCFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1F1F1078B4D
                22D39A14E3A600DC9900DB9750EABC68E6BF008B49BEC2C0DFDFDFF1F1F1F9F9
                F9FCFCFCFEFEFEFFFFFFFFFFFFFFFFFFFFFFFFE9E9E9008E4E38996CCDCDCDEB
                EBEBFCFCFCFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF9F9F94CA77C
                20B67E35E9B400DB9700DB970CDD9F64EEC538C996168B52A6B5AECECECEDCDC
                DCE2E2E2E7E7E7E9E9E9E9E9E9E9E9E9E9E9E9D8D8D8008B4B00C6863E986ECD
                CDCDEBEBEBFCFCFCFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFEFEBDD8CC
                048F5053E9BF19DEA600D89700D8971ADEA54EEABE2EC691008C4A4C9A7594AE
                A293B0A3BBBBBBBCBCBCBCBCBCBCBCBCBCBCBCB7B7B700884800E3A600BF823E
                986DCDCDCDEBEBEBFCFCFCFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7F7F7
                078B4D36BC8D62E8C604D49D00D19800D19813D7A33AE1B632D8AA11B37A0A9D
                610C8E4F00884600884700884700884700884700874600834200D9A100D8A000
                BA813E986DCDCDCDEBEBEBFBFBFBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFEFE
                ADD4C2008A4965DBB966E5C600CD9A00CB9700CC9801CE9B18D5A624D8AD27DB
                AF29E0B100D8A100D9A200D9A200D9A200D9A200D9A200D7A000D19C00D19C00
                D4A000B8803E986DD1D1D1F2F2F2FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FCFCFC50AB811195598AE3CE75E4CD10CCA200C59800C69900C69A00C79A00C7
                9B00CB9A00CB9A00CB9B00CB9B00CB9B00CB9B00CB9B00CB9B00CB9A00CB9A00
                CC9B00D09F00B7813F9C71EDEDEDFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFAFAFA18945912955978DAC18CE7D843D4B800C19B00C09700C09800C0
                9900C39900C39900C39900C39900C39900C39900C39900C29A00C89A00C89B00
                C89B07CBA12CD9B8008A48F2F2F2FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFBFBFB3FA676008C4A44B8908DE2D58DE4D858D6C139CCB31BC4
                A71EC5A800C4A301C4A301C4A301C4A301C4A301C4A300C4A300C29800C29900
                C39A62DFC700B2814CAA7EFBFBFBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFEFEFEB4DAC9078C4C008D4C3AB1876DCFB898E6E198E5
                E09BE7E04EE3D551E3D552E3D552E3D552E3D551E2D54DE1D293E6D700BF9760
                DCC600AE7F51AB7FFCFCFCFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFEFEB3DAC853AE82098C4C0088430087
                4200874200874300874300874300874300874300874200823B82E3D65ADAC400
                AB7F51AB7FFCFCFCFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE9E9E90089447BE3DA00A97F51
                AC7FFCFCFCFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEEEEEE008C4A00AB8751AC7FFC
                FCFCFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF9F9F9008E4D4CAD80FBFBFBFF
                FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}
              Margin = 20
              ParentFont = False
            end
          end
        end
      end
      object tsMaintTeach: TTabSheet
        Caption = 'Teach'
      end
      object tsMaintMotor: TTabSheet
        Caption = 'Motor Test'
      end
      object tsMaintIO: TTabSheet
        Caption = 'IO Monitor'
      end
      object tsMaintSECS: TTabSheet
        Caption = 'SECS/GEM'
      end
      object tsMaintCOM: TTabSheet
        Caption = 'COM Port'
      end
      object tsMaintMCUDisplay: TTabSheet
        Caption = 'Bin Display'
        object pnlMCUSetup: TPanel
          Left = 20
          Top = 20
          Width = 780
          Height = 210
          BevelOuter = bvLowered
          Color = 12761254
          TabOrder = 0
          object lblMCUSetupTitle: TLabel
            Left = 16
            Top = 14
            Width = 160
            Height = 20
            AutoSize = False
            Caption = 'Bin Display TCP Setup'
          end
          object lblMCUIPCap: TLabel
            Left = 16
            Top = 82
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'COM Port'
          end
          object lblMCUPortCap: TLabel
            Left = 16
            Top = 118
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'Baud'
          end
          object lblMCUReconnectCap: TLabel
            Left = 310
            Top = 118
            Width = 120
            Height = 20
            AutoSize = False
            Caption = 'Delay (s)'
          end
          object chkMCUEnabled: TCheckBox
            Left = 16
            Top = 46
            Width = 240
            Height = 24
            Caption = 'Bin Display Installed'
            TabOrder = 0
          end
          object edMCUIP: TEdit
            Left = 108
            Top = 78
            Width = 160
            Height = 24
            TabOrder = 1
            Text = 'COM5'
          end
          object edMCUPort: TComboBox
            Left = 108
            Top = 114
            Width = 95
            Height = 24
            ItemHeight = 16
            TabOrder = 2
            Text = '9600'
            Items.Strings = (
              '9600'
              '19200'
              '38400'
              '57600'
              '115200')
          end
          object edMCUReconnect: TEdit
            Left = 420
            Top = 114
            Width = 80
            Height = 24
            TabOrder = 4
            Text = '5'
          end
          object btnMCUSave: TButton
            Left = 560
            Top = 76
            Width = 90
            Height = 28
            Caption = 'Save'
            TabOrder = 5
            OnClick = btnMCUSaveClick
          end
          object btnMCUReload: TButton
            Left = 660
            Top = 76
            Width = 90
            Height = 28
            Caption = 'Reload'
            TabOrder = 6
            OnClick = btnMCUReloadClick
          end
          object btnMCURefresh: TButton
            Left = 560
            Top = 114
            Width = 190
            Height = 28
            Caption = 'Refresh Status'
            TabOrder = 7
            OnClick = btnMCURefreshClick
          end
          object pnlMCUStatus: TPanel
            Left = 16
            Top = 152
            Width = 734
            Height = 42
            BevelOuter = bvLowered
            TabOrder = 8
            object lblMCUStatusEnabled: TLabel
              Left = 8
              Top = 12
              Width = 130
              Height = 20
              AutoSize = False
              Caption = 'Enabled: -'
            end
            object lblMCUStatusConnected: TLabel
              Left = 146
              Top = 12
              Width = 150
              Height = 20
              AutoSize = False
              Caption = 'Connected: -'
            end
            object lblMCUStatusQueue: TLabel
              Left = 304
              Top = 12
              Width = 110
              Height = 20
              AutoSize = False
              Caption = 'Queue: 0'
            end
            object lblMCUStatusError: TLabel
              Left = 420
              Top = 12
              Width = 300
              Height = 20
              AutoSize = False
              Caption = 'Last Error: '
            end
          end
        end
        object pnlMCUTest: TPanel
          Left = 20
          Top = 246
          Width = 780
          Height = 210
          BevelOuter = bvLowered
          Color = 12761254
          TabOrder = 1
          object lblMCUTestTitle: TLabel
            Left = 16
            Top = 14
            Width = 180
            Height = 20
            AutoSize = False
            Caption = 'Manual Test'
          end
          object lblMCUAddressCap: TLabel
            Left = 16
            Top = 52
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'Address'
          end
          object lblMCUTextCap: TLabel
            Left = 16
            Top = 88
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'Text'
          end
          object lblMCUColorCap: TLabel
            Left = 16
            Top = 124
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'Color'
          end
          object lblMCULightValueCap: TLabel
            Left = 260
            Top = 88
            Width = 100
            Height = 20
            AutoSize = False
            Caption = 'Color Code'
          end
          object edMCUAddress: TEdit
            Left = 108
            Top = 48
            Width = 80
            Height = 24
            TabOrder = 0
            Text = '0'
          end
          object edMCUText: TEdit
            Left = 108
            Top = 84
            Width = 80
            Height = 24
            TabOrder = 1
            Text = '9'
          end
          object cbbMCUColor: TComboBox
            Left = 108
            Top = 120
            Width = 120
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            ItemIndex = 0
            TabOrder = 2
            Text = 'GREEN'
            Items.Strings = (
              'GREEN'
              'RED')
          end
          object chkMCUCodeSymbol: TCheckBox
            Left = 260
            Top = 48
            Width = 130
            Height = 24
            Caption = 'Symbol Code'
            TabOrder = 3
          end
          object edMCULightValue: TEdit
            Left = 370
            Top = 84
            Width = 80
            Height = 24
            TabOrder = 4
            Text = '0'
          end
          object btnMCUSendDisplay: TButton
            Left = 520
            Top = 48
            Width = 150
            Height = 28
            Caption = 'Send Display'
            TabOrder = 5
            OnClick = btnMCUSendDisplayClick
          end
          object btnMCUSendCode: TButton
            Left = 520
            Top = 86
            Width = 150
            Height = 28
            Caption = 'Send Code'
            TabOrder = 6
            OnClick = btnMCUSendCodeClick
          end
          object btnMCUSendLight: TButton
            Left = 520
            Top = 124
            Width = 150
            Height = 28
            Caption = 'Send Light'
            TabOrder = 7
            OnClick = btnMCUSendLightClick
          end
        end
        object memMCULog: TMemo
          Left = 20
          Top = 474
          Width = 780
          Height = 300
          ReadOnly = True
          ScrollBars = ssVertical
          TabOrder = 2
        end
      end
      object tsMaintTopCcd: TTabSheet
        Caption = 'Top CCD'
        object pnlTopCcdSetup: TPanel
          Left = 20
          Top = 20
          Width = 780
          Height = 170
          BevelOuter = bvLowered
          Color = 12761254
          TabOrder = 0
          object lblTopCcdSetupTitle: TLabel
            Left = 16
            Top = 14
            Width = 200
            Height = 20
            AutoSize = False
            Caption = 'Top CCD TCP Setup'
          end
          object lblTopCcdIPCap: TLabel
            Left = 16
            Top = 50
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'IP'
          end
          object lblTopCcdPortCap: TLabel
            Left = 16
            Top = 86
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'Port'
          end
          object edTopCcdIP: TEdit
            Left = 108
            Top = 46
            Width = 160
            Height = 24
            TabOrder = 0
            Text = '172.16.8.89'
          end
          object edTopCcdPort: TEdit
            Left = 108
            Top = 82
            Width = 80
            Height = 24
            TabOrder = 1
            Text = '5001'
          end
          object chkTopCcdBottomReserved: TCheckBox
            Left = 310
            Top = 48
            Width = 240
            Height = 24
            Caption = 'Bottom CCD (reserved)'
            Enabled = False
            TabOrder = 2
          end
          object btnTopCcdSave: TButton
            Left = 560
            Top = 44
            Width = 90
            Height = 28
            Caption = 'Save'
            TabOrder = 3
            OnClick = btnTopCcdSaveClick
          end
          object btnTopCcdReload: TButton
            Left = 660
            Top = 44
            Width = 90
            Height = 28
            Caption = 'Reload'
            TabOrder = 4
            OnClick = btnTopCcdReloadClick
          end
          object btnTopCcdConnect: TButton
            Left = 560
            Top = 82
            Width = 90
            Height = 28
            Caption = 'Connect'
            TabOrder = 5
            OnClick = btnTopCcdConnectClick
          end
          object btnTopCcdDisconnect: TButton
            Left = 660
            Top = 82
            Width = 90
            Height = 28
            Caption = 'Disconnect'
            TabOrder = 6
            OnClick = btnTopCcdDisconnectClick
          end
          object pnlTopCcdStatus: TPanel
            Left = 16
            Top = 118
            Width = 734
            Height = 42
            BevelOuter = bvLowered
            TabOrder = 7
            object lblTopCcdStatusConn: TLabel
              Left = 8
              Top = 12
              Width = 180
              Height = 20
              AutoSize = False
              Caption = 'Connected: -'
            end
            object lblTopCcdStatusError: TLabel
              Left = 200
              Top = 12
              Width = 520
              Height = 20
              AutoSize = False
              Caption = 'Last Error: '
            end
          end
        end
        object pnlTopCcdTest: TPanel
          Left = 20
          Top = 206
          Width = 780
          Height = 110
          BevelOuter = bvLowered
          Color = 12761254
          TabOrder = 1
          object lblTopCcdTestTitle: TLabel
            Left = 16
            Top = 14
            Width = 200
            Height = 20
            AutoSize = False
            Caption = 'Manual Shot (LON)'
          end
          object lblTopCcdResultCap: TLabel
            Left = 190
            Top = 52
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'Result'
          end
          object btnTopCcdShot: TButton
            Left = 16
            Top = 48
            Width = 150
            Height = 28
            Caption = 'Trigger Shot'
            TabOrder = 0
            OnClick = btnTopCcdShotClick
          end
          object edTopCcdResult: TEdit
            Left = 280
            Top = 48
            Width = 400
            Height = 24
            ReadOnly = True
            TabOrder = 1
          end
        end
        object memTopCcdLog: TMemo
          Left = 20
          Top = 332
          Width = 780
          Height = 300
          ReadOnly = True
          ScrollBars = ssVertical
          TabOrder = 2
        end
      end
      object tsMaintColorCcd: TTabSheet
        Caption = 'Color CCD'
        object pnlColorCcdSetup: TPanel
          Left = 20
          Top = 20
          Width = 780
          Height = 170
          BevelOuter = bvLowered
          Color = 12761254
          TabOrder = 0
          object lblColorCcdSetupTitle: TLabel
            Left = 16
            Top = 14
            Width = 200
            Height = 20
            AutoSize = False
            Caption = 'Color CCD TCP Setup'
          end
          object lblColorCcdIPCap: TLabel
            Left = 16
            Top = 50
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'IP'
          end
          object lblColorCcdPortCap: TLabel
            Left = 16
            Top = 86
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'Port'
          end
          object edColorCcdIP: TEdit
            Left = 108
            Top = 46
            Width = 160
            Height = 24
            TabOrder = 0
            Text = '172.16.8.100'
          end
          object edColorCcdPort: TEdit
            Left = 108
            Top = 82
            Width = 80
            Height = 24
            TabOrder = 1
            Text = '5000'
          end
          object chkColorCcdEnable: TCheckBox
            Left = 310
            Top = 48
            Width = 240
            Height = 24
            Caption = 'Enable Color CCD'
            TabOrder = 2
            OnClick = chkColorCcdEnableClick
          end
          object btnColorCcdSave: TButton
            Left = 560
            Top = 44
            Width = 90
            Height = 28
            Caption = 'Save'
            TabOrder = 3
            OnClick = btnColorCcdSaveClick
          end
          object btnColorCcdReload: TButton
            Left = 660
            Top = 44
            Width = 90
            Height = 28
            Caption = 'Reload'
            TabOrder = 4
            OnClick = btnColorCcdReloadClick
          end
          object btnColorCcdConnect: TButton
            Left = 560
            Top = 82
            Width = 90
            Height = 28
            Caption = 'Connect'
            TabOrder = 5
            OnClick = btnColorCcdConnectClick
          end
          object btnColorCcdDisconnect: TButton
            Left = 660
            Top = 82
            Width = 90
            Height = 28
            Caption = 'Disconnect'
            TabOrder = 6
            OnClick = btnColorCcdDisconnectClick
          end
          object pnlColorCcdStatus: TPanel
            Left = 16
            Top = 118
            Width = 734
            Height = 42
            BevelOuter = bvLowered
            TabOrder = 7
            object lblColorCcdStatusConn: TLabel
              Left = 8
              Top = 12
              Width = 180
              Height = 20
              AutoSize = False
              Caption = 'Connected: -'
            end
            object lblColorCcdStatusError: TLabel
              Left = 200
              Top = 12
              Width = 520
              Height = 20
              AutoSize = False
              Caption = 'Last Error: '
            end
          end
        end
        object pnlColorCcdTest: TPanel
          Left = 20
          Top = 206
          Width = 780
          Height = 110
          BevelOuter = bvLowered
          Color = 12761254
          TabOrder = 1
          object lblColorCcdTestTitle: TLabel
            Left = 16
            Top = 14
            Width = 200
            Height = 20
            AutoSize = False
            Caption = 'Manual Shot (LON)'
          end
          object lblColorCcdResultCap: TLabel
            Left = 190
            Top = 52
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'Result'
          end
          object btnColorCcdShot: TButton
            Left = 16
            Top = 48
            Width = 150
            Height = 28
            Caption = 'Trigger Shot'
            TabOrder = 0
            OnClick = btnColorCcdShotClick
          end
          object edColorCcdResult: TEdit
            Left = 280
            Top = 48
            Width = 400
            Height = 24
            ReadOnly = True
            TabOrder = 1
          end
        end
        object memColorCcdLog: TMemo
          Left = 20
          Top = 332
          Width = 780
          Height = 300
          ReadOnly = True
          ScrollBars = ssVertical
          TabOrder = 2
        end
      end
      object tsMaintLotApi: TTabSheet
        Caption = 'Lot WebAPI'
        object pnlLotApiSetup: TPanel
          Left = 20
          Top = 20
          Width = 780
          Height = 130
          BevelOuter = bvLowered
          Color = 12761254
          TabOrder = 0
          object Label1: TLabel
            Left = 16
            Top = 16
            Width = 80
            Height = 16
            Caption = 'WebAPI Path'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object lblLotApiUrl: TLabel
            Left = 16
            Top = 46
            Width = 620
            Height = 20
            AutoSize = False
            Caption = 'URL: '
          end
          object lblLotApiSaveHint: TLabel
            Left = 220
            Top = 80
            Width = 540
            Height = 20
            AutoSize = False
            Caption = 
              'Save/Reload persist the URL to system\General.ini [LotWebApi] Ba' +
              'seUrl.'
          end
          object edWebapiPath: TEdit
            Left = 110
            Top = 12
            Width = 560
            Height = 21
            TabOrder = 0
          end
          object btnLotApiSave: TButton
            Left = 16
            Top = 76
            Width = 90
            Height = 28
            Caption = 'Save'
            TabOrder = 1
            OnClick = btnLotApiSaveClick
          end
          object btnLotApiReload: TButton
            Left = 116
            Top = 76
            Width = 90
            Height = 28
            Caption = 'Reload'
            TabOrder = 2
            OnClick = btnLotApiReloadClick
          end
          object chkLotApiUsePull: TCheckBox
            Left = 16
            Top = 106
            Width = 540
            Height = 24
            Caption = 
              'Auto-pull on Lot Start / SECS LOTSTART (off until customer API r' +
              'eady)'
            TabOrder = 3
          end
        end
        object pnlLotApiStatus: TPanel
          Left = 20
          Top = 160
          Width = 780
          Height = 42
          BevelOuter = bvLowered
          TabOrder = 1
          object lblLotApiStatus: TLabel
            Left = 8
            Top = 12
            Width = 260
            Height = 20
            AutoSize = False
            Caption = 'State: idle'
          end
          object lblLotApiError: TLabel
            Left = 280
            Top = 12
            Width = 490
            Height = 20
            AutoSize = False
            Caption = 'Last Error: '
          end
        end
        object pnlLotApiTest: TPanel
          Left = 20
          Top = 212
          Width = 780
          Height = 200
          BevelOuter = bvLowered
          Color = 12761254
          TabOrder = 2
          object lblLotApiTestTitle: TLabel
            Left = 16
            Top = 14
            Width = 200
            Height = 20
            AutoSize = False
            Caption = 'Manual Fetch (test)'
          end
          object lblLotApiTestLotCap: TLabel
            Left = 16
            Top = 50
            Width = 80
            Height = 20
            AutoSize = False
            Caption = 'Lot ID'
          end
          object edLotApiTestLot: TEdit
            Left = 108
            Top = 46
            Width = 360
            Height = 24
            TabOrder = 0
            Text = 'A5921.RCS.TEST99'
          end
          object btnLotApiFetch: TButton
            Left = 480
            Top = 44
            Width = 120
            Height = 28
            Caption = 'Fetch'
            TabOrder = 1
            OnClick = btnLotApiFetchClick
          end
          object memLotApiResult: TMemo
            Left = 16
            Top = 82
            Width = 748
            Height = 104
            ReadOnly = True
            ScrollBars = ssBoth
            TabOrder = 2
            WordWrap = False
          end
        end
        object memLotApiLog: TMemo
          Left = 20
          Top = 422
          Width = 780
          Height = 210
          ReadOnly = True
          ScrollBars = ssVertical
          TabOrder = 3
        end
      end
    end
  end
  object tmrTowerLightBlink: TTimer
    Enabled = False
    Interval = 300
    OnTimer = tmrTowerLightBlinkTimer
    Left = 8
    Top = 8
  end
end
