#ifndef TOUCHORD_COMPOSE_H
#define TOUCHORD_COMPOSE_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"

void compose_start();
void compose_end();
void compose_draw();
void compose_update();
void compose_key_down(uint8_t key);
void compose_key_up(uint8_t key);
void compose_button_down(uint8_t button);
void compose_button_double_down(uint8_t button);
void compose_button_up(uint8_t button);
void compose_trill_down(float pos, float size);
void compose_trill_up();

#endif