/*
 * Copyright (c) 2026
 * Author: Eder Knoner
 * Email: eder.knoner@ostermayer.com.br
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * @file input_pipeline.h
 * @brief Generic sample-stream pipe descriptor.
 *
 * Defines the SamplePipe contract used between any producer module
 * (ADC, UART, SPI sensor, ...) and the DSP consumer.
 *
 * Neither the producer nor the consumer includes the other's header;
 * both depend only on this interface.
 */

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************************************
 * Sample-stream pipe descriptor
 *****************************************************************************************************/

/**
 * @brief Descriptor of a zero-copy sample stream between a producer and a consumer.
 *
 * A producer thread fills int16_t blocks and posts pointers to @p filled_q.
 * The consumer reads from @p filled_q, processes the data, then returns
 * the pointer to @p free_q so the producer can refill it.
 *
 * Usage:
 *  - The producer module owns the queues and defines its SamplePipe.
 *  - The application wires the consumer by passing `SamplePipe *` as the thread arg.
 *  - To swap producers, pass a different SamplePipe — no DSP code changes.
 */
typedef struct {
    struct k_msgq *filled_q;   /**< Producer → consumer: filled int16_t* blocks. */
    struct k_msgq *free_q;     /**< Consumer → producer: recycled int16_t* blocks. */
    uint16_t       block_size; /**< Samples per block. */
    int32_t        vref_mv;    /**< Reference voltage in millivolts. */
    uint8_t        bits;       /**< ADC resolution in bits. */
} SamplePipe;

/******************************************************************************************************
 * Raw-to-millivolt conversion
 *****************************************************************************************************/

/**
 * @brief Convert a raw sample to millivolts using the pipe's calibration.
 * @param pipe  Pointer to a SamplePipe.
 * @param raw   int32_t raw ADC count.
 * @return      int32_t value in millivolts.
 */
#define SAMPLE_PIPE_TO_MV(pipe, raw) \
    ((int32_t)(raw) * (int32_t)(pipe)->vref_mv / ((1 << (int32_t)(pipe)->bits) - 1))

#ifdef __cplusplus
}
#endif
