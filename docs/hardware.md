# BitRacer Pro Max Hardware Notes

## System Architecture

```
Micro USB ──5V──▶ TP4056 charge IC ──▶ 14500 Li-ion battery (4.2V)
                                        │
                    ┌───────────────────┤
                    ▼                   ▼
            TB6612FNG motor driver  MCP1826 LDO ──3.3V──▶ micro:bit
                    │                   │
                    ▼                   ▼
                 Motors ◀──PWM── MCU (ATSAMD21G18) ◀──I2C──▶ micro:bit
                                 │        │
              ┌──────────┬───────┼────────┼──────────┐
              ▼          ▼       ▼        ▼          ▼
            Buzzer   Ultrasonic  IR      Magnetic    1.8" TFT LCD
                    (HC-SR04P) (VCNT2020) encoders    (SPI)
```

## Arduino Pin Definitions (ATSAMD21G18)

### IR Sensors

| Define | Pin |
|--------|-----|
| IR_1 | A6 |
| IR_2 | A5 |
| IR_3 | A4 |
| IR_4 | A3 |
| IR_5 | A0 |
| IR_L | A2 |
| IR_R | A1 |
| IRcontrol (IR power switch) | 13 |

### LEDs and Button

| Define | Pin |
|--------|-----|
| LED_R | 30 |
| LED_L | 31 |
| BUTTON | 25 |

### Motors (TB6612FNG)

| Define | Pin | Description |
|--------|-----|-------------|
| PWML | 2 | Left motor PWM |
| BIN_1 | 28 | Left motor direction 1 |
| BIN_2 | 29 | Left motor direction 2 |
| AIN_1 | 32 | Right motor direction 1 |
| AIN_2 | 33 | Right motor direction 2 |
| PWMR | 3 | Right motor PWM |

## Core Formulas

### IR Normalization

```
y = (y_max − y_min) / (AD_max − AD_min) × (AD − AD_min) + y_min
```

### Track Position Estimation (weighted average)

```
Lp = (S1·x1 + S2·x2 + S3·x3 + S4·x4 + S5·x5) / (S1 + S2 + S3 + S4 + S5)
Sensor position weights x = [-2, -1, 0, 1, 2]
```

### PD Feedback Control

```
ΔPWM  = Kp·e[n] + Kd·(e[n] − e[n−1])
PWM_L = PWM_B − ΔPWM
PWM_R = PWM_B + ΔPWM
```

### Ultrasonic Ranging

```
D = t × 340 / 2   (t = round-trip time, speed of sound 340 m/s)
```

### Encoder Odometry

```
d = (PL + PR) / 2 × (π × 23) / 32   (wheel diameter 23 mm, 32 pulses per revolution)
```
