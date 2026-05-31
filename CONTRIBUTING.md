# Contributing to Touchord

For anyone hacking on the firmware: adding modes, changing settings,
fixing bugs, or shipping a release. Assumes you've read
[README.md](README.md) and have a working build.

---

## Architecture

### Two cores

`main()` runs on **core 0**. After bringing the SDK and TinyUSB up it
launches `io_task` on **core 1** and enters its own loop:

```
core 0 main loop                       core 1 io_task
─────────────────                      ──────────────
tud_task()                             multicore_lockout_victim_init()
poll_buttons()  (if MIDI mounted)      init_i2c, trill_init, ssd1306_init
led_blinking_task()                    anim_boot_intro
                                       while (tc_running) {
                                         poll trill ×5
                                         render screen
                                       }
```

Core 0 owns USB, button polling, and MIDI sent in response to button
events. Core 1 owns the I2C buses (trill + screen), all rendering,
mode-transition animations, and MIDI sent in response to trill events.

Button events on core 0 invoke the active mode's `tc_key_down` /
`tc_button_down` callbacks; trill events on core 1 invoke `tc_trill_down`
/ `tc_trill_up`. Both paths mutate `tc_app` and emit MIDI, so they
share state and need locking.

### Concurrency

One cross-core mutex: `tc_app_lock`, declared in `src/Sync.h` and
initialised by `tc_sync_init()`. Anything reading or writing `tc_app`,
mode-local "playing" state (`compose_last_*`, `omniLastNote*`,
`lastNote`, `prev_seg`, `omniPrevSegment`, `prevSegment`,
`trill_jazz_seg`), or emitting a related batch of MIDI must hold the
lock for the whole sequence:

```c
critical_section_enter_blocking(&tc_app_lock);
/* snapshot prev chord, build new, send diff */
critical_section_exit(&tc_app_lock);
```

Already enforced in Compose, Strum, Omni, Settings triggers, and the
`poll_trill_bar` callees in Touchord.c.

The lock is a Pico SDK `critical_section_t` — a spinlock. ~1 µs per
acquire but it disables core-local interrupts, so don't hold it across
I2C transactions or `sleep_ms`. Keep sections short.

You can skip the lock for:

- Single 32-bit reads/writes (`uint32_t`, `int32_t`, pointers, `bool`)
  — atomic on Cortex-M33.
- A `volatile` flag that signals "do this later" (see
  `tc_trigger_bootsel`). Reach for `volatile` whenever one core sets a
  flag the other core polls.

`uint64_t` reads/writes are not atomic. Guard them with the lock or
split into two `uint32_t`s — see how `tc_overlay_start_ms` /
`tc_overlay_dur_ms` is laid out.

### Modes

Six modes: `Compose`, `Strum`, `Perform`, `Omni`, `Drum`, `Settings`.
Each lives in `src/Modes/<Name>.{c,h}` and exposes `<name>_start()` and
`<name>_end()` plus the handler functions it binds. The handler table
is built by the `TC_MODE_HANDLERS(prefix)` macro in `Globals.h`, which
derives the full table from the prefix — every mode follows the
`<prefix>_<role>` naming convention:

```c
static const ModeHandlers compose_handlers = TC_MODE_HANDLERS(compose);

void compose_start(void)
{
    tc_bind_handlers(&compose_handlers);
    /* mode-specific init */
}
```

The bound handlers live as global function pointers in `Globals.{c,h}`
(`tc_draw`, `tc_key_down`, etc.). Core 1 calls them every render frame;
core 0 calls them on input events.

Mode switches go through `select_mode()` in `Touchord.c`, which calls
the previous mode's `_end()` (typically NOTE_OFF anything playing) then
the new mode's `_start()`. The transition animation lives in
`anim_transition()` in `Graphics.c`.

### Settings & flash

`TouchordSettings` (in `src/Types.h`) is the canonical user-state
struct: keys, octave, extension count, MIDI channel, custom scales,
mode-specific options. Live copy is `tc_app` in `Globals.c`; defaults
are `tc_app_default`.

