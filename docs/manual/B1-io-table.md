# 附錄 B　全機 I/O 對照表

資料來源：`system/IO_Table.csv`（機台設定檔，2026-07-16 重生）。位址格式 Lane/IP/Port/Bit（IP=W 為寫出型點位）。IO_Table.csv 無中文標籤欄，畫面顯示名稱由程式以「前綴＋Alias」慣例產生。最終位址請以機台 State Record 內 MachineConfig\system 副本核對（repo 工作副本與機台副本可能 drift）。

## Cylinder（39 點）
| Alias | Lane | IP | Port | Bit | InType | 啟用 | OnDelay | OffDelay | Note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| C_TrayArmZ_Up | 0 | 0 | 2 | 5 | 1 | 是 | 5 | 5 |  |
| C_TrayArm_FrontClamp | 0 | 0 | 2 | 6 | 1 | 是 | 500 | 500 |  |
| C_TrayArm_RearClamp | 0 | 0 | 2 | 7 | 1 | 是 | 500 | 500 |  |
| C_TrayArmZ_Down | 0 | 0 | 2 | 4 | 1 | 是 | 5 | 5 |  |
| C_Empty_FrontRiseTray_1 | 0 | 8 | 0 | 0 | 1 | 是 | 5 | 5 |  |
| C_Empty_FrontRiseTray_2 | 0 | 8 | 3 | 6 | 1 | 是 | 3000 | 3000 |  |
| C_Empty_PushTray | 0 | 8 | 0 | 1 | 1 | 是 | 5 | 5 |  |
| C_Empty_LeanOnTray | 0 | 8 | 0 | 2 | 1 | 是 | 5 | 5 |  |
| C_Empty_FrontSeparateTray_1 | 0 | 8 | 0 | 3 | 1 | 是 | 5 | 5 |  |
| C_Loader_FrontRiseTray_1 | 0 | 8 | 1 | 4 | 1 | 是 | 5000 | 5000 |  |
| C_Loader_FrontRiseTray_2 | 0 | 8 | 3 | 7 | 1 | 是 | 5000 | 5000 |  |
| C_Loader1_PushTray | 0 | 8 | 1 | 5 | 1 | 是 | 5 | 5 |  |
| C_Loader2_PushTray | 0 | 8 | 1 | 6 | 1 | 是 | 5 | 5 |  |
| C_Loader1_LeanOnTray | 0 | 8 | 1 | 7 | 1 | 是 | 5 | 5 |  |
| C_Loader2_LeanOnTray | 0 | 8 | 2 | 0 | 1 | 是 | 5 | 5 |  |
| C_Loader_FrontSeparateTray_1 | 0 | 8 | 2 | 1 | 1 | 是 | 5000 | 5000 |  |
| C_Auto1_FrontRiseTray | 0 | 8 | 2 | 6 | 1 | 是 | 5 | 5 |  |
| C_Auto1_PushTray | 0 | 8 | 2 | 7 | 1 | 是 | 5 | 5 |  |
| C_Auto1_LeanOnTray | 0 | 8 | 3 | 0 | 1 | 是 | 5 | 5 |  |
| C_Auto2_FrontRiseTray | 0 | 8 | 2 | 5 | 1 | 是 | 5 | 5 |  |
| C_Auto2_PushTray | 0 | 8 | 3 | 1 | 1 | 是 | 5 | 5 |  |
| C_Auto2_LeanOnTray | 0 | 8 | 3 | 2 | 1 | 是 | 5 | 5 |  |
| C_Auto3_FrontRiseTray | 0 | 9 | 1 | 0 | 1 | 是 | 5 | 5 |  |
| C_Auto3_PushTray | 0 | 9 | 1 | 1 | 1 | 是 | 5 | 5 |  |
| C_Auto3_LeanOnTray | 0 | 9 | 1 | 2 | 1 | 是 | 5 | 5 |  |
| C_Auto4_FrontRiseTray | 0 | 9 | 2 | 0 | 1 | 是 | 5 | 5 |  |
| C_Auto4_PushTray | 0 | 9 | 2 | 1 | 1 | 是 | 5 | 5 |  |
| C_Auto4_LeanOnTray | 0 | 9 | 2 | 2 | 1 | 是 | 5 | 5 |  |
| C_Auto5_FrontRiseTray | 0 | 9 | 3 | 0 | 1 | 是 | 5 | 5 |  |
| C_Auto5_PushTray | 0 | 9 | 3 | 1 | 1 | 是 | 5 | 5 |  |
| C_Auto5_LeanOnTray | 0 | 9 | 3 | 2 | 1 | 是 | 5 | 5 |  |
| C_Auto6_FrontRiseTray | 0 | W | 0 | 2 | 1 | 是 | 5 | 5 |  |
| C_Auto6_PushTray | 0 | W | 0 | 3 | 1 | 是 | 5 | 5 |  |
| C_Auto6_LeanOnTray | 0 | W | 0 | 4 | 1 | 是 | 5 | 5 |  |
| C_Color_LeanOnTray | 0 | W | 1 | 2 | 1 | 是 | 5 | 5 |  |
| C_Color_PushTray | 0 | W | 1 | 1 | 1 | 是 | 5 | 5 |  |
| C_Color_FrontRiseTray_1 | 0 | W | 0 | 7 | 1 | 是 | 5 | 5 |  |
| C_Color_FrontRiseTray_2 | 0 | W | 1 | 0 | 1 | 是 | 5 | 5 |  |
| C_Color_FrontSeparateTray_1 | 0 | W | 1 | 3 | 1 | 是 | 5 | 5 |  |

