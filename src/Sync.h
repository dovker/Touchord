/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_SYNC_H
#define TOUCHORD_SYNC_H

#include "pico/critical_section.h"
#include "Types.h"

extern critical_section_t tc_app_lock;

void tc_sync_init(void);

TouchordMode tc_app_get_mode(void);
void tc_app_set_mode(TouchordMode mode);

#endif
