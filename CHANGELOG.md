# Changelog

All notable changes to Touchord are recorded here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

The single source of truth for the firmware version is `src/version.h`. CMake
parses it so build artifacts and `pico_set_program_version()` stay in lockstep
with what the firmware reports at runtime. Bump it together with this file
when cutting a release.

## [Unreleased]

### Added

### Changed

### Fixed

### Removed

## [1.0.0] — 2026-05-31

### Added

- Compose Jazz mode (tritone sub, secondary dominant, parallel maj↔min, vii°7).
- Compose voice-leading toggle.
- Compose trill tooltip.
- Drum mode and visual 7-pad grid.
- Trill bar position indicator.
- Transitions between modes.
- Boot Intro.
- Octave / range visuals.
- Settings page.
- Dual-I2C and single-I2C firmware.
- Persistent presets.
- Custom scales: four user-configurable scale slots.
- Compose sustain toggle, Perform CC mappings/defaults/reset-on-lift toggles.
- Versioning system: `src/version.h`.
- `CONTRIBUTING.md` developer guide (architecture, concurrency, mode/setting how-tos, pitfalls, PR + release flow).
- SPDX copyright header (`Copyright (C) 2025-2026 MB Daugdara`, `GPL-3.0-or-later`, `info@daugdara.com`) on every non-vendored source file.
- DMA-based screen send.
- Other features.

### Changed

- `compose_key_down` snapshots and emits set-difference (no transient natural chord when pressing a key with the bar held in JAZZ).
- `compose_trill_down` updates trill state and tooltip outside the chord-name guard so the bar's effect is in place when a key press arrives.
- `io_task` polls the trill 5× per frame so latency drops from ~30 ms to a few ms.
- Screen I2C bumped to 1 MHz.
- `tc_app_default` now uses designated initialisers — adding fields can't silently misalign the list.
- Trill 4 % deadzone.
- `io_task` calls `multicore_lockout_victim_init()` so `flash_save_preset` can pause it without hanging.
- `trill_init` now requests 16-bit resolution to match the `TRILL_MAX_SIZE` normalisation.
- Other changes.

### Fixed

- Too many to track for initial release.

### Removed

- Legacy single-frame `main()` splash (replaced by `anim_boot_intro`).
- USB CDC settings protocol.

[Unreleased]: https://github.com/dovker/Touchord/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/dovker/Touchord/releases/tag/v1.0.0
