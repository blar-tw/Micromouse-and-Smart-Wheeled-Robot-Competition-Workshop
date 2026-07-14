# Competition algorithm — `my_follower`

A line-following algorithm written from scratch using techniques common in line-follower **racing**,
rather than the workshop's teaching version. Same A / B / A+B controls.

> New here? Do the setup and connection test in the [parent README](../README.md) first.

## File

- [`my_follower.py`](my_follower.py) — **MakeCode Python**. Paste into MakeCode's **Python** tab, then Download.

## What it adds over the workshop version

| Technique | Why |
|-----------|-----|
| Read all 7 sensors in **one** I2C transaction (`get_all_ir_values`) | 7× fewer bus transactions → faster control loop |
| Position scale −2000..+2000 (Pololu `readLine` convention) | finer resolution than 5 discrete weights |
| **Line-loss memory**: report full-scale position toward the last-seen side | PD naturally steers back to reacquire the line |
| Full **PID** with anti-windup (KI defaults to 0) | add a tiny KI only if the car consistently hugs one side |
| **Adaptive speed**: slow in curves, full speed on straights | the main trick for fast lap times |
| D-term low-pass filter | less jitter from sensor noise |
| Lost-line timeout auto-stop (1.2 s) | safety |
| Marker detection with **hysteresis** (>600 on, <400 off) | no double-counting from flicker |
| Pressing B resets counters / PID state | workshop version would instantly stop on a second run |

## Config (all at the top of the file)

| Setting | Meaning |
|---------|---------|
| `KP`, `KI`, `KD` | PID gains (defaults 0.25 / 0 / 1.0 — field-tested starting point, see log below) |
| `SPEED_MAX`, `SPEED_MIN` | straight-line vs. curve base speed (defaults 300 / 180) |
| `SPEED_LOST` | speed while searching for a lost line |
| `CORR_MAX` | clamp on a single steering correction — prevents violent full-speed swings (and the current spike they cause) |
| `LINE_IS_WHITE` | `True` for white line on black; set `False` for black line on white |
| `LINE_THRESHOLD` | how bright (0–1000) counts as "seeing the line" |
| `LOST_TIMEOUT` | ms without a line before auto-stop |
| `FINISH_COUNT` | right-side markers to count before stopping (start + finish = 2) |

## Field log (real-track debugging record)

**2026-07-14 — Pressing B rebooted the micro:bit.** Button A (300 ms nudge) was fine; B
(continuous drive) browned out the shared battery rail and reset the micro:bit. Causes: speeds too
high for the battery + the D-term spiking to full scale the instant the line was lost (`pos` jumps
to ±2000 → near ±1000 wheel command → stall-level current). Fixes: lowered `SPEED_MAX/MIN/LOST`
to 300/180/150, added the `CORR_MAX = 500` clamp, and D is now zeroed while the line is lost.

**2026-07-14 — Car ran but vibrated left–right constantly.** Cause: **KD was ~11× too hot due to a
unit-scale mistake.** This program's position scale is −2000..+2000 (1000 per sensor spacing) vs the
workshop's 1..5 (1 per spacing), so gains must be ~1000× smaller to be equivalent. `KD = 4.0` here
equals ~4000 in workshop units (theirs: 350). The D-term amplified per-loop sensor noise into rapid
alternating corrections that slammed the `CORR_MAX` clamp — bang-bang steering = vibration.
(Matches the NOTE.md symptom table: "twitchy/jittery → KD too high".) Fixes: `KP` 0.35 → **0.25**
(equivalent to the workshop's proven 250), `KD` 4.0 → **1.0** (~3× workshop damping, reasonable with
the low-pass filter). Rule of thumb recorded in the code: **on this scale, KD +1 ≈ workshop +1000.**

## Tuning order

1. Set `KI = 0`, raise `KP` until it follows the line but wobbles on straights
2. Raise `KD` until the wobble is damped out
3. Raise `SPEED_MAX` for speed; re-check `KP`/`KD` as it gets faster

Full symptom → fix table is in [`../../NOTE.md`](../../NOTE.md). Enable the
`serial.write_value("pos", pos)` line (in `on_forever`) and use **Show data 裝置** to watch position while tuning.

## References

- [Pololu QTR `readLine` behavior](https://www.pololu.com/docs/0J19/3)
- [Pololu 3pi line-following example](https://www.pololu.com/docs/0J21/7.b)
- [PID tuning guide (ThinkRobotics)](https://thinkrobotics.com/blogs/learn/pid-tuning-for-line-follower-robot-complete-how-to-guide)
- [PID tuning for speed competitions (Zbotic)](https://zbotic.in/pid-line-follower-robot-tuning-speed-competition/)
