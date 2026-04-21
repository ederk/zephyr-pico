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

#include <dsp/dsp.h>

#include <zephyr/sys/printk.h>

#ifdef CONFIG_ADC

/******************************************************************************************************
 * Thread entry
 *****************************************************************************************************/

void *DSP_Thread(void *arg)
{
    (void)arg;

    printk("DSP thread started\n");

    static char line[32 * 7 + 8];
    uint32_t block_count = 0;

    while (1) {
        int16_t *buf;
        k_msgq_get(&adc_filled_q, &buf, K_FOREVER);

        block_count++;

        if ((block_count % ADC_LOG_BLOCKS) == 0) {
            int64_t ts_ms = k_uptime_get();

            printk("[%lld ms] ADC block %u (%d samples):\n",
                   ts_ms, block_count, ADC_BLOCK_SIZE);

            for (int row = 0; row < ADC_BLOCK_SIZE / 32; row++) {
                int pos = 0;

                for (int col = 0; col < 32; col++) {
                    pos += snprintk(line + pos, sizeof(line) - pos,
                                    "%5d", buf[row * 32 + col]);
                }

                printk("%s\n", line);
            }
        }

        /* Return buffer to ADC_Thread. */
        k_msgq_put(&adc_free_q, &buf, K_NO_WAIT);
    }

    return NULL;
}

#endif /* CONFIG_ADC */
