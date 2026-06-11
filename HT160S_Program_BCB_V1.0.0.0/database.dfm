object DataModule1: TDataModule1
  OldCreateOrder = True
  Left = 192
  Top = 114
  Height = 260
  Width = 320
  object UserActionList: TActionList
    Left = 32
    Top = 16
    object InitialMotorName: TAction
      Caption = 'InitialMotorName'
      OnExecute = InitialMotorNameExecute
    end
    object SpecificSetupForMotorParameter: TAction
      Caption = 'SpecificSetupForMotorParameter'
    end
    object InitialCylinderName: TAction
      Caption = 'InitialCylinderName'
      OnExecute = InitialCylinderNameExecute
    end
    object SpecificSetupForCylinderParameter: TAction
      Caption = 'SpecificSetupForCylinderParameter'
    end
    object InitialSensorName: TAction
      Caption = 'InitialSensorName'
      OnExecute = InitialSensorNameExecute
    end
    object SpecificSetupForSensorParameter: TAction
      Caption = 'SpecificSetupForSensorParameter'
    end
    object InitialSwitchName: TAction
      Caption = 'InitialSwitchName'
      OnExecute = InitialSwitchNameExecute
    end
    object SpecificSetupForSwitchParameter: TAction
      Caption = 'SpecificSetupForSwitchParameter'
    end
    object InitialSuckerName: TAction
      Caption = 'InitialSuckerName'
      OnExecute = InitialSuckerNameExecute
    end
    object SpecificSetupForSuckerParameter: TAction
      Caption = 'SpecificSetupForSuckerParameter'
    end
    object Initial_IO_Setup: TAction
      Caption = 'Initial_IO_Setup'
    end
  end
  object UserMotion: TActionList
    Left = 116
    Top = 16
    object actEmpty: TAction
      Caption = 'Empty'
      OnExecute = actEmptyExecute
    end
    object actLoader1: TAction
      Caption = 'Loader1'
      OnExecute = actLoader1Execute
    end
    object actLoader2: TAction
      Caption = 'Loader2'
      OnExecute = actLoader2Execute
    end
    object actAuto1to6: TAction
      Caption = 'Auto1'
      OnExecute = actAuto1to6Execute
    end
    object actTrayArm: TAction
      Caption = 'TrayArm'
      OnExecute = actTrayArmExecute
    end
    object actSortArm: TAction
      Caption = 'SortArm'
      OnExecute = actSortArmExecute
    end
    object actColor: TAction
      Caption = 'Color'
      OnExecute = actColorExecute
    end
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 100
    OnTimer = Timer1Timer
    Left = 36
    Top = 164
  end
end
