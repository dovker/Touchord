#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "Defines.h"
#include "Types.h"
#include "Helper.h"

#include "Notes/Note.h"
#include "Midi.h"

#include "bsp/board.h"
#include "tusb.h"

#include "Trill.h"
#include "ssd1306.h"


TouchordData appData = 
{
    {{"C", "min"}, {"E", "min"}, {"D", "maj"}}, 0,
    4, 6, 0, 100, 
    TOUCHORD_COMPOSE, 1, 0,
    {0, 0, 0, 0, 0, 0}, {'\0'}, CHORD_DEFAULT
};



void led_blinking_task(void);

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
bool button_states[NUM_BUTTONS];
bool control_states[NUM_CONTROLS];
int last_button;


void poll_buttons()
{
    for (int i = 3; i < NUM_CONTROLS; i++)
    {   
        bool curr_state = gpio_get(i + CONTROL_0);
        if(!control_states[i] && curr_state)
        {
            appData.current_key = i-3;
        }
        control_states[i] = curr_state;
    }

    for (int i = 0; i < NUM_BUTTONS; i++)
    {   
        bool curr_state = gpio_get(i + BUTTON_0);
        if(button_states[i] && !curr_state)
        {
            send_midi_chord(NOTE_OFF, appData.chord, appData.extension_count, appData.velocity);
            build_chord(appData.key[appData.current_key], appData.octave, i, appData.degree, 
                        appData.extension_count, appData.inversion, appData.chord, appData.chord_name);
            send_midi_chord(NOTE_ON, appData.chord, appData.extension_count, appData.velocity);
            last_button = i;
            button_states[i] = curr_state;
            break;
        }
        else if(!button_states[i] && curr_state && last_button == i)
        {
            send_midi_chord(NOTE_OFF, appData.chord, appData.extension_count, appData.velocity);
            build_chord(appData.key[appData.current_key], appData.octave, 0, CHORD_DEFAULT, 
                        0, appData.inversion, appData.chord, appData.chord_name);
            appData.chord_name[0] = '\0';
        }
        button_states[i] = curr_state;
    }
    sleep_ms(10);
}

void init_buttons()
{
    for (int i = 0; i < NUM_CONTROLS; i++)
    {
        gpio_init(i + CONTROL_0);
        gpio_set_dir(i + CONTROL_0, GPIO_IN);
        gpio_pull_up(i + CONTROL_0);

        control_states[i] = true;
      }

    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        gpio_init(i + BUTTON_0);
        gpio_set_dir(i + BUTTON_0, GPIO_IN);
        gpio_pull_up(i + BUTTON_0);
        button_states[i] = true;
    }
}

void init_i2c()
{
    i2c_init(i2c0, 400000);

    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
}


void io_task()
{
    init_i2c();
    ssd1306_t disp;
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c0);
    ssd1306_clear(&disp);
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    TrillBar bar = trill_init(i2c0, TRILL_ADDR);
    trill_set_noise_threshold(&bar, 255);

    uint8_t count = 0;
    Touch touches[TRILL_MAX_TOUCHES];

    while(true)
    {
        ssd1306_clear(&disp);

        ssd1306_draw_string(&disp, 8, 24, 2, appData.chord_name);

        ssd1306_draw_string(&disp, 0, 0, 1, "Key: ");
        ssd1306_draw_string(&disp, 26, 0, 1, appData.key[appData.current_key].root);
        ssd1306_draw_string(&disp, 38, 0, 1, appData.key[appData.current_key].quality);
        // sprintf(buf, "Ext: %d", appData.exttension_count);
        // ssd1306_draw_string(&disp, 0, (i+1) * 10, 1, buf);
        
        
        trill_read(&bar);
        
        int seg = segments(trill_calculate_touch(&bar), 5);
        if(seg >= 0)
        {
            switch (seg)
            {
                case 0: 
                    appData.extension_count = 6; 
                    appData.degree = CHORD_DEFAULT;
                    break;
                case 1: 
                    appData.extension_count = 5; 
                    appData.degree = CHORD_DEFAULT;
                    break;
                case 2: 
                    appData.extension_count = 4; 
                    appData.degree = CHORD_DEFAULT;
                    break;
                case 3: 
                    appData.extension_count = 4; 
                    appData.degree = CHORD_PARALLEL;
                    break;
                case 4: 
                    appData.extension_count = DEFAULT_EXTENSIONS; 
                    appData.degree = CHORD_PARALLEL;
                    break;
            }
        }
        else
        {
            appData.extension_count = DEFAULT_EXTENSIONS;
            appData.degree = CHORD_DEFAULT;
        }

        char buf[32];
        sprintf(buf, "Seg: %f", seg);
        ssd1306_draw_string(&disp, 0, 54, 1, buf);

        float pos = trill_calculate_touch(&bar);
        sprintf(buf, "Pos: %f", pos);
        ssd1306_draw_string(&disp, 64, 0, 1, buf);

        float size = trill_calculate_size(&bar, pos);
        sprintf(buf, "Siz: %f", size);
        ssd1306_draw_string(&disp, 64, 9, 1, buf);


        // for(int i = 0; i < count; i++)
        // {
        //     char buf[32];
        //     sprintf(buf, "%f, %f", touches[i].pos, touches[i].size);
        //     ssd1306_draw_string(&disp, 0, (i+1) * 10, 1, buf);
        // }
        for (int i =0; i < 26; i++)
        {
            ssd1306_draw_square(&disp, 0, i*2, bar.raw_data[i]*128/0xFFFF, 2);
        }
        
        ssd1306_show(&disp);
    }
}

int main()
{
    board_init();
    sleep_ms(2000);
    tud_init(0);

    init_buttons();
    multicore_launch_core1(io_task);

    while (true) {
        tud_task();
        if (tud_midi_mounted()) 
        {
            poll_buttons();
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