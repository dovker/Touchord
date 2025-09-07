#ifndef TOUCHORD_NOTE_H
#define TOUCHORD_NOTE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "Defines.h"
#include "Types.h"

static const char* sharp_names[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};
static const char* flat_names[12] = {
    "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
};

static const uint8_t major_scale_intervals[SCALE_LEN] = { 0, 2, 4, 5, 7, 9, 11 };
static const uint8_t minor_scale_intervals[SCALE_LEN] = { 0, 2, 3, 5, 7, 8, 10 };

static const ChordDegree major_scale[SCALE_LEN] = 
{
    CHORD_MAJOR, CHORD_MINOR, CHORD_MINOR, CHORD_MAJOR, CHORD_DOMINANT, CHORD_MINOR, CHORD_DIM
};

static const ChordDegree minor_scale[SCALE_LEN] = 
{
    CHORD_MINOR, CHORD_DIM, CHORD_MAJOR, CHORD_MINOR, CHORD_MINOR, CHORD_MAJOR, CHORD_MAJOR
};

static const uint8_t chord_intervals[8][6] = 
{
    {0, 0, 0, 0,  0,  0},
    {0, 4, 7, 11, 14, 18}, // Major
    {0, 3, 7, 10, 14, 17}, // Minor
    {0, 4, 7, 10, 14, 17}, // Dominant
    {0, 3, 6, 10, 14, 17}, // Dim
    {0, 4, 8, 0, 0, 0}, // Aug
    {0, 2, 7, 0, 0, 0}, // Sus2
    {0, 5, 7, 0, 0, 0}  // Sus4
};

const char* interval_name_from_semitones(int semitones);
const char* deg_name(ChordDegree deg);
const char* get_note_name(int midi, bool flat);

uint8_t note_name_to_midi(const char* note, int octave, bool* flat);

void build_chord(Scale key, int octave, int degree, ChordDegree chord_type, int extensions, int inversion,
    uint8_t* midi_out, char* chord_name);

#endif
