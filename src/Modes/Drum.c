#include "Drum.h"
#include "Globals.h"
#include "IO/Output.h"
#include "Notes/Note.h"
#include "Rendering/Graphics.h"


//TODO: ADD INVERSIONS
uint8_t drum_velocity = DEFAULT_VELOCITY;
uint8_t last_note = -1;

void drum_start()
{
    tc_draw = &drum_draw;
    tc_update = &drum_update;
    tc_key_down = &drum_key_down;
    tc_key_up = &drum_key_up;
    tc_key_up_independent = &drum_key_up_independent;
    tc_button_down = &drum_button_down;
    tc_button_double_down = &drum_button_double_down;
    tc_button_up = &drum_button_up;
    tc_trill_down = &drum_trill_down;
    tc_trill_up = &drum_trill_up;

    drum_velocity = tc_app.velocity;
}

void drum_end()
{
    tc_output_note(tc_app.channel, NOTE_OFF, last_note, drum_velocity);
}

void drum_draw()
{
    ssd1306_draw_line(&tc_disp, 0, 60, 44, 60);
    ssd1306_draw_string_with_font(&tc_disp, 50, 54, 2, font_3x6, "Drum");
    ssd1306_draw_line(&tc_disp, 84, 60, 128, 60);

    draw_string_int_centered("Velocity: ", drum_velocity, 2);
    if(last_note != 255)
        draw_int_center(last_note);
}

void drum_update()
{

}

void drum_key_down(uint8_t key)
{
    uint8_t note = 12 * (tc_app.octave + 1) + key;
    tc_output_note(tc_app.channel, NOTE_ON, note, drum_velocity);
    last_note = note;
}

void drum_key_up(uint8_t key)
{
    
}

void drum_key_up_independent(uint8_t key)
{
    tc_output_note(tc_app.channel, NOTE_OFF, 12 * (tc_app.octave + 1) + key, 0);
    last_note = -1;
}

void drum_button_down(uint8_t button)
{
    switch(button)
    {
        case 0: tc_app.mode = TOUCHORD_SETTINGS; break;
        case 1: 
        if(tc_app.octave > 0) tc_app.octave--;
        break;
        case 2: 
        if(tc_app.octave < 7) tc_app.octave++;
        break;
        default: break;
    }
}

void drum_button_double_down(uint8_t button)
{
    
}

void drum_button_up(uint8_t button)
{

}

void drum_trill_down(float pos, float size)
{
    drum_velocity = 127 - (pos * 127);
}

void drum_trill_up()
{
    
}
