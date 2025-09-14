#ifndef TOUCHORD_MIDI_H
#define TOUCHORD_MIDI_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "Defines.h"
#include "Types.h"

void switch_midi_trs(MidiType type);
void setup_midi_trs();
void send_trs_midi(uint8_t* data, uint8_t len);

void send_midi_note(uint8_t channel, uint8_t status, uint8_t note, uint8_t velocity);

void send_poly_aftertouch(uint8_t channel, uint8_t note, uint8_t pressure);
void send_aftertouch(uint8_t channel, uint8_t pressure);

void send_midi_cc(uint8_t channel, uint8_t cc_num, uint8_t cc_value);

void send_midi_chord(uint8_t channel, uint8_t status, uint8_t* notes, uint8_t length, uint8_t velocity);

#endif