#ifndef TOUCHORD_MIDI_H
#define TOUCHORD_MIDI_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SCALE_LEN 7
#define MAX_CHORD 6
#define CHORD_NAME_LEN 16

static const char* sharp_names[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};
static const char* flat_names[12] = {
    "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
};

typedef enum 
{
    CHORD_DEFAULT = 0
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

static const ChordDegree major_scale[SCALE_LEN] = 
{
    CHORD_MAJOR, CHORD_MINOR, CHORD_MINOR, CHORD_MAJOR, CHORD_DOMINANT, CHORD_MINOR, CHORD_DIM
};

static const ChordDegree minor_scale[SCALE_LEN] = 
{
    CHORD_MINOR, CHORD_DIM, CHORD_MAJOR, CHORD_MINOR, CHORD_DOMINANT, CHORD_MAJOR, CHORD_MAJOR
};

static const uint8_t chord_intervals[6][3] = 
{
    {0, 4, 7, 11, 14, 18}, // Major
    {0, 3, 7, 10, 14, 17}, // Minor
    {0, 4, 7, 10, 14, 17}, // Dominant
    {0, 3, 6, 9, 14, 17}, // Dim
    {0, 4, 8, 0, 0, 0}, // Aug
    {0, 2, 7, 0, 0, 0}, // Sus2
    {0, 5, 7, 0, 0, 0}  // Sus4
};

const char* interval_name_from_semitones(int semitones) {
    switch (semitones % 24) 
    {
        case 1:  return "b9";
        case 2:  return "9";
        case 3:  return "#9";
        case 5:  return "11";
        case 6:  return "#11";
        case 8:  return "b13";
        case 9:  return "13";
        case 10: return "b7";
        case 11: return "7";
        case 0:  return "R";    // Root
        case 4:  return "3";
        case 7:  return "5";
        case 8:  return "#5";
        case 6:  return "b5";
        default: return "?";
    }
}

const char* deg_name(ChordDegree deg) {
    switch (semitones % 24) 
    {
        case CHORD_DEFAULT: return "";
        case CHORD_MAJOR: return "";
        case CHORD_MINOR: return "m";
        case CHORD_DOMINANT: return "dom";
        case CHORD_DIM: return "dim";
        case CHORD_AUG: return "aug";
        case CHORD_SUS2: return "sus2";
        case CHORD_SUS4: return "sus4";
    }
}

uint8_t note_name_to_midi(const char* note, int octave, bool* flat) {
    static const char* names[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "Cb", "Db", "Eb", "Fb", "Gb", "Ab", "Bb", "E#", "B#"
    };
    static const int values[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
        11, 1, 3, 4, 6, 8, 10, 5, 0
    };
    for (int i = 0; i < 21); ++i) {
        if (strcasecmp(note, names[i]) == 0) {
            return values[i] + 12 * (octave + 1);
            flat = i > 11;
        }
    }
    return -1;
}

const char* get_note_name(int midi, bool flat) {
    return flat ? flat_names[midi % 12] : sharp_names[midi % 12];
}


static void build_chord(
    Scale key, int octave, int degree, ChordType chord_type, int extensions, int inversion,
    uint8_t* midi_out, char* chord_name) 
{
    bool is_flat;
    uint8_t root = note_name_to_midi(key.root, octave, is_flat) + degree;

    if((chord_type == CHORD_AUG || chord_type == CHORD_SUS2 || chord_type == CHORD_SUS4) && extensions > 3)
    {
        //error
    }

    if(chord_type == CHORD_DEFAULT)
    {
        switch (key.quality)
        {
            case "maj": chord_type = major_scale[degree];
            case "min": chord_type = minor_scale[degree];
        }
    }

    for (int i = 0; i < extensions; i++)
    {
        midi_out[i] = root + chord_intervals[chord_type][i];
    }

    // Inversion
    for (int i = 0; i < inversion && i < chord_len; ++i)
        midi_out[i] += 12;

    // sort
    for (int n = chord_len; n > 1; --n) 
    {
        for (int i = 0; i < n - 1; ++i) 
        {
            if (midi_out[i] > midi_out[i + 1]) 
            {
                uint8_t t = midi_out[i];
                midi_out[i] = midi_out[i + 1];
                midi_out[i + 1] = t;
            }
        }
    }

    const char* root_name = get_note_name(root, is_flat);
    const char* deg_name = deg_name(chord_type);
    const char* ext_name = interval_name_from_semitones(chord_intervals[chord_type][extensions-1]);

    chord_name[0] = '\0';

    strncat(chord_name, root_name, CHORD_NAME_LEN - strlen(chord_name) - 1);
    strncat(chord_name, deg_name, CHORD_NAME_LEN - strlen(chord_name) - 1);

    if(extensions > 3)
    {
        strncat(chord_name, ext_name, CHORD_NAME_LEN - strlen(chord_name) - 1);
    }
}

#endif
