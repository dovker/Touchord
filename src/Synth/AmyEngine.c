#include "Synth/AmyEngine.h"

#include "Globals.h"
#include "Audio/Pcm5102Audio.h"
#include "Debug.h"
#include "Defines.h"
#include "amy.h"
#include "pico/util/queue.h"

typedef enum
{
    TC_SYNTH_EVENT_NOTE_ON = 0,
    TC_SYNTH_EVENT_NOTE_OFF,
    TC_SYNTH_EVENT_CC,
    TC_SYNTH_EVENT_PROGRAM,
    TC_SYNTH_EVENT_PITCH_BEND,
    TC_SYNTH_EVENT_PEDAL,
    TC_SYNTH_EVENT_ALL_NOTES_OFF
} TcSynthEventType;

typedef struct
{
    uint8_t type;
    uint16_t a;
    uint16_t b;
} TcSynthEvent;

typedef struct
{
    uint8_t note;
    bool active;
    bool pending_release;
    uint32_t generation;
} TcSynthVoice;

static queue_t tc_synth_event_queue;
static volatile bool tc_synth_ready = false;
static volatile bool tc_synth_queue_overflowed = false;
static volatile bool tc_synth_test_tone = AUDIO_TEST_TONE_DEFAULT;
static int16_t tc_synth_test_tone_buffer[AMY_BLOCK_SIZE * 2];
static TcSynthVoice tc_synth_voices[TC_SYNTH_VOICE_COUNT];
static uint32_t tc_synth_voice_generation = 1;
static bool tc_synth_pedal_down = false;
static float tc_synth_pitch_bend_state = 0.0f;
#if ENABLE_INTERNAL_SYNTH_CC_PITCH
#define TC_SYNTH_CC_PITCH_CENTER 64
#define TC_SYNTH_CC_PITCH_MAX_OCTAVES (2.0f / 12.0f)
#endif

static void tc_synth_fill_test_tone(size_t frame_count)
{
    static uint32_t phase = 0;
    const uint32_t step = (uint32_t)(((uint64_t)AUDIO_TEST_TONE_FREQUENCY << 32u) / AMY_SAMPLE_RATE);

    for (size_t i = 0; i < frame_count; ++i) {
        int16_t sample = (phase & 0x80000000u) ? AUDIO_TEST_TONE_LEVEL : -AUDIO_TEST_TONE_LEVEL;

        tc_synth_test_tone_buffer[i * 2] = sample;
        tc_synth_test_tone_buffer[i * 2 + 1] = sample;
        phase += step;
    }
}

static void tc_synth_noop_midi_input(uint8_t *bytes, uint16_t len, uint8_t is_sysex)
{
    (void)bytes;
    (void)len;
    (void)is_sysex;
}

#if ENABLE_INTERNAL_SYNTH_CC_PITCH
static float tc_synth_pitch_bend_from_cc(uint8_t value)
{
    float centered = ((float)value - (float)TC_SYNTH_CC_PITCH_CENTER) / 63.0f;

    if (centered < -1.0f) {
        centered = -1.0f;
    } else if (centered > 1.0f) {
        centered = 1.0f;
    }

    return centered * TC_SYNTH_CC_PITCH_MAX_OCTAVES;
}
#endif

static void tc_synth_queue_event(TcSynthEventType type, uint16_t a, uint16_t b)
{
    TcSynthEvent event = {
        .type = (uint8_t)type,
        .a = a,
        .b = b
    };

    if (!tc_synth_ready) {
        return;
    }

    if (!queue_try_add(&tc_synth_event_queue, &event)) {
        tc_synth_queue_overflowed = true;
    }
}

static void tc_synth_stop_voice(uint8_t voice_index)
{
    amy_event event = amy_default_event();

    event.osc = voice_index;
    event.velocity = 0.0f;
    amy_add_event(&event);

    tc_synth_voices[voice_index].active = false;
    tc_synth_voices[voice_index].pending_release = false;
}

static void tc_synth_configure_voice(uint8_t voice_index, float pan)
{
    amy_event event = amy_default_event();

    event.osc = voice_index;
    event.wave = TRIANGLE;
    event.pan_coefs[COEF_CONST] = pan;
    event.amp_coefs[COEF_CONST] = 0.22f;
    event.amp_coefs[COEF_VEL] = 1.0f;
    event.amp_coefs[COEF_EG0] = 1.0f;
    event.eg0_times[0] = 5;
    event.eg0_values[0] = 1.0f;
    event.eg0_times[1] = 80;
    event.eg0_values[1] = 0.72f;
    event.eg0_times[2] = 180;
    event.eg0_values[2] = 0.0f;
    amy_add_event(&event);
}

