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

typedef struct 
{
    const char* root;
    const char* quality;
} Scale;

typedef struct 
{
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