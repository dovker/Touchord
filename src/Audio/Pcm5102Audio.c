#include "Audio/Pcm5102Audio.h"

#include <string.h>

#include "Defines.h"
#include "amy.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

#include "tc_audio_i2s.pio.h"

static PIO tc_audio_pio(void)
{
    return AUDIO_I2S_PIO_INDEX ? pio1 : pio0;
}

static uint tc_audio_dreq(void)
{
    return (AUDIO_I2S_PIO_INDEX ? DREQ_PIO1_TX0 : DREQ_PIO0_TX0) + AUDIO_I2S_SM;
}

static bool tc_audio_started = false;
static uint8_t tc_audio_next_buffer = 0;
static uint32_t tc_audio_packed_buffers[2][AMY_BLOCK_SIZE];

static void tc_audio_pack(const int16_t *samples, uint32_t *packed, size_t frame_count)
{
    for (size_t i = 0; i < frame_count; ++i) {
        uint16_t left = (uint16_t)samples[i * 2];
        uint16_t right = (uint16_t)samples[i * 2 + 1];
        packed[i] = ((uint32_t)left << 16) | right;
    }
}

static void tc_audio_start_dma(const uint32_t *packed, size_t frame_count)
{
    dma_channel_transfer_from_buffer_now(AUDIO_I2S_DMA_CHANNEL, packed, frame_count);
}

bool tc_audio_init(void)
{
    PIO pio = tc_audio_pio();
    uint offset = pio_add_program(pio, &tc_audio_i2s_program);
    uint32_t divider = (clock_get_hz(clk_sys) * 4u) / AMY_SAMPLE_RATE;
    dma_channel_config dma_cfg = dma_channel_get_default_config(AUDIO_I2S_DMA_CHANNEL);

    pio_gpio_init(pio, AUDIO_I2S_DATA_PIN);
    pio_gpio_init(pio, AUDIO_I2S_CLOCK_PIN_BASE);
    pio_gpio_init(pio, AUDIO_I2S_CLOCK_PIN_BASE + 1);

    tc_audio_i2s_program_init(
        pio,
        AUDIO_I2S_SM,
        offset,
        AUDIO_I2S_DATA_PIN,
        AUDIO_I2S_CLOCK_PIN_BASE
    );
    pio_sm_set_clkdiv_int_frac(pio, AUDIO_I2S_SM, divider >> 8u, divider & 0xFFu);

    dma_channel_claim(AUDIO_I2S_DMA_CHANNEL);
    channel_config_set_dreq(&dma_cfg, tc_audio_dreq());
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_cfg, true);
    channel_config_set_write_increment(&dma_cfg, false);
    dma_channel_configure(
        AUDIO_I2S_DMA_CHANNEL,
        &dma_cfg,
        &pio->txf[AUDIO_I2S_SM],
        NULL,
        0,
        false
    );

    pio_sm_set_enabled(pio, AUDIO_I2S_SM, true);

    memset(tc_audio_packed_buffers, 0, sizeof(tc_audio_packed_buffers));
    tc_audio_started = false;
    tc_audio_next_buffer = 0;

    return true;
}

void tc_audio_write_blocking(const int16_t *samples, size_t frame_count)
{
    uint32_t *packed = tc_audio_packed_buffers[tc_audio_next_buffer];

    tc_audio_pack(samples, packed, frame_count);

    if (tc_audio_started) {
        dma_channel_wait_for_finish_blocking(AUDIO_I2S_DMA_CHANNEL);
    }

    tc_audio_start_dma(packed, frame_count);
    tc_audio_started = true;
    tc_audio_next_buffer ^= 1u;
}
