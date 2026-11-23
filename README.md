<div align="center">

# 3310

**Hypnotic techno firmware for the M-VAVE FM-1, styled after a two-tone phone screen.**

[![build](https://github.com/orion-musica/3310_FM-1/actions/workflows/build.yml/badge.svg)](https://github.com/orion-musica/3310_FM-1/actions/workflows/build.yml)
![license](https://img.shields.io/badge/license-GPL--3.0%20%2B%20SDK%20exception-43523d)
![device](https://img.shields.io/badge/device-M--VAVE%20FM--1-43523d)
![status](https://img.shields.io/badge/status-experimental-c7a800)

[**Website & web installer**](https://orion-musica.github.io/3310_FM-1/) ·
[Install](#installation) ·
[Manual](#using-3310) ·
[Build](#building-from-source) ·
[Design](#design-concepts) ·
[License](#license)

<img src="docs/assets/rhythm.png" width="360" alt="3310 ARRANGE page in RHYTHM mode: eight tracks, each with a phone symbol and sixteen bar cells">

</div>

---

> [!WARNING]
> **3310 is experimental.** The code has been checked in a host simulator that runs the real screen
> code, but it has not yet been tested on many physical devices. Back up your FM-1 before you install it,
> and keep the original firmware within reach. The web installer can always put the stock V15 back.

## Contents

- [What is 3310](#what-is-3310)
- [Features](#features)
- [Screens](#screens)
- [Installation](#installation)
- [Using 3310](#using-3310)
- [Parameter reference](#parameter-reference)
- [What is not (yet) possible](#what-is-not-yet-possible)
- [Building from source](#building-from-source)
- [Project layout](#project-layout)
- [How it works](#how-it-works)
- [Design concepts](#design-concepts)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [Credits](#credits)
- [License](#license)

## What is 3310

3310 is a free, open-source firmware for the **M-VAVE FM-1** pocket FM synthesizer. It turns the FM-1
into an eight-track arranger for hypnotic, looping techno. Everything is drawn in two shades of
green on a pale LCD background, like the phone screens of the early 2000s.

3310 is built on **FM1 Open V14**, M-VAVE's open firmware. The sound engine and the sequencer are
M-VAVE's own, so timing and sound are exactly as on FM1 Open. 3310 adds a new **ARRANGE** page with
four knob modes, a new look, a phone-style boot screen and a phone symbol for every track. Everything
FM1 Open can do keeps working.

3310 is the monochrome sister of **HASTA LA VISTA**, which shares the same ARRANGE module with a
different look. No phone maker's logo or name is used anywhere.

## Features

**ARRANGE page**

- All eight tracks on one screen: a phone symbol, the sound source and up to 16 bar cells per track,
  grouped in four sections of four bars.
- The bar that is playing has an outline. A zoom strip shows the playing section of the selected track.
- Every track symbol flips to inverse on each hit. A square in the header pulses with the overall level.
  Both are two-frame animations that follow the music.

**Four knob modes on K1**

| Mode | K2 | K3 | K4 |
|---|---|---|---|
| **RHYTHM** | HITS | ROTATE | MUTATE |
| **GROOVE** | GATE | SWING | CHANCE |
| **SPACE** | CUTOFF | PUNCH | THROW |
| **USER** | your choice | your choice | your choice |

- **Euclidean HITS** spread any number of hits evenly over the track. Turning left past zero returns to
  your own hand-written pattern (**ORIG**).
- **MUTATE** changes the pattern once per loop. Left **erodes** it (hits drop out, fresh each loop).
  Right lets it **evolve** (hits wander and the changes add up). The centre (**HOLD**) brings back the
  generated pattern.
- **THROW** is a dub-delay throw: it raises the delay send while you turn and returns it to where it was
  1.5 seconds after you stop.
- **USER mode** lets you put any three of the eleven parameters on K2 to K4.

**Look**

- Two-tone LCD palette: background `#c7f0d8`, pixels `#43523d`.
- Boot screen with an antenna, signal and battery bars that fill up, a large **3310** and **MENU** at the
  bottom.
- Eight track symbols: bell, battery, keypad, handset, snake, note, message and signal.

## Screens

Screens rendered from the real firmware code in the host simulator:

| Boot | RHYTHM | GROOVE |
|:---:|:---:|:---:|
| <img src="docs/assets/boot.png" width="240" alt="Boot screen"> | <img src="docs/assets/rhythm.png" width="240" alt="ARRANGE in RHYTHM mode"> | <img src="docs/assets/groove.png" width="240" alt="ARRANGE in GROOVE mode"> |
| **SPACE** | **USER: editing a knob** | **MUTATE: evolve** |
| <img src="docs/assets/space.png" width="240" alt="ARRANGE in SPACE mode"> | <img src="docs/assets/user-edit.png" width="240" alt="USER knob editing list"> | <img src="docs/assets/evolve.png" width="240" alt="Pattern evolving with MUTATE"> |

### Track symbols

| Track | Default role | Symbol | What it suggests |
|:---:|---|---|---|
| 1 | Kick | Bell | rings on every hit |
| 2 | Rumble | Battery | fills up with low end |
| 3 | Hats | Keypad | ticks like key presses |
| 4 | Percussion | Handset | sends out sound waves |
| 5 | Acid | Snake | squiggles like a 303 line |
| 6 | Pad | Note | holds the chords |
| 7 | FX | Message | pings when it rises |
| 8 | Noise | Signal | static at full bars |

The symbol belongs to the track number, not to the sound: change the preset and the symbol stays.

## Installation

### What you need

- An **M-VAVE FM-1**, on stock V15 or any FM1 Open build.
- A **USB data cable** (not a charge-only cable), plugged straight into the computer.
- **Desktop Chrome or Edge** on Windows, macOS or Linux. The installer uses Web MIDI with SysEx.
  Safari, Firefox and phones are not supported.

### Before you start

1. Export the projects, voices and samples you want to keep.
2. Keep a copy of the original `FM-1.fwsc` from [M-VAVE's download page](https://www.m-vave.com/download).
3. Close DAWs and any other app or browser tab that uses MIDI.

### Install with the web installer

1. Open the [**installer page**](https://orion-musica.github.io/3310_FM-1/install/). Its ready check
   should show **READY**.
2. Switch the FM-1 on, connect it, press **START INSTALLER** and then **Connect device**. Allow MIDI and
   SysEx when the browser asks.
3. **Coming from stock V15:** the installer sees `FM-1_015`. Click **Enable force flash and reconnect**.
   This only reconnects; nothing is written yet. Coming from FM1 Open? Skip this step.
4. Tick the backup confirmation, click **Force flash** (or **Install**) and accept the warning. Do not
   unplug or switch off while it writes.
5. Wait for the sample check to finish. The FM-1 restarts into the 3310 boot screen.

Each build also comes as a single offline HTML installer (`flasher.html`) and as a bare firmware
package (`3310.fwsc`) with its SHA-256 checksum. You find them in the
[Actions](https://github.com/orion-musica/3310_FM-1/actions) artifacts and on the website.

### Going back to stock

Open the installer and click **Restore original V15**. It installs the original firmware and restores
all 128 factory voices across the four banks. 3310 projects and samples do not work on stock firmware,
so export them first.

## Using 3310

The FM-1 starts on the ARRANGE page. **SEQ** switches between ARRANGE and FM1 Open's own step editor.
**HOME** opens FM1 Open's overview. All other buttons work as in FM1 Open.

### Knobs on the ARRANGE page

| Control | Function |
|---|---|
| **SELECT** | Choose the track (1–8). |
| **PRESETS** | Choose the sound (program) of the selected track. |
| **ALGORITHM** | Track length: 16, 32, 48 or 64 steps. |
| **K1** | Mode: RHYTHM, GROOVE, SPACE or USER. The name shows inverted in the header. |
| **K2 – K4** | The three parameters of the current mode. |

The info line under the header shows the selected track, its source, preset, length and the playing
step. A pending length change is shown until the track reaches it.

### Building your own knob map (USER mode)

1. Turn **K1** to **USER**.
2. **Hold EDIT** and turn **K2**, **K3** or **K4**. A list of the eleven parameters opens; turning scrolls
   it.
3. **Release EDIT** to keep the new map (**SAVED**), or press **PLAY while holding EDIT** to cancel
   (**CANCELLED**).

- If you pick a parameter that another knob already has, the two knobs swap.
- Changing the map never changes any track's values.
- A short press of EDIT without turning opens the normal sound editor.
- In RHYTHM, GROOVE and SPACE the map is fixed: holding EDIT shows **USER ONLY**.

## Parameter reference

| Parameter | Mode | Range | What it does |
|---|---|---|---|
| **HITS** | RHYTHM | ORIG, 0 … length | Spreads the hits evenly: step `s` plays when `(s × hits) mod length < hits`. **ORIG** is your own pattern and the left end of the knob. |
| **ROTATE** | RHYTHM | 0 … length − 1 | Shifts the generated pattern around the loop. |
| **MUTATE** | RHYTHM | ERODE 8 … HOLD … EVOLVE 8 | Changes the pattern at every loop wrap. ERODE removes up to 80 % of the hits, fresh each loop. EVOLVE moves hits to empty steps and the moves add up. HOLD restores the generated pattern. |
| **GATE** | GROOVE | 1 … 15 | Note length of every step. Melodic tracks only. |
| **SWING** | GROOVE | engine range | Global swing of the sequencer. |
| **CHANCE** | GROOVE | 12 % … 100 % | Probability that a step plays, in eighths. |
| **CUTOFF** | SPACE | ±99 | Filter cutoff of the track. Not on DX7 tracks. |
| **PUNCH** | SPACE | engine range | Decay of the amp envelope. DX7 tracks only; switches the envelope on if needed. |
| **THROW** | SPACE | 0 … 15 | Raises the delay send while you turn; the old value comes back 1.5 s after you stop. |
| **REVERB** | USER | 0 … 15 | Reverb send of the track. |
| **VOLUME** | USER | engine range | Track volume. |

When a parameter does not apply to the selected track, a short notice explains why, for example
`CUTOFF: NOT ON DX7 TRACKS`.

## What is not (yet) possible

3310 works within FM1 Open's closed sequencer. That sets a few limits:

- **Track length** is 16, 32, 48 or 64 steps. Free lengths from 1 to 128 would need a new sequencer.
- **Ratchets and nudge** are not available, so GROOVE uses GATE and SWING instead.
- **HITS, ROTATE, MUTATE and the USER map live in memory.** They reset when the FM-1 is switched off.
  What they produce (steps, chances and sends) is part of the project and is saved normally with SAVE.

## Building from source

### In the cloud (recommended)

Every push to `main` builds 3310 on GitHub's own Windows machines. The workflow is
[`.github/workflows/build.yml`](.github/workflows/build.yml). It:

1. downloads JieLi's Windows toolchain 2.5.3 and unpacks or installs it;
2. builds the firmware, the web installer and the website with `tools/Make-Site.ps1`;
3. uploads the firmware, the installer and `SHA256SUMS.txt` as the **3310** artifact;
4. publishes `docs/` to GitHub Pages, with an installer that is ready to use.

To run it by hand: **Actions → build → Run workflow**. For the website, set
**Settings → Pages → Source** to **GitHub Actions** once.

### On Windows

Requires Windows x64, PowerShell and **JieLi Windows toolchain 2.5.3** (PI32v2 clang 4.0.1, `r3`
target). The build checks all toolchain files against `sdk/manifest.json`. See
[README.FM1-OPEN.md](README.FM1-OPEN.md) for the download and the installation guide.

The easy way: double-click **`INSTALUJ-3310.bat`**. It finds the toolchain under `C:\JL\pi32`, builds
everything and opens the installer at `http://localhost:8766`. **`OTWORZ-STRONE.bat`** opens the site
again later without building.

By hand, in PowerShell from the repository root:

```powershell
$env:FM1_PI32_TOOLCHAIN = 'C:\JL\pi32'

# Browser simulator only
.\tools\Build-Web.ps1 -LocalPreview

# Firmware only, for local tests on your own FM-1
.\sdk\Build.ps1 -FirmwareOnly -Toolchain $env:FM1_PI32_TOOLCHAIN

# Distribution package + website in docs\
$env:FM1_SOURCE_URL = 'https://github.com/orion-musica/3310_FM-1/archive/refs/heads/main.zip'
.\tools\Make-Site.ps1
```

| Output | Where |
|---|---|
| Firmware package | `build/dx7/generated/FM1_OFFICIAL_OPEN.fwsc` → `docs/downloads/3310.fwsc` |
| Offline web installer | `build/official-web/FM1-Official-Open-Source.html` → `docs/install/flasher.html` |
| Checksum | `docs/downloads/SHA256SUMS.txt` |
| Build info for the installer page | `docs/assets/build-info.js` |

## Project layout

| Path | Contents |
|---|---|
| `app/hlv.inc` | **The ARRANGE module:** modes, parameters, USER editing, mutation, track symbols and drawing. Shared with HASTA LA VISTA. |
| `app/screen.c` | FM1 Open's panel code with small hooks: the LCD palette, the ARRANGE page, the boot screen and button routing. |
| `app/` (rest) | FM1 Open application code: audio, MIDI, projects, samples, presets. |
| `sdk/` | FM1 SDK: build script, engine library, headers, packaging tools, stock V15 recovery image. |
| `web/`, `third_party/` | Browser simulator engine and synth sources. |
| `docs/` | Website, installer page, downloads and screenshots (GitHub Pages). |
| `docs/design/` | Design concept images of every planned screen. |
| `tools/` | Site build (`Make-Site.ps1`), local server (`Serve-Site.ps1`), one-click install (`Install-Local.ps1`). |
| `.github/workflows/` | Cloud build and Pages deployment. |

## How it works

- **Screen.** The ST7789 display (240 × 240, RGB565) is painted in bands 16 rows high. Every area of
  the ARRANGE page keeps a signature of what it shows and is redrawn only when that changes, so the page
  stays light on the CPU while the music plays.
- **Engine access.** The module only uses FM1 Open's public instrument API (`app/instrument.h`): reading
  and replacing sequence steps, per-step gate and probability, track controls, sends and the DX7
  envelope. Nothing in the closed engine is patched.
- **Patterns.** HITS and ROTATE compute a Euclidean pattern from a snapshot of your own steps, so ORIG
  can always bring the original back. MUTATE runs when a track wraps from its last step to its first.
- **Track symbols** are 12 × 12 one-bit bitmaps in `app/hlv.inc`, drawn as runs of pixels in a 14 × 14
  box. On a hit, the box turns dark and the symbol light.
- **Testing.** A host harness compiles the real `screen.c` against a fake engine with AddressSanitizer
  and UndefinedBehaviorSanitizer, simulates knob turns and button presses, and writes every screen as an
  image. The screenshots in this README come from it.

## Design concepts

`docs/design/` holds a concept image for every screen of the full 3310 interface. They show
where the project is heading. **Only the boot screen and the ARRANGE pages (images 01 and 03–08) are
in the firmware today.** The other screens use FM1 Open's own pages for now.

<table>
<tr>
<td align="center"><img src="docs/design/01-boot-splash.png" width="200" alt="Boot concept"><br><sub>Boot</sub></td>
<td align="center"><img src="docs/design/03-arrange-1-rhythm.png" width="200" alt="Arrange concept"><br><sub>Arrange</sub></td>
<td align="center"><img src="docs/design/13-sound-fm6.png" width="200" alt="Sound FM6 concept"><br><sub>Sound · FM6</sub></td>
<td align="center"><img src="docs/design/16-mixer.png" width="200" alt="Mixer concept"><br><sub>Mixer</sub></td>
</tr>
<tr>
<td align="center"><img src="docs/design/10-patterns-clips.png" width="200" alt="Patterns concept"><br><sub>Patterns · clips</sub></td>
<td align="center"><img src="docs/design/11-song.png" width="200" alt="Song concept"><br><sub>Song</sub></td>
<td align="center"><img src="docs/design/17-fx.png" width="200" alt="FX concept"><br><sub>FX</sub></td>
<td align="center"><img src="docs/design/12-arpeggio-arp-hold.png" width="200" alt="Arpeggio concept"><br><sub>Arpeggio</sub></td>
</tr>
</table>

## Troubleshooting

| Problem | What to do |
|---|---|
| The browser does not find the FM-1 | Close other MIDI apps and installer tabs, switch the FM-1 off and on, reconnect USB and click Connect again. The installer's **Connection details** lists the MIDI ports it found. |
| START is greyed out | The ready check on the installer page says why: wrong browser, page opened as a file, or no build yet. |
| The update stopped halfway | Keep the cable in, power-cycle the FM-1, reconnect and run the installer again with force flash. |
| The installer page says **NOT BUILT** | No build has finished yet. Wait for the cloud build, or build locally with `INSTALUJ-3310.bat`. |
| A knob shows `… ONLY` or `NOT ON …` | That parameter does not apply to the selected track's sound source. Pick another track or mode. |
| Patterns reset after power-off | HITS, ROTATE, MUTATE and the USER map are not stored yet; press SAVE to keep the resulting steps. |

## Contributing

Bug reports and ideas are welcome in [Issues](https://github.com/orion-musica/3310_FM-1/issues). For a
bug, please include:

- what you did and what you expected;
- a photo of the screen if you can;
- the build's SHA-256 from the installer page;
- whether you came from stock V15 or FM1 Open.

Pull requests should keep `app/hlv.inc` shared with HASTA LA VISTA. Only colours and the boot screen
differ between the two.

## Credits

- **3310 and HASTA LA VISTA:** BULØKE. The code was written together with Claude.
- **FM1 Open:** M-VAVE (珠海笙科智能科技有限公司), the base firmware, SDK and web installer.
- **Third-party components** (Open303, DaisySP, stmlib, MSFA / Dexed, emu2149, SEA codec, jl-misctools
  and others): see [THIRD_PARTY.md](THIRD_PARTY.md).

## License

3310's application code is licensed under **GPL-3.0-only with the FM1 SDK linking exception**, like
FM1 Open; see [LICENSE.md](LICENSE.md). The engine in `sdk/lib/libfm1_engine.a` is M-VAVE's binary SDK
under its own terms, and the boot and recovery resources are Apache-2.0; see
[THIRD_PARTY.md](THIRD_PARTY.md). If you share a firmware built from this code, share its source too.

3310 is a community project. It is not affiliated with or endorsed by M-VAVE or any phone maker.
Installing custom firmware is at your own risk.
