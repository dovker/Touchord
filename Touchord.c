#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "Defines.h"
#include "Types.h"

#include "Notes/Note.h"
#include "Midi.h"

#include "bsp/board.h"
#include "tusb.h"


void led_blinking_task(void);

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
bool button_states[NUM_BUTTONS];

void buttons()
{
    for (int i = 0; i < NUM_BUTTONS; i++)
    {   
        bool curr_state = gpio_get(i + 1);
        if(button_states[i] && !curr_state)
        {
            send_midi_note(NOTE_ON, 60 + i, 100);
            gpio_put(LED_PIN, 1);
        }
        else if(!button_states[i] && curr_state)
        {
            send_midi_note(NOTE_OFF, 60 + i, 0);
            gpio_put(LED_PIN, 0);
        }
        button_states[i] = curr_state;
    }
    sleep_ms(10);
}

int main()
{
    stdio_init_all();
    sleep_ms(100);
    tusb_init();

    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        gpio_init(i + 1);
        gpio_set_dir(i + 1, GPIO_IN);
        gpio_pull_up(i + 1);

        button_states[i] = true;
    }

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // uint8_t midi[MAX_CHORD];
    // char chord_name[CHORD_NAME_LEN];
    // build_chord({"C", "maj"}, 4, 4, DEG_DEFAULT, 5, 1, midi, chord_name);

    
    while (true) {
        tud_task();
        if (tud_midi_mounted()) 
        {
            buttons();
        }
        led_blinking_task();
    }
}

void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}