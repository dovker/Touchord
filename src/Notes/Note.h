/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_NOTE_H
#define TOUCHORD_NOTE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "Defines.h"
#include "Types.h"
#include "Globals.h"

extern const char* sharp_names[12];
extern const char* flat_names[12];
extern const char* root_names[17];   /* Note enum -> spelled name (C, C#, Db, ...) */

extern uint8_t scale_intervals[SCALE_COUNT][SCALE_LEN];
extern ChordDegree scale_chords[SCALE_COUNT][SCALE_LEN];

extern const char* scale_names[SCALE_COUNT];
extern const uint8_t chord_intervals[8][6];

void reload_custom_scales();

const char* interval_name_from_semitones(int semitones);
const char* deg_name(ChordDegree deg);
const char* get_note_name(int midi, bool flat);

uint8_t note_name_to_midi(const char* note, int octave, bool* flat);
uint8_t note_to_midi(Note note, int octave, bool* flat);

void build_chord(Scale key, int octave, int degree, ChordDegree chord_type, int extensions, int inversion,
    uint8_t* midi_out, char* chord_name);

static inline uint8_t chord_note_at(const uint8_t* chord, int seg, int ext)
{
    int n = chord[seg % ext] + (seg / ext) * 12;
    return n > 127 ? 127 : (uint8_t)n;
}

void sort_chord_notes(uint8_t* arr, int len);

void build_jazz_chord(Scale key, int octave, int degree, int sub_seg,
    int extensions, uint8_t* out, char* name);

void apply_voice_leading(uint8_t* chord, int extensions, const uint8_t* prev);

#endif
