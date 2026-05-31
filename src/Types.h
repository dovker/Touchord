/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_TYPES_H
#define TOUCHORD_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "Defines.h"

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

typedef enum
{
    NOTE_C = 0,
    NOTE_C_SHARP,
    NOTE_D_FLAT,
    NOTE_D,
    NOTE_D_SHARP,
    NOTE_E_FLAT,
    NOTE_E,
    NOTE_F,
    NOTE_F_SHARP,
    NOTE_G_FLAT,
    NOTE_G,
    NOTE_G_SHARP,
    NOTE_A_FLAT,
    NOTE_A,
    NOTE_A_SHARP,
    NOTE_B_FLAT,
    NOTE_B,
} Note;

typedef enum
{
    TOUCHORD_COMPOSE = 0,
    TOUCHORD_PERFORM,
    TOUCHORD_STRUM,
    TOUCHORD_OMNI,
    TOUCHORD_DRUM,
    TOUCHORD_SETTINGS
} TouchordMode;

typedef enum
{
    CHORD_DEFAULT = 0,
    CHORD_MAJOR,
    CHORD_MINOR,
    CHORD_DOMINANT,
    CHORD_DIM,
    CHORD_AUG,
    CHORD_SUS2,
    CHORD_SUS4,
    CHORD_PARALLEL
} ChordDegree;

typedef enum
{
    MIDI_TRS_A = 0,
    MIDI_TRS_B
} MidiType;

typedef enum
{
    SCALE_NULL = 0,
    SCALE_MAJOR,
    SCALE_MINOR,
    SCALE_DORIAN,
    SCALE_PHRYGIAN,
    SCALE_LYDIAN,
    SCALE_MIXOLYDIAN,
    SCALE_LOCRIAN,
    SCALE_CUSTOM0,
    SCALE_CUSTOM1,
    SCALE_CUSTOM2,
    SCALE_CUSTOM3
} ScaleType;

typedef enum
{
    COMPOSE_DEGREE = 0,
    // COMPOSE_SUS,
    COMPOSE_INV,
    COMPOSE_JAZZ
} ComposeType;

#define SCALE_COUNT 12

typedef struct
{
    Note root;
    ScaleType quality;
} Scale;

typedef struct
{
    uint32_t magic;
    uint16_t schema_version;
    uint16_t struct_size;
    Scale key[3];
    uint8_t current_key;
    uint8_t octave;
    uint8_t extension_count;
    uint8_t inversion;
    uint8_t velocity;

    TouchordMode mode;
    uint8_t octave_count;
    uint8_t cutoff;

    uint8_t chord[MAX_CHORD];
    char chord_name[CHORD_NAME_LEN];
    ChordDegree degree;

    uint8_t prev_extension;
    uint8_t channel;
    MidiType midi_type;

    ChordDegree custom_scale_chords[4][SCALE_LEN];
    uint8_t custom_scale_intervals[4][SCALE_LEN];

    ComposeType compose_type;
    bool compose_sustain;
    bool compose_voice_lead;

    uint8_t perform_pos_cc;
    uint8_t perform_size_cc;
    uint8_t perform_pos_default;
    uint8_t perform_size_default;
    bool perform_reset_pos_on_lift;
    bool perform_reset_size_on_lift;
} TouchordSettings;

typedef enum
{
    UI_SUBMENU = 0, // Select Setting
    UI_INT, //Select int value
    UI_ENUM, //Select Enum value
    UI_TOGGLE, // On/Off
    UI_TRIGGER, //Automatically starts
    UI_PER_BUTTON_INT,
    UI_PER_BUTTON_ENUM
} UINodeType;

typedef union Data {
    bool* toggle;
    uint8_t* en_val;
    uint8_t* i_val;
    void* custom;
} DataU;

typedef void (*TrigFn)(const DataU*);

typedef struct Node {
    const char* title;
    UINodeType type;
    uint8_t n_child; //Can be used as Range
    uint8_t first_child;  // tree[] index offset, can be used as starting value
    TrigFn trig;
    DataU data;
    const char* const * opts;  // ENUM strings
} UINode;

#endif