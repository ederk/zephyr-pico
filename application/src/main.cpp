/*
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

 /******************************************************************************************************
 * Include Section
 *****************************************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <tasks.h>


/******************************************************************************************************
 * LED Task Section
 *****************************************************************************************************/

/** Number of LED strip pixels. */
 #ifdef CONFIG_LED_STRIP

#if defined(CONFIG_BOARD_RP2040_ZERO)
    #define LED_STRIP_PIXELS 1
#elif defined(CONFIG_BOARD_RPI_PICO2_RP2350A_M33_W)
    #define LED_STRIP_PIXELS 16
#else
    #define LED_STRIP_PIXELS 0
#endif

/* Convert HSV to RGB */
static void hsv_to_rgb(uint16_t hue, uint8_t saturation, uint8_t value, struct led_rgb *rgb)
{
    uint8_t region, remainder, p, q, t;

    if (saturation == 0) {
        rgb->r = rgb->g = rgb->b = value * 255 / 100;
        return;
    }

    region = hue / 60;
    remainder = (hue % 60) * 255 / 60;

    p = (value * (100 - saturation)) / 100;
    q = (value * (100 - (saturation * remainder) / 255)) / 100;
    t = (value * (100 - (saturation * (255 - remainder)) / 255)) / 100;

    value = value * 255 / 100;
    p = p * 255 / 100;
    q = q * 255 / 100;
    t = t * 255 / 100;

    switch (region) {
    case 0: rgb->r = value; rgb->g = t; rgb->b = p; break;
    case 1: rgb->r = q; rgb->g = value; rgb->b = p; break;
    case 2: rgb->r = p; rgb->g = value; rgb->b = t; break;
    case 3: rgb->r = p; rgb->g = q; rgb->b = value; break;
    case 4: rgb->r = t; rgb->g = p; rgb->b = value; break;
    default: rgb->r = value; rgb->g = p; rgb->b = q; break;
    }
}


static void *led_thread(void *arg)
{
    (void)arg;

    const struct device *strip = DEVICE_DT_GET(DT_NODELABEL(ws2812));
    if (!device_is_ready(strip)) {
        printk("LED strip not ready — LED thread not started\n");
        return NULL;
    }

    printk("LED thread: rainbow started\n");
    
    struct led_rgb pixel[LED_STRIP_PIXELS];
    uint32_t hue = 0;

    while (1) 
    {

        for (int i = 0; i < LED_STRIP_PIXELS; i++) {
            hue = (hue + 1) % 360;
            hsv_to_rgb(hue, 80, 4, &pixel[i]);
        }
        led_strip_update_rgb(strip, pixel, LED_STRIP_PIXELS);

        usleep(2000 * LED_STRIP_PIXELS);
    }

    return NULL;
}
#endif
 
/******************************************************************************************************
 * ADC Task Section
 *****************************************************************************************************/

#ifdef CONFIG_ADC

/** Sampling rate in Hz. */
#define ADC_FS_HZ       10000
/** Number of samples captured per async block. */
#define ADC_BLOCK_SIZE  256
/** Print stats every N completed blocks. */
#define ADC_LOG_BLOCKS  4

static const struct adc_dt_spec adc_ch0 =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

/* Ping-pong buffers — DMA fills one while the thread processes the other. */
static int16_t adc_buf[2][ADC_BLOCK_SIZE];

static void *adc_thread(void *arg)
{
    (void)arg;

    if (!adc_is_ready_dt(&adc_ch0)) {
        printk("ADC device not ready\n");
        return NULL;
    }

    int ret = adc_channel_setup_dt(&adc_ch0);
    if (ret < 0) {
        printk("ADC channel setup failed: %d\n", ret);
        return NULL;
    }

    printk("ADC thread started (ch%d, %d-bit)\n",
           adc_ch0.channel_id, adc_ch0.resolution);

    int16_t buf;
    struct adc_sequence seq = {
        .buffer      = &buf,
        .buffer_size = sizeof(buf),
    };
    adc_sequence_init_dt(&adc_ch0, &seq);

    while (1) {
        ret = adc_read_dt(&adc_ch0, &seq);
        if (ret < 0) {
            printk("ADC read error: %d\n", ret);
        } else {
            int32_t mv = buf;
            adc_raw_to_millivolts_dt(&adc_ch0, &mv);
            printk("ADC ch%d: raw=%d  %d mV\n",
                   adc_ch0.channel_id, buf, (int)mv);
        }
        usleep(500000); /* 500 ms */
    }

    return NULL;
}
#endif /* CONFIG_ADC */

/******************************************************************************************************
 * Application Section
 *****************************************************************************************************/

static const TaskSpec g_task_specs[] = {
#ifdef CONFIG_LED_STRIP
    { "led",  &led_thread,  5, SCHED_FIFO, true },
#endif
#ifdef CONFIG_ADC
    { "adc",  &adc_thread,  5, SCHED_FIFO, true },
#endif
};

int main(void)
{
    k_sleep(K_MSEC(1000));

    printk("===================================\n");
    printk("Hello World! Board: %s\n", CONFIG_BOARD);
    printk("===================================\n");

    const int tasks_ret = TASKS_Init(
        g_task_specs,
        static_cast<int>(sizeof(g_task_specs) / sizeof(g_task_specs[0])));

    if (tasks_ret != 0) 
    {
        printk("TASKS_Init falhou: %d\n", tasks_ret);
    }

    return 0;
}