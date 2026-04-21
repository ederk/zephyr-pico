/*
 * Copyright (c) 2026
 * Author: Eder Knoner
 * Email: eder.knoner@ostermayer.com.br
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************************************
 * Include Section
 *****************************************************************************************************/

#include <led_service.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#include <unistd.h>
#include <errno.h>

#ifdef CONFIG_LED_STRIP

/******************************************************************************************************
 * Private state
 *****************************************************************************************************/

static const struct device *s_strip;
static struct led_rgb        s_pixels[LED_STRIP_PIXELS];

/******************************************************************************************************
 * Private helpers
 *****************************************************************************************************/

/**
 * @brief Convert HSV to RGB.
 *
 * @param hue        Hue in degrees [0, 359].
 * @param saturation Saturation percentage [0, 100].
 * @param value      Brightness percentage [0, 100].
 * @param rgb        Output RGB pixel.
 */
static void hsv_to_rgb(uint16_t hue, uint8_t saturation, uint8_t value,
                       struct led_rgb *rgb)
{
    uint8_t region, remainder, p, q, t;

    if (saturation == 0U) {
        rgb->r = rgb->g = rgb->b = (uint8_t)(value * 255U / 100U);
        return;
    }

    region    = (uint8_t)(hue / 60U);
    remainder = (uint8_t)((hue % 60U) * 255U / 60U);

    p = (uint8_t)(value * (100U - saturation) / 100U);
    q = (uint8_t)(value * (100U - (saturation * remainder) / 255U) / 100U);
    t = (uint8_t)(value * (100U - (saturation * (255U - remainder)) / 255U) / 100U);

    value = (uint8_t)(value * 255U / 100U);
    p     = (uint8_t)(p     * 255U / 100U);
    q     = (uint8_t)(q     * 255U / 100U);
    t     = (uint8_t)(t     * 255U / 100U);

    switch (region) {
    case 0:  rgb->r = value; rgb->g = t;     rgb->b = p;     break;
    case 1:  rgb->r = q;     rgb->g = value; rgb->b = p;     break;
    case 2:  rgb->r = p;     rgb->g = value; rgb->b = t;     break;
    case 3:  rgb->r = p;     rgb->g = q;     rgb->b = value; break;
    case 4:  rgb->r = t;     rgb->g = p;     rgb->b = value; break;
    default: rgb->r = value; rgb->g = p;     rgb->b = q;     break;
    }
}

/******************************************************************************************************
 * Public API
 *****************************************************************************************************/

int LED_Init(void)
{
    s_strip = DEVICE_DT_GET(DT_NODELABEL(ws2812));

    if (!device_is_ready(s_strip)) {
        printk("LED service: strip device not ready\n");
        s_strip = NULL;
        return -ENODEV;
    }

    printk("LED service: initialised (%d pixel(s))\n", LED_STRIP_PIXELS);
    return 0;
}

int LED_SetHSV(uint16_t hue, uint8_t saturation, uint8_t value)
{
    if (s_strip == NULL) {
        return -ENODEV;
    }

    for (int i = 0; i < LED_STRIP_PIXELS; i++) {
        hsv_to_rgb(hue, saturation, value, &s_pixels[i]);
    }

    return led_strip_update_rgb(s_strip, s_pixels, LED_STRIP_PIXELS);
}

int LED_RainbowTick(void)
{
    static uint16_t s_hue = 0U;

    if (s_strip == NULL) {
        return -ENODEV;
    }

    for (int i = 0; i < LED_STRIP_PIXELS; i++) {
        s_hue = (s_hue + 1U) % 360U;
        hsv_to_rgb(s_hue, 80U, 4U, &s_pixels[i]);
    }

    return led_strip_update_rgb(s_strip, s_pixels, LED_STRIP_PIXELS);
}

void *LED_Thread(void *arg)
{
    (void)arg;

    if (LED_Init() != 0) {
        return NULL;
    }

    printk("LED thread: rainbow started\n");

    while (1) {
        (void)LED_RainbowTick();
        usleep(2000U * LED_STRIP_PIXELS);
    }

    return NULL;
}

#endif /* CONFIG_LED_STRIP */
