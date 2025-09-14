#include "Midi.h"
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
void setup_midi_trs()
{
    gpio_init(MIDI_PIN_TYPE_A);
    gpio_init(MIDI_PIN_TYPE_B);
    gpio_set_dir(MIDI_PIN_TYPE_A, GPIO_OUT);
    gpio_set_dir(MIDI_PIN_TYPE_B, GPIO_OUT);

    switch_midi_trs(MIDI_TRS_A);

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
    tud_midi_stream_write(0, packet, 3);
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