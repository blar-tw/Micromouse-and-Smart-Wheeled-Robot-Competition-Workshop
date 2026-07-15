# Way 2 — Arduino IDE (text-based, direct to the MCU)

The advanced path. You program the car's on-board MCU (**ATSAMD21G18**) directly in C/C++.
**No micro:bit is involved** — the micro:bit is only used in Way 1.

> Prefer visual blocks? See [Way 1 — MakeCode](../makecode/README.md).

## Can I use VS Code instead of the Arduino IDE app?

Short answer: **use the Arduino IDE for this board.** Longer answer:

- **Arduino IDE 2.x** — recommended here. The workshop's custom board definition installs straight
  into it via a Boards Manager URL, and the pin numbers in the sketch assume that board.
- **VS Code + PlatformIO extension** — a fully independent option with a nicer editor, but you'd
  have to re-create the BitRacer's custom SAMD21 board/pin config in `platformio.ini` yourself.
  Extra work; not worth it just to get started.
- **VS Code + Microsoft "Arduino" extension** — deprecated, and it still needs the Arduino IDE /
  `arduino-cli` installed underneath, so it doesn't actually save you the install.

Bottom line: start with Arduino IDE 2.x. Move to PlatformIO later if you want to live in VS Code.

## What you need

- [Arduino IDE 2.x](https://www.arduino.cc/en/software)
- The workshop's **SAMD21 / BitRacer board package** (ask your instructor for the Boards Manager URL)
- A micro-USB cable

## Step 1 — Install the board definition ⚠️ important

The sketch's pin numbers (LEDs 30/31, button 25, motors 28/29/32/33, etc.) are **specific to the
BitRacer board mapping**. You must install the board package the workshop provides — a generic
"Arduino Zero" definition will compile but the pins won't line up.

1. Arduino IDE → **File → Preferences → Additional boards manager URLs** → paste the workshop URL
2. **Tools → Board → Boards Manager** → search for and install that SAMD package
3. **Tools → Board** → select the BitRacer / SAMD21G18 board

## Step 2 — Connect the car

Plug the micro-USB into the **USB port on the car board** (next to the power switch), not the
micro:bit. This one cable powers, programs, and carries serial data.

## Step 3 — Open the sketch and select the port

1. Open [`line_follower.ino`](line_follower/line_follower.ino)
2. **Tools → Port** → select the COM port that appeared

## Step 4 — Upload

Click the **→ (Upload)** arrow. Wait for "Done uploading."

## Step 5 — Run it

The program waits for the button after upload:

1. Place the car on the line
2. Press the **button on the car** → it nudges forward to calibrate the IR max/min
3. It then follows the line using PD control
4. It stops after the right-side marker passes **twice** (finish line)

## Getting output (for tuning)

1. **Tools → Serial Monitor**, set baud to **9600** (matches `Serial.begin(9600)`)
2. The `Print1()` function has all its `Serial.print` lines commented out — uncomment the ones you
   want (e.g. the normalized values) to see data
3. For live graphs, use **Tools → Serial Plotter** (also 9600) instead of the monitor

## Tuning knobs

At the top of `line_follower.ino`:

| Define | Meaning |
|--------|---------|
| `#define kp 100` | proportional gain (correction strength) |
| `#define kd 200` | derivative gain (damping) |
| `#define Basic_speed 100` | base speed |

Tuning method (symptom → fix table) is in [`../docs/pd-tuning.md`](../docs/pd-tuning.md).

## Reference

- Pin definitions, wiring diagram, and all formulas: [`../docs/hardware.md`](../docs/hardware.md)
- I2C command table (used by the MCU firmware): [`../docs/i2c-commands.md`](../docs/i2c-commands.md)

## Notes / troubleshooting

- **Port not showing / upload fails?** Many SAMD21 boards need a quick **double-tap of the reset
  button** to enter the bootloader; then re-select the port and upload.
- **Compiles but motors/sensors behave wrong?** Almost always the wrong board definition (Step 1) —
  the pin numbers are BitRacer-specific.
- Serial Monitor is empty until you uncomment lines in `Print1()`.
- Sensor array layout differs from Way 1: here `IR[0]=IR_R`, `IR[6]=IR_L`, `IR[1..5]=IR_1..IR_5`.
