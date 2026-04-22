#ifndef TOUCHORD_GRAPHICS_H
#define TOUCHORD_GRAPHICS_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"
#include "ssd1306.h"
#include "Globals.h"
#include "Notes/Note.h"
#include "tusb.h"
#include <stdlib.h>

static void draw_current_chord()
{
    static const char* root_names[17] = {
        "C", "C#", "Db", "D", "D#", "Eb", "E", "F", "F#", "Gb", "G", "G#", "Ab", "A", "A#", "Bb", "B"
    };
    uint32_t len = strlen(tc_app.chord_name);
    uint8_t text_w = len * 6 - 1;
    ssd1306_draw_string(&tc_disp, 64 - text_w, 24, 2, tc_app.chord_name);

    uint8_t rootLen = strlen(root_names[tc_app.key[tc_app.current_key].root]);
    uint8_t qualLen = strlen(scale_names[tc_app.key[tc_app.current_key].quality]);
    text_w = (6 + rootLen + qualLen) * 6 - 1;
    uint8_t pos = 64-text_w/2;
    ssd1306_draw_string(&tc_disp, pos, 0, 1, "Key:");
    ssd1306_draw_string(&tc_disp, pos + 25, 0, 1, root_names[tc_app.key[tc_app.current_key].root]);
    ssd1306_draw_string(&tc_disp, pos + 30 + rootLen * 6, 0, 1, scale_names[tc_app.key[tc_app.current_key].quality]);
}

static void draw_string_top(const char * str)
{
    uint32_t len = strlen(str);
    uint8_t text_w = len * 6 - 1;
    ssd1306_draw_string(&tc_disp, 64 - text_w/2, 0, 1, str);
}

static void draw_string_int_centered(const char * str, int32_t num, uint8_t y)
{
    char buf[12];
    itoa(num, buf, 10);

    uint8_t rootLen = strlen(str);
    uint8_t intLen = strlen(buf);
    
    int text_w = (rootLen + intLen) * 6;
    uint8_t pos = 64-text_w/2;
    ssd1306_draw_string(&tc_disp, pos, y, 1, str);
    ssd1306_draw_string(&tc_disp, pos + rootLen * 6, y, 1, buf);
}

static void draw_string_center(const char * str)
{
    uint32_t len = strlen(str);
    uint8_t text_w = len * 6 - 1;
    ssd1306_draw_string(&tc_disp, 64 - text_w, 24, 2, str);
}

static void draw_int_center(int32_t num)
{
    char buf[12];
    itoa(num, buf, 10);
    uint32_t len = strlen(buf);
    uint8_t text_w = len * 6 - 1;
    ssd1306_draw_string(&tc_disp, 64 - text_w, 24, 2, buf);
}

static void draw_debug_overlay(void)
{
    char buf[24];
    int x = 74;

    if (!tc_app.debug_overlay) {
        return;
    }

    snprintf(
        buf,
        sizeof(buf),
        "K%d:%lu",
        tc_debug_last_key_down,
        (unsigned long)tc_debug_key_down_count
    );
    ssd1306_draw_string(&tc_disp, x, 8, 1, buf);

    snprintf(
        buf,
        sizeof(buf),
        "N%d:%lu",
        tc_debug_last_output_note,
        (unsigned long)tc_debug_output_note_count
    );
    ssd1306_draw_string(&tc_disp, x, 16, 1, buf);

    snprintf(
        buf,
        sizeof(buf),
        "M%d:%lu",
        tc_debug_last_usb_midi_note,
        (unsigned long)tc_debug_usb_midi_note_count
    );
    ssd1306_draw_string(&tc_disp, x, 24, 1, buf);

    snprintf(
        buf,
        sizeof(buf),
        "U%d:%d%d",
        tc_app.output_mode,
        tud_mounted() ? 1 : 0,
        tud_midi_mounted() ? 1 : 0
    );
    ssd1306_draw_string(&tc_disp, x, 32, 1, buf);
}

#endif
