# Touchord User Manual

## Built-In Synth Edition

This manual covers the Touchord firmware with the built-in AMY synth enabled.

Touchord can now work in three different ways:

- `External`: Touchord behaves like a MIDI controller and sends notes to USB MIDI and TRS MIDI.
- `Internal`: Touchord plays its own built-in synth through an external I2S DAC.
- `Both`: Touchord plays the built-in synth and also sends MIDI out.

The same playing modes and controls drive all three output paths, so once you learn the front panel, the instrument feels consistent whether you are playing standalone or driving other gear.

## 1. What You Need

### Required for normal operation

- A Touchord running the built-in synth firmware
- USB power
- The Trill bar, OLED, and key/control hardware installed as usual

### Required for built-in audio

To hear the internal synth, you must connect an external PCM5102-style I2S DAC.

Firmware pin mapping:

- `GPIO 21` -> I2S `BCLK`
- `GPIO 22` -> I2S `LRCLK`
- `GPIO 23` -> I2S `DATA`

The firmware expects the DAC to provide the analog audio output stage. Depending on your PCM5102 board, you may need powered speakers, a mixer, an amp, or headphones through a suitable output stage.

If no DAC is connected, `Internal` mode will still run the synth engine, but you will not hear anything.

## 2. First Power-On

When Touchord boots:

- the OLED shows the splash screen
- the Trill bar and display initialize
- the AMY synth engine starts in the background
- the default play mode is `Compose`
- the default output mode is `Both`

That means a fresh boot now gives you local synth playback and external MIDI at the same time, assuming the DAC is connected.

If you want standalone synth playback:

1. Power on Touchord.
2. Enter `Settings`.
3. Go to `MIDI -> Output`.
4. Choose `Internal` or `Both`.
5. Return to a play mode and start playing.

## 3. Front Panel Basics

Touchord has:

- `7 chord keys`
- `3 control buttons`
- `1 Trill touch bar`
- `1 OLED display`

### Control button layout

The firmware treats the three control buttons as:

- `Button 1`: mode advance or settings exit, depending on mode
- `Button 2`: usually octave down or back
- `Button 3`: usually octave up or enter/apply

### Mode order

Pressing `Button 1` cycles through the main modes in this order:

`Compose -> Perform -> Strum -> Omni -> Drum -> Settings -> Compose`

Whenever the mode changes, Touchord sends an all-notes-off event so the internal synth and external MIDI devices stay tidy.

## 4. Output Modes

Open `Settings -> MIDI -> Output` to choose how Touchord behaves.

### External

Use this when you want Touchord to behave like the original controller:

- sends note and control data over USB MIDI
- sends note and control data over TRS MIDI
- built-in synth is not heard

### Internal

Use this when you want Touchord to be a standalone instrument:

- notes are played by the internal AMY synth
- no normal external note output is sent
- audio comes from the I2S DAC

### Both

Use this when you want layered behavior:

- the internal synth plays locally
- MIDI is still sent to external gear

### Important limitation

`Drum` mode is currently external-only. Even if `Output` is set to `Internal` or `Both`, Drum mode keeps using external MIDI behavior and does not play the built-in synth.

## 5. Playing Modes

## Compose Mode

Compose mode is for choosing and reshaping chords.

How it works:

- press one of the 7 chord keys to generate a chord in the selected key
- use the Trill bar to reshape the chord
- release the key to stop the chord unless sustain is enabled

Controls:

- `Button 1`: go to `Perform`
- `Button 2`: octave down
- `Button 3`: octave up

Trill behavior:

- in `Degree` mode, the bar changes chord extension and parallel behavior
- in `Inversion` mode, the bar changes inversion

This mode works well with the internal synth because it sends clear note on/off events and updates only the notes that actually changed.

## Perform Mode

Perform mode is for holding a chord and using the touch bar expressively.

How it works:

- press a chord key to play the chord
- slide on the Trill bar to send position and size controls
- release the key to stop the chord

Controls:

- `Button 1`: go to `Strum`
- `Button 2`: octave down
- `Button 3`: octave up

Important current behavior:

- the built-in synth now responds to Perform-mode CC gestures
- touch position drives the internal filter cutoff
- touch size drives filter modulation depth on the internal AMY path

In practice, Perform mode is expressive in `Internal`, `External`, and `Both`, though external synths will still interpret those CCs according to their own patches.

## Strum Mode

Strum mode turns the Trill bar into a note-strumming surface.

How it works:

- press and hold a chord key to choose the chord
- slide on the Trill bar to play notes from that chord across the configured range
- lifting the touch stops the sounding note

Controls:

- `Button 1`: go to `Omni`
- `Button 2`: octave down
- `Button 3`: octave up
- double-click `Button 2`: reduce chord extension count
- double-click `Button 3`: increase chord extension count

This is one of the best modes for the internal synth because the note stream is simple and immediate.

## Omni Mode

Omni mode gives an Omnichord-style texture with a sustained backing interval and moving notes on the bar.

How it works:

- press a chord key to set the harmonic center
- Touchord holds a low root and fifth
- sliding the Trill bar adds moving notes above that base

