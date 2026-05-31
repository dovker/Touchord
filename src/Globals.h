/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_GLOBALS_H
#define TOUCHORD_GLOBALS_H
#include "Defines.h"
#include "Types.h"

#include "IO/Trill.h"
#include "ssd1306.h"
#include "font.h"

/* MAGIC tags a flash blob as Touchord settings; SCHEMA_VERSION tags the field layout. Versioning policy:
     - Adding a new field at the END of TouchordSettings: leave both alone.
     flash_load_preset overlays stored bytes onto tc_app_default, so the new field keeps its default value when reading older presets.
     - Removing/reordering/resizing a field: bump SETTINGS_SCHEMA_VERSION.
     Old presets are rejected (load returns false) so tc_app stays at defaults instead of misinterpreting bytes.
     - Changing what "Touchord settings" means at the file-format level: bump SETTINGS_MAGIC. */
#define SETTINGS_MAGIC          0x544F5544u
#define SETTINGS_SCHEMA_VERSION 1

extern const TouchordSettings tc_app_default;
extern TouchordSettings tc_app;
extern TouchordSettings tc_app_working;

extern void (*tc_draw)();
extern void (*tc_update)();
extern void (*tc_key_down)(uint8_t);
extern void (*tc_key_up)(uint8_t);
extern void (*tc_key_up_independent)(uint8_t);
extern void (*tc_button_down)(uint8_t);
extern void (*tc_button_double_down)(uint8_t);
extern void (*tc_button_up)(uint8_t);
extern void (*tc_trill_down)(float, float);
extern void (*tc_trill_up)();
extern TouchordMode tc_prev_mode;

typedef struct {
    void (*draw)(void);
    void (*update)(void);
    void (*key_down)(uint8_t);
    void (*key_up)(uint8_t);
    void (*key_up_independent)(uint8_t);
    void (*button_down)(uint8_t);
    void (*button_double_down)(uint8_t);
    void (*button_up)(uint8_t);
    void (*trill_down)(float, float);
    void (*trill_up)(void);
} ModeHandlers;

#define TC_MODE_HANDLERS(prefix) {                          \
    .draw               = prefix##_draw,                    \
    .update             = prefix##_update,                  \
    .key_down           = prefix##_key_down,                \
    .key_up             = prefix##_key_up,                  \
    .key_up_independent = prefix##_key_up_independent,      \
    .button_down        = prefix##_button_down,             \
    .button_double_down = prefix##_button_double_down,      \
    .button_up          = prefix##_button_up,               \
    .trill_down         = prefix##_trill_down,              \
    .trill_up           = prefix##_trill_up,                \
}

void tc_bind_handlers(const ModeHandlers *h);

extern bool tc_running;
extern volatile bool tc_trigger_bootsel;

extern TrillBar tc_bar;
extern ssd1306_t tc_disp;

extern bool tc_key_states[NUM_KEYS];
extern bool tc_control_states[NUM_CONTROLS];
extern bool tc_control_double_click[NUM_CONTROLS];
extern bool tc_touch_state;

extern uint64_t tc_time_last_control;
extern uint8_t tc_last_control_clicks;
extern uint8_t tc_last_key;
extern uint8_t tc_last_control;

extern float   tc_trill_pos;
extern uint8_t tc_trill_segs;
extern bool    tc_trill_show;

extern char     tc_overlay_msg[24];
extern uint32_t tc_overlay_start_ms;
extern uint32_t tc_overlay_dur_ms;

#endif