#ifndef TOUCHORD_TYPES_H
#define TOUCHORD_TYPES_H

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
    TOUCHORD_STRUM
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
    CHORD_SUS4
} ChordDegree;

typedef struct 
{
    const char* root;
    const char* quality;
} Scale;

typedef struct 
{
    Scale key;
    int octave;
    int extension_count;
    int velocity;
    
    TouchordMode mode;
    int octave_count;
    int aftertouch;

    uint8_t current_chord[MAX_CHORD];
    uint8_t current_length;
} TouchordData;

#endif