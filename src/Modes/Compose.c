/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#include "Compose.h"
#include "Globals.h"
#include "Helper.h"
#include "Sync.h"
#include "IO/Midi.h"
#include "Notes/Note.h"
#include "Rendering/Graphics.h"
#include "pico/critical_section.h"
#include <string.h>
#include <stdio.h>

uint8_t compose_last_degree = -1;
uint8_t compose_last_extension;
uint8_t compose_extension;

static int prev_seg = -1;
static int trill_jazz_seg = -1;
static char tc_compose_tooltip[16] = {'\0'};

static const char *jazz_label[4] = { "Tritone", "Sec.Dom", "Parallel", "Dim7" };
static const char *deg_label[5]  = { "Ext 6", "Ext 5", "Ext 4", "Ext 4 Par", "Ext 3 Par" };

static int compose_segment_count(void)
{
    switch (tc_app.compose_type) {
        case COMPOSE_INV:  return tc_app.extension_count;
        case COMPOSE_JAZZ: return 4;
        default:           return 5;
    }
}

static void build_current_chord(int key)
{
    if (tc_app.compose_type == COMPOSE_JAZZ && trill_jazz_seg >= 0) {
        build_jazz_chord(tc_app.key[tc_app.current_key], tc_app.octave,
                         key, trill_jazz_seg,
                         compose_extension, tc_app.chord, tc_app.chord_name);
    } else {
        build_chord(tc_app.key[tc_app.current_key], tc_app.octave, key, tc_app.degree,
                    compose_extension, tc_app.inversion,
                    tc_app.chord, tc_app.chord_name);
    }
}

static const ModeHandlers compose_handlers = TC_MODE_HANDLERS(compose);

void compose_start()
{
    tc_bind_handlers(&compose_handlers);

    compose_last_extension = tc_app.extension_count;
    compose_extension      = tc_app.extension_count;
    tc_trill_segs          = compose_segment_count();
    tc_trill_show          = true;
    tc_compose_tooltip[0]  = '\0';
    prev_seg               = -1;
    trill_jazz_seg         = -1;
}

void compose_end()
{
    send_midi_chord(tc_app.channel, NOTE_OFF, tc_app.chord, compose_last_extension, tc_app.velocity);
    tc_app.chord_name[0]  = '\0';
    tc_compose_tooltip[0] = '\0';
}

void compose_draw()
{
    draw_current_chord();
    if (tc_compose_tooltip[0]) {
        int adv = font_3x6[1] + font_3x6[2];
        int w   = (int)strlen(tc_compose_tooltip) * adv - font_3x6[2];
        ssd1306_draw_string_with_font(&tc_disp, (128 - w) / 2, 42, 1,
                                      font_3x6, tc_compose_tooltip);
    }
    draw_mode_label("Compose");
}

void compose_update() {}

void compose_key_down(uint8_t key)
{
    critical_section_enter_blocking(&tc_app_lock);
    compose_last_degree = key;

    uint8_t prev_chord[MAX_CHORD];
    memcpy(prev_chord, tc_app.chord, MAX_CHORD);

    build_current_chord(key);
    if (tc_app.compose_voice_lead)
        apply_voice_leading(tc_app.chord, compose_extension, prev_chord);

    send_midi_chord_diff(prev_chord, tc_app.chord, tc_app.channel, tc_app.velocity, true);
    compose_last_extension = compose_extension;

    prev_seg = -1;
    critical_section_exit(&tc_app_lock);
}

void compose_key_up(uint8_t key)
{
    if (compose_last_degree != key) return;

    critical_section_enter_blocking(&tc_app_lock);
    compose_last_degree = -1;
    if (!tc_app.compose_sustain) {
        send_midi_chord(tc_app.channel, NOTE_OFF, tc_app.chord, compose_last_extension, tc_app.velocity);
        build_chord(tc_app.key[tc_app.current_key], tc_app.octave, 0, CHORD_DEFAULT,
                    0, tc_app.inversion, tc_app.chord, tc_app.chord_name);
        tc_app.chord_name[0]  = '\0';
        tc_compose_tooltip[0] = '\0';
    }
    critical_section_exit(&tc_app_lock);
}

void compose_key_up_independent(uint8_t key) {}

