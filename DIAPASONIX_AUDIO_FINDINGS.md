# Diapasonix AMY Audio Findings

Research target: `/Users/davidricaud/git/synth/Diapasonix` and
<https://github.com/TuriSc/Diapasonix>.

Touchord context: recent crackling correlated with nonzero `audio_underruns`
while playing chords. The test tone did not clip, and disabling the internal
AMY CC/filter path brought underruns back to zero. That points more toward CPU
deadline misses than analog clipping.

## Executive Summary

Diapasonix gets clean AMY output mostly by buying more headroom, not by using a
magic limiter. It targets Pico 2 / RP2350, overclocks it to about 226 MHz, uses
AMY in fixed-point dual-core mode, feeds I2S through a queued DMA audio buffer
pool, limits normal playing to one AMY voice per string, and leaves meaningful
gain headroom through conservative note velocity and default volume.

The biggest finding for Touchord is this: Diapasonix itself documents that a
plain Raspberry Pi Pico / RP2040 works but crackles noticeably. So any expensive
AMY path that runs cleanly there should be treated as RP2350-specific until we
prove otherwise on Touchord hardware.

## High-Confidence Optimizations

### 1. RP2350 plus overclocked clock budget

Diapasonix is designed around Raspberry Pi Pico 2 / RP2350:

- `README.md` lists Pico 2 / RP2350 as the central unit.
- `CMakeLists.txt` defines `PICO_PLATFORM=rp2350` and `PICO_BOARD=pico2`.
- `config.h` sets PLL constants for a roughly 226 MHz system clock.
- `main.c` calls `set_sys_clock_pll(VCO_FREQ, PLL_PD1, PLL_PD2)` at startup.
- `README.md` says RP2040 support has very poor performance and noticeable crackle.

Touchord implication: this is the most important difference. Touchord is
currently being debugged on RP2040-class hardware, so CPU-heavy AMY features
that Diapasonix can tolerate may still underrun for us.

### 2. AMY rendering is split across both cores

Diapasonix does not call `amy_simple_fill_buffer()` from one core. Its
`multicore_audio.c` manually performs the same rough sequence:

- Core 0 runs `amy_execute_deltas()`.
- Core 0 signals Core 1.
- Core 0 renders oscillator range `0..AMY_OSCS/2`.
- Core 1 renders oscillator range `AMY_OSCS/2..AMY_OSCS`.
- Core 0 calls `amy_fill_buffer()` to mix both core framebuffers.
- Optional global effects are applied after AMY renders the block.

This uses AMY's fixed-point dual-core support. The AMY submodule enables
`AMY_DUALCORE` when built for Arduino RP2040/RP2350-style targets, and Diapasonix
defines the RP2350/RP2040 compatibility macros in CMake.

Touchord implication: our current internal synth intentionally disables AMY
multicore in `src/Synth/AmyEngine.c`. That keeps the architecture simple and
lets the audio task own Core 1, but it also means any AMY render must finish on
one core before the next DMA deadline.

### 3. DMA output is fed through a buffer pool, not a single blocking write

Diapasonix adapts Pico's `pico_audio` style buffer pools:

- `init_audio()` creates a producer pool with 3 AMY-sized buffers.
- `audio_i2s_connect()` creates a consumer pool with 2 output buffers.
- The DMA interrupt gives back the played buffer and immediately starts the next
  transfer.
- If no prepared buffer is available, the I2S layer outputs a short silence
  buffer instead of reading stale memory.

Touchord currently packs one AMY block, waits for the current DMA transfer to
finish, then starts the next transfer. That is simple and deterministic, but it
has very little slack: if rendering or event processing runs late, the status
counter sees an underrun immediately.

Touchord implication: a small queued output pool may be worth borrowing before
re-enabling heavier synth modulation. It will not reduce CPU load, but it can
absorb jitter from USB, display, input, and AMY event bursts.

### 4. The main loop explicitly keeps audio buffers filled before UI work

Diapasonix has a custom `delay_ms()` that repeatedly calls
`rp2040_fill_audio_buffer()` while waiting. The main loop calls `delay_ms(5)`
before `display_task()`, with a comment noting that display I2C can block.

Touchord already moved AMY/audio to its own task/core, which is good. The lesson
to keep is priority: OLED, serial, encoder, and touch work should never block
audio long enough to miss an AMY block deadline.

