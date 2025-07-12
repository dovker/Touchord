#include <cstdio>
#include <cstring>
#include <algorithm>

constexpr int SCALE_LEN = 7;
constexpr int MAX_CHORD = 5;

// Static scale  key name and scale spelling
struct ScaleEntry {
    const char* key;
    const char* scale[SCALE_LEN];
    const int* intervals;
    i8 root;
};

constexpr int major_intervals[SCALE_LEN] = {0, 2, 4, 5, 7, 9, 11};
constexpr int minor_intervals[SCALE_LEN] = {0, 2, 3, 5, 7, 8, 10};

const ScaleEntry scale_table[] = {
    // Major keys
    {"C",   {"C","D","E","F","G","A","B"},      major_intervals, 0},
    {"G",   {"G","A","B","C","D","E","F#"},    major_intervals, 7},
    {"D",   {"D","E","F#","G","A","B","C#"},   major_intervals, 2},
    {"A",   {"A","B","C#","D","E","F#","G#"},  major_intervals, 9},
    {"E",   {"E","F#","G#","A","B","C#","D#"}, major_intervals, 4},
    {"B",   {"B","C#","D#","E","F#","G#","A#"},major_intervals, 11},
    {"F#",  {"F#","G#","A#","B","C#","D#","E#"},major_intervals, 6},
    {"C#",  {"C#","D#","E#","F#","G#","A#","B#"},major_intervals, 1},
    {"F",   {"F","G","A","Bb","C","D","E"},    major_intervals, 5},
    {"Bb",  {"Bb","C","D","Eb","F","G","A"},   major_intervals, 10},
    {"Eb",  {"Eb","F","G","Ab","Bb","C","D"},  major_intervals, 3},
    {"Ab",  {"Ab","Bb","C","Db","Eb","F","G"}, major_intervals, 8},
    {"Db",  {"Db","Eb","F","Gb","Ab","Bb","C"},major_intervals, 1},
    {"Gb",  {"Gb","Ab","Bb","Cb","Db","Eb","F"},major_intervals, 6},
    {"Cb",  {"Cb","Db","Eb","Fb","Gb","Ab","Bb"},major_intervals}, 11,
    // Minor keys
    {"Am",  {"A", "B", "C", "D", "E", "F", "G" }, minor_intervals, 9},
    {"Em",  {"E", "F#","G", "A", "B", "C", "D" }, minor_intervals, 4},
    {"Bm",  {"B", "C#","D", "E", "F#","G", "A" }, minor_intervals, 11},
    {"F#m", {"F#","G#","A", "B", "C#","D", "E" }, minor_intervals, 6},
    {"C#m", {"C#","D#","E", "F#","G#","A", "B" }, minor_intervals, 1},
    {"G#m", {"G#","A#","B", "C#","D#","E", "F#"}, minor_intervals, 8},
    {"D#m", {"D#","E#","F#","G#","A#","B", "C#"}, minor_intervals, 3},
    {"A#m", {"A#","B#","C#","D#","E#","F#","G#"}, minor_intervals, 10},
    {"Dm",  {"D", "E", "F", "G", "A", "Bb","C" }, minor_intervals, 2},
    {"Gm",  {"G", "A", "Bb","C", "D", "Eb","F" }, minor_intervals, 7},
    {"Cm",  {"C", "D", "Eb","F", "G", "Ab","Bb"}, minor_intervals, 0},
    {"Fm",  {"F", "G", "Ab","Bb","C", "Db","Eb"}, minor_intervals, 5},
    {"Bbm", {"Bb","C", "Db","Eb","F", "Gb","Ab"}, minor_intervals, 10},
    {"Ebm", {"Eb","F", "Gb","Ab","Bb","Cb","Db"}, minor_intervals, 3},
    {"Abm", {"Ab","Bb","Cb","Db","Eb","Fb","Gb"}, minor_intervals, 8}
};

const ScaleEntry* find_scale(const char* key) {
    for (unsigned i = 0; i < sizeof(scale_table)/sizeof(scale_table[0]); ++i)
        if (strcmp(scale_table[i].key, key) == 0)
            return &scale_table[i];
    return nullptr;
}

