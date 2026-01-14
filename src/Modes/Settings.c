#include "Settings.h"
#include "IO/Midi.h"
#include "Notes/Note.h"
#include "Globals.h"
#include "Rendering/Graphics.h"

uint8_t stack[MAX_UI_DEPTH], depth=0, sel=0, cur=0;

const UINode* curr_node(void) { return &tree[cur]; }

const UINode* sel_child(void) {
    const UINode* n = curr_node();
    if (sel >= n->n_child) return NULL;
    return &tree[n->first_child + sel];
}

const UINode* get_node(uint8_t idx) {
    return &tree[idx];
}


void push(uint8_t idx) {
    if (depth < MAX_UI_DEPTH) {
        stack[depth++] = cur;
        cur = idx;
        sel = 0;
    }
}

uint8_t pop(void) {
    if (depth) cur = stack[--depth];
    sel = 0;
    return cur;
}

int16_t clamp(int16_t val, int16_t min, int16_t max)
{
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

void trigger_boot_sel(const DataU* data)
{
    tc_trigger_bootsel = true;
}

void trigger_factory_reset(const DataU* data)
{
    tc_app = tc_app_default;
}

void trigger_midi_type(const DataU* data)
{
    switch_midi_trs(tc_app.midi_type);
}

void trigger_custom_scale(const DataU* data)
{
    reload_custom_scales();
}

const char* trs_midi_sel[2] = {"Type A", "Type B"};
const char* on_off_sel[2] = {"Off", "On"};
const char* compose_sel[3] = {"Degree",  "Inversion"}; //"Alternative",
const char* degree_sel[9] = {"ERR", "Major", "Minor", "Dominant", "Diminished", "Augmented", "Sus2", "Sus4", "ERR"};
const char* scale_sel[12] = {"ERR", "Major", "Minor", "Dorian", "Phrygian", "Lydian", "Mixolydian", "Locrian", "Custom 0", "Custom 1", "Custom 2", "Custom 3"};
const char* root_names[17] = {
    "C", "C#", "Db", "D", "D#", "Eb", "E", "F", "F#", "Gb", "G", "G#", "Ab", "A", "A#", "Bb", "B"
};

UINode tree[MAX_UI_NODES] = {
    // 0: ROOT "Settings"
    {"Settings", UI_SUBMENU, 6, 1, NULL, {0}, NULL},
    
    // 1-6: First Page
    {"Modes", UI_SUBMENU, 2, 7,  NULL, {0}, NULL},
    {"MIDI", UI_SUBMENU, 3, 17,  NULL, {0}, NULL},
    {"Customize", UI_SUBMENU, 7, 20,  NULL, {0}, NULL},
    {"Load Preset", UI_INT, 8, 0, NULL, {0}, NULL},
    {"Save Preset", UI_INT, 8, 0, NULL, {0}, NULL},
    {"Firmware", UI_SUBMENU, 2, 41, NULL, {0}, NULL},
    
    // 7-8: Modes Settings
    {"Compose", UI_SUBMENU, 2, 9, NULL, {0}, NULL},
    {"Perform", UI_SUBMENU, 6, 11, NULL, {0}, NULL},

    // 9-10: Modes.Compose Settings
    {"Mode",    UI_ENUM, 2, 0, NULL, {.en_val=&tc_app.compose_type}, compose_sel},
    {"Sustain", UI_TOGGLE, 2, 0, NULL, {.toggle=&tc_app.compose_sustain}, on_off_sel},

    // 11-16: Modes.Perform Settings
    {"Pos CC", UI_INT, 128, 0, NULL, {.i_val=&tc_app.perform_pos_cc}, NULL},
    {"Size CC", UI_INT, 128, 0, NULL, {.i_val=&tc_app.perform_size_cc}, NULL},
    {"Pos Default", UI_INT, 128, 0, NULL, {.i_val=&tc_app.perform_pos_default}, NULL},
    {"Size Default", UI_INT, 128, 0, NULL, {.i_val=&tc_app.perform_size_default}, NULL},
    {"Pos Keep", UI_TOGGLE, 2, 0, NULL, {.toggle=&tc_app.perform_reset_pos_on_lift}, on_off_sel},
    {"Size Keep", UI_TOGGLE, 2, 0, NULL, {.toggle=&tc_app.perform_reset_size_on_lift}, on_off_sel},
    
    // 17-19: MIDI Settings
    {"Channel", UI_INT, 16, 0, NULL,  {.i_val=&tc_app.channel}, NULL},
    {"TRS Midi", UI_ENUM, 2, 0, trigger_midi_type, {.en_val=&tc_app.midi_type}, trs_midi_sel},
    {"Velocity", UI_INT, 128, 0, NULL, {.i_val=&tc_app.velocity}, NULL},

    // 20-22 Customize (Buttons)
    {"Button 1", UI_SUBMENU, 2, 27, NULL, {0}, NULL},
    {"Button 2", UI_SUBMENU, 2, 29, NULL, {0}, NULL},
    {"Button 3", UI_SUBMENU, 2, 31, NULL, {0}, NULL},
    //23-26 Customize (Custom Scales)
    {"Custom 0", UI_SUBMENU, 2, 33, NULL, {0}, NULL},
    {"Custom 1", UI_SUBMENU, 2, 35, NULL, {0}, NULL},
    {"Custom 2", UI_SUBMENU, 2, 37, NULL, {0}, NULL},
    {"Custom 3", UI_SUBMENU, 2, 39, NULL, {0}, NULL},

    // 27-28 Customize.B1
    {"Root", UI_ENUM, 17, 0, NULL, {.en_val = &tc_app.key[0].root}, root_names},
    {"Scale", UI_ENUM, 12, 1, NULL, {.en_val = &tc_app.key[0].quality}, scale_sel},
    // 29-30 Customize.B2
    {"Root", UI_ENUM, 17, 0, NULL, {.en_val = &tc_app.key[1].root}, root_names},
    {"Scale", UI_ENUM, 12, 1, NULL, {.en_val = &tc_app.key[1].quality}, scale_sel},
    // 31-32 Customize.B3
    {"Root", UI_ENUM, 17, 0, NULL, {.en_val = &tc_app.key[2].root}, root_names},
    {"Scale", UI_ENUM, 12, 1, NULL, {.en_val = &tc_app.key[2].quality}, scale_sel},

    // 33-34 Customize.C0
    {"Intervals", UI_PER_BUTTON_INT, 12, 0, trigger_custom_scale, {.i_val = tc_app.custom_scale_intervals[0]}, NULL},
    {"Degrees", UI_PER_BUTTON_ENUM, 7, 1, trigger_custom_scale, {.en_val = tc_app.custom_scale_chords[0]}, degree_sel},
    // 35-36 
    {"Intervals", UI_PER_BUTTON_INT, 12, 0, trigger_custom_scale, {.i_val = tc_app.custom_scale_intervals[1]}, NULL},
    {"Degrees", UI_PER_BUTTON_ENUM, 7, 1, trigger_custom_scale, {.en_val = tc_app.custom_scale_chords[1]}, degree_sel},
    // 37-38
    {"Intervals", UI_PER_BUTTON_INT, 12, 0, trigger_custom_scale, {.i_val = tc_app.custom_scale_intervals[2]}, NULL},
    {"Degrees", UI_PER_BUTTON_ENUM, 7, 1, trigger_custom_scale, {.en_val = tc_app.custom_scale_chords[2]}, degree_sel},
    // 39-40 Customize.C3
    {"Intervals", UI_PER_BUTTON_INT, 12, 0, trigger_custom_scale, {.i_val = tc_app.custom_scale_intervals[3]}, NULL},
    {"Degrees", UI_PER_BUTTON_ENUM, 7, 1, trigger_custom_scale, {.en_val = tc_app.custom_scale_chords[3]}, degree_sel},

    // 41-42: Firmware
    {"Reset Factory", UI_TRIGGER, 0, 0, trigger_factory_reset, {0}, NULL},
    {"Firmware Update", UI_TRIGGER, 0, 0, trigger_boot_sel, {0}, NULL},
};


void settings_start()
{
    tc_draw        = &settings_draw;
    tc_update      = &settings_update;
    tc_key_down    = &settings_key_down;
    tc_key_up      = &settings_key_up;
    tc_key_up_independent      = &settings_key_up_independent;
    tc_button_down = &settings_button_down;
    tc_button_up   = &settings_button_up;
    tc_trill_down  = &settings_trill_down;
    tc_trill_up    = &settings_trill_up;

    depth = 0; cur = 0; sel = 0;
}

void settings_end()
{
    
}

int settings_current_key = 0;
void settings_draw()
{
    ssd1306_draw_line(&tc_disp, 0, 7, 128, 7);
    const UINode* cn = curr_node();
    draw_string_top(cn->title);
    switch (cn->type)
    {
        case UI_SUBMENU:
            for (uint8_t i = 0; i < cn->n_child && i < 7; ++i) {
                uint8_t line = 1 + i;
                uint8_t child_idx = cn->first_child + i;
                const UINode* child = get_node(child_idx);

                // Cursor marker
                bool is_selected = (i == sel);
                if (is_selected) {
                    ssd1306_draw_string(&tc_disp, 0, 6 + line * 8, 1, ">");
                }

                ssd1306_draw_string(&tc_disp, 5, 6 + line * 8, 1, child->title);
            }
        break;
        case UI_ENUM:
            if(cn->data.en_val)
                draw_string_center(cn->opts[*cn->data.en_val]);
        break;
        case UI_INT:
            if(cn->data.i_val)
                draw_int_center(*cn->data.i_val);
        break;
        case UI_TOGGLE:
            if(cn->data.toggle)
                draw_string_center(cn->opts[*cn->data.toggle]);
        break;
        case UI_PER_BUTTON_ENUM:
            draw_string_int_centered("Selected Key: ", settings_current_key, 7);
            if(cn->data.en_val)
                draw_string_center(cn->opts[cn->data.en_val[settings_current_key]]);
        break;
        case UI_PER_BUTTON_INT:
            draw_string_int_centered("Selected Key: ", settings_current_key, 7);
            if(cn->data.i_val)
                draw_int_center(cn->data.i_val[settings_current_key]);
        break;
    }
}

void settings_update()
{

}

void settings_key_down(uint8_t key)
{
    settings_current_key = key;
}

void settings_key_up(uint8_t key)
{

}

void settings_key_up_independent(uint8_t key)
{

}

void settings_button_down(uint8_t button)
{
    const UINode* cn = curr_node();
    switch(button)
    {
        case 0: tc_app.mode = TOUCHORD_COMPOSE; break;
        case 1: pop(); break;
        case 2: 
            if(cn->type == UI_SUBMENU)
            {
                const UINode* sn = sel_child();
                if(sn && sn->type == UI_TRIGGER)
                {
                    if (sn->trig) sn->trig(&sn->data);
                }
                else push(cn->first_child + sel);
            }
            else 
            {
                if (cn->trig) cn->trig(&cn->data);
                pop();
            }
        break;
        case 3: tc_app.current_key = 0; break;
        case 4: tc_app.current_key = 1; break;
        case 5: tc_app.current_key = 2; break;
    }
}

void settings_button_double_down(uint8_t button)
{

}

void settings_button_up(uint8_t button)
{

}

int settings_prev_seg = -1;
void settings_trill_down(float pos, float size)
{
    int seg = segments(pos, 10);
    if(settings_prev_seg != -1)
    {
        int delta = (seg - settings_prev_seg);
        if(delta != 0)
        {
            const UINode* cn = curr_node();
            switch (cn->type)
            {
                case UI_SUBMENU:
                    sel = clamp(sel + delta, 0, cn->n_child-1);
                break;
                case UI_ENUM:
                    if(cn->data.en_val)
                        *cn->data.en_val = clamp(*cn->data.en_val - delta, cn->first_child, cn->n_child-1);
                break;
                case UI_INT:
                    if(cn->data.i_val)
                        *cn->data.i_val = clamp(*cn->data.i_val - delta, cn->first_child, cn->n_child-1);
                break;
                case UI_TOGGLE:
                    if(cn->data.toggle)
                        *cn->data.toggle = (delta > 0);
                break;
                case UI_PER_BUTTON_ENUM:
                    if(cn->data.en_val)
                        cn->data.en_val[settings_current_key] = clamp(cn->data.en_val[settings_current_key] - delta, cn->first_child, cn->n_child-1);
                break;
                case UI_PER_BUTTON_INT:
                    if(cn->data.i_val)
                        cn->data.i_val[settings_current_key] = clamp(cn->data.i_val[settings_current_key] - delta, cn->first_child, cn->n_child-1);
                break;
            }
        }
    }
    settings_prev_seg = seg;
}

void settings_trill_up()
{
    settings_prev_seg = -1;
}