Controls:

- `Button 1`: go to `Drum`
- `Button 2`: octave down
- `Button 3`: octave up
- double-click `Button 2`: decrease octave span
- double-click `Button 3`: increase octave span

This mode works well with the internal synth because it combines a drone-like base with moving melodic notes.

## Drum Mode

Drum mode maps the keybed to direct notes and uses the Trill bar for velocity.

How it works:

- each key sends one note
- the Trill bar sets the hit velocity
- note number is based on the current octave

Controls:

- `Button 1`: go to `Settings`
- `Button 2`: octave down
- `Button 3`: octave up

Important limitation:

- Drum mode currently does not use the built-in AMY synth
- it always behaves like an external MIDI drum trigger mode

## Settings Mode

Settings mode is where you configure Touchord.

Controls:

- `Button 1`: leave Settings and return to `Compose`
- `Button 2`: go back one menu level
- `Button 3`: enter the selected item, or apply the current value
- chord keys: select which scale degree is being edited in custom-scale menus
- Trill bar: scroll through menu items or adjust values

Main sections:

- `Modes`
- `MIDI`
- `Customize`
- `Load Preset`
- `Save Preset`
- `Firmware`

## 6. Synth-Specific Settings

The built-in synth is controlled mostly through the normal Touchord settings.

### MIDI -> Output

Select:

- `External`
- `Internal`
- `Both`

Changing this setting sends all-notes-off before switching, so it is safe to change during use.

### MIDI -> Velocity

Sets the note velocity used by most non-drum modes. This affects:

- outgoing MIDI note velocity
- internal synth note velocity

### MIDI -> Channel

This still matters for external MIDI.

For the built-in synth, Touchord currently routes all internal note events to a fixed AMY synth slot, so changing the MIDI channel mainly affects external devices.

### MIDI -> TRS Midi

Choose `Type A` or `Type B` for the physical TRS MIDI output jack.

This has no effect on the internal synth audio path.

## 7. Current Built-In Synth Behavior

The current AMY integration is intentionally focused and simple.

What is implemented now:

- one internal synth instance
- one default preset loaded at startup
- polyphonic note playback
- note off handling
- all-notes-off handling
- pitch bend event support in the synth layer
- program change support in the synth layer
- sustain pedal event support in the synth layer

What is not fully surfaced on the front panel yet:

- on-device patch browsing or patch selection
- internal mapping of Perform-mode CC gestures to audible synth controls
- internal drum synth playback

The result is a stable first standalone instrument, not yet a full deep-synthesis front panel.

## 8. Quick Start Recipes

### A. Play Touchord as a standalone synth

1. Connect the PCM5102 DAC.
2. Connect the DAC audio outputs to speakers, mixer, or amp.
3. Power Touchord.
4. Open `Settings -> MIDI -> Output`.
5. Select `Internal`.
6. Return to `Compose`, `Strum`, or `Omni`.
7. Play.

### B. Play Touchord and an external synth together

1. Connect your external synth over USB MIDI or TRS MIDI.
2. Connect the PCM5102 DAC for local audio.
3. Open `Settings -> MIDI -> Output`.
4. Select `Both`.
5. Play normally.

### C. Use Touchord only as a MIDI controller

1. Open `Settings -> MIDI -> Output`.
2. Select `External`.
3. Use Touchord as before.

## 9. Serial Settings Editor

Touchord still supports the USB CDC serial settings interface.

Basic commands:

```text
read
write {"octave":5,"extension_count":4,"inversion":0,"velocity":100,"mode":0,"octave_count":3,"cutoff":0,"channel":0,"output_mode":1}
```

`output_mode` values are:

- `0` = External
- `1` = Internal
- `2` = Both

If you are using the included `settings_editor.html`, make sure the JSON includes `output_mode` if you want the editor to preserve the current routing choice.

## 10. Troubleshooting

### I switched to Internal and hear nothing

Check the following:

- a PCM5102-compatible DAC is connected
- the DAC is wired to `GPIO 21`, `GPIO 22`, and `GPIO 23`
- your speakers or amp are connected to the DAC output, not to the Touchord directly
- `Settings -> MIDI -> Output` is set to `Internal` or `Both`

### External MIDI works, but Drum mode does not make local sound

That is expected in the current firmware. Drum mode is external-only right now.

### Perform mode plays notes, but the touch bar does not change the internal sound much

That is no longer expected after the current AMY update. In `Internal` or `Both`, the touch bar should now open and close the built-in filter and add filter modulation as the size CC rises.

### Notes seem stuck

Try one of these:

- switch to another mode
- change the output mode
- power-cycle the device if needed

Mode changes and output-mode changes both send all-notes-off to clear hanging notes.

## 11. Practical Expectations

Right now, the built-in synth version of Touchord is best thought of as:

- a standalone playable chord instrument
- a layered controller-plus-synth when using `Both`
- a very good base for future sound design features

For the best current standalone experience, start with:

- `Internal` output
- `Compose`, `Strum`, or `Omni`
- moderate velocity
- a properly wired PCM5102 DAC feeding powered speakers or a mixer