## Switch（41 點）
| Alias | Lane | IP | Port | Bit | InType | 啟用 | OnDelay | OffDelay | Note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| SwTowerRed | 1 | W | 2 | 4 | 1 | 是 |  |  |  |
| SwTowerYellow | 1 | W | 2 | 5 | 1 | 是 |  |  |  |
| SwTowerGreen | 1 | W | 2 | 6 | 1 | 是 |  |  |  |
| SwMusic1 | 1 | W | 2 | 0 | 1 | 是 |  |  |  |
| SwMusic2 | 1 | W | 2 | 1 | 1 | 是 |  |  |  |
| SwMusic3 | 1 | W | 2 | 2 | 1 | 是 |  |  |  |
| SwMusic4 | 1 | W | 2 | 3 | 1 | 是 |  |  |  |
| SwMotorRelay | 0 | 0 | 2 | 0 | 1 | 是 |  |  |  |
| SwLight | 0 | 5 | 0 | 1 | 1 | 是 |  |  |  |
| SwFKPowerOff |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKPowerOn |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFrontActiveLed |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKReset |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKPause |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKHome |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKStart |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKOneCycle |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKRetry |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKSkip |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKCleanOut |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKTrayFeed |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKTrayEnd |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwFKAlarmReset |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKPowerOff |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKPowerOn |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKReset |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKPause |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKHome |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKStart |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKOneCycle |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKRetry |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKSkip |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKCleanOut |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKTrayFeed |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKTrayEnd |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKAlarmReset |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKManualStep |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKManualTStart |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRKSafeLock |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwRearActiveLed |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SwServerON |  |  |  |  | 1 | 否 |  |  |  |

## Sucker_On（4 點）
| Alias | Lane | IP | Port | Bit | InType | 啟用 | OnDelay | OffDelay | Note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Suck1_On | 0 | W | 2 | 0 | 1 | 是 | 30 | 10 |  |
| Suck2_On | 0 | W | 2 | 2 | 1 | 是 | 30 | 10 |  |
| Suck3_On | 0 | W | 2 | 4 | 1 | 是 | 30 | 10 |  |
| Suck4_On | 0 | W | 2 | 6 | 1 | 是 | 30 | 10 |  |

