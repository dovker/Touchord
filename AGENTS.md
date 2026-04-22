# Repository Guidelines

## Project Structure & Module Organization
`src/Touchord.c` is the firmware entry point. Shared state lives in `src/Globals.*`, with common constants and types in `src/Defines.h` and `src/Types.h`. Feature code is split by responsibility: `src/Modes/` for play modes, `src/IO/` for MIDI and Trill input, `src/Rendering/` for OLED drawing, `src/Data/` for JSON settings parsing, and `src/Notes/` for music helpers. Vendored libraries live in `src/thirdparty/`; avoid style-only edits there. Hardware references are under `Hardware/`, documentation images under `img/`, and `settings_editor.html` plus `touchord_preset.json` support settings development.

## Build, Test, and Development Commands
Configure with `cmake -S . -B build -G Ninja` after setting Pico SDK environment variables such as `PICO_SDK_PATH` and the ARM toolchain path. Build with `cmake --build build` or `ninja -C build`; both produce the firmware outputs in `build/`. Flash from the command line with `picotool load build/Touchord.uf2 -fx`. If you use VS Code, the checked-in tasks and launch profiles support `Compile Project`, device flashing, and OpenOCD-based debugging.

## Coding Style & Naming Conventions
Follow the existing C style: 4-space indentation, opening braces on the next line, and small `.c/.h` pairs per subsystem. Use lowercase snake_case for functions and files, `ALL_CAPS` for macros and hardware constants, and the established `tc_` prefix for shared runtime state. Keep mode lifecycle names consistent with the current codebase, such as `compose_start()` or `tc_button_down()`.

## Testing Guidelines
There is no automated `tests/` suite yet. Every change should at minimum compile cleanly and be smoke-tested on hardware. Validate the affected mode behavior, OLED rendering, button and Trill input, and USB or TRS MIDI output paths. For settings changes, exercise JSON `read` and `write` flows with `settings_editor.html` or a serial CDC client, then summarize that manual coverage in the pull request.

## Commit & Pull Request Guidelines
Recent commits use short, imperative subjects such as `Added drum mode, settings page` and `Update README.md`. Keep commit titles concise and action-oriented. Pull requests should describe the behavior change, list the touched modules, note the board or setup used for validation, and link the related issue when available. Include photos, logs, or short clips when changing visible UI, enclosure assets, or physical interaction behavior.
