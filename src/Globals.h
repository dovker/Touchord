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
    {NOTE_C, SCALE_MAJOR},
    DEFAULT_OCTAVE, DEFAULT_EXTENSIONS, DEFAULT_INVERSION, DEFAULT_VELOCITY, 
    TOUCHORD_COMPOSE, DEFAULT_OCTAVE_COUNT, 0,
    {0, 0, 0, 0, 0, 0}, {'\0'}, CHORD_DEFAULT,
    DEFAULT_EXTENSIONS, MIDI_CHANNEL, MIDI_TRS_A, TOUCHORD_OUTPUT_BOTH, false,

    //Custom Scales
    {
        {CHORD_MAJOR, CHORD_MAJOR, CHORD_MAJOR, CHORD_MAJOR, CHORD_MAJOR, CHORD_MAJOR, CHORD_MAJOR}, // Custom0
        {CHORD_MINOR, CHORD_MINOR, CHORD_MINOR, CHORD_MINOR, CHORD_MINOR, CHORD_MINOR, CHORD_MINOR},
        {CHORD_DOMINANT, CHORD_DOMINANT, CHORD_DOMINANT, CHORD_DOMINANT, CHORD_DOMINANT, CHORD_DOMINANT, CHORD_DOMINANT},
        {CHORD_DIM, CHORD_DIM, CHORD_DIM, CHORD_DIM, CHORD_DIM, CHORD_DIM, CHORD_DIM} // Custom3
    },
    {
        { 0, 1, 2, 3, 4, 5, 6 }, // C0
        { 0, 1, 2, 3, 4, 5, 6 }, // C1
        { 0, 1, 2, 3, 4, 5, 6 }, // C2
        { 0, 1, 2, 3, 4, 5, 6 } // C3
    },

    //Mode Customization
    COMPOSE_DEGREE, false,

    MIDI_CUTOFF, MIDI_MOD, DEFAULT_CUTOFF, 0, false, true,
};
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
 
extern bool tc_running;
extern bool tc_trigger_bootsel;
extern bool tc_trigger_panic;
 
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
extern int8_t tc_debug_last_key_down;
extern int8_t tc_debug_last_output_note;
extern int8_t tc_debug_last_usb_midi_note;
extern uint32_t tc_debug_key_down_count;
extern uint32_t tc_debug_output_note_count;
extern uint32_t tc_debug_usb_midi_note_count;

#endif