static void tc_synth_configure_touchord_synth(void)
{
    amy_event event = amy_default_event();

    event.reset_osc = RESET_AMY;
    amy_add_event(&event);

    tc_synth_voice_generation = 1;
    tc_synth_pedal_down = false;
    tc_synth_pitch_bend_state = 0.0f;

    for (uint8_t i = 0; i < TC_SYNTH_VOICE_COUNT; ++i) {
        float pan = 0.5f;

        tc_synth_voices[i].note = 0;
        tc_synth_voices[i].active = false;
        tc_synth_voices[i].pending_release = false;
        tc_synth_voices[i].generation = 0;

        if (TC_SYNTH_VOICE_COUNT > 1) {
            pan = 0.2f + (0.6f * (float)i) / (float)(TC_SYNTH_VOICE_COUNT - 1);
        }

        tc_synth_configure_voice(i, pan);
    }
}

static int tc_synth_find_voice_for_note(uint8_t note)
{
    for (uint8_t i = 0; i < TC_SYNTH_VOICE_COUNT; ++i) {
        if (tc_synth_voices[i].active && tc_synth_voices[i].note == note) {
            return i;
        }
    }

    return -1;
}

static uint8_t tc_synth_pick_voice(void)
{
    uint8_t oldest_voice = 0;
    uint32_t oldest_generation = UINT32_MAX;

    for (uint8_t i = 0; i < TC_SYNTH_VOICE_COUNT; ++i) {
        if (!tc_synth_voices[i].active) {
            return i;
        }

        if (tc_synth_voices[i].generation < oldest_generation) {
            oldest_generation = tc_synth_voices[i].generation;
            oldest_voice = i;
        }
    }

    return oldest_voice;
}

static void tc_synth_play_note(uint8_t note, uint8_t velocity)
{
    amy_event event = amy_default_event();
    int voice_index = tc_synth_find_voice_for_note(note);

    if (voice_index < 0) {
        voice_index = (int)tc_synth_pick_voice();
    }

    if (tc_synth_voices[voice_index].active) {
        tc_synth_stop_voice((uint8_t)voice_index);
    }

    event.osc = (uint16_t)voice_index;
    event.midi_note = (float)note;
    event.velocity = (float)velocity / 127.0f;
    event.pitch_bend = tc_synth_pitch_bend_state;
    amy_add_event(&event);

    tc_synth_voices[voice_index].note = note;
    tc_synth_voices[voice_index].active = true;
    tc_synth_voices[voice_index].pending_release = false;
    tc_synth_voices[voice_index].generation = tc_synth_voice_generation++;
}

static void tc_synth_release_note(uint8_t note)
{
    for (uint8_t i = 0; i < TC_SYNTH_VOICE_COUNT; ++i) {
        if (!tc_synth_voices[i].active || tc_synth_voices[i].note != note) {
            continue;
        }

        if (tc_synth_pedal_down) {
            tc_synth_voices[i].pending_release = true;
        } else {
            tc_synth_stop_voice(i);
        }
    }
}

static void tc_synth_release_sustained_voices(void)
{
    for (uint8_t i = 0; i < TC_SYNTH_VOICE_COUNT; ++i) {
        if (!tc_synth_voices[i].pending_release) {
            continue;
        }

        tc_synth_stop_voice(i);
    }
}

static void tc_synth_all_notes_off_immediate(void)
{
    tc_synth_pedal_down = false;

    for (uint8_t i = 0; i < TC_SYNTH_VOICE_COUNT; ++i) {
        tc_synth_stop_voice(i);
    }
}

static void tc_synth_apply_pitch_bend(float bend)
{
    tc_synth_pitch_bend_state = bend;

    for (uint8_t i = 0; i < TC_SYNTH_VOICE_COUNT; ++i) {
        amy_event event = amy_default_event();

        event.osc = i;
        event.pitch_bend = bend;
        amy_add_event(&event);
    }
}

static void tc_synth_start_engine(void)
{
    amy_config_t config = amy_default_config();

    config.audio = AMY_AUDIO_IS_NONE;
    config.midi = AMY_MIDI_IS_NONE;
    config.features.default_synths = 0;
    config.features.startup_bleep = 0;
    config.features.audio_in = 0;
    config.features.reverb = 0;
    config.features.echo = 0;
    config.features.chorus = 0;
    config.features.partials = 0;
    config.features.custom = 0;
    config.platform.multicore = 0;
    config.platform.multithread = 0;
    config.ks_oscs = 0;
    config.max_oscs = TC_SYNTH_VOICE_COUNT + 4;
    config.max_voices = TC_SYNTH_VOICE_COUNT;
    config.max_synths = 1;
    config.max_sequencer_tags = 8;
    config.max_memory_patches = 1;
    config.amy_external_midi_input_hook = tc_synth_noop_midi_input;

    amy_start(config);
    tc_synth_configure_touchord_synth();

    tc_synth_ready = true;
    tc_debug_logf("synth engine ready voices=%u sample_rate=%u block=%u",
        TC_SYNTH_VOICE_COUNT,
        AMY_SAMPLE_RATE,
        AMY_BLOCK_SIZE);
}

