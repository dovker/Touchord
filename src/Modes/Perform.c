#include "Perform.h"
#include "Globals.h"
#include "IO/Output.h"
#include "Notes/Note.h"
#include "Rendering/Graphics.h"


void perform_start()
{
    tc_draw        = &perform_draw;
    tc_update      = &perform_update;
    tc_key_down    = &perform_key_down;
    tc_key_up      = &perform_key_up;
    tc_key_up_independent      = &perform_key_up_independent;
    tc_button_down = &perform_button_down;
    tc_button_double_down = &perform_button_double_down;
    tc_button_up   = &perform_button_up;
    tc_trill_down  = &perform_trill_down;
    tc_trill_up    = &perform_trill_up;
}

void perform_end()
{
    tc_output_chord(tc_app.channel, NOTE_OFF, tc_app.chord, tc_app.extension_count, tc_app.velocity);
    tc_output_cc(tc_app.channel, tc_app.perform_pos_cc, tc_app.perform_pos_default);
    tc_output_cc(tc_app.channel, tc_app.perform_size_cc, tc_app.perform_size_default);
    tc_app.chord_name[0] = '\0';
}

void perform_draw()
{
    draw_current_chord();

    ssd1306_draw_line(&tc_disp, 0, 60, 30, 60);
    ssd1306_draw_string_with_font(&tc_disp, 37, 54, 2, font_3x6, "Perform");
    ssd1306_draw_line(&tc_disp, 96, 60, 128, 60);
}

void perform_update()
{

}

void perform_key_down(uint8_t key)
{
    tc_output_chord(tc_app.channel, NOTE_OFF, tc_app.chord, tc_app.extension_count, tc_app.velocity);
    build_chord(tc_app.key, tc_app.octave, key, tc_app.degree, 
                tc_app.extension_count, tc_app.inversion, tc_app.chord, tc_app.chord_name);
    tc_output_chord(tc_app.channel, NOTE_ON, tc_app.chord, tc_app.extension_count, tc_app.velocity);
}

void perform_key_up(uint8_t key)
{
    tc_output_chord(tc_app.channel, NOTE_OFF, tc_app.chord, tc_app.extension_count, tc_app.velocity);

    build_chord(tc_app.key, tc_app.octave, 0, CHORD_DEFAULT, 
                        0, tc_app.inversion, tc_app.chord, tc_app.chord_name);
    tc_app.chord_name[0] = '\0';
}

void perform_key_up_independent(uint8_t key)
{

}

void perform_button_down(uint8_t button)
{
    switch(button)
    {
        case 0: tc_app.mode = TOUCHORD_STRUM; break;
        case 1: 
        if(tc_app.octave > 0) tc_app.octave--;
        break;
        case 2: 
        if(tc_app.octave < 7) tc_app.octave++;
        break;
        default: break;
    }
}

void perform_button_double_down(uint8_t button)
{
    
}

void perform_button_up(uint8_t button)
{

}

void perform_trill_down(float pos, float size)
{
    tc_output_cc(tc_app.channel, tc_app.perform_pos_cc, 127 - (pos * 127));
    tc_output_cc(tc_app.channel, tc_app.perform_size_cc, (127 - (pos * 127))*size);
}

void perform_trill_up()
{
    if(tc_app.perform_reset_pos_on_lift)
        tc_output_cc(tc_app.channel, tc_app.perform_pos_cc, tc_app.perform_pos_default);
    if(tc_app.perform_reset_size_on_lift)
        tc_output_cc(tc_app.channel, tc_app.perform_size_cc, tc_app.perform_size_default);
}
