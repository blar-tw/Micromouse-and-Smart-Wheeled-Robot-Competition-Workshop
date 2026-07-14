# Way 1 — MakeCode (block-based, micro:bit)

The beginner-friendly path. You program the **micro:bit** with visual blocks; the micro:bit
sits on the car as the host controller and talks to the on-board MCU (ATSAMD21G18) over I2C.

> Prefer text code / want to program the MCU directly? See [Way 2 — Arduino IDE](../arduino/README.md).

## What you need

- A micro:bit (V2 recommended) seated on the BitRacer Pro Max
- A micro-USB cable
- Chrome or Edge browser (for one-click download / WebUSB), at <https://makecode.microbit.org/>

## Step 0 — Add the extension (do this first!)

Without this you won't see any of the `SmartCar` (循線自走車) blocks.

1. In MakeCode, open **Extensions**
2. Paste this URL and add it: `https://github.com/CorgiQQQ/BitRacer_Pro_Max`
3. A new **循線自走車 / SmartCar** block category appears

## Step 1 — Verify the connection with a tiny program

Don't start with the full program. First confirm the micro:bit actually talks to the car.

Paste this into the **JavaScript** tab, then switch back to **Blocks**:

```javascript
SmartCar.setIRPower(true)
basic.forever(function () {
    basic.showNumber(SmartCar.getIR(2))   // show middle sensor IR-3
})
```

Connect the micro:bit (plug in USB → `...` next to **Download** → **Connect Device** → pair),
then **Download**. Move the middle sensor over the white line vs. the black surface — the number
on the LED grid should change clearly. If it does, wiring + I2C + extension all work.

Motor test (press A to nudge forward):

```javascript
input.onButtonPressed(Button.A, function () {
    SmartCar.setMotor(SmartCar.MotorList.雙輪, 300)
    basic.pause(500)
    SmartCar.setMotor(SmartCar.MotorList.雙輪, 0)
})
```

## Step 2 — Load the full line follower

1. Open [`line_follower.ts`](line_follower.ts)
2. Copy everything into the MakeCode **JavaScript** tab
3. Switch back to **Blocks** — it converts to blocks automatically
4. **Download** to the micro:bit

### How to drive it

| Button | Action |
|--------|--------|
| **A** | Calibrate — car nudges forward and records each sensor's max/min |
| **B** | Start line following |
| **A+B** | Stop |

Typical run: place the car on the line → press **A** (calibrate over the line) → press **B** (it follows).

The main loop uses a `校正模式` (mode) variable as a switch:
mode 1 = calibrate, mode 2 = follow (sample → normalize → weight → `PD(250,350)` → finish check), mode 3 = stop.

## Getting output (for tuning)

In `line_follower.ts`, the `權重()` function has a commented line:

```javascript
// serial.writeValue("LinePOS", LinePOS)
```

Uncomment it, download, then click **Show data 裝置** below the simulator to see the track
position live (stream + downloadable CSV). This tells you whether the car is wobbling or lagging.

## Tuning knobs

In `line_follower.ts`:

| Where | Meaning |
|-------|---------|
| `PD(250, 350)` in the forever loop | `KP = 250` (correction strength), `KD = 350` (damping) |
| `let bassSPD = 350` | base speed |

Tuning method (symptom → fix table) is in [`../NOTE.md`](../NOTE.md).

## Notes / troubleshooting

- **No `SmartCar` blocks?** You skipped Step 0 (add the extension).
- **Can't connect / no download to device?** Use Chrome or Edge (WebUSB). You can also download
  the `.hex` and drag it onto the `MICROBIT` drive.
- **LED blocks in `起終點()` are commented out** — line following and finish-stop work without them.
  The exact LED block names weren't confirmed against the extension; enable those lines once verified.
- Sensor index: `getIR(0..4)` = IR-1..IR-5, `getIR(5)` = IR-L, `getIR(6)` = IR-R.
