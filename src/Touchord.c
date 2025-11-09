#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "pico/time.h"

#include "Defines.h"
#include "Types.h"
#include "Helper.h"
#include "Globals.h"
#include "IO/Midi.h"
#include "Data/Parser.h"
#include "Notes/Note.h"

#include "bsp/board.h"
#include "tusb.h"

#include "Modes/Compose.h"
#include "Modes/Perform.h"
#include "Modes/Strum.h"
#include "Modes/Omni.h"
#include "Modes/Settings.h"


void led_blinking_task(void);
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void poll_buttons()
{
    uint64_t now = to_ms_since_boot(get_absolute_time());
    if(now - tc_time_last_control > DEFAULT_DOUBLE_CLICK_MS)
    {
        if(tc_last_control_clicks >= 2)
        {
            tc_button_double_down(tc_last_control);
        }
        else if (tc_last_control_clicks == 1)
        {
            tc_button_down(tc_last_control);
        }
        tc_last_control_clicks = 0;
    }

    for (int i = 0; i < NUM_CONTROLS; i++)
    {   
        bool curr_state = gpio_get(i + CONTROL_0);
        if(tc_control_states[i] && !curr_state)
        {
            if(tc_control_double_click[i]) // Improves responsiveness for other buttons
            {
                if(now - tc_time_last_control < DEFAULT_DOUBLE_CLICK_MS && tc_last_control == i)
                {
                    tc_last_control_clicks++;
                } 
                else
                {
                    tc_time_last_control = now;
                    tc_last_control_clicks = 1;
                }
            }
            else
            {
                tc_button_down(i);
            }
            tc_last_control = i;
        }
        else if(!tc_control_states[i] && curr_state)
        {
            tc_button_up(i);
        }
        if(!tc_control_states[0] && !tc_control_states[2] && !tc_control_states[4])
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
        case TOUCHORD_OMNI: omni_end(); break;
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
        case TOUCHORD_OMNI:
        {
            omni_start();
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
        
        ssd1306_show(&tc_disp);
    }
}

void serial_poll(void)
{
    static char buf[1024];
    static int  idx = 0;

    while (tud_cdc_available())
    {
        char c = (char)tud_cdc_read_char();

        if (c == '\r' || c == '\n')
        {
            if (idx)          
            {
                buf[idx] = '\0';
                process_cmd(buf);
                idx = 0;
            }
        }
        else if (idx < (int)sizeof buf - 1)
        {
            buf[idx++] = c;
        }
    }
}

int main()
{
    //stdio_init_all();
    board_init();
    sleep_ms(500);
    tud_init(0);
    sleep_ms(500);

    init_GPIO();
    init_i2c();
    setup_midi_trs();

    tc_bar = trill_init(i2c0, TRILL_ADDR);
    trill_set_auto_scan(&tc_bar, 1);
    trill_set_noise_threshold(&tc_bar, 255);

    tc_disp.external_vcc = false;
    ssd1306_init(&tc_disp, 128, 64, 0x3C, i2c0);
    ssd1306_contrast(&tc_disp, 0xFF);
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

        serial_poll();
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