## Sensor（101 點）
| Alias | Lane | IP | Port | Bit | InType | 啟用 | OnDelay | OffDelay | Note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| SnIonFan_Balance | 1 | W | 1 | 3 | 1 | 是 |  |  |  |
| SnIonFan_Power | 1 | W | 1 | 4 | 1 | 是 |  |  |  |
| SnEMG_1 | 1 | W | 0 | 0 | 1 | 是 |  |  |  |
| SnEMG_2 | 1 | W | 0 | 1 | 1 | 是 |  |  |  |
| SnEMG_3 | 1 | W | 0 | 2 | 1 | 是 |  |  |  |
| SnEMG_4 | 1 | W | 0 | 3 | 1 | 是 |  |  |  |
| SnSafeDoorFront | 1 | W | 0 | 5 | 1 | 是 |  |  |  |
| SnSafeDoorRight | 1 | W | 1 | 1 | 1 | 是 |  |  |  |
| SnSafeDoorLeft | 1 | W | 0 | 6 | 1 | 是 |  |  |  |
| SnSafeSlideDoorRight | 1 | W | 1 | 0 | 1 | 是 |  |  |  |
| SnSafeSlideDoorLeft | 1 | W | 0 | 7 | 1 | 是 |  |  |  |
| SnSafeAuto6 | 1 | W | 1 | 6 | 1 | 是 |  |  |  |
| SnSafeLock | 1 | W | 0 | 4 | 0 | 是 |  |  |  |
| SnMotorPower | 0 | 0 | 0 | 0 | 1 | 是 |  |  |  |
| SnAirIsEnough | 0 | 0 | 0 | 1 | 1 | 是 |  |  |  |
| SnEMG | 0 | 0 | 0 | 2 | 1 | 是 |  |  |  |
| SnEmpty_InputHasTray | 0 | 1 | 0 | 4 | 0 | 是 |  |  |  |
| SnEmpty_InputFullTray | 0 | 1 | 0 | 5 | 0 | 是 |  |  |  |
| SnEmpty_TrayPos1 | 0 | 1 | 0 | 6 | 0 | 是 |  |  |  |
| SnEmpty_TrayPos2 | 0 | 1 | 0 | 7 | 1 | 是 |  |  |  |
| SnEmpty_OutputBottomHasTray | 0 | 1 | 1 | 3 | 0 | 是 |  |  |  |
| SnLoader_InputHasTray | 0 | 1 | 2 | 4 | 0 | 是 |  |  |  |
| SnLoader_InputFullTray | 0 | 1 | 2 | 5 | 0 | 是 |  |  |  |
| SnLoader_TrayPos1 | 0 | 1 | 2 | 6 | 0 | 是 |  |  |  |
| SnLoader_TrayPos2 | 0 | 1 | 2 | 7 | 1 | 是 |  |  |  |
| SnLoader_OutputBottomHasTray | 0 | 1 | 3 | 5 | 0 | 是 |  |  |  |
| SnAuto1_InputHasTray | 0 | 3 | 0 | 4 | 0 | 是 |  |  |  |
| SnAuto1_InputFullTray | 0 | 3 | 0 | 5 | 0 | 是 |  |  |  |
| SnAuto1_InputEnd | 0 | 2 | 1 | 0 | 0 | 是 |  |  |  |
| SnAuto1_TrayPos1 | 0 | 3 | 0 | 6 | 0 | 是 |  |  |  |
| SnAuto1_TrayPos2 | 0 | 3 | 0 | 7 | 1 | 是 |  |  |  |
| SnAuto1_OutputBottomHasTray | 0 | 3 | 1 | 3 | 0 | 是 |  |  |  |
| SnAuto2_InputHasTray | 0 | 3 | 2 | 2 | 0 | 是 |  |  |  |
| SnAuto2_InputFullTray | 0 | 3 | 2 | 3 | 0 | 是 |  |  |  |
| SnAuto2_InputEnd | 0 | 2 | 1 | 1 | 0 | 是 |  |  |  |
| SnAuto2_TrayPos1 | 0 | 3 | 2 | 4 | 0 | 是 |  |  |  |
| SnAuto2_TrayPos2 | 0 | 3 | 2 | 5 | 1 | 是 |  |  |  |
| SnAuto2_OutputBottomHasTray | 0 | 3 | 3 | 1 | 0 | 是 |  |  |  |
| SnAuto3_InputHasTray | 0 | 4 | 0 | 0 | 0 | 是 |  |  |  |
| SnAuto3_InputFullTray | 0 | 4 | 0 | 1 | 0 | 是 |  |  |  |
| SnAuto3_InputEnd | 0 | 2 | 1 | 2 | 0 | 是 |  |  |  |
| SnAuto3_TrayPos1 | 0 | 4 | 0 | 2 | 0 | 是 |  |  |  |
| SnAuto3_TrayPos2 | 0 | 4 | 0 | 3 | 1 | 是 |  |  |  |
| SnAuto3_OutputBottomHasTray | 0 | 4 | 0 | 7 | 0 | 是 |  |  |  |
| SnAuto4_InputHasTray | 0 | 5 | 0 | 4 | 0 | 是 |  |  |  |
| SnAuto4_InputFullTray | 0 | 5 | 0 | 5 | 0 | 是 |  |  |  |
| SnAuto4_InputEnd | 0 | 2 | 1 | 3 | 0 | 是 |  |  |  |
| SnAuto4_TrayPos1 | 0 | 5 | 0 | 6 | 0 | 是 |  |  |  |
| SnAuto4_TrayPos2 | 0 | 5 | 0 | 7 | 1 | 是 |  |  |  |
| SnAuto4_OutputBottomHasTray | 0 | 5 | 1 | 3 | 0 | 是 |  |  |  |
| SnAuto5_InputHasTray | 0 | 5 | 2 | 2 | 0 | 是 |  |  |  |
| SnAuto5_InputFullTray | 0 | 5 | 2 | 3 | 0 | 是 |  |  |  |
| SnAuto5_InputEnd | 0 | 2 | 1 | 4 | 0 | 是 |  |  |  |
| SnAuto5_TrayPos1 | 0 | 5 | 2 | 4 | 0 | 是 |  |  |  |
| SnAuto5_TrayPos2 | 0 | 5 | 2 | 5 | 1 | 是 |  |  |  |
| SnAuto5_OutputBottomHasTray | 0 | 5 | 3 | 1 | 0 | 是 |  |  |  |
| SnAuto6_InputHasTray | 0 | 6 | 0 | 0 | 0 | 是 |  |  |  |
| SnAuto6_InputFullTray | 0 | 6 | 0 | 1 | 0 | 是 |  |  |  |
| SnAuto6_InputEnd | 0 | 2 | 1 | 5 | 0 | 是 |  |  |  |
| SnAuto6_TrayPos1 | 0 | 6 | 0 | 2 | 0 | 是 |  |  |  |
| SnAuto6_TrayPos2 | 0 | 6 | 0 | 3 | 1 | 是 |  |  |  |
| SnAuto6_OutputBottomHasTray | 0 | 6 | 0 | 7 | 0 | 是 |  |  |  |
| SnEmpty_InputEnd | 0 | 2 | 0 | 2 | 0 | 是 |  |  |  |
| SnLoader_Inputend | 0 | 2 | 0 | 3 | 0 | 是 |  |  |  |
| SnColor_InputHasTray | 0 | 6 | 1 | 6 | 0 | 是 |  |  |  |
| SnColor_InputFullTray | 0 | 6 | 1 | 7 | 0 | 是 |  |  |  |
| SnColor_TrayPos1 | 0 | 6 | 2 | 0 | 0 | 是 |  |  |  |
| SnColor_OutputBottomHasTray | 0 | 6 | 2 | 3 | 0 | 是 |  |  |  |
| SnColor_InputEnd | 0 | 6 | 2 | 4 | 0 | 是 |  |  |  |
| SnFKPowerOff |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKPowerOn |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFrontPadActive |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKReset |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKPause |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKHome |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKStart |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKOneCycle |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKRetry |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKSkip |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKCleanOut |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKTrayFeed |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKTrayEnd |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnFKAlarmReset |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKPowerOff |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKPowerOn |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRearPadActive |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKReset |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKPause |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKHome |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKStart |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKOneCycle |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKRetry |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKSkip |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKCleanOut |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKTrayFeed |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKTray |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKTrayEnd |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKAlarmReset |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKManualStep |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKManualTStart |  |  |  |  | 0 | 否 |  |  | COMM_PAD |
| SnRKSafeLock |  |  |  |  | 0 | 否 |  |  | COMM_PAD |

