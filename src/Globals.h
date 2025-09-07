#ifndef TOUCHORD_GLOBALS_H
#define TOUCHORD_GLOBALS_H
#include "Defines.h"
#include "Types.h"

#include "Trill.h"
#include "ssd1306.h"
#include "font.h"

extern TouchordSettings tc_app;
 
extern void (*tc_draw)();
extern void (*tc_update)();
extern void (*tc_key_down)(uint8_t);
extern void (*tc_key_up)(uint8_t);
extern void (*tc_button_down)(uint8_t);
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
extern bool tc_touch_state;
 
extern uint8_t tc_last_key;
extern uint8_t tc_last_control;



#endif