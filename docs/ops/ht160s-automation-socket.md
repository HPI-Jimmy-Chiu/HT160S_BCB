# HT160S_BCB Automation Socket

## Purpose

HT160S_BCB provides a lightweight TCP Automation socket for internal tests and future customer-side Lot setup.

The first implementation is intentionally narrow:

- It does not start motion.
- It does not press Lot Start or Lot End.
- It only reads status and updates Lot edit fields.
- If the main form is not ready yet, Lot setup is stored as pending and applied after the main form is attached.

## Default Endpoint

- Host: `127.0.0.1` or machine IP
- Port: `16060`
- Protocol: ASCII line protocol, commands end with `CRLF`

Optional configuration file:

```ini
[Automation]
Enabled=1
LocalOnly=1
Port=16060
```

Location: `system\automation.ini` from the HT160S_BCB repository root. If the file is missing, Automation is enabled on `127.0.0.1:16060`.

`LocalOnly=1` binds only to loopback and avoids the Windows firewall network-permission prompt during normal local testing. Set `LocalOnly=0` only when an external customer host must connect to this machine.

## Commands

### PING

Request:

```text
PING
```

Reply:

```text
OK|PONG|HT160S_BCB
```

### GET_STATUS

Request:

```text
GET_STATUS
```

Reply example:

```text
OK|STATUS|RUNNING=0;RUN_MODE=0;PORT=16060;BIND=127.0.0.1;MAIN_FORM_READY=0;PENDING_LOT=1;LOT_NO=
```

Fields:

- `RUNNING`: `1` when `HSys.Sys.SystemStart` is true, otherwise `0`.
- `RUN_MODE`: current `HSys.Sys.RunMode` integer.
- `BIND`: current Automation bind address.
- `MAIN_FORM_READY`: `1` after `TfMain` is attached.
- `PENDING_LOT`: `1` when a Lot setup command was accepted before the main form was ready.
- `LOT_NO`: current Lot number when the main form is available.

### GET_LOT_INFO

Request:

```text
GET_LOT_INFO
```

Reply example after the main form is ready:

```text
OK|LOT_INFO|LOT_NO=LOT001;WAFER_LOT=W1;CUS_DEVICE=DUT;INSERTION=I1;FLOW_ID=F1;OPERATOR=OP1;RUN_CARD=R1
```

If the main form is not ready:

```text
ERR|MAIN_FORM_NOT_READY
```

### SET_LOT_INFO

Request:

```text
SET_LOT_INFO LOT_NO=LOT001;WAFER_LOT=W1;CUS_DEVICE=DUT;INSERTION=I1;FLOW_ID=F1;OPERATOR=OP1;RUN_CARD=R1
```

Accepted keys:

- `LOT_NO` or `LOTNO`
- `WAFER_LOT` or `WAFERLOT`
- `CUS_DEVICE` or `DEVICE`
- `INSERTION`
- `FLOW_ID` or `FLOWID`
- `OPERATOR` or `OPERATOR_ID`
- `RUN_CARD` or `RUNCARD`

Reply when the main form is ready:

```text
OK|LOT_INFO|LOT_NO=LOT001;WAFER_LOT=W1;CUS_DEVICE=DUT;INSERTION=I1;FLOW_ID=F1;OPERATOR=OP1;RUN_CARD=R1
```

Reply when accepted before the main form is ready:

```text
OK|LOT_INFO_PENDING|MAIN_FORM_NOT_READY
```

Reply when the machine is running:

```text
ERR|SYSTEM_RUNNING
```

### SMOKE_TOP_FORMS

Internal smoke-test command for the manual-port build. It creates and briefly shows/hides the lazy top forms from the main page:

- Language
- Product
- Maintance
- Offset
- Speed
- Tools
- Message

Request:

```text
SMOKE_TOP_FORMS
```

Successful reply:

```text
OK|TOP_FORMS|OPENED=Language,Product,Maintance,Offset,Speed,Tools,Message
```

If the main form is not ready or the machine is running, it returns an `ERR|...` reply. This command is for startup smoke testing only; it does not start motion or modify Lot data.

## Value Encoding

Reply values encode these separators:

- `%` -> `%25`
- `;` -> `%3B`
- `=` -> `%3D`
- `|` -> `%7C`

Clients should use the same encoding for request values if a value contains protocol separators.

## Startup Evidence

Automation startup writes a small diagnostic log:

```text
logs\automation\automation_startup.log
```

Expected successful startup:

```text
Start enabled=1 port=16060 bind=127.0.0.1
Listen OK
```
