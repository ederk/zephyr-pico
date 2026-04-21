/*
 * Copyright (c) 2026
 * Author: Eder Knoner
 * Email: eder.knoner@ostermayer.com.br
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************************************
 * Board-specific configuration
 *****************************************************************************************************/

#if defined(CONFIG_BOARD_RP2040_ZERO)
    /** Number of WS2812 pixels on target board. */
    #define LED_STRIP_PIXELS 1
#elif defined(CONFIG_BOARD_RPI_PICO2_RP2350A_M33_W)
    /** Number of WS2812 pixels on target board. */
    #define LED_STRIP_PIXELS 16
#else
    #define LED_STRIP_PIXELS 0
#endif

/******************************************************************************************************
 * Public API
 *****************************************************************************************************/

/**
 * @brief Initialise the LED strip service.
 *
 * Binds to the `ws2812` device-tree node and verifies the device
 * is ready. Must be called once before any other LED_* function.
 *
 * @return 0 on success, -ENODEV if the strip device is not ready.
 */
int LED_Init(void);

/**
 * @brief Set all pixels to a single HSV colour and push to the strip.
 *
 * @param hue        Hue in degrees [0, 359].
 * @param saturation Saturation percentage [0, 100].
 * @param value      Brightness percentage [0, 100].
 * @return 0 on success, negative errno on driver error.
 */
int LED_SetHSV(uint16_t hue, uint8_t saturation, uint8_t value);

/**
 * @brief Advance the rainbow animation by one step and push to the strip.
 *
 * Each call increments the internal hue counter.
 *
 * @return 0 on success, negative errno on driver error.
 */
int LED_RainbowTick(void);

/**
 * @brief POSIX thread entry that runs the continuous rainbow animation.
 *
 * Intended to be registered as a `TaskSpec::entry` in the application
 * task table. Loops forever; does not return.
 *
 * @param arg Unused.
 * @return NULL (never reached under normal operation).
 */
void *LED_Thread(void *arg);

#ifdef __cplusplus
}
#endif
