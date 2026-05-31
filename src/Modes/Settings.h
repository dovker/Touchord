/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_SETTINGS_H
#define TOUCHORD_SETTINGS_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"
#include "Types.h"

void settings_start();
void settings_end();
void settings_draw();
void settings_update();
void settings_key_down(uint8_t key);
void settings_key_up(uint8_t key);
void settings_key_up_independent(uint8_t key);
void settings_button_down(uint8_t button);
void settings_button_double_down(uint8_t button);
void settings_button_up(uint8_t button);
void settings_trill_down(float pos, float size);
void settings_trill_up();

extern UINode tree[MAX_UI_NODES];

#endif