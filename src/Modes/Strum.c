/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#include "Strum.h"
#include "Globals.h"
#include "Helper.h"
#include "Sync.h"
#include "IO/Midi.h"
#include "Notes/Note.h"
#include "Rendering/Graphics.h"
#include "pico/critical_section.h"

uint8_t lastNote = 0;
uint8_t prevSegment = -1;

static void rebuild_chord_for(uint8_t key)
{
    build_chord(tc_app.key[tc_app.current_key], tc_app.octave, key, tc_app.degree,
                tc_app.extension_count, tc_app.inversion,
                tc_app.chord, tc_app.chord_name);
}

static const ModeHandlers strum_handlers = TC_MODE_HANDLERS(strum);

void strum_start()
{
    tc_bind_handlers(&strum_handlers);
    tc_trill_segs = tc_app.extension_count * tc_app.octave_count;
    tc_trill_show = true;
}

void strum_end()
{
    send_midi_note(tc_app.channel, NOTE_OFF, lastNote, tc_app.velocity);
    tc_app.chord_name[0] = '\0';
}

void strum_draw()
{
    tc_trill_segs = tc_app.extension_count * tc_app.octave_count;
    draw_current_chord_ex(tc_app.octave_count);
    draw_mode_label("Strum");
}

void strum_update() {}

void strum_key_down(uint8_t key)
{
    critical_section_enter_blocking(&tc_app_lock);
    rebuild_chord_for(key);
    prevSegment = -1;
    critical_section_exit(&tc_app_lock);
}
void strum_key_up(uint8_t key)               {}
void strum_key_up_independent(uint8_t key)   {}

void strum_button_down(uint8_t button)
{
    switch (button) {
        case 0: tc_app_set_mode(TOUCHORD_OMNI); break;
        case 1:
            if (tc_app.octave > 1) {
                critical_section_enter_blocking(&tc_app_lock);
                tc_app.octave--;
                rebuild_chord_for(tc_last_key);
                critical_section_exit(&tc_app_lock);
            }
            break;
        case 2:
            if (tc_app.octave < 7) {
                critical_section_enter_blocking(&tc_app_lock);
                tc_app.octave++;
                rebuild_chord_for(tc_last_key);
                critical_section_exit(&tc_app_lock);
            }
            break;
        case 3: tc_app.current_key = 0; break;
        case 4: tc_app.current_key = 1; break;
        case 5: tc_app.current_key = 2; break;
    }
}

void strum_button_double_down(uint8_t button)
{
    critical_section_enter_blocking(&tc_app_lock);
    switch (button) {
        case 1: if (tc_app.extension_count > 1) tc_app.extension_count--; break;
        case 2: if (tc_app.extension_count < 4) tc_app.extension_count++; break;
    }
    critical_section_exit(&tc_app_lock);
}

void strum_button_up(uint8_t button) {}

void strum_trill_down(float pos, float size)
{
    critical_section_enter_blocking(&tc_app_lock);
    int ext     = tc_app.extension_count;
    int max_seg = ext * tc_app.octave_count;
    int seg = max_seg - segments(pos, max_seg);
    if (seg < 0)       seg = 0;
    if (seg > max_seg) seg = max_seg;

    if (!tc_touch_state) {
        lastNote = chord_note_at(tc_app.chord, seg, ext);
        send_midi_note(tc_app.channel, NOTE_ON, lastNote, tc_app.velocity);
    } else if (seg != prevSegment) {
        send_midi_note(tc_app.channel, NOTE_OFF, lastNote, tc_app.velocity);
        lastNote = chord_note_at(tc_app.chord, seg, ext);
        send_midi_note(tc_app.channel, NOTE_ON, lastNote, tc_app.velocity);
    }
    prevSegment = seg;
    critical_section_exit(&tc_app_lock);
}

void strum_trill_up()
{
    critical_section_enter_blocking(&tc_app_lock);
    send_midi_note(tc_app.channel, NOTE_OFF, lastNote, tc_app.velocity);
    critical_section_exit(&tc_app_lock);
}
