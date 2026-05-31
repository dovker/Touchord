/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#include "Sync.h"
#include "Globals.h"

critical_section_t tc_app_lock;

void tc_sync_init(void)
{
    critical_section_init(&tc_app_lock);
}

TouchordMode tc_app_get_mode(void)
{
    critical_section_enter_blocking(&tc_app_lock);
    TouchordMode mode = tc_app.mode;
    critical_section_exit(&tc_app_lock);
    return mode;
}

void tc_app_set_mode(TouchordMode mode)
{
    critical_section_enter_blocking(&tc_app_lock);
    tc_app.mode = mode;
    critical_section_exit(&tc_app_lock);
}
