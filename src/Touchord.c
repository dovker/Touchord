#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "pico/time.h"

#include "Defines.h"
#include "Debug.h"
#include "Types.h"
#include "Helper.h"
#include "Globals.h"
#include "IO/Midi.h"
#include "IO/Output.h"
#include "Data/Parser.h"
#include "Notes/Note.h"
#include "Rendering/Graphics.h"
#include "Synth/AmyEngine.h"

#include "bsp/board.h"
#include "tusb.h"

#include "Modes/Compose.h"
#include "Modes/Perform.h"
#include "Modes/Strum.h"
#include "Modes/Omni.h"
#include "Modes/Drum.h"
#include "Modes/Settings.h"

#if NUM_CONTROLS < 1 || NUM_CONTROLS > 6
#error "NUM_CONTROLS must be between 1 and 6"
#endif

#if NUM_KEYS < 1 || NUM_KEYS > 7
#error "NUM_KEYS must be between 1 and 7"
#endif

#if NUM_CONTROLS > 3 && !defined(CONTROL_3)
#error "CONTROL_3 must be defined when NUM_CONTROLS is greater than 3"
#endif

#if NUM_CONTROLS > 4 && !defined(CONTROL_4)
#error "CONTROL_4 must be defined when NUM_CONTROLS is greater than 4"
#endif

#if NUM_CONTROLS > 5 && !defined(CONTROL_5)
#error "CONTROL_5 must be defined when NUM_CONTROLS is greater than 5"
#endif

#if NUM_KEYS > 1 && !defined(KEY_1)
#error "KEY_1 must be defined when NUM_KEYS is greater than 1"
#endif

#if NUM_KEYS > 2 && !defined(KEY_2)
#error "KEY_2 must be defined when NUM_KEYS is greater than 2"
#endif

#if NUM_KEYS > 3 && !defined(KEY_3)
#error "KEY_3 must be defined when NUM_KEYS is greater than 3"
#endif

#if NUM_KEYS > 4 && !defined(KEY_4)
#error "KEY_4 must be defined when NUM_KEYS is greater than 4"
#endif

#if NUM_KEYS > 5 && !defined(KEY_5)
#error "KEY_5 must be defined when NUM_KEYS is greater than 5"
#endif

#if NUM_KEYS > 6 && !defined(KEY_6)
#error "KEY_6 must be defined when NUM_KEYS is greater than 6"
#endif

#if ((PIN_SDA % 4) == 0) && (PIN_SCL == (PIN_SDA + 1))
#define TC_I2C_INSTANCE i2c0
#elif ((PIN_SDA % 4) == 2) && (PIN_SCL == (PIN_SDA + 1))
#define TC_I2C_INSTANCE i2c1
#else
#error "PIN_SDA and PIN_SCL must be a valid RP2040 I2C SDA/SCL pair"
#endif

#if ENABLE_BOOTSEL_BUTTON_CHORD && NUM_CONTROLS < 5
#error "ENABLE_BOOTSEL_BUTTON_CHORD requires at least 5 control buttons"
#endif

static const uint8_t tc_control_pins[NUM_CONTROLS] = {
    CONTROL_0,
#if NUM_CONTROLS > 1
    CONTROL_1,
#endif
#if NUM_CONTROLS > 2
    CONTROL_2,
#endif
#if NUM_CONTROLS > 3
    CONTROL_3,
#endif
#if NUM_CONTROLS > 4
    CONTROL_4,
#endif
#if NUM_CONTROLS > 5
    CONTROL_5,
#endif
};

static const uint8_t tc_key_pins[NUM_KEYS] = {
    KEY_0,
#if NUM_KEYS > 1
    KEY_1,
#endif
#if NUM_KEYS > 2
    KEY_2,
#endif
#if NUM_KEYS > 3
    KEY_3,
#endif
#if NUM_KEYS > 4
    KEY_4,
#endif
#if NUM_KEYS > 5
    KEY_5,
#endif
#if NUM_KEYS > 6
    KEY_6,
#endif
};

static bool tc_boot_display_ready = false;
static bool tc_usb_stack_started = false;

void led_blinking_task(void);
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

static bool tc_usb_ready(void)
{
    return tc_usb_stack_started && tud_inited();
}

static void boot_status(const char *stage)
{
    tc_debug_stage(stage);

    if (tc_boot_display_ready) {
        char usb_line[24];

        ssd1306_clear(&tc_disp);
        ssd1306_draw_string(&tc_disp, 0, 0, 1, "Boot");
        ssd1306_draw_string(&tc_disp, 0, 12, 1, stage);

        if (tc_usb_ready()) {
            snprintf(usb_line, sizeof usb_line, "U%u C%u M%u",
                tud_mounted() ? 1u : 0u,
                tud_cdc_connected() ? 1u : 0u,
                tud_midi_mounted() ? 1u : 0u);
        } else {
            snprintf(usb_line, sizeof usb_line, "U- C- M-");
        }

        ssd1306_draw_string(&tc_disp, 0, 24, 1, usb_line);
        ssd1306_show(&tc_disp);
    }

    tc_debug_service_usb(25);
}

static void log_pin_map(void)
{
    tc_debug_logf("i2c pins sda=%u scl=%u", PIN_SDA, PIN_SCL);
    tc_debug_logf("audio pins bclk=%u din=%u lrclk=%u",
        AUDIO_I2S_BCLK_PIN,
        AUDIO_I2S_DATA_PIN,
        AUDIO_I2S_LRCLK_PIN);

    for (uint8_t i = 0; i < NUM_CONTROLS; ++i) {
        tc_debug_logf("control[%u]=gpio%u", i, tc_control_pins[i]);
    }

    for (uint8_t i = 0; i < NUM_KEYS; ++i) {
        tc_debug_logf("key[%u]=gpio%u", i, tc_key_pins[i]);
    }
}

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
        bool curr_state = gpio_get(tc_control_pins[i]);
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
#if ENABLE_BOOTSEL_BUTTON_CHORD
        if(!tc_control_states[0] && !tc_control_states[2] && !tc_control_states[4])
        {
            tc_trigger_bootsel = true;
        }
