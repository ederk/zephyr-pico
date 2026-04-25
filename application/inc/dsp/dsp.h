/*
 * Copyright (c) 2026
 * Author: Eder Knoner
 * Email: eder.knoner@ostermayer.com.br
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/kernel.h>
#include <dsp/input_pipeline.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************************************
 * Configuration
 *****************************************************************************************************/

/**
 * Log statistics every N completed blocks.
 * Default: 200 → ~1 log/s at ADC_BLOCK_MS=5ms (200 blocks/s).
 * At high sample rates, keep this large enough so printk does not
 * saturate the USB CDC-ACM TX buffer and block DSP_Thread.
 */
#ifndef DSP_LOG_BLOCKS
/* 1000 ms / ADC_BLOCK_MS = 100 blocks/s → default gives 1 log/s.
 * Keeping this value large enough prevents printk from saturating
 * the USB CDC-ACM TX buffer and blocking DSP_Thread. */
#define DSP_LOG_BLOCKS 3
#endif

/******************************************************************************************************
 * Data types
 *****************************************************************************************************/

/** All metrics are in raw ADC counts; the consumer converts to physical units. */

/** Metrics computed over one ADC block. */
typedef struct {
    int32_t mean_raw;    /**< Arithmetic mean (raw ADC counts). */
    int32_t rms_raw;     /**< RMS value      (raw ADC counts). */
    int16_t peak_min;    /**< Minimum sample (raw ADC counts). */
    int16_t peak_max;    /**< Maximum sample (raw ADC counts). */
    int16_t peak2peak;   /**< Peak-to-peak   (raw ADC counts). */
} DSP_BlockMetrics;

/******************************************************************************************************
 * Public API
 *****************************************************************************************************/

/**
 * @brief Compute mean, RMS and peak-to-peak over a block of ADC samples.
 *
 * @param buf    Pointer to ADC_BLOCK_SIZE int16_t samples.
 * @param n      Number of samples in buf.
 * @param out    Output metrics structure.
 */
void DSP_ComputeMetrics(const int16_t *buf, int n, DSP_BlockMetrics *out);

/**
 * @brief POSIX thread entry — generic sample-block consumer.
 *
 * @p arg must point to a valid SamplePipe.  The thread reads filled int16_t*
 * blocks from pipe->filled_q, computes DSP metrics every DSP_LOG_BLOCKS
 * blocks, prints results in millivolts, then returns the buffer to
 * pipe->free_q for the producer to refill.
 *
 * Decoupled from any specific producer: pass any SamplePipe to connect
 * a different source without modifying this module.
 *
 * @param arg  Pointer to a SamplePipe (must not be NULL).
 * @return NULL (never reached under normal operation).
 */
void *DSP_Thread(void *arg);

#ifdef __cplusplus
}
#endif
