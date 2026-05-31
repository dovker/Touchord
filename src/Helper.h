/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_HELPER_H
#define TOUCHORD_HELPER_H

#include "tusb.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"


static inline void tc_log(const char* str)
{
    tud_cdc_n_write_str(0, str);
    tud_cdc_n_write_char(0, '\n');
    tud_cdc_n_write_flush(0);
}

static inline uint8_t clamp_midi(int n)
{
    if (n < 0)   return 0;
    if (n > 127) return 127;
    return (uint8_t)n;
}

static inline int16_t clamp_i16(int16_t val, int16_t min, int16_t max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

static inline int segments(float pos, int seg_count)
{
    if (pos < 0) return -1;
    return (int)(pos * seg_count);
}

#endif