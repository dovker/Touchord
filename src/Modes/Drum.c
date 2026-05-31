/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#include "Drum.h"
#include "Globals.h"
#include "Sync.h"
#include "IO/Midi.h"
#include "Rendering/Graphics.h"

uint8_t drum_velocity = DEFAULT_VELOCITY;
uint8_t last_note     = -1;

static const ModeHandlers drum_handlers = TC_MODE_HANDLERS(drum);

void drum_start()
{
    tc_bind_handlers(&drum_handlers);
    drum_velocity = tc_app.velocity;
    tc_trill_segs = 0;
    tc_trill_show = true;
}

void drum_end()
{
    send_midi_note(tc_app.channel, NOTE_OFF, last_note, drum_velocity);
}

void drum_draw()
{
    draw_mode_label("Drum");
    draw_octave_dots(128 - 32, 2, tc_app.octave, 1, 8);

    const int pad_w = 12, pad_h = 14, gap = 3;
    const int total_w = NUM_KEYS * pad_w + (NUM_KEYS - 1) * gap;
    const int start_x = (128 - total_w) / 2;
    const int pad_y   = 14;
    for (int i = 0; i < NUM_KEYS; i++) {
        int px = start_x + i * (pad_w + gap);
        if (!tc_key_states[i]) ssd1306_draw_square(&tc_disp, px, pad_y, pad_w, pad_h);
        else                   ssd1306_draw_empty_square(&tc_disp, px, pad_y, pad_w, pad_h);
    }

    draw_string_int_centered("Velocity: ", drum_velocity, 36);
}

void drum_update() {}

void drum_key_down(uint8_t key)
{
    uint8_t note = 12 * (tc_app.octave + 1) + key;
    send_midi_note(tc_app.channel, NOTE_ON, note, drum_velocity);
    last_note = note;
}

void drum_key_up(uint8_t key) {}

void drum_key_up_independent(uint8_t key)
{
    send_midi_note(tc_app.channel, NOTE_OFF, 12 * (tc_app.octave + 1) + key, 0);
    last_note = -1;
}

void drum_button_down(uint8_t button)
{
    switch (button) {
        case 0: tc_app_set_mode(TOUCHORD_SETTINGS); break;
        case 1: if (tc_app.octave > 0) tc_app.octave--; break;
        case 2: if (tc_app.octave < 7) tc_app.octave++; break;
        default:
            if (button > 2) tc_app.current_key = button - 3;
            break;
    }
}

void drum_button_double_down(uint8_t button) {}
void drum_button_up(uint8_t button) {}

void drum_trill_down(float pos, float size)
{
    drum_velocity = 127 - (uint8_t)(pos * 127);
}

void drum_trill_up() {}
