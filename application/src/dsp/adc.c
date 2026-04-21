/*
 * Copyright (c) 2026
 * Author: Eder Knoner
 * Email: eder.knoner@ostermayer.com.br
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************************************
 * Include Section
 *****************************************************************************************************/

#include <dsp/adc.h>

#include <zephyr/drivers/adc.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#ifdef CONFIG_ADC

/******************************************************************************************************
 * Message queues (zero-copy buffer handoff to DSP_Thread)
 *****************************************************************************************************/

K_MSGQ_DEFINE(adc_filled_q, sizeof(int16_t *), 2, 4);
K_MSGQ_DEFINE(adc_free_q,   sizeof(int16_t *), 2, 4);

/******************************************************************************************************
 * Private state
 *****************************************************************************************************/

static const struct adc_dt_spec adc_ch0 =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

/** Ping-pong buffers: hardware fills one while DSP_Thread processes the other. */
static int16_t adc_buf[2][ADC_BLOCK_SIZE];

/******************************************************************************************************
 * Thread entry
 *****************************************************************************************************/

void *ADC_Thread(void *arg)
{
    (void)arg;

    if (!adc_is_ready_dt(&adc_ch0)) {
        printk("ADC not ready\n");
        return NULL;
    }

    int ret = adc_channel_setup_dt(&adc_ch0);
    if (ret < 0) {
        printk("ADC channel setup failed: %d\n", ret);
        return NULL;
    }

    printk("ADC thread started (ch%d, %d-bit, %d Hz, block=%d)\n",
           adc_ch0.channel_id, adc_ch0.resolution, ADC_FS_HZ, ADC_BLOCK_SIZE);

    /* Sequence options: inter-sample interval + number of samples per block. */
    static const struct adc_sequence_options opts = {
        .interval_us     = 1000000U / ADC_FS_HZ,
        .extra_samplings = ADC_BLOCK_SIZE - 1,
    };

    /* Pre-populate free queue with both ping-pong buffers. */
    int16_t *p0 = adc_buf[0], *p1 = adc_buf[1];
    k_msgq_put(&adc_free_q, &p0, K_NO_WAIT);
    k_msgq_put(&adc_free_q, &p1, K_NO_WAIT);

    struct k_poll_signal sig;
    struct adc_sequence   seq;
    struct k_poll_event   evt;

    while (1) {
        /* Wait for a free buffer from DSP_Thread. */
        int16_t *buf;
        k_msgq_get(&adc_free_q, &buf, K_FOREVER);

        /* Configure sequence for this buffer. */
        k_poll_signal_init(&sig);
        adc_sequence_init_dt(&adc_ch0, &seq);
        seq.buffer      = buf;
        seq.buffer_size = ADC_BLOCK_SIZE * sizeof(int16_t);
        seq.options     = &opts;

        ret = adc_read_async(adc_ch0.dev, &seq, &sig);
        if (ret < 0) {
            printk("ADC async read failed: %d\n", ret);
            /* Return buffer to avoid starvation. */
            k_msgq_put(&adc_free_q, &buf, K_NO_WAIT);
            continue;
        }

        /* Wait for hardware to finish filling the buffer. */
        k_poll_event_init(&evt, K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &sig);
        k_poll(&evt, 1, K_FOREVER);

        /* Hand filled buffer to DSP_Thread. */
        k_msgq_put(&adc_filled_q, &buf, K_FOREVER);
    }

    return NULL;
}

#endif /* CONFIG_ADC */
