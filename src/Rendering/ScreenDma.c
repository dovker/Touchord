/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#include "ScreenDma.h"
#include "Defines.h"
#include "ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/regs/i2c.h"
#include "pico/stdlib.h"

#if TOUCHORD_DUAL_I2C

/* I2C IC_DATA_CMD register layout (RP2040/RP2350 datasheet):
     bits  7:0  - data byte
     bit   8    - command (0 = write, 1 = read)
     bit   9    - STOP (issue STOP after this byte)
     bit   10   - RESTART (issue RESTART before this byte)
     other      - test/control bits we don't touch
   The DMA writes 16-bit words to data_cmd; only bits 0..15 matter. */
#define I2C_CMD_STOP_BIT (1u << 9)

static int       dma_chan = -1;
static uint16_t  dma_buf[1 + 128 * 8];   /* 0x40 prefix + max ssd1306 buffer */

void screen_dma_init(void)
{
    if (dma_chan >= 0) return; /* idempotent */
    dma_chan = dma_claim_unused_channel(true);
}

static void send_addr_setup(ssd1306_t *p)
{
    uint8_t payload[] = {
        SET_COL_ADDR, 0, p->width - 1,
        SET_PAGE_ADDR, 0, p->pages - 1,
    };
    if (p->width == 64) { payload[1] += 32; payload[2] += 32; }

    for (size_t i = 0; i < sizeof payload; i++) {
        uint8_t cmd[2] = { 0x00, payload[i] };
        i2c_write_blocking(p->i2c_i, p->address, cmd, 2, false);
    }
}

void screen_show_async(ssd1306_t *p)
{
    send_addr_setup(p);

    size_t total = p->bufsize + 1;
    if (total > sizeof dma_buf / sizeof dma_buf[0]) total = sizeof dma_buf / sizeof dma_buf[0];

    dma_buf[0] = 0x40;
    for (size_t i = 0; i < p->bufsize; i++) dma_buf[i + 1] = p->buffer[i];
    dma_buf[total - 1] |= I2C_CMD_STOP_BIT;

    /* Slave address is already in TAR from the sync writes above. */
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, i2c_get_dreq(p->i2c_i, true));

    dma_channel_configure(
        dma_chan, &c,
        &i2c_get_hw(p->i2c_i)->data_cmd,
        dma_buf,
        total,
        true
    );
}

bool screen_show_busy(void)
{
    return dma_chan >= 0 && dma_channel_is_busy(dma_chan);
}

void screen_show_wait(void)
{
    if (dma_chan < 0) return;
    dma_channel_wait_for_finish_blocking(dma_chan);
    while (i2c_get_hw(DISP_I2C)->status & I2C_IC_STATUS_ACTIVITY_BITS)
        tight_loop_contents();
}

#else /* !TOUCHORD_DUAL_I2C */

// Single bus build

void screen_dma_init(void) {}
void screen_show_async(ssd1306_t *p) { ssd1306_show(p); }
bool screen_show_busy(void)          { return false; }
void screen_show_wait(void)          { }

#endif
