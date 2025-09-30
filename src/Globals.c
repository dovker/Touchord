#include "Globals.h"

TouchordSettings tc_app = tc_app_default;
TouchordSettings tc_app_working = tc_app_default;

void (*tc_draw)() = NULL;
void (*tc_update)() = NULL;
void (*tc_key_down)(uint8_t) = NULL;
void (*tc_key_up)(uint8_t) = NULL;
void (*tc_button_down)(uint8_t) = NULL;
void (*tc_button_double_down)(uint8_t) = NULL;
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
bool tc_control_double_click[NUM_CONTROLS] = {false, true, true, false, false, false};
bool tc_touch_state = false;

uint64_t tc_time_last_control = 0;
uint8_t tc_last_control_clicks = 0;
uint8_t tc_last_key = 0;
uint8_t tc_last_control = 0;