#endif
        tc_control_states[i] = curr_state;
    }

    for (int i = 0; i < NUM_KEYS; i++)
    {   
        bool curr_state = gpio_get(tc_key_pins[i]);
        if(tc_key_states[i] && !curr_state)
        {
            tc_debug_last_key_down = i;
            tc_debug_key_down_count++;
            tc_key_down(i);

            tc_last_key = i;
            tc_key_states[i] = curr_state;
            break;
        }
        else if(!tc_key_states[i] && curr_state)
        {
            if(tc_last_key == i)
            {
                tc_key_up(i);
            }
            tc_key_up_independent(i);
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
        gpio_init(tc_control_pins[i]);
        gpio_set_dir(tc_control_pins[i], GPIO_IN);
        gpio_pull_up(tc_control_pins[i]);

        tc_control_states[i] = true;
      }

    for (int i = 0; i < NUM_KEYS; i++)
    {
        gpio_init(tc_key_pins[i]);
        gpio_set_dir(tc_key_pins[i], GPIO_IN);
        gpio_pull_up(tc_key_pins[i]);
        tc_key_states[i] = true;
    }
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

void init_i2c()
{
    i2c_init(TC_I2C_INSTANCE, 400000);

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
        case TOUCHORD_DRUM: drum_end(); break;
        case TOUCHORD_SETTINGS: settings_end(); break;
    }

    tc_output_all_notes_off(tc_app.channel);

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
        case TOUCHORD_DRUM:
        {
            drum_start();
        }
        break;
        case TOUCHORD_SETTINGS:
        {
            settings_start();
        }
        break;
    }

    tc_prev_mode = mode;
}


static void update_ui(void)
{   
    if(tc_app.mode != prevMode) 
    { 
        select_mode(tc_app.mode);
        prevMode = tc_app.mode;
    }

    poll_trill_bar(&tc_bar);
    
    ssd1306_clear(&tc_disp);

    tc_draw();
    tc_update();
    draw_debug_overlay();
    
    ssd1306_show(&tc_disp);
}

static void enter_bootsel(void)
{
    tc_output_all_notes_off(tc_app.channel);
    tc_running = false;
    ssd1306_clear(&tc_disp);
    ssd1306_draw_string(&tc_disp, 10, 24, 2, "Firm Mode");
    ssd1306_show(&tc_disp);
    sleep_ms(50);
    rom_reset_usb_boot(0, 0);
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
    tc_debug_init();
    boot_status("board");
    board_init();
    tc_debug_logf("board init ok");

    boot_status("gpio");
    init_GPIO();
    log_pin_map();

    sleep_ms(500);
    
#if ENABLE_BOOTSEL_ON_STARTUP
    if(!gpio_get(tc_control_pins[0])) {
        tc_debug_logf("bootsel on startup requested");
        rom_reset_usb_boot(0, 0);
    }
#endif

    boot_status("usb");
    tud_init(0);
    tc_usb_stack_started = true;
    tc_debug_mark_usb_started();
    tc_debug_service_usb(500);

    boot_status("i2c");
    init_i2c();
    tc_debug_logf("i2c init ok");

    boot_status("display");
    tc_disp.external_vcc = false;
    ssd1306_init(&tc_disp, 128, 64, DISP_ADDR, TC_I2C_INSTANCE);
    ssd1306_contrast(&tc_disp, 0xFF);
    tc_boot_display_ready = true;
    boot_status("display ok");

    boot_status("midi trs");
    setup_midi_trs(tc_app.midi_type);
    tc_debug_logf("midi trs init ok");

    boot_status("trill");
    tc_bar = trill_init(TC_I2C_INSTANCE, TRILL_ADDR);
    trill_set_auto_scan(&tc_bar, 1);
    trill_set_noise_threshold(&tc_bar, 255);
    tc_debug_logf("trill init ok addr=0x%02x", TRILL_ADDR);

    boot_status("touchord");
    ssd1306_clear(&tc_disp);
    ssd1306_draw_string(&tc_disp, 10, 24, 2, "Touchord");
    ssd1306_show(&tc_disp);
    
    boot_status("settings");
    reload_custom_scales();
    compose_start();
    tc_debug_logf("mode init ok");

    boot_status("output");
    tc_output_init();
    tc_debug_logf("output init ok");

    boot_status("audio core");
    multicore_launch_core1(tc_synth_audio_task);
    tc_debug_logf("core1 launch requested");

    boot_status("loop");

    while (tc_running) {
        tud_task();
        if (tc_trigger_panic) {
            tc_output_all_notes_off(tc_app.channel);
            tc_trigger_panic = false;
        }
        if (tc_trigger_bootsel) {
            enter_bootsel();
            break;
        }
        poll_buttons();
        update_ui();
        led_blinking_task();
        serial_poll();
    }
}

void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
  tc_trigger_panic = true;
  tc_debug_logf("usb mounted");
}

void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
  tc_trigger_panic = true;
  tc_debug_logf("usb unmounted");
}

void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
  tc_trigger_panic = true;
  tc_debug_logf("usb suspended");
}

void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
  tc_trigger_panic = true;
  tc_debug_logf("usb resumed");
}

void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  if ( board_millis() - start_ms < blink_interval_ms) return;
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state;
}
