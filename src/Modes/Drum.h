#ifndef TOUCHORD_DRUM_H
#define TOUCHORD_DRUM_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"

void drum_start();
void drum_end();
void drum_draw();
void drum_update();
void drum_key_down(uint8_t key);
void drum_key_up(uint8_t key);
void drum_key_up_independent(uint8_t key);
void drum_button_down(uint8_t button);
void drum_button_double_down(uint8_t button);
void drum_button_up(uint8_t button);
void drum_trill_down(float pos, float size);
void drum_trill_up();

#endif