/*
 * Copyright (c) 2026
 * Author: Eder Knoner
 * Email: eder.knoner@ostermayer.com.br
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/kernel.h>
#include <zephyr/posix/pthread.h>

/**
 * @brief Declarative task specification consumed by `TASKS_Init()`.
 *
 * The application (for example in `main.cpp`) owns the thread entry
 * functions and provides an array of this struct to the tasks module.
 */
struct TaskSpec {
    /** Task name used for diagnostics/logging. */
    const char* name;
    /** POSIX thread entry function. */
    void* (*entry)(void*);
    /** Scheduler priority (for example with `SCHED_FIFO`). */
    int prio;
    /** POSIX scheduler policy (for example `SCHED_FIFO`). */
    int policy;
    /** Enables/disables task creation for this entry. */
    bool enabled;
};

/**
 * @brief Runtime metadata of created tasks.
 */
struct TaskHandle {
    /** Task name copied from `TaskSpec::name`. */
    const char* name;
    /** POSIX thread identifier returned by `pthread_create()`. */
    pthread_t tid;
};


/**
 * @brief Creates and starts all enabled tasks from a user-provided list.
 *
 * @param specs Pointer to a valid array of `TaskSpec`.
 * @param spec_count Number of entries in `specs`.
 * @return 0 on success, negative errno-like value on failure.
 */
int TASKS_Init(const TaskSpec* specs, int spec_count);
