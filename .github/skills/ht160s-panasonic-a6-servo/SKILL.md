---
name: ht160s-panasonic-a6-servo
description: "Use when working on the Panasonic MINAS A6 servo DRIVE side behind the MC88X1 axis cards in HT160S_BCB: electronic gear / command pulse resolution (Pr0.08/Pr0.09/Pr0.10), encoder pulse output division (Pr0.11/Pr5.03), 23-bit encoder (8,388,608 P/r), command-vs-encoder scale mismatch (the Now Position / Encoder 4x difference in Motor Test), drive alarms Err27.2 / Err25.0, position-control pulse-train wiring, absolute encoder battery/init. Triggers: Panasonic A6, MINAS, Pr0.08, Pr0.09, Pr0.10, Pr0.11, Pr5.03, electronic gear, 电子齿轮, 分倍频, command pulses per rev, pulse output per rev, 23bit encoder, 8388608, quadrature x4, Now Position Encoder not synced / 4x, Encoder mismatch, Err27.2, Err25.0, servo parameter."
---

# HT160S Panasonic MINAS A6 Servo (drive side) Skill

## Authoritative reference (read before changing any A6 gear/encoder behaviour)

- `docs/panasonicA6/panasonicA6_servo-notes.md` — distilled digest (UTF-8) of the
  facts below, with the exact formulas and the page index into the manual.
- `docs/panasonicA6/panasonicA6_document.pdf` — full vendor manual (Simplified
  Chinese, 596 pages). Use PyMuPDF (`fitz`) or `pdftotext` to extract pages; the
  Read tool's PDF rendering needs poppler which is not installed here.

This is the DRIVE-side companion to the card-side skill `ht160s-motion-card`
(MC88X1). All 20 axes are MC88X1 cards driving Panasonic A6 servos in **pulse-train
position control (P) mode**: the card sends command pulses, the A6 closes the
position loop internally on its 23-bit encoder, and the A6 echoes encoder feedback
on OA/OB line-driver outputs back to the card for monitoring only.

## The non-obvious invariants

- **Encoder resolution E = 2^23 = 8,388,608 pulse/rev** (23-bit absolute). This is
  the internal feedback unit; it is NOT what the card counts (see below).
- There are **two independent "pulses per revolution" settings**, one per direction,
  and they do not auto-match:
  - **Command side** `Pr0.08` = command pulses per motor rev (factory 10000). This is
    what the card's *theoretical* register (`MC88X1PGetTheorecticalRegister`) counts
    and what `myMC88X1motor.cpp ReadMC88X1RealPos()` reads -> **Now Position**.
    (`Pr0.09`/`Pr0.10` are the numerator/denominator alternative, used only when
    `Pr0.08`=0.)
  - **Feedback side** `Pr0.11` = output pulses per rev on OA/OB (factory 2500),
    `Pr5.03` = output divider denominator (factory 0). The card's *practical*
    register (`MC88X1PGetPracticalRegister`, `ReadMC88X1EnCoderRealPos()`) counts
    OA/OB with **x4 quadrature**, so it reads **Pr0.11 x 4** per rev -> **Encoder**.
- Factory alignment: Pr0.08=10000 and Pr0.11 x4 = 2500 x4 = 10000 -> Command and
  Encoder read the same. They diverge only when these two settings disagree.

## The "Now Position vs Encoder differ by 4x" diagnosis (Motor Test)

```
Encoder / NowPosition = (Pr0.11 x 4) / (command pulses per rev, ~Pr0.08)
```
Both values are multiplied by the SAME Mot_Table GearRatio in `myMC88X1motor.cpp`,
so GearRatio is NOT the cause. A constant 4.0 ratio means `Pr0.11 x4 = 4 x Pr0.08`,
i.e. `Pr0.11` was set equal to the command pulses/rev (e.g. both 10000); the card's
x4 quadrature then makes Encoder read 4x.

