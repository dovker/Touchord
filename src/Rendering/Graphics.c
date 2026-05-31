/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#include "Graphics.h"
#include "ssd1306.h"
#include "font.h"
#include "Globals.h"
#include "Notes/Note.h"
#include "version.h"
#include <stdlib.h>
#include <string.h>

void draw_current_chord_ex(uint8_t octave_span)
{
    uint8_t text_w = strlen(tc_app.chord_name) * 6 - 1;
    ssd1306_draw_string(&tc_disp, 64 - text_w, 24, 2, tc_app.chord_name);

    const char *root  = root_names[tc_app.key[tc_app.current_key].root];
    const char *scale = scale_names[tc_app.key[tc_app.current_key].quality];
    uint8_t rl = strlen(root);
    uint8_t ql = strlen(scale);
    uint8_t header_w = 8 + 2 + rl * 6 + 2 + ql * 6 - 1;
    uint8_t hx = (128 - header_w) / 2;
    draw_icon_treble(hx, 0);
    ssd1306_draw_string(&tc_disp, hx + 10, 1, 1, root);
    ssd1306_draw_string(&tc_disp, hx + 10 + rl * 6, 1, 1, scale);

    if (octave_span == 0) octave_span = 1;
    if (tc_app.octave + octave_span > 8) octave_span = 8 - tc_app.octave;
    draw_octave_dots(128 - 32, 2, tc_app.octave, octave_span, 8);
}

void draw_current_chord(void)
{
    draw_current_chord_ex(1);
}

void draw_string_top(const char *str)
{
    uint8_t text_w = strlen(str) * 6 - 1;
    ssd1306_draw_string(&tc_disp, 64 - text_w / 2, 0, 1, str);
}

void draw_string_int_centered(const char *str, int32_t num, uint8_t y)
{
    char buf[12];
    itoa(num, buf, 10);

    uint8_t sl = strlen(str);
    uint8_t nl = strlen(buf);
    uint8_t pos = 64 - ((sl + nl) * 6) / 2;
    ssd1306_draw_string(&tc_disp, pos, y, 1, str);
    ssd1306_draw_string(&tc_disp, pos + sl * 6, y, 1, buf);
}

void draw_string_center(const char *str)
{
    uint8_t text_w = strlen(str) * 6 - 1;
    ssd1306_draw_string(&tc_disp, 64 - text_w, 24, 2, str);
}

void draw_int_center(int32_t num)
{
    char buf[12];
    itoa(num, buf, 10);
    uint8_t text_w = strlen(buf) * 6 - 1;
    ssd1306_draw_string(&tc_disp, 64 - text_w, 24, 2, buf);
}

void draw_inverted_bar(uint32_t y, uint32_t height)
{
    ssd1306_draw_square(&tc_disp, 0, y, 124, height);
}

void draw_string_inverted(uint32_t x, uint32_t y, const char *str)
{
    uint32_t len = strlen(str);
    ssd1306_draw_square(&tc_disp, x, y, len * 6 + 2, 9);

    uint8_t cols = font_8x5[1];
    uint8_t first = font_8x5[3], last = font_8x5[4];
    for (uint32_t i = 0; i < len; i++) {
        char c = str[i];
        if (c < first || c > last) continue;
        const uint8_t *glyph = &font_8x5[5 + (c - first) * cols];
        for (uint8_t col = 0; col < cols; col++) {
            uint8_t line = glyph[col];
            for (int8_t j = 0; j < 8; j++, line >>= 1)
                if (line & 1)
                    ssd1306_clear_pixel(&tc_disp, x + 1 + i * 6 + col, y + j);
        }
    }
}

void draw_mode_label(const char *name)
{
    uint32_t text_w = strlen(name) * (font_3x6[1] + font_3x6[2]) * 2;
    uint32_t text_x = 64 - text_w / 2;
    uint32_t line_gap = 4;

    if (text_x > line_gap)
        ssd1306_draw_line(&tc_disp, 0, 60, text_x - line_gap, 60);
    ssd1306_draw_string_with_font(&tc_disp, text_x, 54, 2, font_3x6, name);
    uint32_t right_start = text_x + text_w + line_gap;
    if (right_start < 128)
        ssd1306_draw_line(&tc_disp, right_start, 60, 127, 60);
}

void draw_scroll_indicator(uint8_t sel, uint8_t total)
{
    if (total <= 1) return;

    uint32_t track_top = 10;
    uint32_t track_height = 42;
    uint32_t track_x = 126;

    for (uint32_t y = track_top; y < track_top + track_height; y += 4)
        ssd1306_draw_pixel(&tc_disp, track_x, y);

    uint32_t thumb_y = track_top + (sel * (track_height - 4)) / (total - 1);
    ssd1306_draw_square(&tc_disp, track_x - 1, thumb_y, 3, 4);
}

static const uint8_t icon_treble[8] = {
    0b00100000, 0b00100000, 0b00100000, 0b00100000,
    0b00111110, 0b00111111, 0b00011111, 0b00001110,
};

