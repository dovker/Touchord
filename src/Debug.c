#include "Debug.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "pico/critical_section.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "tusb.h"

#define TC_DEBUG_LOG_BUFFER_SIZE 4096u
#define TC_DEBUG_STAGE_SIZE      32u

static critical_section_t tc_debug_lock;
static bool tc_debug_lock_ready = false;
static bool tc_debug_usb_started = false;
static bool tc_debug_log_truncated = false;
static char tc_debug_stage_name[TC_DEBUG_STAGE_SIZE] = "reset";
static char tc_debug_log_buffer[TC_DEBUG_LOG_BUFFER_SIZE];
static size_t tc_debug_log_len = 0;

static bool tc_debug_usb_can_write(void)
{
    return tc_debug_usb_started &&
           tud_inited() &&
           tud_mounted() &&
           tud_cdc_connected() &&
           get_core_num() == 0;
}

static void tc_debug_write_bytes_best_effort(const char *data, size_t len)
{
    size_t sent = 0;

    if (!tc_debug_usb_can_write()) {
        return;
    }

    while (sent < len) {
        uint16_t room = tud_cdc_write_available();
        if (!room) {
            break;
        }

        size_t chunk = len - sent;
        if (chunk > room) {
            chunk = room;
        }

        tud_cdc_write(data + sent, (uint16_t)chunk);
        sent += chunk;
    }

    tud_cdc_write_flush();
}

static void tc_debug_write_bytes_blocking(const char *data, size_t len)
{
    size_t sent = 0;

    if (!tc_debug_usb_can_write()) {
        return;
    }

    while (sent < len && tc_debug_usb_can_write()) {
        uint16_t room;

        tud_task();
        room = tud_cdc_write_available();
        if (!room) {
            sleep_ms(1);
            continue;
        }

        size_t chunk = len - sent;
        if (chunk > room) {
            chunk = room;
        }

        tud_cdc_write(data + sent, (uint16_t)chunk);
        sent += chunk;
        tud_cdc_write_flush();
    }
}

static void tc_debug_append(const char *line, size_t len)
{
    if (tc_debug_lock_ready) {
        critical_section_enter_blocking(&tc_debug_lock);
    }

    if (tc_debug_log_len < TC_DEBUG_LOG_BUFFER_SIZE) {
        size_t room = TC_DEBUG_LOG_BUFFER_SIZE - tc_debug_log_len;
        size_t copy_len = len < room ? len : room;

        memcpy(tc_debug_log_buffer + tc_debug_log_len, line, copy_len);
        tc_debug_log_len += copy_len;

        if (copy_len < len) {
            tc_debug_log_truncated = true;
        }
    } else {
        tc_debug_log_truncated = true;
    }

    if (tc_debug_lock_ready) {
        critical_section_exit(&tc_debug_lock);
    }
}

void tc_debug_init(void)
{
    if (!tc_debug_lock_ready) {
        critical_section_init(&tc_debug_lock);
        tc_debug_lock_ready = true;
    }

    tc_debug_logf("debug init");
}

void tc_debug_mark_usb_started(void)
{
    tc_debug_usb_started = true;
    tc_debug_logf("usb stack init");
}

void tc_debug_service_usb(uint32_t duration_ms)
{
    absolute_time_t until = make_timeout_time_ms(duration_ms);

    if (!tc_debug_usb_started || !tud_inited() || get_core_num() != 0) {
        return;
    }

    do {
        tud_task();
        sleep_ms(1);
    } while (!time_reached(until));
}

void tc_debug_stage(const char *stage)
{
    if (!stage) {
        stage = "unknown";
    }

    if (tc_debug_lock_ready) {
        critical_section_enter_blocking(&tc_debug_lock);
    }

    snprintf(tc_debug_stage_name, sizeof tc_debug_stage_name, "%s", stage);

    if (tc_debug_lock_ready) {
        critical_section_exit(&tc_debug_lock);
    }

    tc_debug_logf("stage: %s", stage);
}

const char *tc_debug_get_stage(void)
{
    return tc_debug_stage_name;
}

void tc_debug_logf(const char *fmt, ...)
{
    char line[192];
    va_list args;
    int prefix_len;
    int msg_len;
    uint32_t now = to_ms_since_boot(get_absolute_time());

    prefix_len = snprintf(line, sizeof line, "[%lu] ", (unsigned long)now);
    if (prefix_len < 0) {
        return;
    }
    if ((size_t)prefix_len >= sizeof line) {
        prefix_len = (int)sizeof line - 1;
    }

    va_start(args, fmt);
    msg_len = vsnprintf(line + prefix_len, sizeof line - (size_t)prefix_len, fmt, args);
    va_end(args);

    if (msg_len < 0) {
        return;
    }

    size_t len = (size_t)prefix_len + (size_t)msg_len;
    if (len >= sizeof line) {
        len = sizeof line - 1;
    }

    if (len + 2u < sizeof line) {
        line[len++] = '\r';
        line[len++] = '\n';
        line[len] = '\0';
    } else {
        line[sizeof line - 3u] = '\r';
        line[sizeof line - 2u] = '\n';
        line[sizeof line - 1u] = '\0';
        len = sizeof line - 1u;
    }

    tc_debug_append(line, len);
    tc_debug_write_bytes_best_effort(line, len);
}

void tc_debug_dump_log(void)
{
    size_t offset = 0;
    bool truncated = false;

    tc_debug_write_line("{\"log\":\"begin\"}");

    while (true) {
        char chunk[96];
        size_t len;
        size_t copy_len;

        if (tc_debug_lock_ready) {
            critical_section_enter_blocking(&tc_debug_lock);
        }

        len = tc_debug_log_len;
        truncated = tc_debug_log_truncated;
        copy_len = offset < len ? len - offset : 0;
        if (copy_len > sizeof chunk) {
            copy_len = sizeof chunk;
        }
        if (copy_len) {
            memcpy(chunk, tc_debug_log_buffer + offset, copy_len);
        }

        if (tc_debug_lock_ready) {
            critical_section_exit(&tc_debug_lock);
        }

        if (!copy_len) {
            break;
        }

        tc_debug_write_bytes_blocking(chunk, copy_len);
        offset += copy_len;
    }

    if (truncated) {
        tc_debug_write_line("{\"log\":\"truncated\"}");
    }

    tc_debug_write_line("{\"log\":\"end\"}");
}

void tc_debug_write_line(const char *line)
{
    if (!line) {
        return;
    }

    tc_debug_write_bytes_blocking(line, strlen(line));
    tc_debug_write_bytes_blocking("\r\n", 2);
}
