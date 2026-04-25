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

#include <zephyr/device.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* pico-sdk ADC HAL — available because ADC_RPI_PICO=y selects PICOSDK_USE_ADC. */
#include <hardware/adc.h>

/* RP2040 DMA slot constant for the ADC DREQ. */
#include <zephyr/dt-bindings/dma/rpi-pico-dma-rp2040.h>

#ifdef CONFIG_ADC

/******************************************************************************************************
 * Message queues (zero-copy buffer handoff to DSP_Thread)
 *****************************************************************************************************/

K_MSGQ_DEFINE(adc_filled_q, sizeof(int16_t *), 2, 4);
K_MSGQ_DEFINE(adc_free_q,   sizeof(int16_t *), 2, 4);

/** SamplePipe descriptor published for consumers (e.g. DSP_Thread). */
const SamplePipe adc_pipe = {
    .filled_q   = &adc_filled_q,
    .free_q     = &adc_free_q,
    .block_size = ADC_BLOCK_SIZE,
    .vref_mv    = 3300,
    .bits       = 12,
};

/******************************************************************************************************
 * Private state
 *****************************************************************************************************/

/** Ping-pong buffers: DMA fills one while DSP_Thread processes the other. */
static int16_t adc_buf[2][ADC_BLOCK_SIZE];

static K_SEM_DEFINE(adc_dma_done, 0, 1);
static const struct device *adc_dma_dev = DEVICE_DT_GET(DT_NODELABEL(dma));

/* Use DMA channel 0 exclusively for ADC. */
#define ADC_DMA_CHANNEL 0U

/******************************************************************************************************
 * DMA callback — invoked from ISR when block transfer completes
 *****************************************************************************************************/

static void adc_dma_callback(const struct device *dev, void *user_data,
                              uint32_t channel, int status)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(user_data);
    ARG_UNUSED(channel);
    ARG_UNUSED(status);
    /* Stop free-running ADC; thread will restart it for the next block. */
    adc_run(false);
    k_sem_give(&adc_dma_done);
}

/******************************************************************************************************
 * Thread entry
 *****************************************************************************************************/

void *ADC_Thread(void *arg)
{
    (void)arg;

    if (!device_is_ready(adc_dma_dev)) {
        printk("ADC: DMA device not ready\n");
        return NULL;
    }

    /* Configure ADC hardware via pico-sdk HAL. */
    adc_init();
    adc_gpio_init(26U);          /* GP26 = ADC input 0 */
    adc_select_input(0U);

    /*
     * clkdiv formula (RP2040 datasheet §4.9.2):
     *   fs = 48 MHz / (clkdiv + 1)   →   clkdiv = 48 MHz / fs - 1
     * For ADC_FS_HZ = 40 000 Hz: 48 000 000 / 40 000 - 1 = 1199
     */
    adc_set_clkdiv((48000000.0f / (float)ADC_FS_HZ) - 1.0f);

    /*
     * FIFO: enable=true, dreq_en=true (gates DMA via DREQ),
     * thresh=1 (fire DMA DREQ for every sample), err_in_fifo=false, byte_shift=false.
     */
    adc_fifo_setup(true, true, 1U, false, false);

    /*
     * Disable ADC FIFO IRQ.  The in-tree adc_rpi_pico driver enables it at
     * boot; DMA does not use interrupts on the ADC side.
     */
    adc_irq_set_enabled(false);

    printk("ADC DMA thread started (%d Hz, block=%d samples, DMA ch%u)\n",
           ADC_FS_HZ, ADC_BLOCK_SIZE, ADC_DMA_CHANNEL);

    /* Pre-populate free queue with both ping-pong buffers. */
    int16_t *p0 = adc_buf[0], *p1 = adc_buf[1];
    k_msgq_put(&adc_free_q, &p0, K_NO_WAIT);
    k_msgq_put(&adc_free_q, &p1, K_NO_WAIT);

    /*
     * DMA block config: ADC FIFO → ping-pong buffer, 16-bit transfers, ADC DREQ pacing.
     *
     * NOTE: The Zephyr RP2040 DMA driver passes dma_block_config.block_size directly
     * to the pico-sdk dma_channel_configure() as the transfer_count (number of
     * transfers, NOT bytes).  For 16-bit transfers, set block_size = number of
     * samples so that the hardware performs exactly ADC_BLOCK_SIZE × 16-bit reads.
     */
    struct dma_block_config block = {
        .source_address  = (uint32_t)&adc_hw->fifo,
        .source_addr_adj = DMA_ADDR_ADJ_NO_CHANGE,
        .dest_addr_adj   = DMA_ADDR_ADJ_INCREMENT,
        .block_size      = ADC_BLOCK_SIZE,   /* transfer_count, not bytes — see note above */
    };
    struct dma_config dma_cfg = {
        .channel_direction = PERIPHERAL_TO_MEMORY,
        .source_data_size  = sizeof(uint16_t),  /* 2 → DMA_SIZE_16 */
        .dest_data_size    = sizeof(uint16_t),
        /*
         * The Zephyr rpi-pico-dma-rp2040.h header defines RPI_PICO_DMA_SLOT_ADC
         * with DREQ=0x34 (52), which is WRONG for RP2040.  The actual ADC DREQ
         * from the pico-sdk HAL is DREQ_ADC=36 (0x24).  The header has a +16
         * offset bug for all entries from I2C onwards.  Use the raw value so
         * the Zephyr driver calls channel_config_set_dreq(config, 36).
         */
        .dma_slot          = RPI_PICO_DMA_DREQ_TO_SLOT(36U),  /* DREQ_ADC = 36 */
        .dma_callback      = adc_dma_callback,
        .head_block        = &block,
        .block_count       = 1U,
    };

    /* Diagnostics: measure actual hardware sample rate, log once per second. */
    uint32_t diag_blocks = 0U;
    uint32_t diag_cyc_t0 = k_cycle_get_32();

    while (1) {
        int16_t *buf;
        k_msgq_get(&adc_free_q, &buf, K_FOREVER);

        block.dest_address = (uint32_t)buf;

        int ret = dma_config(adc_dma_dev, ADC_DMA_CHANNEL, &dma_cfg);
        if (ret < 0) {
            printk("ADC: dma_config failed: %d\n", ret);
            k_msgq_put(&adc_free_q, &buf, K_NO_WAIT);
            continue;
        }

        ret = dma_start(adc_dma_dev, ADC_DMA_CHANNEL);
        if (ret < 0) {
            printk("ADC: dma_start failed: %d\n", ret);
            k_msgq_put(&adc_free_q, &buf, K_NO_WAIT);
            continue;
        }

        adc_run(true);
        k_sem_take(&adc_dma_done, K_FOREVER);

        /* Measure hardware throughput ~1×/s to verify clkdiv setting. */
        diag_blocks++;
        if (diag_blocks >= (1000U / ADC_BLOCK_MS)) {
            uint32_t elapsed_us =
                k_cyc_to_us_near32(k_cycle_get_32() - diag_cyc_t0);
            printk("[ADC] hw_fs=%d Hz (target %d)\n",
                   (int)((int64_t)diag_blocks * ADC_BLOCK_SIZE * 1000000LL /
                         elapsed_us),
                   ADC_FS_HZ);
            diag_blocks  = 0U;
            diag_cyc_t0  = k_cycle_get_32();
        }

        k_msgq_put(&adc_filled_q, &buf, K_FOREVER);
    }

    return NULL;
}

#endif /* CONFIG_ADC */
