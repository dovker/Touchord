#include "Compose.h"
#include "Globals.h"
#include "Midi.h"
#include "Notes/Note.h"

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
}

void compose_update()
{

}

void compose_key_down(uint8_t key)
{
    send_midi_chord(tc_app.channel, NOTE_OFF, tc_app.chord, tc_app.prev_extension, tc_app.velocity);
    build_chord(tc_app.key[tc_app.current_key], tc_app.octave, key, tc_app.degree, 
                tc_app.extension_count, tc_app.inversion, tc_app.chord, tc_app.chord_name);
    send_midi_chord(tc_app.channel, NOTE_ON, tc_app.chord, tc_app.extension_count, tc_app.velocity);

    tc_app.prev_extension = tc_app.extension_count;
}

void compose_key_up(uint8_t key)
{
    send_midi_chord(tc_app.channel, NOTE_OFF, tc_app.chord, tc_app.prev_extension, tc_app.velocity);
    
    build_chord(tc_app.key[tc_app.current_key], tc_app.octave, 0, CHORD_DEFAULT, 
                        0, tc_app.inversion, tc_app.chord, tc_app.chord_name);
    tc_app.chord_name[0] = '\0';
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
        case 3: tc_app.current_key = 0; break;
        case 4: tc_app.current_key = 1; break;
        case 5: tc_app.current_key = 2; break;
    }
}

void compose_button_up(uint8_t button)
{

}

void compose_trill_down(float pos, float size)
{
    int seg = segments(pos, 5);
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

void compose_trill_up()
{
    tc_app.extension_count = DEFAULT_EXTENSIONS;
    tc_app.degree = CHORD_DEFAULT;
}