void draw_icon_treble(uint32_t x, uint32_t y)
{
    for (int r = 0; r < 8; r++) {
        uint8_t row = icon_treble[r];
        for (int c = 0; c < 8; c++)
            if (row & (1u << c))
                ssd1306_draw_pixel(&tc_disp, x + c, y + r);
    }
}

void draw_octave_dots(uint32_t x, uint32_t y, uint8_t octave, uint8_t span, uint8_t max)
{
    if (max == 0) return;
    if (span == 0) span = 1;
    uint8_t end = octave + span;
    for (uint8_t i = 0; i < max; i++) {
        uint32_t cx = x + i * 4;
        if (i >= octave && i < end)
            ssd1306_draw_square(&tc_disp, cx, y, 3, 3);
        else
            ssd1306_draw_pixel(&tc_disp, cx + 1, y + 1);
    }
}

void draw_trill_indicator(void)
{
    if (!tc_touch_state || !tc_trill_show) return;

    const int strip_x = 1, strip_w = 5, strip_y = 0, strip_h = 63;

    ssd1306_clear_square(&tc_disp, strip_x, strip_y, strip_w + 1, strip_h + 1);
    ssd1306_draw_empty_square(&tc_disp, strip_x, strip_y, strip_w, strip_h);

    int inner_h = strip_h - 1;
    int inner_w = strip_w - 1;
    int inner_x = strip_x + 1;
    int inner_y = strip_y + 1;

    if (tc_trill_segs == 0) {
        const int block_h = 6;
        int travel = inner_h - block_h;
        if (travel < 0) travel = 0;
        int by = inner_y + (int)(tc_trill_pos * travel);
        ssd1306_draw_square(&tc_disp, inner_x, by, inner_w, block_h);
    } else {
        int n = tc_trill_segs > 16 ? 16 : tc_trill_segs;
        int active = (int)(tc_trill_pos * n);
        if (active >= n) active = n - 1;
        if (active < 0)  active = 0;

        const int gap = 1;
        int block_h = (inner_h - (n - 1) * gap) / n;
        if (block_h < 1) block_h = 1;
        int total_h = n * block_h + (n - 1) * gap;
        int top_y   = inner_y + (inner_h - total_h) / 2;

        for (int i = 0; i < n; i++) {
            int by = top_y + i * (block_h + gap);
            if (i == active)
                ssd1306_draw_square(&tc_disp, inner_x, by, inner_w, block_h);
            else
                ssd1306_draw_empty_square(&tc_disp, inner_x, by, inner_w - 1, block_h - 1);
        }
    }
}

void draw_overlay(void)
{
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((now - tc_overlay_start_ms) >= tc_overlay_dur_ms) return;
    if (tc_overlay_msg[0] == '\0') return;

    int text_w = (int)strlen(tc_overlay_msg) * 6 - 1;
    int box_w = text_w + 16;
    if (box_w < 64)  box_w = 64;
    if (box_w > 124) box_w = 124;
    const int box_h = 22;
    const int box_x = (128 - box_w) / 2;
    const int box_y = (64  - box_h) / 2;

    ssd1306_clear_square(&tc_disp, box_x - 1, box_y - 1, box_w + 2, box_h + 2);
    ssd1306_draw_square(&tc_disp, box_x, box_y, box_w, box_h);

    int text_x = (128 - text_w) / 2;
    int text_y = box_y + (box_h - 8) / 2;
    draw_string_inverted(text_x - 1, text_y - 1, tc_overlay_msg);
}

void trigger_overlay(const char *msg, uint32_t ms)
{
    snprintf(tc_overlay_msg, sizeof tc_overlay_msg, "%s", msg ? msg : "");
    tc_overlay_start_ms = to_ms_since_boot(get_absolute_time());
    tc_overlay_dur_ms   = ms;
}

void draw_settings_footer(void)
{
    ssd1306_draw_line(&tc_disp, 0, 54, 127, 54);
    ssd1306_draw_string_with_font(&tc_disp, 0, 57, 1, font_3x6,
                                  "B0:Exit B1:Back B2:OK T:Scroll");
}

