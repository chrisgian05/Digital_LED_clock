# Digital LED Clock

A 6-digit 7-segment clock (HH:MM:SS) built around an ATmega328P, an
external RTC, and a constant-current LED driver — split across two
custom PCBs I designed in KiCad. This was mostly an excuse to actually
use everything I've learned so far about digital electronics and
embedded C in one project I could see and touch, instead of another
breadboard that gets torn down at the end of the semester.

## What it does

- Shows time as HH:MM:SS across 6 seven-segment digits
- Keeps time with a DS3231 RTC (accurate even when the board is
  powered off, thanks to a coin cell)
- 4 buttons: set hour, set minute, reset seconds, and toggle to show
  the DS3231's built-in temperature reading instead of the time
- Multiplexed display — only one digit is physically powered at a
  time, cycled fast enough (~166Hz) that it looks fully lit

## Hardware

Two boards, connected by a short ribbon cable:

- **`clock_board`** — the "brains." ATmega328P, DS3231 RTC, a 74HC595
  shift register + 6 transistors for digit selection, an ISP header
  for programming, and the 4 push buttons.
- **`led_board`** — the "face." A TLC5916 constant-current LED driver
  feeding three 2-digit common-anode 7-segment modules (6 digits
  total), plus the transistor-switched power lines coming in from the
  clock board.

Full schematics and PCB layouts are in [`hardware/`](hardware/), one
subfolder per board. Both were designed from scratch in KiCad — no
dev-board shields, no premade modules other than the RTC/driver ICs
themselves.

## Firmware

Bare-metal AVR C — no Arduino core, straight `avr-libc` and direct
register access. I wanted to actually deal with I2C, timers, and
interrupts myself instead of hiding behind `Wire.h` and `digitalWrite`.

- **`twi.c`** — hardware I2C driver for talking to the DS3231
- **`ds3231.c`** — RTC register access (BCD conversion, get/set time, temperature)
- **`display.c`** — 7-segment font table + the Timer1 interrupt that handles
  digit multiplexing (bit-banging both the 74HC595 and TLC5916 shift chains)
- **`buttons.c`** — debounced button reads with auto-repeat on hold
- **`main.c`** — main loop tying it all together

All of it lives in [`firmware/`](firmware/), buildable with either the
included `Makefile` or `platformio.ini`.

## Status

Schematics and PCBs are designed; firmware is written and reviewed but
**not yet validated on the assembled board** (still waiting on
boards/soldering as of writing this). A couple of polarity assumptions
(digit-select active-high/low, segment driver active-high/low) are
flagged with `#define`s in `firmware/src/config.h` specifically because
they're easier to confirm with a multimeter on the real board than to
guess from a schematic.
