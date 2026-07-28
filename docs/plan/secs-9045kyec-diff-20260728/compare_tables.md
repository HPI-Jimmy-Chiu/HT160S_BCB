## A. Message layer (SxFy primaries)

- BOTH (both really reply): 1F1 1F3 1F11 1F13 1F15 1F17 2F13 2F15 2F17 2F25 2F31 2F33 2F35 2F37 2F41 5F3 5F5 5F7
- 9045 only (HT160 no dispatch -> log-only S9F3, host T3 timeout): 1F23 2F23 2F29 2F43 6F15 6F17 6F19 6F23 14F3 100F3 101F1 101F3 101F5 101F7 101F11 103F11 110F2 110F6 110F8 120F2 125F1 125F3
- 9045 has, HT160 dispatches but STUB (no reply -> T3 timeout): 7F1 7F3 7F5 7F17 7F19 10F3 10F5
- HT160 only: 14F1

KYEC host actually sent in this log: 1F1 1F3 1F13 1F17 2F15 2F33 2F35 2F37 2F41 5F1 5F3 6F11 6F15 6F19 10F3 10F5 125F1

**HOT gap (host really sent it AND HT160 would not answer): 6F15 6F19 10F3 10F5 125F1**


## B. Remote command layer (S2F41 RCMD)

- BOTH (8): LOTSTART ONLINE_LOCAL ONLINE_REMOTE PAUSE SET_LOT_INFO START START_AGV STOP
- HT160 only (3): CLEARCOUNT HOME ONLINE
- 9045 only (39): AUTHORITY_CHECK AUTOSITEMAP AUTO_CLEAN AUTO_RETEST CLEAN_AUTO_SORT_COUNT CLEAN_OUT CLEAR_LOT_INFO CLOSE_ONECYCLE CONTINUE_RETEST_ART CONTINUE_START_ART CONTINUE_START_MRT DEVTEMPOFFSETADJUST DOWNLOAD_RECIPE_BY_FTP EESUG_OFFSET ENERGY_SAVING HALT INITIAL_START INITIAL_START_ART INITIAL_START_MRT LOTORDER ONE_CYCLE PP_MUSIC PP_PASSWORD PP_SELECT PP_SIGNALTOWER REMOTE_SAVE REMOTE_START REMOTE_UPDATE_PROGRAM RESET RETEST_MRT START_AQL START_LOT STOP_LOT SWITCH_TO_FT SWITCH_TO_RT TESTTEMPSETTING TRAY_FEED TRAY_MAP YIELD_FAIL

KYEC host actually sent (10): CLEAN_AUTO_SORT_COUNT ENERGY_SAVING INITIAL_START_ART LOTSTART ONE_CYCLE PP_MUSIC PP_SIGNALTOWER START START_AGV SWITCH_TO_FT

**HOT gap (host really sent it, HT160 has no branch): CLEAN_AUTO_SORT_COUNT ENERGY_SAVING INITIAL_START_ART ONE_CYCLE PP_MUSIC PP_SIGNALTOWER SWITCH_TO_FT**


## C. Event layer (CEID)

- 9045 firmware CEID catalog (EventReport_CEID.def): 286 defined
- HT160 CEID: 41 registered + 6 fired-but-unregistered = 47
- Numbers present on BOTH sides: 46 (aligned meaning 13 / CONFLICTING meaning 33)
- Numbers only in HT160: 142
- Numbers only in 9045: 240 (see report)

#### C-1. Same number, ALIGNED meaning (safe)

| CEID | HT9045 | HT160S |
|---|---|---|
| 35 | Auto1 Full | Auto1 Full |
| 36 | Auto2 Full | Auto2 Full |
| 37 | Auto3 Full | Auto3 Full |
| 136 | Auto 1 Unloading tray | Auto1 Unloadtray |
| 137 | Auto 2 Unloading tray | Auto2 Unloadtray |
| 138 | Auto 3 Unloading tray | Auto3 Unloadtray |
| 148 | Auto 4 Full | Auto4 Full |
| 149 | Auto 5 Full | Auto5 Full |
| 150 | Auto 6 Full | Auto6 Full |
| 272 | AMR Supplement | AGVSupplement |
| 273 | AMR LDUnLD Status | AGVLDUnLDStatus |
| 274 | AMR LDUnLD Finish | AGVLDUnLDFinish |
| 275 | AMR LD ID | AGVLdID |

#### C-2. Same number, DIFFERENT meaning (host will misread)