**The x4 is the MC88X1 card's encoder input multiplier, not the drive.** InitMotor in
`myMC88X1motor.cpp` calls `SetEncodeMultiple(...)` = `MC88X1PSetEncoderMultiple`, whose
value means (MC88 manual `docs/MC88X1_Driver/MC88系列使用手冊v35.pdf` P.53):
0=CW/CCW, 1=A/B x1, 2=A/B x2, 3=A/B x4. So practical register = OA/OB (Pr0.11/rev) x
card-multiplier.

CONFIRMED + RESOLVED: most axes ship at the factory Pr0.11=2500, which the card's x4
already aligns (2500x4=10000=command). An early (2026-06-17) reading mistook this for
"every axis Pr0.08=Pr0.11=10000" and briefly flipped InitMotor to `SetEncodeMultiple(1)`.
Re-check found only M12 `MTopCCDX` and M20 `MTopCCDX_Color` (same mechanism + same drive
setup) were set to Pr0.11=10000, so only those two read 4x (10000x4=40000 vs command 10000).
Final fix (2026-06-22): reset Pr0.11 from 10000 to 2500 on the M12 and M20 drives so ALL
20 axes are Pr0.11=2500, and `database.cpp` now calls `SetEncodeMultiple(3)` (x4) uniformly
for every axis (the old `Alias=="MTopCCDX_Color"?1:3` per-axis case was removed). The
practical/encoder register is monitor-only in pulse-train P mode (drive closes the loop;
InPos comes from the drive signal; soft limits track the command counter), so the multiplier
does not affect positioning. Needs on-machine read-back to confirm.

Why the drive (not the card) was changed: with the card kept at x4, aligning an axis just
means Pr0.11 = Pr0.08/4 = 2500 (a `*` param: front panel, EEPROM write, power cycle). Only
two drives were off, so this beat re-introducing a card-side per-axis multiplier. ALWAYS
read an axis's real `Pr0.08`/`Pr0.11` before picking a number -- code cannot read drive
parameters.

Position-sign convention (RESOLVED 2026-06-22, matches HT9045): all per-axis polarity is
one flag, one operation -- `if(Direction) v=-v` -- applied SYMMETRICALLY to the command
read (`ReadMC88X1RealPos`), encoder read (`ReadMC88X1EnCoderRealPos`), command write
(`SetCommand`) and encoder write (`SetPosition`); the card encoder dir is a fixed
`SetEncodeDir(1)` for every axis (never varied per-axis), exactly as HT9045
(`Motor/HTMC88X1Motor.cpp:116` dir=1, `:492`/`:508` both reads `if(Direction)`,
`RealG00` write `if(Direction)`; its raw `SetPos`/`SetEnCoderPos` carry NO flip and are
only called with 0). The old HT160S asymmetry was `SetPosition` using `if(Direction==0)`
while everything else used `if(Direction)`, so after a non-zero SetPos the Encoder read
back inverted vs NowPos (they agreed only at 0). The encoder/practical register is
monitor-only in P mode, so this was a display-side defect, not a motion one. Pending
on-machine read-back to confirm. HT172 never showed it: MN200/SMC apply the same
`if(Direction)` to both command and encoder, and a step motor's "encoder" IS the command
counter (cannot disagree).

## Alarms tied to gear/encoder setup

- **Err27.2** command-pulse-multiply abnormal: gear ratio too aggressive. Keep the
  divide/multiply ratio within 1/1000..1000 (full-closed 1/1000..8000).
- **Err25.0** hybrid deviation over-limit: command division not fixed in full-closed
  mode. Alarm-code details: manual P.393-412.

## Scope / boundaries

- This skill is drive parameters and the command<->encoder scaling contract only.
  Card register reads, jog/home/speed model, Mot_Table, AxisParaSet ranges live in
  `ht160s-motion-card`.
- Do NOT change `Pr*` values in code — they live in the drive's EEPROM, set via the
  front panel or PANATERM. Code changes are limited to how the card registers are
  read/scaled in `myMC88X1motor.cpp`.
- Honour the write boundary: the manual + this digest are under `D:\HT160S_BCB`
  (writable). Update the digest when you learn more from the PDF; do not re-derive
  these constants from memory.
