# PD Line-Following Tuning

How to tune the three knobs that control line following, and how to read the
car's behavior to know which one to change.

The three knobs (defines at the top of the sketch / config):

| Knob | Meaning |
|------|---------|
| `Kp` | proportional gain — the further off-line, the harder it corrects |
| `Kd` | derivative gain — damps left/right oscillation |
| `Basic_speed` | base speed — how fast overall |

> Values differ per platform. Arduino `line_follower.ino` starts at `kp 100 / kd 200 / Basic_speed 100`;
> MakeCode `line_follower` uses `PD(250, 350)` with `bassSPD 350`. Rule of thumb: **Kd ≈ 2–5× Kp**.

## Tuning procedure (change one thing at a time)

1. **Set the speed low first** (`Basic_speed` ~80–100). Slower is safer to tune.
2. **Kd = 0, tune Kp only.** Raise it from small until the car follows the line but
   weaves in an S-shape on straights — that means Kp is strong enough.
3. **Add Kd.** Raise it until the S-weave is damped: straights are smooth and corners
   aren't sluggish.
4. **Then raise the speed.** As `Basic_speed` goes up, Kp and Kd usually need to go up
   together too — re-check steps 2–3.

## Symptom → fix

| Behavior | Cause | Fix |
|----------|-------|-----|
| Weaves S-shape on straights | Kp too high, or Kd too low | lower Kp, or add Kd |
| Cuts corners / flies off track | Kp too low, or speed too high | add Kp, or lower speed |
| Sluggish, can't keep up with corners | Kp too low | add Kp |
| Twitchy / jittery / neurotic | Kd too high (amplifies sensor noise) | lower Kd |
| Fine when slow, goes wild when fast | speed and Kp/Kd mismatched | raise Kp and Kd together with speed |

## Tune with data, not blind

Let the car "talk" before tuning:

- **Arduino** — `Print1()` has its `Serial.print` lines commented out. Uncomment the values
  you want (position `x` / error `err` / `pd`), then watch **Serial Plotter** at 9600 baud
  while it runs.
- **MakeCode** — enable the commented `serial.writeValue`/`serial.write_value` line
  (`LinePOS` / `pos`) and open **Show data 裝置** to graph track position live.

Or expose `Kp`/`Kd`/`Basic_speed` to the buttons so you can tweak on the track instead of
recompiling and re-uploading each time.
