#ifndef TOUCHORD_MIDI_H
#define TOUCHORD_MIDI_H

#include "tusb.h"

#define NOTE_ON  0x90
#define NOTE_OFF 0x80
#define MIDI_CHANNEL 0

void send_midi_note(uint8_t status, uint8_t note, uint8_t velocity) {
    uint8_t packet[4] = {
        0x09,
        status | (MIDI_CHANNEL & 0x0F),
        note,
        velocity
    };
    tud_midi_stream_write(0, packet, 4);
}

void send_poly_aftertouch(uint8_t note, uint8_t pressure) {
    uint8_t status = 0xA0 | (MIDI_CHANNEL & 0x0F);

    uint8_t packet[4] = {
        0x0A,
        status,
        note,
        pressure
    };
    tud_midi_stream_write(0, packet, 4);
}




#endif