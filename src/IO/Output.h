#ifndef TOUCHORD_OUTPUT_H
#define TOUCHORD_OUTPUT_H

#include <stdint.h>

void tc_output_init(void);

void tc_output_note(uint8_t channel, uint8_t status, uint8_t note, uint8_t velocity);
void tc_output_chord(uint8_t channel, uint8_t status, const uint8_t *notes, uint8_t length, uint8_t velocity);
void tc_output_cc(uint8_t channel, uint8_t cc_num, uint8_t cc_value);
void tc_output_program_change(uint8_t channel, uint8_t program);
void tc_output_pitch_bend(uint8_t channel, uint16_t bend);
void tc_output_pedal(uint8_t channel, uint8_t value);
void tc_output_all_notes_off(uint8_t channel);

#endif