## Cylinder_On（39 點）
| Alias | Lane | IP | Port | Bit | InType | 啟用 | OnDelay | OffDelay | Note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| C_TrayArmZ_Up_On | 0 | 0 | 1 | 5 | 1 | 是 | 5 | 5 |  |
| C_TrayArm_FrontClamp_On | 0 | 0 | 1 | 6 | 1 | 是 | 5 | 5 |  |
| C_TrayArm_RearClamp_On | 0 | 0 | 1 | 7 | 1 | 是 | 5 | 5 |  |
| C_Empty_FrontRiseTray_1_On | 0 | 2 | 0 | 0 | 1 | 是 | 5 | 5 |  |
| C_Empty_FrontRiseTray_2_On | 0 | 1 | 0 | 0 | 1 | 是 | 5 | 5 |  |
| C_Empty_PushTray_On | 0 | 1 | 0 | 3 | 1 | 是 | 5 | 5 |  |
| C_Empty_LeanOnTray_On | 0 | 1 | 1 | 0 | 1 | 是 | 5 | 5 |  |
| C_Loader_FrontRiseTray_1_On | 0 | 2 | 0 | 1 | 1 | 是 | 5 | 5 |  |
| C_Loader_FrontRiseTray_2_On | 0 | 1 | 1 | 6 | 1 | 是 | 5 | 5 |  |
| C_Loader1_PushTray_On | 0 | 1 | 2 | 1 | 1 | 是 | 5 | 5 |  |
| C_Loader2_PushTray_On | 0 | 1 | 2 | 3 | 1 | 是 | 5 | 5 |  |
| C_Loader1_LeanOnTray_On | 0 | 1 | 3 | 0 | 1 | 是 | 5 | 5 |  |
| C_Loader2_LeanOnTray_On | 0 | 1 | 3 | 2 | 1 | 是 | 5 | 5 |  |
| C_Auto1_FrontRiseTray_On | 0 | 3 | 0 | 0 | 1 | 是 | 5 | 5 |  |
| C_Auto1_PushTray_On | 0 | 3 | 0 | 3 | 1 | 是 | 5 | 5 |  |
| C_Auto1_LeanOnTray_On | 0 | 3 | 1 | 0 | 1 | 是 | 5 | 5 |  |
| C_Auto2_FrontRiseTray_On | 0 | 3 | 1 | 6 | 1 | 是 | 5 | 5 |  |
| C_Auto2_PushTray_On | 0 | 3 | 2 | 1 | 1 | 是 | 5 | 5 |  |
| C_Auto2_LeanOnTray_On | 0 | 3 | 2 | 6 | 1 | 是 | 5 | 5 |  |
| C_Auto3_FrontRiseTray_On | 0 | 3 | 3 | 4 | 1 | 是 | 5 | 5 |  |
| C_Auto3_PushTray_On | 0 | 3 | 3 | 7 | 1 | 是 | 5 | 5 |  |
| C_Auto3_LeanOnTray_On | 0 | 4 | 0 | 4 | 1 | 是 | 5 | 5 |  |
| C_Auto4_FrontRiseTray_On | 0 | 5 | 0 | 0 | 1 | 是 | 5 | 5 |  |
| C_Auto4_PushTray_On | 0 | 5 | 0 | 3 | 1 | 是 | 5 | 5 |  |
| C_Auto4_LeanOnTray_On | 0 | 5 | 1 | 0 | 1 | 是 | 5 | 5 |  |
| C_Auto5_FrontRiseTray_On | 0 | 5 | 1 | 6 | 1 | 是 | 5 | 5 |  |
| C_Auto5_PushTray_On | 0 | 5 | 2 | 1 | 1 | 是 | 5 | 5 |  |
| C_Auto5_LeanOnTray_On | 0 | 5 | 2 | 6 | 1 | 是 | 5 | 5 |  |
| C_Auto6_FrontRiseTray_On | 0 | 5 | 3 | 4 | 1 | 是 | 5 | 5 |  |
| C_Auto6_PushTray_On | 0 | 5 | 3 | 7 | 1 | 是 | 5 | 5 |  |
| C_Auto6_LeanOnTray_On | 0 | 6 | 0 | 4 | 1 | 是 | 5 | 5 |  |
| C_TrayArmZ_Down_On | 0 | 0 | 1 | 4 | 1 | 是 | 5 | 5 |  |
| C_Color_LeanOnTray_On | 0 | 6 | 2 | 1 | 1 | 是 | 5 | 5 |  |
| C_Color_PushTray_On | 0 | 6 | 1 | 5 | 1 | 是 | 5 | 5 |  |
| C_Color_FrontRiseTray_1_On | 0 | 6 | 2 | 5 | 1 | 是 | 5 | 5 |  |
| C_Color_FrontRiseTray_2_On | 0 | 6 | 1 | 2 | 1 | 是 | 5 | 5 |  |
| C_Empty_FrontSeparateTray_1_On |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_Loader_FrontSeparateTray_1_On |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_Color_FrontSeparateTray_1_On |  |  |  |  | 0 | 否 | 5 | 5 |  |

