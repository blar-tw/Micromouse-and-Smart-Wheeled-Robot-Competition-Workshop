# MakeCode Resources

## Extension Blocks

BitRacer Pro Max custom extension blocks (simplify I2C communication between the
micro:bit and the on-board MCU):

- GitHub: <https://github.com/CorgiQQQ/BitRacer_Pro_Max>
- In MakeCode, go to **Extensions** and paste the URL above to add the blocks

Provided blocks (sensor control):

- IR power on/off
- Read the index-th sensor
- Read all seven IR sensors at once
- Set LED state
- Read ultrasonic distance (cm)
- Set motor (left / right / both) speed

## Example Programs

- Parking program: <https://makecode.microbit.org/S92273-39996-50243-0737>

## Line-Following Flow (workshop example)

1. **Button A**: calibration mode 1 — robot moves forward and samples each sensor's max/min
2. **Button B**: calibration mode 2 — start line following (IR sample → normalize → weight → PD(250, 350) → start/finish detection)
3. **Button A+B**: calibration mode 3 — stop (both wheels speed 0)

Core PD function (KP, KD are parameters):

```
pError  = 3 − LinePOS
dError  = pError − pErrorOld
PDvalue = KP × pError + KD × dError
left  wheel speed = clamp(bassSPD − PDvalue, −1000, 1000)
right wheel speed = clamp(bassSPD + PDvalue, −1000, 1000)
```
