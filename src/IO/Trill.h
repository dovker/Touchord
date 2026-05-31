/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TRILL_BAR_H
#define TRILL_BAR_H

//Driver Inspired by https://github.com/Heerkog/MicroPythonTrill

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

#define TRILL_REG_COMMAND 0x00
#define TRILL_REG_DATA 0x04

#define TRILL_COMMAND_NONE 0x00
#define TRILL_COMMAND_MODE 0x01
#define TRILL_COMMAND_SCAN_SETTINGS 0x02
#define TRILL_COMMAND_PRESCALER 0x03
#define TRILL_COMMAND_NOISE_THRESHOLD 0x04
#define TRILL_COMMAND_IDAC 0x05
#define TRILL_COMMAND_BASELINE_UPDATE 0x06
#define TRILL_COMMAND_MINIMUM_SIZE 0x07
#define TRILL_COMMAND_AUTO_SCAN_INTERVAL 0x10
#define TRILL_COMMAND_IDENTIFY 0xFF

#define TRILL_MODE_CENTROID 0x00
#define TRILL_MODE_RAW 0x01
#define TRILL_MODE_BASELINE 0x02
#define TRILL_MODE_DIFF 0x03

#define TRILL_MAX_TOUCHES 5
#define TRILL_MAX_POS 3200
#define TRILL_MAX_SIZE 0xFFFF
#define TRILL_SIZE_THRESHOLD 5000
#define TRILL_SLEEP 25

typedef struct {
    float pos;
    float size;
} Touch;

typedef struct {
    i2c_inst_t *i2c_i;
    uint8_t address;
    uint8_t mode;
    uint16_t data[TRILL_MAX_TOUCHES * 2];
    uint16_t raw_data[26];
} TrillBar;

void trill_writeto(TrillBar* bar, uint8_t* data, uint8_t len);
void trill_writeto_mem(TrillBar* bar, uint8_t reg, uint8_t* data, uint8_t len);
void trill_readfrom(TrillBar* bar, uint8_t* data, uint8_t len);
void trill_readfrom_mem8(TrillBar* bar, uint8_t reg, uint8_t* data, uint8_t len);
void trill_readfrom_mem16(TrillBar* bar, uint8_t reg, uint16_t* data, uint8_t len);
void trill_identify(TrillBar* bar);
void trill_read(TrillBar* bar);
void trill_set_mode(TrillBar* bar, uint8_t mode);
void trill_set_noise_threshold(TrillBar* bar, uint8_t value);
void trill_set_auto_scan(TrillBar* bar, uint8_t value);
void trill_update_baseline(TrillBar* bar);
void trill_set_scan_settings(TrillBar* bar, uint8_t speed, uint8_t resolution);
void trill_touches(TrillBar* bar, Touch touches[TRILL_MAX_TOUCHES], uint8_t* count);
float trill_calculate_touch(TrillBar* bar);
float trill_calculate_size(TrillBar* bar, float pos);
TrillBar trill_init(i2c_inst_t *i2c_i, uint8_t address);

#endif // TRILL_BAR_H
