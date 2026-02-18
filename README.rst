Zephyr Embedded Project Code Base
#################################

Overview
********

This repository is a **Zephyr-based firmware code base** for embedded processing
units (microcontrollers), with focus on real hardware targets and LED strip
control.

The project is structured to keep application logic generic while isolating
board-specific configuration in dedicated files.

Main Characteristics
********************

- C/C++ application on Zephyr RTOS
- POSIX-thread based task orchestration
- WS2812 LED strip support via DeviceTree
- Multi-board organization (for example RP2040 family targets)

Project Layout
**************

- ``application/src``: main application sources
- ``application/inc``: public headers
- ``application/boards``: board-specific ``.conf`` and ``.overlay`` files
- ``application/prj.conf``: common Zephyr configuration

Build (Real Board)
******************

Example build for RP2040 Zero:

.. code-block:: console

   west build application -b rp2040_zero --build-dir application/build/rp2040_zero

Example build for Raspberry Pi Pico 2:

.. code-block:: console

   west build application -b rpi_pico2/rp2350a --build-dir application/build/rpi_pico2

Flash (UF2 runner)
******************

.. code-block:: console

   west flash --build-dir application/build/rp2040_zero -r uf2

Notes
*****

- This documentation intentionally removes QEMU-focused instructions.
- The intended workflow is build/flash on physical MCU targets.
