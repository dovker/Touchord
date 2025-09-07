#include "Omni.h"
#include "Globals.h"
#include "Midi.h"
#include "Notes/Note.h"

uint8_t omniLastNote = 0;
uint8_t omniPrevSegment = -1;

void omni_start()
{
    tc_draw        = &omni_draw;
    tc_update      = &omni_update;
    tc_key_down    = &omni_key_down;
    tc_key_up      = &omni_key_up;
    tc_button_down = &omni_button_down;
    tc_button_up   = &omni_button_up;
    tc_trill_down  = &omni_trill_down;
    tc_trill_up    = &omni_trill_up;
}

uint8_t playingRoot = 0;
uint8_t playingFifth = 0;
void omni_end()
{
    send_midi_note(tc_app.channel, NOTE_OFF, omniLastNote, tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_OFF, playingRoot, tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_OFF, playingFifth, tc_app.velocity);
}

void omni_draw()
{
    ssd1306_draw_line(&tc_disp, 0, 60, 40, 60);
    ssd1306_draw_string_with_font(&tc_disp, 45, 54, 2, font_3x6, "Omni>");
    ssd1306_draw_line(&tc_disp, 88, 60, 128, 60);
}

void omni_update()
{

}


void omni_key_down(uint8_t key)
{
    send_midi_note(tc_app.channel, NOTE_OFF, playingRoot, tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_OFF, playingFifth, tc_app.velocity);

    build_chord(tc_app.key[tc_app.current_key], tc_app.octave, key, tc_app.degree, 
                tc_app.extension_count, tc_app.inversion, tc_app.chord, tc_app.chord_name);

    playingRoot = tc_app.chord[0] - 12;
    playingFifth = tc_app.chord[1] - 12;

    send_midi_note(tc_app.channel, NOTE_ON, playingRoot, tc_app.velocity);
    send_midi_note(tc_app.channel, NOTE_ON, playingFifth, tc_app.velocity);
}

void omni_key_up(uint8_t key)
{
    // send_midi_note(tc_app.channel, NOTE_OFF, playingRoot, tc_app.velocity);
    // send_midi_note(tc_app.channel, NOTE_OFF, playingFifth, tc_app.velocity);
}

void omni_button_down(uint8_t button)
{
    switch(button)
    {
        case 0: tc_app.mode = TOUCHORD_COMPOSE; break;
        case 1: 
        if(tc_app.octave > 1) tc_app.octave--;
        break;
        case 2: 
        if(tc_app.octave < 7) tc_app.octave++;
        break;
        case 3: tc_app.current_key = 0; break;
        case 4: tc_app.current_key = 1; break;
        case 5: tc_app.current_key = 2; break;
    }
}

void omni_button_up(uint8_t button)
{

}


void omni_trill_down(float pos, float size)
{
    int seg = tc_app.extension_count * tc_app.octave_count - segments(pos, tc_app.extension_count * tc_app.octave_count);
    if(!tc_touch_state)
    {
        uint8_t note = tc_app.chord[seg%tc_app.extension_count];
        uint8_t octave = seg / tc_app.extension_count;
        omniLastNote = note + octave * 12;
        send_midi_note(tc_app.channel, NOTE_ON, omniLastNote, tc_app.velocity);
    }
    else if(tc_touch_state && seg != omniPrevSegment)
    {
        send_midi_note(tc_app.channel, NOTE_OFF, omniLastNote, tc_app.velocity);
        uint8_t note = tc_app.chord[seg%tc_app.extension_count];
        uint8_t octave = seg / tc_app.extension_count;
        omniLastNote = note + octave * 12;
        send_midi_note(tc_app.channel, NOTE_ON, omniLastNote, tc_app.velocity);
    }
    omniPrevSegment = seg;
}

void omni_trill_up()
{
    send_midi_note(tc_app.channel, NOTE_OFF, omniLastNote, tc_app.velocity);
}
