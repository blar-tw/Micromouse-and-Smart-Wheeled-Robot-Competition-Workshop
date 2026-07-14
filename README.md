# Micromouse & Smart Wheeled Robot Competition Workshop

2026 TMIRC Summer Camp — Dual-Platform Wheeled Robot Design & Application (BitRacer Pro Max)

Instructors: Su Ching-Hui, Li Ping-Hui | BitRacer Pro Max developer: Cheng Li-Wei (Lunghwa University of Science and Technology)

## Overview

BitRacer Pro Max is an educational wheeled robot designed around Taiwan's 108 Curriculum
and STEM education. It uses a **dual-platform development architecture**:

- **MakeCode (block-based)** — for beginners. The micro:bit acts as the host controller and
  talks to the on-board MCU (ATSAMD21G18) over I2C.
- **Arduino IDE (text-based)** — programs the on-board ATSAMD21G18 directly, for advanced learners.

## Hardware Specs

| Item | Detail |
|------|--------|
| Microcontroller | ATSAMD21G18 (on-board) + micro:bit (upper-level host) |
| Motors | 2× DC motors, TB6612FNG driver, four-wheel gear drive |
| Sensors | 5× IR reflectance sensors (VCNT2020) + 2× left/right expansion |
| Ranging | HC-SR04P ultrasonic module (expansion slot) |
| Encoders | Magnetic encoders (left/right wheel) |
| Display | 1.8" TFT LCD (128×160, SPI) |
| Other | Passive buzzer, 2× LED, 2× buttons |
| Power | 14500 Li-ion battery, TP4056 charging IC, MCP1826 LDO (3.3V) |

## Repository Structure

```
├── slides/      Workshop presentation (PDF)
├── docs/        Hardware notes and I2C command table
├── arduino/     Arduino IDE line-following sample (PD control)
└── makecode/    MakeCode extension blocks and example links
```

## Quick Links

- MakeCode extension blocks (BitRacer Pro Max): <https://github.com/CorgiQQQ/BitRacer_Pro_Max>
- MakeCode parking example: <https://makecode.microbit.org/S92273-39996-50243-0737>
- MakeCode environment: <https://makecode.microbit.org/>

## Course Units (MakeCode block modules)

1. Development environment & extension blocks
2. IR values & normalization
3. Track position estimation (weighted average)
4. Proportional-Derivative (PD) feedback control
5. Ultrasonic distance monitoring
6. Encoder & LCD display
