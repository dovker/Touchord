#include "Output.h"

#include "Globals.h"
#include "Midi.h"
#include "Synth/AmyEngine.h"

static bool tc_output_external_enabled(void)
{
    if (tc_app.mode == TOUCHORD_DRUM) {
        return true;
    }

    return tc_app.output_mode != TOUCHORD_OUTPUT_INTERNAL;
}

static bool tc_output_internal_enabled(void)
{
    if (tc_app.mode == TOUCHORD_DRUM) {
        return false;
    }

    return tc_app.output_mode != TOUCHORD_OUTPUT_EXTERNAL;
}

void tc_output_init(void)
{
    tc_synth_init();
}

void tc_output_note(uint8_t channel, uint8_t status, uint8_t note, uint8_t velocity)
{
    if (status == NOTE_ON && velocity != 0) {
        tc_debug_last_output_note = (int8_t)note;
        tc_debug_output_note_count++;
    }

    if (tc_output_external_enabled()) {
        send_midi_note(channel, status, note, velocity);
    }

    if (!tc_output_internal_enabled()) {
        return;
    }

    if (status == NOTE_ON && velocity != 0) {
        tc_synth_note_on(note, velocity);
    } else {
        tc_synth_note_off(note);
    }
}

void tc_output_chord(uint8_t channel, uint8_t status, const uint8_t *notes, uint8_t length, uint8_t velocity)
{
    for (uint8_t i = 0; i < length; ++i) {
        if (notes[i] == 0) {
            continue;
        }

        tc_output_note(channel, status, notes[i], velocity);
    }
}

void tc_output_cc(uint8_t channel, uint8_t cc_num, uint8_t cc_value)
{
    if (tc_output_external_enabled()) {
        send_midi_cc(channel, cc_num, cc_value);
    }

    if (tc_output_internal_enabled()) {
        tc_synth_control_change(cc_num, cc_value);
    }
}

void tc_output_program_change(uint8_t channel, uint8_t program)
{
    if (tc_output_external_enabled()) {
        send_midi_program_change(channel, program);
    }

    if (tc_output_internal_enabled()) {
        tc_synth_program_change(program);
    }
}

void tc_output_pitch_bend(uint8_t channel, uint16_t bend)
{
    if (tc_output_external_enabled()) {
        send_midi_pitch_bend(channel, bend);
    }

    if (tc_output_internal_enabled()) {
        tc_synth_pitch_bend(bend);
    }
}

void tc_output_pedal(uint8_t channel, uint8_t value)
{
    if (tc_output_external_enabled()) {
        send_midi_pedal(channel, value);
    }

    if (tc_output_internal_enabled()) {
        tc_synth_pedal(value);
    }
}

void tc_output_all_notes_off(uint8_t channel)
{
    send_midi_all_notes_off(channel);
    tc_synth_all_notes_off();
}
