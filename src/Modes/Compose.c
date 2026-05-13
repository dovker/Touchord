#include "Compose.h"
#include "Globals.h"
#include "IO/Output.h"
#include "Notes/Note.h"
#include "Rendering/Graphics.h"


//TODO: ADD INVERSIONS
uint8_t compose_last_degree = -1;
uint8_t compose_last_extension;
uint8_t compose_extension;
static int compose_test_value = 50;

void compose_start()
{
    tc_draw = &compose_draw;
    tc_update = &compose_update;
    tc_key_down = &compose_key_down;
    tc_key_up = &compose_key_up;
    tc_key_up_independent = &compose_key_up_independent;
    tc_button_down = &compose_button_down;
    tc_button_double_down = &compose_button_double_down;
    tc_button_up = &compose_button_up;
    tc_trill_down = &compose_trill_down;
    tc_trill_up = &compose_trill_up;

    compose_last_extension = tc_app.extension_count;
    compose_extension = tc_app.extension_count;
}

void compose_end()
{
    tc_output_chord(tc_app.channel, NOTE_OFF, tc_app.chord, compose_last_extension, tc_app.velocity);
    tc_app.chord_name[0] = '\0';
}

void compose_draw()
{
    draw_current_chord();
    draw_string_int_centered("Enc: ", compose_test_value, 40);

    ssd1306_draw_line(&tc_disp, 0, 60, 30, 60);
    ssd1306_draw_string_with_font(&tc_disp, 37, 54, 2, font_3x6, "Compose");
    ssd1306_draw_line(&tc_disp, 96, 60, 128, 60);
    //TODO: For different inner modes draw something different
}

void compose_update()
{

}

void compose_key_down(uint8_t key)
{
    compose_last_degree = key;
    tc_output_chord(tc_app.channel, NOTE_OFF, tc_app.chord, compose_last_extension, tc_app.velocity);
    build_chord(tc_app.key, tc_app.octave, key, tc_app.degree, 
                compose_extension, tc_app.inversion, tc_app.chord, tc_app.chord_name);
    tc_output_chord(tc_app.channel, NOTE_ON, tc_app.chord, compose_extension, tc_app.velocity);

    compose_last_extension = compose_extension;
}

void compose_key_up(uint8_t key)
{
    compose_last_degree = -1;
    if(!tc_app.compose_sustain)
    {
        tc_output_chord(tc_app.channel, NOTE_OFF, tc_app.chord, compose_last_extension, tc_app.velocity);
        
        build_chord(tc_app.key, tc_app.octave, 0, CHORD_DEFAULT, 
                            0, tc_app.inversion, tc_app.chord, tc_app.chord_name);
        tc_app.chord_name[0] = '\0';
    }
}

void compose_key_up_independent(uint8_t key)
{

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
}

void compose_button_double_down(uint8_t button)
{
    
}

void compose_button_up(uint8_t button)
{

}

int prev_seg = -1;
void compose_trill_down(float pos, float size)
{
    if(tc_app.chord_name[0] != '\0')
    {
        compose_last_extension = compose_extension;
        int seg = segments(pos, 5);
        switch(tc_app.compose_type)
        {
            case COMPOSE_DEGREE:
            {
                switch (seg)
                {
                    case 0: 
                        compose_extension = 6; 
                        tc_app.degree = CHORD_DEFAULT;
                        break;
                    case 1: 
                        compose_extension = 5; 
                        tc_app.degree = CHORD_DEFAULT;
                        break;
                    case 2: 
                        compose_extension = 4; 
                        tc_app.degree = CHORD_DEFAULT;
                        break;
                    case 3: 
                        compose_extension = 4; 
                        tc_app.degree = CHORD_PARALLEL;
                        break;
                    case 4: 
                        compose_extension = 3; 
                        tc_app.degree = CHORD_PARALLEL;
                        break;
                }
            } break;
            // case COMPOSE_SUS:
            // {
            //     compose_extension = 3;
            //     switch (seg)
            //     {
            //         case 0: 
            //             tc_app.degree = CHORD_DOMINANT;
            //             break;
            //         case 1: 
            //             tc_app.degree = CHORD_DIM;
            //             break;
            //         case 2: 
            //             tc_app.degree = CHORD_AUG;
            //             break;
            //         case 3: 
            //             tc_app.degree = CHORD_SUS2;
            //             break;
            //         case 4: 
            //             tc_app.degree = CHORD_SUS4;
            //             break;
            //     }
            // } break;
            case COMPOSE_INV:
            {
                seg = tc_app.extension_count - segments(pos, tc_app.extension_count);
                tc_app.inversion = seg;
            }
        }
        if(prev_seg != seg)
        {
            uint8_t prev_chord[6];
            memcpy(prev_chord, tc_app.chord, 6);
            build_chord(tc_app.key, tc_app.octave, compose_last_degree, tc_app.degree, 
                            compose_extension, tc_app.inversion, tc_app.chord, tc_app.chord_name);

            for(int i = 0; i < MAX_CHORD; i++)
            {
                if(tc_app.chord[i] != prev_chord[i])
                {
                    if(prev_chord[i] != 0)
                        tc_output_note(tc_app.channel, NOTE_OFF, prev_chord[i], tc_app.velocity);
                    if(tc_app.chord[i] != 0)
                        tc_output_note(tc_app.channel, NOTE_ON, tc_app.chord[i], tc_app.velocity);
                }
            }
            compose_last_extension = compose_extension;
        }
        prev_seg = seg;
    }
}

void compose_trill_up()
{
    compose_last_extension = compose_extension;
    compose_extension = tc_app.extension_count;
    tc_app.degree = CHORD_DEFAULT;
}

void compose_adjust_test_value(int delta)
{
    compose_test_value += delta;
    if (compose_test_value < 0) {
        compose_test_value = 0;
    } else if (compose_test_value > 100) {
        compose_test_value = 100;
    }
}
