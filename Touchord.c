#include <stdio.h>
#include "pico/stdlib.h"
#include "Notes/Note.h"
#include "Midi.h"

typedef enum 
{
    TOUCHORD_COMPOSE = 0,
    TOUCHORD_PERFORM,
    TOUCHORD_STRUM
} TouchordMode;

typedef struct 
{
    const char* key;
    int octave;
    int extension_count;
    int velocity;
    TouchordMode mode;
    int octave_count;
    int aftertouch;

    uint8_t[MAX_CHORD] current_chord;
    uint8_t current_length;
} TouchordData;



#define LED_PIN 25

int main()
{
    stdio_init_all();
    tusb_init();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint8_t midi[MAX_CHORD];
    char chord_name[CHORD_NAME_LEN];
    build_chord({"C", "maj"}, 4, 4, DEG_DEFAULT, 5, 1, midi, chord_name);


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
