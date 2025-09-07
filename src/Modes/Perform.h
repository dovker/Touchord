#ifndef TOUCHORD_PERFORM_H
#define TOUCHORD_PERFORM_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"
#include "Types.h"

void perform_start();
void perform_end();
void perform_draw();
void perform_update();
void perform_key_down(uint8_t key);
void perform_key_up(uint8_t key);
void perform_button_down(uint8_t button);
void perform_button_up(uint8_t button);
void perform_trill_down(float pos, float size);
void perform_trill_up();

#endif