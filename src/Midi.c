#include "Midi.h"
#include "tusb.h"

void send_midi_chord(uint8_t channel, uint8_t status, uint8_t* notes, uint8_t length, uint8_t velocity)
{
    for(int i = 0; i < length; i++)
    {
        send_midi_note(channel, status, notes[i], velocity);
    }
}

void send_midi_note(uint8_t channel, uint8_t status, uint8_t note, uint8_t velocity) {
    uint8_t packet[3] = {
        status | (channel & 0x0F),
        note,
        velocity
    };
    tud_midi_stream_write(0, packet, 3);
}

void send_midi_cc(uint8_t channel, uint8_t cc_num, uint8_t cc_value) {
    uint8_t status = 0xB0 | (channel & 0x0F); 
    uint8_t packet[3] = {
        status,
        cc_num,
        cc_value
    };
    tud_midi_stream_write(0, packet, 3);
}

void send_poly_aftertouch(uint8_t channel, uint8_t note, uint8_t pressure) {
    uint8_t status = 0xA0 | (channel & 0x0F);

    uint8_t packet[3] = {
        status,
        note,
        pressure
    };
   tud_midi_stream_write(0, packet, 3);
}

void send_aftertouch(uint8_t channel, uint8_t pressure)
{
    uint8_t status = 0xA0 | (channel & 0x0F);

    uint8_t packet[2] = {
        status,
        pressure
    };
   tud_midi_stream_write(0, packet, 2);
}