Storage is in the last two flash sectors (8 preset slots, 4 per
sector, one page per slot). Preset 0 auto-loads at boot. Reads are
XIP-direct (memory-mapped). Writes go through `flash_save_preset`,
which uses the multicore lockout to halt core 1 during the erase +
program (~10 ms).

### Versioning

Two version numbers govern firmware ↔ preset compatibility:

- `SETTINGS_MAGIC` (`Globals.h`) — identifies the blob. Bump for
  file-format-level changes. Old presets are rejected.
- `SETTINGS_SCHEMA_VERSION` (`Globals.h`) — identifies the field
  layout. Bump for non-additive struct changes (rename / reorder /
  resize / remove). Old presets are rejected; `tc_app` stays at
  defaults.

For additive changes (new field at the end of `TouchordSettings`),
don't bump either. `flash_load_preset` overlays stored bytes onto a
default-initialised struct, so the new field gets its default value
when loading older presets. Take this path whenever you can.

The firmware version itself is in `src/version.h`. CMake parses it for
`pico_set_program_version()`, and the boot splash reads it via
`TOUCHORD_VERSION_STRING`. Bump when cutting a release.

---

## Adding a new mode

1. Create `src/Modes/MyMode.h` declaring `mymode_start()`,
   `mymode_end()`, and the handler functions.
2. Create `src/Modes/MyMode.c`. `Drum.c` is the smallest reference.
3. Write the handlers (`mymode_draw`, `mymode_key_down`, etc.). Wrap
   any `tc_app` or mode-local state mutation in
   `critical_section_enter_blocking(&tc_app_lock)` / `_exit`. Set
   `tc_trill_segs` and `tc_trill_show` in `_start()` even if you don't
   use the trill — they control the on-screen indicator.
4. Bind in `_start()`:

   ```c
   static const ModeHandlers mymode_handlers = TC_MODE_HANDLERS(mymode);
   void mymode_start(void) { tc_bind_handlers(&mymode_handlers); ... }
   ```

5. Register the mode:
   - Add to `TouchordMode` enum in `src/Types.h`.
   - Add to both switches in `select_mode()` in `src/Touchord.c`.
   - Add `src/Modes/MyMode.c` to `TOUCHORD_SOURCES` in `CMakeLists.txt`.
6. Wire entry: pick a sibling mode whose `_button_down` should jump
   into yours (e.g. Compose's `B0` jumps to Perform).

## Adding a new setting

Prefer the additive path — it doesn't invalidate saved presets.

1. Add the field at the end of `TouchordSettings` in `src/Types.h`.
2. Add it to the `tc_app_default` initialiser in `src/Globals.c` using
   designated initialisers (`.field_name = value`).
3. Add a `UINode` entry to the menu tree in `src/Modes/Settings.c`. The
   tree is a flat array indexed by absolute position — bump the parent
   submenu's `n_child` and update every `first_child` index that comes
   after the insertion point.
4. Read `tc_app.<field_name>` from any handler that needs it. Same
   locking rule applies if the field drives chord-state mutation.

Don't bump `SETTINGS_MAGIC` or `SETTINGS_SCHEMA_VERSION` for an
additive field. Old presets stay valid.

To remove or rename a field, bump `SETTINGS_SCHEMA_VERSION` so old
presets get rejected instead of decoded at the wrong offsets.

---

## Pitfalls

**Sleeping starves USB.** The original boot put `anim_boot_intro`
(~2 s) and the I2C inits (~150 ms) on core 0 before `tud_init()` —
hosts gave up before MIDI ever appeared. Anything that sleeps must run
on core 1, not in `main()` before the `tud_task()` loop starts.

**Flash erase needs the lockout.** `flash_save_preset` calls
`multicore_lockout_start_blocking()`, which blocks until core 1 has
called `multicore_lockout_victim_init()`. That init is at the top of
`io_task`. Don't call `flash_save_preset` before `io_task` reaches it,
or core 0 hangs.

**Stack vs BSS.** Each core gets 2 KB of stack (`PICO_STACK_SIZE =
0x800`). Big arrays — the slide-animation framebuffers `prev_fb` /
`next_fb`, the flash sector buffer — are `static` so they live in BSS.
Don't put `uint8_t buffer[1024]` on the stack; declare it static or
move it to BSS.

**`tc_app.chord` is shared mutable state.** Most chord-related bugs
have been one core mutating it while the other reads or sends
NOTE_ON/OFF against the previous contents. Snapshot first, then
mutate, then diff/send:

```c
uint8_t prev_chord[MAX_CHORD];
critical_section_enter_blocking(&tc_app_lock);
memcpy(prev_chord, tc_app.chord, MAX_CHORD);
build_chord(/* ... */, tc_app.chord, tc_app.chord_name);
send_midi_chord_diff(prev_chord, tc_app.chord, /* ... */, false);
critical_section_exit(&tc_app_lock);
```

**Inversions reorder slots.** `build_chord` shifts the lowest *N* notes
up an octave then sorts. Code that compares old vs new index-by-index
and emits NOTE_OFF for `prev[i]` + NOTE_ON for `curr[i]` will OFF a
note a previous iteration just ON'd. Use `send_midi_chord_diff` (set
difference) for chord transitions.

