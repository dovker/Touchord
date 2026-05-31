/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#include <stdio.h>
#include <string.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "pico/time.h"

#include "Defines.h"
#include "Types.h"
#include "Helper.h"
#include "Globals.h"
#include "IO/Midi.h"
#include "Data/Flash.h"
#include "Notes/Note.h"
#include "Sync.h"
#include "Rendering/Graphics.h"
#include "Rendering/ScreenDma.h"

#include "bsp/board.h"
#include "tusb.h"

#include "Modes/Compose.h"
#include "Modes/Perform.h"
#include "Modes/Strum.h"
#include "Modes/Omni.h"
#include "Modes/Drum.h"
#include "Modes/Settings.h"

void led_blinking_task(void);
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void poll_buttons()
{
    uint64_t now = to_ms_since_boot(get_absolute_time());
    if (now - tc_time_last_control > DEFAULT_DOUBLE_CLICK_MS) {
        if (tc_last_control_clicks >= 2)      tc_button_double_down(tc_last_control);
        else if (tc_last_control_clicks == 1) tc_button_down(tc_last_control);
        tc_last_control_clicks = 0;
    }

    for (int i = 0; i < NUM_CONTROLS; i++) {
        bool curr_state = gpio_get(i + CONTROL_0);
        if (tc_control_states[i] && !curr_state) {
            if (tc_control_double_click[i]) {
                if (now - tc_time_last_control < DEFAULT_DOUBLE_CLICK_MS && tc_last_control == i) {
                    tc_last_control_clicks++;
                } else {
                    tc_time_last_control = now;
                    tc_last_control_clicks = 1;
                }
            } else {
                tc_button_down(i);
            }
            tc_last_control = i;
        } else if (!tc_control_states[i] && curr_state) {
            tc_button_up(i);
        }
        tc_control_states[i] = curr_state;
    }

    //B0 + B2 + B4: drop into BOOTSEL.
    if (!tc_control_states[0] && !tc_control_states[2] && !tc_control_states[4])
        tc_trigger_bootsel = true;

    for (int i = 0; i < NUM_KEYS; i++) {
        bool curr_state = gpio_get(i + KEY_0);
        if (tc_key_states[i] && !curr_state) {
            tc_key_down(i);
            tc_last_key = i;
        } else if (!tc_key_states[i] && curr_state) {
            tc_key_up(i);
            tc_key_up_independent(i);
        }
        tc_key_states[i] = curr_state;
    }
    sleep_ms(10);
}

void poll_trill_bar(TrillBar* bar)
{
    trill_read(bar);
    float pos  = trill_calculate_touch(bar);
    float size = trill_calculate_size(bar, pos);

    bool curr_state = pos >= 0.0f;
    if (curr_state) {
        const float DEAD_HIGH = 0.04f;
        pos = pos / (1.0f - DEAD_HIGH);
        if (pos > 1.0f) pos = 1.0f;

        tc_trill_pos = pos;
        tc_trill_down(pos, size);
    } else if (tc_touch_state) {
        tc_trill_up();
    }
    tc_touch_state = curr_state;
}

static void init_input_bank(int base, int count, bool *states)
{
    for (int i = 0; i < count; i++) {
        gpio_init(base + i);
        gpio_set_dir(base + i, GPIO_IN);
        gpio_pull_up(base + i);
        states[i] = true;
    }
}

