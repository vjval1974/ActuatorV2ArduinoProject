# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

PlatformIO firmware for a vacuum/suction-cup actuator built on the SparkFun Pro Micro (ATmega32U4, AVR). The microcontroller drives a linear actuator motor, a vacuum solenoid, and a suction-cup solenoid; it senses board contact via an FSR (force-sensitive resistor) "touch sensor" and confirms grip via a vacuum pressure switch. Two FSR-based pushbuttons (start/stop on `A0`/`A1`) drive the user interaction.

`Hardware/Actuators/` contains the KiCad PCB and schematic (`ActuatorsExpanded.*`) for the custom driver board. Firmware is the only thing built from this repo.

## Build / flash / monitor

Everything goes through PlatformIO — there is no Makefile or alternate build system.

```bash
pio run                         # compile for env:sparkfun_promicro16
pio run -t upload               # flash the Pro Micro
pio device monitor              # serial monitor at 115200 baud (set in platformio.ini)
pio run -t clean
```

VS Code's default build task (`.vscode/tasks.json`) wraps `pio run`.

There are **no unit tests**. The `test/`, `lib/`, and `include/` directories contain only the stock PlatformIO READMEs — do not assume CI runs `pio test`. The committed `.travis.yml` is an unmodified template (all lines commented), and `azure-pipelines.yml` is a placeholder "hello world". Treat the build itself as the only check.

### Debug vs. log build modes

`platformio.ini` toggles two **mutually exclusive** preprocessor flags:

- `-DDEBUG_MODE="1"` (default, currently uncommented) — verbose human-readable state-transition prints.
- `-DLOG_MODE="1"` — tabular CSV-ish log of cycle counts, tuning iterations, and resting pressure %, intended for capturing calibration runs.

Both flags also gate `ShouldTransitionOnPress` in `ActuatorStateMachine.cpp`: in debug/log builds a single press of `start` force-advances the state machine through stages that would otherwise wait on sensor conditions. A release build (neither flag) would refuse those manual transitions — keep this in mind before "fixing" the `#ifdef`s.

## Architecture

`src/main.cpp` is thin: it instantiates one `ActuatorStateMachine` and calls `.Process()` every 20 ms in `loop()`. Almost all behavior lives inside that state machine.

### State machine (`src/StateMachines/ActuatorStateMachine.*`)

The `ActuatorState` enum defines the operational cycle:

```
STOPPED → DRIVING_UP → BOARD_SENSED → (TUNING_DOWN ⇄ TUNING_UP) → AT_BOARD
       → RAISING_CUP → APPLYING_VACUUM → IN_POSITION → DRIVING_DOWN → STOPPED
```

`Process()` is a single `switch` that, per state, performs an action and then evaluates transition conditions. The state machine **owns** all peripheral objects (motor controller, both solenoids, pressure switch, touch sensor, both pushbuttons) by value — there is no DI or global registry. `ResetActions()` is the only "safe state" helper.

Several transitions in the latter half of the cycle (`AT_BOARD` → `RAISING_CUP`, `RAISING_CUP` → `APPLYING_VACUUM`, etc.) use a shared `static long cnt` counter as an ad-hoc timeout because the suction-cup position switch and vacuum-confirmation paths are commented out. If you reinstate the sensor-based transitions, remove the counter fallbacks too — they were added because real position feedback isn't wired up yet.

### Peripheral module convention

Every peripheral lives in its own `src/<Peripheral>/` directory and follows the same trio of headers:

- `<Thing>Command.h` — enum of inputs (`MotorCommand`, `SolenoidCommand`, …).
- `<Thing>State.h` — enum of outputs (`MotorState`, `SolenoidState`, `FsrState`, `PressState`, `SuctionCupPosition`, `PressureSwitchState`).
- `<Thing>.h`/`.cpp` — class that exposes `Command(…)` and/or `GetState()`.

When adding a new peripheral, mirror this layout. State machines should only ever read state enums and write command enums — they should not poke pins directly.

Note the typo `DEACTICTIVATED` in `SolenoidState` (and the `SOLENIODSTATE_H`/`SOLENIODCOMMAND_H` include-guard typos) is load-bearing — it's referenced from `ActuatorStateMachine.cpp`. Fix it everywhere at once or not at all.

### Persistent calibration (`src/Data/SavedData.*`)

Sensor thresholds are stored in EEPROM at fixed byte addresses (`0` = lower, `1` = upper). `SavedData` is a static-only class; getters auto-repair corrupt values by writing hard-coded defaults (`10` and `20`). `ActuatorStateMachine`'s constructor reads these to seed the `TouchSensor` thresholds — so any "magic numbers" you see for FSR thresholds in `Calibrate.h` defaults (15/25) are independent of the live values.

`Calibrate` (`src/Calibrate/`) is the auto-tuning routine that's meant to populate EEPROM, but note its `CalibrationState::STOPPED` collides with `ActuatorState::STOPPED` — both enums are unscoped. Don't `#include` both headers in the same translation unit without renaming.

### Pin map

The authoritative pin assignments are the comment block at the top of `src/main.cpp`. Pro Micro silkscreen pin numbers are used throughout (e.g. `8`, `9`, `10`, `14` for solenoids/sensors, `A0`–`A3` for analog). The `MotorController()` default constructor and the `ActuatorStateMachine` constructor both hard-code these — if you change a wire, update both.

## Conventions

- C++ with Arduino framework idioms (`Serial.print`, `pinMode`, etc.). Stick to AVR-friendly types (`uint8_t`, `int`, `long`) — no STL containers, no dynamic allocation. `String` from the Arduino core is used sparingly (e.g. `statesText[]`); prefer `const char*` for new code.
- Headers use `#ifndef`/`#define` include guards, not `#pragma once`.
- Includes between sibling modules use **relative paths** (`#include "../Motor/Motor.h"`), not project-root paths. Keep that convention; PlatformIO is not configured with extra include roots beyond `include/` (which is empty).
- Indentation is tabs in most files, but a few files mix tabs and spaces — match the file you're editing rather than reformatting.
