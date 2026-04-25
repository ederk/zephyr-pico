/*
 * Copyright (c) 2026
 * Author: Eder Knoner
 * Email: eder.knoner@ostermayer.com.br
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <dsp/input_pipeline.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************************************
 * Acquisition parameters
 *****************************************************************************************************/

/** Sampling rate in Hz. */
#define ADC_FS_HZ      40000
/** Block duration in milliseconds. */
#define ADC_BLOCK_MS   10
/** Number of samples per async block (derived from rate and duration). */
#define ADC_BLOCK_SIZE (ADC_FS_HZ * ADC_BLOCK_MS / 1000)

/******************************************************************************************************
 * Inter-thread queues (zero-copy buffer handoff)
 *****************************************************************************************************/

/** Queue of filled buffers — posted by ADC_Thread, consumed by the DSP consumer. */
extern struct k_msgq adc_filled_q;
/** Queue of free buffers — returned by the DSP consumer, recycled by ADC_Thread. */
extern struct k_msgq adc_free_q;

/**
 * @brief SamplePipe descriptor for the ADC source.
 *
 * Pass as the @p arg of DSP_Thread (or any other consumer) to wire it to this
 * producer.  To connect a different source, define and pass its own SamplePipe.
 */
extern const SamplePipe adc_pipe;

/******************************************************************************************************
 * Public API
 *****************************************************************************************************/

/**
 * @brief POSIX thread entry for ADC async ping-pong acquisition.
 *
 * Collects ADC_BLOCK_SIZE samples at ADC_FS_HZ using two alternating ping-pong
 * buffers. Posts filled int16_t* to adc_filled_q and recycles from adc_free_q.
 *
 * @param arg Unused.
 * @return NULL (never reached under normal operation).
 */
void *ADC_Thread(void *arg);

#ifdef __cplusplus
}
#endif
