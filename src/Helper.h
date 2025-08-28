#ifndef TOUCHORD_HELPER_H
#define TOUCHORD_HELPER_H

#include "tusb.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

static inline void tc_log(const char* str)
{
    tud_cdc_n_write_str(0, str);
    tud_cdc_n_write_char(0, '\n');
    tud_cdc_n_write_flush(0);
}

#endif