### 5. Playing is capped to one AMY voice per string

Diapasonix configures four AMY synths, one per string, with `num_voices = 1`.
Each string sends note-on/note-off through its own synth slot. This gives a hard
musical polyphony cap of four active notes, and no string can stack multiple
voices accidentally.

Touchord already has `TC_SYNTH_VOICE_COUNT` set to 4, so we are aligned here.
The important part is to preserve that cap while experimenting with CC or
expressive modulation.

### 6. It uses conservative musical gain before AMY's built-in soft clipping

Diapasonix note-on velocity is fixed at `0.5`. Its UI volume range is `0..8`,
mapped to AMY global volume `0..11`. The default volume is `3`, which maps to
about `4.125`; AMY then applies a `0.1 * amy_global.volume` scale in
`amy_fill_buffer()`.

AMY's own final output path also applies a soft-clipping lookup table before
writing int16 samples. Diapasonix does not bypass that; its custom I2S path calls
`amy_fill_buffer()` and copies those already-clipped int16 samples to the DMA
buffer.

Touchord implication: we should continue using the existing serial `audio_min`
and `audio_max` values to distinguish amplitude clipping from CPU underruns. In
the recent failure case, the samples were small, so gain was not the problem.

## Things That Are Not Directly Portable

### Global filter and distortion are not free

Diapasonix adds `global_filter.c` and `global_distortion.c` after AMY rendering.
The filter processes each channel separately, generates biquad coefficients, and
runs an LPF24-style path by applying the filter twice. That is a nice feature on
RP2350, but it is exactly the kind of work that can push RP2040 over budget.

The distortion path intentionally waveshapes and hard-clips the signal. It is an
effect, not an anti-clipping safety system.

### Flash writes mute audio instead of trying to keep it seamless

Diapasonix disables I2S, pauses AMY, resets Core 1, writes flash, restarts Core
1, and then re-enables audio. The README mentions a short audio gap during
automatic saves. This is a reliability choice, not a sound-quality optimization.

## Practical Recommendations For Touchord

1. Keep the CC pitch path disabled until the baseline shows `audio_underruns:0`
   while playing real chords.
2. Treat any AMY filter, per-voice modulation oscillator, reverb, chorus, or echo
   path as suspect on RP2040 unless status proves no underruns.
3. If we want more expressive CC soon, prefer cheap global pitch bend or
   amplitude/pan changes over AMY filters.
4. Consider adding a small queued DMA buffer pool to Touchord's
   `Pcm5102Audio.c` so the audio task can get one or two blocks ahead.
5. Keep using `status`: if `audio_min/audio_max` approach +/-32767, it is
   clipping; if `audio_underruns` increments while sample values are modest, it
   is CPU/jitter.
6. For bigger AMY features, consider an RP2350 build target before trying to
   force Diapasonix-level effects onto RP2040.

## Files Checked

Diapasonix:

- `/Users/davidricaud/git/synth/Diapasonix/README.md`
- `/Users/davidricaud/git/synth/Diapasonix/CMakeLists.txt`
- `/Users/davidricaud/git/synth/Diapasonix/config.h`
- `/Users/davidricaud/git/synth/Diapasonix/main.c`
- `/Users/davidricaud/git/synth/Diapasonix/multicore_audio.c`
- `/Users/davidricaud/git/synth/Diapasonix/audio/audio_i2s.c`
- `/Users/davidricaud/git/synth/Diapasonix/audio/audio_buffer.c`
- `/Users/davidricaud/git/synth/Diapasonix/global_filter.c`
- `/Users/davidricaud/git/synth/Diapasonix/global_distortion.c`
- `/Users/davidricaud/git/synth/Diapasonix/flash.c`
- `/Users/davidricaud/git/synth/Diapasonix/lib/amy/amy/src/amy.h`
- `/Users/davidricaud/git/synth/Diapasonix/lib/amy/amy/src/amy.c`

Touchord comparison points:

- `/Users/davidricaud/git/synth/Touchord/src/Synth/AmyEngine.c`
- `/Users/davidricaud/git/synth/Touchord/src/Audio/Pcm5102Audio.c`
- `/Users/davidricaud/git/synth/Touchord/src/Data/Parser.c`
- `/Users/davidricaud/git/synth/Touchord/src/Defines.h`
