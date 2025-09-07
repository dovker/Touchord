#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"

#include "Defines.h"
#include "Types.h"
#include "Helper.h"
#include "Globals.h"

#include "bsp/board.h"
#include "tusb.h"

#include "Modes/Compose.h"
#include "Modes/Perform.h"
#include "Modes/Strum.h"
#include "Modes/Settings.h"


void led_blinking_task(void);
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;


void poll_buttons()
{
    for (int i = 0; i < NUM_CONTROLS; i++)
    {   
        bool curr_state = gpio_get(i + CONTROL_0);
        if(tc_control_states[i] && !curr_state)
        {
            tc_button_down(i);

            //appData.current_key = i-3;
        }
        else if(!tc_control_states[i] && curr_state)
        {
            tc_button_up(i);
        }
        if(!tc_control_states[0] && !tc_control_states[1] && !tc_control_states[2])
        {
            tc_trigger_bootsel = true;
        }
        tc_control_states[i] = curr_state;
    }

    for (int i = 0; i < NUM_KEYS; i++)
    {   
        bool curr_state = gpio_get(i + KEY_0);
        if(tc_key_states[i] && !curr_state)
        {
            tc_key_down(i);

            tc_last_key = i;
            tc_key_states[i] = curr_state;
            break;
        }
        else if(!tc_key_states[i] && curr_state && tc_last_key == i)
        {
            tc_key_up(i);
        }
        tc_key_states[i] = curr_state;
    }
    sleep_ms(10);
}



void poll_trill_bar(TrillBar* bar)
{
    trill_read(bar);
    float touchPos = trill_calculate_touch(bar);
    float touchSize = trill_calculate_size(bar, touchPos);

    bool curr_state = touchPos >= 0.0f;
    if(curr_state)
    {
        tc_trill_down(touchPos, touchSize);
    }
    else if(tc_touch_state)
    {
        tc_trill_up();
    }
    tc_touch_state = curr_state;
}

void init_GPIO()
{
    for (int i = 0; i < NUM_CONTROLS; i++)
    {
        gpio_init(i + CONTROL_0);
        gpio_set_dir(i + CONTROL_0, GPIO_IN);
        gpio_pull_up(i + CONTROL_0);

        tc_control_states[i] = true;
      }

    for (int i = 0; i < NUM_KEYS; i++)
    {
        gpio_init(i + KEY_0);
        gpio_set_dir(i + KEY_0, GPIO_IN);
        gpio_pull_up(i + KEY_0);
        tc_key_states[i] = true;
    }
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

void init_i2c()
{
    i2c_init(i2c0, 400000);

    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
}

TouchordMode prevMode = TOUCHORD_COMPOSE;

void select_mode(TouchordMode mode)
{
    switch(prevMode)
    {
        case TOUCHORD_COMPOSE: compose_end(); break;
        case TOUCHORD_STRUM: strum_end(); break;
        case TOUCHORD_PERFORM: perform_end(); break;
        case TOUCHORD_SETTINGS: settings_end(); break;
    }
    switch(mode)
    {
        case TOUCHORD_COMPOSE:
        {
            compose_start();
        }
        break;
        case TOUCHORD_STRUM:
        {
            strum_start();
        }
        break;
        case TOUCHORD_PERFORM:
        {
            perform_start();
        }
        break;
        case TOUCHORD_SETTINGS:
        {
            // compose_start();
            // tc_state->draw        = compose_draw;
            // tc_state->update      = compose_update;
            // tc_state->key_down    = compose_key_down;
            // tc_state->key_up      = compose_key_up;
            // tc_state->button_down = compose_button_down;
            // tc_state->button_up   = compose_button_up;
            // tc_state->trill_down  = compose_trill_down;
            // tc_state->trill_up    = compose_trill_up;
        }
        break;
    }
}


void io_task()
{   
    
    sleep_ms(1000);
    ssd1306_clear(&tc_disp);
    while(tc_running)
    {
        if(tc_trigger_bootsel)
        {
            tc_running = false;
            ssd1306_clear(&tc_disp);
            ssd1306_draw_string(&tc_disp, 10, 24, 2, "Firm Mode");
            ssd1306_show(&tc_disp);
            rom_reset_usb_boot(0, 0);
            break;
        }

        if(tc_app.mode != prevMode) 
        { 
            select_mode(tc_app.mode);
            prevMode = tc_app.mode;
        }

        poll_trill_bar(&tc_bar);
        
        ssd1306_clear(&tc_disp);

        tc_draw();
        tc_update();

        uint32_t len = strlen(tc_app.chord_name);
        uint8_t text_w = len * 6 - 1;
        ssd1306_draw_string(&tc_disp, 64 - text_w, 24, 2, tc_app.chord_name);

        uint8_t rootLen = strlen(tc_app.key[tc_app.current_key].root);
        uint8_t qualLen = strlen(tc_app.key[tc_app.current_key].quality);
        text_w = (5 + rootLen + qualLen) * 6 - 1;
        uint8_t pos = 64-text_w/2;
        ssd1306_draw_string(&tc_disp, pos, 0, 1, "Key:");
        ssd1306_draw_string(&tc_disp, pos + 25, 0, 1, tc_app.key[tc_app.current_key].root);
        ssd1306_draw_string(&tc_disp, pos + 25 + rootLen * 6, 0, 1, tc_app.key[tc_app.current_key].quality);
        
        ssd1306_show(&tc_disp);
    }
}

int main()
{
    board_init();
    sleep_ms(500);
    tud_init(0);
    sleep_ms(500);

    init_GPIO();
    init_i2c();

    tc_bar = trill_init(i2c0, TRILL_ADDR);
    trill_set_noise_threshold(&tc_bar, 255);

    tc_disp.external_vcc = false;
    ssd1306_init(&tc_disp, 128, 64, 0x3C, i2c0);
    ssd1306_clear(&tc_disp);
    ssd1306_draw_string(&tc_disp, 10, 24, 2, "Touchord");
    ssd1306_show(&tc_disp);
    
    compose_start();

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