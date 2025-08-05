# Embedded C
This repository contains the work I completed during the **Piscine Embarquée** at 42, focused on embedded systems programming using the **ATmega328P** microcontroller.

>*The Piscine Embarquée is a 2-week intensive program at 42 School that introduces low-level embedded C programming, electronics fundamentals, and direct interaction with microcontroller registers (without relying on high-level libraries).

## Tools Used
- **AVR-GCC** (`avr-gcc`, `avr-objcopy`)
- **AVRDUDE** for flashing binaries
- **screen** for serial monitoring
- **LEDs, resistors, buttons, potentiometer**

## Modules Completed
Each module below contains commented source files and related Makefiles.

| Module | Description |
|--------|-------------|
| [`00: First Programs`](./module_00/) | Project setup with a custom Makefile. Writing first programs to blink an LED and handle button inputs using GPIOs. |
| [`01: Timers`](./module_01/)         | Introduction to hardware timers: configuring Timer0/1 for delays, counters, and generating periodic events. |
| [`02: UART`](./module_02/)           | Serial communication using the UART protocol: configuring baud rate, sending/receiving data over USB.       |
| [`03: RGB & advanced timers`](./module_03/) | Advanced timer usage with PWM to control RGB LED brightness and blending colors.                     |
| [`04: Interruptions`](./module_04/)  | Handling asynchronous events using external and timer-based interrupts (INT0/INT1, overflow).               |
| [`05: Analog`](./module_05/) | Reading analog input from a potentiometer using the ADC subsystem with AVCC as reference voltage.                   |
| [`06: I2C Protocol`](./module_06/)   | I2C master implementation to communicate with an AHT20 sensor and read temperature & humidity data.         |
| [`07: EEPROM`](./module_07/)         | Writing to and reading from the internal EEPROM; displaying stored values via UART.                         |
| [`08: SPI`](./module_08/)            | SPI master communication to interface with components like potentiometers, buttons, and LEDs.               |
| [`09: 7 segments display`](./module_09/)  | Driving a 7-segment display using GPIOs and timers to show numerical values (e.g. counter or sensor data).  |

> All modules are written in pure C, using direct register access (no Arduino libraries).


## Hardware
You can find the development board schematic used during the piscine here:
→ [`devkit_schema.pdf`](./devkit_schema.pdf)


## Build and Flash
Each module contains a Makefile :

```bash
cd module_00/ex00
make
