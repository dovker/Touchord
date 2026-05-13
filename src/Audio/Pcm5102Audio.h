#ifndef TOUCHORD_PCM5102_AUDIO_H
#define TOUCHORD_PCM5102_AUDIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    bool initialized;
    bool started;
    bool dma_busy;
    uint32_t blocks_written;
    uint32_t dma_transfers;
    uint32_t underruns;
    uint32_t last_frame_count;
    uint8_t buffer_count;
    uint8_t queued_buffers;
    uint8_t free_buffers;
    uint8_t playing_silence;
    int16_t last_min_sample;
    int16_t last_max_sample;
    uint8_t left_lrclk_level;
    uint8_t i2s_delay;
    uint8_t pin_pair_swap;
} TcAudioStatus;

bool tc_audio_init(void);
void tc_audio_write_blocking(const int16_t *samples, size_t frame_count);
void tc_audio_get_status(TcAudioStatus *status);
void tc_audio_set_left_lrclk_level(uint8_t level);
void tc_audio_set_i2s_delay(bool enabled);
void tc_audio_set_pin_pair_swap(bool enabled);

#endif