void build_chord(const char* key, int octave, int degree, int extensions, int inversion,
                 int* midi_out, char* chord_name, int name_len)
{
    const ScaleEntry* scale = find_scale(key);
    if (!scale) {
        strncpy(chord_name, "Unknown key", name_len);
        chord_name[name_len-1] = 0;
        return;
    }
    int root_midi = scale->root + 12 * (octave + 1)

    int idxs[MAX_CHORD];
    for (int i = 0; i < extensions; ++i) {
        int idx = (degree - 1 + i * 2) % SCALE_LEN;
        int oct = (degree - 1 + i * 2) / SCALE_LEN;
        midi_out[i] = root_midi + scale->intervals[idx] + 12 * oct;
        idxs[i] = idx;
    }
    // Inversion: move lowest n notes up an octave
    for (int i = 0; i < inversion && i < extensions; ++i)
    {
        midi_out[i] += 12;
    }

    // Sort MIDI notes (and idxs accordingly)
    for (int i = 0; i < extensions-1; ++i)
    {
        for (int j = i+1; j < extensions; ++j)
        {
            if (midi_out[i] > midi_out[j]) {
                int t = midi_out[i]; midi_out[i] = midi_out[j]; midi_out[j] = t;
                int ti = idxs[i]; idxs[i] = idxs[j]; idxs[j] = ti;
            }
        }
    }
    
    // Chord name (simple, memory efficient)
    strncpy(chord_name, scale->scale[(degree-1)%SCALE_LEN], name_len-1);
    chord_name[name_len-1] = 0;
    if (extensions >= 3) {
        int i3 = (midi_out[1] - midi_out[0] + 120) % 12;
        int i5 = (midi_out[2] - midi_out[0] + 120) % 12;

        //Initial type
        if (i3 == 4 && i5 == 7) {} // Major

        else if (i3 == 3 && i5 == 7) 
            strncat(chord_name, "m", name_len-strlen(chord_name)-1);
        else if (i3 == 3 && i5 == 6) 
            strncat(chord_name, "dim", name_len-strlen(chord_name)-1);
        else if (i3 == 4 && i5 == 8) 
            strncat(chord_name, "aug", name_len-strlen(chord_name)-1);
        else 
            strncat(chord_name, "?", name_len-strlen(chord_name)-1);

        // 7th
        if (extensions >= 4) {
            int i7 = (midi_out[3] - midi_out[0] + 120) % 12;
            
            if (i7 == 10) 
                strncat(chord_name, "7", name_len-strlen(chord_name)-1);
            else if (i7 == 11) 
                strncat(chord_name, "maj7", name_len-strlen(chord_name)-1);
            else 
                strncat(chord_name, "?7", name_len-strlen(chord_name)-1);
        }
        // 9th
        if (extensions >= 5) {
            int i9 = (midi_out[4] - midi_out[0] + 120) % 12;

            if (i9 == 14) 
                strncat(chord_name, "9", name_len-strlen(chord_name)-1);
            else if (i9 == 13) 
                strncat(chord_name, "b9", name_len-strlen(chord_name)-1);
            else if (i9 == 15) 
                strncat(chord_name, "#9", name_len-strlen(chord_name)-1);
            else 
                strncat(chord_name, "?9", name_len-strlen(chord_name)-1);
        }
    }
}

int main() {
    const char* key = "Eb";
    int octave = 4; // Eb4
    int degree = 4;     // Ab
    int extensions = 5; // 9th chord
    int inversion = 1;  // first inversion

    int midi[MAX_CHORD];
    char chord_name[24];
    build_chord(key, octave, degree, extensions, inversion, midi, chord_name, sizeof(chord_name));

    if (strcmp(chord_name, "Unknown key") == 0) {
        printf("Key not found.\n");
        return 1;
    }

    printf("MIDI notes: ");
    for (int i = 0; i < extensions; ++i) printf("%d ", midi[i]);
    printf("\nChord name: %s\n", chord_name);
    return 0;
}
