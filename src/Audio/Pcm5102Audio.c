#include "Audio/Pcm5102Audio.h"

#include <string.h>

#include "Debug.h"
#include "Defines.h"
#include "amy.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/sync.h"

#include "tc_audio_i2s.pio.h"

#if AUDIO_I2S_BCLK_PIN == AUDIO_I2S_LRCLK_PIN || \
    AUDIO_I2S_BCLK_PIN == AUDIO_I2S_DATA_PIN || \
    AUDIO_I2S_LRCLK_PIN == AUDIO_I2S_DATA_PIN
#error "AUDIO_I2S_BCLK_PIN, AUDIO_I2S_LRCLK_PIN, and AUDIO_I2S_DATA_PIN must all be distinct"
#endif

#if AUDIO_I2S_LRCLK_PIN == (AUDIO_I2S_DATA_PIN + 1)
#define TC_AUDIO_DATA_LRCLK_PIN_BASE AUDIO_I2S_DATA_PIN
#define TC_AUDIO_DATA_PIN_SHIFT      0
#define TC_AUDIO_LRCLK_PIN_SHIFT     1
#elif AUDIO_I2S_DATA_PIN == (AUDIO_I2S_LRCLK_PIN + 1)
#define TC_AUDIO_DATA_LRCLK_PIN_BASE AUDIO_I2S_LRCLK_PIN
#define TC_AUDIO_DATA_PIN_SHIFT      1
#define TC_AUDIO_LRCLK_PIN_SHIFT     0
#else
#error "AUDIO_I2S_DATA_PIN and AUDIO_I2S_LRCLK_PIN must be adjacent GPIOs"
#endif

#define TC_AUDIO_WORDS_PER_FRAME 2u
#define TC_AUDIO_DEFAULT_LRCLK_LEFT  (AUDIO_I2S_LEFT_LRCLK_LEVEL ? true : false)
#define TC_AUDIO_DEFAULT_I2S_DELAY   true
#define TC_AUDIO_DEFAULT_PAIR_SWAP   false

#if AUDIO_I2S_BUFFER_COUNT < 2
#error "AUDIO_I2S_BUFFER_COUNT must be at least 2"
#endif

typedef enum
{
    TC_AUDIO_BUFFER_FREE = 0,
    TC_AUDIO_BUFFER_FILLING,
    TC_AUDIO_BUFFER_QUEUED,
    TC_AUDIO_BUFFER_PLAYING
} TcAudioBufferState;

static PIO tc_audio_pio(void)
{
    return AUDIO_I2S_PIO_INDEX ? pio1 : pio0;
}

static uint tc_audio_dreq(void)
{
    return (AUDIO_I2S_PIO_INDEX ? DREQ_PIO1_TX0 : DREQ_PIO0_TX0) + AUDIO_I2S_SM;
}

static bool tc_audio_started = false;
static bool tc_audio_initialized = false;
static volatile int8_t tc_audio_playing_buffer = -1;
static volatile bool tc_audio_playing_silence = false;
static volatile uint8_t tc_audio_queue[AUDIO_I2S_BUFFER_COUNT];
static volatile uint8_t tc_audio_queue_read = 0;
static volatile uint8_t tc_audio_queue_write = 0;
static volatile uint8_t tc_audio_queue_count = 0;
static volatile TcAudioBufferState tc_audio_buffer_states[AUDIO_I2S_BUFFER_COUNT];
static volatile uint32_t tc_audio_blocks_written = 0;
static volatile uint32_t tc_audio_dma_transfers = 0;
static volatile uint32_t tc_audio_underruns = 0;
static volatile uint32_t tc_audio_last_frame_count = 0;
static volatile int16_t tc_audio_last_min_sample = 0;
static volatile int16_t tc_audio_last_max_sample = 0;
static volatile bool tc_audio_left_lrclk = TC_AUDIO_DEFAULT_LRCLK_LEFT;
static volatile bool tc_audio_i2s_delay = TC_AUDIO_DEFAULT_I2S_DELAY;
static volatile bool tc_audio_pin_pair_swap = TC_AUDIO_DEFAULT_PAIR_SWAP;
static const int16_t tc_audio_zero_samples[AMY_BLOCK_SIZE * 2u] = {0};
static uint32_t tc_audio_silence_buffer[AMY_BLOCK_SIZE * TC_AUDIO_WORDS_PER_FRAME];
static uint32_t tc_audio_packed_buffers[AUDIO_I2S_BUFFER_COUNT][AMY_BLOCK_SIZE * TC_AUDIO_WORDS_PER_FRAME];

