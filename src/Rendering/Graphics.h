#ifndef TOUCHORD_GRAPHICS_H
#define TOUCHORD_GRAPHICS_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"
#include "ssd1306.h"
#include "Globals.h"
#include "Notes/Note.h"
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

#endif