**MIDI byte ≥ 0x80 corrupts the stream.** MIDI data bytes are 7-bit.
Any byte ≥ 0x80 is interpreted as a status byte and re-syncs the
parser. Pass note values through `clamp_midi()` (`src/Helper.h`) before
`send_midi_note`, especially after adding octave offsets. Strum and
Omni learned this the hard way.

**Trill bus speed.** The Bela Trill chip caps at I2C Fast Mode
(400 kHz). The screen runs at 1 MHz on its own bus when
`TOUCHORD_DUAL_I2C` is on. Don't push the trill clock past 400 kHz —
the chip will NACK or return garbage.

**`uint8_t = -1` sentinel.** Several places use `uint8_t x = -1` as a
"no value" flag. It evaluates to 255 and never collides with the valid
0..6 key indices. It works but isn't obvious — comment it, or use
`int8_t` / explicit `0xFF` in new code.

---

## Debugging

**printf.** `pico_enable_stdio_usb` is on, so `printf` over USB CDC
works. `printf("trill_pos=%f\n", pos);` in the loop, then re-flash and
attach `screen /dev/cu.usbmodem* 115200` (or any serial monitor).

**Boot hang / locked up.** B0 + B2 + B4 simultaneously forces BOOTSEL
(RPI-RP2 drive appears). Holding B0 at power-on does the same. If
neither works the unit is stuck before `poll_buttons` runs — probe
`main()` and the early init.

**Stuck notes.** Switch modes; each mode's `_end()` sends NOTE_OFF for
what it was tracking. If notes persist across switches, `_end()` is
missing an OFF for new state. If notes hang inside a single mode, the
mutation likely isn't going through `send_midi_chord_diff` — search
for raw `send_midi_note` / `send_midi_chord` calls and verify each
NOTE_ON has a matching OFF path.

**Race symptoms.** "Sometimes" reproducible. Same sequence works
slowly, fails fast. Notes hang only when X happens *during* Y. Check
that both X's and Y's handlers wrap `tc_app` access in `tc_app_lock`.

---

## Build

```bash
cmake -B build
cmake --build build
```

The Pico SDK toolchain is auto-located via
`~/.pico-sdk/cmake/pico-vscode.cmake` if the VS Code Pico extension is
installed; otherwise set `PICO_SDK_PATH` and use
`pico_sdk_import.cmake`. The build uses Ninja by default.

Two firmware variants build from the same source list in one go:

- `build/Touchord-dual.uf2` — Trill on i2c0 (GPIO16/17), screen on
  i2c1 (GPIO14/15). Screen runs at 1 MHz with DMA; trill is polled in
  parallel during the screen send.
- `build/Touchord-single.uf2` — Trill + screen share i2c0 at 400 kHz.
  Trill polls run before the synchronous screen send.

