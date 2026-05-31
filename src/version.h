/*
 * Touchord — MIDI chord controller firmware.
 * Copyright (C) 2025-2026 MB Daugdara
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * For more info, email info@daugdara.com
 */

#ifndef TOUCHORD_VERSION_H
#define TOUCHORD_VERSION_H

/* Single source of truth for the firmware version. CMake also reads
   these via project(... VERSION) for the build artifact. Bump in lockstep
   with CHANGELOG.md when cutting a release. */
#define TOUCHORD_VERSION_MAJOR 1
#define TOUCHORD_VERSION_MINOR 0
#define TOUCHORD_VERSION_PATCH 0

#define _TOUCHORD_STR(x)  #x
#define _TOUCHORD_XSTR(x) _TOUCHORD_STR(x)
#define TOUCHORD_VERSION_STRING                                       \
    "v" _TOUCHORD_XSTR(TOUCHORD_VERSION_MAJOR) "."                    \
        _TOUCHORD_XSTR(TOUCHORD_VERSION_MINOR) "."                    \
        _TOUCHORD_XSTR(TOUCHORD_VERSION_PATCH)

#endif
