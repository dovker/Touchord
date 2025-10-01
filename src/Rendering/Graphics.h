#ifndef TOUCHORD_GRAPHICS_H
#define TOUCHORD_GRAPHICS_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"
#include "ssd1306.h"
#include "Globals.h"
#include "Notes/Note.h"

static void draw_current_chord()
{
    uint32_t len = strlen(tc_app.chord_name);
    uint8_t text_w = len * 6 - 1;
    ssd1306_draw_string(&tc_disp, 64 - text_w, 24, 2, tc_app.chord_name);

    uint8_t rootLen = strlen(tc_app.key[tc_app.current_key].root);
    uint8_t qualLen = strlen(scale_names[tc_app.key[tc_app.current_key].quality]);
    text_w = (6 + rootLen + qualLen) * 6 - 1;
    uint8_t pos = 64-text_w/2;
    ssd1306_draw_string(&tc_disp, pos, 0, 1, "Key:");
    ssd1306_draw_string(&tc_disp, pos + 25, 0, 1, tc_app.key[tc_app.current_key].root);
    ssd1306_draw_string(&tc_disp, pos + 30 + rootLen * 6, 0, 1, scale_names[tc_app.key[tc_app.current_key].quality]);
}

#endif