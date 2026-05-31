/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_GRAPHICS_H
#define TOUCHORD_GRAPHICS_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"

void draw_current_chord(void);
void draw_current_chord_ex(uint8_t octave_span);
void draw_string_top(const char * str);
void draw_string_int_centered(const char * str, int32_t num, uint8_t y);
void draw_string_center(const char * str);
void draw_int_center(int32_t num);

// Inverted (white-on-black) text highlight
void draw_inverted_bar(uint32_t y, uint32_t height);
void draw_string_inverted(uint32_t x, uint32_t y, const char *str);

// Mode label with decorative lines
void draw_mode_label(const char *name);

// Animations
void anim_boot_intro(void);
void anim_transition_wipe(void);

// direction > 0 = slide left (next mode comes in from right),
// direction < 0 = slide right (next mode comes from left),
// direction == 0 = dithered crossfade. */
void anim_transition(const uint8_t *old_fb, const uint8_t *new_fb, int direction);

// Settings helpers
void draw_scroll_indicator(uint8_t sel, uint8_t total);
void draw_settings_footer(void);

// Trill bar position indicator (right-edge strip; visible only while touched).
void draw_trill_indicator(void);

// Centered notification overlay (preset loads, factory reset, etc.).
void trigger_overlay(const char *msg, uint32_t ms);
void draw_overlay(void);

// 8x8 monochrome icons (musical/UI glyphs).
void draw_icon_treble(uint32_t x, uint32_t y);
void draw_octave_dots(uint32_t x, uint32_t y, uint8_t octave, uint8_t span, uint8_t max);

#endif
