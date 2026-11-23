# 3310 – firmware dla M-VAVE FM-1 (faza 1)

Wersja HASTA LA VISTA w stylu dwukolorowego ekranu klasycznego telefonu: zielonkawe tło LCD, ciemne piksele,
ekran startowy z paskami zasięgu i baterii. Bez logo i nazwy producenta telefonu. Funkcje są identyczne jak w HASTA LA VISTA.

Fork FM1 Open V14. Silnik dźwięku i sekwencer pozostają oryginalne (zamknięta biblioteka M-VAVE),
więc timing jest dokładny. 3310 dodaje własny ekran ARRANGE, monochromatyczny wygląd i ekran startowy.

## Co się zmieniło w kodzie

| Plik | Zmiana |
|---|---|
| `app/hlv.inc` | **Nowy.** Cały moduł ARRANGE (wspólny z HASTA LA VISTA): ekran ARRANGE, 4 tryby, parametry, edycja USER, animacje. |
| `app/screen.c` | Małe haki: paleta LCD (BACK/TEXT/DIM/PANEL), nowa strona `HLV`, ekran startowy, SEQ otwiera ARRANGE, start na ARRANGE. |

Wszystkie oryginalne funkcje FM1 Open zostają (HOME, edycja kroków, FM, mikser, projekty, MIDI, Bluetooth).

## Obsługa

**SEQ** przełącza: ARRANGE ↔ oryginalny edytor kroków. **HOME** to oryginalny ekran przeglądu.

| Pokrętło | ARRANGE |
|---|---|
| SELECT | ścieżka 1–8 |
| PRESETS | brzmienie (program) wybranej ścieżki |
| ALGORITHM | długość ścieżki 16 / 32 / 48 / 64 kroki |
| K1 | tryb: RHYTHM, GROOVE, SPACE, USER (nazwa w odwróconym polu u góry) |
| K2–K4 | trzy parametry trybu |

| Tryb | K2 | K3 | K4 |
|---|---|---|---|
| RHYTHM | HITS (euklidesowe rozłożenie; w lewo do końca = ORIG) | ROTATE | MUTATE (lewo ERODE, prawo EVOLVE, środek HOLD) |
| GROOVE | GATE (tylko ścieżki melodyczne) | SWING (globalny) | CHANCE (100% … 12%) |
| SPACE | CUTOFF (bez DX7) | PUNCH (decay ADSR, tylko DX7) | THROW (delay send na czas kręcenia) |
| USER | własny wybór z 11 parametrów | | |

**Edycja USER:** w trybie USER trzymaj **EDIT** i kręć K2/K3/K4 → lista parametrów. Puść EDIT = zapis,
**EDIT + PLAY** = anuluj. Ten sam parametr na innym pokrętle = pokrętła się zamieniają.
Krótkie kliknięcie EDIT (bez kręcenia) otwiera zwykły edytor brzmienia.

**Animacje 2-klatkowe:** kwadrat w nagłówku i numery ścieżek odwracają się w rytm dźwięku; paski zasięgu
i baterii na ekranie startowym napełniają się, kursor miga. Grający bar jest obrysowany.

## Różnice względem makiety (ograniczenia silnika)

- Długość 16/32/48/64 zamiast 1–128 (sekwencer jest w zamkniętej bibliotece).
- RATCHET i NUDGE niedostępne → w GROOVE są GATE i SWING.
- Ustawienia HITS/ROTATE/MUTATE i mapa USER żyją w RAM (po wyłączeniu wracają do zera);
  sam wynik (kroki, szanse, wysyłki) zapisuje się normalnie w projekcie przez SAVE.

## Budowanie (Windows)

1. Zrób kopię zapasową FM-1 (M-Flasher) i miej pod ręką oryginalny `FM-1.fwsc` z m-vave.com/download.
2. Zainstaluj JieLi toolchain 2.5.3 (link w README.FM1-OPEN.md) i ustaw `FM1_PI32_TOOLCHAIN` na folder `pi32`.
3. Podgląd w przeglądarce: `.\tools\Build-Web.ps1 -LocalPreview`
4. Firmware: `./sdk/Build.ps1 -FirmwareOnly -Toolchain 'C:\JL\pi32'`
5. Wgraj plik `.fwsc` przez Workbench (open.m-vave.com/fm1-workbench) i sprawdź na urządzeniu.

Jeśli kompilator zgłosi błąd, wklej mi pełny komunikat.

## Licencja

GPL-3.0 z wyjątkiem linkowania SDK (jak FM1 Open). Przy udostępnianiu firmware publikuj też kod źródłowy.
