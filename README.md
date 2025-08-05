# Touchord

Touchord is an open source chord-generating synthesizer featuring a touch bar interface. It is designed for musicians seeking versatile chordal playing and performance options. The project is currently in the prototyping stage and aims to provide customizable, high-quality hardware and expressive control.

## Features

- **Three Operating Modes:**
  - **Strum Mode:** An omni-chord style play mode mimicking the strumming of an omnichord.
  - **Composer Mode:** Allows adjustment of chord qualities directly via the touch bar for deep chord customization.
  - **Perform Mode:** Uses the touch bar’s pressure sensitivity for expressive live performances.

- **Chord Layout:**
  - Each key corresponds to a chord degree in a musical key.
  - Supports custom musical keys and customizable chords on each key.

- **Hardware:**
  - Uses the RP2350 microcontroller chip.
  - Programmed with the PicoSDK in C.
  - Cherry MX style mechanical switches.
  - 6 control buttons.
  - 7 chord keys sized 3u, equipped with 3u plate-mounted stabilizers for enhanced stability.
  - Touch bar: [Trill Bar by Bella](https://bela.io/products/trill/) ([GitHub](github.com/BelaPlatform/Trill)) providing multi-touch and pressure sensing.
  - Display: Standard 0.96" 128x64 OLED screen (commonly SSD1306 controller).

- **Enclosure:**
  - Combination of 3D printed carbon fiber reinforced PETG base and CNC machined aluminum top.
  - Optionally, the top can also be fabricated via 3D printing.

- **Connectivity:**
  - USB MIDI support.
  - TRS MIDI support implementing the two most popular MIDI modes:
    - **TRS Type A (Standard MIDI TRS)**
    - **TRS Type B (Roland TRS MIDI)**

## Project Stage

- Currently in prototyping phase with hardware and firmware development underway.
- A working prototype will soon be available.
- Next steps include testing, marketing, and launching an Indiegogo crowdfunding campaign.
- After the campaign, the project will enter the production phase.

## Versions

Users will be able to select from three versions:

1. **PCB Only Kit:**  
   For DIY enthusiasts wanting to fully customize switches and 3D printed enclosures.

2. **3D Printed Model:**  
   Features a carbon fiber PETG base and glass fiber PETG top, providing a sturdy printed enclosure.

3. **Premium Metal Model:**  
   Includes a CNC machined aluminum top for a high-end aesthetic and durability.

---

## Visuals

![Render Side View](./img/TC%20Render%202.png)  
![Render Front View](./img/TC%20Render%201.png)  
![PCB Example](./img/PCB.png)  
*(Images show design renders from different angles and PCB layout)*

---

Touchord aims to empower players with a unique blend of tactile key control and expressive touch bar interaction for chordal synthesis. Stay tuned for updates and prototype demonstrations!

---

*Find the Trill Bar by Bella:* [https://bela.io/products/trill/](https://bela.io/products/trill/) 
*Trill Bar GitHub repository:* [github.com/bela-platform/trill](github.com/BelaPlatform/Trill)  
