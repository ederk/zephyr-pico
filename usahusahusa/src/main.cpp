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

/* Defina stacks estáticos alinhados */
K_THREAD_STACK_DEFINE(task_stack, 4096);
#ifdef CONFIG_LED_STRIP
K_THREAD_STACK_DEFINE(led_stack, 2048);
#endif

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

    struct led_rgb pixel;
    uint16_t hue = 0;

    while (1) {
        hsv_to_rgb(hue, 100, 30, &pixel);
        led_strip_update_rgb(strip, &pixel, 1);
        hue = (hue + 1) % 360;
        usleep(10000);
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
    int prio;
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

    /* Task thread attrs + stack (note: stackaddr first, then size) */
    ret = pthread_attr_init(&task_attr);
    if (ret != 0) {
        printk("ERRO pthread_attr_init: %d\n", ret);
        return -1;
    }

    ret = pthread_attr_setstack(&task_attr,
                                (void *)task_stack,
                                K_THREAD_STACK_SIZEOF(task_stack)); /* CORRETO: addr, size */
    if (ret != 0) {
        printk("ERRO pthread_attr_setstack task: %d\n", ret);
        return -1;
    }

    ret = pthread_attr_setschedpolicy(&task_attr, SCHED_FIFO);
    if (ret != 0) {
        printk("ERRO pthread_attr_setschedpolicy task: %d\n", ret);
        return -1;
    }

    param.sched_priority = 3;
    ret = pthread_attr_setschedparam(&task_attr, &param);
    if (ret != 0) {
        printk("ERRO pthread_attr_setschedparam task: %d\n", ret);
        return -1;
    }

    ret = pthread_create(&task_tid, &task_attr, task_thread, NULL);
    if (ret != 0) {
        printk("ERRO pthread_create Task: %d\n", ret);
        return -1;
    }

#ifdef CONFIG_LED_STRIP
    /* LED thread attrs + stack */
    ret = pthread_attr_init(&led_attr);
    if (ret != 0) {
        printk("ERRO pthread_attr_init led: %d\n", ret);
        return -1;
    }

    ret = pthread_attr_setstack(&led_attr,
                                (void *)led_stack,
                                K_THREAD_STACK_SIZEOF(led_stack)); /* CORRETO: addr, size */
    if (ret != 0) {
        printk("ERRO pthread_attr_setstack led: %d\n", ret);
        return -1;
    }

    ret = pthread_attr_setschedpolicy(&led_attr, SCHED_FIFO);
    if (ret != 0) {
        printk("ERRO pthread_attr_setschedpolicy led: %d\n", ret);
        return -1;
    }

    param.sched_priority = 5;
    ret = pthread_attr_setschedparam(&led_attr, &param);
    if (ret != 0) {
        printk("ERRO pthread_attr_setschedparam led: %d\n", ret);
        return -1;
    }

    ret = pthread_create(&led_tid, &led_attr, led_thread, NULL);
    if (ret != 0) {
        printk("ERRO pthread_create LED: %d\n", ret);
        return -1;
    }

    printk("Thread LED criada (verifica se dispositivo está pronto)\n");
#else
    printk("Board sem LED strip configurado - thread LED não criada\n");
#endif

    printk("Threads POSIX iniciadas com sucesso\n");

    while (1) {
        printk("Main thread alive\n");
        k_msleep(5000);
    }

    return 0;
}