Flash whichever matches your hardware. The variants only differ in the
`TOUCHORD_DUAL_I2C` compile definition. To build one target:

```bash
cmake --build build --target Touchord-dual
cmake --build build --target Touchord-single
```

In the VS Code Pico extension, pick the variant in CMake Tools' target
picker (CMD/CTRL+SHIFT+P → "CMake: Set Launch/Debug Target").

---

## Submitting a PR

1. Branch off `main` with a short descriptive name (`feat/jazz-mode`,
   `fix/strum-hung-notes`).
2. Build clean — both `Touchord-dual.uf2` and `Touchord-single.uf2`
   must compile without warnings.
3. Flash to real hardware and verify behaviour. Type-checking only
   gets you so far; many bugs here are timing- or peripheral-related.
4. Add an entry under `## [Unreleased]` in
   [CHANGELOG.md](CHANGELOG.md) (`Added` / `Changed` / `Fixed` /
   `Removed`).
5. If `TouchordSettings` changed in a non-additive way, bump
   `SETTINGS_SCHEMA_VERSION` in the same PR (see Versioning).
6. Open the PR with a description that says *what changed* and *why*.
   Reference any related issue. Include a short test plan — e.g.
   "Compose Jazz mode: cycle through all four trill segments, verify
   tooltip updates and no notes hang on release."

Squash work-in-progress commits before opening if the history is
noisy. The project doesn't use Conventional Commits; a clear
imperative subject line is enough.

---

## Cutting a release

1. Bump `TOUCHORD_VERSION_{MAJOR,MINOR,PATCH}` in `src/version.h`.
2. In [CHANGELOG.md](CHANGELOG.md), rename `## [Unreleased]` to
   `## [X.Y.Z] — YYYY-MM-DD` and start a fresh empty Unreleased
   section.
3. If `TouchordSettings` changed in a non-additive way, also bump
   `SETTINGS_SCHEMA_VERSION` in `src/Globals.h`.
4. `cmake --build build` — the resulting `build/Touchord-dual.uf2` and
   `build/Touchord-single.uf2` are the release artefacts.
5. Tag: `git tag vX.Y.Z && git push --tags`.

---

## File map

| Path                            | Role                                                              |
| ------------------------------- | ----------------------------------------------------------------- |
| `src/Touchord.c`                | `main()`, `io_task`, polling loops                                |
| `src/Globals.{c,h}`             | Shared state, defaults, handler-pointer slots, `TC_MODE_HANDLERS` |
| `src/Defines.h`                 | Pin numbers, MIDI constants, magic numbers                        |
| `src/Types.h`                   | Enums, `TouchordSettings`, `Scale`, `UINode`                      |
| `src/Helper.h`                  | `tc_log`, `clamp_midi`, `clamp_i16`, `segments`                   |
| `src/Sync.{c,h}`                | `tc_app_lock`, mode getter / setter                               |
| `src/version.h`                 | Firmware version macros                                           |
| `src/Data/Flash.{c,h}`          | Preset persistence (load / save / clear)                          |
| `src/IO/Midi.{c,h}`             | USB-MIDI + TRS UART send; `send_midi_chord_diff`                  |
| `src/IO/Trill.{c,h}`            | Bela Trill bar driver                                             |
| `src/Notes/Note.{c,h}`          | Scale theory, chord building, voice leading, Jazz substitution    |
| `src/Modes/<Name>.{c,h}`        | One per playing mode                                              |
| `src/Rendering/Graphics.{c,h}`  | Drawing primitives, animations, overlays                          |
| `src/Rendering/ScreenDma.{c,h}` | Non-blocking framebuffer send via DMA (dual-bus)                  |
| `src/usb_descriptors.c`         | TinyUSB descriptors (CDC + MIDI)                                  |
| `src/tusb_config.h`             | TinyUSB compile-time config                                       |
| `src/thirdparty/`               | Vendored libraries (ssd1306, fonts) — don't modify                |
