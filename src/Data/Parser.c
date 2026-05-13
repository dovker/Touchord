#include "Parser.h"
#include "Debug.h"
#include "Helper.h"
#include "tiny-json.h"
#include "Audio/Pcm5102Audio.h"
#include "IO/Midi.h"
#include "IO/Output.h"
#include "Notes/Note.h"
#include "Synth/AmyEngine.h"
#include "tusb.h"

static bool parse_root_name(const char *name, Note *note)
{
    static const struct {
        const char *name;
        Note note;
    } note_map[] = {
        {"C", NOTE_C},
        {"C#", NOTE_C_SHARP},
        {"Db", NOTE_D_FLAT},
        {"D", NOTE_D},
        {"D#", NOTE_D_SHARP},
        {"Eb", NOTE_E_FLAT},
        {"E", NOTE_E},
        {"F", NOTE_F},
        {"F#", NOTE_F_SHARP},
        {"Gb", NOTE_G_FLAT},
        {"G", NOTE_G},
        {"G#", NOTE_G_SHARP},
        {"Ab", NOTE_A_FLAT},
        {"A", NOTE_A},
        {"A#", NOTE_A_SHARP},
        {"Bb", NOTE_B_FLAT},
        {"B", NOTE_B},
    };

    for (size_t i = 0; i < sizeof note_map / sizeof *note_map; ++i) {
        if (!strcmp(name, note_map[i].name)) {
            *note = note_map[i].note;
            return true;
        }
    }

    return false;
}

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

    json_t const *jkey = json_getProperty(root, "key");
    if (jkey) {
        json_t const *key_obj = NULL;

        if (json_getType(jkey) == JSON_OBJ) {
            key_obj = jkey;
        } else if (json_getType(jkey) == JSON_ARRAY) {
            // Backward compatibility with the old three-slot key array.
            key_obj = json_getChild(jkey);
        }

        if (key_obj && json_getType(key_obj) == JSON_OBJ) {
            json_t const *r = json_getProperty(key_obj, "Root");
            json_t const *q = json_getProperty(key_obj, "Quality");

            if (r && json_getType(r) == JSON_TEXT) {
                Note parsed_root;
                if (parse_root_name(json_getValue(r), &parsed_root)) {
                    s->key.root = parsed_root;
                }
            }

            if (q && json_getType(q) == JSON_INTEGER) {
                s->key.quality = (ScaleType)json_getInteger(q);
            }
        }
    }

    return 0;
}


void touchord_settings_to_json(char *buf, size_t n, const TouchordSettings *s)
{
    static const char* root_names[17] = {
        "C", "C#", "Db", "D", "D#", "Eb", "E", "F", "F#", "Gb", "G", "G#", "Ab", "A", "A#", "Bb", "B"
    };
    char keybuf[128]; 
    snprintf(keybuf, sizeof keybuf,
             "{\"Root\":\"%s\",\"Quality\":%d}",
             root_names[s->key.root], s->key.quality);
    
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

    tud_cdc_write("\r\n", 2);
    tud_cdc_write_flush();
}

void process_cmd(const char *line)
{
    if (!strcmp(line, "status"))
    {
        TcAudioStatus audio;
        char response[512];

        tc_audio_get_status(&audio);

        snprintf(response, sizeof response,
            "{\"stage\":\"%s\",\"usb_inited\":%u,\"mounted\":%u,\"cdc\":%u,\"midi\":%u,\"synth_ready\":%u,\"tone\":%u,"
            "\"audio_init\":%u,\"audio_started\":%u,\"audio_busy\":%u,\"audio_blocks\":%lu,\"audio_dma\":%lu,"
            "\"audio_underruns\":%lu,\"audio_frames\":%lu,\"audio_min\":%d,\"audio_max\":%d,"
            "\"left_lrclk\":%u,\"i2s_delay\":%u,\"pair_swap\":%u}",
            tc_debug_get_stage(),
            tud_inited() ? 1u : 0u,
            tud_mounted() ? 1u : 0u,
            tud_cdc_connected() ? 1u : 0u,
            tud_midi_mounted() ? 1u : 0u,
            tc_synth_is_ready() ? 1u : 0u,
            tc_synth_test_tone_enabled() ? 1u : 0u,
            audio.initialized ? 1u : 0u,
            audio.started ? 1u : 0u,
            audio.dma_busy ? 1u : 0u,
            (unsigned long)audio.blocks_written,
            (unsigned long)audio.dma_transfers,
            (unsigned long)audio.underruns,
            (unsigned long)audio.last_frame_count,
            audio.last_min_sample,
            audio.last_max_sample,
            audio.left_lrclk_level,
            audio.i2s_delay,
            audio.pin_pair_swap);
        tc_debug_write_line(response);
        return;
    }

    if (!strcmp(line, "audio lrclk 0"))
    {
        tc_audio_set_left_lrclk_level(0);
        tc_debug_write_line("{\"left_lrclk\":0}");
        return;
    }

    if (!strcmp(line, "audio lrclk 1"))
    {
        tc_audio_set_left_lrclk_level(1);
        tc_debug_write_line("{\"left_lrclk\":1}");
        return;
    }

    if (!strcmp(line, "audio align i2s"))
    {
        tc_audio_set_i2s_delay(true);
        tc_debug_write_line("{\"i2s_delay\":1}");
        return;
    }

    if (!strcmp(line, "audio align lj"))
    {
        tc_audio_set_i2s_delay(false);
        tc_debug_write_line("{\"i2s_delay\":0}");
        return;
    }

    if (!strcmp(line, "audio pair normal"))
    {
        tc_audio_set_pin_pair_swap(false);
        tc_debug_write_line("{\"pair_swap\":0}");
        return;
    }

    if (!strcmp(line, "audio pair swap"))
    {
        tc_audio_set_pin_pair_swap(true);
        tc_debug_write_line("{\"pair_swap\":1}");
        return;
    }

    if (!strcmp(line, "log"))
    {
        tc_debug_dump_log();
        return;
    }

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