static uint32_t tc_audio_pack_pin_pair(uint32_t sample_bit, uint32_t lrclk_bit)
{
    uint32_t pin_bits = 0;

    pin_bits |= sample_bit << TC_AUDIO_DATA_PIN_SHIFT;
    pin_bits |= lrclk_bit << TC_AUDIO_LRCLK_PIN_SHIFT;

    if (tc_audio_pin_pair_swap) {
        return ((pin_bits & 0x1u) << 1u) | ((pin_bits >> 1u) & 0x1u);
    }

    return pin_bits;
}

static uint32_t tc_audio_expand_half(uint16_t sample, bool lrclk_high, bool next_lrclk_high, bool i2s_delay)
{
    uint32_t packed = 0;
    uint32_t lrclk_bit = lrclk_high ? 1u : 0u;
    uint32_t next_lrclk_bit = next_lrclk_high ? 1u : 0u;

    for (int bit = 15; bit >= 0; --bit) {
        uint32_t sample_bit = (sample >> bit) & 0x1u;
        uint32_t pair = tc_audio_pack_pin_pair(sample_bit, i2s_delay && bit == 0 ? next_lrclk_bit : lrclk_bit);
        packed = (packed << 2) | pair;
    }

    return packed;
}

static void tc_audio_pack(const int16_t *samples, uint32_t *packed, size_t frame_count)
{
    bool left_lrclk = tc_audio_left_lrclk;
    bool right_lrclk = !left_lrclk;
    bool i2s_delay = tc_audio_i2s_delay;

    for (size_t i = 0; i < frame_count; ++i) {
        uint16_t left = (uint16_t)samples[i * 2];
        uint16_t right = (uint16_t)samples[i * 2 + 1];
        packed[i * TC_AUDIO_WORDS_PER_FRAME] = tc_audio_expand_half(left, left_lrclk, right_lrclk, i2s_delay);
        packed[i * TC_AUDIO_WORDS_PER_FRAME + 1] = tc_audio_expand_half(right, right_lrclk, left_lrclk, i2s_delay);
    }
}

static void tc_audio_start_dma(const uint32_t *packed, size_t word_count)
{
    dma_channel_transfer_from_buffer_now(AUDIO_I2S_DMA_CHANNEL, packed, word_count);
    tc_audio_dma_transfers++;
}

static bool tc_audio_pop_queued_buffer(uint8_t *buffer_index)
{
    if (tc_audio_queue_count == 0) {
        return false;
    }

    *buffer_index = tc_audio_queue[tc_audio_queue_read];
    tc_audio_queue_read = (uint8_t)((tc_audio_queue_read + 1u) % AUDIO_I2S_BUFFER_COUNT);
    tc_audio_queue_count--;
    return true;
}

static void tc_audio_start_next_dma(void)
{
    uint8_t buffer_index;
    const size_t word_count = AMY_BLOCK_SIZE * TC_AUDIO_WORDS_PER_FRAME;

    if (tc_audio_pop_queued_buffer(&buffer_index)) {
        tc_audio_playing_buffer = (int8_t)buffer_index;
        tc_audio_playing_silence = false;
        tc_audio_buffer_states[buffer_index] = TC_AUDIO_BUFFER_PLAYING;
        tc_audio_start_dma(tc_audio_packed_buffers[buffer_index], word_count);
    } else {
        tc_audio_playing_buffer = -1;
        tc_audio_playing_silence = true;
        tc_audio_underruns++;
        tc_audio_start_dma(tc_audio_silence_buffer, word_count);
    }

    tc_audio_started = true;
}

static void __isr __time_critical_func(tc_audio_dma_irq_handler)(void)
{
    if (!dma_irqn_get_channel_status(0, AUDIO_I2S_DMA_CHANNEL)) {
        return;
    }

    dma_irqn_acknowledge_channel(0, AUDIO_I2S_DMA_CHANNEL);

    if (tc_audio_playing_buffer >= 0) {
        tc_audio_buffer_states[(uint8_t)tc_audio_playing_buffer] = TC_AUDIO_BUFFER_FREE;
        tc_audio_playing_buffer = -1;
    }
    tc_audio_playing_silence = false;

    tc_audio_start_next_dma();
}

