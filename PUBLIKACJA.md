# Jak opublikować 3310 (GitHub + GitHub Pages)

> **Najpierw lokalnie:** kliknij dwa razy `INSTALL-3310.bat`. Zbuduje firmware,
> uruchomi lokalny serwer i otworzy instalator w Chrome/Edge (`http://localhost:8766`).
> `OPEN-SITE.bat` otwiera samą stronę. Publikacja na GitHubie jest potrzebna dopiero,
> gdy chcesz udostępnić firmware innym.

Robisz to raz. Później każda nowa wersja to tylko kroki 4–6.

## 1. Konto i repozytorium

1. Załóż konto na github.com (jeśli nie masz).
2. Kliknij **New repository**, nazwa np. `3310-fm1`, ustaw **Public**.
   Nie dodawaj README ani licencji – są już w folderze.

## 2. Wyślij folder na GitHub

Najprościej przez **GitHub Desktop** (desktop.github.com):
1. File → **Add local repository** → wskaż folder
   `C:\Music Files\M-Vave\Firmware\26 11 21 3310 Firmware 01\fm1_open-main\3310`.
2. Jeśli zapyta, kliknij **create a repository** w tym folderze.
3. Wpisz opis zmian (np. „3310 0.1”), **Commit to main**, potem **Publish repository**
   – wybierz konto i nazwę z kroku 1, odznacz „Keep this code private”.

Plik `.gitignore` już pomija folder `build/` i klucze, więc nie wyślesz śmieci ani haseł.

## 3. Włącz stronę

Na GitHubie w repozytorium: **Settings → Pages → Build and deployment**:
Source **Deploy from a branch**, Branch **main**, folder **/docs**, **Save**.
Po minucie strona będzie pod adresem `https://<twoj-login>.github.io/3310-fm1/`.
Linki do kodu na stronie ustawią się same na podstawie tego adresu.

## 4. Zbuduj firmware i instalator (Windows)

W PowerShell, w folderze projektu:

```powershell
$env:FM1_PI32_TOOLCHAIN = 'C:\JL\pi32'
$env:FM1_SOURCE_URL = 'https://github.com/<twoj-login>/3310-fm1/archive/refs/heads/main.zip'
.\tools\Make-Site.ps1
```

Skrypt zbuduje firmware i skopiuje do `docs\` instalator (`docs\install\index.html`)
oraz plik `docs\downloads\3310.fwsc`. Do tego momentu przycisk instalatora
pokazuje stronę „not built yet”.

## 5. Przetestuj zanim udostępnisz

1. Backup FM-1 przez M-Flasher.
2. Otwórz `docs\install\index.html` w Chrome i wgraj firmware na swój FM-1.
3. Sprawdź ARRANGE, tryby, EDIT w USER. Jeśli coś nie działa – wklej mi komunikat.

## 6. Opublikuj

W GitHub Desktop: commit („Build 0.1”) → **Push origin**. Strona i instalator są publiczne.

## Uwagi

- Licencja GPL wymaga, żeby kod źródłowy był dostępny – publiczne repo to spełnia.
- Nie wrzucaj do repo kopii zapasowych z M-Flashera ani prywatnych plików.
- Folder `_hlv_backup` możesz usunąć przed publikacją (to tylko kopia oryginału).
