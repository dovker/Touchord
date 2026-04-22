#include "Midi.h"
#include "Globals.h"
#include "tusb.h"
#include "hardware/uart.h"

void switch_midi_trs(MidiType type)
{
    //Toggles the TS5A23157DGSR
    switch(type)
    {
        case MIDI_TRS_A:
        gpio_put(MIDI_PIN_IN_1, 0);
        gpio_put(MIDI_PIN_IN_2, 0);
        break;
        case MIDI_TRS_B:
        gpio_put(MIDI_PIN_IN_1, 1);
        gpio_put(MIDI_PIN_IN_2, 1);
        break;
    }
}
void setup_midi_trs(MidiType type)
{
    gpio_init(MIDI_PIN_IN_1);
    gpio_init(MIDI_PIN_IN_2);
    gpio_set_dir(MIDI_PIN_IN_1, GPIO_OUT);
    gpio_set_dir(MIDI_PIN_IN_2, GPIO_OUT);

    switch_midi_trs(type);

    uart_init(uart1, MIDI_BAUD_RATE);
    gpio_set_function(MIDI_PIN_DAT, GPIO_FUNC_UART); //MIDI_PIN_DAT is TX pin
    uart_set_hw_flow(uart1, false, false);
    uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart1, false);
}

void send_trs_midi(uint8_t* data, uint8_t len)
{
    for(int i = 0; i < len; i++)
    {
        uart_putc_raw(uart1, data[i]);
    }
}

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
    uint32_t bytes_written = tud_midi_stream_write(0, packet, 3);

    if (status == NOTE_ON && velocity != 0 && bytes_written == 3) {
        tc_debug_last_usb_midi_note = (int8_t)note;
        tc_debug_usb_midi_note_count++;
    }

    send_trs_midi(packet, 3);
}

void send_midi_cc(uint8_t channel, uint8_t cc_num, uint8_t cc_value) {
    uint8_t status = 0xB0 | (channel & 0x0F); 
    uint8_t packet[3] = {
        status,
        cc_num,
        cc_value
    };
    tud_midi_stream_write(0, packet, 3);
    send_trs_midi(packet, 3);
}

void send_midi_program_change(uint8_t channel, uint8_t program)
{
    uint8_t packet[2] = {
        (uint8_t)(0xC0 | (channel & 0x0F)),
        program
    };

    tud_midi_stream_write(0, packet, 2);
    send_trs_midi(packet, 2);
}

void send_midi_pitch_bend(uint8_t channel, uint16_t bend)
{
    uint8_t packet[3] = {
        (uint8_t)(0xE0 | (channel & 0x0F)),
        (uint8_t)(bend & 0x7F),
        (uint8_t)((bend >> 7) & 0x7F)
    };

    tud_midi_stream_write(0, packet, 3);
    send_trs_midi(packet, 3);
}

void send_midi_pedal(uint8_t channel, uint8_t value)
{
    send_midi_cc(channel, 0x40, value);
}

void send_midi_all_notes_off(uint8_t channel)
{
    send_midi_cc(channel, 0x7B, 0);
}

void send_poly_aftertouch(uint8_t channel, uint8_t note, uint8_t pressure) {
    uint8_t status = 0xA0 | (channel & 0x0F);

    uint8_t packet[3] = {
        status,
        note,
        pressure
    };
   tud_midi_stream_write(0, packet, 3);
   send_trs_midi(packet, 3);
}

void send_aftertouch(uint8_t channel, uint8_t pressure)
{
    uint8_t status = 0xA0 | (channel & 0x0F);

    uint8_t packet[2] = {
        status,
        pressure
    };
   tud_midi_stream_write(0, packet, 2);
   send_trs_midi(packet, 2);
}