static uint8_t tc_audio_acquire_free_buffer(void)
{
    while (true) {
        uint32_t ints = save_and_disable_interrupts();

        for (uint8_t i = 0; i < AUDIO_I2S_BUFFER_COUNT; ++i) {
            if (tc_audio_buffer_states[i] == TC_AUDIO_BUFFER_FREE) {
                tc_audio_buffer_states[i] = TC_AUDIO_BUFFER_FILLING;
                restore_interrupts(ints);
                return i;
            }
        }

        restore_interrupts(ints);
        tight_loop_contents();
    }
}

static void tc_audio_queue_filled_buffer(uint8_t buffer_index)
{
    uint32_t ints = save_and_disable_interrupts();

    tc_audio_buffer_states[buffer_index] = TC_AUDIO_BUFFER_QUEUED;
    tc_audio_queue[tc_audio_queue_write] = buffer_index;
    tc_audio_queue_write = (uint8_t)((tc_audio_queue_write + 1u) % AUDIO_I2S_BUFFER_COUNT);
    tc_audio_queue_count++;

    if (!tc_audio_started) {
        tc_audio_start_next_dma();
    }

    restore_interrupts(ints);
}

static uint8_t tc_audio_count_free_buffers(void)
{
    uint8_t free_buffers = 0;

    for (uint8_t i = 0; i < AUDIO_I2S_BUFFER_COUNT; ++i) {
        if (tc_audio_buffer_states[i] == TC_AUDIO_BUFFER_FREE) {
            free_buffers++;
        }
    }

    return free_buffers;
}

static void tc_audio_record_sample_range(const int16_t *samples, size_t sample_count)
{
    int16_t min_sample = samples[0];
    int16_t max_sample = samples[0];

    for (size_t i = 1; i < sample_count; ++i) {
        if (samples[i] < min_sample) {
            min_sample = samples[i];
        }
        if (samples[i] > max_sample) {
            max_sample = samples[i];
        }
    }

    tc_audio_last_min_sample = min_sample;
    tc_audio_last_max_sample = max_sample;
}

