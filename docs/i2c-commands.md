# BitRacer Pro Max I2C Command Table

The micro:bit (host) sends commands over I2C to the on-board MCU ATSAMD21G18 (slave).

## Basic Commands

| Function | Command | R/W |
|----------|---------|-----|
| Motor left wheel | 0x20 | W |
| Motor right wheel | 0x21 | W |
| Motor both wheels | 0x22 | W |
| Read IR-1 | 0x30 | R |
| Read IR-2 | 0x31 | R |
| Read IR-3 | 0x32 | R |
| Read IR-4 | 0x33 | R |
| Read IR-5 | 0x34 | R |
| Read IR-L | 0x35 | R |
| Read IR-R | 0x36 | R |
| IR power switch | 0x37 | W |
| Read all IR | 0x38 | R |

## Peripheral Commands

| Function | Command | R/W |
|----------|---------|-----|
| Read encoder values | 0x40 | R |
| Reset encoders to zero | 0x41 | W |
| LED control | 0x42 | W |
| Read ultrasonic distance | 0x43 | R |
| Show text/number (LCD) | 0x44 | W |
| Clear screen | 0x45 | W |
| Show image | 0x46 | W |
| Set background color | 0x47 | W |
