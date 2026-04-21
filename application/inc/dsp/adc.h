/*
 * Copyright (c) 2026
 * Author: Eder Knoner
 * Email: eder.knoner@ostermayer.com.br
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************************************
 * Parameters
 *****************************************************************************************************/

/** Sampling rate in Hz. */
#define ADC_FS_HZ           40000
/** Block duration in milliseconds. */
#define ADC_BLOCK_MS        5
/** Number of samples per async block — derived from rate and duration. */
#define ADC_BLOCK_SIZE      (ADC_FS_HZ * ADC_BLOCK_MS / 1000)
/** Print statistics every N completed blocks. */
#define ADC_LOG_BLOCKS  4

/******************************************************************************************************
 * Inter-thread queues (zero-copy buffer handoff)
 *****************************************************************************************************/

/**
 * @brief Queue of filled buffers sent from ADC_Thread to DSP_Thread.
 *        Each element is a int16_t* pointing to ADC_BLOCK_SIZE samples.
 */
extern struct k_msgq adc_filled_q;

/**
 * @brief Queue of free buffers returned from DSP_Thread to ADC_Thread.
 *        Each element is a int16_t* ready to be refilled.
 */
extern struct k_msgq adc_free_q;

/******************************************************************************************************
 * Public API
 *****************************************************************************************************/

/**
 * @brief POSIX thread entry that runs the ADC async ping-pong acquisition.
 *
 * Collects ADC_BLOCK_SIZE samples at ADC_FS_HZ using two alternating DMA
 * buffers (ping-pong). While the hardware fills one buffer via adc_read_async,
 * the thread processes the previous buffer, minimising dead time between blocks.
 *
 * Intended to be registered as a TaskSpec::entry in the application task table.
 * Loops forever; does not return under normal operation.
 *
 * @param arg Unused.
 * @return NULL (never reached under normal operation).
 */
void *ADC_Thread(void *arg);

#ifdef __cplusplus
}
#endif
