/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_COMPOSE_H
#define TOUCHORD_COMPOSE_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"

void compose_start();
void compose_end();
void compose_draw();
void compose_update();
void compose_key_down(uint8_t key);
void compose_key_up(uint8_t key);
void compose_key_up_independent(uint8_t key);
void compose_button_down(uint8_t button);
void compose_button_double_down(uint8_t button);
void compose_button_up(uint8_t button);
void compose_trill_down(float pos, float size);
void compose_trill_up();

#endif