#ifndef TOUCHORD_MIDI_H
#define TOUCHORD_MIDI_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"

void send_midi_note(uint8_t status, uint8_t note, uint8_t velocity);

void send_poly_aftertouch(uint8_t note, uint8_t pressure);

#endif