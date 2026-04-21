/*
 * Copyright (c) 2026
 * Author: Eder Knoner
 * Email: eder.knoner@ostermayer.com.br
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/kernel.h>
#include <dsp/adc.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************************************
 * Public API
 *****************************************************************************************************/

/**
 * @brief POSIX thread entry that consumes ADC blocks from ADC_Thread.
 *
 * Receives filled int16_t buffers via adc_filled_q (zero-copy), processes
 * each block (currently prints it formatted as 32 samples per line), then
 * returns the buffer via adc_free_q so ADC_Thread can reuse it.
 *
 * Intended to be registered as a TaskSpec::entry in the application task table.
 * Loops forever; does not return under normal operation.
 *
 * @param arg Unused.
 * @return NULL (never reached under normal operation).
 */
void *DSP_Thread(void *arg);

#ifdef __cplusplus
}
#endif
