#include "Note.h"
#include <string.h>

const char* interval_name_from_semitones(int semitones) {
    switch (semitones % 12) 
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
        default: return "?";
    }
}

const char* deg_name(ChordDegree deg) {
    switch (deg) 
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
    for (int i = 0; i < 21; ++i) {
        if (strcmp(note, names[i]) == 0) {
            *flat = i > 11;
            return values[i] + 12 * (octave + 1);
        }
    }
    return -1;
}

const char* get_note_name(int midi, bool flat) {
    return flat ? flat_names[midi % 12] : sharp_names[midi % 12];
}


void build_chord(
    Scale key, int octave, int degree, ChordDegree chord_type, int extensions, int inversion,
    uint8_t* midi_out, char* chord_name) 
{
    for(int i = 0; i < MAX_CHORD; i++)
    {
        midi_out[i] = 0;
    }
    bool is_flat;
    uint8_t root = note_name_to_midi(key.root, octave, &is_flat);
    if(!strcmp(key.quality, "maj"))
    {
        if(chord_type != CHORD_PARALLEL)
            root += major_scale_intervals[degree];
        else root += minor_scale_intervals[degree];

    }
    else if(!strcmp(key.quality, "min"))
    {
        if(chord_type != CHORD_PARALLEL)
        {
            root += minor_scale_intervals[degree];
            if(!strcmp(key.root, "C"))
                is_flat = true;
        }
        else 
            root += major_scale_intervals[degree];
    }

    if((chord_type == CHORD_AUG || chord_type == CHORD_SUS2 || chord_type == CHORD_SUS4))
    {
        extensions = 3;
    }

    if(chord_type == CHORD_DEFAULT)
    {
        if(!strcmp(key.quality, "maj"))
        {
            chord_type = major_scale[degree];
        }
        else if(!strcmp(key.quality, "min"))
        {
            chord_type = minor_scale[degree];
        }
    } else if (chord_type == CHORD_PARALLEL)
    {
        if(!strcmp(key.quality, "min"))
        {
            chord_type = major_scale[degree];
        }
        else if(!strcmp(key.quality, "maj"))
        {
            chord_type = minor_scale[degree];
        }
    }

    for (int i = 0; i < extensions; i++)
    {
        midi_out[i] = root + chord_intervals[chord_type][i];
    }

    // Inversion
    for (int i = 0; i < inversion && i < extensions; ++i)
        midi_out[i] += 12;

    // sort
    for (int n = extensions; n > 1; --n) 
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
    const char* degree_name = deg_name(chord_type);
    const char* ext_name = interval_name_from_semitones(chord_intervals[chord_type][extensions-1]);

    chord_name[0] = '\0';

    strncat(chord_name, root_name, CHORD_NAME_LEN - strlen(chord_name) - 1);
    strncat(chord_name, degree_name, CHORD_NAME_LEN - strlen(chord_name) - 1);

    if(extensions > 3)
    {
        strncat(chord_name, ext_name, CHORD_NAME_LEN - strlen(chord_name) - 1);
    }
}