## Cylinder_Off（39 點）
| Alias | Lane | IP | Port | Bit | InType | 啟用 | OnDelay | OffDelay | Note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| C_Empty_FrontRiseTray_1_Off | 0 | 1 | 0 | 1 | 1 | 是 | 5 | 5 |  |
| C_Empty_PushTray_Off | 0 | 1 | 0 | 2 | 1 | 是 | 5 | 5 |  |
| C_Empty_LeanOnTray_Off | 0 | 1 | 1 | 1 | 1 | 是 | 5 | 5 |  |
| C_Loader_FrontRiseTray_1_Off | 0 | 1 | 1 | 7 | 1 | 是 | 5 | 5 |  |
| C_Loader1_PushTray_Off | 0 | 1 | 2 | 0 | 1 | 是 | 5 | 5 |  |
| C_Loader2_PushTray_Off | 0 | 1 | 2 | 2 | 1 | 是 | 5 | 5 |  |
| C_Loader1_LeanOnTray_Off | 0 | 1 | 3 | 1 | 1 | 是 | 5 | 5 |  |
| C_Loader2_LeanOnTray_Off | 0 | 1 | 3 | 3 | 1 | 是 | 5 | 5 |  |
| C_Auto1_FrontRiseTray_Off | 0 | 3 | 0 | 1 | 1 | 是 | 5 | 5 |  |
| C_Auto1_PushTray_Off | 0 | 3 | 0 | 2 | 1 | 是 | 5 | 5 |  |
| C_Auto1_LeanOnTray_Off | 0 | 3 | 1 | 1 | 1 | 是 | 5 | 5 |  |
| C_Auto2_FrontRiseTray_Off | 0 | 3 | 1 | 7 | 1 | 是 | 5 | 5 |  |
| C_Auto2_PushTray_Off | 0 | 3 | 2 | 0 | 1 | 是 | 5 | 5 |  |
| C_Auto2_LeanOnTray_Off | 0 | 3 | 2 | 7 | 1 | 是 | 5 | 5 |  |
| C_Auto3_FrontRiseTray_Off | 0 | 3 | 3 | 5 | 1 | 是 | 5 | 5 |  |
| C_Auto3_PushTray_Off | 0 | 3 | 3 | 6 | 1 | 是 | 5 | 5 |  |
| C_Auto3_LeanOnTray_Off | 0 | 4 | 0 | 5 | 1 | 是 | 5 | 5 |  |
| C_Auto4_FrontRiseTray_Off | 0 | 5 | 0 | 1 | 1 | 是 | 5 | 5 |  |
| C_Auto4_PushTray_Off | 0 | 5 | 0 | 2 | 1 | 是 | 5 | 5 |  |
| C_Auto4_LeanOnTray_Off | 0 | 5 | 1 | 1 | 1 | 是 | 5 | 5 |  |
| C_Auto5_FrontRiseTray_Off | 0 | 5 | 1 | 7 | 1 | 是 | 5 | 5 |  |
| C_Auto5_PushTray_Off | 0 | 5 | 2 | 0 | 1 | 是 | 5 | 5 |  |
| C_Auto5_LeanOnTray_Off | 0 | 5 | 2 | 7 | 1 | 是 | 5 | 5 |  |
| C_Auto6_FrontRiseTray_Off | 0 | 5 | 3 | 5 | 1 | 是 | 5 | 5 |  |
| C_Auto6_PushTray_Off | 0 | 5 | 3 | 6 | 1 | 是 | 5 | 5 |  |
| C_Auto6_LeanOnTray_Off | 0 | 6 | 0 | 5 | 1 | 是 | 5 | 5 |  |
| C_Color_LeanOnTray_Off | 0 | 6 | 2 | 2 | 1 | 是 | 5 | 5 |  |
| C_Color_PushTray_Off | 0 | 6 | 1 | 4 | 1 | 是 | 5 | 5 |  |
| C_Color_FrontRiseTray_1_Off | 0 | 6 | 1 | 3 | 1 | 是 | 5 | 5 |  |
| C_TrayArmZ_Up_Off |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_TrayArm_FrontClamp_Off |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_TrayArm_RearClamp_Off |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_Empty_FrontRiseTray_2_Off |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_Empty_FrontSeparateTray_1_Off |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_Loader_FrontRiseTray_2_Off |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_Loader_FrontSeparateTray_1_Off |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_TrayArmZ_Down_Off |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_Color_FrontRiseTray_2_Off |  |  |  |  | 0 | 否 | 5 | 5 |  |
| C_Color_FrontSeparateTray_1_Off |  |  |  |  | 0 | 否 | 5 | 5 |  |

## Sucker（4 點）
| Alias | Lane | IP | Port | Bit | InType | 啟用 | OnDelay | OffDelay | Note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Suck1 | 0 | 0 | 1 | 0 | 1 | 是 | 30 | 10 |  |
| Suck2 | 0 | 0 | 1 | 1 | 1 | 是 | 30 | 10 |  |
| Suck3 | 0 | 0 | 1 | 2 | 1 | 是 | 30 | 10 |  |
| Suck4 | 0 | 0 | 1 | 3 | 0 | 是 | 30 | 10 |  |

## Sucker_Off（4 點）
| Alias | Lane | IP | Port | Bit | InType | 啟用 | OnDelay | OffDelay | Note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Suck1_Off | 0 | W | 2 | 1 | 1 | 是 | 30 | 10 |  |
| Suck2_Off | 0 | W | 2 | 3 | 1 | 是 | 30 | 10 |  |
| Suck3_Off | 0 | W | 2 | 5 | 1 | 是 | 30 | 10 |  |
| Suck4_Off | 0 | W | 2 | 7 | 1 | 是 | 30 | 10 |  |

