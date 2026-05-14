# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

PlatformIO firmware for a vacuum/suction-cup actuator that supports surfboard blanks under a CNC shaping machine. An array of these actuators sits below the blank; each one independently drives its arm up until an FSR (force-sensitive resistor) "touch sensor" reads a calibrated preload pressure, then deploys a suction cup and applies vacuum to grip the blank from below. The goal is a compliant Z-axis support that cancels the cutter head's downward load without crushing the foam (PU or EPS) — the FSR threshold pair acts as a **force window**, not a position target.

Target hardware is a SparkFun Pro Micro (ATmega32U4, AVR — 32 K flash, 2.5 K RAM, 1 K EEPROM). `Hardware/Actuators/` contains the KiCad PCB and schematic (`ActuatorsExpanded.*`) for the custom driver board. Firmware is the only thing built from this repo.

## Build / flash / monitor

Everything goes through PlatformIO — there is no Makefile or alternate build system.

```bash
pio run                         # compile for env:sparkfun_promicro16
pio run -t upload               # flash the Pro Micro
pio device monitor              # serial monitor at 115200 baud (set in platformio.ini)
pio run -t clean
```

VS Code's default build task (`.vscode/tasks.json`) wraps `pio run`.

There are **no unit tests**. The `test/`, `lib/`, and `include/` directories contain only the stock PlatformIO READMEs — do not assume CI runs `pio test`. The committed `.travis.yml` is an unmodified template (all lines commented) and `azure-pipelines.yml` is a placeholder "hello world". Treat the build itself as the only check.

### Offline compile-check (when `pio` can't reach the network)

If PlatformIO can't fetch the `atmelavr` platform package (sandbox / offline), you can still syntax-check changes against the Arduino-AVR core that ships in `arduino-core-avr` (Ubuntu/Debian package) using `avr-g++` directly. The Leonardo variant is binary-compatible with the Pro Micro for compile purposes (same ATmega32U4, same 16 MHz clock). This won't produce a flashable image but catches every header/typo/API mistake. See the project history for the helper script.

### Debug vs. log build modes

`platformio.ini` toggles two **mutually exclusive** preprocessor flags:

- `-DDEBUG_MODE="1"` (default, currently uncommented) — verbose human-readable state-transition prints.
- `-DLOG_MODE="1"` — tabular CSV-ish log of cycle counts, tuning iterations, and resting pressure %, intended for capturing calibration runs.

Both flags also gate `ShouldTransitionOnPress` in `ActuatorStateMachine.cpp`: in debug/log builds a single press of `start` force-advances the state machine through stages that would otherwise wait on sensor conditions. A release build (neither flag) refuses those manual transitions — keep this in mind before "fixing" the `#ifdef`s. (The `#else` branch suppresses the unused-parameter warning with `(void)state;`.)

## Architecture

`src/main.cpp` is thin: it instantiates one `ActuatorStateMachine` at file scope and calls `.Process()` every 20 ms in `loop()`. Almost all behavior lives inside that state machine.

### State machine (`src/StateMachines/ActuatorStateMachine.*`)

The `ActuatorState` enum (scoped, `enum class : uint8_t`) defines the operational cycle:

```
STOPPED → DRIVING_UP → BOARD_SENSED → (TUNING_DOWN ⇄ TUNING_UP) → AT_BOARD
       → RAISING_CUP → APPLYING_VACUUM → IN_POSITION → DRIVING_DOWN → STOPPED
```

`ERROR` exists in the enum but is currently unreachable — see "Known bugs".

`Process()` is a single `switch` that, per state, performs an action and then evaluates transition conditions. The state machine **owns** all peripheral objects (motor controller, both solenoids, pressure switch, touch sensor, both pushbuttons) by value and constructs them via a member-initialiser list using `static constexpr` pin/window constants declared on the class. There is no DI; peripherals are non-copyable (`= delete` on copy ctor/assignment) so pin ownership can't accidentally fork.

`ResetActions()` is the "safe state" helper. `ResetStateTimersOnEntry()` zeroes the per-state tick counters (`atBoardTicks`, `raisingCupTicks`, `applyingVacuumTicks`) whenever a transition was just taken — those counters are ad-hoc timeouts used in the latter half of the cycle (`AT_BOARD → RAISING_CUP`, etc.) because the suction-cup position switch and vacuum confirmation paths aren't wired up. **If you reinstate the sensor-based transitions, remove the timer fallbacks too.**

### Peripheral module convention

Every peripheral lives in its own `src/<Peripheral>/` directory and follows the same trio:

- `<Thing>Command.h` — `enum class <Thing>Command : uint8_t` of inputs.
- `<Thing>State.h` — `enum class <Thing>State : uint8_t` of outputs.
- `<Thing>.h`/`.cpp` — class that exposes `Command(…)` and/or `GetState()`. Copy ctor/assignment are deleted; non-mutating accessors are `const`.

When adding a new peripheral, mirror this layout. State machines should only ever read state enums and write command enums — they should not poke pins directly.

