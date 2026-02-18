# Generic Zephyr project for multiple microcontrollers with LED Strip

This project demonstrates a **generic and reusable** Zephyr foundation for controlling **WS2812 LED strips** across different boards/microcontrollers, keeping the same application code and changing only board configuration, DeviceTree, and `prj.conf`.

## Goal

- Reuse the same application logic on different hardware targets.
- Isolate hardware details in board files (`boards/*.overlay`, `boards/*.conf`).
- Enable easy expansion to new microcontroller families with minimal code impact.

## Proposed architecture

### 1) Application layer (generic)

- `src/main.cpp`: main logic, task definitions, and initialization.
- `src/tasks.cpp` + `inc/tasks.h`: generic POSIX thread creation module, receiving tasks externally.

This layer **must not** depend on board-specific pins or peripherals.

### 2) Board hardware layer

- `boards/<board>.overlay`: defines DeviceTree nodes (for example: `ws2812`, GPIO, PIO, etc.).
- `boards/<board>.conf`: board-specific Kconfig adjustments (drivers, stack sizes, logs, kernel options).

All microcontroller differences should stay in this layer.

## Recommended structure

- `CMakeLists.txt`: adds application sources.
- `prj.conf`: common base options.
- `boards/`: per-board customizations.
- `src/` and `inc/`: portable application logic.

## Example supported boards

- RP2040 Zero (`rp2040_zero`)
- Raspberry Pi Pico 2 (`rpi_pico2/rp2350a` in the build directory)

> The same pattern can be used for STM32, NXP, ESP32, and other Zephyr-supported targets.

## How to adapt to another microcontroller

1. Create `boards/<new_board>.overlay` with the LED strip definition.
2. Create `boards/<new_board>.conf` with the required Kconfig symbols.
3. Ensure the `ws2812` node is available in the new board DeviceTree.
4. Validate LED strip timing/frequency parameters for that hardware.
5. Build using the corresponding board target.

## Task initialization contract

The tasks module receives an array of `TaskSpec` defined by the application:

- `name`: diagnostic name
- `entry`: thread entry function
- `stack`: stack size
- `prio`: priority
- `policy`: policy (`SCHED_FIFO`, etc.)
- `enabled`: enables/disables creation

API:

- `TASKS_Init(const TaskSpec* specs, int spec_count)`

## Best practices for a generic project

- Avoid board-specific `#ifdef` in business logic whenever possible.
- Prefer DeviceTree + board Kconfig for hardware differences.
- Keep consistent node names (`ws2812`) across overlays.
- Handle `device_is_ready()` failures and thread creation errors.
- Document limits (for example: maximum internal task count).

## Scalability

With this organization, adding a new microcontroller mainly requires changes to:

- `boards/` files
- build parameters

without rewriting the core application logic.

## License

Apache-2.0 (following Zephyr project conventions).
