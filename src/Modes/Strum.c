#include "Strum.h"
#include "Globals.h"
#include "IO/Midi.h"
#include "Notes/Note.h"
#include "Rendering/Graphics.h"

uint8_t lastNote = 0;
uint8_t prevSegment = -1;

void strum_start()
{
    tc_draw        = &strum_draw;
    tc_update      = &strum_update;
    tc_key_down    = &strum_key_down;
    tc_key_up      = &strum_key_up;
    tc_button_down = &strum_button_down;
    tc_button_double_down = &strum_button_double_down;
    tc_button_up   = &strum_button_up;
    tc_trill_down  = &strum_trill_down;
    tc_trill_up    = &strum_trill_up;
}

void strum_end()
{
    send_midi_note(tc_app.channel, NOTE_OFF, lastNote, tc_app.velocity);
    tc_app.chord_name[0] = '\0';
}

void strum_draw()
{
    draw_current_chord();

    ssd1306_draw_line(&tc_disp, 0, 60, 40, 60);
    ssd1306_draw_string_with_font(&tc_disp, 45, 54, 2, font_3x6, "Strum");
    ssd1306_draw_line(&tc_disp, 88, 60, 128, 60);
}

void strum_update()
{

}


void strum_key_down(uint8_t key)
{
    build_chord(tc_app.key[tc_app.current_key], tc_app.octave, key, tc_app.degree, 
                tc_app.extension_count, tc_app.inversion, tc_app.chord, tc_app.chord_name);
}

void strum_key_up(uint8_t key)
{

}

void strum_button_down(uint8_t button)
{
    switch(button)
    {
        case 0: tc_app.mode = TOUCHORD_OMNI; break;
        case 1: 
            if(tc_app.octave > 1) 
            {
            tc_app.octave--;
            build_chord(tc_app.key[tc_app.current_key], tc_app.octave, tc_last_key, tc_app.degree, 
                tc_app.extension_count, tc_app.inversion, tc_app.chord, tc_app.chord_name);
            }
        break;
        case 2: 
            if(tc_app.octave < 7) 
            {
                tc_app.octave++;
                build_chord(tc_app.key[tc_app.current_key], tc_app.octave, tc_last_key, tc_app.degree, 
                tc_app.extension_count, tc_app.inversion, tc_app.chord, tc_app.chord_name);
            }
        break;
        case 3: tc_app.current_key = 0; break;
        case 4: tc_app.current_key = 1; break;
        case 5: tc_app.current_key = 2; break;
    }
}

void strum_button_double_down(uint8_t button)
{
    switch(button)
    {
        case 1: 
            if(tc_app.extension_count > 1) 
            {
                tc_app.extension_count--;
            }
        break;
        case 2: 
            if(tc_app.extension_count < 4) 
            {
                tc_app.extension_count++;
            }
        break;
    }
}

void strum_button_up(uint8_t button)
{

}


void strum_trill_down(float pos, float size)
{
    int seg = tc_app.extension_count * tc_app.octave_count - segments(pos, tc_app.extension_count * tc_app.octave_count);
    if(!tc_touch_state)
    {
        uint8_t note = tc_app.chord[seg%tc_app.extension_count];
        uint8_t octave = seg / tc_app.extension_count;
        lastNote = note + octave * 12;
        send_midi_note(tc_app.channel, NOTE_ON, lastNote, tc_app.velocity);
    }
    else if(tc_touch_state && seg != prevSegment)
    {
        send_midi_note(tc_app.channel, NOTE_OFF, lastNote, tc_app.velocity);
        uint8_t note = tc_app.chord[seg%tc_app.extension_count];
        uint8_t octave = seg / tc_app.extension_count;
        lastNote = note + octave * 12;
        send_midi_note(tc_app.channel, NOTE_ON, lastNote, tc_app.velocity);
    }
    prevSegment = seg;
}

void strum_trill_up()
{
    send_midi_note(tc_app.channel, NOTE_OFF, lastNote, tc_app.velocity);
}
