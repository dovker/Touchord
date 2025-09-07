#include "Globals.h"

TouchordSettings tc_app = {
    {{"C", "min"}, {"E", "min"}, {"D", "maj"}}, 0,
    DEFAULT_OCTAVE, DEFAULT_EXTENSIONS, DEFAULT_INVERSION, DEFAULT_VELOCITY, 
    TOUCHORD_COMPOSE, DEFAULT_OCTAVE_COUNT, 0,
    {0, 0, 0, 0, 0, 0}, {'\0'}, CHORD_DEFAULT,
    DEFAULT_EXTENSIONS, MIDI_CHANNEL
};

void (*tc_draw)() = NULL;
void (*tc_update)() = NULL;
void (*tc_key_down)(uint8_t) = NULL;
void (*tc_key_up)(uint8_t) = NULL;
void (*tc_button_down)(uint8_t) = NULL;
void (*tc_button_up)(uint8_t) = NULL;
void (*tc_trill_down)(float, float) = NULL;
void (*tc_trill_up)() = NULL;
TouchordMode tc_prev_mode = TOUCHORD_COMPOSE;

bool tc_running = true;
bool tc_trigger_bootsel = false;

TrillBar tc_bar;
ssd1306_t tc_disp;

bool tc_key_states[NUM_KEYS];
bool tc_control_states[NUM_CONTROLS];
bool tc_touch_state = false;

uint8_t tc_last_key;
uint8_t tc_last_control;