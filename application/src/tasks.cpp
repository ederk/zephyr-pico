/*
 * Copyright (c) 2026
 * Author: Eder Knoner
 * Email: eder.knoner@ostermayer.com.br
 * 
 * SPDX-License-Identifier: Apache-2.0
 */

/** Include of this module */
#include <tasks.h>

/** Zephyr includes */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/posix/pthread.h>

/** Standard includes */
#include <errno.h>

/**
 * @file tasks.cpp
 * @brief Task orchestration module.
 *
 * Responsibilities:
 * - Receive task definitions from application layer (`main.cpp`).
 * - Configure POSIX pthread attributes.
 * - Create enabled tasks and keep minimal runtime handles.
 */

static constexpr int TASKS_MAX = 8;

// ----------------- Estate/handles -----------------
static TaskHandle g_handles[TASKS_MAX];
static int  g_created = 0;
static bool g_started = false;

// ----------------- Creation Helper -----------------
/**
 * @brief Internal helper to create a single POSIX thread from a `TaskSpec`.
 */
static int create_thread(const TaskSpec& spec, pthread_t* tid)
{
    pthread_attr_t attr;
    struct sched_param param;
    int ret = pthread_attr_init(&attr);
    if (ret) return ret;

    pthread_attr_setstacksize(&attr, spec.stack);
    pthread_attr_setschedpolicy(&attr, spec.policy);
    param.sched_priority = spec.prio;
    pthread_attr_setschedparam(&attr, &param);

    ret = pthread_create(tid, &attr, spec.entry, nullptr);
    pthread_attr_destroy(&attr);
    return ret;
}


int TASKS_Init(const TaskSpec* specs, int spec_count)
{
    /* Idempotent behavior: calling again after success is a no-op. */
    if (g_started) 
    {
        return 0;
    }

    /* Validate arguments from application layer. */
    if ((specs == nullptr) || (spec_count <= 0)) 
    {
        return -EINVAL;
    }

    /* Bound check against fixed internal runtime storage. */
    if (spec_count > TASKS_MAX) 
    {
        return -ENOMEM;
    }

    g_created = 0;

    for (int i = 0; i < spec_count; ++i) 
    {
        if (!specs[i].enabled) 
        {
            continue;
        }

        const int ret = create_thread(specs[i], &g_handles[g_created].tid);
        if (ret != 0) 
        {
            printk("Fail to create thread '%s': %d\n", specs[i].name, ret);
            return -ret;
        }

        g_handles[g_created].name = specs[i].name;
        g_created++;
    }

    g_started = true;
    printk("TASKS_Init: %d thread(s) started\n", g_created);

    return 0;
}