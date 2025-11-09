#ifndef TRILL_BAR_H
#define TRILL_BAR_H

//Driver Inspired by https://github.com/Heerkog/MicroPythonTrill

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "hardware/i2c.h"
#include "Helper.h"

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

static void trill_writeto(TrillBar* bar, uint8_t* data, uint8_t len)
{
    i2c_write_blocking(bar->i2c_i, bar->address, data, len, false);
}

static void trill_writeto_mem(TrillBar* bar, uint8_t reg, uint8_t* data, uint8_t len)
{
    uint8_t out[len+1];
    out[0] = reg;
    memcpy(&out[1], data, len);
    trill_writeto(bar, out, len+1);
}

static void trill_readfrom(TrillBar* bar, uint8_t* data, uint8_t len)
{
    i2c_read_blocking(bar->i2c_i, bar->address, data, len, false);
}

static void trill_readfrom_mem8(TrillBar* bar, uint8_t reg, uint8_t* data, uint8_t len)
{
    i2c_write_blocking(bar->i2c_i, bar->address, &reg, 1, true);
    i2c_read_blocking(bar->i2c_i, bar->address, data, len, false);
}

static void trill_readfrom_mem16(TrillBar* bar, uint8_t reg, uint16_t* data, uint8_t len)
{
    uint8_t* ptr = (uint8_t*)data;
    i2c_write_blocking(bar->i2c_i, bar->address, &reg, 1, false);
    i2c_read_blocking(bar->i2c_i, bar->address, ptr, len * 2, false);

    for (size_t i = 0; i < len * 2; i += 2) {
        uint8_t temp = ptr[i];
        ptr[i] = ptr[i + 1];
        ptr[i + 1] = temp;
    }
}


static void trill_identify(TrillBar* bar)
{
    uint8_t data = TRILL_REG_DATA;
    trill_writeto(bar, &data, 1);
    sleep_ms(TRILL_SLEEP);

    data = TRILL_COMMAND_IDENTIFY;
    trill_writeto_mem(bar, TRILL_REG_COMMAND, &data, 1);
    sleep_ms(TRILL_SLEEP + 15);

    uint8_t buf[3];
    trill_readfrom(bar, buf, 3);

    if(buf[1] != 1) tc_log("Not a trill bar!");
}

static void trill_read(TrillBar* bar)
{
    // uint8_t data = TRILL_REG_DATA;
    // trill_writeto(bar, &data, 1);
    // sleep_ms(TRILL_SLEEP);
    if(bar->mode == TRILL_MODE_CENTROID)
    {
        trill_readfrom_mem16(bar, TRILL_REG_DATA, bar->data, 2 * TRILL_MAX_TOUCHES);
    } else if (bar->mode == TRILL_MODE_DIFF)
    {
        trill_readfrom_mem16(bar, TRILL_REG_DATA, bar->raw_data, 26);
    }
}

static void trill_set_mode(TrillBar* bar, uint8_t mode)
{
    bar->mode = mode;
    uint8_t data[2] = {TRILL_COMMAND_MODE, mode};
    trill_writeto_mem(bar, TRILL_REG_COMMAND, data, 2);
    sleep_ms(TRILL_SLEEP);
}

static void trill_set_noise_threshold(TrillBar* bar, uint8_t value)
{
    uint8_t data[2] = {TRILL_COMMAND_NOISE_THRESHOLD, value};
    trill_writeto_mem(bar, TRILL_REG_COMMAND, data, 2);
    sleep_ms(TRILL_SLEEP);
}

static void trill_set_auto_scan(TrillBar* bar, uint8_t value)
{
    uint8_t data[2] = {TRILL_COMMAND_AUTO_SCAN_INTERVAL, value};
    trill_writeto_mem(bar, TRILL_REG_COMMAND, data, 2);
    sleep_ms(TRILL_SLEEP);
}

static void trill_update_baseline(TrillBar* bar)
{
    uint8_t data = TRILL_COMMAND_BASELINE_UPDATE;
    trill_writeto_mem(bar, TRILL_REG_COMMAND, &data, 1);
    sleep_ms(TRILL_SLEEP);
}

static void trill_set_scan_settings(TrillBar* bar, uint8_t speed, uint8_t resolution)
{
    if(speed < 0) speed = 0;
    else if(speed > 3) speed = 3;

    if(resolution < 9) resolution = 9;
    else if(resolution > 16) resolution = 16;

    uint8_t data[3] = {TRILL_COMMAND_SCAN_SETTINGS, speed, resolution};
    trill_writeto_mem(bar, TRILL_REG_COMMAND, data, 2);
    sleep_ms(TRILL_SLEEP);
}

static void trill_touches(TrillBar* bar, Touch touches[TRILL_MAX_TOUCHES], uint8_t* count)
{
    int c = 0;
    for(int i = 0; i < TRILL_MAX_TOUCHES; i++)
    {
        uint16_t rawPos = bar->data[i];
        uint16_t rawSize = bar->data[i + TRILL_MAX_TOUCHES];
        if(rawPos != 0xFFFF && rawSize > TRILL_SIZE_THRESHOLD)
        {
            float pos = (float)rawPos / TRILL_MAX_POS;
            float size = (float)(rawSize - TRILL_SIZE_THRESHOLD) / TRILL_MAX_SIZE;
            Touch t = {pos, size};
            touches[c] = t;
            c++;
        }
    }
    *count = c;
}

static float trill_calculate_touch(TrillBar* bar)
{
    float sum = 0.0f, weighted_sum = 0.0f;
    for (size_t i = 0; i < 26; ++i) {
        float norm_intensity = (float)bar->raw_data[i] / (float)TRILL_MAX_SIZE;
        if(norm_intensity > 0.05)
        {
            weighted_sum += i * norm_intensity;
            sum += norm_intensity;
        }
    }
    return sum > 0.0f ? (weighted_sum / sum)/25.0f : -1.0f;
}

static float trill_calculate_size(TrillBar* bar, float pos)
{
    if(pos >= 0.0f)
    {
        float sum = 0.0f, var_sum = 0.0f;
        for (size_t i = 0; i < 26; ++i) {
            float norm_intensity = (float)bar->raw_data[i] / (float)TRILL_MAX_SIZE;
            if(norm_intensity > 0.05)
            {
                float it_pos = (float)i / 25.0f;

                var_sum += (it_pos - pos) * (it_pos - pos) * norm_intensity;
                sum += norm_intensity;
            }
        }
        return sum > 0.0f ? sqrtf(var_sum / sum)*10.0f : 0.0f;
    }      
    return 0.0f;
}

static int segments(float pos, int segments)
{    
    if(pos < 0)
    {
        return -1;
    }
    else
    {
        return (int)(pos * segments);
    }
}

static TrillBar trill_init(i2c_inst_t *i2c_i, uint8_t address)
{
    TrillBar bar;
    bar.i2c_i = i2c_i;
    bar.address = address;
    bar.mode = TRILL_MODE_DIFF;

    trill_set_mode(&bar, bar.mode);
    trill_set_scan_settings(&bar, 0, 9);
    trill_set_noise_threshold(&bar, 255);
    trill_update_baseline(&bar);

    return bar;
}

#endif // TRILL_BAR_H
