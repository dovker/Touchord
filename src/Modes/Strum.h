#ifndef TOUCHORD_STRUM_H
#define TOUCHORD_STRUM_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"

void strum_start();
void strum_end();
void strum_draw();
void strum_update();
void strum_key_down(uint8_t key);
void strum_key_up(uint8_t key);
void strum_button_down(uint8_t button);
void strum_button_double_down(uint8_t button);
void strum_button_up(uint8_t button);
void strum_trill_down(float pos, float size);
void strum_trill_up();

#endif