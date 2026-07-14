# Way 1 — MakeCode (block-based, micro:bit)

The beginner-friendly path. You program the **micro:bit** with visual blocks; the micro:bit
sits on the car as the host controller and talks to the on-board MCU (ATSAMD21G18) over I2C.

> Prefer text code / want to program the MCU directly? See [Way 2 — Arduino IDE](../arduino/README.md).

## Two algorithms in here

| Folder | What it is |
|--------|-----------|
| [`line_follower/`](line_follower/) | The **workshop algorithm** — rebuilt from the slides (p.35–43). TypeScript + Python. Start here. |
| [`my_follower/`](my_follower/) | A **competition-style algorithm** written from scratch (PID + line-loss memory + adaptive speed). |

Both reuse the same extension and the same setup below. Read this page first, then pick a folder.

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

## Step 2 — Load an algorithm

- **TypeScript** file → paste into the MakeCode **JavaScript** tab, then switch to **Blocks** (converts automatically).
- **Python** file → paste into the MakeCode **Python** tab.

(Blocks / JavaScript / Python are three views of one program.) Then **Download** to the micro:bit.

Go to the folder for the algorithm you want:

- **[`line_follower/`](line_follower/README.md)** — the workshop version (recommended first)
- **[`my_follower/`](my_follower/README.md)** — the competition version

## How to drive it (both algorithms)

| Button | Action |
|--------|--------|
| **A** | Calibrate — car nudges forward and records each sensor's max/min |
| **B** | Start line following |
| **A+B** | Stop |

Typical run: place the car on the line → press **A** (calibrate over the line) → press **B** (it follows).

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