| CEID | HT9045 | HT160S |
|---|---|---|
| 1 | Start Pressed | Handler change status |
| 2 | Pause Pressed | Recipe Change |
| 3 | OneCycle Pressed | Press Clear Count button |
| 4 | CleanOut Pressed | Press Start button without IC |
| 5 | ClearCount Pressed | Press Start button with IC |
| 6 | Lot Start | Press Pause button |
| 7 | Lot | Press Home button |
| 8 | Lot End | Press One Cycle button |
| 9 | Switch Real Dummy Mode | Press Clean Out button |
| 10 | Switch Tester Online | Press Tray Feed button |
| 11 | Switch Production Mode | Press Lot Start button |
| 12 | Switch Engineer Mode | Press Lot End button |
| 13 | Switch Temperature Mode | Press Exit button |
| 14 | Switch StartMode | Press Retry button |
| 15 | Switch Setup File | Press Skip button |
| 16 | Switch UserLevel | Press Alarm Reset button |
| 17 | Enter Tool Page | Show Alarm |
| 18 | Enter Maintenance Page | Release Alarm |
| 19 | Enter Offset Page | Show Message |
| 20 | Enter Speed Page | Release Message |
| 21 | Enter IO Page | Switching User Level |
| 22 | Enter Message Page | Enter Setup Page |
| 23 | Enter Debug Page | Enter Maintenance Page |
| 24 | Exit Pressed | Enter I/O Page |
| 25 | Home Pressed | Enter Teach Page |
| 26 | Get Test Result | Enter SECS GEM Page |
| 27 | Change Machine State | One Cycle Finish |
| 28 | Retry Pressed | Clean Out Finish |
| 29 | Skip Pressed | Tray Feed Finish |
| 30 | Alarm Reset Pressed | Time Event |
| 31 | Tray End Pressed | Switching Real/Dummy Mode |
| 140 | Prepare Load Tray | Auto4 Unloadtray |
| 141 | GEM Control State Change | Auto5 Unloadtray |

#### C-3. CEIDs the 9045 actually FIRED at KYEC on 2026-06-08 vs HT160 coverage

| CEID | HT9045 meaning | S6F11 sent | aborted(disabled) | HT160 has # | HT160 meaning of that # |
|---|---|---|---|---|---|
| 1 | Start Pressed | 5 | 0 | YES | Handler change status |
| 2 | Pause Pressed | 24 | 0 | YES | Recipe Change |
| 3 | OneCycle Pressed | 5 | 0 | YES | Press Clear Count button |
| 4 | CleanOut Pressed | 6 | 0 | YES | Press Start button without IC |
| 6 | Lot Start | 2 | 0 | YES | Press Pause button |
| 14 | Switch StartMode | 16 | 12 | YES | Press Retry button |
| 15 | Switch Setup File | 1 | 0 | YES | Press Skip button |
| 17 | Enter Tool Page | 5 | 0 | YES | Show Alarm |
| 18 | Enter Maintenance Page | 1 | 0 | YES | Release Alarm |
| 20 | Enter Speed Page | 1 | 0 | YES | Release Message |
| 21 | Enter IO Page | 3 | 0 | YES | Switching User Level |
| 22 | Enter Message Page | 2 | 0 | YES | Enter Setup Page |
| 23 | Enter Debug Page | 9 | 0 | YES | Enter Maintenance Page |
| 25 | Home Pressed | 4 | 0 | YES | Enter Teach Page |
| 26 | Get Test Result | 12 | 0 | YES | Enter SECS GEM Page |
| 27 | Change Machine State | 406 | 3 | YES | One Cycle Finish |
| 28 | Retry Pressed | 9 | 0 | YES | Clean Out Finish |
| 34 | Auto Clean Start | 1 | 0 | no | - |
| 41 | One Cycle Finish | 3 | 0 | no | - |
| 42 | Clean Out Finish | 2 | 0 | no | - |
| 44 | Site On Off | 31 | 3 | no | - |
| 47 | Change HandlerSpeed | 2 | 0 | no | - |
| 49 | Tray Feed Finish | 1 | 0 | no | - |
| 50 | Auto Clean Finish | 1 | 0 | no | - |
| 53 | UPH Record Start | 5 | 0 | no | - |
| 55 | Initial ART Start | 1 | 1 | no | - |
| 58 | Ready for ART | 2 | 0 | no | - |
| 59 | ART Receive Tray OK | 2 | 0 | no | - |
| 60 | ART Receive Tray START | 1 | 0 | no | - |
| 63 | FT Finish | 1 | 0 | no | - |
| 66 | Load Tray Finish | 5 | 0 | no | - |
| 67 | Tray Test Finish | 6 | 0 | no | - |
| 70 | Barcode Reader Enter | 13 | 0 | no | - |
| 73 | Mymessbox OK | 7 | 0 | no | - |
| 76 | Start Pressed HasIC | 13 | 0 | no | - |
| 80 | Read Now Handler Data | 0 | 10962 | no | - |
| 93 | SECS/GEM Online Remote | 0 | 3 | no | - |
| 123 | Safe Door On Off | 9 | 0 | no | - |
| 124 | Save Recipe | 8 | 0 | no | - |
| 136 | Auto 1 Unloading tray | 2 | 0 | YES | Auto1 Unloadtray |
| 137 | Auto 2 Unloading tray | 3 | 0 | YES | Auto2 Unloadtray |
| 141 | GEM Control State Change | 0 | 3 | YES | Auto5 Unloadtray |
| 212 | Energy Saving Start | 1 | 0 | no | - |
| 213 | Energy Saving End | 1 | 0 | no | - |
| 250 | START Auto contact height | 1 | 0 | no | - |
| 272 | AMR Supplement | 2 | 0 | YES | AGVSupplement |
| 273 | AMR LDUnLD Status | 4 | 0 | YES | AGVLDUnLDStatus |
| 274 | AMR LDUnLD Finish | 4 | 0 | YES | AGVLDUnLDFinish |
| 275 | AMR LD ID | 2 | 0 | YES | AGVLdID |
