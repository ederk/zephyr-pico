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
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>

/** Tasks includes */
#include <tasks.h>
#include <led_service.h>
#include <dsp/adc.h>
#include <dsp/dsp.h>

/** Standard POSIX API */
#include <unistd.h>


/******************************************************************************************************
 * Application Section
 *****************************************************************************************************/

static const TaskSpec g_task_specs[] = {
#ifdef CONFIG_LED_STRIP
    { "led",  &LED_Thread,  0, SCHED_FIFO, true },
#endif
#ifdef CONFIG_ADC
    { "adc",  &ADC_Thread,  5, SCHED_FIFO, true },
    { "dsp",  &DSP_Thread,  4, SCHED_FIFO, true, (void *)&adc_pipe },
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