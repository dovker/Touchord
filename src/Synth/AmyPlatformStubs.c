#include "amy.h"

void amy_platform_init(void)
{
}

void amy_platform_deinit(void)
{
}

void amy_update_tasks(void)
{
}

int16_t *amy_render_audio(void)
{
    return amy_simple_fill_buffer();
}

size_t amy_i2s_write(const uint8_t *buffer, size_t nbytes)
{
    (void)buffer;
    return nbytes;
}

void run_midi(void)
{
}

void stop_midi(void)
{
}

void midi_out(uint8_t *bytes, uint16_t len)
{
    (void)bytes;
    (void)len;
}

void amy_send_midi_note_on(uint16_t osc)
{
    (void)osc;
}

void amy_send_midi_note_off(uint16_t osc)
{
    (void)osc;
}
