# 3310

Free, open-source firmware for the **M-VAVE FM-1** in the style of a classic two-tone phone screen:
an eight-track ARRANGE page with four knob modes for hypnotic techno. Monochrome edition of
HASTA LA VISTA, based on M-VAVE's FM1 Open V14. No phone maker's logo or name is used.

**Website, manual and web installer:** the `docs/` folder, published with GitHub Pages.

![ARRANGE page](docs/assets/rhythm.png)

## What it adds

- **ARRANGE** (press SEQ): all eight tracks with section bars, a zoom on the playing section and a
  header LED that pulses with the music.
- **SELECT** track, **PRESETS** sound, **ALGORITHM** length (16/32/48/64), **K1** mode.
- **RHYTHM** – HITS (Euclidean), ROTATE, MUTATE (erode / evolve)
- **GROOVE** – GATE, SWING, CHANCE
- **SPACE** – CUTOFF, PUNCH, THROW (dub delay throw)
- **USER** – your own three knobs: hold EDIT and turn K2–K4, release to keep, EDIT+PLAY to cancel.
- Two-tone LCD theme and a phone-style boot screen with signal and battery bars. Everything from FM1 Open keeps working.

Details: [3310_README.md](3310_README.md). Original FM1 Open notes: [README.FM1-OPEN.md](README.FM1-OPEN.md).

## Source layout

| Path | Contents |
|---|---|
| `app/hlv.inc` | The ARRANGE module (ARRANGE page, modes, parameters, drawing). |
| `app/screen.c` | FM1 Open panel code with small hooks for the module. |
| `docs/` | Website, installer and downloads (GitHub Pages). |
| `tools/Make-Site.ps1` | Builds the package and refreshes `docs/`. |

## Build

Windows x64, PowerShell, JieLi toolchain 2.5.3 (see README.FM1-OPEN.md).

```powershell
$env:FM1_PI32_TOOLCHAIN = 'C:\JL\pi32'
.\tools\Build-Web.ps1 -LocalPreview                  # browser simulator
./sdk/Build.ps1 -FirmwareOnly -Toolchain $env:FM1_PI32_TOOLCHAIN   # firmware only
$env:FM1_SOURCE_URL = 'https://github.com/<user>/<repo>/archive/refs/heads/main.zip'
.\tools\Make-Site.ps1                               # package + website
```

## License

GPL-3.0-only with the FM1 SDK linking exception, like FM1 Open. The engine is M-VAVE's binary
SDK with its own terms; see [THIRD_PARTY.md](THIRD_PARTY.md). Not affiliated with M-VAVE.
Installing custom firmware is at your own risk: back up your FM-1 first.

Made by **BULØKE**.
