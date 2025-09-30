#ifndef TOUCHORD_OMNI_H
#define TOUCHORD_OMNI_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"

void omni_start();
void omni_end();
void omni_draw();
void omni_update();
void omni_key_down(uint8_t key);
void omni_key_up(uint8_t key);
void omni_button_down(uint8_t button);
void omni_button_double_down(uint8_t button);
void omni_button_up(uint8_t button);
void omni_trill_down(float pos, float size);
void omni_trill_up();

#endif