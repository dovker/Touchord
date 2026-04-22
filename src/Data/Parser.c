#include "Parser.h"
#include "Helper.h"
#include "tiny-json.h"
#include "IO/Midi.h"
#include "IO/Output.h"
#include "Notes/Note.h"
#include "Synth/AmyEngine.h"

int touchord_settings_from_json(char *text, TouchordSettings *s)
{
    enum { MAX_FIELDS = 64 };
    json_t pool[MAX_FIELDS];

    json_t const *root = json_create(text, pool, MAX_FIELDS);
    if (!root || json_getType(root) != JSON_OBJ) return -1;

    /* integers */
    struct { const char *tag; uint8_t *dst; } ints[] = {
        {"octave",         &s->octave},
        {"extension_count",&s->extension_count},
        {"inversion",      &s->inversion},
        {"velocity",       &s->velocity},
        {"mode",           (uint8_t*)&s->mode},
        {"octave_count",   &s->octave_count},
        {"cutoff",         &s->cutoff},
        {"channel",        (uint8_t*)&s->channel},
        {"output_mode",    (uint8_t*)&s->output_mode},
        {"debug_overlay",  (uint8_t*)&s->debug_overlay},
    };
    for (size_t i = 0; i < sizeof ints/sizeof *ints; ++i) {
        json_t const *j = json_getProperty(root, ints[i].tag);
        if (j && json_getType(j) == JSON_INTEGER)
            *ints[i].dst = (int)json_getInteger(j);
    }

    /* key[]: iterate with child/sibling only */
    json_t const *karr = json_getProperty(root, "key");
    if (karr && json_getType(karr) == JSON_ARRAY) {
        json_t const *el = json_getChild(karr);  /* first element */
        int idx = 0;
        while (el && idx < 3) {
            if (json_getType(el) == JSON_OBJ) {
                json_t const *r = json_getProperty(el, "Root");
                json_t const *q = json_getProperty(el, "Quality");
                if (r && json_getType(r) == JSON_TEXT) {
                    // strncpy(s->key[idx].root, json_getValue(r),
                    //         sizeof s->key[idx].root - 1);
                    // s->key[idx].root[sizeof s->key[idx].root - 1] = '\0';
                }
                if (q && json_getType(q) == JSON_INTEGER) {
                    s->key[idx].quality = (ScaleType)json_getInteger(q);
                }
            }
            idx++;
            el = json_getSibling(el);
        }
    }

    return 0;
}


void touchord_settings_to_json(char *buf, size_t n, const TouchordSettings *s)
{
    char keybuf[128]; 
    snprintf(keybuf, sizeof keybuf,
             "[{\"Root\":\"%s\",\"Quality\":\"%d\"},"
             "{\"Root\":\"%s\",\"Quality\":\"%d\"},"
             "{\"Root\":\"%s\",\"Quality\":\"%d\"}]",
             s->key[0].root, s->key[0].quality,
             s->key[1].root, s->key[1].quality,
             s->key[2].root, s->key[2].quality);
    
    snprintf(buf, n,
        "{\"key\":%s,\"octave\":%d,\"extension_count\":%d,"
        "\"inversion\":%d,\"velocity\":%d,\"mode\":%d,\"octave_count\":%d,"
        "\"cutoff\":%d,\"channel\":%u,\"output_mode\":%u,\"debug_overlay\":%u}",
        keybuf, s->octave, s->extension_count,
        s->inversion, s->velocity, s->mode, s->octave_count,
        s->cutoff, s->channel, s->output_mode, s->debug_overlay ? 1 : 0);
}

static void usb_write_json(const char *s)
{
    size_t len  = strlen(s);
    size_t sent = 0;

    while (sent < len) {
        tud_task();

        uint16_t room = tud_cdc_write_available();
        if (!room) {
            sleep_ms(1);
            continue;
        }

        size_t chunk = room < (len - sent) ? room : (len - sent);
        tud_cdc_write(s + sent, (uint16_t)chunk);
        sent += chunk;
        tud_cdc_write_flush();
    }

    tud_cdc_write_char('\n');
    tud_cdc_write_flush();
}

void process_cmd(const char *line)
{
    if (!strcmp(line, "tone"))
    {
        char response[32];

        snprintf(response, sizeof response, "{\"tone\":%d}", tc_synth_test_tone_enabled() ? 1 : 0);
        tc_log(response);
        return;
    }

    if (!strcmp(line, "tone on"))
    {
        tc_output_all_notes_off(tc_app.channel);
        tc_synth_set_test_tone(true);
        tc_log("{\"tone\":1}");
        return;
    }

    if (!strcmp(line, "tone off"))
    {
        tc_output_all_notes_off(tc_app.channel);
        tc_synth_set_test_tone(false);
        tc_log("{\"tone\":0}");
        return;
    }

    if (!strcmp(line, "read"))
    {
        TouchordSettings local;
        local = tc_app;

        char js[2048];
        touchord_settings_to_json(js, sizeof js, &local);
        usb_write_json(js);
        return;
    }

    if (!strncmp(line, "write ", 6))
    {
        TouchordSettings temp = tc_app;
        char json_text[1024];
        size_t json_len = strnlen(line + 6, sizeof(json_text) - 1);

        memcpy(json_text, line + 6, json_len);
        json_text[json_len] = '\0';

        if (!touchord_settings_from_json(json_text, &temp))
        {            
            tc_app = temp;
            switch_midi_trs(tc_app.midi_type);
            reload_custom_scales();
            tc_output_all_notes_off(tc_app.channel);

            tc_log("{\"resp\":\"ok\"}");
        }
        else  tc_log("{\"resp\":\"error\"}");
        return;
    }

    tc_log("{\"resp\":\"unknown\"}");
}
