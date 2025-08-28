#include "Midi.h"
#include "tusb.h"

void send_midi_chord(uint8_t status, uint8_t* notes, uint8_t length, uint8_t velocity)
{
    for(int i = 0; i < length; i++)
    {
        send_midi_note(status, notes[i], velocity);
    }
}

void send_midi_note(uint8_t status, uint8_t note, uint8_t velocity) {
    uint8_t packet[3] = {
        status | (MIDI_CHANNEL & 0x0F),
        note,
        velocity
    };
    tud_midi_stream_write(0, packet, 3);
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
