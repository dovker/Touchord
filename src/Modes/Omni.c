/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#include "Omni.h"
#include "Globals.h"
#include "Helper.h"
#include "Sync.h"
#include "IO/Midi.h"
#include "Notes/Note.h"
#include "Rendering/Graphics.h"
#include "pico/critical_section.h"

uint8_t omniLastNote1 = 0;
uint8_t omniLastNote2 = 0;
uint8_t omniPrevSegment = -1;
uint8_t playingRoot = 0;
uint8_t playingFifth = 0;

static int8_t omni_active_key = -1;

static const ModeHandlers omni_handlers = TC_MODE_HANDLERS(omni);

void omni_start()
{
    tc_bind_handlers(&omni_handlers);
    tc_app.extension_count = 3;
    tc_trill_segs = tc_app.extension_count * tc_app.octave_count;
    tc_trill_show = true;
}

void omni_end()
{
    send_midi_note(tc_app.channel, NOTE_OFF, omniLastNote1, tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_OFF, omniLastNote2, tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_OFF, playingRoot,   tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_OFF, playingFifth,  tc_app.velocity);
    tc_app.chord_name[0] = '\0';
    tc_app.extension_count = DEFAULT_EXTENSIONS;
}

void omni_draw()
{
    tc_trill_segs = tc_app.extension_count * tc_app.octave_count;
    draw_current_chord_ex(tc_app.octave_count);
    draw_mode_label("Omni");
}

void omni_update() {}

void omni_key_down(uint8_t key)
{
    critical_section_enter_blocking(&tc_app_lock);
    send_midi_note(tc_app.channel, NOTE_OFF, playingRoot,  tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_OFF, playingFifth, tc_app.velocity);

    build_chord(tc_app.key[tc_app.current_key], tc_app.octave, key, tc_app.degree,
                tc_app.extension_count, tc_app.inversion, tc_app.chord, tc_app.chord_name);

    // Omnichord-style fifth
    playingRoot  = clamp_midi((int)tc_app.chord[0] - 12);
    playingFifth = clamp_midi((int)tc_app.chord[1] - 12);
    send_midi_note(tc_app.channel, NOTE_ON, playingRoot,  tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_ON, playingFifth, tc_app.velocity);
    omni_active_key = (int8_t)key;

    omniPrevSegment = -1;
    critical_section_exit(&tc_app_lock);
}

void omni_key_up(uint8_t key)
{
    if (omni_active_key != (int8_t)key) return;
    critical_section_enter_blocking(&tc_app_lock);
    omni_active_key = -1;
    send_midi_note(tc_app.channel, NOTE_OFF, playingRoot,  tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_OFF, playingFifth, tc_app.velocity);
    critical_section_exit(&tc_app_lock);
}

void omni_key_up_independent(uint8_t key) {}

void omni_button_down(uint8_t button)
{
    switch (button) {
        case 0: tc_app_set_mode(TOUCHORD_DRUM); break;
        case 1: if (tc_app.octave > 1) tc_app.octave--; break;
        case 2: if (tc_app.octave < 7) tc_app.octave++; break;
        case 3: tc_app.current_key = 0; break;
        case 4: tc_app.current_key = 1; break;
        case 5: tc_app.current_key = 2; break;
    }
}

void omni_button_double_down(uint8_t button)
{
    critical_section_enter_blocking(&tc_app_lock);
    switch (button) {
        case 1: if (tc_app.octave_count > 1) tc_app.octave_count--; break;
        case 2: if (tc_app.octave_count < 4) tc_app.octave_count++; break;
    }
    critical_section_exit(&tc_app_lock);
}

void omni_button_up(uint8_t button) {}

void omni_trill_down(float pos, float size)
{
    critical_section_enter_blocking(&tc_app_lock);
    int ext     = tc_app.extension_count;
    int max_seg = ext * tc_app.octave_count;
    int seg     = max_seg - segments(pos, max_seg);
    if (seg < 0)       seg = 0;
    if (seg > max_seg) seg = max_seg;

    uint8_t note1 = chord_note_at(tc_app.chord, seg,     ext);
    uint8_t note2 = chord_note_at(tc_app.chord, seg + 1, ext);
    if (note2 < note1) note2 = clamp_midi((int)note2 + 12);

    if (!tc_touch_state) {
        send_midi_note(tc_app.channel, NOTE_ON, note1, tc_app.velocity);
        send_midi_note(tc_app.channel, NOTE_ON, note2, tc_app.velocity);
    } else if (seg != omniPrevSegment) {
        if (omniLastNote1 != note1 && omniLastNote1 != note2)
            send_midi_note(tc_app.channel, NOTE_OFF, omniLastNote1, tc_app.velocity);
        if (omniLastNote2 != note1 && omniLastNote2 != note2)
            send_midi_note(tc_app.channel, NOTE_OFF, omniLastNote2, tc_app.velocity);
        if (note1 != omniLastNote1 && note1 != omniLastNote2)
            send_midi_note(tc_app.channel, NOTE_ON, note1, tc_app.velocity);
        if (note2 != omniLastNote1 && note2 != omniLastNote2)
            send_midi_note(tc_app.channel, NOTE_ON, note2, tc_app.velocity);
    }

    omniLastNote1   = note1;
    omniLastNote2   = note2;
    omniPrevSegment = seg;
    critical_section_exit(&tc_app_lock);
}

void omni_trill_up()
{
    critical_section_enter_blocking(&tc_app_lock);
    send_midi_note(tc_app.channel, NOTE_OFF, omniLastNote1, tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_OFF, omniLastNote2, tc_app.velocity);
    critical_section_exit(&tc_app_lock);
}