void compose_button_down(uint8_t button)
{
    switch (button) {
        case 0: tc_app_set_mode(TOUCHORD_PERFORM); break;
        case 1: if (tc_app.octave > 0) tc_app.octave--; break;
        case 2: if (tc_app.octave < 7) tc_app.octave++; break;
        default:
            if (button > 2) tc_app.current_key = button - 3;
            break;
    }
}

void compose_button_double_down(uint8_t button) {}
void compose_button_up(uint8_t button) {}

static int update_trill_state(float pos)
{
    int seg = segments(pos, compose_segment_count());
    switch (tc_app.compose_type) {
        case COMPOSE_DEGREE:
            if (seg < 0) seg = 0;
            if (seg > 4) seg = 4;
            switch (seg) {
                case 0: compose_extension = 6; tc_app.degree = CHORD_DEFAULT;  break;
                case 1: compose_extension = 5; tc_app.degree = CHORD_DEFAULT;  break;
                case 2: compose_extension = 4; tc_app.degree = CHORD_DEFAULT;  break;
                case 3: compose_extension = 4; tc_app.degree = CHORD_PARALLEL; break;
                case 4: compose_extension = 3; tc_app.degree = CHORD_PARALLEL; break;
            }
            snprintf(tc_compose_tooltip, sizeof tc_compose_tooltip, "%s", deg_label[seg]);
            break;
        case COMPOSE_INV: {
            int s = segments(pos, tc_app.extension_count);
            if (s < 0) s = 0;
            if (s >= tc_app.extension_count) s = tc_app.extension_count - 1;
            seg = (tc_app.extension_count - 1) - s;
            tc_app.inversion = seg;
            snprintf(tc_compose_tooltip, sizeof tc_compose_tooltip, "Inv %d", seg);
        } break;
        case COMPOSE_JAZZ:
            if (seg < 0) seg = 0;
            if (seg > 3) seg = 3;
            trill_jazz_seg = seg;
            snprintf(tc_compose_tooltip, sizeof tc_compose_tooltip, "%s", jazz_label[seg]);
            break;
    }
    return seg;
}

void compose_trill_down(float pos, float size)
{
    critical_section_enter_blocking(&tc_app_lock);

    int seg = update_trill_state(pos);

    if (tc_app.chord_name[0] == '\0' || prev_seg == seg) {
        prev_seg = seg;
        critical_section_exit(&tc_app_lock);
        return;
    }

    compose_last_extension = compose_extension;

    uint8_t prev_chord[MAX_CHORD];
    memcpy(prev_chord, tc_app.chord, MAX_CHORD);

    build_current_chord(compose_last_degree);
    if (tc_app.compose_voice_lead && tc_app.compose_type != COMPOSE_INV)
        apply_voice_leading(tc_app.chord, compose_extension, prev_chord);

    send_midi_chord_diff(prev_chord, tc_app.chord, tc_app.channel, tc_app.velocity,
               tc_app.compose_voice_lead);
    compose_last_extension = compose_extension;
    prev_seg = seg;
    critical_section_exit(&tc_app_lock);
}

void compose_trill_up()
{
    critical_section_enter_blocking(&tc_app_lock);
    prev_seg              = -1;
    trill_jazz_seg        = -1;
    tc_compose_tooltip[0] = '\0';

    compose_last_extension = compose_extension;
    compose_extension      = tc_app.extension_count;
    tc_app.degree          = CHORD_DEFAULT;
    tc_app.inversion       = 0;

    if (compose_last_degree != (uint8_t)-1 && tc_app.chord_name[0] != '\0') {
        uint8_t prev_chord[MAX_CHORD];
        memcpy(prev_chord, tc_app.chord, MAX_CHORD);
        build_chord(tc_app.key[tc_app.current_key], tc_app.octave,
                    compose_last_degree, tc_app.degree,
                    compose_extension, tc_app.inversion,
                    tc_app.chord, tc_app.chord_name);
        if (tc_app.compose_voice_lead)
            apply_voice_leading(tc_app.chord, compose_extension, prev_chord);
        send_midi_chord_diff(prev_chord, tc_app.chord, tc_app.channel, tc_app.velocity,
                   tc_app.compose_voice_lead);
        compose_last_extension = compose_extension;
    }
    critical_section_exit(&tc_app_lock);
}
