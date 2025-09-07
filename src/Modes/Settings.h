#ifndef TOUCHORD_SETTINGS_H
#define TOUCHORD_SETTINGS_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"

void settings_start();
void settings_end();
void settings_draw();
void settings_update();
void settings_key_down(uint8_t key);
void settings_key_up(uint8_t key);
void settings_button_down(uint8_t button);
void settings_button_up(uint8_t button);
void settings_trill_down(float pos, float size);
void settings_trill_up();

#endif