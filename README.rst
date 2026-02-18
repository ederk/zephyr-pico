Zephyr Pico
===========

Generic Zephyr RTOS project for Raspberry Pi Pico and other microcontrollers with WS2812 LED strip support.

Overview
********

This project demonstrates a **generic and reusable** Zephyr foundation for controlling **WS2812 LED strips** 
across different boards/microcontrollers. The architecture keeps the same application code while only changing 
board configuration, DeviceTree, and ``prj.conf``.

Features
********

* Reusable application logic across different hardware targets
* Hardware-specific details isolated in board files
* Support for multiple microcontroller families
* Easy expansion to new targets with minimal code changes

Supported Boards
****************

* RP2040 Zero (``rp2040_zero``)
* Raspberry Pi Pico 2 (``rpi_pico2/rp2350a``)

The same pattern can be used for STM32, NXP, ESP32, and other Zephyr-supported targets.

Project Structure
*****************

::

    zephyr-pico/
    ├── LICENSE              # Apache-2.0 license
    ├── README.rst           # This file
    ├── application/         # Main application directory
    │   ├── src/            # Generic application logic
    │   ├── inc/            # Header files
    │   ├── boards/         # Board-specific configurations
    │   ├── prj.conf        # Common project configuration
    │   └── CMakeLists.txt  # Build configuration
    └── west-manifest/       # West manifest for Zephyr dependencies

Getting Started
***************

Prerequisites
=============

1. Install Zephyr development environment following the `official Zephyr documentation`_
2. Set up West workspace
3. Install required dependencies for your target board

.. _official Zephyr documentation: https://docs.zephyrproject.org/latest/develop/getting_started/index.html

Building
========

To build for a specific board:

.. code-block:: bash

    cd application
    west build -b <board_name>

For example, to build for Raspberry Pi Pico 2:

.. code-block:: bash

    west build -b rpi_pico2/rp2350a

Flashing
========

Flash the built firmware to your board:

.. code-block:: bash

    west flash

Architecture
************

Application Layer (Generic)
============================

* ``src/main.cpp``: Main logic, task definitions, and initialization
* ``src/tasks.cpp`` + ``inc/tasks.h``: Generic POSIX thread creation module

This layer **must not** depend on board-specific pins or peripherals.

Board Hardware Layer
====================

* ``boards/<board>.overlay``: Defines DeviceTree nodes (ws2812, GPIO, PIO, etc.)
* ``boards/<board>.conf``: Board-specific Kconfig adjustments

All microcontroller differences should stay in this layer.

Adding a New Board
******************

1. Create ``boards/<new_board>.overlay`` with the LED strip definition
2. Create ``boards/<new_board>.conf`` with required Kconfig symbols
3. Ensure the ``ws2812`` node is available in the new board DeviceTree
4. Validate LED strip timing/frequency parameters for that hardware
5. Build using the corresponding board target

How to Make This Repository Public
***********************************

To make this GitHub repository publicly accessible:

GitHub Web Interface
====================

1. Navigate to your repository on GitHub (https://github.com/ederk/zephyr-pico)
2. Click on **Settings** (top menu bar)
3. Scroll down to the **Danger Zone** section
4. Click **Change visibility**
5. Select **Make public**
6. Confirm by typing the repository name
7. Click **I understand, change repository visibility**

Using GitHub CLI
================

If you have the GitHub CLI (``gh``) installed:

.. code-block:: bash

    gh repo edit ederk/zephyr-pico --visibility public

Considerations Before Making Public
====================================

* ✅ LICENSE file is present (Apache-2.0)
* ✅ README documentation is complete
* ⚠️ Review all code for sensitive information (API keys, passwords, etc.)
* ⚠️ Ensure all code follows best practices and is ready for public viewing
* ⚠️ Consider adding a CONTRIBUTING.md file with contribution guidelines
* ⚠️ Add appropriate .gitignore entries for build artifacts and dependencies

License
*******

This project is licensed under the Apache License 2.0. See the LICENSE file for details.

Contributing
************

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

Resources
*********

* `Zephyr Project Documentation`_
* `Zephyr Board Support`_
* `WS2812 LED Driver`_

.. _Zephyr Project Documentation: https://docs.zephyrproject.org/
.. _Zephyr Board Support: https://docs.zephyrproject.org/latest/boards/index.html
.. _WS2812 LED Driver: https://docs.zephyrproject.org/latest/hardware/peripherals/led_strip.html
