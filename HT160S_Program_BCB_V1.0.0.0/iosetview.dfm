object fiosetview: Tfiosetview
  Left = 402
  Top = 65
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'IO check and verify'
  ClientHeight = 838
  ClientWidth = 1057
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
  object pn_IOSetViewMenu: TPanel
    Left = 850
    Top = 1
    Width = 205
    Height = 786
    BevelOuter = bvLowered
    Color = 12761254
    TabOrder = 0
    object sbIOExit: TSpeedButton
      Left = 8
      Top = 721
      Width = 190
      Height = 55
      AllowAllUp = True
      GroupIndex = 2
      Caption = 'Exit'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      Glyph.Data = {
        66100000424D6610000000000000360000002800000025000000250000000100
        18000000000030100000232E0000232E00000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00F5F5F5D6D7D8CACACBBDBEBFB0B1B3A5A6A8A4A5A7A2
        A4A6A1A3A5A0A1A39D9EA09A9B9E999B9D9A9B9D999A9C999A9B9FA0A2A1A3A5
        A4A5A7A7A8AAACADAFB6B6B8BFBFC1C8C9CACCCDCED2D2D3D5D6D7D8D9DAD7D8
        D9D7D8D9D8D9DADFDFE0E8E8E9EEEEEFF4F4F4F8F8F8FCFCFC00FFFFFFF0F1F1
        D9DADAC8C9CAB8B9BBA8A9AB97999B8F91938D8F9191939598999A999A9B9798
        999697989697989596979092938D8F918D8F918D8F918D8F918D8F919394979F
        A0A2ABACADB3B4B5BABBBCC3C4C5CBCCCDD1D2D3D1D2D3D2D2D3D9DADAE1E2E2
        EAEAEAF1F1F1F6F6F700FFFFFFFFFFFFFFFFFFE0E1E1B5B5B6B1B2B3ADAFB0A5
        A6A79B9C9D98999A98999A98999A98999A98999A98999A979899A5A6A7B6B7B8
        B6B7B8B6B7B8B6B7B8BABABBE3E3E3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        D8D8D8686A6C9FA0A19A9B9C9A9B9C9A9B9C9A9B9C9A9B9C9A9B9C9A9B9C9A9B
        9C9A9B9C9A9B9C98999A8384867273757273756F707272737578797B797B7DDD
        DEDFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA6A7A99D9EA09C9D9E9C9D9E9C9D9E9C
        9D9E9C9D9E9C9D9E9C9D9E9C9D9E9C9D9E9C9D9E9C9D9E999A9B6B6C6F3F4145
        3F41453F41453E40459B9B9D9A9B9CBFC0C0FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        A0A2A3A0A1A29FA0A19FA0A19FA0A19FA0A19FA0A19FA0A19FA0A19FA0A19FA0
        A19FA0A19FA0A19B9C9D949496939395939395939395939395515357818385BC
        BDBEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA2A3A4A1A2A3A1A2A3A1A2A3A1A2A3A1
        A2A3A1A2A3A1A2A3A1A2A3A1A2A3A1A2A3A1A2A3A1A2A39D9E9F9191938D8D8F
        8D8D8F8D8D8F8D8D8F5E60636E6F72BEBFC0FFFFFFFFFFFFFFFFFFCACCD6FFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        A2A4A5A3A4A5A4A5A5A4A5A5A4A5A5A4A5A5A4A5A5A4A5A5A4A5A5A4A5A5A4A5
        A5A4A5A5A4A5A59FA0A08E8F9187888A87888A87888A87888A5B5D60707274C0
        C1C1FFFFFFFFFFFF8E95AD405382CFD1DAFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA4A5A6A5A6A7A6A7A8A6A7A8A6A7A8A6
        A7A8A6A7A8A6A7A8A6A7A8A6A7A8A6A7A8A6A7A8A6A7A8A0A1A28C8D8E838385
        838385838385838385585A5D727476C2C3C3FFFFFFFFFFFF808BA82B41853646
        74FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        A5A6A7A6A8A8A9AAAAA9AAAAA9AAAAA9AAAAA9AAAAA9AAAAA9AAAAA9AAAAA9AA
        AAA9AAAAA9AAAAA2A3A48A8B8C7D7E807D7E807D7E807D7E8055575A737577C3
        C4C5FFFFFFFFFFFF808BA82B41852B469241517EFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA7A8A9A8A9AAABACADABACADABACADAB
        ACADABACADABACADABACADABACADABACADABACADABACADA4A5A687888A77787A
        77787A77787A77787A515356757779C5C6C6FFFFFFFFFFFF828CA929468C284D
        9F284A9755658EFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        A7A9AAA9AAABADAEAFADAEAFADAEAFADAEAFADAEAFADAEAFADAEAFADAEAFADAE
        AFADAEAFADAEAFA5A6A78586887172757172756F70746F70744D4E53747579C3
        C3C5FAFAFBFAFAFB828CA9274A932653A92653A9284E98707EA2FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA1A2A3A8A9ABAFB0B1AFB0B1AFB0B1AF
        B0B1AFB0B1AFB0B1AFB0B1AFB0B1AFB0B1AFB0B1AFB0B1A7A8A98485876D6E71
        60728A2E406F2D40702B3E6D2E4070334676374A79374A79354878254F9C2359
        B42359B42359B42B5197909CB7FFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        A2A3A4A9AAACB1B2B3B1B2B3B1B2B3B1B2B3B1B2B3B1B2B3B1B2B3B1B2B3B1B2
        B3B1B2B3B1B2B3A9AAAB81828465666A324572215FBE215FBE215FBE215FBE21
        5FBE215FBE215FBE215FBE215FBE215FBE215FBE215FBE215FBE305394B1B9CC
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA2A4A5ABACADB4B5B6B4B5B6B4B5B6B4
        B5B6B4B5B6B4B5B6B4B5B6B4B5B6B0B1B2838487B4B5B6ABACAD7F80825F6165
        3044721E65C81E65C81E65C81E65C81E65C81E65C81E65C81E65C81E65C81E65
        C81E65C81E65C81E65C81E65C835538CD6D9E3FFFFFFFFFFFF00FFFFFFFFFFFF
        A3A5A7ADAEAEB7B8B8B7B8B8B7B8B8B7B8B8B7B8B8B7B8B8B7B8B8B7B8B8C6C7
        C7DDDEDEB7B8B8ADAEAF7E7F815B5D612E43721C6CD31C6CD31C6CD31C6CD31C
        6CD31C6CD31C6CD31C6CD31C6CD31C6CD31C6CD31C6CD31C6CD31C6CD31C6CD3
        475B88F0F0F3FFFFFF00FFFFFFFFFFFFA5A6A7AEB0B0B9BABAB9BABAB9BABAB9
        BABAB9BABAB9BABAB9BABAB9BABA7D7F81171A1FB9BABAAFB0B07D7E8056575C
        334673478EE5478EE5478EE5478EE5478EE5478EE5478EE5478EE5478EE5478E
        E5478EE5478EE5478EE5478EE5478EE5496191E3E4EAFFFFFF00FFFFFFFFFFFF
        A6A7A8B0B1B2BBBCBCBBBCBCBBBCBCBBBCBCBBBCBCBBBCBCBBBCBCBBBCBCBBBC
        BCBBBCBCBBBCBCB1B2B27B7C7E5052562C41701778E71778E71778E71778E717
        78E71778E71778E71778E71778E71778E71778E71778E71778E71778E72758A1
        B1BDD2FFFFFFFFFFFF00FFFFFFFFFFFFA7A8A9B1B3B3BEBFBFC8C9C9D8D8D8C3
        C4C4BEBFBFBEBFBFBEBFBFBEBFBFBEBFBFBEBFBFBEBFBFB4B5B57A7B7D4C4D52
        2B41701778E71778E71778E71778E71778E71778E71778E71778E71778E71778
        E71778E71778E71778E71D71D5607095FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        A8A9AAB3B4B4C4C5C5E3E3E3E4E5E5DCDCDCC0C1C1C0C1C1C0C1C1C0C1C1C0C1
        C1C0C1C1C0C1C1B6B7B7787A7C46484C2E3D652258A3205AA82059A7225BA924
        5DAC255EAD255EAD245DAC196FD41778E71778E71778E71775E14B5C85FFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA9AAABB4B5B6D6D7D8E1E2E2E1E2E2E1
        E2E2CACBCCC2C3C4C2C3C4C2C3C4C2C3C4C2C3C4C2C3C4B8B9BA77787A414347
        414347393E4E393E4E2F34446469789A9EACB5B8C6B5B8C66B76961B68C71778
        E71778E71778E73F517DFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        AAABACBABBBCDEDEDFDEDEDFDEDEDFDEDEDFD7D8D8C4C5C6C4C5C6C4C5C6C4C5
        C6C4C5C6C4C5C6B9BBBC7677793B3D413B3D413B3D413B3D412E3035848688D6
        D7D8FFFFFFFFFFFF7B87A51B68C71778E71778E73D5586F2F3F5FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA1A3A5BEBFBFDCDCDCDCDCDCDCDCDCDC
        DCDCDCDCDCCDCECEC7C8C8C7C8C8C7C8C8C7C8C8C7C8C8BCBDBE75767837393D
        37393D37393D37393D2C2E3285878AD8D8D9FFFFFFFFFFFF7B87A51B68C71778
        E73A5C95D8DBE5FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        A3A4A5C0C1C1DADADADADADADADADADADADADADADAD8D8D8CBCCCCCACBCBCACB
        CBCACBCBCACBCBBFC0C0747578313338313338313338313338282A2F87888ADA
        DADAFFFFFFFFFFFF7B87A51B68C73462A4B9C0D1FFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA4A5A6BFBFC0D7D8D8D7D8D8D7D8D8D7
        D8D8D7D8D8D7D8D8D4D5D5CDCECECCCDCDCCCDCDCCCDCDC1C2C27375772B2E32
        2B2E322B2E322B2E3224272C898A8CDCDCDCFFFFFFFFFFFF7F89A52C4274B4C0
        D4FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        A5A6A7BDBEBFD5D6D6D5D6D6D5D6D6D5D6D6D5D6D6D5D6D6D5D6D6D4D5D5CFD0
        D0CECFCFCECFCFC4C5C573747627292E27292E27292E27292E2224298A8C8DDD
        DEDEFFFFFFFFFFFFD2D4DC6A7290E5E6EBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA5A7A8BDBEBFD4D5D5D4D5D5D4D5D5D4
        D5D5D4D5D5D4D5D5D4D5D5D4D5D5D3D4D4D1D2D2D0D1D1C6C7C7727375212428
        2124282124282124281E21268B8D8FDEDFDFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        A6A7A9BCBDBED3D4D4D3D4D4D3D4D4D3D4D4D3D4D4D3D4D4D3D4D4D3D4D4D3D4
        D4D3D4D4D3D4D4C9CACA7173751B1E231B1E231B1E231B1E231B1E238C8E90E0
        E0E0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFA7A8AABBBCBDD5D6D6D5D6D6D5D6D6D5
        D6D6D5D6D6D5D6D6D5D6D6D5D6D6D5D6D6D5D6D6D5D6D6BABBBB66686B171A1F
        171A1F171A1F171A1F25282D8D8F91E2E2E2FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        A8A9ABC1C2C3D7D8D8D7D8D8D7D8D8D7D8D8D7D8D8D7D8D8D6D7D7D0D1D1C8C9
        C9BFC0C0B0B1B27C7D7F383A3E27292E27292E27292E2D3034ADAFB0D5D6D6E3
        E4E4FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFC4C6C7ADAFB0DCDDDDDADBDBD6D7D7D0
        D1D1C7C8C8BCBDBDB2B3B3ADAEAFABACADAAABACABACADABACADABACADABACAD
        ABACADABACADABACADD7D8D8D7D8D8EAEBEBFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FDFDFDADAEAF888A8C9B9D9EB1B2B4C7C8C9DDDEDEDDDEDEDDDEDEDDDEDEDDDE
        DEDDDEDEDDDEDEDDDEDEDDDEDEDDDEDEDDDEDEDDDEDEDDDEDEDEDFDFE7E7E7FD
        FDFDFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00}
      Margin = 10
      ParentFont = False
      Spacing = 20
      OnClick = sbIOExitClick
    end
  end
  object PageIO: TPageControl
    Left = 0
    Top = 40
    Width = 849
    Height = 729
    ActivePage = ts_IOTapeLoadUnload
    Style = tsButtons
    TabIndex = 1
    TabOrder = 2
    object ts_IOLoader: TTabSheet
      Caption = 'Loader'
      object PageControl1: TPageControl
        Left = 0
        Top = 0
        Width = 841
        Height = 698
        ActivePage = TabSheet1
        Align = alClient
        TabIndex = 0
        TabOrder = 0
        object TabSheet1: TTabSheet
          Caption = 'Loader'
          ImageIndex = 3
          object Panel11: TPanel
            Left = 10
            Top = 66
            Width = 820
            Height = 127
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Color = 12761254
            TabOrder = 0
            object mlC_Empty_FrontRiseTray_1_On: TMyLed
              Left = 43
              Top = 96
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Empty_FrontRiseTray_1_On'
            end
            object mlC_Empty_FrontRiseTray_2_On: TMyLed
              Left = 43
              Top = 77
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Empty_FrontRiseTray_2_On'
            end
            object mlC_Empty_PushTray_On: TMyLed
              Left = 175
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Empty_PushTray_On'
            end
            object mlC_Empty_PushTray_Off: TMyLed
              Left = 175
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Empty_PushTray_Off'
            end
            object mlC_Empty_LeanOnTray_On: TMyLed
              Left = 255
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Empty_LeanOnTray_On'
            end
            object mlC_Empty_LeanOnTray_Off: TMyLed
              Left = 255
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Empty_LeanOnTray_Off'
            end
            object mlSnEmpty_InputHasTray: TMyLed
              Left = 6
              Top = 77
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnEmpty_InputHasTray'
            end
            object mlSnEmpty_InputFullTray: TMyLed
              Left = 6
              Top = 24
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnEmpty_InputFullTray'
            end
            object mlSnEmpty_OutputBottomHasTray: TMyLed
              Left = 780
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnEmpty_OutputBottomHasTray'
            end
            object mlSnEmpty_InputEnd: TMyLed
              Left = 6
              Top = 61
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnEmpty_InputEnd'
            end
            object mlC_Empty_FrontRiseTray_1_Off: TMyLed
              Left = 43
              Top = 108
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Empty_FrontRiseTray_1_Off'
            end
            object bpC_Empty_LeanOnTray: TBtnPanel
              Left = 277
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 0
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Empty_LeanOnTray'
              Style = tsButtons
            end
            object bpC_Empty_FrontRiseTray_1: TBtnPanel
              Left = 65
              Top = 96
              Width = 45
              Height = 27
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 1
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Empty_FrontRiseTray_1'
              Style = tsButtons
            end
            object bpC_Empty_PushTray: TBtnPanel
              Left = 197
              Top = 80
              Width = 45
              Height = 30
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 2
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Empty_PushTray'
              Style = tsButtons
            end
            object Panel97: TPanel
              Left = 32
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 3
            end
            object Panel4: TPanel
              Left = 32
              Top = 64
              Width = 745
              Height = 9
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 4
            end
            object Panel6: TPanel
              Left = 4
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Front'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 5
            end
            object Panel8: TPanel
              Left = 767
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Rear'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 6
            end
            object Panel10: TPanel
              Left = 768
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 9
            end
            object bpC_Empty_FrontRiseTray_2: TBtnPanel
              Left = 66
              Top = 76
              Width = 45
              Height = 17
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 7
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Empty_FrontRiseTray_2'
              Style = tsButtons
            end
            object bpC_Empty_FrontSeparateTray_1: TBtnPanel
              Left = 65
              Top = 45
              Width = 45
              Height = 18
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 8
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Empty_FrontSeparateTray_1'
              Style = tsButtons
            end
          end
          object Panel7: TPanel
            Left = 10
            Top = 42
            Width = 820
            Height = 25
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Empty'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -21
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 1
          end
          object Panel5: TPanel
            Left = 3
            Top = 3
            Width = 454
            Height = 30
            BevelInner = bvLowered
            Color = 9534289
            TabOrder = 2
            object Panel9: TPanel
              Left = 5
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'M :'#39340#36948
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 0
            end
            object Panel12: TPanel
              Left = 90
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = '<-> : '#21069#24460#27773#32568
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 1
            end
            object Panel15: TPanel
              Left = 176
              Top = 5
              Width = 100
              Height = 20
              BevelInner = bvLowered
              Caption = 'v  ^ : '#19978#19979#27773#32568
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 2
            end
            object Panel13: TPanel
              Left = 281
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'V : '#30495#31354
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 3
            end
            object Panel14: TPanel
              Left = 366
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'D : '#30772#22750
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 4
            end
          end
          object Panel52: TPanel
            Left = 10
            Top = 194
            Width = 820
            Height = 25
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Loader'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -21
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 3
          end
          object Panel53: TPanel
            Left = 10
            Top = 218
            Width = 820
            Height = 159
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Color = 12761254
            TabOrder = 4
            object mlSnLoader_InputHasTray: TMyLed
              Left = 6
              Top = 78
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnLoader_InputHasTray'
            end
            object mlSnLoader_OutputBottomHasTray: TMyLed
              Left = 780
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnLoader_OutputBottomHasTray'
            end
            object mlSnLoader_TrayPos1: TMyLed
              Left = 271
              Top = 48
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnLoader_TrayPos1'
            end
            object mlSnLoader_TrayPos2: TMyLed
              Left = 567
              Top = 168
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnLoader_TrayPos2'
            end
            object mlC_Loader_FrontRiseTray_2_On: TMyLed
              Left = 43
              Top = 77
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Loader_FrontRiseTray_2_On'
            end
            object mlC_Loader_FrontRiseTray_1_On: TMyLed
              Left = 43
              Top = 96
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Loader_FrontRiseTray_1_On'
            end
            object mlSnLoader_Inputend: TMyLed
              Left = 6
              Top = 61
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnLoader_Inputend'
            end
            object mlC_Loader_FrontRiseTray_1_Off: TMyLed
              Left = 43
              Top = 110
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Loader_FrontRiseTray_1_Off'
            end
            object mlSnLoader_InputFullTray: TMyLed
              Left = 6
              Top = 24
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnLoader_InputFullTray'
            end
            object Panel98: TPanel
              Left = 32
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 0
            end
            object Panel99: TPanel
              Left = 32
              Top = 64
              Width = 745
              Height = 9
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 1
            end
            object bpC_Loader_FrontSeparateTray_1: TBtnPanel
              Left = 65
              Top = 45
              Width = 45
              Height = 18
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 2
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Loader_FrontSeparateTray_1'
              Style = tsButtons
            end
            object Panel100: TPanel
              Left = 4
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Front'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 3
            end
            object Panel101: TPanel
              Left = 767
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Rear'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 4
            end
            object Panel102: TPanel
              Left = 768
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 5
            end
            object GroupBox1: TGroupBox
              Left = 184
              Top = 80
              Width = 201
              Height = 65
              Caption = 'Loader Car 1'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWindowText
              Font.Height = -16
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 6
              object mlC_Loader1_PushTray_Off: TMyLed
                Left = 20
                Top = 42
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'C_Loader1_PushTray_Off'
              end
              object mlC_Loader1_PushTray_On: TMyLed
                Left = 20
                Top = 27
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'C_Loader1_PushTray_On'
              end
              object mlC_Loader1_LeanOnTray_On: TMyLed
                Left = 100
                Top = 27
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'C_Loader1_LeanOnTray_On'
              end
              object mlC_Loader1_LeanOnTray_Off: TMyLed
                Left = 100
                Top = 42
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'C_Loader1_LeanOnTray_Off'
              end
              object bpC_Loader1_PushTray: TBtnPanel
                Left = 44
                Top = 27
                Width = 45
                Height = 30
                Hint = #20998#38626#27773#32568
                BevelInner = bvRaised
                Caption = '<->'
                Color = 8404992
                Font.Charset = CHINESEBIG5_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = #26032#32048#26126#39636
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 0
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'C_Loader1_PushTray'
                Style = tsButtons
              end
              object bpC_Loader1_LeanOnTray: TBtnPanel
                Left = 124
                Top = 27
                Width = 45
                Height = 30
                Hint = #20998#38626#27773#32568
                BevelInner = bvRaised
                Caption = '<->'
                Color = 8404992
                Font.Charset = CHINESEBIG5_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = #26032#32048#26126#39636
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 1
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'C_Loader1_LeanOnTray'
                Style = tsButtons
              end
            end
            object GroupBox2: TGroupBox
              Left = 392
              Top = 80
              Width = 201
              Height = 65
              Caption = 'Loader Car 2'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWindowText
              Font.Height = -16
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 7
              object mlC_Loader2_PushTray_Off: TMyLed
                Left = 20
                Top = 42
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'C_Loader2_PushTray_Off'
              end
              object mlC_Loader2_PushTray_On: TMyLed
                Left = 20
                Top = 27
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'C_Loader2_PushTray_On'
              end
              object mlC_Loader2_LeanOnTray_On: TMyLed
                Left = 100
                Top = 27
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'C_Loader2_LeanOnTray_On'
              end
              object mlC_Loader2_LeanOnTray_Off: TMyLed
                Left = 100
                Top = 42
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'C_Loader2_LeanOnTray_Off'
              end
              object bpC_Loader2_PushTray: TBtnPanel
                Left = 44
                Top = 27
                Width = 45
                Height = 30
                Hint = #20998#38626#27773#32568
                BevelInner = bvRaised
                Caption = '<->'
                Color = 8404992
                Font.Charset = CHINESEBIG5_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = #26032#32048#26126#39636
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 0
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'C_Loader2_PushTray'
                Style = tsButtons
              end
              object bpC_Loader2_LeanOnTray: TBtnPanel
                Left = 124
                Top = 27
                Width = 45
                Height = 30
                Hint = #20998#38626#27773#32568
                BevelInner = bvRaised
                Caption = '<->'
                Color = 8404992
                Font.Charset = CHINESEBIG5_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = #26032#32048#26126#39636
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 1
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'C_Loader2_LeanOnTray'
                Style = tsButtons
              end
            end
            object bpC_Loader_FrontRiseTray_2: TBtnPanel
              Left = 66
              Top = 76
              Width = 45
              Height = 17
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 9
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Loader_FrontRiseTray_2'
              Style = tsButtons
            end
            object bpC_Loader_FrontRiseTray_1: TBtnPanel
              Left = 65
              Top = 96
              Width = 45
              Height = 29
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 8
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Loader_FrontRiseTray_1'
              Style = tsButtons
            end
          end
          object Panel28: TPanel
            Left = 9
            Top = 378
            Width = 820
            Height = 25
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Tray Arm'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -21
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 5
          end
          object Panel29: TPanel
            Left = 9
            Top = 402
            Width = 820
            Height = 119
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Color = 12761254
            TabOrder = 6
            object mlC_TrayArm_FrontClamp_On: TMyLed
              Left = 352
              Top = 48
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_TrayArm_FrontClamp_On'
            end
            object mlC_TrayArm_FrontClamp_Off: TMyLed
              Left = 352
              Top = 127
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_TrayArm_FrontClamp_Off'
            end
            object mlC_TrayArmZ_Up_On: TMyLed
              Left = 390
              Top = 8
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_TrayArmZ_Up_On'
            end
            object mlC_TrayArmZ_Down_On: TMyLed
              Left = 390
              Top = 27
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_TrayArmZ_Down_On'
            end
            object mlC_TrayArm_RearClamp_On: TMyLed
              Left = 440
              Top = 48
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_TrayArm_RearClamp_On'
            end
            object mlC_TrayArm_RearClamp_Off: TMyLed
              Left = 440
              Top = 127
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_TrayArm_RearClamp_Off'
            end
            object bpC_TrayArmZ_Down: TBtnPanel
              Left = 415
              Top = 26
              Width = 45
              Height = 17
              BevelInner = bvRaised
              Caption = 'v'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 0
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_TrayArmZ_Down'
              Style = tsButtons
            end
            object bpC_TrayArm_FrontClamp: TBtnPanel
              Left = 377
              Top = 48
              Width = 45
              Height = 30
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 1
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_TrayArm_FrontClamp'
              Style = tsButtons
            end
            object Panel32: TPanel
              Left = 24
              Top = 88
              Width = 745
              Height = 9
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 2
            end
            object bpC_TrayArm_RearClamp: TBtnPanel
              Left = 465
              Top = 48
              Width = 45
              Height = 30
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 3
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_TrayArm_RearClamp'
              Style = tsButtons
            end
            object pnl7: TPanel
              Left = 4
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Front'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 4
            end
            object pnl8: TPanel
              Left = 767
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Rear'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 5
            end
            object bpC_TrayArmZ_Up: TBtnPanel
              Left = 415
              Top = 6
              Width = 45
              Height = 17
              BevelInner = bvRaised
              Caption = '^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 6
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_TrayArmZ_Up'
              Style = tsButtons
            end
          end
        end
      end
    end
    object ts_IOTapeLoadUnload: TTabSheet
      Caption = 'Unloader'
      ImageIndex = 6
      object PageControl2: TPageControl
        Left = 0
        Top = 0
        Width = 841
        Height = 698
        ActivePage = TabSheet3
        Align = alClient
        TabIndex = 0
        TabOrder = 0
        object TabSheet3: TTabSheet
          Caption = 'Auto 1~3'
          ImageIndex = 3
          object Panel22: TPanel
            Left = 3
            Top = 3
            Width = 454
            Height = 30
            BevelInner = bvLowered
            Color = 9534289
            TabOrder = 0
            object Panel23: TPanel
              Left = 5
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'M :'#39340#36948
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 0
            end
            object Panel24: TPanel
              Left = 90
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = '<-> : '#21069#24460#27773#32568
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 1
            end
            object Panel25: TPanel
              Left = 176
              Top = 5
              Width = 100
              Height = 20
              BevelInner = bvLowered
              Caption = 'v  ^ : '#19978#19979#27773#32568
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 2
            end
            object Panel26: TPanel
              Left = 281
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'V : '#30495#31354
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 3
            end
            object Panel27: TPanel
              Left = 366
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'D : '#30772#22750
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 4
            end
          end
          object Panel16: TPanel
            Left = 10
            Top = 42
            Width = 820
            Height = 25
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Auto 1'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -21
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 1
          end
          object Panel17: TPanel
            Left = 10
            Top = 66
            Width = 820
            Height = 119
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Color = 12761254
            TabOrder = 2
            object mlC_Auto1_FrontRiseTray_Off: TMyLed
              Left = 43
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto1_FrontRiseTray_Off'
            end
            object mlC_Auto1_FrontRiseTray_On: TMyLed
              Left = 43
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto1_FrontRiseTray_On'
            end
            object mlC_Auto1_PushTray_On: TMyLed
              Left = 175
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto1_PushTray_On'
            end
            object mlC_Auto1_PushTray_Off: TMyLed
              Left = 175
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto1_PushTray_Off'
            end
            object mlC_Auto1_LeanOnTray_On: TMyLed
              Left = 255
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto1_LeanOnTray_On'
            end
            object mlC_Auto1_LeanOnTray_Off: TMyLed
              Left = 255
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto1_LeanOnTray_Off'
            end
            object mlSnAuto1_InputHasTray: TMyLed
              Left = 6
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto1_InputHasTray'
            end
            object mlSnAuto1_InputFullTray: TMyLed
              Left = 6
              Top = 24
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto1_InputFullTray'
            end
            object mlSnAuto1_InputEnd: TMyLed
              Left = 6
              Top = 45
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto1_InputEnd'
            end
            object mlSnAuto1_OutputBottomHasTray: TMyLed
              Left = 780
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto1_OutputBottomHasTray'
            end
            object bpC_Auto1_LeanOnTray: TBtnPanel
              Left = 277
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 0
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto1_LeanOnTray'
              Style = tsButtons
            end
            object bpC_Auto1_FrontRiseTray: TBtnPanel
              Left = 65
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 1
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto1_FrontRiseTray'
              Style = tsButtons
            end
            object bpC_Auto1_PushTray: TBtnPanel
              Left = 197
              Top = 80
              Width = 45
              Height = 30
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 2
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto1_PushTray'
              Style = tsButtons
            end
            object Panel18: TPanel
              Left = 32
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 3
            end
            object Panel19: TPanel
              Left = 32
              Top = 64
              Width = 745
              Height = 9
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 4
            end
            object Panel20: TPanel
              Left = 4
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Front'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 5
            end
            object Panel21: TPanel
              Left = 767
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Rear'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 6
            end
            object Panel31: TPanel
              Left = 768
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 7
            end
          end
          object Panel42: TPanel
            Left = 10
            Top = 191
            Width = 820
            Height = 25
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Auto 2'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -21
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 3
          end
          object Panel43: TPanel
            Left = 10
            Top = 216
            Width = 820
            Height = 121
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Color = 12761254
            TabOrder = 4
            object mlC_Auto2_FrontRiseTray_Off: TMyLed
              Left = 43
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto2_FrontRiseTray_Off'
            end
            object mlC_Auto2_FrontRiseTray_On: TMyLed
              Left = 43
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto2_FrontRiseTray_On'
            end
            object mlC_Auto2_PushTray_On: TMyLed
              Left = 175
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto2_PushTray_On'
            end
            object mlC_Auto2_PushTray_Off: TMyLed
              Left = 175
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto2_PushTray_Off'
            end
            object mlC_Auto2_LeanOnTray_On: TMyLed
              Left = 255
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto2_LeanOnTray_On'
            end
            object mlC_Auto2_LeanOnTray_Off: TMyLed
              Left = 255
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto2_LeanOnTray_Off'
            end
            object mlSnAuto2_InputHasTray: TMyLed
              Left = 6
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto2_InputHasTray'
            end
            object mlSnAuto2_InputFullTray: TMyLed
              Left = 6
              Top = 24
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto2_InputFullTray'
            end
            object mlSnAuto2_InputEnd: TMyLed
              Left = 6
              Top = 45
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto2_InputEnd'
            end
            object mlSnAuto2_OutputBottomHasTray: TMyLed
              Left = 780
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto2_OutputBottomHasTray'
            end
            object bpC_Auto2_LeanOnTray: TBtnPanel
              Left = 277
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 0
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto2_LeanOnTray'
              Style = tsButtons
            end
            object bpC_Auto2_FrontRiseTray: TBtnPanel
              Left = 65
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 1
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto2_FrontRiseTray'
              Style = tsButtons
            end
            object bpC_Auto2_PushTray: TBtnPanel
              Left = 197
              Top = 80
              Width = 45
              Height = 30
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 2
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto2_PushTray'
              Style = tsButtons
            end
            object Panel54: TPanel
              Left = 32
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 3
            end
            object Panel66: TPanel
              Left = 32
              Top = 64
              Width = 745
              Height = 9
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 4
            end
            object Panel67: TPanel
              Left = 4
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Front'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 5
            end
            object Panel68: TPanel
              Left = 767
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Rear'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 6
            end
            object Panel71: TPanel
              Left = 768
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 7
            end
          end
          object Panel72: TPanel
            Left = 10
            Top = 343
            Width = 820
            Height = 25
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Auto 3'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -21
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 5
          end
          object Panel73: TPanel
            Left = 10
            Top = 367
            Width = 820
            Height = 122
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Color = 12761254
            TabOrder = 6
            object mlC_Auto3_FrontRiseTray_Off: TMyLed
              Left = 43
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto3_FrontRiseTray_Off'
            end
            object mlC_Auto3_FrontRiseTray_On: TMyLed
              Left = 43
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto3_FrontRiseTray_On'
            end
            object mlC_Auto3_PushTray_On: TMyLed
              Left = 175
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto3_PushTray_On'
            end
            object mlC_Auto3_PushTray_Off: TMyLed
              Left = 175
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto3_PushTray_Off'
            end
            object mlC_Auto3_LeanOnTray_On: TMyLed
              Left = 255
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto3_LeanOnTray_On'
            end
            object mlC_Auto3_LeanOnTray_Off: TMyLed
              Left = 255
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto3_LeanOnTray_Off'
            end
            object mlSnAuto3_InputHasTray: TMyLed
              Left = 6
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto3_InputHasTray'
            end
            object mlSnAuto3_InputFullTray: TMyLed
              Left = 6
              Top = 24
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto3_InputFullTray'
            end
            object mlSnAuto3_InputEnd: TMyLed
              Left = 6
              Top = 45
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto3_InputEnd'
            end
            object mlSnAuto3_OutputBottomHasTray: TMyLed
              Left = 780
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto3_OutputBottomHasTray'
            end
            object bpC_Auto3_LeanOnTray: TBtnPanel
              Left = 277
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 0
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto3_LeanOnTray'
              Style = tsButtons
            end
            object bpC_Auto3_FrontRiseTray: TBtnPanel
              Left = 65
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 1
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto3_FrontRiseTray'
              Style = tsButtons
            end
            object bpC_Auto3_PushTray: TBtnPanel
              Left = 197
              Top = 80
              Width = 45
              Height = 30
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 2
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto3_PushTray'
              Style = tsButtons
            end
            object Panel103: TPanel
              Left = 32
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 3
            end
            object Panel104: TPanel
              Left = 32
              Top = 64
              Width = 745
              Height = 9
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 4
            end
            object Panel105: TPanel
              Left = 4
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Front'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 5
            end
            object Panel106: TPanel
              Left = 767
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Rear'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 6
            end
            object Panel107: TPanel
              Left = 768
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 7
            end
          end
        end
        object TabSheet2: TTabSheet
          Caption = 'Auto 4~6'
          ImageIndex = 1
          object Panel108: TPanel
            Left = 3
            Top = 3
            Width = 454
            Height = 30
            BevelInner = bvLowered
            Color = 9534289
            TabOrder = 0
            object Panel109: TPanel
              Left = 5
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'M :'#39340#36948
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 0
            end
            object Panel110: TPanel
              Left = 90
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = '<-> : '#21069#24460#27773#32568
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 1
            end
            object Panel111: TPanel
              Left = 176
              Top = 5
              Width = 100
              Height = 20
              BevelInner = bvLowered
              Caption = 'v  ^ : '#19978#19979#27773#32568
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 2
            end
            object Panel112: TPanel
              Left = 281
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'V : '#30495#31354
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 3
            end
            object Panel113: TPanel
              Left = 366
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'D : '#30772#22750
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 4
            end
          end
          object Panel114: TPanel
            Left = 10
            Top = 42
            Width = 820
            Height = 25
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Auto 4'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -21
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 1
          end
          object Panel115: TPanel
            Left = 10
            Top = 66
            Width = 820
            Height = 119
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Color = 12761254
            TabOrder = 2
            object mlC_Auto4_FrontRiseTray_Off: TMyLed
              Left = 43
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto4_FrontRiseTray_Off'
            end
            object mlC_Auto4_FrontRiseTray_On: TMyLed
              Left = 43
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto4_FrontRiseTray_On'
            end
            object mlC_Auto4_PushTray_On: TMyLed
              Left = 175
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto4_PushTray_On'
            end
            object mlC_Auto4_PushTray_Off: TMyLed
              Left = 175
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto4_PushTray_Off'
            end
            object mlC_Auto4_LeanOnTray_On: TMyLed
              Left = 255
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto4_LeanOnTray_On'
            end
            object mlC_Auto4_LeanOnTray_Off: TMyLed
              Left = 255
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto4_LeanOnTray_Off'
            end
            object mlSnAuto4_InputHasTray: TMyLed
              Left = 6
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto4_InputHasTray'
            end
            object mlSnAuto4_InputFullTray: TMyLed
              Left = 6
              Top = 24
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto4_InputFullTray'
            end
            object mlSnAuto4_InputEnd: TMyLed
              Left = 6
              Top = 45
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto4_InputEnd'
            end
            object mlSnAuto4_OutputBottomHasTray: TMyLed
              Left = 780
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto4_OutputBottomHasTray'
            end
            object bpC_Auto4_LeanOnTray: TBtnPanel
              Left = 277
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 0
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto4_LeanOnTray'
              Style = tsButtons
            end
            object bpC_Auto4_FrontRiseTray: TBtnPanel
              Left = 65
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 1
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto4_FrontRiseTray'
              Style = tsButtons
            end
            object bpC_Auto4_PushTray: TBtnPanel
              Left = 197
              Top = 80
              Width = 45
              Height = 30
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 2
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto4_PushTray'
              Style = tsButtons
            end
            object Panel116: TPanel
              Left = 32
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 3
            end
            object Panel117: TPanel
              Left = 32
              Top = 64
              Width = 745
              Height = 9
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 4
            end
            object Panel118: TPanel
              Left = 4
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Front'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 5
            end
            object Panel119: TPanel
              Left = 767
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Rear'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 6
            end
            object Panel120: TPanel
              Left = 768
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 7
            end
          end
          object Panel121: TPanel
            Left = 10
            Top = 191
            Width = 820
            Height = 25
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Auto 5'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -21
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 3
          end
          object Panel122: TPanel
            Left = 10
            Top = 216
            Width = 820
            Height = 121
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Color = 12761254
            TabOrder = 4
            object mlC_Auto5_FrontRiseTray_Off: TMyLed
              Left = 43
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto5_FrontRiseTray_Off'
            end
            object mlC_Auto5_FrontRiseTray_On: TMyLed
              Left = 43
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto5_FrontRiseTray_On'
            end
            object mlC_Auto5_PushTray_On: TMyLed
              Left = 175
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto5_PushTray_On'
            end
            object mlC_Auto5_PushTray_Off: TMyLed
              Left = 175
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto5_PushTray_Off'
            end
            object mlC_Auto5_LeanOnTray_On: TMyLed
              Left = 255
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto5_LeanOnTray_On'
            end
            object mlC_Auto5_LeanOnTray_Off: TMyLed
              Left = 255
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto5_LeanOnTray_Off'
            end
            object mlSnAuto5_InputHasTray: TMyLed
              Left = 6
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto5_InputHasTray'
            end
            object mlSnAuto5_InputFullTray: TMyLed
              Left = 6
              Top = 24
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto5_InputFullTray'
            end
            object mlSnAuto5_InputEnd: TMyLed
              Left = 6
              Top = 45
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto5_InputEnd'
            end
            object mlSnAuto5_OutputBottomHasTray: TMyLed
              Left = 780
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto5_OutputBottomHasTray'
            end
            object bpC_Auto5_LeanOnTray: TBtnPanel
              Left = 277
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 0
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto5_LeanOnTray'
              Style = tsButtons
            end
            object bpC_Auto5_FrontRiseTray: TBtnPanel
              Left = 65
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 1
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto5_FrontRiseTray'
              Style = tsButtons
            end
            object bpC_Auto5_PushTray: TBtnPanel
              Left = 197
              Top = 80
              Width = 45
              Height = 30
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 2
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto5_PushTray'
              Style = tsButtons
            end
            object Panel123: TPanel
              Left = 32
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 3
            end
            object Panel124: TPanel
              Left = 32
              Top = 64
              Width = 745
              Height = 9
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 4
            end
            object Panel125: TPanel
              Left = 4
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Front'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 5
            end
            object Panel126: TPanel
              Left = 767
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Rear'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 6
            end
            object Panel127: TPanel
              Left = 768
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 7
            end
          end
          object Panel128: TPanel
            Left = 10
            Top = 343
            Width = 820
            Height = 25
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Auto 6'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -21
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 5
          end
          object Panel131: TPanel
            Left = 10
            Top = 367
            Width = 820
            Height = 122
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Color = 12761254
            TabOrder = 6
            object mlC_Auto6_FrontRiseTray_Off: TMyLed
              Left = 43
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto6_FrontRiseTray_Off'
            end
            object mlC_Auto6_FrontRiseTray_On: TMyLed
              Left = 43
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto6_FrontRiseTray_On'
            end
            object mlC_Auto6_PushTray_On: TMyLed
              Left = 175
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto6_PushTray_On'
            end
            object mlC_Auto6_PushTray_Off: TMyLed
              Left = 175
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto6_PushTray_Off'
            end
            object mlC_Auto6_LeanOnTray_On: TMyLed
              Left = 255
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto6_LeanOnTray_On'
            end
            object mlC_Auto6_LeanOnTray_Off: TMyLed
              Left = 255
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Auto6_LeanOnTray_Off'
            end
            object mlSnAuto6_InputHasTray: TMyLed
              Left = 6
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto6_InputHasTray'
            end
            object mlSnAuto6_InputFullTray: TMyLed
              Left = 6
              Top = 24
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto6_InputFullTray'
            end
            object mlSnAuto6_InputEnd: TMyLed
              Left = 6
              Top = 45
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto6_InputEnd'
            end
            object mlSnAuto6_OutputBottomHasTray: TMyLed
              Left = 780
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnAuto6_OutputBottomHasTray'
            end
            object bpC_Auto6_LeanOnTray: TBtnPanel
              Left = 277
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 0
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto6_LeanOnTray'
              Style = tsButtons
            end
            object bpC_Auto6_FrontRiseTray: TBtnPanel
              Left = 65
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 1
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto6_FrontRiseTray'
              Style = tsButtons
            end
            object bpC_Auto6_PushTray: TBtnPanel
              Left = 197
              Top = 80
              Width = 45
              Height = 30
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 2
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Auto6_PushTray'
              Style = tsButtons
            end
            object Panel132: TPanel
              Left = 32
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 3
            end
            object Panel141: TPanel
              Left = 32
              Top = 64
              Width = 745
              Height = 9
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 4
            end
            object Panel143: TPanel
              Left = 4
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Front'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 5
            end
            object Panel144: TPanel
              Left = 767
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Rear'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 6
            end
            object Panel145: TPanel
              Left = 768
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 7
            end
          end
          object Panel146: TPanel
            Left = 9
            Top = 492
            Width = 820
            Height = 25
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Color'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -21
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 7
            Visible = False
          end
          object Panel147: TPanel
            Left = 9
            Top = 516
            Width = 820
            Height = 141
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Color = 12761254
            TabOrder = 8
            object mlC_Color_FrontRiseTray_1_Off: TMyLed
              Left = 43
              Top = 111
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Color_FrontRiseTray_1_Off'
            end
            object mlC_Color_FrontRiseTray_1_On: TMyLed
              Left = 43
              Top = 96
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Color_FrontRiseTray_1_On'
            end
            object mlC_Color_PushTray_On: TMyLed
              Left = 175
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Color_PushTray_On'
            end
            object mlC_Color_PushTray_Off: TMyLed
              Left = 175
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Color_PushTray_Off'
            end
            object mlC_Color_LeanOnTray_On: TMyLed
              Left = 255
              Top = 80
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Color_LeanOnTray_On'
            end
            object mlC_Color_LeanOnTray_Off: TMyLed
              Left = 255
              Top = 95
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Color_LeanOnTray_Off'
            end
            object mlSnColor_InputHasTray: TMyLed
              Left = 6
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnColor_InputHasTray'
            end
            object mlSnColor_InputFullTray: TMyLed
              Left = 6
              Top = 24
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnColor_InputFullTray'
            end
            object mlSnColor_OutputBottomHasTray: TMyLed
              Left = 780
              Top = 56
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnColor_OutputBottomHasTray'
            end
            object mlSnColor_TrayPos1: TMyLed
              Left = 271
              Top = 48
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnColor_TrayPos1'
            end
            object mlSnColor_InputEnd: TMyLed
              Left = 6
              Top = 45
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'SnColor_InputEnd'
            end
            object mlC_Color_FrontRiseTray_2_On: TMyLed
              Left = 43
              Top = 77
              Width = 22
              Height = 14
              LEDStyle = LEDHorizontal
              Alias = 'C_Color_FrontRiseTray_2_On'
            end
            object bpC_Color_LeanOnTray: TBtnPanel
              Left = 277
              Top = 80
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 0
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Color_LeanOnTray'
              Style = tsButtons
            end
            object bpC_Color_FrontRiseTray_1: TBtnPanel
              Left = 65
              Top = 96
              Width = 45
              Height = 30
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 1
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Color_FrontRiseTray_1'
              Style = tsButtons
            end
            object bpC_Color_PushTray: TBtnPanel
              Left = 197
              Top = 80
              Width = 45
              Height = 30
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 2
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Color_PushTray'
              Style = tsButtons
            end
            object Panel148: TPanel
              Left = 32
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 3
            end
            object Panel149: TPanel
              Left = 32
              Top = 64
              Width = 745
              Height = 9
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 4
            end
            object bpC_Color_FrontSeparateTray_1: TBtnPanel
              Left = 65
              Top = 45
              Width = 45
              Height = 14
              Hint = #20998#38626#27773#32568
              BevelInner = bvRaised
              Caption = '<->'
              Color = 8404992
              Font.Charset = CHINESEBIG5_CHARSET
              Font.Color = clWhite
              Font.Height = -12
              Font.Name = #26032#32048#26126#39636
              Font.Style = []
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 5
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Color_FrontSeparateTray_1'
              Style = tsButtons
            end
            object Panel150: TPanel
              Left = 4
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Front'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 6
            end
            object Panel151: TPanel
              Left = 767
              Top = 3
              Width = 50
              Height = 15
              BevelInner = bvLowered
              Caption = 'Rear'
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 7
            end
            object Panel152: TPanel
              Left = 768
              Top = 16
              Width = 9
              Height = 49
              BevelInner = bvLowered
              Color = 9534289
              TabOrder = 8
            end
            object bpC_Color_FrontRiseTray_2: TBtnPanel
              Left = 66
              Top = 76
              Width = 45
              Height = 17
              BevelInner = bvRaised
              Caption = 'v    ^'
              Color = 8404992
              Font.Charset = ANSI_CHARSET
              Font.Color = clWhite
              Font.Height = -15
              Font.Name = 'Times New Roman'
              Font.Style = [fsBold]
              ParentFont = False
              ParentShowHint = False
              ShowHint = True
              TabOrder = 9
              OnClick = BtnPanelClick
              TrueColor = 16744448
              FalseColor = 8404992
              FalseFontColor = clWhite
              Alias = 'C_Color_FrontRiseTray_2'
              Style = tsButtons
            end
          end
        end
      end
    end
    object ts_IOTapeShuttle: TTabSheet
      Caption = 'Sucker'
      object PageControl3: TPageControl
        Left = 0
        Top = 0
        Width = 841
        Height = 698
        ActivePage = TabSheet5
        Align = alClient
        TabIndex = 0
        TabOrder = 0
        object TabSheet5: TTabSheet
          Caption = 'Sucker'
          ImageIndex = 3
          object Panel34: TPanel
            Left = 3
            Top = 3
            Width = 454
            Height = 30
            BevelInner = bvLowered
            Color = 9534289
            TabOrder = 0
            object Panel35: TPanel
              Left = 5
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'M :'#39340#36948
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 0
            end
            object Panel36: TPanel
              Left = 90
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = '<-> : '#21069#24460#27773#32568
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 1
            end
            object Panel37: TPanel
              Left = 176
              Top = 5
              Width = 100
              Height = 20
              BevelInner = bvLowered
              Caption = 'v  ^ : '#19978#19979#27773#32568
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 2
            end
            object Panel38: TPanel
              Left = 281
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'V : '#30495#31354
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 3
            end
            object Panel39: TPanel
              Left = 366
              Top = 5
              Width = 80
              Height = 20
              BevelInner = bvLowered
              Caption = 'D : '#30772#22750
              Color = 9534289
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = []
              ParentFont = False
              TabOrder = 4
            end
          end
          object Panel30: TPanel
            Left = 20
            Top = 45
            Width = 355
            Height = 20
            BevelInner = bvLowered
            BevelOuter = bvSpace
            Caption = 'Sucker'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -16
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 1
          end
          object Panel33: TPanel
            Left = 20
            Top = 72
            Width = 355
            Height = 137
            BevelOuter = bvNone
            Color = 9534289
            TabOrder = 2
            object Panel40: TPanel
              Left = 5
              Top = 5
              Width = 80
              Height = 124
              BevelInner = bvRaised
              BevelOuter = bvLowered
              Color = 12761254
              TabOrder = 0
              object mlSuck1: TMyLed
                Left = 30
                Top = 106
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'Suck1'
              end
              object Panel41: TPanel
                Left = 10
                Top = 10
                Width = 60
                Height = 60
                BevelInner = bvLowered
                Caption = '1'
                Color = 9534289
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -27
                Font.Name = #26032#32048#26126#39636
                Font.Style = [fsBold]
                ParentFont = False
                TabOrder = 0
              end
              object bpSuck1_On: TBtnPanel
                Left = 10
                Top = 81
                Width = 30
                Height = 20
                BevelInner = bvRaised
                Caption = 'V'
                Color = 8404992
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = 'Times New Roman'
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 1
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'Suck1_On'
                Style = tsButtons
              end
              object bpSuck1_Off: TBtnPanel
                Left = 43
                Top = 81
                Width = 30
                Height = 20
                BevelInner = bvRaised
                Caption = 'D'
                Color = 8404992
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = 'Times New Roman'
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 2
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'Suck1_Off'
                Style = tsButtons
              end
            end
            object Panel44: TPanel
              Left = 93
              Top = 5
              Width = 80
              Height = 124
              BevelInner = bvRaised
              BevelOuter = bvLowered
              Color = 12761254
              TabOrder = 1
              object mlSuck2: TMyLed
                Left = 30
                Top = 106
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'Suck2'
              end
              object Panel45: TPanel
                Left = 10
                Top = 10
                Width = 60
                Height = 60
                BevelInner = bvLowered
                Caption = '2'
                Color = 9534289
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -27
                Font.Name = #26032#32048#26126#39636
                Font.Style = [fsBold]
                ParentFont = False
                TabOrder = 0
              end
              object bpSuck2_On: TBtnPanel
                Left = 10
                Top = 81
                Width = 30
                Height = 20
                BevelInner = bvRaised
                Caption = 'V'
                Color = 8404992
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = 'Times New Roman'
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 1
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'Suck2_On'
                Style = tsButtons
              end
              object bpSuck2_Off: TBtnPanel
                Left = 43
                Top = 81
                Width = 30
                Height = 20
                BevelInner = bvRaised
                Caption = 'D'
                Color = 8404992
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = 'Times New Roman'
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 2
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'Suck2_Off'
                Style = tsButtons
              end
            end
            object Panel55: TPanel
              Left = 181
              Top = 5
              Width = 80
              Height = 124
              BevelInner = bvRaised
              BevelOuter = bvLowered
              Color = 12761254
              TabOrder = 2
              object mlSuck3: TMyLed
                Left = 30
                Top = 106
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'Suck3'
              end
              object Panel56: TPanel
                Left = 10
                Top = 10
                Width = 60
                Height = 60
                BevelInner = bvLowered
                Caption = '3'
                Color = 9534289
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -27
                Font.Name = #26032#32048#26126#39636
                Font.Style = [fsBold]
                ParentFont = False
                TabOrder = 0
              end
              object bpSuck3_On: TBtnPanel
                Left = 10
                Top = 81
                Width = 30
                Height = 20
                BevelInner = bvRaised
                Caption = 'V'
                Color = 8404992
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = 'Times New Roman'
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 1
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'Suck3_On'
                Style = tsButtons
              end
              object bpSuck3_Off: TBtnPanel
                Left = 43
                Top = 81
                Width = 30
                Height = 20
                BevelInner = bvRaised
                Caption = 'D'
                Color = 8404992
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = 'Times New Roman'
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 2
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'Suck3_Off'
                Style = tsButtons
              end
            end
            object Panel57: TPanel
              Left = 269
              Top = 5
              Width = 80
              Height = 124
              BevelInner = bvRaised
              BevelOuter = bvLowered
              Color = 12761254
              TabOrder = 3
              object mlSuck4: TMyLed
                Left = 30
                Top = 106
                Width = 22
                Height = 14
                LEDStyle = LEDHorizontal
                Alias = 'Suck4'
              end
              object Panel58: TPanel
                Left = 10
                Top = 10
                Width = 60
                Height = 60
                BevelInner = bvLowered
                Caption = '4'
                Color = 9534289
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -27
                Font.Name = #26032#32048#26126#39636
                Font.Style = [fsBold]
                ParentFont = False
                TabOrder = 0
              end
              object bpSuck4_On: TBtnPanel
                Left = 10
                Top = 81
                Width = 30
                Height = 20
                BevelInner = bvRaised
                Caption = 'V'
                Color = 8404992
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = 'Times New Roman'
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 1
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'Suck4_On'
                Style = tsButtons
              end
              object bpSuck4_Off: TBtnPanel
                Left = 43
                Top = 81
                Width = 30
                Height = 20
                BevelInner = bvRaised
                Caption = 'D'
                Color = 8404992
                Font.Charset = ANSI_CHARSET
                Font.Color = clWhite
                Font.Height = -12
                Font.Name = 'Times New Roman'
                Font.Style = []
                ParentFont = False
                ParentShowHint = False
                ShowHint = True
                TabOrder = 2
                OnClick = BtnPanelClick
                TrueColor = 16744448
                FalseColor = 8404992
                FalseFontColor = clWhite
                Alias = 'Suck4_Off'
                Style = tsButtons
              end
            end
          end
        end
      end
    end
    object ts_IOPanel: TTabSheet
      Caption = 'Panel'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ImageIndex = 12
      ParentFont = False
      object pn_IOPanel: TPanel
        Left = 2
        Top = 0
        Width = 839
        Height = 660
        BevelInner = bvRaised
        BevelOuter = bvLowered
        Color = 12761254
        TabOrder = 0
        object pn_PanelFrontOb: TPanel
          Left = 3
          Top = 40
          Width = 415
          Height = 480
          BevelInner = bvLowered
          BevelOuter = bvSpace
          Color = 12761254
          TabOrder = 0
          object sb_IO_CommunicationPad: TSpeedButton
            Left = 7
            Top = 12
            Width = 80
            Height = 45
            Caption = 'Pad'
            Glyph.Data = {
              F60F0000424DF60F000000000000360000002800000025000000240000000100
              180000000000C00F0000252E0000252E00000000000000000000FFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFEFEFEFCFCFCFCFCFCFCFCFDFDFDFDFDFDFDFDFDFDFEFE
              FEFEFEFEFEFEFEFEFEFEFFFFFFFFFFFFFFFFFFFFFFFFFEFEFEFEFEFEFEFEFEFE
              FEFEFEFEFEFDFDFDFDFDFDFDFDFDFDFDFDFCFCFCFCFCFCFCFCFCFCFCFCFDFDFE
              FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFF2F3F3BBBDBE96979A9C
              9D9EA5A6A8AEAFB1B8B9BAC3C3C4CCCCCDD4D4D6DEDEDFE7E8E8F1F1F2FBFBFB
              FCFCFCF6F6F6EEEFEFE6E6E7DEDEDFD5D6D7CDCECFC5C5C7BCBCBEB4B5B7ABAD
              AFA4A4A79B9C9F9293968E9092B4B4B6F3F3F3FFFFFFFFFFFF00FFFFFFFFFFFF
              FFFFFFECECEC57595D6263667E7F81898A8C9494969E9FA1A9AAACB6B6B8C1C1
              C3CCCCCDD7D7D8E0E0E1EBEBEBF7F7F7FCFCFDF7F7F6EEEEEEE5E5E6DCDCDDD2
              D2D3CACACBC1C1C2B6B6B8ADAEAFA4A5A69C9C9E9293948A8B8C80818358595C
              45474BEEEEEEFFFFFF00FFFFFFFFFFFFF9F9F9949598595A5D8F8F91A1A2A3AA
              ABACB2B3B3B8B9B9C2C2C3C9C9CAD2D2D3DBDBDCE1E1E1E9E9E9F2F2F3F9F9F9
              F7F7F7F4F4F4F3F3F3ECECECE7E7E8E2E2E3DCDBDCD5D6D6D0D0D1CACACCC4C4
              C6BFBFC0B8B8B9B4B3B4A6A6A79798995E5F61909193FCFCFC00FFFFFFFFFFFF
              F3F4F4595B5F7071719B9C9DA3A4A5ACADAEB3B4B5BBBCBBC3C4C4CACACBD2D2
              D3DBDBDCE1E2E2E9EAEAF5F4F5EEEEEECFCFCFD7D7D7F2F2F2EEEEEFE8E8E8E3
              E3E3DCDCDDD7D7D8D2D2D3CCCCCDC6C6C7C1C1C2BBBBBCB6B6B7AAAAABA8A8A9
              7273745C5E62FAFAFA00FFFFFFFFFFFFF3F3F35254587172759C9C9DA3A4A5AC
              ADAEB3B4B5BABBBBC3C3C4CACACBD2D2D4DDDCDDE3E4E4ECECECF9F9F9ECECED
              DEDFE0E7E7E7F0F0F0F1F1F2EAEAEAE5E5E5DEDEDED8D8D9D2D2D3CCCCCDC6C6
              C7C2C2C3BCBCBDB7B7B8ABABACA9A9AA74757655575AFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759C9C9DA3A4A5ADAEAFB8B9B9C2C3C2CDCDCED1D1D2D3D3
              D4D7D7D8D7D8D8DADADBDDDCDEDFDFE0E2E3E3E0E1E1DDDDDEDADBDCD9DADAD9
              D9DAD7D7D8D5D5D6D4D4D5CFCFD0C9C9CAC2C2C3B8B8B9B3B3B4AAAAABA9A9AA
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759D9D9E9FA0A18E
              8F91939496A7A9AAB6B6B8ADADB097999C87898B7274765D5F63484B4F313438
              292C3034373B4245485254576264687173778284868E90929FA0A1A1A2A39798
              9A8D8E907576788B8B8DA7A7A8AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F96979836393D03060B06090F05080E05080F080B
              100A0D120C0F140E111610131812151A13161B11141A1114180F12170D10150C
              0F140A0D12080C11070A0F070A10080B11090C1204080D595A5EA3A4A5AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759E9E9F9698993D
              414412151A171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F
              171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A
              1F171A1F11141956585BA3A3A4AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F9698993D414412151A171A1F171A1F171A1F171A
              1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F17
              1A1F171A1F171A1F171A1F171A1F171A1F171A1F11141957595CA3A3A4AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759E9E9F9698993D
              414412151A171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F
              171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A1F171A
              1F171A1F11141957595CA3A3A4AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F9698993D414412151A171A1F171A1F171A1F171A
              1F14171E13171E171A1F11161D13171D181B1F15191F15191F171A1F171A1F17
              1A1F171A1F171A1F171A1F171A1F171A1F171A1F11141957595CA3A3A4AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759E9E9F9698993D
              414412151A171A1F171A1F171A1F171A1E252422272726181A1C2D2A23272626
              15181D1B1C1E14181F14181F171A1F171A1F171A1F171A1F171A1F171A1F171A
              1F171A1F11141957595CA3A3A4AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F9698993D414412151A171A1F171A1F15191F1B1B
              19735E347C6B4C1B170E977639726247040B185D46195E481C12171F0F141F17
              1A1F171A1F171A1F171A1F171A1F171A1F171A1F11141957595CA3A3A4AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759E9E9F9698993D
              414412151A171A1F171A1F1418201E1B15A4823EB096641D1506D3A2459C855A
              000016976B14E19E1A7158271C1E200C121D16191F171A1F171A1F171A1F171A
              1F171A1F11141957595CA3A3A4AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F9698993D414412151A171A1F171A1F1418201D1B
              169C7B3DA88E601C1607C89A43947F57000116886114FEB01BE8AA3298783632
              2E260C121C14181E171A1F171A1F171A1F171A1F11141957595CA3A3A4AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759E9E9F9698993D
              414412151A171A1F171A1F1418201D1B169C7B3DA88E601C1607C89A43947F57
              000116886214F1A91AE9AB32F6BB47BE9955584E3B1D1F2115181E171A1F171A
              1F171A1F11141957595CA3A3A4AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F9698993D414412151A16191E16191E13171F1C1A
              159B7B3CA88F601C1607C89A43947F57000116886214F2A91AE4A831E9B043FF
              D874CDAE6C39352D10141C171A1F171A1F171A1F11141957595CA3A3A4AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759E9E9F9698993D
              414414171C1D20251D20251A1E25211F1A9D7C3EA88E601C1507C89A42947F57
              000116886214F2A91AE5A932F9BD48D3AB5D78674826252513171E171A1F171A
              1F171A1F11141957595CA3A3A4AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F9698994043461A1D222A2C31292C31262A322C2B
              25A08041AB9263251F10CA9C44947E56000015866014F8AD1AF3B233B28A3B4B
              412F10151D11161D181A1F171A1F171A1F171A1F11141957595CA3A3A4AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759E9E9F97989942
              45481D20253133382F31362C303733312BA28344AF95662F2819CC9E4699835B
              010D22976C17F1A8198D6B282B29220C121C14181E171A1F171A1F171A1F171A
              1F171A1F11141957595CA3A3A4AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F9798994246491D202634373B3235392F343A3734
              2DAE8C47B9A16C342B1ADAAA48A5906311192A785B238363252A2A2D171C271B
              1E23171A1F15181D15181D16191E16191E16191E11141957595CA3A3A4AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759E9E9F97989943
              464A202328393B4037393E36393F373839594F3E5D564A34333267583F565046
              282C333A39333133342A2E372C2F33272B3026282D24272B2023281D20251C1F
              231C1F2313161B585A5DA3A3A4AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F97989944464A22262A3C40443A3E423A3E42393D
              412F343D2F333B37393F2E333D2E323934363A2F333A30333A34363A3133382F
              32363032373032372E31352A2C31282B2F292C3016191F595B5EA3A3A4AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759E9E9F97989A44
              474B24272C4245493F42463F42463E41453B3E423B3E423B3E423B3E42393C40
              37393E37393E37393E37393E35373C33353A33353A33353A33353A3033372F32
              36313438191C205A5C5FA3A3A4AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F97989A45484C262A2E474A4E44474B44474B4345
              494041454042464042464042463E40443B3E423B3E423B3E423B3F42393B4037
              393E37393E37393E37393E34363B33353A35373C1A1D225A5C5FA3A3A4AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35355597172759E9E9F97989A46
              484C292B304B4D51484A4E484A4E47494D44474B44474B44474B44474B424549
              4042464042464042464142463D40443B3E423B3E423B3E423B3E42383B3F373A
              3E393C401B1E235B5C5FA3A3A4AAAAAB74757556585BFAFAFA00FFFFFFFFFFFF
              F3F3F35355597172759E9E9F97989A44474B272A2F4D5054494C504A4C50494B
              4F45484C46494D48494D484A4E47494D44474B44474B44474B44474B4143473E
              40443E40443D40443D4044393C40383C403B3E421C1F245B5C5FA3A3A4AAAAAB
              74757556585BFAFAFA00FFFFFFFFFFFFF3F3F35254587172759E9E9F999A9C54
              56593A3D4166696B65686B6366695E5F64585A5D5255594D5054494B4F424549
              3E414541434745474A494B4E494C504B4E5250525652545856585B5255595053
              565354581F22265B5C60A3A3A5AAAAAB74757655575AFAFAFA00FFFFFFFFFFFF
              F4F4F45C5E616F70729C9D9EA2A3A4A1A2A3AEB0B0C5C6C6D3D3D4CECED0BFC0
              C1B4B5B6A5A6A896989A888A8D77797D727377797C7E8485888F9092999B9CA5
              A5A8B0B0B2B7B8B9C3C5C5C4C4C5B9BABBB0B1B39597989C9C9DA9A9AAA9A9AA
              727375606265FAFAFA00FFFFFFFFFFFFFAFAFA9E9FA25354588D8E8F9F9FA0AB
              ACADB4B5B6BCBDBDC6C8C7CCCCCDD2D2D3D9D9DADDDDDDE3E4E4E9E9E9EDEDEF
              F0F0F0EEEEEEEAEAEAE4E5E5E0E1E2DEDEDFD9DADAD5D5D6D2D1D3CBCCCDC5C6
              C7BFBFC0B6B7B7B1B2B2A5A5A695969658595C9A9B9DFCFCFC00FFFFFFFFFFFF
              FFFFFFEDEEEF686A6F57585C7172757D7E8089898B949597A0A1A3B0B0B1BBBC
              BDC7C8C9D3D4D5DFDFE0EBEBECF8F8F8FDFCFCF4F3F4E9E9EADFDFE0D5D5D6CC
              CCCDC2C2C3B8B9B9ABABADA0A1A29697988C8D8F838386797A7C6F70724A4B4E
              55575AF1F1F1FFFFFF00FFFFFFFFFFFFFFFFFFFEFEFEF5F5F6F3F3F3F4F4F5F5
              F5F6F6F6F6F7F7F7F8F8F8F9F9F9FAFAFAFBFBFBFBFBFCFCFCFCFDFDFDFFFFFF
              FFFFFFFEFEFEFEFEFEFCFCFCFCFCFCFBFBFBFAFAFAF9F9FAF8F8F9F8F8F8F7F7
              F7F6F6F7F6F6F6F5F5F5F4F4F4F1F2F2F3F4F4FFFFFFFFFFFF00FFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00}
            OnClick = sb_IO_CommunicationPadClick
          end
        end
        object pn_PanelLampTitle: TPanel
          Left = 419
          Top = 18
          Width = 415
          Height = 20
          BevelInner = bvLowered
          BevelOuter = bvSpace
          Caption = 'LAMP'
          Color = 9534289
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -16
          Font.Name = #26032#32048#26126#39636
          Font.Style = []
          ParentFont = False
          TabOrder = 1
        end
        object pn_PanelMusicTitle: TPanel
          Left = 420
          Top = 138
          Width = 415
          Height = 20
          BevelInner = bvLowered
          BevelOuter = bvSpace
          Caption = 'MUSIC'
          Color = 9534289
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -16
          Font.Name = #26032#32048#26126#39636
          Font.Style = []
          ParentFont = False
          TabOrder = 2
        end
        object pn_PanelLamp: TPanel
          Left = 419
          Top = 38
          Width = 415
          Height = 90
          BevelInner = bvLowered
          BevelOuter = bvSpace
          Color = 12761254
          TabOrder = 4
          object bpSwTowerRed: TBtnPanel
            Left = 40
            Top = 20
            Width = 85
            Height = 50
            BevelInner = bvRaised
            BevelWidth = 2
            Caption = 'RED'
            Color = 8404992
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -12
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentFont = False
            ParentShowHint = False
            ShowHint = True
            TabOrder = 0
            OnClick = BtnPanelClick
            TrueColor = 16744448
            FalseColor = 8404992
            FalseFontColor = clWhite
            Alias = 'SwTowerRed'
            Style = tsButtons
          end
          object bpSwTowerYellow: TBtnPanel
            Left = 165
            Top = 20
            Width = 85
            Height = 50
            BevelInner = bvRaised
            BevelWidth = 2
            Caption = 'YELLOW'
            Color = 8404992
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -12
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentFont = False
            ParentShowHint = False
            ShowHint = True
            TabOrder = 1
            OnClick = BtnPanelClick
            TrueColor = 16744448
            FalseColor = 8404992
            FalseFontColor = clWhite
            Alias = 'SwTowerYellow'
            Style = tsButtons
          end
          object bpSwTowerGreen: TBtnPanel
            Left = 285
            Top = 20
            Width = 85
            Height = 50
            BevelInner = bvRaised
            BevelWidth = 2
            Caption = 'GREEN'
            Color = 8404992
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -12
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentFont = False
            ParentShowHint = False
            ShowHint = True
            TabOrder = 2
            OnClick = BtnPanelClick
            TrueColor = 16744448
            FalseColor = 8404992
            FalseFontColor = clWhite
            Alias = 'SwTowerGreen'
            Style = tsButtons
          end
        end
        object pn_PanelMusic: TPanel
          Left = 420
          Top = 158
          Width = 415
          Height = 90
          BevelInner = bvLowered
          BevelOuter = bvSpace
          Color = 12761254
          TabOrder = 3
          object bpSwMusic1: TBtnPanel
            Left = 15
            Top = 20
            Width = 85
            Height = 50
            BevelInner = bvRaised
            BevelWidth = 2
            Caption = 'MUSIC 1'
            Color = 8404992
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -12
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentFont = False
            ParentShowHint = False
            ShowHint = True
            TabOrder = 0
            OnClick = BtnPanelClick
            TrueColor = 16744448
            FalseColor = 8404992
            FalseFontColor = clWhite
            Alias = 'SwMusic1'
            Style = tsButtons
          end
          object bpSwMusic2: TBtnPanel
            Left = 115
            Top = 20
            Width = 85
            Height = 50
            BevelInner = bvRaised
            BevelWidth = 2
            Caption = 'MUSIC 2'
            Color = 8404992
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -12
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentFont = False
            ParentShowHint = False
            ShowHint = True
            TabOrder = 1
            OnClick = BtnPanelClick
            TrueColor = 16744448
            FalseColor = 8404992
            FalseFontColor = clWhite
            Alias = 'SwMusic2'
            Style = tsButtons
          end
          object bpSwMusic3: TBtnPanel
            Left = 215
            Top = 20
            Width = 85
            Height = 50
            BevelInner = bvRaised
            BevelWidth = 2
            Caption = 'MUSIC 3'
            Color = 8404992
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -12
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentFont = False
            ParentShowHint = False
            ShowHint = True
            TabOrder = 2
            OnClick = BtnPanelClick
            TrueColor = 16744448
            FalseColor = 8404992
            FalseFontColor = clWhite
            Alias = 'SwMusic3'
            Style = tsButtons
          end
          object bpSwMusic4: TBtnPanel
            Left = 315
            Top = 20
            Width = 85
            Height = 50
            BevelInner = bvRaised
            BevelWidth = 2
            Caption = 'MUSIC 4'
            Color = 8404992
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -12
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentFont = False
            ParentShowHint = False
            ShowHint = True
            TabOrder = 3
            OnClick = BtnPanelClick
            TrueColor = 16744448
            FalseColor = 8404992
            FalseFontColor = clWhite
            Alias = 'SwMusic4'
            Style = tsButtons
          end
        end
      end
    end
    object ts_IOSystem: TTabSheet
      Caption = 'System'
      ImageIndex = 7
      object Panel129: TPanel
        Left = 2
        Top = 0
        Width = 839
        Height = 660
        BevelInner = bvRaised
        BevelOuter = bvLowered
        Color = 12761254
        TabOrder = 0
        object Panel130: TPanel
          Left = 16
          Top = 8
          Width = 809
          Height = 577
          BevelInner = bvLowered
          Color = 12761254
          TabOrder = 0
          object mlSnMotorPower: TMyLed
            Left = 10
            Top = 546
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnMotorPower'
          end
          object mlSnEMG: TMyLed
            Left = 207
            Top = 543
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnEMG'
          end
          object img1: TImage
            Left = 16
            Top = 36
            Width = 766
            Height = 496
            AutoSize = True
            Picture.Data = {
              0A544A504547496D61676554F80000FFD8FFE000104A46494600010101006000
              600000FFE111064578696600004D4D002A000000080004013B00020000001800
              00084A8769000400000001000008629C9D000100000024000010DAEA1C000700
              00080C0000003E000000001CEA00000008000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              00000000000000000000000000000000000000000000004561736F6E6C697528
              E58A89E5AE9CE6BE8433353036290000059003000200000014000010B0900400
              0200000014000010C49291000200000003353400009292000200000003353400
              00EA1C00070000080C000008A4000000001CEA00000008000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000000000
              0000000000000000000000000000000000000000000000000000000000323032
              323A30383A31302031363A34393A343200323032323A30383A31302031363A34
              393A34320000004500610073006F006E006C0069007500280089529C5B846F33
              0035003000360029000000FFE10B2A687474703A2F2F6E732E61646F62652E63
              6F6D2F7861702F312E302F003C3F787061636B657420626567696E3D27EFBBBF
              272069643D2757354D304D7043656869487A7265537A4E54637A6B633964273F
              3E0D0A3C783A786D706D65746120786D6C6E733A783D2261646F62653A6E733A
              6D6574612F223E3C7264663A52444620786D6C6E733A7264663D22687474703A
              2F2F7777772E77332E6F72672F313939392F30322F32322D7264662D73796E74
              61782D6E7323223E3C7264663A4465736372697074696F6E207264663A61626F
              75743D22757569643A66616635626464352D626133642D313164612D61643331
              2D6433336437353138326631622220786D6C6E733A64633D22687474703A2F2F
              7075726C2E6F72672F64632F656C656D656E74732F312E312F222F3E3C726466
              3A4465736372697074696F6E207264663A61626F75743D22757569643A666166
              35626464352D626133642D313164612D616433312D6433336437353138326631
              622220786D6C6E733A786D703D22687474703A2F2F6E732E61646F62652E636F
              6D2F7861702F312E302F223E3C786D703A437265617465446174653E32303232
              2D30382D31305431363A34393A34322E3534323C2F786D703A43726561746544
              6174653E3C2F7264663A4465736372697074696F6E3E3C7264663A4465736372
              697074696F6E207264663A61626F75743D22757569643A66616635626464352D
              626133642D313164612D616433312D6433336437353138326631622220786D6C
              6E733A64633D22687474703A2F2F7075726C2E6F72672F64632F656C656D656E
              74732F312E312F223E3C64633A63726561746F723E3C7264663A53657120786D
              6C6E733A7264663D22687474703A2F2F7777772E77332E6F72672F313939392F
              30322F32322D7264662D73796E7461782D6E7323223E3C7264663A6C693E4561
              736F6E6C697528E58A89E5AE9CE6BE8433353036293C2F7264663A6C693E3C2F
              7264663A5365713E0D0A0909093C2F64633A63726561746F723E3C2F7264663A
              4465736372697074696F6E3E3C2F7264663A5244463E3C2F783A786D706D6574
              613E0D0A20202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              20202020202020200A2020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              202020202020202020202020200A202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020200A20202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              20202020202020202020202020202020202020202020200A2020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              202020202020202020202020202020202020202020202020202020200A202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              200A202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020200A20202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              20202020202020202020200A2020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              202020202020202020202020202020200A202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020200A20202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              20202020202020202020202020202020202020202020202020200A2020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              202020202020202020202020202020202020202020202020202020202020200A
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              202020200A202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020200A20202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              20202020202020202020202020200A2020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              202020202020202020202020202020202020200A202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020200A20202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              20202020202020202020202020202020202020202020202020202020200A2020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              20200A2020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              2020202020202020202020202020202020202020202020202020202020202020
              202020202020200A202020202020202020202020202020202020202020202020
              202020203C3F787061636B657420656E643D2777273F3EFFDB00430007050506
              050407060506080707080A110B0A09090A150F100C1118151A19181518171B1E
              27211B1D251D1718222E222528292B2C2B1A202F332F2A32272A2B2AFFDB0043
              010708080A090A140B0B142A1C181C2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A
              2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A
              2AFFC000110801F002FE03012200021101031101FFC4001F0000010501010101
              010100000000000000000102030405060708090A0BFFC400B510000201030302
              0403050504040000017D01020300041105122131410613516107227114328191
              A1082342B1C11552D1F02433627282090A161718191A25262728292A34353637
              38393A434445464748494A535455565758595A636465666768696A7374757677
              78797A838485868788898A92939495969798999AA2A3A4A5A6A7A8A9AAB2B3B4
              B5B6B7B8B9BAC2C3C4C5C6C7C8C9CAD2D3D4D5D6D7D8D9DAE1E2E3E4E5E6E7E8
              E9EAF1F2F3F4F5F6F7F8F9FAFFC4001F01000301010101010101010100000000
              00000102030405060708090A0BFFC400B5110002010204040304070504040001
              0277000102031104052131061241510761711322328108144291A1B1C1092333
              52F0156272D10A162434E125F11718191A262728292A35363738393A43444546
              4748494A535455565758595A636465666768696A737475767778797A82838485
              868788898A92939495969798999AA2A3A4A5A6A7A8A9AAB2B3B4B5B6B7B8B9BA
              C2C3C4C5C6C7C8C9CAD2D3D4D5D6D7D8D9DAE2E3E4E5E6E7E8E9EAF2F3F4F5F6
              F7F8F9FAFFDA000C03010002110311003F00FA468A28A0028A28A0028AF3BBAF
              8C3041A8DE5ADAF823C69A8A5A5D4D6A6EAC74912C323C52346C51C3F237291F
              E155AE7E334EB6B2B59FC34F1E4B701098A39746288CD8E0330662A09EA4038F
              43D2803D368AF2DD3FE22FC49D4EC23BBB6F8417091499DAB73AE456F20C1239
              8E44561D3B8191CF4351EABE31F8C735AAAE87F0BECECEE3782D25E6B505C215
              C1C80AAF190738E727A1E39C800F56A2BCA74CF88BF132E967B66F8591DC5E69
              EE96F7AD16BF0468263124876AB03805645603736376324835A56DE34F890D75
              12DE7C27922B72E04B245E21B57755CF2554ED0C40E80919F51D6803D128AE6F
              FE128D5FFE844F107FDFFD3FFF0092A8FF0084A357FF00A113C41FF7FF004FFF
              00E4AA00CFF18FC52D0BC11AD5AE9BAB5BEA53C92C4279E5B3B432C7670962A2
              494E785F95CFCA18E11B8E99ED2BC17E26F85BC6BE32F105EEA3A2F86356B34B
              8D1D34EF2A7BAB1C390F3331205C103EFA00DC900C830090C3D5BFE128D5FF00
              E844F107FDFF00D3FF00F92A803A4A2B9BFF0084A357FF00A113C41FF7FF004F
              FF00E4AACDD5FC5FE3387C9FEC1F867A85EE7779BF6DD5ACADB674C6DDB249BB
              3CE738C6075CF001DB515E6DFF0009B7C4EFFA247FF972DB7FF135A7A478BFC6
              7379DFDBDF0CF50B2C6DF2BEC5AB595CEFEB9DDBA48F6E38C6339C9E98E403B6
              A2B9BFF84A357FFA113C41FF007FF4FF00FE4AA3FE128D5FFE844F107FDFFD3F
              FF0092A803A4A2B9BFF84A357FFA113C41FF007FF4FF00FE4AA3FE128D5FFE84
              4F107FDFFD3FFF0092A803A4A2B9BFF84A357FFA113C41FF007FF4FF00FE4AA3
              FE128D5FFE844F107FDFFD3FFF0092A803A4A2B9BFF84A357FFA113C41FF007F
              F4FF00FE4AA3FE128D5FFE844F107FDFFD3FFF0092A803A4A2B9BFF84A357FFA
              113C41FF007FF4FF00FE4AA3FE128D5FFE844F107FDFFD3FFF0092A803A4A2B9
              BFF84A357FFA113C41FF007FF4FF00FE4AA3FE128D5FFE844F107FDFFD3FFF00
              92A803A4A2B9BFF84A357FFA113C41FF007FF4FF00FE4AA3FE128D5FFE844F10
              7FDFFD3FFF0092A803A4A2B9BFF84A357FFA113C41FF007FF4FF00FE4AA3FE12
              8D5FFE844F107FDFFD3FFF0092A803A4A2B9BFF84A357FFA113C41FF007FF4FF
              00FE4AA3FE128D5FFE844F107FDFFD3FFF0092A803A4A2B9BFF84A357FFA113C
              41FF007FF4FF00FE4AA8AE7C55AF2DACAD67E00D725B8084C51CB776088CD8E0
              330B962A09EA4038F43D2803A9A2BCDBFE136F89DFF448FF00F2E5B6FF00E26A
              5B6F1A7C486BA896F3E13C915B97025922F10DABBAAE792AA76862074048CFA8
              EB401E8945737FF0946AFF00F4227883FEFF00E9FF00FC9547FC251ABFFD089E
              20FF00BFFA7FFF002550074945737FF0946AFF00F4227883FEFF00E9FF00FC95
              47FC251ABFFD089E20FF00BFFA7FFF002550074945737FF0946AFF00F4227883
              FEFF00E9FF00FC9547FC251ABFFD089E20FF00BFFA7FFF002550074945737FF0
              946AFF00F4227883FEFF00E9FF00FC9547FC251ABFFD089E20FF00BFFA7FFF00
              2550074945737FF0946AFF00F4227883FEFF00E9FF00FC9547FC251ABFFD089E
              20FF00BFFA7FFF002550074945737FF0946AFF00F4227883FEFF00E9FF00FC95
              47FC251ABFFD089E20FF00BFFA7FFF002550074945737FF0946AFF00F4227883
              FEFF00E9FF00FC9547FC251ABFFD089E20FF00BFFA7FFF002550074945737FF0
              946AFF00F4227883FEFF00E9FF00FC9547FC251ABFFD089E20FF00BFFA7FFF00
              2550074945709AAF8C7C750DD2AE87F0BEF2F2DF602D25E6B5676EE1B27202AB
              C808C639C8EA78E3268FFC26DF13BFE891FF00E5CB6DFF00C4D007A4D15C4E91
              E2FF0019CDE77F6F7C33D42CB1B7CAFB16AD6573BFAE776E923DB8E318CE727A
              639D2FF84A357FFA113C41FF007FF4FF00FE4AA00E928AE6FF00E128D5FF00E8
              44F107FDFF00D3FF00F92A8FF84A357FFA113C41FF007FF4FF00FE4AA00E928A
              E6FF00E128D5FF00E844F107FDFF00D3FF00F92A8FF84A357FFA113C41FF007F
              F4FF00FE4AA00E928AE6FF00E128D5FF00E844F107FDFF00D3FF00F92A8FF84A
              357FFA113C41FF007FF4FF00FE4AA00E928AE6FF00E128D5FF00E844F107FDFF
              00D3FF00F92A8FF84A357FFA113C41FF007FF4FF00FE4AA00E928AE6FF00E128
              D5FF00E844F107FDFF00D3FF00F92A8FF84A357FFA113C41FF007FF4FF00FE4A
              A00E928AE6FF00E128D5FF00E844F107FDFF00D3FF00F92A8FF84A357FFA113C
              41FF007FF4FF00FE4AA00E928AE6FF00E128D5FF00E844F107FDFF00D3FF00F9
              2A8FF84A357FFA113C41FF007FF4FF00FE4AA00E928AE6FF00E128D5FF00E844
              F107FDFF00D3FF00F92A8FF84A357FFA113C41FF007FF4FF00FE4AA00E928AE6
              FF00E128D5FF00E844F107FDFF00D3FF00F92A8FF84A357FFA113C41FF007FF4
              FF00FE4AA00E928AE4B50F1678962B091F4AF879AC5CDD8C79715CDFD8C31B72
              339759DC8E327EE9C9E38EA30FFE136F89DFF448FF00F2E5B6FF00E26803D268
              AF3FD3FC67F1065BF8D355F857716D6873E64B6DAF5A4D22F071846280F381F7
              86073CF43B9FF0946AFF00F4227883FEFF00E9FF00FC95401D2515CDFF00C251
              ABFF00D089E20FFBFF00A7FF00F25561EA7E2CF882BE24D3868DF0F2E1F44E7F
              B41AEEFED16E4E781E5059CA8DBD7E6277671F2637100F40A2B9BFF84A357FFA
              113C41FF007FF4FF00FE4AA3FE128D5FFE844F107FDFFD3FFF0092A803A4A2B8
              9D5FC5FE3387C9FEC1F867A85EE7779BF6DD5ACADB674C6DDB249BB3CE738C60
              75CF199FF09B7C4EFF00A247FF00972DB7FF0013401E93457925978BFE3526AB
              7126A1F0CF4F9F4F6DDE44106AD0C52C7F37CBBA432306C2E41C22E4F3C74AD2
              FF0084DBE277FD123FFCB96DBFF89A00F49A2B8DD2BC5BE2C9AD59B5CF871AA5
              9DC6F2163B3D4ACAE10AE0609669A320E73C60F41CF3817BFE128D5FFE844F10
              7FDFFD3FFF0092A803A4A2A8E89AAC1AF681A7EAF669225BEA16B1DD44B2801D
              55D4300C0123383CE09ABD400514514005145140051451401CCF807FE45EBCFF
              00B0DEABFF00A5F3D74D5CCF807FE45EBCFF00B0DEABFF00A5F3D74D40051451
              401CDF85FF00E462F19FFD86A3FF00D37D9D7495CDF85FFE462F19FF00D86A3F
              FD37D9D749400514514005145140051451400514514005145140051451400514
              5140051451400514514005145140051451400514514005145140051451400514
              5140051451400514514005145140051451400514514005145140051451400514
              5140051451400514514005145140051451400514514005145140051451400514
              5140051451400514514005145140051451400514514005145140051451400514
              514005145140051451401CDFC38FF9259E14FF00B02D9FFE884AE92B9BF871FF
              0024B3C29FF605B3FF00D1095D2500145145001451450014514500733E01FF00
              917AF3FEC37AAFFE97CF5D35733E01FF00917AF3FEC37AAFFE97CF5D35001451
              4500737E17FF00918BC67FF61A8FFF004DF675D25737E17FF918BC67FF0061A8
              FF00F4DF675D2500145145001451450014514500145145001451450014514500
              1451450014514500145145001451450014514500145145001451450014514500
              1451450014514500145145001451450014514500145145001451450014514500
              1451450014514500145145001451450014514500145145001451450014514500
              1451450014514500145145001451450014514500145145001451450014514500
              145145001451450014514500737F0E3FE4967853FEC0B67FFA212BA4AE6FE1C7
              FC92CF0A7FD816CFFF004425749400514514005145140051451401C1FC15D567
              D7BE15586AF7891A5C6A1777B752AC4084567BB9988504938C9E324D7795E6BF
              B3DFFC90BD03EB73FF00A532D7A550014514500737E17FF918BC67FF0061A8FF
              00F4DF675D25737E17FF00918BC67FF61A8FFF004DF675D25001451450014514
              5001451450014514500145145001451450014514500145145001451450014514
              5001451450014514500145145001451450014514500145145001451450014514
              5001451450014514500145145001451450014514500145145001451450014514
              5001451450014514500145145001451450014514500145145001451450014514
              50014514500145145001451450014514500145145001451450014514500709F0
              4F559F59F82DE1ABABA48D1E3B536A046081B6176854F24F256304FBE7A74AEE
              EBCDBF67CFF9213E1EFF00B79FFD2996BD26800A28A2800A28A2800A28A2803C
              D7F67BFF009217A07D6E7FF4A65AF4AAE1FE0F691FF08FFC34B6D1BCFF00B47F
              675F5FDA79DB36799E5DE4C9BB6E4E338CE326BB8A0028A28A00E6FC2FFF0023
              178CFF00EC351FFE9BECEBA4AE6FC2FF00F23178CFFEC351FF00E9BECEBA4A00
              28A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A00
              28A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A00
              28A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A00
              28A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A00
              28A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A00
              28A28A00F36FD9F3FE484F87BFEDE7FF004A65AF49AF36FD9F3FE484F87BFEDE
              7FF4A65AF49A0028A28A0028A28A0028A28A00E67C03FF0022F5E7FD86F55FFD
              2F9EBA6AE67C03FF0022F5E7FD86F55FFD2F9EBA6A0028A28A00E6FC2FFF0023
              178CFF00EC351FFE9BECEBA4AE6FC2FF00F23178CFFEC351FF00E9BECEBA4A00
              28A28A0028A28A002BCFF47F89F26A1E2AD1F4CBBD17ECD69AECDA8C3A75C2CE
              E65CD9B956F3A17890C5B82920658838040E48F40ACDB2F0E687A76AB71A9E9F
              A369F69A85CEEF3EEE0B544965DCDB9B738196CB004E4F24668034A8A28A0028
              A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028
              A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028
              A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028
              A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A00E7FC4DE22B
              BD1AF746D3F4BD3A3BEBED5EE9E0885C4D2410A0485E56669562900384C05C65
              B248E15B07817C59078E7C13A6F88ED6DA4B44BD46260918318D95D91864751B
              94E0F1918381D0696ABA2E95AF5AADAEB9A659EA56E8E2458AF2DD664560080C
              03023382467DCD5AB6B682CED62B5B386382DE1411C51448152350301540E000
              0600140125145140051451400514514005145140051451401CB7C31B682D7E13
              F8563B5863851B49B690AC681416789599B03B966249EE493DEBA9AE6FE1C7FC
              92CF0A7FD816CFFF0044257494005145140051451400515C8F8B3E26F86BC1EE
              D6F7D74F79A8860834EB1512CE5C80CA84642A3329DCAAECA5C03B431E2B98B2
              F8FF00A1DEC6922785BC5AB0C8AAE929D2C32BA9239055CE460EEFA038C9C020
              1D7F807FE45EBCFF00B0DEABFF00A5F3D74D5C8FC31BD8F52F05B5F409224575
              AAEA5322CC851D435F4E40653C83CF23B575D40051451401CDF85FFE462F19FF
              00D86A3FFD37D9D7495CDF85FF00E462F19FFD86A3FF00D37D9D749400514514
              0051451400514514005145140051451400514514005145140051451400514514
              0051451400514514005145140051451400514514005145140051451400514514
              0051451400514514005145140051451400514514005145140051451400514514
              0051451400514514005145140051451400514514005145140051451400514514
              0051451400514514005145140051451400514514005145140051451400514514
              01CDFC38FF009259E14FFB02D9FF00E884AE92BC6FC23F18B46D1BE1C68168DA
              2788EEE7B4D2ED6165B5D29D83958D5495624291C6739E456A697FB40F83AF75
              3163AA26A3A0485C441B568A38B0E76E032ABB32021B3BD942000E585007A851
              451400547732BC16B2CB14125CBC68596188A87908190ABB885C9E83240F522A
              4A2803E56F0AE973FC49F89AFA7EA56575696D37DAAEF509ADCC11C814CB2210
              482498CC91A663E54C924AD8C1047A627ECD7E085812232EA2C1028DC4C1B8E3
              B93E573D39F5AE67E07FFC957D53FEC1773FFA7296BE83A6C48E3BE1669B068D
              E051A65A6EFB3D96A7A8DBC5BB19DA97B328CE001D07602BB1AE67C03FF22F5E
              7FD86F55FF00D2F9EBA6A430A28A28039BF0BFFC8C5E33FF00B0D47FFA6FB3AE
              92B9BF0BFF00C8C5E33FFB0D47FF00A6FB3AE92800A28A2800A28A2800A28A28
              00A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A28
              00A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A28
              00A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A28
              00A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A28
              00A28A2800A28A2800A28A2800A28A2800A28A2800A28A2803C37C1DF00FC21A
              CF81741D4EEDAF3ED17BA6DBDC4BB4418DCF12B1C66227A9EE4D66F8FF00E15E
              95F0FB4182FB409352BC8AF75186DAE2C64923F2CF985543281B02302A30C3E6
              DE54E4286CFB07C38FF9259E14FF00B02D9FFE884AC0F8D5FF0022669DFF0061
              BB1FFD1C29ADC4C77C0DBDB9BEF839A1BDDC5708C88F1A3CCEAC2450E70530C7
              0833B429DB8D98002819F41AE07E06FF00C913F0DFFD707FFD1AF5DF52185145
              1401F3E7C0FF00F92AFAA7FD82EE7FF4E52D7D075F3E7C0FFF0092AFAA7FD82E
              E7FF004E52D7D074DEE24733E01FF917AF3FEC37AAFF00E97CF5D35733E01FF9
              17AF3FEC37AAFF00E97CF5D3521851451401CDF85FFE462F19FF00D86A3FFD37
              D9D7495CDF85FF00E462F19FFD86A3FF00D37D9D749400514514005145140051
              4514005145140051451400514514005145140051451400514514005145140051
              4514005145140051451400514514005145140051451400514514005145140051
              4514005145140051451400514514005145140051451400514514005145140051
              4514005145140051451400514514005145140051451400514514005145140051
              451400514514005145140051451400514514005145140051451401CDFC38FF00
              9259E14FFB02D9FF00E884AC0F8D5FF22669DFF61BB1FF00D1C2B7FE1C7FC92C
              F0A7FD816CFF00F4425607C6AFF91334EFFB0DD8FF00E8E14D6E264BF037FE48
              9F86FF00EB83FF00E8D7AEFAB81F81BFF244FC37FF005C1FFF0046BD77D48615
              1DCB4EB6B2B59C71CB701098A3964288CD8E03300C5413D48071E87A54945007
              CCBF083599F47F8AC24D4E1B482DEFEDAEEDDE637981062E249F71CA0EBBE300
              1C65644619C951F4347E27D0268925875CD3648DD4323ADDC64303D0839E4579
              BFC41F822BE20BC9754F0C5E436D7933992E2D7515F3E1B872E4EE0D2093CBC6
              F73808C3B284DCE5B85B2FD9DFC650C70C33DCF860471C414B08239199860039
              6B4CFAF5249F5A7A0B53DBBE1EC8937866E6585D648DF59D5195D4E43037F3E0
              83DC57515C47C20D3E5D23E1B5BE9B7063696CEFAFEDDCC4A02164BC994ED000
              006471803E83A576F48614514500737E17FF00918BC67FF61A8FFF004DF675D2
              5737E17FF918BC67FF0061A8FF00F4DF675D2500145145001451450014514500
              1451450014514500145145001451450014514500145145001451450014514500
              1451450014514500145145001451450014514500145145001451450014514500
              1451450014514500145145001451450014514500145145001451450014514500
              1451450014514500145145001451450014514500145145001451450014514500
              145145001451450014514500145145001451450014514500717F0F75FD1E1F86
              3E178A6D56C6391347B45646B940548853208CF06B9AF8D3E22D3EEBC1F67169
              17FA7DECF0EAF6D24882EC611632246C950D8C2E18E7A26E6E42D79BE81F033C
              4FAB783F49D4B4FB9F0F05BCB382E235B9B584B6D7456C31FB2B1DD83D493CF7
              35D1E83FB3E6B93CAF078AF5DB3B2D34138B7D0E18D5E50CA37067F263006513
              20AB86030402A1A9E82D4EEFE041BAFF008533A225E411C3E5AC8916D90B3328
              91B960546D6DDB86D1BB800E79C0F43AA7A469161A0E936FA66916C96B676CBB
              628933C7392493C92492493924924924D5CA430A28A2800A28A280399F00FF00
              C8BD79FF0061BD57FF004BE7AE9AB99F00FF00C8BD79FF0061BD57FF004BE7AE
              9A800A28A28039BF0BFF00C8C5E33FFB0D47FF00A6FB3AE92B9BF0BFFC8C5E33
              FF00B0D47FFA6FB3AE92800A28A2800A28A2800A28A2800A28A2800A28A2800A
              28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A
              28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A
              28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A
              28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A
              28A2800A28A2800A28A2800A28A28039BF871FF24B3C29FF00605B3FFD1095D2
              5737F0E3FE4967853FEC0B67FF00A212BA4A0028A28A0028A28A0028A28A00E6
              7C03FF0022F5E7FD86F55FFD2F9EBA6AE67C03FF0022F5E7FD86F55FFD2F9EBA
              6A0028A28A00E6FC2FFF0023178CFF00EC351FFE9BECEBA4AE6FC2FF00F23178
              CFFEC351FF00E9BECEBA4A0028A28A0028A28A0028A28A0028A28A0028A28A00
              28A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A00
              28A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A00
              28A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A00
              28A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A00
              28A28A0028A28A0028A28A0028A28A00E6FE1C7FC92CF0A7FD816CFF00F44257
              495CDFC38FF9259E14FF00B02D9FFE884AE92800A28A2800A28A2800A28A2803
              99F00FFC8BD79FF61BD57FF4BE7AE9AB87F83DABFF00C241F0D2DB59F23ECFFD
              A37D7F77E4EFDFE5F997933EDDD819C6719C0AEE2800A28A28039BF0BFFC8C5E
              33FF00B0D47FFA6FB3AE92B9BF0BFF00C8C5E33FFB0D47FF00A6FB3AE92800A2
              8A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A2
              8A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A2
              8A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A2
              8A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A2
              8A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A2
              8A28039BF871FF0024B3C29FF605B3FF00D1095D25713F06B57FEDBF837E19BA
              F23C8F2EC85A6DDFBB3E4130EECE07DEF2F763B671CE335DB500145145001451
              45001451450079AFECF7FF00242F40FADCFF00E94CB5E955E6BFB3DFFC90BD03
              EB73FF00A532D7A550014514500737E17FF918BC67FF0061A8FF00F4DF675D25
              737E17FF00918BC67FF61A8FFF004DF675D25001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450079B7ECF9FF2427C3D
              FF006F3FFA532D7A4D79B7ECF9FF002427C3DFF6F3FF00A532D7A4D001451450
              01451450014514500707F0574A9F41F85561A45E346F71A7DDDEDACAD1125199
              2EE65254900E3238C815DE5733E01FF917AF3FEC37AAFF00E97CF5D350014514
              500737E17FF918BC67FF0061A8FF00F4DF675D25737E17FF00918BC67FF61A8F
              FF004DF675D25001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              45145001451450014514500709F04F4A9F46F82DE1AB5BA68DDE4B53740C6491
              B66769947207216400FBE7AF5AEEEB9BF871FF0024B3C29FF605B3FF00D1095D
              2500145145001451450014514500733E01FF00917AF3FEC37AAFFE97CF5D3573
              3E01FF00917AF3FEC37AAFFE97CF5D350014514500737E17FF00918BC67FF61A
              8FFF004DF675D25737E17FF918BC67FF0061A8FF00F4DF675D25001451450014
              5145001451450014514500145145001451450014514500145145001451450014
              5145001451450014514500145145001451450014514500145145001451450014
              5145001451450014514500145145001451450014514500145145001451450014
              5145001451450014514500145145001451450014514500145145001451450014
              5145001451450014514500145145001451450014514500145145001451450073
              7F0E3FE4967853FEC0B67FFA212BA4AE6FE1C7FC92CF0A7FD816CFFF00442574
              9400514514005145140051451401CCF807FE45EBCFFB0DEABFFA5F3D74D5CAFC
              39B982F3C273DD59CD1CF6F36AFA9C914B13864914DF4E43291C1041C822BAAA
              0028A28A00E6FC2FFF0023178CFF00EC351FFE9BECEAD6B7ADCB6D70BA5E8EB1
              CFAACC81C090131DAC6491E74B820E3208540419082010AAEE957C2FFF002317
              8CFF00EC351FFE9BECEBC4F5FF001C78AC7C40F1ABF85AE2089E2D3AE2E9D750
              8895B686CE592DDBC96F2F32316412A0DDE52B4932956625A803A5F1BDD4778B
              A8E83078DB5B3E3416CB711FF65A5E7976ABB81DA60B405550E719937C816452
              59FE5AC7B7F8856FA25DE97A5F843C5379A96A88E969AAFF006D4B74FE64C0AA
              922D2706567C79EE238648FE645425D9A38DBAEF08F83B43D53413A8EA7649A9
              C3A8DCCD7B125DA8786647918C77261C6CF3648F6B972BBD7CC641B102C6BCB7
              C60F0A699A268969A868D69696DE5FDA161B692D967B7824581E6DC90BE63405
              609119554066944872D1AD007AF7827C5D6FE34F0CC1AAC36D259CCC91FDA2D6
              460C617789250370E1814911811D98642B02A3A0AF3AB3F14E8F1FC6C5D1EDAF
              E35BF9B4F6B5BEB62A773CABB67B6032390237BB62CBF28DC031C98C57A2D001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500145145001
              4514500145145001451450014514500145145001451450014514500737F0E3FE
              4967853FEC0B67FF00A212BA4AE4BE14EA16BA9FC23F0BCF632F9B12699040CD
              B4AE1E2411B8E47674619E87191915D6D0014514500145145001451450079AFE
              CF7FF242F40FADCFFE94CB5E955E6BFB3DFF00C90BD03EB73FFA532D7A550014
              514500737E17FF00918BC67FF61A8FFF004DF67581A968A5BFE125F084B37D96
              1D7A1BBB9D36455F302472A22DC6790772DC4CD2104E0899429C02A9BFE17FF9
              18BC67FF0061A8FF00F4DF675ABABE916FACD9882E1A48A48DC4B6F7309025B7
              9002048848201C120820860595832B10403C22C7E2E5A7C36FB4E83E2B8EE2F9
              E1BBB961736C8A66B97699D9E50A31118DA4F33A3868DBF745094672CBBF105C
              FC73B84D33C3D6DA869FA401203A84B68AC96CE636473212D8690AC9E5AC68C7
              0B3190924058FA5F147C1ED66EBE1DCDE10D0E4D16EED5DE36B6BBBF12C5716C
              632A1599FF007BE7398D7CBC8110551B5576E15747C0BF0DFC55A1D8EA165AE6
              BD691D95EDFB4ED0D8A3B5C087CB4448D6E0796B100B1AA61211B547C850EDD8
              01D0E8F6716A9F1125D7AD61410E9F61269A6ED507FA548F32B3C61BA9109840
              E32374CEB90C8E2BB2AADA769D6BA4E9F15969F17950479C0DC5892492CCCC49
              2CCC4962C492C49249249AB34005145140051451400514514005145140051451
              4005145140051451400514514005145140051451400514514005145140051451
              4005145140051451400514514005145140051451400514514005145140051451
              4005145140051451400514514005145140051451400514514005145140051451
              4005145140051451400514514005145140051451400514514005145140051451
              4005145140051451401E6DFB3E7FC909F0F7FDBCFF00E94CB5E935E6DFB3E7FC
              909F0F7FDBCFFE94CB5E93400514514005145140051451401E73F012DA7B3F82
              BA35ADE43241710C9751CB14A855E36173282AC0F20823041AF46AE67C03FF00
              22F5E7FD86F55FFD2F9EBA6A0028A28A00E6FC2FFF0023178CFF00EC351FFE9B
              ECEBA4AE6FC2FF00F23178CFFEC351FF00E9BECEBA4A0028A28A0028A28A0028
              A2B8DD23E2768DAC6BF61A6436D791A6AAF769A65F3794F6F7C6D98ACBE5B472
              3301C1605D5410383C80403B2A28A2800A28A2800A28A2800A28A2800A28A280
              0A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A280
              0A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A280
              0A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A2800A28A280
              0A28A2800A28A2800A28A2800A2B9FF11F8C2D3C3BA9E93A59B3BCD4753D61E5
              5B3B3B3118771126F918B48E88028C756C9C8C03CE2CF857C4DA778C7C2F63AF
              E8CD2359DEA16412A6D75218AB2B0F50CA41C6471C123068035E8A28A0028A28
              A0028A28A0028A28A0028A28A00F36FD9F3FE484F87BFEDE7FF4A65AF49AE6FE
              1C7FC92CF0A7FD816CFF00F44257494005145140051451400514566F88ECEFB5
              1F0AEAB63A45CFD9350B9B29A1B5B8F3193CA9590847DCBCAE188391C8C71401
              97E01FF917AF3FEC37AAFF00E97CF5D3570FF07ACEFB4EF8696D63ABDCFDAF50
              B6BEBF86EAE3CC67F3655BC983BEE6E5B2C09C9E4E79AEE2800A28A28039BF0B
              FF00C8C5E33FFB0D47FF00A6FB3AE92B9BF0BFFC8C5E33FF00B0D47FFA6FB3AE
              92800A28A2800A28A2800AE7ED3C0BE1BB1F1449E21B5D2A38F5391E497CDDEE
              5124915564916327623B8450CEAA19B9C93939E828A0028A28A0028A28A0028A
              28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A
              28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A
              28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A28A0028A
              2B8FF891E21B8F0F68D60F05FB69B15CDDBC7737B1C4B24904496D34ECC81959
              73FB900E55B82D819C1001D8515F36DAFC57F116B5E21D134BF0EEB9ACCDFDB3
              6AF710CB7CD616FB363CCACAC12CE4FF009E248209EA3A5759E77C4EFF00A0E7
              FE4FDB7FF2B6803D9A8AF9E3C67E39F1F781B468B52D5B56BA9A196E05BAADB5
              EDA33062ACD9F9B4E031853DFD2BA2F0FF00C46B96F145F5AC1AF6A5ADA699A8
              DA69B7B15FDB5B45196B89843BA268A346DC8FD7702A42B003E60EA01ECB4514
              500145145001451450064788BC2FA4F8AAD6D6DF5CB79264B3BA4BBB7315C490
              3C5320215D5E3656046E38E7DFB559D1B46D3BC3DA35B693A2DA4767636A9B21
              8631C28EBD4F24924924E49249249357A8A0028A28A0028A28A0028A28A0028A
              28A0028A28A00E6FE1C7FC92CF0A7FD816CFFF0044257495CDFC38FF009259E1
              4FFB02D9FF00E884AE92800A28A2800A28A2800A28A280399F00FF00C8BD79FF
              0061BD57FF004BE7AE9ABC5747F8CDA4F8624D67449BC3DE23D426B2D7352596
              6D3EC9258B2D792C8006DE0E70E3391572E7F68DF0FD95BB5C5E7857C5D6F0A6
              37492E9D1AAAE4E0649971D4D007AF515E5BA7FED1BF0DAF6C23B8B9D5EE34F9
              5F3BADAE6C656913048E4C6ACBCE33C31E0F63C559FF008683F863FF004337FE
              485CFF00F1BA00E97C2FFF0023178CFF00EC351FFE9BECEBA4AF1BD07E397C3A
              B2D6BC4D3DCF8876457DA9A4F6EDF62B83BD059DB464F11F1F3C6E307078CF42
              2B6FFE1A0FE18FFD0CDFF92173FF00C6E803D268AE057E33783EEECA3B8D26F6
              EAF11DD4071A5DE842BBC07219606C903760772304AE722D7FC2DAF087FCFD6A
              5FF824BDFF00E33401DA515C5FFC2DAF087FCFD6A5FF00824BDFFE33459FC5AF
              08EA2B2B69F73A95D2C321864306897AE11C755388786191C75E6803B4A2B88F
              F85C1E0CDCABF6ED432F2B42A3FB1AF7E691776E41FBAFBC36B64751B4FA1AAE
              FF001BFC011D8C77AFACDCADA4AACD1CE74ABB11B8560AC4379582033053E848
              1DE9D98ECD1DFD15C25CFC67F035946CF79A9DE5BA2A6F669748BC501772AE4E
              62E9B9D467D580EE29CFF193C11149711C9A8DF23DA98D6E15B47BC0612FF703
              0F2BE5DD918CF5ED484B5D8EE68AF3A9FE3DFC36B599A1B9F10BC32AFDE4934D
              BA561DFA18A9CBF1E3E1CB2865D7A5208C8234DBAE7FF2151B8DA69D99E87457
              9B7FC341FC31FF00A19BFF00242E7FF8DD3CFC7EF86822594F88D846CC555FFB
              3AEB048C6403E5F5191F98A047A3515C12FC6CF01347E62EAD7453CA33EE1A4D
              DE3CB18CBE7CAFBA3239E9C8A747F1A7C0B34C9143AA5E49249089D11748BC2C
              D19E0381E572A7D7A50B55743716B74777457087E3478154465B53BC0259BECF
              1E748BCF9E5C91B07EEB96C8231D720D5DFF00859FE19FEF6AFF00F821BEFF00
              E33408EBA8AE2F4CF8B5E10D69A55D1AEB52D41A1546945AE897B298C38CA13B
              61380C06467A8E95A1FF0009E691FF003E7E20FF00C27350FF00E31401D2515C
              2B7C66F03A3157D4AF5596EBEC441D1EF01171FF003C7FD57FACFF0067AFB545
              73F1C7E1FD9A17BBD66E205590C45A5D2EED4070482BCC5D410463D8D0DDB71A
              4DEC77F4579B7FC341FC31FF00A19BFF00242E7FF8DD5DD37E35781358B836FA
              46AB777F3043218ED749BC95820382D85889C64819F53408EF28AE3AEFE2AF85
              34FF0027EDF36A96BF68956087CED0EF93CC91BEEA2E61E58E0E00E4D3A7F8A3
              E18B5B792E2E5F56861890BC9249A0DF2AA281924930E0003BD0075F4570537C
              6DF015BFDA7ED1ABDD45F64DBF68DFA4DDAF93BBEEEFCC5F2E7B67AD697FC2CA
              F0EFF775AFFC27EFFF00F8CD1B0ECD6E7574571937C59F08DB5DDBDADC5CEA71
              5C5CEEF2219344BD579768CB6D530E5B03938E950B7C63F04AA40EDA85F05B98
              FCD818E8F7989538F997F75F30F99791EA3D693692BB0516DD91DCD15E731FC7
              DF86B36EF2BC44EFB002DB74EBA3B4640C9FDD7A903EA4539FE3CFC388D4B49E
              209154752DA6DD003FF215311E894572571F137C37696D2DC5D7F6C4304285E4
              964D02F955140C9624C38000E7350D97C58F096A566977A75C6A7776D26764D0
              6877AE8D83838610E0F208FC2803B3A2B86B8F8C9E09B49BCABAD42FA193CE5B
              7D9268F78A7CD601953062FBC41040EA4106A6B3F8B3E11D41656B0B9D4EE843
              29865306897AFE5C83AA1C43C30C8C83CF340ECCECE8AE427F8A3E18B5B792E2
              E5F56861890BC9249A0DF2AA281924930E0003BD413FC5EF075A9717379A8C26
              3085C49A2DEAED0EC5509CC3C6E6040F520814059B3B6A2B8193E37F8021F37C
              DD62E53C9884D2EED2AEC7968582076FDD70BB9957278C903A9A853E3CFC3891
              4347E209194F42BA6DD107FF0021524D35740D34ECCF44A2B8FB2F8A7E16D46D
              12EB4F9755BBB77242CD06857CE8D8241C110E0E0823EA29B17C57F09CD7B716
              70CFAA49756BB7ED1026877A5E1DC32BB97C9CAE47233D6988ECA8AE3A7F8A9E
              15B5318B9975584CAFB23F3342BE5DED82D819879385271E80FA5410FC62F055
              CCF6905BDFDFCB2DEA97B58E3D1AF19A750BB894022F9805E78CF1CD1E63B3B5
              CEE28AE63FE160E89FF3EFAF7FE13B7FFF00C62B31BE33F81D58AB6A57A08BAF
              B110748BCE2E3FE78FFAAFBFFECF5F6A04775457033FC6DF015B4524973ABDD4
              31C4C52479349BB508C1B690498B821B8C7AF155E3F8FDF0D256DB1788D9CE33
              85D3AE8FFED3A49A7B0DC5ADD1E8D4570B61F19FC0DAA5DB5AE99A9DE5E5C2A7
              9861B7D22F24709903760444E3240CFBD59BBF8ABE14D3FC9FB7CDAA5AFDA255
              821F3B43BE4F3246FBA8B98796383803934C4763457213FC51F0C5ADBC97172F
              AB430C485E4924D06F955140C9249870001DEA8CDF1B7C056FF69FB46AF7517D
              936FDA37E9376BE4EEFBBBF317CB9ED9EB40ECCEF68AE53FE1657877FBBAD7FE
              13F7FF00FC66ABCDF167C236D776F6B7173A9C57173BBC8864D12F55E5DA32DB
              54C396C0E4E3A5023B3A2B838FE35781268619A2D56EDE2B80CD0BAE93785640
              A4062A7CAE7048071D09AAB1FC7DF86B36EF2BC44EFB002DB74EBA3B4640C9FD
              D7A903EA452BABD876695CF46A2BCEDFE3CFC388D4B49E209154752DA6DD003F
              F2156DFF00C2C5D07FE796B9FF0084F5FF00FF0019A623A9A2B8CB2F8B1E12D4
              ACD2EF4EB8D4EEEDA4CEC9A0D0EF5D1B07070C21C1E411F854371F193C136937
              9575A85F43279CB6FB24D1EF14F9AC032A60C5F7882081D4820D03B367735C67
              C43FF8FAF09FFD869BFF00486EA8B3F8B3E11D41656B0B9D4EE8432986530689
              7AFE5C83AA1C43C30C8C83CF35CAFC47F1868DACE9DA5885F5DB6B7B5BC966BB
              BB8F47BC85AD623677319943B43C10D22F3CE3AF406811E7DF12AEE0B3F89D67
              89A682F6EEDAC2D217B79DA29046F7529970558100AA0048E4640EF5A4B0CD04
              CE20D43552250D0E1F54B87FAE3739C3719C8C115E7FE23D30C3E2CF09CD36B5
              7BAADC59CD6B6B2CB7F617304D26FB99A40EC655C631F28F9893B5B030A6BD2E
              E2C04CFB96429F4159D584E70B4373971119CA3683B3FEBD0E23E21D8DB7FC22
              F6EF77797B3A79F2802EF529E64DEB6F314203B91BB705E9F4EF8AF5CBC92F66
              F0E68326AD0A417EFA9692D7314672A929BC837A8E4F00E4753F535E57F146C2
              3B9F0BE9F67F698ED10DFA8F3A6591957114A7908ACDCE31C03D7B0AEAECB538
              2FB559B53B3D5B5ED674ED4756B1B9841D16EDA0B18EDA7124814AC7839F2CA0
              DAB9DDB7767E671AD9A5666D0BA8AB9F42D15C6AFC56F09B5E35A2CFAA1B94CE
              E846877BBD70109CAF939E049193ECEBEA2967F8A9E15B5318B9975584CAFB23
              F3342BE5DED82D819879385271E80FA52343B1A2B84FF85D1E052D6CA353BCCD
              DA97B61FD9179FBE50324A7EEBE61820F1DAA7FF0085B5E10FF9FAD4BFF0497B
              FF00C669D983D373B4A2B8FB7F8A7E16BB87CDB493569E3DCCBBE2D0AF9972A4
              AB0C887A82083E841A97FE1657877FBBAD7FE13F7FFF00C66901D5D15CA7FC2C
              AF0EFF00775AFF00C27EFF00FF008CD1FF000B2BC3BFDDD6BFF09FBFFF00E334
              01D5D15CA7FC2CAF0EFF00775AFF00C27EFF00FF008CD1FF000B2BC3BFDDD6BF
              F09FBFFF00E33401D5D15C84FF00147C316B6F25C5CBEAD0C312179249341BE5
              54503249261C00077A92E3E24F87AD2D65B9BB5D6A08214324B2CBE1FBF55450
              3258930E00039C9A00EAE8A28A0028A28A0028A28A00E6FE1C7FC92CF0A7FD81
              6CFF00F44257495CDFC38FF9259E14FF00B02D9FFE884AE92800A28A2800A28A
              28028EABADE95A0DAADD6B9A9D9E9B6EEE23596F2E1614662090A0B1033804E3
              D8D50B4F1CF84F5098C561E28D16EA455DC521D42272074CE0374E457CF0892F
              C61F89775069D7504D34A6491EEE7912616964B26CF2C478D8FF0038040208C1
              89F0AEF2495D75BFECD0B05B4517FC2516EDE5A05DC7C3F6A49C0C672724FE24
              9F7A6239CD06449B5FF194913ABC6FE27BE6575390C0B0C107B8ADBAE5FC13A6
              FF00633F8934BF344DF61D7AEADFCC1188C3ECDAB9DA385CE3A0E057515AAD8C
              DEE15B7F0F7FE4A9DB7FD816F3FF0047DA56252FC18F0B597857E2425BE9F2DC
              C893693792319E4DD83E75A0E00000FAE327B9E0614B61C773CA3E3CC11CFF00
              1DBC4DE6DDC36DE5C503AF9A1CF9AC2DA2C22ED53F31ED9C0F522B5B543A5C96
              7A35A6B53411C13F8674DF9259BCBDFB4CA783907838E9595F1DDE01F1EBC449
              751AB472FD910C84B6E84791092CB838CE011C8230C78CE08E2EE6DF50D67403
              ABCD2A1B7D3C2DAAABCD23BAA0DBB554312028DFC00477AC654258884A11F5D3
              A5B5B9D547151C2CD54924FA59F5BE96FC4B1A178DB5BD2A3B4D3E1D4DADB4F8
              E4C305823628A5B2C412A49EA4F7AE93C2DE30D775ED7B4BB0B9F103C3F69BB8
              A09916DE1DCC1E68D3284A601DB231C61B1E593D0F1CDAF832FD7573A78B9816
              ED61170A416D9B37153F36321B38E31F88C72B2685ABE9D62DAC49A9796B6531
              28F0CAE644952428ACBD31F381CE72073D78AED960314AEF95AB6FF2DCF35633
              0CE4BDEDFF005D8FA0BC43E03D5744F0AEA1AAC7E30BE964B0D3A4B8646B3B70
              259238A4638F93E552DE59039202B0C92C0AEB7C32D1A1D2BC0F6B751CF34F36
              B2A9AA5D34C57FD74B1A17DA1400172381FAD7CFF707C7777A84BA25D78B2FA6
              F3AD0C92249A94ED1BC64EC2A41EB9F4C6315BBE0BF1CF8D7C3FE0F8058CBA6D
              ED9C93AA43FDA2D3492440B2C4A8B860150100803A64FD294B038A6EDC8FAFE1
              6BFDD74358CC3A57E65D3F1DBF22EFC498E197C152ADCCFE420F10DFB07D85B2
              C25BB2AB81FDE60173DB39ED56FC2369E269BC2FF0F6F7C0D67F6CD6F4D8353B
              A8226789632BF6A8E3712798CB9529232FCA43658118C64715A84B3DCF8DB4BB
              0F17DCC26CAFB565BCBC892E254B78E396625F1B9F083E6939001039DC726BD8
              FC21E2BF0A7802EF43BBD42FD2CF44F275BB5B2963592E1597FB422318050312
              36213B8F5C75E6B2AF19427C93566B4FC0ECF6F0AF18CE1B58E2FE20E83AA683
              E1BBD8B55D1EE34C126992B47E6FD9555CFDB6C73B23B66D88071901541273C9
              2C6B9AF017956BE09F15335C44D1ACFA43BC832AABFE91920EE03EEF209E9907
              048C1AF4EF8D3E2DD17C7BE0AB9D5BC2974F7F61A7D9496D73706DE48952592E
              EC9910798ABB8ED8DC9DB9C719C6E5CF83F87AD6FB55B6BDD274FB8F27CF5592
              65691D525556E370538241208054F5272302A214E55A5ECE0AEDE9F80BDB4687
              EF65B23A08FC2F7FE3FF00889A9D9F866CC6AD22DB2CF882E2340142C6A5833B
              05382C06327AF4ACFB3FF8F0B7FF00AE4BFCAA3B4D3BC47E15B2B9D6F48D624D
              39A32D6F24963752452B012EC232A065772838CF614F3E0DD722BB8AD05FC219
              E2675C4CFB4052A08FBBFED8ADE9E5B898C54141E897E3A2FBEC615B31C3D4A9
              2A8E4B56FF00CDFDD720F0B69DA6DD466E6E26BE4BCB798320B584C8140C1527
              08DDC1EBE954751585341B75B4767806A3762266EACBB61C13D3B55C7F055F0B
              613C7716ED1F9E2DCEE2C0EFF33CB3C60F1BB3CFA738078A0F83F543AA2E8E6E
              6DCB2C26E94798DB002C14E3E5EA703B7402B4797E295BDC7ADBF1D8C963B0CE
              FEFAD3F43B3D2EEEDAE340923B7B88A5787C2D76B22A3862876C3C103A743F95
              607C2EB9365E36105B4126A335DDA3471476C55496C2C84132140301181E7A8E
              33C1AA579777C9A0596AF1D9E9B696F25AAE9A56CBCC85E755DDB9A50AC03B36
              DF9893CE178AE87C1B058E95F12F48BEF0EAB6AB676F6F7533ADBB289B679B3C
              285FCC751BCA989B184F9587CB9C93E7CA954C351E54B5B36BCEFAA3D49E2A9E
              32A26F457B3F95933A7B0B19C45697569346D6FF00DAF68F711C0E15807BC531
              34A1D48946063CF8DCEF6E0990217AF63AF20B9BAD1AF2FB499ECAD2DB4C5FED
              7B44B6B3583CE08FE7C659159702D26DA33246415381B4962E6BD7EB9301294E
              9C9CBBBFEBFAD8F360A093E4B5AEF66DA7F7EB1F38F47B6963CE7F677FEDCBFB
              FF00123E837969A6ECB4D2D255D4B4E79CBED81D415DB347B41C13939C8653C7
              7F70FB0F8DFF00E861F0FF00FE0867FF00E4CAF24FD990C8751F14F9CAAADF62
              D23015B70C7D9DF07381C91824763C64F5AF7FAED3A8F86AE22947ED0170D797
              5049749E2CD9208E36412B1BA6DCEAA4B05504742C48DC3EF7244DF112EED9EC
              AF6D52E226B84D76E99E10E0BA8F3A6E48EA3A8FCEB2BC693C769F183C5372F2
              491490EB774F134648656172791820E40C91C8E40E6A793C2DA93EA8CB790699
              757375E65C34B34F7049C150D939C924BE7272793CF4ADA9E06AE2EDC8BE169F
              E8BEF14B1F4F071929FDA4D7E4DFE41AF7816FBC2DAD1D3BC47A4B585C35BA4E
              9135C2C9956675DD94761D508C673C5773F00E1483E295DC712ED45D124C0CE7
              FE5BC75C5DECDAF6BAB7FA9EAB2437D2E9C5EDE496EAFAEE49088F2D852D21F9
              7E6240C8E49A8EDB40F14697AE07D2F58FECFBAB8864224B4BB99088C32E5370
              1BB1965E093D2BAFFB37136BC637F4BFA75B7538BEBF42F693B7F57E87D09F16
              FF00D57837FEC6BB1FFD9EB4FC73FF0024F3C47FF60ABAFF00D14D5F39EADA97
              8A24D1FF00B4754BD1770D8DCEF8D64D4AF1DA3952431875064C020E70783835
              D4DF7C40F1DEA56F73E15BD5D0A57BAD35966B9F2E50CC8C0C6CD90701F92785
              C7B76A8A9976214924AF7B7E3B6F6DECC70C75069B6ED6BFE1BFDD7470FF0013
              561FF8581ADBCAD990790B1A0620E4C484B7DD20800118C839652320115F5C57
              C631DCDBEB10EBB7BABB29BC4B146B6692E1CB3CBF6885782CC4B1F28BF1CF03
              38E335F67570557CD2B9E84E5CF272EE7887ED111413EA1E128AF2E7ECB6F24B
              70B2DC7965FCA426105B68E5B039C0EB8A6EAD776D65A0780E4BCB88ADD0E84E
              A1A570A09DB6BC64FD2A7FDA264B14B4D296F61792792DEE56CD94F11CBE6DB1
              2CDC8E3CB120EFCB0E3B8F26820D5B5ED063BDB99A3BAB2D251E34B7BAB99980
              55452428DD85180BC2951C0EC29C70D2C541D18F543863238392AD2E82E8BE13
              D66EBC1F75E28B5B398E95697696D737493C6AA0978B08C85B7372E878523247
              A70CD5FF00E41537FC07FF004215B0D6FAF69B141E1C89AD62B2D4A4794DA457
              B76207740ADB9D37E09F9570707EE8F41587A8E9D7D36977B7645B430D85C35B
              CC914D2B1918328CE1C918C91E9DEBD0A997D7A69DD6DBEFD15DF4ECEE79D4F1
              D466D5BAFA77B77EFA1F5DF8F7FE49BF897FEC1375FF00A25AB92F82BFF24834
              5FFB6FFF00A512578D97F1C1CE97A86BD2DFADD45217173AA5D9568C6D564203
              0041DFD0839E735A9E1AF1578BFC21A335BE8DFD91269DF6810A5BDC89984321
              97CB729F36429739EB8C0C8504B02E795E2B96FCBAAE844730C3B76E6DFA96FE
              3222C8BABA3C8B12B788ADC191C12A83EC11F27009C0F604D757FB3DA2C7E02D
              49124595575794091010AE3CA8B91900E0FB806BC77C4D797B3EBB790F8B6E97
              FD3966BFCDBCD33209B6388942B1200CAA47D384C0C8C647B97C1A86C6DB47F1
              2C1A3943A7C5E22BA4B531C9BD4C41630986C9DC36E39C9CD70D784A9BE49AB3
              47A11AD0AD15286D63A8F1CFFC93CF11FF00D82AEBFF004535784F86974DB5F0
              7F8926B5D4ADEED9A5D326BD9A18CC304721BE90E1159136A84D87A601271C70
              3DDBC73FF24F3C47FF0060ABAFFD14D5F29C328D4F5CD4B49F0D235A69BA95C4
              8D0C33CD2281129678D5C2B10C540E33BB9EF59D383A9FBB8AD5E9F794AAAA2F
              DA4B65A9D16A56979E2DF88169A3F85DBFB464BFB230BC16D74AAB3046794A33
              160B81B15F04F551DF1598BA7CFA4493E9B791186E2CEE258258D98314749194
              82549070475071562CF4DD57439EEB53D322B0B2B9D337667B5BABA8E41FBA0C
              7630707957C751DC74A92F343D6AE75068E7162D7576B2CED706EEE998B6E1B9
              896639625F3920E4E735E8D2C9F114E0A0B5B7AF7B76EE71D7CE2857AAEA3D2F
              AF4ED7EFD8FA1FE0D7FC927D2BFEBADD7FE94CB543C37FF259BE217FDC37FF00
              49CD7CFDA76A7E2DD0746BD96D75DBA163A7CEF6ED6A9A85C46A1F78CB2AA32F
              52D9EBDC9C66B7349D5BC5DA378E8CD697168FA8C900B9B869EEEEA58EE94031
              289773E5F6EEC8CF4C0C1ED52B015E51D16BDBD6F6F2E8C878CA317ABD3BFA5A
              FF009A3DC3C71FEBBC39FF006156FF00D24B8AF99FC21A4DCDF78DB49B5F0F48
              752B89D87996D19F29DE3F2834F19DE550A9532A60B6182B6461867ADF18F8C3
              C61AEE970EA9A8BE9F6D0E9934A152C25B888B397F28B1F9F9230C01C8E1DBD6
              9FF03E2B0FF858BE13B88446B7F25F6A4B322C84958859A98FE524E065A5C1EF
              83C9C71854A3528250A91B37AFC9FF00C31D74F114EB53B41DECDFE87B8EB3E1
              9F15B43E03F26E24BD9F486B78F53923BB20B38783CC9F2E4171E5A5C213F7C8
              988C10CD8F9EAF608FFE17E5C5C7DAE1F37FE13064FB2E1FCCDBF6ACEFCEDDBB
              7B7DECE7B639AFB32BE1EF1ADE25AFC4DF12CA8823B9B6D6AF268664775767F3
              FE5190DC6DC33023072792780302D6E747E35BFB3FEC5F1059FDAE0FB4FF0069
              DCFEE3CC1BFF00E3F18FDDEBD39AC5D67C25A9782FC491E9BADE98FA6DDC969E
              788DA64977217DA0E51987546E339AA5FD8F79737B64F756D61753EB1BEE1259
              E69CB0F9439DC77672739CF2724F35A17B26BBAEFDBB55D55E0BE9B4DF32D9E5
              BBBEBB924DB1FCC554B484EDC9240C8E49AEAA3955687335D5B7D7B27DBB5999
              62B36A559C6FF6525D35B36BBF7D0EDFE04FFC957BCFFB023FFE8F8EBD27E2DF
              FAAF06FF00D8D763FF00B3D7CFB6BE1FF12E97AC17D335A3A7DECF13ED6B4BA9
              5311065DC85FEF632C981CE7049390336756D4BC51268FFDA3AA5E8BB86C6E77
              C6B26A578ED1CA9218C3A83260107383C1C1ADDE5B894A5CF1B5BF4DF6393EBF
              876D72BBDCFA33C73FF24F3C47FF0060ABAFFD14D5F30FC50453F10F57732286
              568008C83B98185724718C0C0EA73C8C679C76F3F8EBC7175A7C9E0EB91A2CDE
              6E946292EA413B48F191E5162E5B990F5CE319E7DAB91F0F9D3AF35DF14BF8B1
              ED5EEADB44BC5B67B9B866F32E940442A64625DF04ED1EC0819008E69E1EB508
              3F691B6BF8A3AE962A9554E1077D9FCB53EB2AF0DFDA3E3596E3C2D1C92A40AE
              D72AD2C8095404C3F31DA09C0EBC027D8D7B95784FED2BFF0032D7FDBD7FED1A
              E38EE6A64E91776D65E06F0549797115BA1B5D49434AE1413F698F8C9FA5715A
              2785358B9F06DD789ED6C653A5DADE476D7376B3A2A825E2C23216DCC32E8785
              2338E783886CACB51D5BC3C974CD04BA7E94250B6B3CF311F7433155070B9EBF
              295C9EB5B4D6FAF69B141E1C89AD62B2D4A4794DA457B76207740ADB9D37E09F
              9570707EE8F415DF4B2CAD3BD64B495BBFA2D9756635F34A5CB1A0FECDFF0024
              DEFD9231F57FF9054DFF0001FF00D0857DAD5F12EA3A75F4DA5DEDD916D0C361
              70D6F324534AC6460CA33872463247A77AEBA0BBF1F5B5C259CDE24B8B992547
              95649356BC180A5411F2B8FEF0EDEBCD6B1CBF1129592EDAEBD745E7BA68E69E
              3A846376FBFE1ABFC0F60F82BFF248345FFB6FFF00A512579D7C644591757479
              16256F115B8323825507D823E4E01381EC09ACCD07C69E2FF027862E5B4F4D21
              F4C865655B69BCE93CA612146299604066E719C7190012D9E6FC4D797B3EBB79
              0F8B6E97FD3966BFCDBCD33209B6388942B1200CAA47D384C0C8C64632C256A1
              794E3A597E3B7E4CE8A38CA33D22EEDDD5BD373D7FF675FF0092797DFF006159
              3FF45455DDF8E7FE49E788FF00EC1575FF00A29AB97F8350D8DB68FE25834728
              74F8BC457496A63937A9882C6130D93B86DC73939AEA3C73FF0024F3C47FF60A
              BAFF00D14D5C2F7373E6CF0C5B5B5B59DAADA5F25E16F1069A6568E3655460F7
              6A00DC0120AAABE703EFE08C835EE55F3C693796B27C42B78F46496DF4A9F5B8
              66B7B791CE5104A7CB0C37104857232727AF3C9CFD0F5D30D8CE6717F13FFE40
              5A7FFD7F8FFD132D7A5FC1AFF924FA57FD75BAFF00D2996BCDBE2659CFA8689A
              7DADA18D6692FB0AD233281FB994E72BCFFF005FAF15E6D61A8F8BB43D22EA6B
              4F105E25869B70F03DA47A84F1A162DC95542BC6E7CF51CE6B5587AB5173C569
              AEBE8AEFF031957A707C927AFF009E8BF13E85B3FF0092C1A97FBD79FF00A4FA
              5559F1C7FAEF0E7FD855BFF492E2BC574EBEF18E8FE2CB5BF86E6CE6BD2B34A4
              DCDE5DCA928DA91B070CE73C7947FED9273F28149E31F1878C35DD2E1D53517D
              3EDA1D326942A584B7111672FE5163F3F246180391C3B7AD6B2CBF134A7CCE3A
              4757F2D58A866187728CB9BAAFCFFE0183E07B6B183E21688FA75F2DCACB0B19
              23DAE1E193ECE7786CA818DE5B1B49E073EFEDD5E31F0FE2B0FEDAF0FDC42235
              BF92FAFD66459092B10B78CC7F2927032D2E0F7C1E4E38F67AE589BCDDD9D07C
              3EFF00913D7FEBFEFBFF004AE6AE9AB99F87DFF227AFFD7FDF7FE95CD5D3572B
              DCD16C145145200A28A28030BC73FF0024F3C47FF60ABAFF00D14D541F49D674
              BF86BF1464D7D2457BEB9D52EAD5A498485ED8DB8588E41380153014E08000C0
              E2AFF8E7FE49E788FF00EC1575FF00A29ABA0F885FF24C7C51FF00607BBFFD12
              F5A440ED68A28AA105145140051451401C3F817C4DA0E9BF0A7C32FA8EB7A75A
              2C3A3D9AC8D3DDC6810F92830727839E2B774FF1AF85757BF8EC74AF12E8F7D7
              72E7CBB7B6BF8A491F0093855624E0027E82BC47C29FB3FAEB3E0DD1754FF848
              6DE1FB769F05C796741B590A6F8D5B1B88CB633D4F26B0FC57F0A2E7E1E5D47A
              8DFEADA5AE9F35F20B6D4C5BADB4F6F2F943660205D9F38278240552F98DC6EA
              623EA5A2B8FF00857E2C93C67F0E34CD5AEAEA0B8BD6531DD1899495914F470B
              C072BB588C01F364000815D8521851451401F3E7C0FF00F92AFAA7FD82EE7FF4
              E52D7D075F3E7C0FFF0092AFAA7FD82EE7FF004E52D7D074DEE247CD1A2FFC8C
              9E35FF00B1A2FF00FF004315B358DA2FFC8C9E35FF00B1A2FF00FF004315B35A
              C76337B85165A85D691AAEB1A969F2F937767E12D567824DA1B63A35B329C104
              1C1038231455597FE662FF00B13358FF00DB7A52D871DCF33F14F86AE3C4BE34
              BEBFF12F8A6C12EE6B08647B89EE6D55A4BA5863531B461A331A641C30524285
              C82C4D724F17F67F86F55D3A7B7BD92786ECC6D3C2CFF66055941CF206783D57
              3CAD335FFECF6D6A5BB92194477E86EA330DF43315DECC7E6080EC3D331B10CB
              DF3C53DF50D3BFE11BD56DAD2EE7B759AECBDBD8940418F72E3736D27200FEF7
              F0FE7BE15A4E4DBB7BB2EDDBCFF4D4C7129B51B2BFBCBBF7F2FF0086357ED907
              FC255E67F66EBBB3EC5B7CADD2F9D9DFF7BEFE7676EB8CF6AADA95D42FE18BB8
              D6C3588D9A7722599A4F257F7C4E1B2E4671C1C8FBDEFCD59FEDCD3BFE12AFB5
              7F6F5D793F62F2FED3E42EEDDBF3B31E5E318E738FC7B556D4B57B19BC31776D
              16B37134CF3BB2DBB42A15C198B024F960F23E6EA39E3DABDDA9529FB3A9EFAD
              A5FCBE5E7F97C8F1A10A9CF4FDD7BC7F9BCFCBF3F9967ED907FC255E67F66EBB
              B3EC5B7CADD2F9D9DFF7BEFE7676EB8CF6AADA6DD429E18B48DAC35891967426
              585A4F25BF7C0E170E0671C0C0FBDEFCD59FEDCD3BFE12AFB57F6F5D793F62F2
              FED3E42EEDDBF3B31E5E318E738FC7B556D3757B187C31696D2EB3710CC93A33
              5BAC2A5500983120F964F03E6EA79E3DA8F694F9E5EFADA7FCBFDDF3FBBF0EA1
              C953917BAF78FF0037F7BCBFAEBD0ABAFC29A9EAC4C227B0F2AD03ECD5656579
              087230A5C9EB9E990386AE83C33A741E2E6F0BF8775EBC92DB4DB39A680BC77B
              01F9E70F2208C08F2A4B4614876724B0DA17A565DEDC787F57F1013A9EB13C96
              BF64555BA684EE471282540541D5370C9040DDDF00575FE0ED0BC38DE2DF0ADA
              E8BA8B5D4579A83DEBB0951A6536D1BB2075E0C6376786405812411815E0E39A
              788938BBEBE5FA687B5834D508A92D6DE7FAEA37C41E1C6B9BBD53C3169E229E
              E74D875FB4025BCB8819B7B5B95998BBECF32453B1446AFC6D60403835CEFF00
              605B785BC48FF67B9BBD6EC26B7CDB5C69AACAD2F237F31B95C29182039FBC87
              1CE05FF8B1A65A693E26D5EDE486EC93A91BD89A49634122DCA23B95046E750F
              1C8B9030BC0624F079DD16F346D3B5859AD752BCB3824B560EEC8AEEAFE6709F
              708C6D0A72075EFDA8C0B4B111949DB5EB6FD74FBC58C8B742496BA74BFE9A93
              6A5750BF862EE35B0D623669DC8966693C95FDF1386CB919C70723EF7BF35A72
              5F5B9D5606FECAF11002090142F36F3F32723F799C0C73F55F6ACCD4B57B19BC
              31776D16B37134CF3BB2DBB42A15C198B024F960F23E6EA39E3DAB4E4F106967
              558251E23BB28B048A64FB3AE549642063CAEF83DBF8474EFF00431A94B9BE35
              B43F93BBF3E9D7F13C2953A9CBF03DE5FCDD9797F5D0ABF6C83FB376FF0066EB
              B9FB6EEDDBA5D98FB4676FDFC6FC71EBBBBE79A3ED907FC255E67F66EBBB3EC5
              B7CADD2F9D9DFF007BEFE7676EB8CF6A3FB734EFECDF2FFB7AEB7FDB7CCF2FC8
              5C6DFB46EDF9F2FAEDF9B19EBC63B51FDB9A77FC255F6AFEDEBAF27EC5E5FDA7
              C85DDBB7E7663CBC631CE71F8F6A6EA52F73DF5F67F93CFCFF00AEA0A9D4F7BD
              C7F6BF9BCBCBFAE8625D4C8DE0CB2885B5FABACE4999CB7D9DB97E17E6DB9E7B
              01D0D69F85A3BDD13C49710E9F796D25CCD09B682E2DB5081103B10558F988C5
              D415E554063C0047439D2EA36BFF0008969F6DF6B7B89A1B8DF258C8988C2E5C
              FDE0037391FC5FC47A76EAB49D0BC0DB6E353B4D6A59BFB393CE30CF7696EF21
              0090230C119DBE5E021CE71EA33F3B8A69B8F2BFB31EDDBCBFE1FB9EEE1934A5
              CCBABEFDFCFF00E1BB1DE5B40BE26D0F4BD6209A286FCADBDC79B19668B7C6EB
              21474571BC060C30C72A49C60E6B7FFB53C53FF410D23FF05B2FFF0024562F82
              ACE3B1F0469314258AB5B2CC4B1E72FF0039FC32C71ED5B958A8ADCD2C937635
              BE0468BFD85E22F135BFDA3CFDD61A5FCDB36FDC49E2E993D7CBCFE38ED9AF68
              AF2CF84FFF00237F893FEBC34FFF00D19775EA759BDCD56C7E7DEAFE1BD7D358
              BE6D4ED3509258A7916FAF2789A40B32B1133348BB830570F9604E719F6AE8A4
              F0FE963558221E1CBB08D048C63FB42E5886400E7CDED93DFF008875ED2F8D7C
              417F6FE3CF16DBDB69D6AC8751BED35AF240E0AA4D732BED2DBC2039DC4123A2
              9EC0D45258DB8D5605FED5F11106090972936F1F32703F779C1CF3F45F6AF6B2
              C842519F3453D63BDBBF9A7F87CCF273194E328F2C9AD25B5FB7935F8FC8CC83
              48B17D2B5A95B46B867B79E758A4132E200AB9018799CEDEF80D9F7AD57F0EE9
              4351850787EE954C521301B85DCE414F981F37185C907241F9C601C1C65416B0
              9D2B5A637FAC031CF3854559364985E0CBF26327F8B38F7C569C9636E355817F
              B57C444182425CA4DBC7CC9C0FDDE7073CFD17DABD2A34E9726B08F4FE4FE67E
              5FD75D0F3EAD4A9CDA4DF5FE6ECBCFFAF533352D22C61F0C5DDCC5A35C43324E
              EAB70D329540262A011E613C0F97A1E79F7AB3FD87A77FC255F65FEC1BAF27EC
              5E67D9BCF5DDBB7E37E7CCC631C633F877AADA95AC29E18BB916FF0058919677
              0229964F25BF7C465B280671C9C9FBDEFC559FB1C1FF00095797FDA5AEECFB16
              EF376CBE7677FDDFB99D9DFA633DEB3F674FDA47DC5B43F97FBDE5D7AFE3D0D3
              DA54E47EF3DE5FCDFDDF3FEBA7539BD1F459B53B8864992E60D33CE54BBD423B
              57992D93237B90BD76A9DD8CE6BEC6F0DEA336B1E15D2753B95459AF6CA1B891
              6304286740C40C927193EB5F24681AC4D1E9E348B993EC9A4DC5D04BABF0273F
              671280ACC563750F8542C1581CED6F7AFABFC1B04D6BE04D06DEE6278668B4DB
              74923914AB2308941041E4107B57CA48FA33C3BE3F585E6A3F126CA2D3ED27BA
              917498D8A41197603CE946703B723F3AE1B4BD2206F0DEA52DEE917325DDBB4A
              BE76F08212A838652E0E41C93F29FE95E9FF00192E6FB46F8A1A56AFA7DAC773
              74D631DADA432C52B195DA49436CD980C543282A4E7F7AB807A8F36B5906ABA3
              6B3A95CDD6A304F3CB3CCD05A2C9F67625777CD85231938393D319F5AF432F8C
              5D47CCAFA3DEDFA9C58D7254D59DB55DFF0042D5F687A745AF697047A0DD451C
              DE77990B4EA5A5C282307CC38C75EA3F1ACDB8D36D1341D6275D2E64920BD68E
              398CA36C2BB946C237F27923383D7AD695F59C0BAF6968352D75D5FCEDD24AB2
              F989851FEAF299E7BE01E3D2B36E2DE21A0EB0E2F354664BD6558DC3F9720DCB
              F349F2E37FD4839038AF5EBD3A69D4B4175FE5FE45D97CF4EBB6A79946736A17
              93E9FCDFCCFCFF003FC8DC93C3FA58D560887872EC23412318FED0B962190039
              F37B64F7FE21D7B577D074D5D3C3AE8970EDF6C09E6ACE31B4CFB7CBC1933903
              E4CE31919CE3E6AB1258DB8D5605FED5F11106090972936F1F32703F779C1CF3
              F45F6AAEF610C7A78DDA9EB85BED83E55F30A60CFF007B2131BF0738CE43F18C
              F15D93A54AF2FDDC7AFF002765E5FD74EA72C6A54B47DF7D3F9BBBF3FEBEE327
              5FD09CEB696BA268D7919FB3891A051E73FDE20B7CACDC741D6BDD7F67EB5B8B
              2F026A36F790496F326AAFBA3950AB2E61848C83CF435E2F79AD4FE15F10B4DA
              6CB75786E2C4C0EDA979AAEAACF93B4FC8C3EE8C11EA7BF4F6DF80F7EDA97826
              FEE1E18E1FF898F96B1C5BB6AAA5BC08A06E24F451D49AF98C7A8AAF3495B5F2
              FD343E8306DBA116DDF4F3FD7527F8EB617F7FF0D1C69CEC045790B4F0A16DD3
              A16D8A800FBC77BC6707FBB9EA057CF1A5E85750EBD6706B5A3DE7973EFDB0C8
              8626970A4F058AF4E0F5AFA7FE2A4B736FF0D752BAB184CF3DABDBDCAA042C31
              1CF1B9240E7680A49F400D7CE7FDBB7BAFDFE856BA841269B696B0C82D1F4F8E
              51218F6EDCA9258B0CC7B723D1B3ED9E11275A09F75F9F9E9F79A621B5466D76
              7F9796A599341D37ECFAB1FEC4B884C2488E579C15B7FDD29CB0121270496E03
              707BF4AB12787F4B1AAC110F0E5D846824631FDA172C4320073E6F6C9EFF00C4
              3AF6AB716700B3D608D4B5D629BB6ABACBB64FDCA9FDE7C9F8738F940EDCD5A9
              2C6DC6AB02FF006AF888830484B949B78F99381FBBCE0E79FA2FB57D5AA74B5F
              723BAFE4FE67E5FD7A1F36EA55D3DF7B3FE6FE55E7FD7A98771A6DA2683AC4EB
              A5CC92417AD1C731946D857728D846FE4F246707AF5AD2FEC3D3BFE12AFB2FF6
              0DD793F62F33ECDE7AEEDDBF1BF3E66318E319FC3BD66DC5BC4341D61C5E6A8C
              C97ACAB1B87F2E41B97E693E5C6FFA90720715A5F6383FE12AF2FF00B4B5DD9F
              62DDE6ED97CECEFF00BBF733B3BF4C67BD71D3A74F997B8BECFF002F79797F5D
              7A1D539CF95FBCFED7F3768F9FF5D3A905FE8D611F86AEEE13499ADA58E5902D
              CC9282A804C405C0727381B3EE919E738F9AAAF867C39E2B3E2FD36CF47B6D4F
              4ED52E59FECD2479B6970A84C85199906426EE370CE719E6AC6A56B0A7862EE4
              5BFD624659DC08A6593C96FDF1196CA019C72727EF7BF15DAFC30D7353F10FC7
              6F0C5F6AFA7C76AE23B88D248E391048BF667703E6620E0481B8ECE3A822BCEC
              C6318CE1CA92F756D6F3ED6FF33D0C0394A136DB7EF3DEFE5DFF00E18F70D7B4
              BF1ACD078344178D3C96B3DB1D69ACE710895C3C25E43F73745B16E0141D4C89
              F21C657E4AD77C35E21B5D6355FED0B6BBBD92D6EA74BABF58E492391D1D848F
              E611C8DCAC493CFAD7DE55F16F8D7C557363E3AF1559C3656859AEEFEC4DC309
              3CCF29EE64723EFEDCE58E0EDAF30F40CF8F48B16B9D014E8D7045D40ED32F9C
              BFE904460E57F79C60F3CEDEBF851069162FA56B52B68D70CF6F3CEB148265C4
              01572030F339DBDF01B3EF4476B09B9D007DBF5802481CB10B2662FDD8388BE4
              E9D8EDCF18A20B584E95AD31BFD6018E79C2A2AC9B24C2F065F93193FC59C7BE
              2BE9234E9DDFB8BAFF002FF247CBE7EBE773E7DCE765EF3FFC9BF99F9FF5E86A
              BF8774A1A8C283C3F74AA6290980DC2EE720A7CC0F9B8C2E483920FCE300E0E3
              2B52D22C61F0C5DDCC5A35C43324EEAB70D329540262A011E613C0F97A1E79F7
              AD57D3A0FED1848D5BC4053CA932CC25F301CA6028D992BD7240C02172464672
              B52B5853C317722DFEB1232CEE04532C9E4B7EF88CB6500CE39393F7BDF8AE8C
              453A4A33B412D25FC9D9797E5F2D6E6142A547285E6DEABF9BBBF3FCFF00C8B3
              FD87A77FC255F65FEC1BAF27EC5E67D9BCF5DDBB7E37E7CCC631C633F877AC48
              74092F35D941B7BBB2D263BD315CDDADB3CEB67186F999B6E776C539201E71D7
              9ADBFB1C1FF095797FDA5AEECFB16EF376CBE7677FDDFB99D9DFA633DEAA596A
              D2C12DD68D3CF2DBE8F3EA0D1DCEA328B82F0ACBF297744750E42A16D8C09386
              18EA2BCCCC21050F76297BCF6E5FD17FC03D0C0CA729EADBF756F7FD5FFC13EA
              DF0DEA336B1E15D2753B95459AF6CA1B8916304286740C40C927193EB5E0DF1F
              AC2F351F89365169F693DD48BA4C6C5208CBB01E74A3381DB91F9D7B8F83609A
              D7C09A0DBDCC4F0CD169B6E9247229564611282083C820F6AF20F8E102C3E3BD
              3EFF00EC697F74F6B6F6D67633DB4AE972DE6CCCE415201643E52EC39CF9C38E
              2BC18EE7B079B69BA2DBFF00C233A84B79A5CF35DC0650640FB3ECECABF75D59
              94E41E4E01EB8EB902F5F687A745AF697047A0DD451CDE77990B4EA5A5C28230
              7CC38C75EA3F1AAB6B20D5746D6752B9BAD4609E796799A0B4593ECEC4AEEF9B
              0A463270727A633EB4FD7204B5BDB47B7D475A77115C387B9122B2158F236E54
              1C13F7B1DBAE3AD7D4538D25868C9C17D9FE5FE6D7A5F5FF0087D0F9F9CAA3C4
              4A2A4FED7F37F2FADB4FF86D4B1A0F8774BD52FE5B1BBD3AE61136A32DBA5C24
              E0181550B84C65B27E5C74239EB566E3C276D61A95D09F466D42D6CD8C4E6C4B
              87676589D4F96D213801D8704FB818CD7413F83E2D0648353B5D42EDA28268A4
              91659589DE665F32666040FF0054594F18DB927BD741743C9F15966E45DD92EC
              C76F29DB767FEFF2E3E87A719F2E55D29A9422BE697F91E9468B716A527F26FF
              00CCF2ABED334E1E14B9BDB6D2664712B79775E7AB205F38A818121CFCBF2E70
              7D73DEAB6BFA139D6D2D744D1AF233F6712340A3CE7FBC416F959B8E83AD7AC4
              9A169134AF24BA5594923B16676B742589EA49C7269BFF0008EE8BFF00407B0F
              FC054FF0A75ABD3A90E55049D976E97BEC96F7FC35B934A8CE9CAEE6DEAFBF5B
              79BDADFE5637FF0067EB5B8B2F026A36F790496F326AAFBA3950AB2E61848C83
              CF435B1F1ABFE4906B5FF6C3FF004A23AE23FE11DD17FE80F61FF80A9FE15897
              9E0AD235BD7A6B778CD9456D6D148AB651C71EE6769012C769CF08B8F4E7D6BC
              E70B6A77295CE3FC2DA44963AF590D674FB9B2B9FB65B4968F3AB45BC89D15D0
              2B0F9B87DDC7236FA135EFF5E3D77F0EADEC3C47A52690DF6A00B5CCF0EA120D
              92246F1FC9F2A1EBBF0720D75BFD971FFD099E1BFF00BFA3FF0091EAE2D21357
              21F8B93430F83E1FB45B2DC2BDD04505D94A318A4DAE31DC1C70720F23DC7969
              B3B09BC33A9DF5B69B37CB7444170641B6242C985237E49C1F43D7AFA755F10F
              C3F2B6911EA30691A4E950D967CD5B390969B7B228E912F43EBEA6A7D5FE1968
              D61A25F5E43737CD25BDBC92A0791304AA9233F274E2B5A7563072BC6F756FC3
              7D53FEBA994E939DACED67FD7533A4F0FE963558221E1CBB08D048C63FB42E58
              86400E7CDED93DFF008875ED99A9691630F862EEE62D1AE219927755B86994AA
              01315008F309E07CBD0F3CFBD7A5FF00C23BA2FF00D01EC3FF000153FC28FF00
              847745FF00A03D87FE02A7F857A35319464A49524AE9FF002E974BFBBD3E470C
              309562E2DD56ECD77D77FEF1E79E01D1B54D3BC7BA5CBA869B776B1B34AA1E78
              19149F25CE32475E0FE55EBB7DAE5BDB4CF6B6DFE957C38FB3C64FC848C8F318
              02231839C9E48076863C562FFC23BA2FFD01EC3FF0153FC29352D9A4F872EFFB
              3923B63142FE424680012107680BD092C471DC9F7AF2D688F45EA5FF00875E35
              D60EAFA2E8CF1D88D3F539AE6E36AC6E65884825B8037EE00E0903EE0E3F3AF6
              4AF966FF00CE8B54B4B7D2EEE7D3DADE26304F6D232C91ED01000C0E71B5C83C
              E4FAD7B37C32D6354D42F353B3D4B519AF61B5B6B5783CF0A5A3DC66046E0033
              711AF2C58F1D793584A3D4D133D028A28ACC6145145006178E7FE49E788FFEC1
              575FFA29AAA5C58EBF63F0C7E25FFC24AF70DE7CDAACDA7F9D7025C5A343FBB0
              BF31D8BC36178C7A0A3E245A4B77F0E75C10DCBDBF956534AC1738955636250E
              08383D78239037065DC8DD5FC42FF9263E28FF00B03DDFFE897AD22076B45145
              50828A28A0028A28A00E6FE1C7FC92CF0A7FD816CFFF004425607C6AFF009133
              4EFF00B0DD8FFE8E15BFF0E3FE4967853FEC0B67FF00A212B03E357FC899A77F
              D86EC7FF00470A6B71325F81BFF244FC37FF005C1FFF0046BD77D5C0FC0DFF00
              9227E1BFFAE0FF00FA35EBBEA430A28A2803E7CF81FF00F255F54FFB05DCFF00
              E9CA5AFA0EBE73F8177B9F8B17E2E217B53369B7291899D32EDF6F95F68C31C9
              C07E07F718F4C13F46537B891F3468BFF23278D7FEC68BFF00FD0C56CD6368BF
              F23278D7FEC68BFF00FD0C56CD6B1D8CDEE15B7F0F7FE4A9DB7FD816F3FF0047
              DA56256DFC3DFF0092A76DFF00605BCFFD1F694A5B0E3B9E7DE38FF92EDF13BF
              EC597FFD26B7AF21B3B9DBE01BFB7FB6DAA6EB807ECCCBFBD7E63E41DDD38FEE
              9E87F0F5EF1C7FC976F89DFF0062CBFF00E935BD790D9DB6EF00DFDC7D8AD5F6
              DC01F6966FDEA731F006DE9CFF0078753F8F560EF79DBF965DFB797FC37731C5
              DB9217FE65DBBF9FFC39D17F68FF00C567E77F6CE97FF20FD9E7ECFDDFFACCED
              C799F7BBF5E9DBBD53D5AFBCCF08DEC3FDABA7CBBAE243E4469FBC7FF482720F
              98783F7BA1E3F3AB9FD9DFF159F93FD8DA5FFC83F7F91BFF0077FEB31BB3E5FD
              EEDD3A77ED54F56B1F2FC237B37F6569F16DB89079F1BFEF13FD208C01E58E07
              DDEA38FCABE8AA7B4F6757D27FCDE5FD6BF33E7E9FB3F694FD63FCBE7FD69F22
              E7F68FFC567E77F6CE97FF0020FD9E7ECFDDFF00ACCEDC799F7BBF5E9DBBD53D
              26FBCBF08D943FDABA7C5B6E233E4489FBC4FF00480724F98381F7BA0E3F3AB9
              FD9DFF00159F93FD8DA5FF00C83F7F91BFF77FEB31BB3E5FDEEDD3A77ED54F49
              B1F33C236537F6569F2EEB88C79F23FEF1FF00D200C11E59E0FDDEA78FCA8FDE
              7B497A4FF9BFBBFD7E7D03F77ECE3EB0FE5FEF7F5F97536342B9FB57C63D15FE
              DB6B798B7907996AB851FBB9783F3373F8F715EC3F057FE49068BFF6DFFF004A
              24AF1ED0ADBECBF18F454FB15AD9E6DE43E5DAB654FEEE5E4FCABCFE1D857B0F
              C15FF9241A2FFDB7FF00D2892BE7732BFB695F7BBEFD977D4FA4C15BEA54EDE7
              DBCBB681F1ABFE4906B5FF006C3FF4A23AF27F175CFD97E396BCFF006DB5B3CD
              BC43CCBA5CA9FDD43C0F9979FC7B1AF58F8D5FF24835AFFB61FF00A511D793F8
              BADBED5F1CB5E4FB15ADE62DE23E5DD36147EEA1E47CADCFE1DCD465B7FAC42D
              DFCFF4D4CF1B6FAACEFF00A7EBA1CF6AD7DE67846F61FED5D3E5DD7121F2234F
              DE3FFA413907CC3C1FBDD0F1F9D6BC9AA675AB67FEDED20E2DE51E608FE55CB4
              7C1FDEF538E39EC7AF6C8D5AC7CBF08DECDFD95A7C5B6E241E7C6FFBC4FF0048
              2300796381F77A8E3F2AD7934BC6B56C9FD83A40CDBCA7CB127CAD868F93FBAE
              A33C71DCF4EFF530F6BCDF287F3F77FD7E47CDCFD9F2FCE7FCBD97F5F9953FB4
              7FE255B3FB674BFF009086FD9B3E6FF8FADDBBFD67DDFE2E9F77BF7A3FB47FE2
              B3F3BFB674BFF907ECF3F67EEFFD6676E3CCFBDDFAF4EDDE8FECEFF8956FFEC6
              D2FF00E421B37EFF009BFE3EB6EDFF0057F77F87AFDDEDDA8FECEFF8ACFC9FEC
              6D2FFE41FBFC8DFF00BBFF00598DD9F2FEF76E9D3BF6A1FB5F73FEDDFE7F3FEB
              F205ECFDFF00FB7BF97CBFAFCCE76F2E777806C2DFEDB6AFB6E09FB32AFEF539
              93927774E7FBA3A8FC7D2BC39FF237E87FF628DBFF00E8C15E6B796DB7C03617
              1F62B54DD7047DA55BF7AFCC9C11B7A71FDE3D07E1E95E1CFF0091BF43FF00B1
              46DFFF00460AF9CC4DEF1BFF002C7BF6F3FF0086EC7D360EDECAADBF99F6EFE5
              FF000E775451457214757F09FF00E46FF127FD7869FF00FA32EEBD4EBCB3E13F
              FC8DFE24FF00AF0D3FFF00465DD7A9D632DCD56C7C73F10E3797C35E298E2567
              77F88376AAAA3258947C002B0E49AE3FB5603FF099DA31F224C4DE4C384F993E
              5EB8E7AFFC04FBD6D7C47FF9157C57FF006502F3FF00407AA526978D6AD93FB0
              74819B794F9624F95B0D1F27F75D4678E3B9E9DFDDCAA0E519DBBC7ACBBFF77F
              5F91E4E69251942FDA5D23DBFBDFA7CCE7E0926FECAD680F125BC61A79F74263
              8F375F2F2CBCE46EE831F8569C935C7F6AC07FE133B463E44989BC98709F327C
              BD71CF5FF809F7AAB6D63BB45F103FF6569EDE55C5C8F319FE6870BD13F77C81
              DB91F856BC9A5E35AB64FEC1D2066DE53E5893E56C347C9FDD7519E38EE7A77F
              4E8D29B82DFA75A9FCCFCFFAE9A9E655A90E7E9D7A43F957F5F9E873FA94931F
              0C5DAB7892DEE54CEF9B358E30D27EF8FCC0839E4FCFC76F6AB3E6CFFF000956
              EFF84AED77FD8B1F6DF2A2DB8DFF00EAF19C67BE7AD1AB58F97E11BD9BFB2B4F
              8B6DC483CF8DFF00789FE904600F2C703EEF51C7E5573FB3BFE2B3F27FB1B4BF
              F907EFF237FEEFFD663767CBFBDDBA74EFDAB3F673F691DF68759FF7BCFF00AE
              9D4BF690E496DBCBA43FBBFD7E7D0C9F0EFF00C935F18FFDB8FF00E8E35F5F57
              C83E1DFF00926BE31FFB71FF00D1C6BEBEAF92A9B47D3F567D64FE18FA7EACE0
              3C67FF002563C03FF711FF00D10B5F3C68AF2AF856FD535D86CD0F999B278D0B
              4DF20E849C8CF4E3D2BE87F19FFC958F00FF00DC47FF00442D7CFF00A059F9DE
              0CD4E6FECEB29F679BFBF99B124788C1F946C3D3A8E473E9D6BD0CB62E536976
              7DFF004D7F4EE79D8F695157EEBB7EBFF0E5BBE9673AF6964F8AED6761E76DB9
              58A20B6FF28EA01C1DDD39ACDB8794E83AC03AEC3229BD62D6C234DD72772FEF
              01CE403D7038E2B7B51D3B67893478FF00B1B4B8FCCF3FF748FF0024B841F7BF
              763A751C1FC2B22E6CF6F86F5D93FB3AC97CAD4193CD56F9E2F9D3E54F93EEF3
              8EA3A9E2BD9C4539A752F7FB5D67FC8BBFEBF3D2C79142716A9EDD3A43F9DF6F
              D3E5ADCD5926B8FED580FF00C26768C7C893137930E13E64F97AE39EBFF013EF
              557CD9FF00B371FF00095DAE3EDB9F2BCA8B39FB47FACEB9C67E7C74C7B56BC9
              A5E35AB64FEC1D2066DE53E5893E56C347C9FDD7519E38EE7A77A9FD9DFF0012
              ADFF00D8DA5FFC84366FDFF37FC7D6DDBFEAFEEFF0F5FBBDBB5764E94F9A5BEC
              FAD4ED1F3FCFE7D0E485485A3B6EBA43BBF2FEBEF3383BBFC49D00CBAB45AA9F
              B4DB7EFA24550BFBEFBBF2923DFF001AF78F843FF1E1E29FFB196EFF009475E1
              B716DF65F89DE1F4FB15AD9E6E2D8F976AD953FBEEA7E55E7F0EC2BDCBE10FFC
              7878A7FEC65BBFE51D7CC6609AAB34FBF9FEBAFDE7D4E09A783835FA7E9A7DC7
              49E39FF9279E23FF00B055D7FE8A6AF15F8BECCBF1734931DF269EDFD9831732
              2A954F9E6ECDC73D3F1AF6AF1CFF00C93CF11FFD82AEBFF453578CFC5887CFF8
              C5A447F6786E776983F7539C237CD375E0FD7A561824DD6825FCCBBF7F2D7EED
              478876C3D46FB3EDDBCF4FBF438EB8967367AC67C576B203BB746228B373FB95
              E9CF19FBBC771EB56A49AE3FB5603FF099DA31F224C4DE4C384F993E5EB8E7AF
              FC04FBD3AE74EDB61AEB7F63696BE56EF995F98BF7087E4FDDFBE7B724FD6ADC
              9A5E35AB64FEC1D2066DE53E5893E56C347C9FDD7519E38EE7A77FAE54A777BE
              EBAD4FE67E7FD7A9F2AEA42CB6D9F487F2AF2FEBD0E5AE1E53A0EB00EBB0C8A6
              F58B5B08D375C9DCBFBC073900F5C0E38AD2F367FF0084AB77FC2576BBFEC58F
              B6F9516DC6FF00F578CE33DF3D6AA5CD9EDF0DEBB27F67592F95A83279AADF3C
              5F3A7CA9F27DDE71D4753C56BFF677FC567E4FF63697FF0020FDFE46FF00DDFF
              00ACC6ECF97F7BB74E9DFB571D3A73E65BFD9EB3EF2FEBF2D6E75D49C795EDF6
              BA43B47FAFCFA195A94931F0C5DAB7892DEE54CEF9B358E30D27EF8FCC0839E4
              FCFC76F6AED6FA0D46E1FE1AA68C6EC5C8B3673F6291D26F296189A5D85086CF
              961F85F98F419240AE4F56B1F2FC237B37F6569F16DB89079F1BFEF13FD208C0
              1E58E07DDEA38FCABD47C19FF23D7C29FF00B07DCFFE910AF3F308B8B57FE55F
              CDDFFBDFA687B395C94A856B77F2EEBB7EBA9E83ACC1E36B887C0684DD89835B
              BEB5F66915079EAF033F985081B3CB17591F709C0E58C62BC3BE217FC8A1E28F
              FB1FAEFF00F45BD7D715F23FC42FF9143C51FF0063F5DFFE8B7AF1D1D6BA9CAC
              724DF69D03FE2A4B752B03ED7F2E3FF43FDD8F95B9E73F77E6C74A20926FECAD
              680F125BC61A79F742638F375F2F2CBCE46EE831F85598AC7375E1A1FD95A79F
              3ADDCED2FC4FFBA07327EEF83DFF008B9FCE8B6B1DDA2F881FFB2B4F6F2AE2E4
              798CFF0034385E89FBBE40EDC8FC2BEA234E777BF5EB3FE48FF5FF0002C7CDCA
              71B2DBA7487F3BFEBFE0DCB524D71FDAB01FF84CED18F912626F261C27CC9F2F
              5C73D7FE027DEB33529263E18BB56F125BDCA99DF366B1C61A4FDF1F981073C9
              F9F8EDED5D049A5E35AB64FEC1D2066DE53E5893E56C347C9FDD7519E38EE7A7
              7C8D5AC7CBF08DECDFD95A7C5B6E241E7C6FFBC4FF00482300796381F77A8E3F
              2AE8C4529A8D4BDF6975A9D9777F9E9DF4B1850A9072A7B6EBA43BBEDFA6BDBA
              879B3FFC255BBFE12BB5DFF62C7DB7CA8B6E37FF00ABC6719EF9EB49A7927C17
              E3C2D3ADC933DA666500093F7EDF30C71CF5E2AEFF00677FC567E4FF0063697F
              F20FDFE46FFDDFFACC6ECF97F7BB74E9DFB554B34F2FC1FE3E4F2D22DB716A3C
              B8CFCA9FE90DC0E0703E82BCEC7425186BDE5D65DBFBDFF0E7AD95CA2EABB7F2
              768F67DBFE18FACEB80F19FF00C958F00FFDC47FF442D77F5C078CFF00E4AC78
              07FEE23FFA216BE721F123D68EE8F9E3457957C2B7EA9AEC36687CCCD93C685A
              6F907424E467A71E9567C4124CD736DBFC496F7E7C8B9C3C71C63CBFDDF2A769
              FE3FBA3F4A6E8167E77833539BFB3ACA7D9E6FEFE66C491E2307E51B0F4EA391
              CFA75AB7E26B1F22EAD07F6569F6DBADEE8ED81F21B116727F76395EA3AF3E9D
              6BEA23097D4D3D768F59FF0037DDFD69A9F3B29C7EB6D79CBA47F97EFF00EB5D
              0F55D62CDF50D0AF6CE12AB25C5BC912163C02CA40CFB73542F6EA3BB8B42D5A
              DD591276DA5DC7DD8A588B056EC32EB10FAE003CF3B72C91C313CB33AC71C6A5
              9DDCE0281C924F6159DA1690B71A25B5BEB16E1AD6DB74705B4BCACD10E22795
              081F3ED19DA7804E480C005F00F6C7D41777B6D610F9B7732C4A4ED5DC7966EB
              B54756271C01C9AD5FF846FC39FF00400D2FFF0000E3FF000AE6F5ED26CF49D6
              A1B8D3ED2DED6DAF62101482258D5654DEE381D4B2B3738E3CBEBC8A064D16B7
              6D3C292C106A12472286474D3AE0AB03C82084E4565EA26EE6BFFB5E917BAA69
              D23C4B14A3FB125983852C57EF27046F6FAE47A5761E155FF8A3B46FFAF083FF
              0045AD6AECA2D7417B1E639F11FF00D0C7A97FE134FF00FC4519F11FFD0C7A97
              FE134FFF00C457A76CA3652E541CCCF31CF88FFE863D4BFF0009A7FF00E22A2B
              9835EBCB49ADAE3C41A93C33234722FF00C23720CA91823217D0D7A9ECA3651C
              A83999C87F6B45FF003EBA97FE0B2E3FF88A21D62CA7BDFB1F98F0DD150CB05C
              C2F0BB8E7955700B0F94F4CF4AEBF65727E208A2B9BAD76C2445924BED3AD2DA
              04600E65792E02B63D14FCE48C90109ED4C45BAC6F13498B1B6831FF001F1751
              8DDFDDD9997A77CF978FC73DB15D3FFC237E1CFF00A00697FF008071FF008555
              D43C25A1DD5AEDB2D3ED34FB943BA2B8B6B745646C11CE31B9482411EFC60E08
              0679A03E66BB3B1E4222283FDD396247B1C6D27F0F6AF59F84A776B3AE7FD7A5
              9FFE87715E61F60B8B0D52F21BE411CFE687280E415DA143038195254E0FE070
              4103D33E101CEB1AF7FD7B59FF00E8571594BE12D6E7A9514515814145145006
              178E7FE49E788FFEC1575FFA29AA83C1AEDBFC35F8A29E2237649B9D51ECBED3
              2171F656B70D179649236609C01C0391C10455FF001CFF00C93CF11FFD82AEBF
              F4535741F10BFE498F8A3FEC0F77FF00A25EB4881DAD14515420A28A2800A28A
              28039BF871FF0024B3C29FF605B3FF00D109581F1ABFE44CD3BFEC3763FF00A3
              856FFC38FF009259E14FFB02D9FF00E884AE6FE394CF0F81F4F78EDA5B8235BB
              225632A08C4B9FE22072401F523381921ADC4CB5F037FE489F86FF00EB83FF00
              E8D7AEFABCEFE034EF37C16D083DB4B00891D11A42844A37B1DEBB58F19247CD
              83953C6304FA2521851451401E0DE38F853E2AD3FC4B36B1E08863D4219AE7ED
              31C50CD1DB5D5AC846084670231180883F88FCB18D8194CA7274FD0BE39ADB45
              04F1EBCAD1C615A57F10D990C4003FB8CDCF5E49F726BE8FA29DD8AC7CAFE058
              EF625F1147ABEFFB7A6BB72B75E648246F346DDF9600063BB3C8001F4AEAAB1B
              45FF009193C6BFF6345FFF00E862B66B55B19BDC2B6FE1EFFC953B6FFB02DE7F
              E8FB4AC4AB3F090EB9FF000B3631E215D3C49FD8F79E5B58B3ED23CEB5E0871C
              1F7CF39E831CA96C38EE721E38FF0092EDF13BFEC597FF00D26B7AF1CB5B1B79
              3C197B78F612BDC473855BA0E0220CA7057764F53FC27AFE5EC7E38FF92EDF13
              BFEC597FFD26B7AF1CB5851BC197B29B9BF5759C010A06FB3B729CB7CBB73CF7
              23A0ADF0A9372BABFBB2EDDBCFFE1FB19E29B51859FDA5DFBF97FC31B7FD87A7
              7FC255F65FEC1BAF27EC5E67D9BCF5DDBB7E37E7CCC631C633F877AADA969163
              0F862EEE62D1AE219927755B86994AA01315008F309E07CBD0F3CFBD59FB1C1F
              F095797FDA5AEECFB16EF376CBE7677FDDFB99D9DFA633DEAB6A56B0A7862EE4
              5BFD624659DC08A6593C96FDF1196CA019C72727EF7BF15EED4A74FD9D4F716D
              2FE5F2F2FCBE47890A9539E9FBCF78FF00379F9FE7F32CFF0061E9DFF0957D97
              FB06EBC9FB1799F66F3D776EDF8DF9F3318C718CFE1DEAB69BA458CDE18B4B99
              746B89A679D15AE16650AE0CC14803CC0791F2F41CF3EF567EC707FC255E5FF6
              96BBB3EC5BBCDDB2F9D9DFF77EE67677E98CF7AADA6DAC2FE18B491AFF00588D
              9A740628564F257F7C0657084671C8C1FBDEFC51ECE9F3CBDC5B4FF97FBBE5F7
              7E1D43DA54E45EF3DE3FCDFDEF3FEBAF436FC33636F61F17F468AD2C25B14304
              8C6395C3927CB9467219BD3D7B57B3FC15FF009241A2FF00DB7FFD2892BC374D
              274FF899A5BD95E5FCAE207225BFB49AE1D72920C08C057618F4E9927B1AF46F
              D9F7C4D712F85F57D3B539E18F4FD1B64B14AF84F29243233EE6E9B4152D93D3
              279C600F0B1E92AB25156D5F6F2EDA1EF6164DE1217777AFE9DF53ACF8D5FF00
              24835AFF00B61FFA511D790F8DAC6DEFFE376BB15DD84B7C82085847138420F9
              510CE4B2FAFAF7AF5EF8D5FF0024835AFF00B61FFA511D790F8DA149FE376BAB
              2DCDFDB0104277D8062E7F7517076A938FFEB54E01275E29ABABF97EBA118B6D
              61E6D3B7DFFA6A737A9691630F862EEE62D1AE219927755B86994AA01315008F
              309E07CBD0F3CFBD69C9E1FD2C6AB0443C397611A0918C7F685CB10C801CF9BD
              B27BFF0010EBDB3352B5853C317722DFEB1232CEE04532C9E4B7EF88CB6500CE
              39393F7BDF8AD392C6DC6AB02FF6AF888830484B949B78F99381FBBCE0E79FA2
              FB57D1469D2E6F816D0FE4EEFCBAF5FC4F02552A72FC6F797F3765E7FD742AFF
              0061E9DFD9BE67F60DD6FF00B6F97E679EB8DBF68DBB31E675DBF2E71D79CF7A
              3FB0F4EFF84ABECBFD8375E4FD8BCCFB379EBBB76FC6FCF998C638C67F0EF47D
              8E0FECDDDFDA5AEE7EDBB76ED97663ED18DDF731BF1CFAEEED9E28FB1C1FF095
              797FDA5AEECFB16EF376CBE7677FDDFB99D9DFA633DE9BA74BDCF717D9FE4F3F
              2FEBA82A953DEF7DFDAFE6F2F3FEBA18975636F1F832CAF12C254B89272AD745
              C147197E02EEC8E83F8474FCFD3FC39FF237E87FF628DBFF00E8C15E61750A2F
              832CA51737ECED3906170DF675E5F95F976E78EC4F535D845ACC9A4FC41F0DA5
              B5CFDB6DA4D3E0B0593EC525AF990B390A712649E76B6E5C038C71CD78388497
              2D95BDD8F6EDE5FF000FDCFA1C2B6E954BBFB4FF003F3FD343D31E0B61AF4370
              6D18DD9B69105D2AFCAA8190EC63EA490403E8D8C739BB4515CE0757F09FFE46
              FF00127FD7869FFF00A32EEBD4EBCB3E13FF00C8DFE24FFAF0D3FF00F465DD7A
              7C57304F24D1C134723DBB88E654704C6DB436D603A1DACA707B303DEB196E6A
              B63E3BF88FFF0022AF8AFF00ECA05E7FE80F58B2787F4B1AAC110F0E5D846824
              631FDA172C4320073E6F6C9EFF00C43AF6DAF88FFF0022AF8AFF00ECA05E7FE8
              0F58B258DB8D5605FED5F11106090972936F1F32703F779C1CF3F45F6AF6B2C8
              C24A7CD14FE1DF97BF9A7F87CCF2F3294A2E1CB26B47B5FB7935FD6C6641A458
              BE95AD4ADA35C33DBCF3AC52099710055C80C3CCE76F7C06CFBD69C9E1FD2C6A
              B0443C397611A0918C7F685CB10C801CF9BDB27BFF0010EBDB320B584E95AD31
              BFD6018E79C2A2AC9B24C2F065F93193FC59C7BE2B4E4B1B71AAC0BFDABE2220
              C1212E526DE3E64E07EEF3839E7E8BED5E952A749C3582E9FC9FCCFCBFAF43CD
              AB52A736937D7F9BB2F3FEBD4CCD4B48B187C317773168D710CC93BAADC34CA5
              50098A8047984F03E5E879E7DEACFF0061E9DFF0957D97FB06EBC9FB1799F66F
              3D776EDF8DF9F3318C718CFE1DEAB6A56B0A7862EE45BFD624659DC08A6593C9
              6FDF1196CA019C72727EF7BF1567EC707FC255E5FF00696BBB3EC5BBCDDB2F9D
              9DFF0077EE67677E98CF7ACFD9D3F691F716D0FE5FEF7975EBF8F434F695391F
              BCF797F37F77CFFAE9D4ABE1DFF926BE31FF00B71FFD1C6BEBEAF8EF487117C3
              BF1088AEBE79E48166B7FB148F8557051BCE07626496E181CEDE3AD7D3FF000F
              3C4171E28F87FA4EAF7AB8B99A22B31C8F9DD18A33F0001B8A96C01C671DABE5
              AA6D1FEBA9F513DA3E9FAB317C67FF002563C03FF711FF00D10B5F3C68BA6DA5
              CF856FEEA7D2E6B89A2F3365CA4A02C78404641704E0F3D0FE3D2BE87F19FF00
              C958F00FFDC47FF442D7CF1A2DBC527856FE47BCD52275F33115B07F25FE41F7
              F0A473D0E48E2BBF2F5173F795F47BDBF53CFC6B6A92B3B6ABBFE8695F687A74
              5AF697047A0DD451CDE77990B4EA5A5C282307CC38C75EA3F1ACDB8D36D1341D
              6275D2E64920BD68E398CA36C2BB946C237F27923383D7AD695F59C0BAF69683
              52D75D5FCEDD24AB2F989851FEAF299E7BE01E3D2B36E2DE21A0EB0E2F354664
              BD6558DC3F9720DCBF349F2E37FD4839038AF5ABD3A69D4B4175FE5FE45D97CF
              4EBB6A79546736A1793E9FCDFCCFCFF3FC8DC93C3FA58D560887872EC2341231
              8FED0B962190039F37B64F7FE21D7B55FEC3D3BFB37CCFEC1BADFF006DF2FCCF
              3D71B7ED1B7663CCEBB7E5CE3AF39EF56A4B1B71AAC0BFDABE2220C1212E526D
              E3E64E07EEF3839E7E8BED557EC707F66EEFED2D773F6DDBB76CBB31F68C6EFB
              98DF8E7D7776CF15D73A54AF2F723B3FE4ECBCBFAFBCE58D4AB68FBEF75FCFDD
              F9910B1B7B0F893A04569612D8A1B9B6631CAE1C93E763390CDE9EBDABDE3E10
              FF00C7878A7FEC65BBFE51D783982083E21691FF00132BF011A39127BFB69667
              0EACC51447856605828C0FEF1AF4DF833E23BA8FC4FE2FB0BEE34C8E7975096F
              1E230476F26FDADB830CA6E519C337CA223EE6BE731C92AB3495B5F2FD343E97
              08DBC246EEFF00D79EA7A7F8E7FE49E788FF00EC1575FF00A29ABC57E2FDB437
              9F1734982E6D5EEE37D306E86360ACD8798F5247A67AF6AF6AF1CFFC93CF11FF
              00D82AEBFF00453578AFC5F8D65F8B9A4A4935D40A74C1992D031907CF374DA0
              9FD3A66B1C1A4EAC53EEBF3F3D3EF2ABB6B0F36BB3FCBCB5392B8D0F4E4B3D61
              D741BA46837796E675C43FB956E7F79CF24B77E0FE156A4F0FE963558221E1CB
              B08D048C63FB42E5886400E7CDED93DFF8875ED56E2CE0167AC11A96BAC5376D
              575976C9FB953FBCF93F0E71F281DB9AB5258DB8D5605FED5F11106090972936
              F1F32703F779C1CF3F45F6AFA954A95DFB91DD7F27F33F2FEBD0F98752AD97BE
              F67FCFFCABCFFAF530EE34DB44D07589D74B992482F5A38E6328DB0AEE51B08D
              FC9E48CE0F5EB5A5FD87A77FC255F65FEC1BAF27EC5E67D9BCF5DDBB7E37E7CC
              C631C633F877ACDB8B788683AC38BCD51992F5956370FE5C8372FCD27CB8DFF5
              20E40E2B4BEC707FC255E5FF00696BBB3EC5BBCDDB2F9D9DFF0077EE67677E98
              CF7AE4A74E9F32F717D9FE5EF2F2FEBAF43AA739F2BF79FDAFE6ED1F3FEBA752
              B6A5A458C3E18BBB98B46B886649DD56E1A652A804C54023CC2781F2F43CF3EF
              5DD4FF00DA9E7FC36FEC2FB47DAFEC126EFB367CDF27ECF1F9DB31F36EF2B7E3
              6FCD9FBBF362B85D4AD614F0C5DC8B7FAC48CB3B8114CB2792DFBE232D940338
              E4E4FDEF7E2BABF0D78A350B2F88DF0E22510CAB02410C65ECE58484B802071F
              337CF85E43AE149EC4715C38F518B4A292F756D6EFE56FF33D6CB66DD1ABCCDB
              D7ADFCBBBFCB43DC3599FC6D6F0F80DC8BB3316B74D6BECD1AB8F3D9E057F302
              02367966EB27EE0383C308CD7877C42FF9143C51FF0063F5DFFE8B7AFACE2B88
              669268E19A391E07F2E554604C6DB436D61D8ED65383D981EF5F267C42FF0091
              43C51FF63F5DFF00E8B7AF251D6BA9CAC7A458B5CE80A746B822EA076997CE5F
              F48223072BFBCE3079E76F5FC288348B17D2B5A95B46B867B79E758A4132E200
              AB9018799CEDEF80D9F7A23B584DCE803EDFAC01240E588593317EEC1C45F274
              EC76E78C5105AC274AD698DFEB00C73CE151564D92617832FC98C9FE2CE3DF15
              F491A74EEFDC5D7F97F923E5F3F5F3B9F3CE73B2F79FFE4DFCCFCFFAF434E4F0
              FE963558221E1CBB08D048C63FB42E5886400E7CDED93DFF008875ED99A96916
              30F862EEE62D1AE219927755B86994AA01315008F309E07CBD0F3CFBD69C9636
              E355817FB57C444182425CA4DBC7CC9C0FDDE7073CFD17DAB3352B5853C31772
              2DFEB1232CEE04532C9E4B7EF88CB6500CE39393F7BDF8ADEBD3A4A352D05B3F
              E4ECBB2FCBE46146A547285E6F75FCDDDF9FE659FEC3D3BFE12AFB2FF60DD793
              F62F33ECDE7AEEDDBF1BF3E66318E319FC3BD269F1243E0BF1E45144D0A24F68
              AB1B1C940276001393D3EA697EC707FC255E5FF696BBB3EC5BBCDDB2F9D9DFF7
              7EE67677E98CF7A8622B6FE10F15A417B2C8D2DDC692C33594AEE5165CA3B4DC
              2A124B6430C9DB8E09AE0C6C6118DE314B596DCBDBC92FF23D5CB252751F349B
              F77ADFB3EEFF00E09F5D5701E33FF92B1E01FF00B88FFE885AD3F875E26B8F10
              FC3FD1753D6A7845F5EF99164613CE74671C2FF78AC65881E8C40007199E33FF
              0092B1E01FFB88FF00E885AF9F87C48F523BA3E78D174DB4B9F0ADFDD4FA5CD7
              1345E66CB9494058F0808C82E09C1E7A1FC7A559F106916369736CB6FA35C5A8
              782E59964995B7958F2A46243F74F27A67DFA556D16DE293C2B7F23DE6A913AF
              9988AD83F92FF20FBF85239E87247156F5CB14FB6DA456F79AD5CBC915C002E5
              24DD9F2F8550501209E1B1DBAE2BE9634E0F089F2ABDA3FCB7F8BD2FFD6BA1E0
              4A735896B99DBDEFE6B7C3EB6FEB4D4F4C99F55BEBC8EE22B0B77B28F0F0C373
              72F0BB383C3BA88DBA705549E3A91BB012E7DBB5EFFA06E9FF00F8307FFE335C
              DB6ADE17586595AFF5711C2FB247335F6D46E98273C1E471EF531BBF0F890466
              E35C0EC0B05DF7F92063271F88FCC578CA9D47B267ACEA416ECDEFB76BDFF40D
              D3FF00F060FF00FC66A9EAABAE6A9A5CD68D61A7C65C02927DBE43E5B82191B1
              E4F38600E3A1C60D64C9A9786A2B769E5BDD61214255A4696FC2A90704139C75
              E3EB48FAA78623BB8ED64BED612E2520470B4D7C1DC9381819C9C9E29384D2D5
              0E328B764CE834ABAD734CD1ACAC3FB374F97ECB6F1C3E67F683AEEDAA1738F2
              78CE3A55BFED7D73FE813A7FFE0C9FFF008C57356773A0EA17F2D8D84FAE5D5D
              C20996DE07BF7923C1C1CA8E460900E7B9AB73D8D9DADBC971736BE27861890B
              C92491EA2AA8A064924F0001DEA2E5D8DAFED7D73FE813A7FF00E0C9FF00F8C5
              1FDAFAE7FD0274FF00FC193FFF0018AE6AE6E741B2BE4B2BC9B5DB7BA9177A41
              2BDFABB2F3C853C91C1E7D8D31AFFC3AAB2B35D6B4161FF584C97F84E33CFA70
              41FA1AD1467257488728C5D9B3A8FED7D73FE813A7FF00E0C9FF00F8C51FDAFA
              E7FD0274FF00FC193FFF0018AE64DDF87C4823371AE0760582EFBFC9031938FC
              47E62A16D5BC2EB0CB2B5FEAE2385F648E66BEDA8DD304E783C8E3DE9BA7516E
              9894E0F66759FDAFAE7FD0274FFF00C193FF00F18AC89A1D666F137F6C3D8D8E
              56D960483FB41CAAB0673E67FA9FBD872A0F605BAEEAA06EFC3E24119B8D703B
              02C177DFE4818C9C7E23F3151C9A9786A2B769E5BDD61214255A4696FC2A9070
              4139C75E3EB43A7516E982A907B34745F6ED7BFE81BA7FFE0C1FFF008CD1F6ED
              7BFE81BA7FFE0C1FFF008CD738FAA786239ADE292FB58592E555E0469AF834AA
              DC29519F981EC475AB9E5E97FDCF117E5A8D665DAC4BACDA6AFAC5B057D3B4F8
              AE23C986717EE4A13D411E4F2A70323BE0742011D27C23B5BAB3D77C410DF469
              1C82DECC8F2DF7AB0DD71C83807DB903907B609E6ADECEC6EE1F36D2DFC4D3C7
              B9977C49A8B2E54956191DC1041F420D765F0CEC1ED35AD6E55B5D4E1824B7B4
              557D4239D4B32B4FB8299B920065E9C73EF59CED62A27A2514515CE585145140
              185E39FF009279E23FFB055D7FE8A6ACC8FF00B6BFE158FC52FEDEFB7FFC7E6A
              FF0062FB6EFF00F8F6F27F77E5EEFF00967D718E3AE2B4FC73FF0024F3C47FF6
              0ABAFF00D14D5D07C42FF9263E28FF00B03DDFFE897AD22076B4514550828A28
              A0028A28A00F997C3BA3FC606F06E93368CBABC962D63035AA43AE5AC60C4517
              6615E2F946DC704E47A9A9ADFC05F15FC43AB410EBD6175645EE58FF006D5F6B
              31CEF670B43E5BA2C70B00D91BF1851FEB1B050B17AF70F871FF0024B3C29FF6
              05B3FF00D1095D253BB158C7F0A786AD3C21E17B3D0B4E9A79ADAD0308DEE0AE
              F219CB73B554705B030070056C51452185145140051451401F3468BFF23278D7
              FEC68BFF00FD0C56CD6368BFF23278D7FEC68BFF00FD0C56CD6D1D8C9EE15B7F
              0F7FE4A9DB7FD816F3FF0047DA56256DFC3DFF0092A76DFF00605BCFFD1F694A
              5B0E3B9E7DE38FF92EDF13BFEC597FFD26B7AF1CB5771E0CBD51AB451219C66C
              0A2EF9794F981CEEFCBFBB5EC7E38FF92EDF13BFEC597FFD26B7AF21B3B6DDE0
              1BFB8FB15ABEDB803ED2CDFBD4E63E00DBD39FEF0EA7F1E8C226DCADFCB2EFDB
              CBF5D3B99629A5185FF9976EFE7FA6BD8D6F367FF84AB77FC2576BBFEC58FB6F
              9516DC6FFF00578CE33DF3D6AB6A524C7C3176ADE24B7B9533BE6CD638C349FB
              E3F3020E793F3F1DBDAB57FB3BFE2B3F27FB1B4BFF00907EFF00237FEEFF00D6
              63767CBFBDDBA74EFDAA9EAD63E5F846F66FECAD3E2DB7120F3E37FDE27FA411
              803CB1C0FBBD471F957D054A73F67577DA7D67E5E7F9FCCF061521CF4F6DE3D2
              1E7FD69F20F367FF0084AB77FC2576BBFEC58FB6F9516DC6FF00F578CE33DF3D
              6AB69B24C3C3168ABE24B7B6513A62CDA38CB47FBE1F3124E783F3F3DBDAB57F
              B3BFE2B3F27FB1B4BFF907EFF237FEEFFD663767CBFBDDBA74EFDAA9E9363E67
              846CA6FECAD3E5DD7118F3E47FDE3FFA401823CB3C1FBBD4F1F951ECE7ED25BE
              D3EB3FEEF9FF005D7A07B4872476DE3D21FDEFEBF2EA5FF0F2B5C7C5CD144FAB
              AEA9889D84F0623DB849085F90FA8E47707078AF5FF857A745AC7C07B1D32E59
              D61BDB7BBB791A32030579A552464119C1F4AF2AD0AD92D7E2FE88A2D2DECDCC
              129315B7298F2E4C3676AF2790463B0E4E78F60F82BFF248345FFB6FFF00A512
              578198C5C6B493EEFBF65DF5FBCFA1C1B4F074DAF3EDE5DB4FB83E357FC920D6
              BFED87FE94475E47E36729F1A75FF2F544D2E431438B89023291E4C594C3773C
              1CE7F84F073C7AE7C6AFF9241AD7FDB0FF00D288EBCA7C5B6C975F1BB5F53696
              F78E208488AE784C795165B3B5B91C0031DCF231CCE5C9BC4452EFE7FA6BF711
              8D6961A6DFE9FAE9F79C9EA524C7C3176ADE24B7B9533BE6CD638C349FBE3F30
              20E793F3F1DBDAB4E49AE3FB5603FF00099DA31F224C4DE4C384F993E5EB8E7A
              FF00C04FBD55D5AC7CBF08DECDFD95A7C5B6E241E7C6FF00BC4FF48230079638
              1F77A8E3F2AD7934BC6B56C9FD83A40CDBCA7CB127CAD868F93FBAEA33C71DCF
              4EFF004B0A53E6EBB43AD4EEFCFF00E076D6E7CF4AA4397A6F3E90ECBFAEFDCC
              8F367FECDC7FC2576B8FB6E7CAF2A2CE7ED1FEB3AE719F9F1D31ED479B3FFC25
              5BBFE12BB5DFF62C7DB7CA8B6E37FF00ABC6719EF9EB575F4D41A6068F47D358
              7DB829919B0D9FB4E0A63CB3F2E72B9CFDDE703EED27F677FC567E4FF63697FF
              0020FDFE46FF00DDFF00ACC6ECF97F7BB74E9DFB512A535C9BFD9EB53CFCFF00
              2F9095487BDB7DAE90F2F2FCFE673174EE7C1964A7568A54139C58045DF172FF
              003139DDF9FF007ABD3FC39FF237E87FF628DBFF00E8C15E6B796DB7C036171F
              62B54DD7047DA55BF7AFCC9C11B7A71FDE3D07E1E95E1CFF0091BF43FF00B146
              DFFF00460AF9FC426B96FF00CB1EFDBCFF004D3B1F498469D2A96FE67DBBF97E
              BAF73B811462669422891942B381F310324027D06E3F99F5A75145728CEAFE13
              FF00C8DFE24FFAF0D3FF00F465DD7A2D8695069D7BA9DD40D233EA574B753072
              085610C70E170381B6253CE7927E83CEBE13FF00C8DFE24FFAF0D3FF00F465DD
              7A9D632DCD56C7C6FF0011FF00E455F15FFD940BCFFD01EB1649AE3FB5603FF0
              99DA31F224C4DE4C384F993E5EB8E7AFFC04FBD6D7C47FF9157C57FF006502F3
              FF00407AA527FC251FDB56DBBFB23CEFB3CBB31E6EDDBBA3DD9EF9CEDC7E35EE
              E551BC67BFD9DAFDFC9AF97E163C9CD1D9C36DA5BDBB79A7FD6F739F8249BFB2
              B5A03C496F1869E7DD098E3CDD7CBCB2F391BBA0C7E15A724D71FDAB01FF0084
              CED18F912626F261C27CC9F2F5C73D7FE027DEAADB7F6E7F62F883CBFECFF27E
              D173F6ADDBF76EDBF3ECED8C74CFE35AF27FC251FDB56DBBFB23CEFB3CBB31E6
              EDDBBA3DD9EF9CEDC7E35E9D183E45F174FE6FE67FDEFEBF13CCAB25CFF67AFF
              002FF2AFEE9CFEA524C7C3176ADE24B7B9533BE6CD638C349FBE3F3020E793F3
              F1DBDAACF9B3FF00C255BBFE12BB5DFF0062C7DB7CA8B6E37FFABC6719EF9EB4
              6ADFDB9FF088DEFDA7FB3FEC7F6893CCF2F7F99BBED0738CF18DDFA7BD5CFF00
              8A8BFE133FF985FDB7FB3FFE9A797E5F99F9EECFE18ACF95FB48FC5B43F9BFBD
              FDEFBBE76B6B7BE65C92F87797F2FF0077FBBF7FE37E993E1DFF00926BE31FFB
              71FF00D1C6BEB7B8B44B99ED657386B594CA9F229C928C9D482470E795C1ED9C
              120FC91E1DFF00926BE31FFB71FF00D1C6BEBEAF92A9B47D3F567D64FE18FA7E
              ACE03C67FF002563C03FF711FF00D10B5F3C68AF2AF856FD535D86CD0F999B27
              8D0B4DF20E849C8CF4E3D2BE87F19FFC958F00FF00DC47FF00442D7CFF00A07F
              6BFF00C219A9FD8BEC5F62FDEF9DE76FF33FD58DDB71C74C633DEBD0CB55E6F7
              D9ED7FD1A3CEC7BB515B6EB7B7EA996EFA59CEBDA593E2BB59D879DB6E562882
              DBFCA3A80707774E6B36E1E53A0EB00EBB0C8A6F58B5B08D375C9DCBFBC07390
              0F5C0E38ADED47FE122FF849347F3FFB2FED3FBFF236799B3EE0DDBB3CF4E98A
              C8B9FED7FF00846F5DF33EC5F66FED06FB4EDDFBFCCDE99D9DB6E71D79EB5ECE
              222EF53E2FB5FCDFC8B7F7BF3BE9DD68791424AD4FE1E9FCBFCEFF00BBFE5AF9
              EA6AC935C7F6AC07FE133B463E44989BC98709F327CBD71CF5FF00809F7AABE6
              CFFD9B8FF84AED71F6DCF95E5459CFDA3FD675CE33F3E3A63DAB5E4FF84A3FB6
              ADB77F6479DF6797663CDDBB7747BB3DF39DB8FC6AA7FC545FD95FF30BF2BFB4
              3FE9A6EF33ED5FFA0EFF00C76FBD764E0F9A5A4B67FCFDA3FDF39212568FC3BA
              FE5EEFFBA670777F893A0197568B553F69B6FDF448AA17F7DF77E5247BFE35ED
              BF0DF4E8B58F0BF8DB4CB967586F75DBFB791A32030574452464119C1F4AF18B
              8FED1FF859DE1FFED7FB2F9DF68B6DBF65DDB76F9DDF7739CE7F4AF72F843FF1
              E1E29FFB196EFF009475F319869567BEFD6F7FC5B7F89F538277C1C3F4FF0080
              92FC0E93C73FF24F3C47FF0060ABAFFD14D5E2BF17D997E2E69263BE4D3DBFB3
              062E64552A9F3CDD9B8E7A7E35ED5E39FF009279E23FFB055D7FE8A6AF19F8B1
              F6BFF85C5A47F67793F69FECC1B3CFCECFBD36738E7A66B0C16B5A1FE25F9F95
              BF31E23FDDEA7A3FC8E3AE259CD9EB19F15DAC80EEDD188A2CDCFEE57A73C67E
              EF1DC7AD5A926B8FED580FFC26768C7C893137930E13E64F97AE39EBFF00013E
              F4EB9FF848BEC1AEF99FD97E5FCDF69DBE667FD426767FC071D7BE7B55B93FE1
              28FEDAB6DDFD91E77D9E5D98F376EDDD1EECF7CE76E3F1AFAE50777A4B75FCFF
              00CCFF00BFFD3F3D4F9572565F0ECFF97F957F77FAF4D0E5AE1E53A0EB00EBB0
              C8A6F58B5B08D375C9DCBFBC073900F5C0E38AD2F367FF0084AB77FC2576BBFE
              C58FB6F9516DC6FF00F578CE33DF3D6AA5CFF6BFFC237AEF99F62FB37F6837DA
              76EFDFE66F4CECEDB738EBCF5AD7FF008A8BFE133FF985FDB7FB3FFE9A797E5F
              99F9EECFE18AE3A717CCBE2FB3FCDDE5FDEFBBE76B6B7EBA925CAFE1FB5FCBDA
              3FDDFEB4DF4B656A524C7C3176ADE24B7B9533BE6CD638C349FBE3F3020E793F
              3F1DBDABA8D521D56CEEFC05A9785FED2DABDC6942D95608DDC88C4237102305
              C7C92C992BF30032B8233581AB7F6E7FC2237BF69FECFF00B1FDA24F33CBDFE6
              6EFB41CE33C6377E9EF5EA3E0CFF0091EBE14FFD83EE7FF488579F982B35BFC2
              B7BF7F36F4FC3C8F632BB3A1576DFA5BCBB25AFF00573B5BBB2F1868F37852F2
              237EF73A95DDB4DAF88512455B9296903EE118C08FCA8EE093CA06E72088F1E3
              5F10BFE450F147FD8FD77FFA2DEBEB8AF91FE217FC8A1E28FF00B1FAEFFF0045
              BD78E8EC5D4E563926FB4E81FF001525BA9581F6BF971FFA1FEEC7CADCF39FBB
              F363A5104937F656B407892DE30D3CFBA131C79BAF97965E72377418FC2ACC5F
              DB9F6AF0D6DFECFDFF00677FB2677E36F9433E67BEDC74EF45B7F6E7F62F883C
              BFECFF0027ED173F6ADDBF76EDBF3ECED8C74CFE35F5118BBBF8BAFF0037F247
              FBDFD2F2D17CDCA4ACBE1E9FCBFCEFFBBFD3F3D4B524D71FDAB01FF84CED18F9
              12626F261C27CC9F2F5C73D7FE027DEB33529263E18BB56F125BDCA99DF366B1
              C61A4FDF1F981073C9F9F8EDED5BC5BC52FAADA3489A54770D6D29543E661577
              47B81C679CEDE9C75F6ACAD5BFB73FE111BDFB4FF67FD8FED12799E5EFF3377D
              A0E719E31BBF4F7AE8C445F2547696D2FE6ECB7BCBEFBDFCCC28497343E1DD7F
              2F77B7BBF9079B3FFC255BBFE12BB5DFF62C7DB7CA8B6E37FF00ABC6719EF9EB
              49A7927C17E3C2D3ADC933DA666500093F7EDF30C71CF5E2AEFF00C545FF0009
              9FFCC2FEDBFD9FFF004D3CBF2FCCFCF767F0C554B3F3BFE10FF1F7DA7679DF68
              B5F33CBCEDDDF686CE33CE335E763A36875DE5BDFB79B7AFE3E67AD95BBD57B7
              C1D2DD9F64BFAE87D5179A7457D7561712B387B0B837110523058C4F160F1D36
              C8DD31C81F4AE2FC67FF002563C03FF711FF00D10B5DFD701E33FF0092B1E01F
              FB88FF00E885AF9C87C48F5A3BA3E78D15E55F0ADFAA6BB0D9A1F33364F1A169
              BE41D093919E9C7A5695F4B39D7B4B27C576B3B0F3B6DCAC5105B7F947500E0E
              EE9CD54D03FB5FFE10CD4FEC5F62FB17EF7CEF3B7F99FEAC6EDB8E3A6319EF5A
              FA8FFC245FF09268FE7FF65FDA7F7FE46CF3367DC1BB7679E9D315F59422FEAF
              0F8BEC7F35BE2FF15BD34DF6B3D4F9AAD25EDE5F0FDAFE5FE5FF000FDFAEDBDD
              6860DC3CA741D601D761914DEB16B611A6EB93B97F780E7201EB81C715B924D7
              1FDAB01FF84CED18F912626F261C27CC9F2F5C73D7FE027DEB2AE7FB5FFE11BD
              77CCFB17D9BFB41BED3B77EFF337A67676DB9C75E7AD74527FC251FDB56DBBFB
              23CEFB3CBB31E6EDDBBA3DD9EF9CEDC7E35A508BBBF8B68FF37F34BFBDF779ED
              67ABCEB495BECEF2FE5FE55FDDFEBADD6873FA94931F0C5DAB7892DEE54CEF9B
              358E30D27EF8FCC0839E4FCFC76F6A903BBFC49D00CBAB45AA9FB4DB7EFA2455
              0BFBEFBBF2923DFF001A9756FEDCFF008446F7ED3FD9FF0063FB449E6797BFCC
              DDF6839C678C6EFD3DE96E3FB47FE167787FFB5FECBE77DA2DB6FD9776DDBE77
              7DDCE739FD2B931916A9F5DA1BF37797793F97E16D6FDB8095EBC76F8A5B5BFB
              BD92F9FE373DBBE144115D68FE2DB7B98926865F115E2491C8A195D4AA02083C
              10476AEA3C73FF0024F3C47FF60ABAFF00D14D5CDFC21FF8F0F14FFD8CB77FCA
              3AE93C73FF0024F3C47FF60ABAFF00D14D5F353F8D9EECFE2678AFC5F665F8B9
              A498EF934F6FECC18B99154AA7CF3766E39E9F8D725712CE6CF58CF8AED64077
              6E8C45166E7F72BD39E33F778EE3D6BB1F8B1F6BFF0085C5A47F67793F69FECC
              1B3CFCECFBD36738E7A66B9AB9FF00848BEC1AEF99FD97E5FCDF69DBE667FD42
              6767FC071D7BE7B57D2E062DE1DBD777B735BE1F2925F86DE5A1E16632B62BA6
              CB7B5FE2F34DFE3FE63649AE3FB5603FF099DA31F224C4DE4C384F993E5EB8E7
              AFFC04FBD61DC3CA741D601D761914DEB16B611A6EB93B97F780E7201EB81C71
              5D3CCDE295D6ED014D29A668260A57CCD8AB98CB16CF3D42818CF535CFDCFF00
              6BFF00C237AEF99F62FB37F6837DA76EFDFE66F4CECEDB738EBCF5AEAC5C5DA5
              F16D2DF9BF957793F9FE37D8E2C34BE1F8778EDCBFCCFB457F5F79AB24D71FDA
              B01FF84CED18F912626F261C27CC9F2F5C73D7FE027DEB33529263E18BB56F12
              5BDCA99DF366B1C61A4FDF1F981073C9F9F8EDED5D049FF0947F6D5B6EFEC8F3
              BECF2ECC79BB76EE8F767BE73B71F8D646ADFDB9FF00088DEFDA7FB3FEC7F689
              3CCF2F7F99BBED0738CF18DDFA7BD6B8883E5A9F16D2FE6ECB7F7BF3BFCC8A12
              5CD4FE1DD7F2F77FDDFF0023AE86CA38FC45F0EEF83379B3692D0B0246D0A96F
              918F7FDE1FD2BD02B874FF008FFF0086BFF60F9BFF004992BB8AF9797C5FD763
              EB715A55FBBF23A0F87DFF00227AFF00D7FDF7FE95CD5D35733F0FBFE44F5FFA
              FF00BEFF00D2B9ABA6AE17B92B60A28A29005145140185E39FF9279E23FF00B0
              55D7FE8A6AA0F3EBB71F0D7E28BF8885D822E7544B2FB4C6507D956DC2C5E582
              00D980704704E4F249357FC73FF24F3C47FF0060ABAFFD14D5D07C42FF009263
              E28FFB03DDFF00E897AD22076B4514550828A28A0028A28A00E6FE1C7FC92CF0
              A7FD816CFF00F44257495CDFC38FF9259E14FF00B02D9FFE884AE92800A28A28
              00A28A2800A28A2803E68D17FE464F1AFF00D8D17FFF00A18AD9AE07E0F7FC8A
              175FF5FEFF00FA2E3AEFAB68EC64F70ADBF87BFF00254EDBFEC0B79FFA3ED2B1
              2AEF863558341F1A4FABDE248F6FA7F87351BA956200BB2A496AC428240CE071
              92294B61C7738CF1C7FC976F89DFF62CBFFE935BD78E5AD8DBC9E0CBDBC7B095
              EE239C2ADD07011065382BBB27A9FE13D7F2EE3E2741E23D6BC443C6BA68548F
              C536685ECB49BA92E248221044A639C845FBDC7CB823208E7193C4C369E57847
              50F3E6D4619E2B8D8D6C15C4390501DFF2E030E7A907815BE1526E5757F765DB
              B79FFC3F632C4C9A8C6CFED2EFDFCBFE18D9FEC3D3BFE12AFB2FF60DD793F62F
              33ECDE7AEEDDBF1BF3E66318E319FC3BD56D4B48B187C317773168D710CC93BA
              ADC34CA550098A8047984F03E5E879E7DEACFD8E0FF84ABCBFED2D7767D8B779
              BB65F3B3BFEEFDCCECEFD319EF55B52B5853C317722DFEB1232CEE04532C9E4B
              7EF88CB6500CE39393F7BDF8AF76A53A7ECEA7B8B697F2F9797E5F23C5854A9C
              F4FDE7BC7F9BCFCFF3F9967FB0F4EFF84ABECBFD8375E4FD8BCCFB379EBBB76F
              C6FCF998C638C67F0EF55B4DD22C66F0C5A5CCBA35C4D33CE8AD70B328570660
              A401E603C8F97A0E79F7AB3F6383FE12AF2FFB4B5DD9F62DDE6ED97CECEFFBBF
              733B3BF4C67BD54B1B78BFE114B69BED9AD799E6AFEE2DC3F95FEB80F90EDDBB
              BB8E7EF7BF14BD9D2E797B8B69FF002FF77CBEEFC3A873D4E48FBCF78FF37F7B
              CFFAEBD0DFF0D69F6BA7FC5AD152CECDEC55A29498247DCD9F2E4F9B219860F4
              EB9F94F038CF7FFB3B477ADE17D46E25BE9A7B2F396082DA476DB6EEBB9DF62E
              480AC254391824EEC8E013E33AFF00DA74DD723B8D2AFB566921B70E6E673224
              B102C57824290BC81E99623BD7AF7C18F881691E9DA07835A28E49E54B922486
              472F13077940915A35500A96C1477E8320678F0F1F655A518AB6BD2DE5DB43DC
              C24DBC2C13777AFF005AEA74FF001CAF6DED7E13EA10CF26C92F25861806D277
              B8915C8E3A7CA8C79F4F5C579578DB4FB5D43E34EBE97966F7CAB1424411BED6
              CF9317CD92CA303A75CFCC383CE3ACF8C56173E37BD9EC749B655B8F0DCB0C65
              A7B911FDA9AEB6656352B83B0796CCECEA1416C82306BCAB4AB6D4AEBC437137
              88AE759B7BA780334A0C86E24E4019E0B15C2E338C0C019E94B0093AF04D5F5F
              2FD74271926B0F3B3B7F5E5A8FD4B48B187C317773168D710CC93BAADC34CA55
              0098A8047984F03E5E879E7DEB4E4F0FE963558221E1CBB08D048C63FB42E588
              6400E7CDED93DFF8875ED99A95AC29E18BB916FF00589196770229964F25BF7C
              465B280671C9C9FBDEFC569C9636E355817FB57C444182425CA4DBC7CC9C0FDD
              E7073CFD17DABE8634E9737C0B687F2777E5D7AFE27832A95397E37BCBF9BB2F
              3FEBA15DF41D3574F0EBA25C3B7DB0279AB38C6D33EDF2F064CE40F9338C6467
              38F9A9BFD87A77FC255F65FEC1BAF27EC5E67D9BCF5DDBB7E37E7CCC631C633F
              877A3EC707F66EEFED2D773F6DDBB76CBB31F68C6EFB98DF8E7D7776CF147D8E
              0FF84ABCBFED2D7767D8B779BB65F3B3BFEEFDCCECEFD319EF4DD3A5EEFB91FB
              3FC9E7E5F9FCC154A9EF7BEFED7F37979FF5D0C4BAB1B78FC1965789612A5C49
              3956BA2E0A38CBF017764741FC23A7E7E85A5C772FE3CF08B5B1611C7E1A85AE
              36BE014DAE064771BCA71EB83DABCF6EA145F06594A2E6FD9DA720C2E1BECEBC
              BF2BF2EDCF1D89EA6B63C09E20934ED4EE2FAF2FA3B89E1B216969677324E649
              86E056388A4520182B80AC5465C73D71E0623962E292B7BB1EDDBCBFE1FB9EFE
              1A6FD9CD3D7DE7DFBF9FFC31EE3454161791EA3A6DB5EC01963B9892640E30C0
              3004671DF9A9EB028EAFE13FFC8DFE24FF00AF0D3FFF00465DD7A9D7967C27FF
              0091BFC49FF5E1A7FF00E8CBBAF53AC65B9AAD8F8BBC6BA85AEAFE01F106A5A7
              CBE75A5E78E6E67824DA577A3C4CCA704023208E08CD67490DC7F6AC03FE10CB
              453E449887CE870FF327CDD31C74FF00811F7AE67C416579A1EBDA9F85175192
              7B5B0D4E5842B3F9714922318FCD285B6A920752781C66BB166B06D6E11B3585
              45825196FB5EE6E6220AFF00163920F4E40CFF000D7B995439A33F7ADAC7B77F
              38BFD3CEE79199CED28595F4977EDE4D7F5B58C18239BFB2B5A23C376F2059E7
              DD31923CDAFCBCAAF193B7A8C7E157E3B4B882F6CE1FF8442DDCA5B3A8492E21
              265C18C6F63B7191FAEF3EF50DB7D8FF00B17C41BBFB437FDA2E7CBDBF68DB8D
              BC6FC719F5DFCFAD6BC9FD9FFDB56D8FED7D9F679739FB5EECEE8F18EF8EB9C7
              1D33DABD3A34538A7CEBA7F2FF0033FEE7F5E9A1E6D5AAD49AE47D7F9BF97FC5
              FD7E273FA94730F0C5DB3786EDED944EF9BC59232D1FEF8FCA0019E0FC9C76F6
              AB3E54FF00F0956DFF008452D77FD8B3F62F362DB8DFFEB338C67B63AD1AB7D8
              FF00E111BDF2FF00B43CDFB449B7CCFB47978FB41EBBBE5CE3D79CF5E6AE7FA0
              FF00C267FF00314F2BFB3FFE9EBCCDDE67FDF7B71FF01CFBD47B25ED23EFADA1
              FCBFDEFEE74FF87BE96BF68F925EEBDE5FCDFDDFEF7F5D2DD727C3BFF24D7C63
              FF006E3FFA38D7D41E08D3B55D27C1B6167E20BB9AEF508C399669E432484348
              CCA18E4FCC14A838240230091835F1F580D4AE449A7E982EA5174C8B25B5BEE6
              F38EE010151F78EE6007B918E4D7DB1637B6FA969D6D7D65279B6D7512CD0BED
              237230054E0F23823AD7C8D477491F55295D25DBFCD9E77E2CD4E0B9F8DFE0DD
              3E10CD259A5EB4AE3050178321339FBC028620F6743DEBC1345495BC2B7EC9A1
              437883CCCDEBC881A1F90740464E3AF1EB5DD7C6EB28B44F8A167AA697AD4D65
              A8EA312C93B0DE9F6450044240E833B595581500B0DADD77015C6681F66FF843
              353F3BEDBE6FEF76F93E7797FEAC6376DF93AF5DDDBAF15E8E5CB9AA357B68FB
              76F34CF3F1D2B525A5F55DFF00468B77D14E35ED2C1F0A5AC0C7CEDB6CB2C456
              E3E51D48181B7AF359335BCABA4EB739D19114DD6C69BCD422D0871F22AE33FC
              5824718C7A5745A8FD87FE124D1F67F6A797FBFDFBFED5BFEE0C6DCFCDF5DBF8
              F159173F66FF00846F5DDBF6DF33FB41BCBDDE76CDBBD3EFE7E5DDD7EF7CDD3B
              E2BD8C4D257A9EF2D39BF97F917F757A696F93D4F270F51DA1EEBD6DFCDFCEFF
              00BDF3EBF7686AC90DC7F6AC03FE10CB453E449887CE870FF327CDD31C74FF00
              811F7AABE54FFD9B9FF8452D71F6DC79BE6C59CFDA3FD5F4CE33F267A63DAB5E
              4FECFF00EDAB6C7F6BECFB3CB9CFDAF7677478C77C75CE38E99ED555D2CA3D30
              06FED22FF6E07E56B92BB4DCE7B71BB69E9F78371F7ABB674759FBEB67FCBD97
              F70E4855D23EE3DD7F3777FDF33423A7C49D004BA4C5A51FB4DB7EE627560DFB
              EFBDF2803DBF0AF65F8350DEAEB7E379E42FF607D61921064CA894339930B9E0
              ED68B271CF1D71C78678BE64B7F105B4DA7497913470ABABCED2891583B10419
              3E61DB18E335EE5FB3DCD25C780B529A791A5964D5E57777625998C5112493D4
              D7CB661EED69C6F7D7CBF4497E07D3E0A77C2C15ADFD79B6FF0013A8F8A3AAAE
              93F0D35A7F2C4D25C5ABDB4716FDACC6452AC47073B50B391E887A0C91E4BF17
              D59BE2E69223B14D41BFB3062DA4650AFF003CDDDB8E3AFE15E8BF1BB47B7D57
              E1ADC4D777FF00628F4E945D83E4993CD7DAC891F046DDCD228DDCE3B8C57805
              ADF2EA1E2DD3AE2F75AD4B57CC520692512F9F100650A9F2B3372BB5CED240DE
              473826B2C1EB5A0BCD7E7E77FC9958895A84F4E8FF002F9176E229C59EB19F0A
              5AC606EDD20962CDB7EE57A71CE3EF71DCFAD3AFB4FB8BAD56C87FC22715B988
              3CA2186E615136193EF7CB8206718EFBBEB56AE7EC3F60D776FF006A6EF9BCBD
              DF6AC7FA84FBF9E3AE7EF76C76C55B93FB3FFB6ADB1FDAFB3ECF2E73F6BDD9DD
              1E31DF1D738E3A67B57D6BA1195D39ADD758FF0033FEE7F5E9A1F2EAB4A366A0
              F67FCDFCABFBFF00D7AEA72D70928D07582742863517AC1AE4489BAD8EE5FDD8
              18C903A6471CD697953FFC255B7FE114B5DFF62CFD8BCD8B6E37FF00ACCE319E
              D8EB552E7ECDFF0008DEBBB7EDBE67F683797BBCED9B77A7DFCFCBBBAFDEF9BA
              77C56BFF00A0FF00C267FF00314F2BFB3FFE9EBCCDDE67FDF7B71FF01CFBD725
              3A6B997BEBECFF002F797F77FAEB7D2DD552A3E57EEBFB5FCDDA3FDEFEBCB5BE
              56A51CC3C3176CDE1BB7B6513BE6F1648CB47FBE3F28006783F271DBDABB5BE7
              BF8BFE15ACBA34BA8457F1DB0685B4E80CB2E0241E66140248F2F79236B02060
              8209AE4F56FB1FFC2237BE5FF6879BF68936F99F68F2F1F683D777CB9C7AF39E
              BCD743F066F8DC7C60F09DA9D46E2F85B2DC6C32EFDB12B591FDDA866200560C
              BC28FBA0E48202F9D98DA128ABDEF15DBBBEC97F9F99EB65952F46AAB5AF2F3F
              2EEDFF009791EF7AF6ABE35860F06982CDA092EA7B61AD2D9C02611397843C67
              EFED8B635C12E3A18D3E719C378078EAE21BBF03788EE6D268E7826F1DDD4914
              B130657531B10C08E08239C8AFAF2BE07F10DBDD683ABEA9E19FED09AE2CF4FD
              4658F664AC72491B18FCCF2F2406207B9C719AF20EE5A1AD1C737DA740FF008A
              6EDD8B40FB53CC8FFD33F763E66E38C7DEF9B3D688239BFB2B5A23C376F2059E
              7DD31923CDAFCBCAAF193B7A8C7E15662FB1FDABC359FED0C7D9DFCDC7DA339F
              287FABEF8CFF00738C7B516DF63FEC5F106EFED0DFF68B9F2F6FDA36E36F1BF1
              C67D77F3EB5F511A6AEFDF5D7F97F923FDDF97FC1D5FCDCAA3B2F75F4FE6FE77
              FDEFEBD3456A486E3FB5601FF0865A29F224C43E74387F993E6E98E3A7FC08FB
              D666A51CC3C3176CDE1BB7B6513BE6F1648CB47FBE3F28006783F271DBDABA29
              12C0EAB038FED5F2D6090302D77BF25931807E623839238FBB9E4AD636ADF63F
              F8446F7CBFED0F37ED126DF33ED1E5E3ED07AEEF9738F5E73D79AE8C451B46A7
              BEB697F2F65FDC5F85BE473D0AB7943DC7BAFE6EEFFBDF9DC3CA9FFE12ADBFF0
              8A5AEFFB167EC5E6C5B71BFF00D66718CF6C75A4D3C11E0BF1E06816D889ED33
              0A90447FBF6F94638E3A71577FD07FE133FF0098A795FD9FFF004F5E66EF33FE
              FBDB8FF80E7DEB94B8FB5CFAA6A163A5CD74DF6CBD112D8AF9A64BA3BDB66571
              F31071C37CD9618079C79998454217E6BFBD2EDDBCA2BFCBC8F572DA9FBCF86D
              EEF9F5BF76FF00CFCCFAB7E1D477A9F0F3486D4EFA6D4279A1338B99DD9A4749
              18BC7B8924EE08CA08C9008C0240CD73FE31BDB793E34F81EC524CDCC315ECD2
              26D3F2A3C2429CF4E4C6FF0097B8AEFEC6F6DF52D3ADAFACA4F36DAEA259A17D
              A46E4600A9C1E470475AF9EBE39E9AFA0F8FA3F10E8B7F7B15E5D448D72D12B2
              0B5254C71E255C637AC527CB9CFEEDCF4381F3D1F8AE7B09D9DCE1745495BC2B
              7EC9A1437883CCCDEBC881A1F90740464E3AF1EB5A57D14E35ED2C1F0A5AC0C7
              CEDB6CB2C456E3E51D48181B7AF355340FB37FC219A9F9DF6DF37F7BB7C9F3BC
              BFF5631BB6FC9D7AEEEDD78AD7D47EC3FF0009268FB3FB53CBFDFEFDFF006ADF
              F70636E7E6FAEDFC78AFACA14D3C3C1F3AFB1FCBFCDFE1BFE3EB75A1F355AA35
              5E4B95FDAFE6FE5FF17E9E967A98370928D07582742863517AC1AE4489BAD8EE
              5FDD818C903A6471CD6E490DC7F6AC03FE10CB453E449887CE870FF327CDD31C
              74FF00811F7ACAB9FB37FC237AEEDFB6F99FDA0DE5EEF3B66DDE9F7F3F2EEEBF
              7BE6E9DF15D149FD9FFDB56D8FED7D9F679739FB5EECEE8F18EF8EB9C71D33DA
              B4A14936FDF5B47F97F9A5FDCFEBADD68B3AD51A5F0BDE5FCDFCABFBDFD74B3D
              4E7F528E61E18BB66F0DDBDB289DF378B2465A3FDF1F940033C1F938EDED5204
              74F893A0097498B4A3F69B6FDCC4EAC1BF7DF7BE5007B7E152EADF63FF008446
              F7CBFED0F37ED126DF33ED1E5E3ED07AEEF9738F5E73D79ACCF17CC96FE20B69
              B4E92F2268E157579DA5122B076208327CC3B631C66B931B051A6DF35F487F2F
              79768AFEB7BE96ECC0D4FDF47DDB6B2EFF00DDEF27FD6D6EBEE7F06A1BD5D6FC
              6F3C85FEC0FAC32420C99512867326173C1DAD164E39E3AE38E9FE28EAABA4FC
              34D69FCB1349716AF6D1C5BF6B31914AB11C1CED42CE47A21E832472FF00B3DC
              D25C780B529A791A5964D5E57777625998C5112493D4D6BFC6AD22CB54F8637B
              2EA176F6834F75BA85D5370794028A8475C317DB9E3048278041F9A93BCAE7BB
              2776D9E6DF17D59BE2E69223B14D41BFB3062DA4650AFF003CDDDB8E3AFE15C9
              5C4538B3D633E14B58C0DDBA412C59B6FDCAF4E39C7DEE3B9F5AA56B7CBA878B
              74EB8BDD6B52D5F31481A4944BE7C401942A7CACCDCAED73B4903791CE09ADA9
              92CA4B4D6923FED20EC5846646B955FF005098DE5B81CF77ED8CF18AFA5C0439
              F0CDF35B57A7BBFCBE716FF1FC753C2CC2A5B13F0F45DFF9BCA497E1FE432486
              E3FB5601FF000865A29F224C43E74387F993E6E98E3A7FC08FBD61DC24A341D6
              09D0A18D45EB06B91226EB63B97F76063240E991C735D4C9FD9FFDB56D8FED7D
              9F679739FB5EECEE8F18EF8EB9C71D33DAB9DB9FB37FC237AEEDFB6F99FDA0DE
              5EEF3B66DDE9F7F3F2EEEBF7BE6E9DF15D78BA49297BE9E92FE5FE55DA0BFADA
              CF538F0D51B71F75EF1FE6FE6F393FEBBAD0D5921B8FED5807FC21968A7C8931
              0F9D0E1FE64F9BA638E9FF00023EF599A94730F0C5DB3786EDED944EF9BC5923
              2D1FEF8FCA0019E0FC9C76F6AE824FECFF00EDAB6C7F6BECFB3CB9CFDAF76774
              78C77C75CE38E99ED591AB7D8FFE111BDF2FFB43CDFB449B7CCFB47978FB41EB
              BBE5CE3D79CF5E6B4C4524A353DF5B4BF97B2FEE2FC2DF2228546E54FDC7BAFE
              6EEFFBDFE675F796F25C2FC38FB3CED14F1DA2C88ABC194016FE62E72303CBDE
              4FA85C77AEFEBC47C0F793CFF10B44B77D4EE2FEDEDE16110959B6C24DB92C8A
              AC780A7E5E383B41E98AF6EAF9572E6773EAEBCF9E773A0F87DFF227AFFD7FDF
              7FE95CD5D35733F0FBFE44F5FF00AFFBEFFD2B9ABA6AE37B82D828A28A401451
              45006178E7FE49E788FF00EC1575FF00A29AAA5C5F6BF7DF0C7E25FF00C24A97
              0BE44DAAC3A7F9D6E22CDA2C3FBB2BF28DEBCB61B9CFA9AB7E39FF009279E23F
              FB055D7FE8A6AE83E217FC931F147FD81EEFFF0044BD69103B5A28A2A8414514
              50014514500737F0E3FE4967853FEC0B67FF00A212BA4AF36FD9F3FE484F87BF
              EDE7FF004A65AF49A0028A28A0028A28A0028A28A00F90FE0F7FC8A175FF005F
              EFFF00A2E3AEFAB81F83DFF2285D7FD7FBFF00E8B8EBBEADA3B193DC2AACBFF3
              317FD899AC7FEDBD5AAAFF000AB58D23E2078CEE2DBECB726C66D0EFED2E629D
              4C7E623C96A180653DC1238391ED904A96C38EE78E6B9E2CD6A7D6EF4E97AB5B
              C76D1CC155A29D021442369557F989C93B88C86E83E502AB6A770E9A3788ADE6
              BFB3F31B5390BC1B312487CC5CB2FCFC0E3A60F43CD69FC59F0C3787FC59AA68
              DA604974BF0F1B585243690C7391340AFBA578A34F3395C6E7C9C91D4B1279CB
              3B6DDE01BFB8FB15ABEDB803ED2CDFBD4E63E00DBD39FEF0EA7F1E9C1DEF3B7F
              2CBBF6F2FF008639F176B46FFCD1EDDFCFFE1CE8BFB47FE2B3F3BFB674BFF907
              ECF3F67EEFFD6676E3CCFBDDFAF4EDDEA9EAD7DE67846F61FED5D3E5DD7121F2
              234FDE3FFA413907CC3C1FBDD0F1F9D5CFECEFF8ACFC9FEC6D2FFE41FBFC8DFF
              00BBFF00598DD9F2FEF76E9D3BF6AA7AB58F97E11BD9BFB2B4F8B6DC483CF8DF
              F789FE904600F2C703EEF51C7E55F4753DA7B3ABE93FE6F2FEB5F99E0D3F67ED
              29FAC7F97CFF00AD3E45CFED1FF8ACFCEFED9D2FFE41FB3CFD9FBBFF00599DB8
              F33EF77EBD3B77AA7A4DF797E11B287FB574F8B6DC467C8913F789FE900E49F3
              0703EF741C7E7573FB3BFE2B3F27FB1B4BFF00907EFF00237FEEFF00D663767C
              BFBDDBA74EFDAA9E9363E67846CA6FECAD3E5DD7118F3E47FDE3FF00A401823C
              B3C1FBBD4F1F951FBCF692F49FF37F77FAFCFA07EEFD9C7D61FCBFDEFEBF2EA1
              A941AB6B5E27FF008916A16F7773159066368FE5ABA897EE1CB3027241C12060
              7E7D1F83340F11E8DE2FD2E518FB55C6B36E99F355CBC290CC266E40014C65F0
              3EF00300640CF21E24B7B9B6F13410D9430E9B3CD68C8A962CC7CDDE1D4A7CA8
              092E094C6307382704E3DC7E17F8726D57C37E1AF10EBBA9EA73DFE9B2DF98AD
              A731ED8E496591252C76798CC48C9DCC4EEFCABE6F316FEB33E6EFE7FAEA7BF8
              1B7D5A16EDE5FA6879C78F354D7346F1DEB1141A9D9C57D3EA3234D21110FF00
              47F2E1FB383B973F2A3E381D431E793581A35EEAEDAF1BED6351B788BDA32A5D
              5CED9232A6527682AC06772BE067A29C0C018F5DF8C5E07B07F0E6B9E23B38E4
              3A9DD0B58DE3586397CD612A46A54B219118AB0188D94360641E73E1FE0CB46B
              8D62456B0B7BB06D99D52ECED5C6F51B87CADCE723A7AF3465CDFD6616DEFE7F
              A6BF70B1D6FAB4EFDBCBF5D0D4D5AFBCCF08DEC3FDABA7CBBAE243E4469FBC7F
              F482720F98783F7BA1E3F3AD79354CEB56CFFDBDA41C5BCA3CC11FCAB968F83F
              BDEA71C73D8F5ED91AB58F97E11BD9BFB2B4F8B6DC483CF8DFF789FE904600F2
              C703EEF51C7E55AF26978D6AD93FB074819B794F9624F95B0D1F27F75D4678E3
              B9E9DFEA21ED79BE50FE7EEFFAFC8F9D9FB3E5F9CFF97B2FEBF32A7F68FF00C4
              AB67F6CE97FF00210DFB367CDFF1F5BB77FACFBBFC5D3EEF7EF47F68FF00C567
              E77F6CE97FF20FD9E7ECFDDFFACCEDC799F7BBF5E9DBBD1FD9DFF12ADFFD8DA5
              FF00C84366FDFF0037FC7D6DDBFEAFEEFF000F5FBBDBB51FD9DFF159F93FD8DA
              5FFC83F7F91BFF0077FEB31BB3E5FDEEDD3A77ED43F6BEE7FDBBFCFE7FD7E40B
              D9FBFF00F6F7F2F97F5F99CEDE5CEEF00D85BFDB6D5F6DC13F6655FDEA732724
              EEE9CFF74751F8F4369E1AF1559EA56F79A93C52C769726569AE6E94C7B10125
              DB209E7276B632BC9C0E0D73D796DB7C036171F62B54DD7047DA55BF7AFCC9C1
              1B7A71FDE3D07E1D3FC2F875A8E5D6B45FB5DCE9BA7EA96D2DBDEC71C716F95A
              3DA8D19F3118A616E0F4C1E7AF15F318CE672827FCB1EFDBCFFE1BB1F4584B25
              36BF9A5DBBF97FC39E83E0A4B94F046922F645924FB3295651C043CA0E83909B
              41F71DFAD6E552B6B192CE6B78AD6658F4EB7B6F252D44796C8C6D62E4E70146
              318EE49278C5DAE7373ABF84FF00F237F893FEBC34FF00FD19775EA75E59F09F
              FE46FF00127FD7869FFF00A32EEBD4EB296E68B63E1EF1A4BA037C41F1643796
              77171A83EA5A8C50AC11373335C3796C5BCDE71D301075E77629249AE3FB5603
              FF00099DA31F224C4DE4C384F993E5EB8E7AFF00C04FBD64FC4BB3B88BE26F89
              A578888E6D56F25461CE53ED32264E3A7CCA473FD4574727FC251FDB56DBBFB2
              3CEFB3CBB31E6EDDBBA3DD9EF9CEDC7E35EE654AF19EFBC76BF7F26BE5BF958F
              1F32769436DA5BDBB79A7FD6F739F8249BFB2B5A03C496F1869E7DD098E3CDD7
              CBCB2F391BBA0C7E15AAED767518651E30B56558A45372208B6C6494F90F38CB
              60919E7E438EF54EDBFB73FB17C41E5FF67F93F68B9FB56EDFBB76DF9F676C63
              A67F1AD793FE128FEDAB6DDFD91E77D9E5D98F376EDDD1EECF7CE76E3F1AF528
              C7DC5A4BA7F37F33FEF7F5E4F53CDAD2F7FECF5FE5ECBFBBFD7E073FA94931F0
              C5DAB7892DEE54CEF9B358E30D27EF8FCC0839E4FCFC76F6AB3E6CFF00F0956E
              FF0084AED77FD8B1F6DF2A2DB8DFFEAF19C67BE7AD1AB7F6E7FC2237BF69FECF
              FB1FDA24F33CBDFE66EFB41CE33C6377E9EF573FE2A2FF0084CFFE617F6DFECF
              FF00A69E5F97E67E7BB3F862B3E57ED23F16D0FE6FEF7F7BEEF9DADADF4E65C9
              2F87797F2FF77FBBF7FE37E98DE1A4D2DB492619CC5E235B86FB02C50DC3CCEF
              B57C9111898057F33382431C91C71CFD49E06FF9279E1CFF00B055AFFE8A5AF8
              EACD6FE09A0BDB05B88E48DCC90CF086055A301CB2B0E85461891D3835F6F410
              456B6F1DBDB4490C312048E38D42AA28180001C00076AF9091F4E7827C70BEB5
              B7F88B6F06A9E649632E9513B4011DD5A459660AC55658F9019C0249C6E3C735
              C05ACD1368DAC9D33598F4EB2796730E9F3223492215F9464B6EC9185E33C8EF
              5E9DFB45DB5CCCBA20B286570C9712DD08509CAC5B36B3E3B279B2609FBBE637
              4DC73E61A07F6BFF00C219A9FD8BEC5F62FDEF9DE76FF33FD58DDB71C74C633D
              EBD1CB55EA3DF67B5FF468E1C73B535B6EB7B7EA996EFA59CEBDA593E2BB59D8
              79DB6E562882DBFCA3A80707774E6B36E1E53A0EB00EBB0C8A6F58B5B08D375C
              9DCBFBC073900F5C0E38ADED47FE122FF849347F3FFB2FED3FBFF236799B3EE0
              DDBB3CF4E98AC8B9FED7FF00846F5DF33EC5F66FED06FB4EDDFBFCCDE99D9DB6
              E71D79EB5ED6222EF53E2FB5FCDFC8B7F7BF3BE9DD68795424AD4FE1E9FCBFCE
              FF00BBFE5AF9EA6AC935C7F6AC07FE133B463E44989BC98709F327CBD71CF5FF
              00809F7AAEE6EA3D3C2C9E29B74CDE0610B43103CCF912F5CE0E449E98F6AD59
              3FE128FEDAB6DDFD91E77D9E5D98F376EDDD1EECF7CE76E3F1AA9FF1517F657F
              CC2FCAFED0FF00A69BBCCFB57FE83BFF001DBEF5D9386B3D25B3FE6ECBFBFF00
              D7E5C909691D63BAFE5EEFFBBFD7E74657D25FC4339F126AD6DAAAFF006737D9
              A628CB1ACDBFE456F20938EA4FB1FA57B5FC0C16A3C1FA97F67B40D07F699C1B
              759163CFD9E0DC14484B6036464F5EB81D2BC03C69FDA3FDB517F6BFD97CEFB3
              8DBF65DDB76EE6EBBB9CE73FA57BE7C07D3AE34AF04DFDA5E2EC99751DEC9820
              AEFB781C020804101B047AE6BE5730D31135E7D6F7FC5B7F89F4782D6845F974
              DBF0497E06FF00C52BB4D3FE1DDE5E4AD32A5BDCDA4AC60729200B75113B5810
              4371C10460F715F36C375A3CBAC69DFD8528D2665697CEBB9212A80141B46D79
              A419FBE3391F7871C57D39F116CADF50F86BE2186EE3F3235B096603711878D4
              BA1E3D1954FBE39E2BE57D0ECB52B0F155A43024716A1B58F93768E9E51DADC3
              8C0392BC8C647CC3DC0CB07AD782F35EBBF959FE26989FE0CDF93FC8DB90DD4D
              6FAB471F8A6DE769095F252188B5D93128C2E0E79FB9C771EB56249AE3FB5603
              FF00099DA31F224C4DE4C384F993E5EB8E7AFF00C04FBD3AE7FE122FB06BBE67
              F65F97F37DA76F999FF5099D9FF01C75EF9ED56E4FF84A3FB6ADB77F6479DF67
              97663CDDBB7747BB3DF39DB8FC6BEBD437D25BAFE6FE67FDFF00E9F9EA7CC396
              DAC767FCBFCABFBBFD7A6872D70F29D0758075D864537AC5AD8469BAE4EE5FDE
              039C807AE071C56979B3FF00C255BBFE12BB5DFF0062C7DB7CA8B6E37FFABC67
              19EF9EB552E7FB5FFE11BD77CCFB17D9BFB41BED3B77EFF337A67676DB9C75E7
              AD6BFF00C545FF00099FFCC2FEDBFD9FFF004D3CBF2FCCFCF767F0C571D38BE6
              5F17D9FE6EF2FEF7DDF3B5B5BF5D492E57F0FDAFE5ED1FEEFF005A6FA5B32FCD
              CCBE1ABB45F1143763CD918D9C7147BA402624BE41CE382FE98F6AD3D3F568B4
              7F1CE96FE00D4D60B7F3CCB33DAC336E8902BAB3C8B3EF43B21794EFC700B920
              0AA7AB7F6E7FC2237BF69FECFF00B1FDA24F33CBDFE66EFB41CE33C6377E9EF5
              B3F02E2BDB4F8BDE1D42F37D86ECCD2A901D6299D2D661DC00CC9BD973CE32D8
              383CF9799AB4E1BFC2B7BF9F76FF00CBC8F472F7784F6F89ED6F2EC97F9F99EF
              3ACF89BC56B0F80FC9B792CA7D59ADE4D4E38ED092AE5E0F320C38250796F70E
              47DF02127202B67E77F195DF86E3F18F8BA2D46C649AFCEA37EB1491C6CBB653
              3BEC62DE76081C71E58FC71CFD63E3AB89AD3E1DF88EE6D269209E1D2AEA48A5
              898AB230898860472083CE457C337F0EA0E23D4F51F3A4FED12F32DCCAC58CED
              BC87258F56DD9CE79E41EE2BCB3D23A28E49BED3A07FC5496EA5607DAFE5C7FE
              87FBB1F2B73CE7EEFCD8E944124DFD95AD01E24B78C34F3EE84C71E6EBE5E597
              9C8DDD063F0AB317F6E7DABC35B7FB3F7FD9DFEC99DF8DBE50CF99EFB71D3BD1
              6DFDB9FD8BE20F2FFB3FC9FB45CFDAB76FDDBB6FCFB3B631D33F8D7D4462EEFE
              2EBFCDFC91FEF7F4BCB45F39292B2F87A7F2FF003BFEEFF4FCF52E3B5D9D4619
              478C2D59562914DC8822DB19253E43CE32D824679F90E3BD656A524C7C3176AD
              E24B7B9533BE6CD638C349FBE3F3020E793F3F1DBDABA093FE128FEDAB6DDFD9
              1E77D9E5D98F376EDDD1EECF7CE76E3F1AC8D5BFB73FE111BDFB4FF67FD8FED1
              2799E5EFF3377DA0E719E31BBF4F7AE8C447DDA9A4B697F3765FDEFCEFFE5CF4
              25EF43E1DD7F2F77FDDFCAC1E6CFFF000956EFF84AED77FD8B1F6DF2A2DB8DFF
              00EAF19C67BE7AD47A62E9AFF6C68EF849E2517D29B1920827696793E5F28C46
              270AAC64CE386E48EC39D0FF008A8BFE133FF985FDB7FB3FFE9A797E5F99F9EE
              CFE18AE46E9754835F9EF615963BB8EEE59167B50C02C919DECC8DD46DE1B3D4
              0C1AF2F3256A7D7E27BDFF0056F5FC7CCF4B00EF3E9F0ADADFA25A7E1E47D73E
              06FF009279E1CFFB055AFF00E8A5AF21F8DD3470F8F226D4A2B8974AFECD804A
              AB149245E7F9B71E5960B2C6376DF376E58F1BF03A91EEF04115ADBC76F6D124
              30C481238E350AA8A06000070001DABC47F68F8E5F2F4336E8FB1C4CF72501C3
              043188CBE3AED3338527A798D8FBC73F3D1DCF68F34B7751A2EACFA7EAC9A658
              4D24EF6F613C685E58F18186639E40DBC67953C93566FA59CEBDA593E2BB59D8
              79DB6E562882DBFCA3A80707774E6AB6869ACC7E0FD512D56CD6D54CC2759C38
              941F2C6E031C74F5EF5ABA8FFC245FF09268FE7FF65FDA7F7FE46CF3367DC1BB
              7679E9D315F5B4637C3C1DA5F63F9ADF17F8ADE9A6FB59EA7CD5597EFE5F0FDA
              FE5FE5FF000DFD75F5BAD0C1B8794E83AC03AEC3229BD62D6C234DD72772FEF0
              1CE403D7038E2B7249AE3FB5603FF099DA31F224C4DE4C384F993E5EB8E7AFFC
              04FBD655CFF6BFFC237AEF99F62FB37F6837DA76EFDFE66F4CECEDB738EBCF5A
              E8A4FF0084A3FB6ADB77F6479DF6797663CDDBB7747BB3DF39DB8FC6B4A11777
              F16D1FE6FE697F7BEEF3DACF5715A4ADF67797F2FF002AFEEFF5D6EB43075537
              29E1ABA49BC450CC0CACDF63F2A3569419B21C1073839DE31C60FA558DDA2C9E
              2793FE12BD620D4A0FECF221B8589B624BBFE5056139381B8F51D7B7149AB7F6
              E7FC2237BF69FECFFB1FDA24F33CBDFE66EFB41CE33C6377E9EF593E34FED1FE
              DA8BFB5FECBE77D9C6DFB2EEDBB77375DDCE739FD2B8F1CAD49BB3DA1BF37797
              793F97E16EBD58377A8B6DE5B5BFBBD92F9FE373E81F822B629E15D546932C72
              DA7F6AB796F147246A7F710E70B23330E73D49FCAB4FE30DC4F67F0AB55B9B49
              A482785EDA48A5898AB46C2E2321811C8208C822B0BF67EB69AD3C037B1DC26C
              76D44C80641F95EDE1653C7AAB035D9F8F62127C3FD71F748925BD9C9750C914
              8C8D1CB10F32370CA41055D148FA57CD3F88F78F97A1BAD1E5D634EFEC294693
              32B4BE75DC9095400A0DA36BCD20CFDF19C8FBC38E2AFC86EA6B7D5A38FC536F
              3B484AF9290C45AEC9894617073CFDCE3B8F5AC4D0ECB52B0F155A43024716A1
              B58F93768E9E51DADC38C0392BC8C647CC3DC0E96E7FE122FB06BBE67F65F97F
              37DA76F999FF005099D9FF0001C75EF9ED5F4B97C6F866ED2DDEDCD6F87CA497
              E1F8687838E76C425A6CB7B7F379A6FF001FF31B24D71FDAB01FF84CED18F912
              626F261C27CC9F2F5C73D7FE027DEB0EE1E53A0EB00EBB0C8A6F58B5B08D375C
              9DCBFBC073900F5C0E38AEA64FF84A3FB6ADB77F6479DF6797663CDDBB7747BB
              3DF39DB8FC6B9DB9FED7FF00846F5DF33EC5F66FED06FB4EDDFBFCCDE99D9DB6
              E71D79EB5D98B8B4A5F16D2DF9BF957793F9F96F75A1CB86926E3F0EF1FE5FE6
              F28AFEBCF535649AE3FB5603FF00099DA31F224C4DE4C384F993E5EB8E7AFF00
              C04FBD666A524C7C3176ADE24B7B9533BE6CD638C349FBE3F3020E793F3F1DBD
              ABA093FE128FEDAB6DDFD91E77D9E5D98F376EDDD1EECF7CE76E3F1AC8D5BFB7
              3FE111BDFB4FF67FD8FED12799E5EFF3377DA0E719E31BBF4F7AD31107CB53E2
              DA5FCDD96FEF7E77F991424B9A9FC3BAFE5EEFFBBFE4745E1F9FC349E30B2B6F
              0B5C27932DC890431FDA549DB6F3862E243B1B0586D200237B0F527D32BC1BE1
              B4520F1EE95294611B34CAAE47CA48858900FA8DC3F31EB5EF35F271D8FA696E
              741F0FBFE44F5FFAFF00BEFF00D2B9ABA6AE67E1F7FC89EBFF005FF7DFFA5735
              74D5CAF7355B05145148028A28A00C2F1CFF00C93CF11FFD82AEBFF4535507D5
              B59D53E1AFC518F5F7919EC6E754B5B559211194B616E1A2180064157C863924
              107278ABFE39FF009279E23FFB055D7FE8A6AE83E217FC931F147FD81EEFFF00
              44BD69103B5A28A2A84145145001451450079B7ECF9FF2427C3DFF006F3FFA53
              2D7A4D79B7ECF9FF002427C3DFF6F3FF00A532D7A4D001451450014514500145
              145007CC7E1CB682CF5AF185AD9C31C16F0F896F638A28902A46A1800AA07000
              03000ADEACB9B41D1EF759D6EE2F34AB1B899F5AD437492DB233362EE503248C
              F4147FC22FA07FD00F4DFF00C048FF00C2B55B19BDCD4ADBF87BFF00254EDBFE
              C0B79FFA3ED2B90FF845F40FFA01E9BFF8091FF851FF0008BE81FF00403D37FF
              000123FF000A6D5D0968CE4BE33EA16517C5FF0019E9DA95FDDD9C17AD605BEC
              B62970CE12DD0E09695368C9078C938EDDF12EBC2BA699D42E83AC2EC0437D93
              C3F72D1C99C107325D86E318E83A9EBC1AF48FF845F40FFA01E9BFF8091FF856
              57FC213A7FFC25FF006FFECDD37FB37EC1E4FD9BC85FF5DE66EDFB76EDFBBC67
              AD4A524EE8ABC5EFF9D8E0EEFC3BA2D858BDEDF69DAB5B5B210AE66D067465CF
              4249BAD817240FBD9CF6EF5D8597C01B6D7F4AB1D56C7C42B696F796D1CF1C63
              4E6C8575DC376676E70403838E2BA1FF00845F40FF00A01E9BFF008091FF0085
              1FF08BE81FF403D37FF0123FF0AA939C959DBEE44A518BBABFDED9CE6A7F03ED
              F42B7B189AF3FB56E2F2E0DBC490D81123B6C79093BAED1000B1B7E9D6B0B5BF
              82BE21DB0B683A25EBC8CEE6613C96D0C6AB85D8107DA246273BC925BBAE0715
              E81FF08BE81FF403D37FF0123FF0A3FE117D03FE807A6FFE0247FE149F335676
              FB902B277FD59E7DA67C1FF16E228EFB41974E780F9897F6334534ECE0E402AD
              72880739C8E7E51EA4D7BA7C3B81ADBC136F0CB2CD34D1DCDD2CEF3C2913997E
              D326FCAA3328F9B70E1883D7DAB8CFF845F40FFA01E9BFF8091FF851FF0008BE
              81FF00403D37FF000123FF000ACDD36FA97CE761F131ADE3F877A94D777EFA74
              7018661731DBF9ECAE932320084804960ABC9039C9E335E0369A2E9DA9E8CD73
              F67D6B57131758EF1B4299E704924B02974236018B1CB12DB8904115E9DFF08B
              E81FF403D37FF0123FF0A3FE117D03FE807A6FFE0247FE14D41AEA1CC9EE79A2
              784B4F95C22E8DADC44FF1CFE1EB8083EA56E98FE40FF5AD2F07FC37F0FF00C4
              3B1BD3A16AFF006392D1D1651369922C8A1B9561FE92CA41DAC3D783C0E09EE7
              FE117D03FE807A6FFE0247FE147FC22FA07FD00F4DFF00C048FF00C2B5729B56
              D3EE5FE467CB04EEAFF7B395D63E02DD681A74D7D0EADA65FDB4113CD72FA843
              3C1E4A20C92A22662DC673F418CE6B843A0DAEB3E1BFB6681637F3DD2B2A3C56
              DA5CCCA24C02CBE679AEBB403C1C6E271F280735ECBFF08BE81FF403D37FF012
              3FF0A3FE117D03FE807A6FFE0247FE152B9947974FB97E7B8ED1E6E677FBDFFC
              31E27A6F86B5C8252975E10B9B9591946FBAB4BA0221DCFEEC838E79E0F4E3DF
              BEF035FDB5CF89AE6DBFB4B53BABC816E65923BED392DB6349246641F2CCF8F9
              973B428E59B9ED5D77FC22FA07FD00F4DFFC048FFC28FF00845F40FF00A01E9B
              FF008091FF00854A8B4EE6929A6AC8BA6C6D8EA4BA81857ED6B11804A3EF6C24
              36D3EA3201E7A738EA6A7AE4E7F0258BF8BADB5282DB4F8F4F4B668A7B236685
              646C92AC38C03CF5EBF201C8638D8FF845F40FFA01E9BFF8091FF855999DFF00
              C27FF91BFC49FF005E1A7FFE8CBBAF53AF9B7FE117D03FE807A6FF00E0247FE1
              47FC22FA07FD00F4DFFC048FFC2A5C6ECA52388F103DB6ABF1435AB4373A8CD3
              D96A77E3EC7169924D02A9BB91F7978A71203B8A7CCAA390A0E40C9CA7F0B69A
              D348E340F10AAB104463C3B3613803033799ED9E49E49ED803D33FE117D03FE8
              07A6FF00E0247FE147FC22FA07FD00F4DFFC048FFC29AE68BD1FE1713E47BA7F
              7D8F33B4F0CF86E5F12586897CF71A7DD6A0CAB0FDA746B8400B121430372181
              2C36E5432E4FDEC648ED3FE19B97ED1E67FC24C9B37EEF2BFB39B6E33F77FD76
              71DBAE7DEB63FE117D03FE807A6FFE0247FE147FC22FA07FD00F4DFF00C048FF
              00C29C9CE5BDBEE424A31DAFF7B297FC33D5B7FD052CBFF0026FFE49AE4BC51F
              053C416BE2E98785742379A3C421F24CF7918F3B11AEFDD9756197DD9C6DEBC6
              062BBAFF00845F40FF00A01E9BFF008091FF00851FF08BE81FF403D37FF0123F
              F0A52E696F6FB92FC82368EDF9DFF330F40F839E22D434A7FB7DE45E132AD342
              2C2DA1FB4ACA92220772E666FBDB40DA4F1E582304E6BDDABC97FE117D03FE80
              7A6FFE0247FE147FC22FA07FD00F4DFF00C048FF00C2B3F66DF52F9EE45F1E6E
              6CADD7465BED52FF004F17115D407EC566939950984B2B1695368F957A672320
              F1D7CDE6F085801683FB275D3E5A7CE60F0F4E7CEC8C7EF37DC8C38C670985C9
              EA4703D37FE117D03FE807A6FF00E0247FE147FC22FA07FD00F4DFFC048FFC2A
              945AD839A2F73CBAE7C3BA2D959CB797BA76AD696F0E37B5CE833A139381B4FD
              A8AF5233B997AF193C5753A37C0ED3BC5DA1D9EB7A3789160B4BA8F2A834C704
              10C43060D3B720820E0E38E33D4F51FF0008BE81FF00403D37FF000123FF000A
              C5BDF02DACDE2DD36FEDECF4D4D32DE3916E2CFECA8BBD8A901B85C3724707A6
              DC8EA6AE4E725676FB92212827757FBDB2D41FB3B41142A926B56B3B0EB24961
              2863FF007CDC01FA560F8CFE05EB36EB62DE1A4FED699B7ADC95758123450823
              004B2B127EF8E1B180A303193D77FC22FA07FD00F4DFFC048FFC28FF00845F40
              FF00A01E9BFF008091FF008526E6E3CB756F4409454B9BF56715E1DF823E21D4
              84961E20D2AD7444588B2EA7BFED1233EF184D8B3ED1C13CEDE83D4E6BDB7C19
              E18B8F0B697716D79A97F694D3CEB2B4FE498C9C431C4320B312711E49CF249E
              2B87FF00845F40FF00A01E9BFF008091FF00851FF08BE81FF403D37FF0123FF0
              ACFD9BEE5F39E81E3211378135E172EF1C274DB8123C681D957CA6C90A4804E3
              B6467D457CDB6BA5E99AEC177A82AEADABF9A16DC4917879B7DB9540014586E1
              62002EDE092781F2E324FA97FC22FA07FD00F4DFFC048FFC28FF00845F40FF00
              A01E9BFF008091FF0085354DAEA1CCBA9E689E12D3E5708BA36B7113FC73F87A
              E020FA95BA63F903FD6B47C27F0D746F1E1D4EDB4BBD9B4D9F4E6449FED5A5BA
              3AB316E00FB530C8D841C8E3F9775FF08BE81FF403D37FF0123FF0A3FE117D03
              FE807A6FFE0247FE15AB94DAB69F72FF00233E5827757FBD9936DFB382412169
              7C4715C8231B25D3D801EFF2CC0D4977FB3FC3058DD4D05D5BDE4F1DB4C60B68
              ADE484CB298984637B4EC061CAB7231F2E0F04D697FC22FA07FD00F4DFFC048F
              FC28FF00845F40FF00A01E9BFF008091FF00854AE651E5D3EE5F9EE3F75BE6FD
              59E6365F07BC756B3179FC2705EA95C08E7BE8C283EBF24CA73F8E39AEAF5DF0
              A6B1F08A0D035CB2F10C7A8B59DDC967690C96896E21FB44726F7F3199864751
              BC150704820107A3FF00845F40FF00A01E9BFF008091FF00851FF08BE81FF403
              D37FF0123FF0A8E47DCBE635BE2DF8CFC45A77C2BD2E59EDA3B36D674975D463
              920650B348912183E6395389A670B9DD98472555C1F06D0BFB335A5D26C2F2E6
              F67B8B58E548EC069AF34072CEE4E629C484E0E72A83EE80720135EC7FF08BE8
              1FF403D37FF0123FF0A3FE117D03FE807A6FFE0247FE14F958B991E66FE16D35
              A691C681E215562088C78766C27006066F33DB3C93C93DB0011F863421ACE9BA
              65E437D6936A73A436ED3E8B346A0B305C36FBA53905867686032304D765AC78
              16D6F35CD1EE74DB3D36D2D6D6476BB8FECA87CE53B70BB76E0F42327EEE7239
              ADAFF845F40FFA01E9BFF8091FF8569194D2B69F72FF0022651A6F5D7EF663BF
              ECDECDE62A78AFCB8DDF798974F3B4119C7598F4C903393CD5BFF867AB6FFA0A
              597FE004DFFC93577FE117D03FE807A6FF00E0247FE147FC22FA07FD00F4DFFC
              048FFC2A62E71EABEE4C1A8BFF0087679FF89FE0878A2DF5F961F0DE93F6BD3E
              38A154B8FB4C49E73889048FB5E4CAE5F79C76CF1C62BA4D03E0E788B50D29FE
              DF791784CAB4D08B0B687ED2B2A48881DCB999BEF6D03693C79608C139ADCFF8
              45F40FFA01E9BFF8091FF851FF0008BE81FF00403D37FF000123FF000A8E47DC
              BE73D6ABC93E3BCB6105A692D7DAD5E69CD325C5BF9567642779E36F299F25A4
              40AA0A203CE4EEC6319A5FF845F40FFA01E9BFF8091FF851FF0008BE81FF0040
              3D37FF000123FF000A4A9B4F70E63CD6EFC2B672CB6F2CBA56BAF2052D2B41E1
              D9CF9C587FCB4DD7230E0E4E23C2E4F5238105CF87745B2B396F2F74ED5AD2DE
              1C6F6B9D067427270369FB515EA467732F5E3278AF51FF00845F40FF00A01E9B
              FF008091FF00851FF08BE81FF403D37FF0123FF0AD62E71D15BEE4FF003264A0
              FBFDECE6347F821A6F8BFC3F65ACE8BE214B7B5B942CB8D3240D90769560D3B7
              2ACA471C7B91835AF07ECED0450AA49AD5ACEC3AC92584A18FFDF37007E95A1F
              F08BE81FF403D37FF0123FF0A3FE117D03FE807A6FFE0247FE14BDFE6E6D3EE5
              F96C2B46DCBAFDECE4FC5DF00F54B4B3B597C2FE56A5706465B8890FD9C2AE01
              4602491B3CEE04EFEEB85E09AA5A0FC13F136A2CBA7EB7A3D968D0EC2CDAA34C
              679739C8511A4DB493D3900632739C67B9FF00845F40FF00A01E9BFF008091FF
              00851FF08BE81FF403D37FF0123FF0A971937765464A2AC8EE3C19E18B8F0B69
              7716D79A97F694D3CEB2B4FE498C9C431C4320B312711E49CF249E2AC78C844D
              E04D785CBBC709D36E048F1A07655F29B242920138ED919F515E7FFF0008BE81
              FF00403D37FF000123FF000AC7D6BC0963A85F6952585B69F676F6B7225BA896
              CD333A8E42E40E9C6083C10D9E768063D9BDEE3E6470F69A4E9BE20B7D475569
              B58D6A59DFCB695B4295E68DF1C94F2AE047C065E1CF0070B8E1A6B2F09D8ACD
              02AE9BAFC32291FE933F87A750A47F11F2EE89CF1D97AFA76F4AFF00845F40FF
              00A01E9BFF008091FF00851FF08BE81FF403D37FF0123FF0AD63CF0774FF0004
              C99284959DFEF3CFFC31E00F0FF8D754BED174ED4DB4FD46CD774826D2A55242
              B8562B9B93D090087553F374E0E3AAB6FD9C12090B4BE238AE4118D92E9EC00F
              7F96606B5BFE117D03FE807A6FFE0247FE147FC22FA07FD00F4DFF00C048FF00
              C287CCE5CDA7DC849452B6BF7B336EFF0067F860B1BA9A0BAB7BC9E3B698C16D
              15BC90996531308C6F69D80C3956E463E5C1E09AE0ECBE0F78EAD662F3F84E0B
              D52B811CF7D18507D7E4994E7F1C735E9DFF0008BE81FF00403D37FF000123FF
              000A3FE117D03FE807A6FF00E0247FE14A4A52777F95BF22A2D4569FE7F998C9
              F0AE6F01EADA0DD36BDFDA109BE9A3483EC622DAD25B4859B76F627885463FC9
              EB6B2FFE117D03FE807A6FFE0247FE147FC22FA07FD00F4DFF00C048FF00C29C
              53484DDCED3E1F7FC89EBFF5FF007DFF00A573574D5E4BFF0008BE81FF00403D
              37FF000123FF000A3FE117D03FE807A6FF00E0247FE159BA7765731EB545792F
              FC22FA07FD00F4DFFC048FFC28FF00845F40FF00A01E9BFF008091FF00852F65
              E61CE7AD515E4BFF0008BE81FF00403D37FF000123FF000A3FE117D03FE807A6
              FF00E0247FE147B2F30E73BDF1CFFC93CF11FF00D82AEBFF0045356647AFEA5A
              F7C31F8A5FDAD73F68FECFBCD5EC6DBF76ABE5C31C3F227CA067193C9C9F7AE0
              35CF04E9F7DFD9DFD99A6E9B6FE45FC535C7EE153CC8573B9385E7391C1E0D3F
              C47E1CD0E0F0AEAB2C1A369F1C91D94CC8E96A819484241040E0D350B0731F40
              7FC251ABFF00D089E20FFBFF00A7FF00F2551FF0946AFF00F4227883FEFF00E9
              FF00FC955D1EE1EB46E1EB48A39CFF0084A357FF00A113C41FF7FF004FFF00E4
              AA3FE128D5FF00E844F107FDFF00D3FF00F92ABA3DC3D68DC3D68039CFF84A35
              7FFA113C41FF007FF4FF00FE4AA3FE128D5FFE844F107FDFFD3FFF0092ABA3DC
              3D68DC3D680397F8636D05AFC27F0AC76B0C70A36936D2158D0282CF12B33607
              72CC493DC927BD7535CDFC38FF009259E14FFB02D9FF00E884AE92800A28A280
              0A28A2800A28A2803E7C8FFE421AC7FD86B51FFD2C9AA5AF36D5E6B97F1C78B1
              7EDD7D1A47AFDF2A245772C6AA3CE63C05603A927F1A87371FF411D4BFF0613F
              FF00175A27A19B5A9E9F4579866E3FE823A97FE0C27FFE2E8CDC7FD04752FF00
              C184FF00FC5D3B858F4FA2BCC3371FF411D4BFF0613FFF0017466E3FE823A97F
              E0C27FFE2E8B858F4FA2BCA5EF244B192E8DFEADE5A233906F6756C0EBC161E9
              57FF00B27C49FF003E9AE7FE0D87FF001FA2E163D1E8AF2E0D7A9713C17373AB
              5BCF0384923935190904A861CAC847461DE9229A69A30E9A86A801E81EF6E14F
              E45B345C2C7A9515E619B8FF00A08EA5FF008309FF00F8BA3371FF00411D4BFF
              000613FF00F1745C2C7A7D15E619B8FF00A08EA5FF008309FF00F8BA3371FF00
              411D4BFF000613FF00F1745C2C7A7D15E582E243726DC6A5A9F9810391F6E9F0
              01381CEEC763C7B54B6B06B17E66FECF1ACDCA432796EE9AA328DDB4363E6941
              E8C3B5170B1E9D457984F1EA96773141A836B168D3233C664D4DD83052A0FDC9
              4FF7875A679D379DE57F686A9BB6EECFDB6E36E3EBBB19F6EB45C2C7A9515E61
              9B8FFA08EA5FF8309FFF008BA3371FF411D4BFF0613FFF001745C2C7A7D15E61
              9B8FFA08EA5FF8309FFF008BA3371FF411D4BFF0613FFF001745C2C7A7D15E61
              9B8FFA08EA5FF8309FFF008BA3371FF411D4BFF0613FFF001745C2C7A7D15E61
              9B8FFA08EA5FF8309FFF008BA64B34D0C65DF50D5081D425EDC31FC83668B858
              F52A2BCB8B5EBDC4105B5CEAD713CEE5238E3D4640490A58F2D201D14F7AB5FD
              93E24FF9F4D73FF06C3FF8FD170B1E8F45794A5E48F631DD0BFD5BCB7457005E
              CECD83D380C7D6A7CDC7FD04752FFC184FFF00C5D170B1E9F4579866E3FE823A
              97FE0C27FF00E2E8CDC7FD04752FFC184FFF00C5D170B1E9F4579866E3FE823A
              97FE0C27FF00E2E8CDC7FD04752FFC184FFF00C5D170B1E9F45796417125CC22
              58752D4CA924737D3A9C8382305B239149036A772D6A90B6AA64BB884F044DAC
              85924423218219F38C03DBB1F4A2E163D528AF2E0D7A9713C17373AB5BCF0384
              923935190904A861CAC847461DEA3B6BB7BB8CC90EA3AA150C54EEBDB85E475E
              0B0A2E163D568AF30CDC7FD04752FF00C184FF00FC5D19B8FF00A08EA5FF0083
              09FF00F8BA2E163D3E8AF30CDC7FD04752FF00C184FF00FC5D19B8FF00A08EA5
              FF008309FF00F8BA2E163D3E8AF2DF3A6F3BCAFED0D5376DDD9FB6DC6DC7D776
              33EDD6A4B58358BF337F678D66E52193CB774D51946EDA1B1F34A0F461DA8B85
              8F4EA2BCC6EA0D62C0C3FDA0359B649A4F2D1DF54661BB696C7CB293D14F6A8F
              CE9BCEF2BFB4354DDB7767EDB71B71F5DD8CFB75A2E163D4A8AF30CDC7FD0475
              2FFC184FFF00C5D19B8FFA08EA5FF8309FFF008BA2E163D3E8AF30CDC7FD0475
              2FFC184FFF00C5D19B8FFA08EA5FF8309FFF008BA2E163D3E8AF2D9669A18CBB
              EA1AA103A84BDB863F906CD296BD7B8820B6B9D5AE279DCA471C7A8C809214B1
              E5A403A29EF45C2C7A8D15E71FD93E24FF009F4D73FF0006C3FF008FD504BC91
              EC63BA17FAB796E8AE00BD9D9B07A7018FAD170B1EAD4579866E3FE823A97FE0
              C27FFE2E8CDC7FD04752FF00C184FF00FC5D170B1E9F4579866E3FE823A97FE0
              C27FFE2E8CDC7FD04752FF00C184FF00FC5D170B1E9F4579866E3FE823A97FE0
              C27FFE2EA1B6BB7BB8CC90EA3AA150C54EEBDB85E475E0B0A2E163D568AF33B5
              B2D7EF6CE1BAB6B7D71E19E359236FED5C6E52320E0CD9E86A20D7A9713C1737
              3AB5BCF0384923935190904A861CAC847461DE8B858F51A2BCB21B89270E62D4
              B53211CA126FA71C83838CB73CF715266E3FE823A97FE0C27FFE2E8B858F4FA2
              BCC3371FF411D4BFF0613FFF0017466E3FE823A97FE0C27FFE2E8B858F4FA2BC
              C3371FF411D4BFF0613FFF00174CF3A6F3BCAFED0D5376DDD9FB6DC6DC7D7763
              3EDD68B858F52A2BCCADEDF57BEF30E9C758BB489FCB91E3D4D942B60363E794
              1E8C0F1C73497506B16061FED01ACDB24D27968EFAA330DDB4B63E5949E8A7B5
              170B1E9D45796F9D379DE57F686A9BB6EECFDB6E36E3EBBB19F6EB4FCDC7FD04
              752FFC184FFF00C5D170B1E9F4579866E3FE823A97FE0C27FF00E2E8CDC7FD04
              752FFC184FFF00C5D170B1E9F4579866E3FE823A97FE0C27FF00E2E992CD3431
              977D43542075097B70C7F20D9A2E163D4A8AF2E2D7AF710416D73AB5C4F3B948
              E38F51901242963CB4807453DEAD7F64F893FE7D35CFFC1B0FFE3F45C2C7A3D1
              5E5297923D8C7742FF0056F2DD15C017B3B360F4E031F5A9F371FF00411D4BFF
              000613FF00F1745C2C7A7D72DF117FB5BFE1109FFB17FDAFB5FDCFF8F7F2DF7F
              DEFC3A73E95CCE6E3FE823A97FE0C27FFE2EAA6AA67FEC7BDDD7FA838FB3BE55
              EFA6653F29E082D823D8D0D8247D7161AE5B6A9A6DB5FD8CBE6DADD42934326D
              2BB918065383823208E0F3563EDDEF5E71E08D4847F0FF00C3C99FBBA5DB0FFC
              84B5BBFDAA3FBD591A1D57DBBDE8FB77BD72BFDAA3FBD47F6A8FEF500755F6EF
              7A3EDDEF5CAFF6A8FEF51FDAA3FBD401BDF0E3FE4967853FEC0B67FF00A212BA
              4AE6FE1C7FC92CF0A7FD816CFF00F4425749400514514005145140051451401F
              316ABF0B3C747C5FE23BBB7F0D4D716D7DAC5D5D5BCB1DE5B00F1BC84A9C34A0
              8C8E704541FF000ABFC7DFF429DD7FE06DA7FF001EAFA928A77158F96FFE157F
              8FBFE853BAFF00C0DB4FFE3D47FC2AFF001F7FD0A775FF0081B69FFC7ABEA4A2
              8BB0B23E5BFF00855FE3EFFA14EEBFF036D3FF008F51FF000ABFC7DFF429DD7F
              E06DA7FF001EAFA928A2EC2C8F940FC24F889269525A5CF872EA59648D91A6FB
              4D9AF5CF3B44DDB3FA569FFC23BF1BBFE84DD37FF02E1FFE48AFA728A2EC2C8F
              95D7E1A7C4AB9B8B8BCD4FC26FF6AB9903BADBDE5A845C22A0033393D141FC69
              B6BF0AFE22456CA973E1ABAB89467749F6AB35DDCFA09ABEAAA28BB0B1F2DFFC
              2AFF001F7FD0A775FF0081B69FFC7A8FF855FE3EFF00A14EEBFF00036D3FF8F5
              7D49451761647CB7FF000ABFC7DFF429DD7FE06DA7FF001EA3FE157F8FBFE853
              BAFF00C0DB4FFE3D5F525145D8591F2C0F85BF1044CCC7C2D7450A8013ED967C
              1E7273E777C8FCA96CFC0DF17B4792E9349F085B3DBCF289BFD26F202E0EC553
              F767031F2D7D4D451761647CB171E00F8B1ABEA104FACF846155B78A448C5A5E
              5B824B94273BA73FDCA07C2DF88226663E16BA2854009F6CB3E0F3939F3BBE47
              E55F53D145D8591F2DFF00C2AFF1F7FD0A775FF81B69FF00C7A8FF00855FE3EF
              FA14EEBFF036D3FF008F57D49451761647CB7FF0ABFC7DFF00429DD7FE06DA7F
              F1EA3FE157F8FBFE853BAFFC0DB4FF00E3D5F525145D8591F2DFFC2AFF001F7F
              D0A775FF0081B69FFC7A8FF855FE3EFF00A14EEBFF00036D3FF8F57D49451761
              647CB7FF000ABFC7DFF429DD7FE06DA7FF001EA649F0B7E20B2811F85AE90EE5
              24FDB2CCE402091FEBBB8C8FC6BEA7A28BB0B23E5693E1A7C4CB5BAB5BCD2FC2
              44DCDB4A5D45CDEDAEC20A321FBB367F8AAEFF00C23BF1BBFE84DD37FF0002E1
              FF00E48AFA728A2EC2C8F9453E137C43874D86DADFC3370924488A243756841D
              B8CF1E7F703D7BD5AFF855FE3EFF00A14EEBFF00036D3FF8F57D4945170B1F2D
              FF00C2AFF1F7FD0A775FF81B69FF00C7A8FF00855FE3EFFA14EEBFF036D3FF00
              8F57D49451761647CB7FF0ABFC7DFF00429DD7FE06DA7FF1EA3FE157F8FBFE85
              3BAFFC0DB4FF00E3D5F525145D8591F2C47F0B7E20AA912785AE9CEE620FDB2C
              C6012481FEBBB0C0FC29961E06F8CBA7DB47043E0FB3645B686D995F508CACA9
              0B6E8CB27DA369656F9836320938C57D5545176163E568FE1A7C4CBABABABCD5
              3C2445CDCCA1D85B5EDAEC0022A0FBD367F869D1FC2DF882AA449E16BA73B988
              3F6CB318049207FAEEC303F0AFA9E8A2EC2C8F96FF00E157F8FBFE853BAFFC0D
              B4FF00E3D47FC2AFF1F7FD0A775FF81B69FF00C7ABEA4A28BB0B23E40D43C39E
              2DD32FE4B4B9F057885E58F1B9ADAC4DC467201E248CB29EBD89C1E3A8AABFD9
              7E27FF00A11FC53FF82897FC2BEC9A28BB0B23E3AD3FC31E33D4F5B8E0B6F08F
              8851648CAAC773A7FD9E30C32DB8CB21551C0C609193EE6BA0B4F02FC5CD19EE
              1347F08DBBC1712F9CDF6ABBB7DCADB157036CF8C6101FC4D7D4B451761647CB
              379E06F8BDAC496A9AB7842D92DE094CDFE8D79007276328FBD3918F9AB2757F
              04F8E344BC864BCF0A6B52C1346CAB0D95BC77786046599A1762BC1C00719E71
              D0D7D79451761647C6DFD97E27FF00A11FC53FF82897FC28FECBF13FFD08FE29
              FF00C144BFE15F64D145D8591F2DFF00C2AFF1F7FD0A775FF81B69FF00C7A8FF
              00855FE3EFFA14EEBFF036D3FF008F57D49451761647CAB75F0AFE224B6CC96D
              E1ABAB794E36C9F6AB36DBCFA19A964F869F132D6EAD6F34BF0913736D297517
              37B6BB0828C87EECD9FE2AFAA68A2EC2C8F98FFE11DF8DDFF426E9BFF8170FFF
              00245660F849F1123D2A3B4B6F0E5D452C71AA2CDF69B36E98E7699BBE3F5AFA
              BE8A2EC2C8F96FFE157F8FBFE853BAFF00C0DB4FFE3D47FC2AFF001F7FD0A775
              FF0081B69FFC7ABEA4A28BB0B23E5BFF00855FE3EFFA14EEBFF036D3FF008F51
              FF000ABFC7DFF429DD7FE06DA7FF001EAFA928A2EC2C8F96FF00E157F8FBFE85
              3BAFFC0DB4FF00E3D4C8FE16FC4155224F0B5D39DCC41FB6598C024903FD7761
              81F857D4F451761647CB761E11F8D5A769B6D650783F4F68EDA24850BDDC2588
              5000CE27EBC5451FC34F89975757579AA78488B9B9943B0B6BDB5D8004541F7A
              6CFF000D7D53451761647CB11FC2DF882AA449E16BA73B9883F6CB318049207F
              AEEC303F0A7FFC2AFF001F7FD0A775FF0081B69FFC7ABEA4A28BB0B23E5BFF00
              855FE3EFFA14EEBFF036D3FF008F51FF000ABFC7DFF429DD7FE06DA7FF001EAF
              A928A2EC2C8F96FF00E157F8FBFE853BAFFC0DB4FF00E3D519F85BF10FED21FF
              00E117B8108423CBFB55A64B67AE7CFF004EDEF5F545145D8591F2D5A7817E2E
              68CF709A3F846DDE0B897CE6FB55DDBEE56D8AB81B67C63080FE2692F3C0DF17
              B5892D5356F085B25BC1299BFD1AF200E4EC651F7A7231F357D4D451761647CB
              07E16FC41332B0F0B5D040A414FB659F278C1CF9DDB07F3A7FFC2AFF001F7FD0
              A775FF0081B69FFC7ABEA4A28BB0B23E5BFF00855FE3EFFA14EEBFF036D3FF00
              8F51FF000ABFC7DFF429DD7FE06DA7FF001EAFA928A2EC2C8F96FF00E157F8FB
              FE853BAFFC0DB4FF00E3D50DD7C2BF8892DB325B786AEADE538DB27DAACDB6F3
              E866AFAAA8A2EC2C8F95DBE1A7C4AB6B8B7BCD33C26FF6AB690BA2DC5E5A946C
              A321071383D189FC2AE7FC23BF1BBFE84DD37FF02E1FFE48AFA728A2EC2C8F94
              07C24F8891E951DA5B7872EA29638D5166FB4D9B74C73B4CDDF1FAD5BFF855FE
              3EFF00A14EEBFF00036D3FF8F57D4945170B1F2DFF00C2AFF1F7FD0A775FF81B
              69FF00C7AA0BDF853F102E74FB8813C27701A5899149BDB4C648C7FCF6AFAAE8
              A2EC2C78169BA178CB42F0ADA4377E0FD59CE9F64892FD9E7B494B6C400EC559
              B7374E00193E99ACDFF84975BFFA113C69FF008267FF001AFA3E8A433E70FF00
              84975BFF00A113C69FF8267FF1A3FE125D6FFE844F1A7FE099FF00C6BE8FA280
              3E70FF0084975BFF00A113C69FF8267FF1ADBD3EDFC59A9D847776DE0CD71229
              33B56E5ADADE41824731C932B0E9DC0C8E7A1AF74A280313C15A7DD691E01F0F
              E9BA845E4DDD9E996D04F1EE0DB1D22556190483820F20E2B6E8A2800A28A280
              3FFFD9}
          end
          object mlSnEMG_1: TMyLed
            Left = 55
            Top = 315
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnEMG_1'
          end
          object mlSnEMG_2: TMyLed
            Left = 719
            Top = 454
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnEMG_2'
          end
          object mlSnEMG_3: TMyLed
            Left = 756
            Top = 86
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnEMG_3'
          end
          object mlSnEMG_4: TMyLed
            Left = 55
            Top = 88
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnEMG_4'
          end
          object mlSnSafeDoorFront: TMyLed
            Left = 135
            Top = 495
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnSafeDoorFront'
          end
          object mlSnSafeSlideDoorLeft: TMyLed
            Left = 135
            Top = 59
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnSafeSlideDoorLeft'
          end
          object mlSnSafeSlideDoorRight: TMyLed
            Left = 567
            Top = 63
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnSafeSlideDoorRight'
          end
          object mlSnSafeDoorLeft: TMyLed
            Left = 55
            Top = 159
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnSafeDoorLeft'
          end
          object mlSnSafeDoorRight: TMyLed
            Left = 716
            Top = 127
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnSafeDoorRight'
          end
          object mlSnSafeAuto6: TMyLed
            Left = 563
            Top = 495
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnSafeAuto6'
          end
          object bpSwMotorRelay: TBtnPanel
            Left = 36
            Top = 539
            Width = 80
            Height = 30
            BevelInner = bvRaised
            Caption = 'Motor Power'
            Color = 8404992
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -12
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentFont = False
            ParentShowHint = False
            ShowHint = True
            TabOrder = 0
            OnClick = BtnPanelClick
            TrueColor = 16744448
            FalseColor = 8404992
            FalseFontColor = clWhite
            Alias = 'SwMotorRelay'
            Style = tsButtons
          end
          object bpSwLight: TBtnPanel
            Left = 120
            Top = 539
            Width = 80
            Height = 30
            BevelInner = bvRaised
            Caption = 'Light'
            Color = 8404992
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -12
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentFont = False
            ParentShowHint = False
            ShowHint = True
            TabOrder = 1
            OnClick = BtnPanelClick
            TrueColor = 16744448
            FalseColor = 8404992
            FalseFontColor = clWhite
            Alias = 'SwLight'
            Style = tsButtons
          end
          object Panel173: TPanel
            Left = 41
            Top = 339
            Width = 50
            Height = 30
            BevelInner = bvLowered
            Caption = 'EMG 1'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 2
          end
          object Panel171: TPanel
            Left = 702
            Top = 479
            Width = 50
            Height = 30
            BevelInner = bvLowered
            Caption = 'EMG 2'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 3
          end
          object Panel172: TPanel
            Left = 743
            Top = 53
            Width = 50
            Height = 30
            BevelInner = bvLowered
            Caption = 'EMG 3'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 4
          end
          object Panel174: TPanel
            Left = 42
            Top = 52
            Width = 50
            Height = 30
            BevelInner = bvLowered
            Caption = 'EMG 4'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 5
          end
          object pnl1: TPanel
            Left = 233
            Top = 539
            Width = 50
            Height = 30
            BevelInner = bvLowered
            Caption = 'EMG '
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 6
          end
          object pnl2: TPanel
            Left = 161
            Top = 491
            Width = 80
            Height = 30
            BevelInner = bvLowered
            Caption = 'Door'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 7
          end
          object pnl3: TPanel
            Left = 161
            Top = 55
            Width = 80
            Height = 30
            BevelInner = bvLowered
            Caption = 'Door'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 8
          end
          object pnl4: TPanel
            Left = 593
            Top = 59
            Width = 80
            Height = 30
            BevelInner = bvLowered
            Caption = 'Door'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 9
          end
          object pnl5: TPanel
            Left = 53
            Top = 187
            Width = 32
            Height = 66
            BevelInner = bvLowered
            Caption = 'Door'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 10
          end
          object pnl6: TPanel
            Left = 713
            Top = 155
            Width = 32
            Height = 66
            BevelInner = bvLowered
            Caption = 'Door'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 11
          end
          object pnl9: TPanel
            Left = 589
            Top = 491
            Width = 80
            Height = 30
            BevelInner = bvLowered
            Caption = 'Door'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -13
            Font.Name = #26032#32048#26126#39636
            Font.Style = []
            ParentFont = False
            TabOrder = 12
          end
        end
        object Panel175: TPanel
          Left = 16
          Top = 592
          Width = 809
          Height = 62
          BevelInner = bvLowered
          Color = 12761254
          TabOrder = 1
          object mlSnSafeLock: TMyLed
            Left = 12
            Top = 6
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnSafeLock'
          end
          object Label1: TLabel
            Left = 42
            Top = 8
            Width = 73
            Height = 19
            Alignment = taCenter
            AutoSize = False
            Caption = 'Lock'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -19
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentColor = False
            ParentFont = False
          end
          object mlSnAirIsEnough: TMyLed
            Left = 12
            Top = 36
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnAirIsEnough'
          end
          object Label2: TLabel
            Left = 42
            Top = 38
            Width = 73
            Height = 19
            Alignment = taCenter
            AutoSize = False
            Caption = 'Air '
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -19
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentColor = False
            ParentFont = False
          end
          object mlSnIonFan_Power: TMyLed
            Left = 132
            Top = 6
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnIonFan_Power'
          end
          object mlSnIonFan_Balance: TMyLed
            Left = 132
            Top = 36
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
            Alias = 'SnIonFan_Balance'
          end
          object Label3: TLabel
            Left = 162
            Top = 38
            Width = 160
            Height = 19
            Alignment = taCenter
            AutoSize = False
            Caption = 'IonFan BalanceAlarm'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -19
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentColor = False
            ParentFont = False
          end
          object Label4: TLabel
            Left = 162
            Top = 8
            Width = 160
            Height = 19
            Alignment = taCenter
            AutoSize = False
            Caption = 'IonFan Power'
            Color = 9534289
            Font.Charset = ANSI_CHARSET
            Font.Color = clWhite
            Font.Height = -19
            Font.Name = 'Times New Roman'
            Font.Style = []
            ParentColor = False
            ParentFont = False
          end
        end
      end
    end
    object ts_IOTool: TTabSheet
      Caption = 'Tool'
      ImageIndex = 4
      object pn_IOTool: TPanel
        Left = 2
        Top = 0
        Width = 839
        Height = 660
        BevelInner = bvRaised
        BevelOuter = bvLowered
        Color = 12761254
        TabOrder = 0
        object pn_ToolTitle: TPanel
          Left = 3
          Top = 10
          Width = 835
          Height = 20
          BevelInner = bvLowered
          BevelOuter = bvSpace
          Caption = 'Tool'
          Color = 9534289
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -16
          Font.Name = #26032#32048#26126#39636
          Font.Style = []
          ParentFont = False
          TabOrder = 0
        end
        object pn_IOTool1: TPanel
          Left = 100
          Top = 57
          Width = 320
          Height = 360
          BevelInner = bvLowered
          Color = 12761254
          TabOrder = 1
          object lb_IOToolOBCard: TLabel
            Left = 20
            Top = 40
            Width = 34
            Height = 20
            Caption = 'Card'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object lb_IOToolOBPort: TLabel
            Left = 20
            Top = 70
            Width = 43
            Height = 20
            Caption = 'PORT'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object lb_IOToolOBOut: TLabel
            Left = 105
            Top = 15
            Width = 44
            Height = 20
            AutoSize = False
            Caption = 'OUT'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object lb_IOToolOBIn: TLabel
            Left = 215
            Top = 15
            Width = 44
            Height = 20
            AutoSize = False
            Caption = 'IN'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object ALedTool0: TALed
            Left = 270
            Top = 112
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool1: TALed
            Left = 270
            Top = 142
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool2: TALed
            Left = 270
            Top = 172
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool3: TALed
            Left = 270
            Top = 202
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool4: TALed
            Left = 270
            Top = 232
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool5: TALed
            Left = 270
            Top = 262
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool6: TALed
            Left = 270
            Top = 292
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool7: TALed
            Left = 270
            Top = 322
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ed_OutPort_1: TEdit
            Left = 80
            Top = 67
            Width = 85
            Height = 28
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 0
          end
          object ed_OutCard_1: TEdit
            Left = 80
            Top = 37
            Width = 85
            Height = 28
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 1
          end
          object ed_InCard_1: TEdit
            Left = 180
            Top = 37
            Width = 85
            Height = 28
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 2
          end
          object ed_InPort_1: TEdit
            Left = 180
            Top = 67
            Width = 85
            Height = 28
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 3
          end
          object cbToolBit0: TCheckBox
            Left = 80
            Top = 110
            Width = 65
            Height = 20
            Caption = 'Bit 0'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 4
          end
          object cbToolBit1: TCheckBox
            Left = 80
            Top = 140
            Width = 65
            Height = 20
            Caption = 'Bit 1'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 5
          end
          object cbToolBit2: TCheckBox
            Left = 80
            Top = 170
            Width = 65
            Height = 20
            Caption = 'Bit 2'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 6
          end
          object cbToolBit3: TCheckBox
            Left = 80
            Top = 200
            Width = 65
            Height = 20
            Caption = 'Bit 3'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 7
          end
          object cbToolBit4: TCheckBox
            Left = 80
            Top = 230
            Width = 65
            Height = 20
            Caption = 'Bit 4'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 8
          end
          object cbToolBit5: TCheckBox
            Left = 80
            Top = 260
            Width = 65
            Height = 20
            Caption = 'Bit 5'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 9
          end
          object cbToolBit6: TCheckBox
            Left = 80
            Top = 290
            Width = 65
            Height = 20
            Caption = 'Bit 6'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 10
          end
          object cbToolBit7: TCheckBox
            Left = 80
            Top = 320
            Width = 65
            Height = 20
            Caption = 'Bit 7'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 11
          end
          object cbToolLoop0: TCheckBox
            Left = 180
            Top = 110
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 12
          end
          object cbToolLoop1: TCheckBox
            Left = 180
            Top = 140
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 13
          end
          object cbToolLoop2: TCheckBox
            Left = 180
            Top = 170
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 14
          end
          object cbToolLoop3: TCheckBox
            Left = 180
            Top = 200
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 15
          end
          object cbToolLoop4: TCheckBox
            Left = 180
            Top = 230
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 16
          end
          object cbToolLoop5: TCheckBox
            Left = 180
            Top = 260
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 17
          end
          object cbToolLoop6: TCheckBox
            Left = 180
            Top = 290
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 18
          end
          object cbToolLoop7: TCheckBox
            Left = 180
            Top = 320
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 19
          end
        end
        object pn_IOTool2: TPanel
          Left = 425
          Top = 57
          Width = 320
          Height = 360
          BevelInner = bvLowered
          Color = 12761254
          TabOrder = 2
          object lb_IOToolOBCard1: TLabel
            Left = 20
            Top = 40
            Width = 34
            Height = 20
            Caption = 'Card'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object lb_IOToolOBPort1: TLabel
            Left = 20
            Top = 70
            Width = 43
            Height = 20
            Caption = 'PORT'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object lb_IOToolOBOut1: TLabel
            Left = 105
            Top = 15
            Width = 44
            Height = 20
            AutoSize = False
            Caption = 'OUT'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object lb_IOToolOBIn1: TLabel
            Left = 215
            Top = 15
            Width = 44
            Height = 20
            AutoSize = False
            Caption = 'IN'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object ALedTool8: TALed
            Left = 270
            Top = 112
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool9: TALed
            Left = 270
            Top = 142
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool10: TALed
            Left = 270
            Top = 172
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool11: TALed
            Left = 270
            Top = 202
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool12: TALed
            Left = 270
            Top = 232
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool13: TALed
            Left = 270
            Top = 262
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool14: TALed
            Left = 270
            Top = 292
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ALedTool15: TALed
            Left = 270
            Top = 322
            Width = 22
            Height = 22
            LEDStyle = LEDSqLarge
          end
          object ed_OutPort_2: TEdit
            Left = 80
            Top = 67
            Width = 85
            Height = 28
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 0
          end
          object ed_OutCard_2: TEdit
            Left = 80
            Top = 37
            Width = 85
            Height = 28
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 1
          end
          object ed_InCard_2: TEdit
            Left = 180
            Top = 67
            Width = 85
            Height = 28
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 2
          end
          object ed_InPort_2: TEdit
            Left = 180
            Top = 37
            Width = 85
            Height = 28
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 3
          end
          object cbToolBit8: TCheckBox
            Left = 80
            Top = 110
            Width = 57
            Height = 20
            Caption = 'Bit 0'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 4
          end
          object cbToolBit9: TCheckBox
            Left = 80
            Top = 140
            Width = 65
            Height = 20
            Caption = 'Bit 1'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 5
          end
          object cbToolBit10: TCheckBox
            Left = 80
            Top = 170
            Width = 65
            Height = 20
            Caption = 'Bit 2'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 6
          end
          object cbToolBit11: TCheckBox
            Left = 80
            Top = 200
            Width = 65
            Height = 20
            Caption = 'Bit 3'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 7
          end
          object cbToolBit12: TCheckBox
            Left = 80
            Top = 230
            Width = 65
            Height = 20
            Caption = 'Bit 4'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 8
          end
          object cbToolBit13: TCheckBox
            Left = 80
            Top = 260
            Width = 65
            Height = 20
            Caption = 'Bit 5'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 9
          end
          object cbToolBit14: TCheckBox
            Left = 80
            Top = 290
            Width = 65
            Height = 20
            Caption = 'Bit 6'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 10
          end
          object cbToolBit15: TCheckBox
            Left = 80
            Top = 320
            Width = 65
            Height = 20
            Caption = 'Bit 7'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 11
          end
          object cbToolLoop8: TCheckBox
            Left = 180
            Top = 110
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 12
          end
          object cbToolLoop9: TCheckBox
            Left = 180
            Top = 140
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 13
          end
          object cbToolLoop10: TCheckBox
            Left = 180
            Top = 170
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 14
          end
          object cbToolLoop11: TCheckBox
            Left = 180
            Top = 200
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 15
          end
          object cbToolLoop12: TCheckBox
            Left = 180
            Top = 230
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 16
          end
          object cbToolLoop13: TCheckBox
            Left = 180
            Top = 260
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 17
          end
          object cbToolLoop14: TCheckBox
            Left = 180
            Top = 290
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 18
          end
          object cbToolLoop15: TCheckBox
            Left = 180
            Top = 320
            Width = 57
            Height = 20
            Caption = 'Loop'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clBlack
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            TabOrder = 19
          end
        end
        object pn_IOTool3: TPanel
          Left = 100
          Top = 420
          Width = 645
          Height = 100
          BevelInner = bvLowered
          Color = 12761254
          TabOrder = 3
          object sbEnableIOChang: TSpeedButton
            Left = 414
            Top = 37
            Width = 161
            Height = 28
            AllowAllUp = True
            GroupIndex = 1
            Caption = 'Enable IO Change '
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clPurple
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
            OnClick = sbEnableIOChangClick
          end
          object lb_IOToolOB1: TLabel
            Left = 250
            Top = 21
            Width = 145
            Height = 20
            Caption = 'Loop Output Interval'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object lb_IOToolOB2: TLabel
            Left = 358
            Top = 53
            Width = 33
            Height = 20
            Caption = 'SEC'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ParentFont = False
          end
          object ComboBox1: TComboBox
            Left = 254
            Top = 49
            Width = 97
            Height = 28
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -16
            Font.Name = 'MS Sans Serif'
            Font.Style = []
            ItemHeight = 20
            ParentFont = False
            TabOrder = 0
            Text = '1'
            OnChange = ComboBox1Change
            Items.Strings = (
              '0.1'
              '0.3'
              '0.5'
              '0.7'
              '1'
              '2'
              '3')
          end
        end
      end
    end
    object ts_IODatabase: TTabSheet
      Caption = 'IODatabase'
      ImageIndex = 5
      object pn_IODatabase: TPanel
        Left = 0
        Top = 0
        Width = 840
        Height = 660
        BevelInner = bvLowered
        Color = 12761254
        TabOrder = 0
        object pnIOTableEditorToolbar: TPanel
          Left = 2
          Top = 2
          Width = 836
          Height = 48
          Align = alTop
          BevelOuter = bvNone
          Color = 12761254
          TabOrder = 0
          object lblIOType: TLabel
            Left = 10
            Top = 7
            Width = 24
            Height = 13
            Caption = 'Type'
          end
          object lblIOLane: TLabel
            Left = 125
            Top = 7
            Width = 24
            Height = 13
            Caption = 'Lane'
          end
          object lblIOSearch: TLabel
            Left = 212
            Top = 7
            Width = 59
            Height = 13
            Caption = 'Search Alias'
          end
          object btnAddIO: TSpeedButton
            Left = 410
            Top = 12
            Width = 70
            Height = 28
            Caption = 'Add'
            OnClick = btnAddIOClick
          end
          object btnDeleteIO: TSpeedButton
            Left = 488
            Top = 12
            Width = 70
            Height = 28
            Caption = 'Delete'
            OnClick = btnDeleteIOClick
          end
          object btnModify: TSpeedButton
            Left = 566
            Top = 12
            Width = 70
            Height = 28
            Caption = 'Modify'
            OnClick = btnModifyClick
          end
          object sbUpdate: TSpeedButton
            Left = 644
            Top = 12
            Width = 70
            Height = 28
            Caption = 'Save'
            OnClick = sbUpdateClick
          end
          object sbIOEditorRefresh: TSpeedButton
            Left = 722
            Top = 12
            Width = 80
            Height = 28
            Caption = 'Refresh'
            OnClick = sbIORefreshClick
          end
          object cbbType: TComboBox
            Left = 10
            Top = 22
            Width = 105
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            ItemIndex = 0
            TabOrder = 0
            Text = 'All'
            OnChange = cbbTypeChange
            Items.Strings = (
              'All'
              'Sensor'
              'Sucker'
              'Switch'
              'Cylinder')
          end
          object cbbLane: TComboBox
            Left = 125
            Top = 22
            Width = 75
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            ItemIndex = 0
            TabOrder = 1
            Text = 'All'
            OnChange = cbbTypeChange
            Items.Strings = (
              'All'
              '0'
              '1'
              '2'
              '3'
              '4'
              '5'
              '6'
              '7'
              '8'
              '9')
          end
          object edtSearchIO: TEdit
            Left = 212
            Top = 22
            Width = 180
            Height = 21
            TabOrder = 2
            OnChange = edtSearchIOChange
          end
        end
        object strngrdIoTable: TStringGrid
          Left = 2
          Top = 50
          Width = 836
          Height = 608
          Align = alClient
          TabOrder = 1
          OnDblClick = strngrdIoTableDblClick
          OnSelectCell = strngrdIoTableSelectCell
        end
      end
    end
    object ts_IOMap: TTabSheet
      Caption = 'IOMap'
      ImageIndex = 9
      object pn_IOMap: TPanel
        Left = 0
        Top = 0
        Width = 840
        Height = 660
        BevelInner = bvLowered
        Color = 12761254
        TabOrder = 0
        object lb_IOMapOB1: TLabel
          Left = 3
          Top = 541
          Width = 835
          Height = 18
          Alignment = taCenter
          AutoSize = False
          Caption = 
            'type =  0 (sensor.db),      1 (cylinder.db),      2 (sucker.db) ' +
            '    3(switch.db)'
          Color = 8421440
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -15
          Font.Name = 'Times New Roman'
          Font.Style = [fsBold]
          ParentColor = False
          ParentFont = False
        end
        object lb_IOMapOBInputMap: TPanel
          Left = 3
          Top = 8
          Width = 415
          Height = 26
          BevelInner = bvLowered
          Caption = 'Input Map'
          Color = 8421440
          Font.Charset = CHINESEBIG5_CHARSET
          Font.Color = clWhite
          Font.Height = -16
          Font.Name = #26032#32048#26126#39636
          Font.Style = []
          ParentFont = False
          TabOrder = 0
        end
        object lb_IOMapOBOutputMap: TPanel
          Left = 420
          Top = 8
          Width = 415
          Height = 26
          BevelInner = bvLowered
          Caption = 'Output Map'
          Color = 8421440
          Font.Charset = CHINESEBIG5_CHARSET
          Font.Color = clWhite
          Font.Height = -16
          Font.Name = #26032#32048#26126#39636
          Font.Style = []
          ParentFont = False
          TabOrder = 1
        end
        object OutputInformationGrid: TStringGrid
          Left = 420
          Top = 38
          Width = 415
          Height = 499
          Color = 14670284
          DefaultColWidth = 50
          DefaultRowHeight = 18
          FixedColor = 16763594
          FixedCols = 0
          RowCount = 1024
          PopupMenu = PopupMenu1
          TabOrder = 2
        end
        object InputInformationGrid: TStringGrid
          Left = 3
          Top = 38
          Width = 415
          Height = 499
          Color = 14670284
          DefaultColWidth = 50
          DefaultRowHeight = 18
          FixedColor = 16763594
          FixedCols = 0
          RowCount = 1024
          PopupMenu = PopupMenu1
          TabOrder = 3
        end
        object MemoIOMap: TMemo
          Left = 3
          Top = 561
          Width = 835
          Height = 90
          Color = 14670284
          TabOrder = 4
        end
      end
    end
    object ts_IOOthers: TTabSheet
      Caption = 'Others'
      ImageIndex = 9
      object pn_PanelRearTitle: TPanel
        Left = 420
        Top = 20
        Width = 415
        Height = 20
        BevelInner = bvLowered
        BevelOuter = bvSpace
        Caption = 'REAR'
        Color = 9534289
        Font.Charset = ANSI_CHARSET
        Font.Color = clWhite
        Font.Height = -16
        Font.Name = #26032#32048#26126#39636
        Font.Style = []
        ParentFont = False
        TabOrder = 0
      end
      object pn_PanelRearOb: TPanel
        Left = 420
        Top = 40
        Width = 415
        Height = 480
        BevelInner = bvLowered
        BevelOuter = bvSpace
        Color = 12761254
        TabOrder = 1
        object mlSnRKPowerOff: TMyLed
          Left = 52
          Top = 30
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKPowerOff'
        end
        object mlSnRKPowerOn: TMyLed
          Left = 147
          Top = 30
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKPowerOn'
        end
        object lb_PanelRearPower: TLabel
          Left = 77
          Top = 8
          Width = 41
          Height = 13
          Caption = 'POWER'
        end
        object mlSnRKStart: TMyLed
          Left = 337
          Top = 120
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKStart'
        end
        object mlSnRKHome: TMyLed
          Left = 242
          Top = 120
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKHome'
        end
        object mlSnRKPause: TMyLed
          Left = 147
          Top = 120
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKPause'
        end
        object mlSnRKReset: TMyLed
          Left = 52
          Top = 120
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKReset'
        end
        object mlSnRKCleanOut: TMyLed
          Left = 337
          Top = 210
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKCleanOut'
        end
        object mlSnRKSkip: TMyLed
          Left = 242
          Top = 210
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKSkip'
        end
        object mlSnRKRetry: TMyLed
          Left = 147
          Top = 210
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKRetry'
        end
        object mlSnRKOneCycle: TMyLed
          Left = 52
          Top = 210
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKOneCycle'
        end
        object mlSnRKAlarmReset: TMyLed
          Left = 247
          Top = 300
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKAlarmReset'
        end
        object mlSnRKTray: TMyLed
          Left = 147
          Top = 300
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKTray'
        end
        object mlSnRKTrayFeed: TMyLed
          Left = 52
          Top = 300
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKTrayFeed'
        end
        object mlSnRearPadActive: TMyLed
          Left = 292
          Top = 36
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRearPadActive'
        end
        object lb_PanelFrontRear: TLabel
          Left = 252
          Top = 76
          Width = 72
          Height = 13
          Caption = 'FRONT/REAR'
        end
        object mlSnRKManualStep: TMyLed
          Left = 242
          Top = 390
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKManualStep'
        end
        object mlSnRKManualTStart: TMyLed
          Left = 337
          Top = 390
          Width = 22
          Height = 22
          LEDStyle = LEDSqLarge
          Alias = 'SnRKManualTStart'
        end
        object bpSwRKPowerOff: TBtnPanel
          Left = 20
          Top = 60
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'OFF'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKPowerOff'
          Style = tsButtons
        end
        object bpSwRKPowerOn: TBtnPanel
          Left = 115
          Top = 60
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'ON'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 1
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKPowerOn'
          Style = tsButtons
        end
        object bpSwRKReset: TBtnPanel
          Left = 20
          Top = 150
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'RESET'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKReset'
          Style = tsButtons
        end
        object bpSwRKPause: TBtnPanel
          Left = 115
          Top = 150
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'PAUSE'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 3
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKPause'
          Style = tsButtons
        end
        object bpSwRKHome: TBtnPanel
          Left = 210
          Top = 150
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'HOME'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 4
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKHome'
          Style = tsButtons
        end
        object bpSwRKStart: TBtnPanel
          Left = 305
          Top = 150
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'START'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 5
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKStart'
          Style = tsButtons
        end
        object bpSwRKOneCycle: TBtnPanel
          Left = 20
          Top = 240
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'ONE CYCLE'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 6
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKOneCycle'
          Style = tsButtons
        end
        object bpSwRKRetry: TBtnPanel
          Left = 115
          Top = 240
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'RETRY'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 7
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKRetry'
          Style = tsButtons
        end
        object bpSwRKSkip: TBtnPanel
          Left = 210
          Top = 240
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'SKIP'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 8
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKSkip'
          Style = tsButtons
        end
        object bpSwRKCleanOut: TBtnPanel
          Left = 305
          Top = 240
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'CLEAN OUT'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 9
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKCleanOut'
          Style = tsButtons
        end
        object bpSwRKTrayFeed: TBtnPanel
          Left = 20
          Top = 330
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'TRAY FEED'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 10
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKTrayFeed'
          Style = tsButtons
        end
        object bpSwRKTrayEnd: TBtnPanel
          Left = 115
          Top = 330
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'TRAY END'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 11
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKTrayEnd'
          Style = tsButtons
        end
        object bpSwRKAlarmReset: TBtnPanel
          Left = 210
          Top = 330
          Width = 95
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'ALARM RESET'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 12
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKAlarmReset'
          Style = tsButtons
        end
        object bpSwRKManualStep: TBtnPanel
          Left = 210
          Top = 420
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'STEP'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 13
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKManualStep'
          Style = tsButtons
        end
        object bpSwRKManualTStart: TBtnPanel
          Left = 305
          Top = 420
          Width = 85
          Height = 50
          BevelInner = bvRaised
          BevelWidth = 2
          Caption = 'T.START'
          Color = 8404992
          Font.Charset = ANSI_CHARSET
          Font.Color = clWhite
          Font.Height = -12
          Font.Name = 'Times New Roman'
          Font.Style = []
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 14
          OnClick = BtnPanelClick
          TrueColor = 16744448
          FalseColor = 8404992
          FalseFontColor = clWhite
          Alias = 'SwRKManualTStart'
          Style = tsButtons
        end
      end
    end
    object tsMN200: TTabSheet
      Caption = 'MN200'
      object lblMN200Summary: TLabel
        Left = 8
        Top = 8
        Width = 820
        Height = 16
        AutoSize = False
        Caption = 'MN200: (refresh pending)'
        WordWrap = True
      end
      object grdMN200: TStringGrid
        Left = 8
        Top = 32
        Width = 820
        Height = 360
        DefaultRowHeight = 22
        FixedCols = 0
        RowCount = 2
        TabOrder = 0
      end
    end
    object ts_IOSelfTest: TTabSheet
      Caption = 'SelfTest'
      object rgSelfTestMode: TRadioGroup
        Left = 8
        Top = 8
        Width = 320
        Height = 44
        Caption = 'Mode'
        Columns = 2
        ItemIndex = 0
        Items.Strings = (
          'Cylinder'
          'Sensor')
        TabOrder = 0
        OnClick = rgSelfTestModeClick
      end
      object pnSelfTestPrecond: TPanel
        Left = 336
        Top = 8
        Width = 496
        Height = 44
        BevelInner = bvLowered
        Caption = 'Precondition :  Machine Stopped   /   EMG Normal   /   Air OK'
        TabOrder = 1
      end
      object pnSelfTestDetect: TPanel
        Left = 8
        Top = 58
        Width = 270
        Height = 560
        BevelInner = bvLowered
        TabOrder = 2
        object lblSelfTestDetectHdr: TLabel
          Left = 8
          Top = 8
          Width = 254
          Height = 18
          AutoSize = False
          Caption = 'Detection - select items'
        end
        object lblSelfTestProgress: TLabel
          Left = 8
          Top = 536
          Width = 254
          Height = 16
          AutoSize = False
          Caption = 'Progress : -'
        end
        object clbSelfTestItems: TCheckListBox
          Left = 8
          Top = 30
          Width = 254
          Height = 400
          ItemHeight = 16
          TabOrder = 0
        end
        object btnSelfTestSelectAll: TButton
          Left = 8
          Top = 436
          Width = 123
          Height = 25
          Caption = 'Select All'
          TabOrder = 1
          OnClick = btnSelfTestSelectAllClick
        end
        object btnSelfTestSelectNone: TButton
          Left = 139
          Top = 436
          Width = 123
          Height = 25
          Caption = 'Select None'
          TabOrder = 2
          OnClick = btnSelfTestSelectNoneClick
        end
        object btnSelfTestStart: TButton
          Left = 8
          Top = 467
          Width = 254
          Height = 30
          Caption = 'Start Test'
          TabOrder = 3
          OnClick = btnSelfTestStartClick
        end
        object btnSelfTestStop: TButton
          Left = 8
          Top = 501
          Width = 254
          Height = 28
          Caption = 'Stop'
          TabOrder = 4
          OnClick = btnSelfTestStopClick
        end
      end
      object pnSelfTestResult: TPanel
        Left = 286
        Top = 58
        Width = 546
        Height = 560
        BevelInner = bvLowered
        TabOrder = 3
        object lblSelfTestResultHdr: TLabel
          Left = 8
          Top = 8
          Width = 530
          Height = 18
          AutoSize = False
          Caption = 'Result'
        end
        object lblSelfTestSummary: TLabel
          Left = 8
          Top = 534
          Width = 530
          Height = 18
          AutoSize = False
          Caption = 'PASS 0    FAIL 0    SKIP 0'
        end
        object grdSelfTest: TStringGrid
          Left = 8
          Top = 30
          Width = 530
          Height = 498
          ColCount = 6
          DefaultRowHeight = 20
          FixedCols = 0
          RowCount = 2
          TabOrder = 0
        end
      end
      object memSelfTestLog: TMemo
        Left = 8
        Top = 624
        Width = 824
        Height = 68
        ScrollBars = ssVertical
        TabOrder = 4
      end
    end
  end
  object plIOForm: TPanel
    Left = 0
    Top = 0
    Width = 850
    Height = 41
    BevelInner = bvRaised
    BevelOuter = bvLowered
    Caption = 'IO'
    Color = 10263630
    Font.Charset = ANSI_CHARSET
    Font.Color = clYellow
    Font.Height = -24
    Font.Name = 'Palatino Linotype'
    Font.Style = []
    ParentFont = False
    TabOrder = 1
  end
  object ioTable: TTable
    DatabaseName = 'HT160S'
    Left = 668
    Top = 1
  end
  object OpenDialog1: TOpenDialog
    DefaultExt = 'txt'
    Filter = '*.txt|*.txt'
    Left = 736
  end
  object SaveDialog1: TSaveDialog
    DefaultExt = 'txt'
    Filter = '*.txt|*.txt'
    Left = 776
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 200
    OnTimer = Timer1Timer
    Left = 700
    Top = 65534
  end
  object DataSource1: TDataSource
    DataSet = ioTable
    Left = 636
    Top = 1
  end
  object PopupMenu1: TPopupMenu
    Left = 592
    Top = 4
    object SaveInputMap1: TMenuItem
      Caption = 'Save Input Map'
      OnClick = SaveInputMap1Click
    end
    object SaveOutputMap1: TMenuItem
      Caption = 'Save Output Map'
      OnClick = SaveOutputMap1Click
    end
  end
end
