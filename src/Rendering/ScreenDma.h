/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_SCREEN_DMA_H
#define TOUCHORD_SCREEN_DMA_H

#include <stdbool.h>
#include "ssd1306.h"

/* Async screen send. In dual-bus mode (TOUCHORD_DUAL_I2C=1) the framebuffer
   is pushed via DMA so the caller can do other work — e.g. polling the
   trill bar on the *other* I2C — while ~9 ms of bytes drain to the screen.
   In single-bus mode there's nothing useful to do during the send (trill is
   on the same bus), so these calls fall through to a synchronous
   ssd1306_show.

   Usage:
       screen_show_async(&tc_disp);
       while (screen_show_busy()) {
           // run other work that doesn't touch the screen's I2C bus
       }
       screen_show_wait();
*/

void screen_dma_init(void);
void screen_show_async(ssd1306_t *p);
bool screen_show_busy(void);
void screen_show_wait(void);

#endif
