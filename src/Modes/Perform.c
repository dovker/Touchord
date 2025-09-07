#include "Perform.h"
#include "Globals.h"
#include "Midi.h"
#include "Notes/Note.h"

void perform_start()
{
    tc_draw        = &perform_draw;
    tc_update      = &perform_update;
    tc_key_down    = &perform_key_down;
    tc_key_up      = &perform_key_up;
    tc_button_down = &perform_button_down;
    tc_button_up   = &perform_button_up;
    tc_trill_down  = &perform_trill_down;
    tc_trill_up    = &perform_trill_up;
}

void perform_end()
{
    send_midi_cc(tc_app.channel, MIDI_CUTOFF, DEFAULT_CUTOFF);
    if(tc_touch_state)
        send_midi_cc(tc_app.channel, MIDI_MOD, 0);
}

void perform_draw()
{
    ssd1306_draw_line(&tc_disp, 0, 60, 30, 60);
    ssd1306_draw_string_with_font(&tc_disp, 37, 54, 2, font_3x6, "Perform");
    ssd1306_draw_line(&tc_disp, 96, 60, 128, 60);
}

void perform_update()
{

}

void perform_key_down(uint8_t key)
{
    send_midi_chord(tc_app.channel, NOTE_OFF, tc_app.chord, tc_app.prev_extension, tc_app.velocity);
    build_chord(tc_app.key[tc_app.current_key], tc_app.octave, key, tc_app.degree, 
                tc_app.extension_count, tc_app.inversion, tc_app.chord, tc_app.chord_name);
    send_midi_chord(tc_app.channel, NOTE_ON, tc_app.chord, tc_app.extension_count, tc_app.velocity);
}

void perform_key_up(uint8_t key)
{
    send_midi_chord(tc_app.channel, NOTE_OFF, tc_app.chord, tc_app.prev_extension, tc_app.velocity);

    build_chord(tc_app.key[tc_app.current_key], tc_app.octave, 0, CHORD_DEFAULT, 
                        0, tc_app.inversion, tc_app.chord, tc_app.chord_name);
    tc_app.chord_name[0] = '\0';
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
        case 3: tc_app.current_key = 0; break;
        case 4: tc_app.current_key = 1; break;
        case 5: tc_app.current_key = 2; break;
    }
}

void perform_button_up(uint8_t button)
{

}

void perform_trill_down(float pos, float size)
{
    send_midi_cc(tc_app.channel, MIDI_CUTOFF, 127 - (pos * 127));
    send_midi_cc(tc_app.channel, MIDI_MOD, (127 - (pos * 127))*size);
}

void perform_trill_up()
{
    send_midi_cc(tc_app.channel, MIDI_MOD, 0);
}
