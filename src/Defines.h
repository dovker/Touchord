/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_DEFINES_H
#define TOUCHORD_DEFINES_H

#define NOTE_ON  0x90
#define NOTE_OFF 0x80
#define MIDI_CHANNEL 0
#define MIDI_NOTE 60
#define MIDI_CUTOFF 74
#define MIDI_MOD 1

#define MIDI_PIN_IN_1 18
#define MIDI_PIN_IN_2 19
#define MIDI_PIN_DAT 20
#define MIDI_BAUD_RATE 31250

#define DEFAULT_CUTOFF 100

#define DEFAULT_DOUBLE_CLICK_MS 300

#ifndef TOUCHORD_DUAL_I2C
#define TOUCHORD_DUAL_I2C 1
#endif

#if TOUCHORD_DUAL_I2C
#define TRILL_I2C       i2c0
#define TRILL_PIN_SDA   16
#define TRILL_PIN_SCL   17
#define DISP_I2C        i2c1
#define DISP_PIN_SDA    14
#define DISP_PIN_SCL    15
#else
#define TRILL_I2C       i2c0
#define TRILL_PIN_SDA   16
#define TRILL_PIN_SCL   17
#define DISP_I2C        i2c0
#define DISP_PIN_SDA    16
#define DISP_PIN_SCL    17
#endif

/* Back-compat aliases for the original single-bus pin names. */
#define PIN_SDA  TRILL_PIN_SDA
#define PIN_SCL  TRILL_PIN_SCL

#define TRILL_ADDR 0x20
#define DISP_ADDR 0x3C

#define LED_PIN 25
#define CONTROL_0 1
#define CONTROL_1 2
#define CONTROL_2 3
#define CONTROL_3 4
#define CONTROL_4 5
#define CONTROL_5 6
#define NUM_CONTROLS 6
#define KEY_0 7
#define KEY_1 8
#define KEY_2 9
#define KEY_3 10
#define KEY_4 11
#define KEY_5 12
#define KEY_6 13
#define NUM_KEYS 7

#define SCALE_LEN 7
#define MAX_CHORD 6
#define CHORD_NAME_LEN 16
#define DEFAULT_OCTAVE 4
#define DEFAULT_EXTENSIONS 4
#define DEFAULT_VELOCITY 100
#define DEFAULT_INVERSION 0
#define DEFAULT_OCTAVE_COUNT 3

#define MAX_UI_DEPTH 8
#define MAX_UI_NODES 64

#endif