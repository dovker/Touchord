/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#include "Flash.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/multicore.h"
#include <string.h>

// Last 2 flash sectors hold one preset per page (4 presets/sector, 8 total)
#define FLASH_STORAGE_SECTORS 2
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - (FLASH_SECTOR_SIZE * FLASH_STORAGE_SECTORS))

static const uint8_t* flash_preset_base(void)
{
    return (const uint8_t*)(XIP_BASE + FLASH_TARGET_OFFSET);
}

_Static_assert(sizeof(TouchordSettings) <= FLASH_PAGE_SIZE,
               "TouchordSettings exceeds flash page size");

bool flash_preset_exists(uint8_t slot)
{
    if (slot >= FLASH_PRESET_COUNT) return false;

    const TouchordSettings* preset =
        (const TouchordSettings*)(flash_preset_base() + slot * FLASH_PAGE_SIZE);
    return preset->magic == SETTINGS_MAGIC;
}

bool flash_load_preset(uint8_t slot, TouchordSettings* settings)
{
    if (slot >= FLASH_PRESET_COUNT) return false;

    const TouchordSettings* preset =
        (const TouchordSettings*)(flash_preset_base() + slot * FLASH_PAGE_SIZE);

    if (preset->magic != SETTINGS_MAGIC) return false;
    // Skip a preset with wrong layout
    if (preset->schema_version > SETTINGS_SCHEMA_VERSION) return false;

    uint16_t stored_size = preset->struct_size;
    if (stored_size == 0 || stored_size > sizeof(TouchordSettings))
        stored_size = sizeof(TouchordSettings);

    *settings = tc_app_default;
    memcpy(settings, preset, stored_size);

    settings->schema_version = SETTINGS_SCHEMA_VERSION;
    settings->struct_size    = sizeof(TouchordSettings);
    return true;
}

void flash_clear_all_presets(void)
{
    uint32_t ints = save_and_disable_interrupts();
    multicore_lockout_start_blocking();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE * FLASH_STORAGE_SECTORS);
    multicore_lockout_end_blocking();
    restore_interrupts(ints);
}

bool flash_save_preset(uint8_t slot, const TouchordSettings* settings)
{
    if (slot >= FLASH_PRESET_COUNT) return false;

    uint32_t sector = slot / (FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE);
    uint32_t sector_offset = FLASH_TARGET_OFFSET + (sector * FLASH_SECTOR_SIZE);

    // static (BSS) — 4KB on the 2KB per-core stack would hard-fault
    static uint8_t sector_buffer[FLASH_SECTOR_SIZE];
    memcpy(sector_buffer, (const uint8_t*)(XIP_BASE + sector_offset), FLASH_SECTOR_SIZE);

    uint32_t slot_in_sector = slot - (sector * (FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE));
    uint8_t *slot_dst = sector_buffer + (slot_in_sector * FLASH_PAGE_SIZE);
    memcpy(slot_dst, settings, sizeof(TouchordSettings));

    // Stamp current schema/size in case the caller passed a mutated copy
    TouchordSettings *stamped = (TouchordSettings *)slot_dst;
    stamped->magic          = SETTINGS_MAGIC;
    stamped->schema_version = SETTINGS_SCHEMA_VERSION;
    stamped->struct_size    = sizeof(TouchordSettings);

    uint32_t ints = save_and_disable_interrupts();
    multicore_lockout_start_blocking();
    flash_range_erase(sector_offset, FLASH_SECTOR_SIZE);
    flash_range_program(sector_offset, sector_buffer, FLASH_SECTOR_SIZE);
    multicore_lockout_end_blocking();
    restore_interrupts(ints);

    return true;
}