bool tc_audio_init(void)
{
    PIO pio = tc_audio_pio();
    int offset;
    uint32_t divider = (clock_get_hz(clk_sys) * 4u) / AMY_SAMPLE_RATE;
    dma_channel_config dma_cfg = dma_channel_get_default_config(AUDIO_I2S_DMA_CHANNEL);
    const struct pio_program *program = &tc_audio_i2s_program;

    tc_debug_logf("audio init bclk=%u din=%u lrclk=%u pio=%u sm=%u dma=%u base=%u data_shift=%u lrclk_shift=%u left_lrclk=%u i2s_delay=%u pair_swap=%u",
        AUDIO_I2S_BCLK_PIN,
        AUDIO_I2S_DATA_PIN,
        AUDIO_I2S_LRCLK_PIN,
        AUDIO_I2S_PIO_INDEX,
        AUDIO_I2S_SM,
        AUDIO_I2S_DMA_CHANNEL,
        TC_AUDIO_DATA_LRCLK_PIN_BASE,
        TC_AUDIO_DATA_PIN_SHIFT,
        TC_AUDIO_LRCLK_PIN_SHIFT,
        tc_audio_left_lrclk ? 1u : 0u,
        tc_audio_i2s_delay ? 1u : 0u,
        tc_audio_pin_pair_swap ? 1u : 0u);

    if (pio_sm_is_claimed(pio, AUDIO_I2S_SM)) {
        tc_debug_logf("audio init failed: pio sm already claimed");
        return false;
    }

    if (dma_channel_is_claimed(AUDIO_I2S_DMA_CHANNEL)) {
        tc_debug_logf("audio init failed: dma channel already claimed");
        return false;
    }

    if (!pio_can_add_program(pio, program)) {
        tc_debug_logf("audio init failed: no pio instruction space");
        return false;
    }

    pio_sm_claim(pio, AUDIO_I2S_SM);
    offset = pio_add_program(pio, program);
    if (offset < 0) {
        pio_sm_unclaim(pio, AUDIO_I2S_SM);
        tc_debug_logf("audio init failed: pio_add_program rc=%d", offset);
        return false;
    }

    pio_gpio_init(pio, TC_AUDIO_DATA_LRCLK_PIN_BASE);
    pio_gpio_init(pio, TC_AUDIO_DATA_LRCLK_PIN_BASE + 1);
    pio_gpio_init(pio, AUDIO_I2S_BCLK_PIN);

    tc_audio_i2s_program_init(
        pio,
        AUDIO_I2S_SM,
        (uint)offset,
        TC_AUDIO_DATA_LRCLK_PIN_BASE,
        AUDIO_I2S_BCLK_PIN
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
    irq_add_shared_handler(DMA_IRQ_0, tc_audio_dma_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    dma_irqn_set_channel_enabled(0, AUDIO_I2S_DMA_CHANNEL, true);
    irq_set_enabled(DMA_IRQ_0, true);

    pio_sm_set_enabled(pio, AUDIO_I2S_SM, true);

    memset(tc_audio_packed_buffers, 0, sizeof(tc_audio_packed_buffers));
    for (uint8_t i = 0; i < AUDIO_I2S_BUFFER_COUNT; ++i) {
        tc_audio_queue[i] = 0;
        tc_audio_buffer_states[i] = TC_AUDIO_BUFFER_FREE;
    }
    tc_audio_pack(tc_audio_zero_samples, tc_audio_silence_buffer, AMY_BLOCK_SIZE);
    tc_audio_initialized = true;
    tc_audio_started = false;
    tc_audio_playing_buffer = -1;
    tc_audio_playing_silence = false;
    tc_audio_queue_read = 0;
    tc_audio_queue_write = 0;
    tc_audio_queue_count = 0;
    tc_audio_blocks_written = 0;
    tc_audio_dma_transfers = 0;
    tc_audio_underruns = 0;
    tc_audio_last_frame_count = 0;
    tc_audio_last_min_sample = 0;
    tc_audio_last_max_sample = 0;

    tc_debug_logf("audio init ok offset=%d clkdiv_fixed=%lu words_per_frame=%u",
        offset,
        (unsigned long)divider,
        TC_AUDIO_WORDS_PER_FRAME);

    return true;
}

void tc_audio_write_blocking(const int16_t *samples, size_t frame_count)
{
    uint8_t buffer_index = tc_audio_acquire_free_buffer();
    uint32_t *packed = tc_audio_packed_buffers[buffer_index];

    tc_audio_record_sample_range(samples, frame_count * 2u);
    tc_audio_pack(samples, packed, frame_count);
    tc_audio_queue_filled_buffer(buffer_index);
    tc_audio_blocks_written++;
    tc_audio_last_frame_count = (uint32_t)frame_count;
}

void tc_audio_get_status(TcAudioStatus *status)
{
    if (!status) {
        return;
    }

    status->initialized = tc_audio_initialized;
    status->started = tc_audio_started;
    status->dma_busy = dma_channel_is_busy(AUDIO_I2S_DMA_CHANNEL);
    status->blocks_written = tc_audio_blocks_written;
    status->dma_transfers = tc_audio_dma_transfers;
    status->underruns = tc_audio_underruns;
    status->last_frame_count = tc_audio_last_frame_count;
    status->buffer_count = AUDIO_I2S_BUFFER_COUNT;
    status->queued_buffers = tc_audio_queue_count;
    status->free_buffers = tc_audio_count_free_buffers();
    status->playing_silence = tc_audio_playing_silence ? 1u : 0u;
    status->last_min_sample = tc_audio_last_min_sample;
    status->last_max_sample = tc_audio_last_max_sample;
    status->left_lrclk_level = tc_audio_left_lrclk ? 1u : 0u;
    status->i2s_delay = tc_audio_i2s_delay ? 1u : 0u;
    status->pin_pair_swap = tc_audio_pin_pair_swap ? 1u : 0u;
}

void tc_audio_set_left_lrclk_level(uint8_t level)
{
    tc_audio_left_lrclk = level ? true : false;
    tc_debug_logf("audio left_lrclk set to %u", tc_audio_left_lrclk ? 1u : 0u);
}

void tc_audio_set_i2s_delay(bool enabled)
{
    tc_audio_i2s_delay = enabled;
    tc_debug_logf("audio i2s_delay set to %u", tc_audio_i2s_delay ? 1u : 0u);
}

void tc_audio_set_pin_pair_swap(bool enabled)
{
    tc_audio_pin_pair_swap = enabled;
    tc_debug_logf("audio pin_pair_swap set to %u", tc_audio_pin_pair_swap ? 1u : 0u);
}
