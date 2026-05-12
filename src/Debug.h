#ifndef TOUCHORD_DEBUG_H
#define TOUCHORD_DEBUG_H

#include <stdint.h>

void tc_debug_init(void);
void tc_debug_mark_usb_started(void);
void tc_debug_service_usb(uint32_t duration_ms);
void tc_debug_stage(const char *stage);
const char *tc_debug_get_stage(void);
void tc_debug_logf(const char *fmt, ...);
void tc_debug_dump_log(void);
void tc_debug_write_line(const char *line);

#endif
