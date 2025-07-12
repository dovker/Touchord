#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"

#define MIDI_NOTE 60      // Middle C
#define MIDI_VELOCITY 100
#define MIDI_CHANNEL 0    // Channel 1 (0-based)
#define NOTE_ON  0x90
#define NOTE_OFF 0x80

void send_midi_note(uint8_t status, uint8_t note, uint8_t velocity) {
    uint8_t packet[4] = {
        0x09,           // USB MIDI CIN: Note On/Off, cable 0
        status | MIDI_CHANNEL,
        note,
        velocity
    };
    tud_midi_stream_write(0, packet, 4);
}

#define LED_PIN 25

int main()
{
    stdio_init_all();
    tusb_init();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        tud_task();
        if (tud_midi_mounted()) {
            gpio_put(LED_PIN, 1);
            send_midi_note(NOTE_ON, MIDI_NOTE, MIDI_VELOCITY);
            sleep_ms(2000);
            gpio_put(LED_PIN, 0);
            send_midi_note(NOTE_OFF, MIDI_NOTE, 0);
            sleep_ms(2000);
        }
    }
}