void init_GPIO()
{
    init_input_bank(CONTROL_0, NUM_CONTROLS, tc_control_states);
    init_input_bank(KEY_0,     NUM_KEYS,     tc_key_states);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

void init_i2c()
{
    // Trill bar's max I2C is 400 kHz
    i2c_init(TRILL_I2C, 400000);
    gpio_set_function(TRILL_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(TRILL_PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(TRILL_PIN_SDA);
    gpio_pull_up(TRILL_PIN_SCL);

#if TOUCHORD_DUAL_I2C
    // Screen owns its own bus, so push to 1 MHz
    i2c_init(DISP_I2C, 1000000);
    gpio_set_function(DISP_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DISP_PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DISP_PIN_SDA);
    gpio_pull_up(DISP_PIN_SCL);
#endif
}

TouchordMode prevMode = TOUCHORD_COMPOSE;

void select_mode(TouchordMode mode)
{
    switch (prevMode) {
        case TOUCHORD_COMPOSE:  compose_end();  break;
        case TOUCHORD_STRUM:    strum_end();    break;
        case TOUCHORD_PERFORM:  perform_end();  break;
        case TOUCHORD_OMNI:     omni_end();     break;
        case TOUCHORD_DRUM:     drum_end();     break;
        case TOUCHORD_SETTINGS: settings_end(); break;
    }
    switch (mode) {
        case TOUCHORD_COMPOSE:  compose_start();  break;
        case TOUCHORD_STRUM:    strum_start();    break;
        case TOUCHORD_PERFORM:  perform_start();  break;
        case TOUCHORD_OMNI:     omni_start();     break;
        case TOUCHORD_DRUM:     drum_start();     break;
        case TOUCHORD_SETTINGS: settings_start(); break;
    }
}

void io_task()
{
    multicore_lockout_victim_init();

    init_i2c();
    setup_midi_trs(tc_app.midi_type);

    tc_bar = trill_init(TRILL_I2C, TRILL_ADDR);
    trill_set_auto_scan(&tc_bar, 1);
    trill_set_noise_threshold(&tc_bar, 255);

    tc_disp.external_vcc = false;
    ssd1306_init(&tc_disp, 128, 64, DISP_ADDR, DISP_I2C);
    ssd1306_contrast(&tc_disp, 0xFF);
    screen_dma_init();

    anim_boot_intro();
    ssd1306_clear(&tc_disp);

    while (tc_running) {
        if (tc_trigger_bootsel) {
            tc_running = false;
            ssd1306_clear(&tc_disp);
            ssd1306_draw_string(&tc_disp, 10, 24, 2, "Firm Mode");
            ssd1306_show(&tc_disp);
            rom_reset_usb_boot(0, 0);
            break;
        }

        TouchordMode currentMode = tc_app_get_mode();
        if (currentMode != prevMode) {
            static uint8_t prev_fb[128 * 8];
            static uint8_t next_fb[128 * 8];
            memcpy(prev_fb, tc_disp.buffer, sizeof prev_fb);

            select_mode(currentMode);
            ssd1306_clear(&tc_disp);
            tc_draw();
            memcpy(next_fb, tc_disp.buffer, sizeof next_fb);

            int direction = (currentMode == TOUCHORD_SETTINGS || prevMode == TOUCHORD_SETTINGS)
                            ? 0
                            : (currentMode > prevMode ? 1 : -1);

            anim_transition(prev_fb, next_fb, direction);
            prevMode = currentMode;
            continue;
        }

        ssd1306_clear(&tc_disp);
        tc_draw();
        tc_update();
        draw_trill_indicator();
        draw_overlay();

#if TOUCHORD_DUAL_I2C
        screen_show_async(&tc_disp);
        do {
            poll_trill_bar(&tc_bar);
        } while (screen_show_busy());
        screen_show_wait();
#else
        for (int i = 0; i < 5; i++) poll_trill_bar(&tc_bar);
        ssd1306_show(&tc_disp);
#endif
    }
}

int main()
{
    board_init();
    init_GPIO();
    sleep_ms(500);

    if (!gpio_get(CONTROL_0))
        rom_reset_usb_boot(0, 0);

    tc_app = tc_app_default;
    tc_app_working = tc_app_default;
    flash_load_preset(0, &tc_app);
    // Always boot into Compose
    tc_app.mode = TOUCHORD_COMPOSE;

    tc_sync_init();
    reload_custom_scales();
    compose_start();

    tud_init(0);
    multicore_launch_core1(io_task);

    while (true) {
        tud_task();
        if (tud_midi_mounted()) poll_buttons();
        led_blinking_task();
    }
}

void tud_mount_cb(void)               { blink_interval_ms = BLINK_MOUNTED; }
void tud_umount_cb(void)              { blink_interval_ms = BLINK_NOT_MOUNTED; }
void tud_suspend_cb(bool remote)      { (void)remote; blink_interval_ms = BLINK_SUSPENDED; }
void tud_resume_cb(void)              { blink_interval_ms = BLINK_MOUNTED; }

void led_blinking_task(void)
{
    static uint32_t start_ms = 0;
    static bool led_state = false;

    if (board_millis() - start_ms < blink_interval_ms) return;
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = !led_state;
}