Because all enums are scoped, the `ActuatorState::STOPPED` / `CalibrationState::STOPPED` collision (and several others between `CalibrationState` and `MotorCommand`) is harmless. Fully-qualified names are required at every use site — don't add `using enum` shortcuts; the explicit qualification is part of the convention.

### Persistent calibration (`src/Data/SavedData.*`)

Sensor thresholds are stored in EEPROM at fixed byte addresses (`0` = lower, `1` = upper). `SavedData` is a static-only class; getters auto-repair corrupt values by writing hard-coded defaults (`10` and `20`). `ActuatorStateMachine`'s constructor reads these to seed the `TouchSensor` thresholds — so any "magic numbers" you see for FSR thresholds in `Calibrate.h` defaults (15/25) are independent of the live values.

`Calibrate` (`src/Calibrate/`) is the auto-tuning routine that's meant to populate EEPROM. Its `.cpp` is **entirely commented out** — the class header is wired up (it expects peripherals injected by reference) but the implementation needs rewriting before it can be called. Nothing currently instantiates `Calibrate`.

### Pin map

Pin assignments are declared as `static constexpr uint8_t` members of `ActuatorStateMachine` (e.g. `kStartPushbuttonPin`, `kVacuumSolenoidPin`). The human-readable summary is the comment block at the top of `src/main.cpp`. **The `MotorController` default constructor still hard-codes pins 2–6 inline** — don't change those without also updating the class constants. Pro Micro silkscreen pin numbers are used throughout (e.g. `8`, `9`, `10`, `14` for solenoids/sensors, `A0`–`A2` for analog).

## Known bugs / out-of-scope items

These bugs are flagged here rather than fixed because their fixes change observable hardware behaviour (motor pin init timing, ADC sensitivity, fault handling) and need bench verification on a real shaper.

1. **`MotorController()` default constructor delegation is broken.** `src/Motor/Motor.cpp` calls `MotorController(2, 3, 4, 5, 6)` as a plain expression inside the default-ctor body, which constructs and discards a temporary instead of delegating. The actual object's `_fwPin`/`_bwPin`/etc. members are left uninitialised and `pinMode` is never called for the motor pins. Fix is C++11 delegation syntax: `MotorController::MotorController() : MotorController(2, 3, 4, 5, 6) {}`. Applying this will start firing `pinMode(OUTPUT)` on those pins for the first time — test the motor's idle behaviour before flashing.

2. **`FsrPushbutton::PollPresses()` truncates `analogRead()` to `uint8_t`.** The 10-bit ADC result (0–1023) is silently narrowed by assignment to a `uint8_t analogValue`. Any pressure thresholds currently tuned were tuned against the truncated value — fixing the truncation invalidates the existing calibration.

3. **`MOTOR_FAULT` software state is never set; `MotorController::HasFault()` is never called.** `_state` is software-tracked and only ever assigned `MOTOR_DRIVING_UP/DOWN`/`MOTOR_STOPPED`. The `STOPPED` guard reads `GetMotorState() != MotorState::MOTOR_FAULT` but that branch is unreachable. `HasFault()` reads the actual fault pin via `digitalRead(_faultPin)` and exists, just isn't wired into the state machine. Closing this loop also requires re-enabling the `ERROR` state in the `switch`.

Other deferred work (out of scope for the current refactor):

- `FsrPushbutton::valueArray` is 200 bytes per instance; the press-detection window only reads indices 0, 1, and a 5-element prefix. A ring buffer of ~16 entries would save ~360 bytes of SRAM and reduce the per-tick shift cost.
- `TouchSensor::GetState()` calls `GetFsrPct()` (and thus `analogRead`) twice per tick when both thresholds are checked in sequence. One sample per tick stored on the instance would be more honest and faster.
- The `Calibrate` class needs reimplementing on top of the new peripheral-reference constructor and `enum class CalibrationState`.

## Conventions

- C++11 with Arduino framework idioms (`Serial.print`, `pinMode`, etc.). Stick to AVR-friendly types (`uint8_t`, `int`, `long`) — no STL containers, no dynamic allocation. `String` from the Arduino core is **not** used; serial literals are wrapped in `F()` so they live in flash.
- All enums are `enum class : uint8_t` and must be referenced with their type prefix (e.g. `MotorCommand::DRIVE_UP_FAST`).
- Headers use `#ifndef`/`#define` include guards, not `#pragma once`. **The guard token must match the `#ifndef` exactly** — both `SolenoidState.h` and `SolenoidCommand.h` previously had typo'd guards that defeated double-include protection.
- Peripheral classes delete copy ctor/assignment (`= delete`) and mark non-mutating accessors `const`. Mirror this when adding new peripherals.
- Includes between sibling modules use **relative paths** (`#include "../Motor/Motor.h"`), not project-root paths. PlatformIO is not configured with extra include roots beyond `include/` (which is empty).
- Magic numbers belong in `static constexpr` class members or namespace-scope constants alongside their owner — see `ActuatorStateMachine`'s pin and tick-count constants.
- Indentation is tabs.
