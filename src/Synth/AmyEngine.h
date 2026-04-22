#ifndef TOUCHORD_AMY_ENGINE_H
#define TOUCHORD_AMY_ENGINE_H

#include <stdbool.h>
#include <stdint.h>

void tc_synth_init(void);
void tc_synth_audio_task(void);
bool tc_synth_is_ready(void);
void tc_synth_set_test_tone(bool enabled);
bool tc_synth_test_tone_enabled(void);

void tc_synth_note_on(uint8_t note, uint8_t velocity);
void tc_synth_note_off(uint8_t note);
void tc_synth_control_change(uint8_t control, uint8_t value);
void tc_synth_program_change(uint8_t program);
void tc_synth_pitch_bend(uint16_t bend);
void tc_synth_pedal(uint8_t value);
void tc_synth_all_notes_off(void);

#endif
