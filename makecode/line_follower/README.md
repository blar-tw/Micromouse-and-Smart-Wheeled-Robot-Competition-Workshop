# Workshop algorithm — `line_follower`

The line-following algorithm taught in the workshop, rebuilt from the presentation slides (p.35–43).
This is the **official / teacher's** version.

> New here? Do the setup and connection test in the [parent README](../README.md) first.

## Files

- [`line_follower.ts`](line_follower.ts) — TypeScript. Paste into MakeCode's **JavaScript** tab,
  then switch back to **Blocks** — it converts automatically.
- [`line_follower.py`](line_follower.py) — the same program in **MakeCode Python**. Paste into the
  **Python** tab. (MakeCode Python, not MicroPython — only works inside MakeCode.)

Pick one file; they are the same program.

## What "teacher's / GitHub" means here

- The **algorithm** (calibrate → normalize → weighted position → PD → finish detection) comes from
  the **workshop slides**.
- The **`SmartCar` blocks** it calls come from the instructor's **GitHub extension**
  (<https://github.com/CorgiQQQ/BitRacer_Pro_Max>) — that repo is the communication library only,
  it does not contain the line-following logic.

So this program = **slides' algorithm + GitHub's communication blocks**.

## How it works

The main loop switches on a `校正模式` (mode) variable:

- **mode 1 (button A)** — calibrate: sample every sensor's min/max as the car nudges over the line
- **mode 2 (button B)** — follow: sample → normalize → weighted position `LinePOS` → `PD(250,350)` → finish check
- **mode 3 (button A+B)** — stop

Position is a weighted average of IR-1..IR-5 (center = 3), and steering is
`ΔPWM = KP·e + KD·(e − e_prev)` applied as `left = base − ΔPWM`, `right = base + ΔPWM`.

## Tuning knobs

| Where | Meaning |
|-------|---------|
| `PD(250, 350)` in the forever loop | `KP = 250` (correction strength), `KD = 350` (damping) |
| `bassSPD = 350` | base speed |

Tuning method (symptom → fix table) is in [`../../NOTE.md`](../../NOTE.md).

## Getting output (for tuning)

In `line_follower.ts`, the `權重()` function has a commented line:

```javascript
// serial.writeValue("LinePOS", LinePOS)
```

Uncomment it, download, then click **Show data 裝置** below the simulator to see the track position live.
