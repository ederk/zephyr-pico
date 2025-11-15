/*
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

/* Converte HSV para RGB */
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

/* Thread LED */
#ifdef CONFIG_LED_STRIP
void *led_thread(void *arg)
{
    (void)arg;

    const struct device *strip = DEVICE_DT_GET(DT_NODELABEL(ws2812));
    if (!device_is_ready(strip)) {
        printk("LED strip not ready - thread LED não iniciada\n");
        return NULL;
    }

    printk("Thread LED: arco-íris iniciado\n");

    struct led_rgb pixel[16];
    uint16_t hue = 0;

    while (1) {

        for (int i = 0; i < 16; i++) {
            hue = (hue + 1) % 360;
            hsv_to_rgb(hue, 80, 4, &pixel[i]);
        }
        led_strip_update_rgb(strip, pixel, 16);

        usleep(50000);
    }

    return NULL;
}
#endif

/* Thread de tarefa genérica (comum a todas boards) */
static void *task_thread(void *arg)
{
    (void)arg;

    printk("Thread Task: iniciando\n");
    uint32_t counter = 0;

    while (1) {
        printk("Task [%s]: contador = %u\n", CONFIG_BOARD, counter++);
        sleep(2);
    }

    return NULL;
}

int main(void)
{
    int ret;
    struct sched_param param;

    k_sleep(K_MSEC(5000));

    printk("===================================\n");
    printk("Hello World! Board: %s\n", CONFIG_BOARD);
    printk("===================================\n");

    pthread_t task_tid;
    pthread_t led_tid;
    pthread_attr_t task_attr;
#ifdef CONFIG_LED_STRIP
    pthread_attr_t led_attr;
#endif

    // Task: atributos sem stack explícita
    ret = pthread_attr_init(&task_attr);
    if (ret) { printk("ERRO pthread_attr_init: %d\n", ret); return -1; }
    pthread_attr_setstacksize(&task_attr, 4096);
    pthread_attr_setschedpolicy(&task_attr, SCHED_FIFO);
    param.sched_priority = 3;
    pthread_attr_setschedparam(&task_attr, &param);
    ret = pthread_create(&task_tid, &task_attr, task_thread, NULL);
    if (ret) { printk("ERRO pthread_create Task: %d\n", ret); return -1; }

#ifdef CONFIG_LED_STRIP
    // LED: atributos sem stack explícita
    ret = pthread_attr_init(&led_attr);
    if (ret) { printk("ERRO pthread_attr_init led: %d\n", ret); return -1; }
    pthread_attr_setstacksize(&led_attr, 2048);
    pthread_attr_setschedpolicy(&led_attr, SCHED_FIFO);
    param.sched_priority = 5;
    pthread_attr_setschedparam(&led_attr, &param);
    ret = pthread_create(&led_tid, &led_attr, led_thread, NULL);
    if (ret) { printk("ERRO pthread_create LED: %d\n", ret); return -1; }
#endif

    printk("Threads POSIX iniciadas com sucesso\n");

    while (1) {
        printk("Main thread alive\n");
        k_msleep(5000);
    }

    return 0;
}