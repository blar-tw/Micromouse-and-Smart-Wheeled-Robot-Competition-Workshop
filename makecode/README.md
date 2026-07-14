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

Two versions of the same program — pick one (Blocks/JavaScript/Python are three views of one program):

- **TypeScript**: open [`line_follower.ts`](line_follower.ts), copy everything into the MakeCode
  **JavaScript** tab, then switch back to **Blocks** — it converts automatically.
- **Python**: open [`line_follower.py`](line_follower.py), copy everything into the MakeCode
  **Python** tab. (This is *MakeCode Python*, not MicroPython — it only works inside MakeCode.
  If any API name shows red there, the authoritative names come from pasting the `.ts` into the
  JavaScript tab and switching to the Python tab.)

Then **Download** to the micro:bit.

### Step 2b — Competition-style algorithm (optional upgrade)

[`my_follower.py`](my_follower.py) is a from-scratch algorithm using techniques common in
line-follower racing. Same A/B/A+B controls as the workshop version. What it adds:

| Technique | Why |
|-----------|-----|
| Read all 7 sensors in **one** I2C transaction (`get_all_ir_values`) | 7× fewer bus transactions → faster control loop |
| Position scale −2000..+2000 (Pololu `readLine` convention) | finer resolution than 5 discrete weights |
| **Line-loss memory**: report full-scale position toward the last-seen side | PD naturally steers back to reacquire the line |
| Full **PID** with anti-windup (KI defaults to 0) | add tiny KI only if the car consistently hugs one side |
| **Adaptive speed**: slow in curves, full speed on straights | the main trick for fast lap times |
| D-term low-pass filter | less jitter from sensor noise |
| Lost-line timeout auto-stop (1.2 s) | safety |
| Marker detection with **hysteresis** (>600 on, <400 off) | no double-counting from flicker |
| Pressing B resets counters/PID state | workshop version would instantly stop on a second run |

All tunables sit in the config block at the top (`KP/KI/KD`, `SPEED_MAX/MIN`, `LINE_IS_WHITE`, …).
Set `LINE_IS_WHITE = False` for a black-line-on-white track.

References: [Pololu QTR `readLine` behavior](https://www.pololu.com/docs/0J19/3) ·
[Pololu 3pi line-following example](https://www.pololu.com/docs/0J21/7.b) ·
[PID tuning guide (ThinkRobotics)](https://thinkrobotics.com/blogs/learn/pid-tuning-for-line-follower-robot-complete-how-to-guide) ·
[PID tuning for speed competitions (Zbotic)](https://zbotic.in/pid-line-follower-robot-tuning-speed-competition/)

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
- Sensor index: `getIR(0..4)` = IR-1..IR-5, `getIR(5)` = IR-L, `getIR(6)` = IR-R.
- **Red "Cannot find name 'SmartCar'" errors in VS Code are expected** — the `SmartCar` blocks are
  defined inside MakeCode (by the extension), not in this repo. The file runs fine once pasted into
  MakeCode. Don't try to "fix" these locally.

## Extension reference (from the extension source)

I2C slave address: **`0x10`**. You normally use the blocks (they wrap the I2C for you). Available blocks:

| Block | Function |
|-------|----------|
| `SmartCar.setIRPower(on)` | IR power on/off (cmd 0x37) |
| `SmartCar.getIR(index)` | read one sensor, index 0–6 (cmd 0x30 + index) |
| `SmartCar.getAllIRValues()` | read all seven at once (cmd 0x38) |
| `SmartCar.setMotor(MotorList.左輪/右輪/雙輪, speed)` | motor, speed ±1000 (cmd 0x20/0x21/0x22) |
| `SmartCar.setLED(LEDList.左邊/右邊/兩邊, on)` | LEDs (cmd 0x42) |
| `SmartCar.getUltrasonicDistance()` | ultrasonic cm (cmd 0x43) |
| `SmartCar.getEncoderValue(wheel)` / `clearEncoders()` | encoders (cmd 0x40 / 0x41) |
| `SmartCar.lcdShowString/lcdShowNumber/lcdClear/setBackgroundColor/showLogo` | 1.8" LCD (cmd 0x44–0x47) |
