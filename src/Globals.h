#ifndef TOUCHORD_GLOBALS_H
#define TOUCHORD_GLOBALS_H
#include "Defines.h"
#include "Types.h"

#include "IO/Trill.h"
#include "ssd1306.h"
#include "font.h"

#define SETTINGS_MAGIC  0x544F5543u

static const TouchordSettings tc_app_default = {
    SETTINGS_MAGIC,
    {{"C", SCALE_MAJOR}, {"E", SCALE_MINOR}, {"D", SCALE_CUSTOM0}}, 0,
    DEFAULT_OCTAVE, DEFAULT_EXTENSIONS, DEFAULT_INVERSION, DEFAULT_VELOCITY, 
    TOUCHORD_COMPOSE, DEFAULT_OCTAVE_COUNT, 0,
    {0, 0, 0, 0, 0, 0}, {'\0'}, CHORD_DEFAULT,
    DEFAULT_EXTENSIONS, MIDI_CHANNEL
};
extern TouchordSettings tc_app;
extern TouchordSettings tc_app_working;
 
extern void (*tc_draw)();
extern void (*tc_update)();
extern void (*tc_key_down)(uint8_t);
extern void (*tc_key_up)(uint8_t);
extern void (*tc_button_down)(uint8_t);
extern void (*tc_button_double_down)(uint8_t);
extern void (*tc_button_up)(uint8_t);
extern void (*tc_trill_down)(float, float);
extern void (*tc_trill_up)();
extern TouchordMode tc_prev_mode;
 
extern bool tc_running;
extern bool tc_trigger_bootsel;
 
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

#endif