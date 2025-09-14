#include "Compose.h"
#include "Globals.h"
#include "Midi.h"
#include "Notes/Note.h"


bool compose_default = true;
bool compose_sustain = false;
uint8_t compose_last_degree = -1;
void compose_start()
{
    tc_draw = &compose_draw;
    tc_update = &compose_update;
    tc_key_down = &compose_key_down;
    tc_key_up = &compose_key_up;
    tc_button_down = &compose_button_down;
    tc_button_up = &compose_button_up;
    tc_trill_down = &compose_trill_down;
    tc_trill_up = &compose_trill_up;
}

void compose_end()
{
    send_midi_chord(tc_app.channel, NOTE_OFF, tc_app.chord, tc_app.prev_extension, tc_app.velocity);
}

void compose_draw()
{
    ssd1306_draw_line(&tc_disp, 0, 60, 30, 60);
    ssd1306_draw_string_with_font(&tc_disp, 37, 54, 2, font_3x6, "Compose");
    ssd1306_draw_line(&tc_disp, 96, 60, 128, 60);
    if(!compose_default)
        ssd1306_draw_line(&tc_disp, 34, 52, 92, 52);
}

void compose_update()
{

}

void compose_key_down(uint8_t key)
{
    compose_last_degree = key;
    send_midi_chord(tc_app.channel, NOTE_OFF, tc_app.chord, tc_app.prev_extension, tc_app.velocity);
    build_chord(tc_app.key[tc_app.current_key], tc_app.octave, key, tc_app.degree, 
                tc_app.extension_count, tc_app.inversion, tc_app.chord, tc_app.chord_name);
    send_midi_chord(tc_app.channel, NOTE_ON, tc_app.chord, tc_app.extension_count, tc_app.velocity);

    tc_app.prev_extension = tc_app.extension_count;
}

void compose_key_up(uint8_t key)
{
    if(!compose_sustain)
    {
        compose_last_degree = -1;
        send_midi_chord(tc_app.channel, NOTE_OFF, tc_app.chord, tc_app.prev_extension, tc_app.velocity);
        
        build_chord(tc_app.key[tc_app.current_key], tc_app.octave, 0, CHORD_DEFAULT, 
                            0, tc_app.inversion, tc_app.chord, tc_app.chord_name);
        tc_app.chord_name[0] = '\0';
    }
}

void compose_button_down(uint8_t button)
{
    switch(button)
    {
        case 0: tc_app.mode = TOUCHORD_PERFORM; break;
        case 1: 
        if(tc_app.octave > 0) tc_app.octave--;
        break;
        case 2: 
        if(tc_app.octave < 7) tc_app.octave++;
        break;
        default: break;
    }
    if(button > 2)
    {
        uint8_t prev_key = tc_app.current_key;
        tc_app.current_key = button - 3;
        if(prev_key == tc_app.current_key)
            compose_default = !compose_default;
    }
}

void compose_button_up(uint8_t button)
{

}

int prev_seg = -1;
void compose_trill_down(float pos, float size)
{
    uint8_t prev_ext_count = tc_app.extension_count;
    int seg = segments(pos, 5);
    if(compose_default)
    {
        switch (seg)
        {
            case 0: 
                tc_app.extension_count = 6; 
                tc_app.degree = CHORD_DEFAULT;
                break;
            case 1: 
                tc_app.extension_count = 5; 
                tc_app.degree = CHORD_DEFAULT;
                break;
            case 2: 
                tc_app.extension_count = 4; 
                tc_app.degree = CHORD_DEFAULT;
                break;
            case 3: 
                tc_app.extension_count = 4; 
                tc_app.degree = CHORD_PARALLEL;
                break;
            case 4: 
                tc_app.extension_count = 3; 
                tc_app.degree = CHORD_PARALLEL;
                break;
        }
    }
    else 
    {
        tc_app.extension_count = 3;
        switch (seg)
        {
            case 0: 
                tc_app.degree = CHORD_DOMINANT;
                break;
            case 1: 
                tc_app.degree = CHORD_DIM;
                break;
            case 2: 
                tc_app.degree = CHORD_AUG;
                break;
            case 3: 
                tc_app.degree = CHORD_SUS2;
                break;
            case 4: 
                tc_app.degree = CHORD_SUS4;
                break;
        }
    }
    if(tc_app.chord_name[0] != '\0' && prev_seg != seg)
    {
        uint8_t prev_chord[6];
        memcpy(prev_chord, tc_app.chord, 6);
        build_chord(tc_app.key[tc_app.current_key], tc_app.octave, compose_last_degree, tc_app.degree, 
                    tc_app.extension_count, tc_app.inversion, tc_app.chord, tc_app.chord_name);

        for(int i = 0; i < MAX_CHORD; i++)
        {
            if(tc_app.chord[i] != prev_chord[i])
            {
                if(prev_chord[i] != 0)
                    send_midi_note(tc_app.channel, NOTE_OFF, prev_chord[i], tc_app.velocity);
                if(tc_app.chord[i] != 0)
                    send_midi_note(tc_app.channel, NOTE_ON, tc_app.chord[i], tc_app.velocity);
            }
        }
        tc_app.prev_extension = tc_app.extension_count;
    }
    prev_seg = seg;
}

void compose_trill_up()
{
    tc_app.extension_count = DEFAULT_EXTENSIONS;
    tc_app.degree = CHORD_DEFAULT;
}