static void tc_synth_dispatch_event(const TcSynthEvent *event)
{
    switch (event->type) {
        case TC_SYNTH_EVENT_NOTE_ON:
            tc_synth_play_note((uint8_t)event->a, (uint8_t)event->b);
            break;
        case TC_SYNTH_EVENT_NOTE_OFF:
            tc_synth_release_note((uint8_t)event->a);
            break;
        case TC_SYNTH_EVENT_PROGRAM:
            break;
        case TC_SYNTH_EVENT_PITCH_BEND:
            tc_synth_apply_pitch_bend(((float)((int)event->a - 8192)) / (6.0f * 8192.0f));
            break;
        case TC_SYNTH_EVENT_PEDAL:
            tc_synth_pedal_down = event->a >= 64;
            if (!tc_synth_pedal_down) {
                tc_synth_release_sustained_voices();
            }
            break;
        case TC_SYNTH_EVENT_ALL_NOTES_OFF:
            tc_synth_all_notes_off_immediate();
            break;
        case TC_SYNTH_EVENT_CC:
#if ENABLE_INTERNAL_SYNTH_CC_PITCH
            if ((uint8_t)event->a == tc_app.perform_pos_cc || (uint8_t)event->a == MIDI_CUTOFF) {
                tc_synth_apply_pitch_bend(tc_synth_pitch_bend_from_cc((uint8_t)event->b));
            }
#endif
            break;
        default:
            break;
    }
}

static void tc_synth_drain_events(void)
{
    TcSynthEvent event;

    while (queue_try_remove(&tc_synth_event_queue, &event)) {
        tc_synth_dispatch_event(&event);
    }
}

void tc_synth_init(void)
{
    queue_init(&tc_synth_event_queue, sizeof(TcSynthEvent), TC_SYNTH_EVENT_QUEUE_DEPTH);
    tc_synth_ready = false;
    tc_synth_queue_overflowed = false;
}

void tc_synth_audio_task(void)
{
    uint32_t boot_tone_blocks_remaining = AUDIO_BOOT_TONE_BLOCKS;
    bool boot_tone_logged = false;

    tc_debug_logf("audio task start");
    if (!tc_audio_init()) {
        tc_debug_logf("audio task stopped: audio init failed");
        return;
    }

    tc_debug_logf("audio init complete");
    tc_synth_start_engine();

    while (tc_running) {
        tc_synth_drain_events();
        if (tc_synth_queue_overflowed) {
            TcSynthEvent panic_event = {
                .type = TC_SYNTH_EVENT_ALL_NOTES_OFF,
                .a = 0,
                .b = 0
            };

            tc_synth_dispatch_event(&panic_event);
            tc_synth_queue_overflowed = false;
            tc_debug_logf("synth queue overflow: all notes off");
        }
        if (tc_synth_test_tone || boot_tone_blocks_remaining > 0) {
            tc_synth_fill_test_tone(AMY_BLOCK_SIZE);
            tc_audio_write_blocking(tc_synth_test_tone_buffer, AMY_BLOCK_SIZE);
            if (boot_tone_blocks_remaining > 0) {
                boot_tone_blocks_remaining--;
                if (boot_tone_blocks_remaining == 0 && !boot_tone_logged) {
                    boot_tone_logged = true;
                    tc_debug_logf("boot tone finished");
                }
            }
        } else {
            tc_audio_write_blocking(amy_simple_fill_buffer(), AMY_BLOCK_SIZE);
        }
    }

    tc_debug_logf("audio task stop");
}

bool tc_synth_is_ready(void)
{
    return tc_synth_ready;
}

void tc_synth_set_test_tone(bool enabled)
{
    tc_synth_test_tone = enabled;
}

bool tc_synth_test_tone_enabled(void)
{
    return tc_synth_test_tone;
}

void tc_synth_note_on(uint8_t note, uint8_t velocity)
{
    tc_synth_queue_event(TC_SYNTH_EVENT_NOTE_ON, note, velocity);
}

void tc_synth_note_off(uint8_t note)
{
    tc_synth_queue_event(TC_SYNTH_EVENT_NOTE_OFF, note, 0);
}

void tc_synth_control_change(uint8_t control, uint8_t value)
{
    tc_synth_queue_event(TC_SYNTH_EVENT_CC, control, value);
}

void tc_synth_program_change(uint8_t program)
{
    tc_synth_queue_event(TC_SYNTH_EVENT_PROGRAM, program, 0);
}

void tc_synth_pitch_bend(uint16_t bend)
{
    tc_synth_queue_event(TC_SYNTH_EVENT_PITCH_BEND, bend, 0);
}

void tc_synth_pedal(uint8_t value)
{
    tc_synth_queue_event(TC_SYNTH_EVENT_PEDAL, value, 0);
}

void tc_synth_all_notes_off(void)
{
    tc_synth_queue_event(TC_SYNTH_EVENT_ALL_NOTES_OFF, 0, 0);
}
