#ifndef TOUCHORD_TYPES_H
#define TOUCHORD_TYPES_H

#include <stdint.h>
#include "Defines.h"

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

typedef enum 
{
    TOUCHORD_COMPOSE = 0,
    TOUCHORD_PERFORM,
    TOUCHORD_STRUM,
    TOUCHORD_OMNI,
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

#define SCALE_COUNT 12

typedef struct 
{
    char root[4];
    ScaleType quality;
} Scale;

typedef struct 
{
    uint32_t magic;
    Scale key[3];
    int current_key;
    int octave;
    int extension_count;
    int inversion;
    int velocity;
    
    TouchordMode mode;
    int octave_count;
    int cutoff;

    uint8_t chord[MAX_CHORD];
    char chord_name[CHORD_NAME_LEN];
    ChordDegree degree;

    uint8_t prev_extension;
    uint8_t channel;
} TouchordSettings;


#endif