void anim_boot_intro(void)
{
    for (int f = 0; f <= 7; f++) {
        ssd1306_clear(&tc_disp);
        int half = (f * 64) / 7;
        ssd1306_draw_line(&tc_disp, 64 - half, 32, 64 + half, 32);
        ssd1306_show(&tc_disp);
        sleep_ms(30);
    }

    for (int f = 0; f <= 6; f++) {
        ssd1306_clear(&tc_disp);
        int offset = (f * 24) / 6;
        ssd1306_draw_line(&tc_disp, 0, 32 - offset, 127, 32 - offset);
        ssd1306_draw_line(&tc_disp, 0, 32 + offset, 127, 32 + offset);
        ssd1306_show(&tc_disp);
        sleep_ms(30);
    }

    const char *title = "Touchord";
    int title_len = 8;
    int title_x = (128 - title_len * 12) / 2;
    int title_y = 9 + ((56 - 8 - 1) - 16) / 2;
    for (int i = 1; i <= title_len; i++) {
        ssd1306_clear(&tc_disp);
        ssd1306_draw_line(&tc_disp, 0, 8, 127, 8);
        ssd1306_draw_line(&tc_disp, 0, 56, 127, 56);
        char buf[9];
        memcpy(buf, title, i);
        buf[i] = '\0';
        ssd1306_draw_string(&tc_disp, title_x, title_y, 2, buf);
        ssd1306_show(&tc_disp);
        sleep_ms(60);
    }

    ssd1306_clear(&tc_disp);
    ssd1306_draw_line(&tc_disp, 0, 8, 127, 8);
    ssd1306_draw_line(&tc_disp, 0, 56, 127, 56);
    ssd1306_draw_string(&tc_disp, title_x, title_y, 2, title);
    {
        const char *ver = TOUCHORD_VERSION_STRING;
        int adv = font_3x6[1] + font_3x6[2];
        int ver_x = (128 - ((int)strlen(ver) * adv - font_3x6[2])) / 2;
        ssd1306_draw_string_with_font(&tc_disp, ver_x, 46, 1, font_3x6, ver);
    }
    ssd1306_show(&tc_disp);
    sleep_ms(500);

    for (int y = 0; y < 64; y += 2) {
        ssd1306_draw_line(&tc_disp, 0, y, 127, y);
        ssd1306_draw_line(&tc_disp, 0, y + 1, 127, y + 1);
        ssd1306_clear_square(&tc_disp, 0, (y > 3) ? y - 3 : 0, 128, 4);
        ssd1306_show(&tc_disp);
        sleep_ms(10);
    }

    ssd1306_clear(&tc_disp);
    ssd1306_show(&tc_disp);
}

void anim_transition_wipe(void)
{
    for (int y = 0; y < 64; y += 8) {
        ssd1306_draw_square(&tc_disp, 0, y, 128, 8);
        ssd1306_show(&tc_disp);
        sleep_ms(12);
    }
    ssd1306_clear(&tc_disp);
}

#define FB_BYTES (128 * 8)

// 4x4 Bayer dither
static const uint8_t bayer4[4][4] = {
    {  0,  8,  2, 10 },
    { 12,  4, 14,  6 },
    {  3, 11,  1,  9 },
    { 15,  7, 13,  5 },
};

static void compose_slide_horizontal(const uint8_t *old_fb, const uint8_t *new_fb,
                                     int offset, int direction)
{
    if (offset < 0)   offset = 0;
    if (offset > 128) offset = 128;
    for (int page = 0; page < 8; page++) {
        uint8_t *dst = tc_disp.buffer + page * 128;
        if (direction > 0) {
            int keep_old = 128 - offset;
            if (keep_old > 0) memcpy(dst,            old_fb + page * 128 + offset, keep_old);
            if (offset > 0)   memcpy(dst + keep_old, new_fb + page * 128,          offset);
        } else {
            int show_new = offset;
            int show_old = 128 - offset;
            if (show_new > 0) memcpy(dst,            new_fb + page * 128 + (128 - show_new), show_new);
            if (show_old > 0) memcpy(dst + show_new, old_fb + page * 128,                    show_old);
        }
    }
}

static void compose_dither_blend(const uint8_t *old_fb, const uint8_t *new_fb, int threshold)
{
    for (int page = 0; page < 8; page++) {
        for (int x = 0; x < 128; x++) {
            uint8_t ob = old_fb[page * 128 + x];
            uint8_t nb = new_fb[page * 128 + x];
            uint8_t out = 0;
            for (int b = 0; b < 8; b++) {
                int y = page * 8 + b;
                int bit = (bayer4[y & 3][x & 3] < threshold) ? ((nb >> b) & 1)
                                                              : ((ob >> b) & 1);
                out |= (uint8_t)(bit << b);
            }
            tc_disp.buffer[page * 128 + x] = out;
        }
    }
}

void anim_transition(const uint8_t *old_fb, const uint8_t *new_fb, int direction)
{
    if (direction == 0) {
        const int frames = 20;
        for (int t = 1; t <= frames; t++) {
            int thresh = (t * 17) / frames;
            compose_dither_blend(old_fb, new_fb, thresh);
            ssd1306_show(&tc_disp);
            sleep_ms(9);
        }
    } else {
        // Quintic ease-out
        const int frames = 24;
        for (int t = 1; t <= frames; t++) {
            int32_t inv  = frames - t;
            int32_t inv5 = inv * inv;
            inv5 = (inv5 * inv) / frames;
            inv5 = (inv5 * inv) / frames;
            inv5 = (inv5 * inv) / frames;
            int32_t eased = frames - inv5;
            int offset = (int)((eased * 128) / frames);
            compose_slide_horizontal(old_fb, new_fb, offset, direction);
            ssd1306_show(&tc_disp);
            sleep_ms(6);
        }
    }
    memcpy(tc_disp.buffer, new_fb, FB_BYTES);
    ssd1306_show(&tc_disp);
}
