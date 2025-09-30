#ifndef TOUCHORD_FLASH
#define TOUCHORD_FLASH

#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Types.h"
#include "Globals.h"

extern void process_cmd(const char *line);
extern int touchord_settings_from_json(char *json, TouchordSettings *s);
extern void touchord_settings_to_json(char *buf, size_t n, const TouchordSettings *s);

#endif