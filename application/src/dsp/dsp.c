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
#include <math.h>

/******************************************************************************************************
 * DSP helpers
 *****************************************************************************************************/

void DSP_ComputeMetrics(const int16_t *buf, int n, DSP_BlockMetrics *out)
{
    int64_t sum   = 0;
    int64_t sum_sq = 0;
    int16_t mn = buf[0], mx = buf[0];

    for (int i = 0; i < n; i++) {
        int16_t s = buf[i];
        sum    += s;
        sum_sq += (int32_t)s * s;
        if (s < mn) { mn = s; }
        if (s > mx) { mx = s; }
    }

    out->mean_raw  = (int32_t)(sum / n);
    out->rms_raw   = (int32_t)sqrtf((float)sum_sq / n);
    out->peak_min  = mn;
    out->peak_max  = mx;
    out->peak2peak = (int16_t)(mx - mn);
}

/******************************************************************************************************
 * Thread entry
 *****************************************************************************************************/

void *DSP_Thread(void *arg)
{
    const SamplePipe *pipe = (const SamplePipe *)arg;

    printk("DSP thread started\n");

    uint32_t block_count = 0;

    while (1) {
        int16_t *buf;
        k_msgq_get(pipe->filled_q, &buf, K_FOREVER);

        block_count++;

        if ((block_count % DSP_LOG_BLOCKS) == 0) {
            DSP_BlockMetrics m;
            DSP_ComputeMetrics(buf, (int)pipe->block_size, &m);

            int32_t mean_mv = SAMPLE_PIPE_TO_MV(pipe, m.mean_raw);
            int32_t rms_mv  = SAMPLE_PIPE_TO_MV(pipe, m.rms_raw);
            int32_t min_mv  = SAMPLE_PIPE_TO_MV(pipe, m.peak_min);
            int32_t max_mv  = SAMPLE_PIPE_TO_MV(pipe, m.peak_max);
            int32_t pp_mv   = SAMPLE_PIPE_TO_MV(pipe, m.peak2peak);

            int64_t ts_ms = k_uptime_get();
            printk("[%lld ms] block %u | mean=%d mV  rms=%d mV  pk-pk=%d mV  [%d..%d mV]\n",
                    ts_ms, block_count,
                    (int)mean_mv, (int)rms_mv, (int)pp_mv,
                    (int)min_mv, (int)max_mv);
        }

        k_msgq_put(pipe->free_q, &buf, K_NO_WAIT);
    }

    return NULL;
}

