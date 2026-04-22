#ifndef TOUCHORD_PCM5102_AUDIO_H
#define TOUCHORD_PCM5102_AUDIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool tc_audio_init(void);
void tc_audio_write_blocking(const int16_t *samples, size_t frame_count);

#endif
