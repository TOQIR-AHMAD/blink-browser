# Privacy Browser

A privacy-first desktop web browser: Chromium for the web, Qt 6 / QML for a
minimal glass interface, and a RAM-first session that leaves no browsing
profile behind.

```text
        PREMIUM UI  +  CHROMIUM  +  STRONG PRIVACY
      RAM-FIRST STORAGE  +  TRACKER BLOCKING
              NO DEVELOPER TRACKING
```

## What it does

- **Tabs and navigation** — drag to reorder, middle-click to close, reopen a
  closed tab, favicons, loading and audio indicators, crash handling, the
  standard keyboard shortcuts.
- **An address bar that stays local** — what you type is classified into
  "address" or "search" on this machine, completions come from this session's
  own history, and `javascript:` and `data:` URLs are never navigated to.
- **Blocking before the connection** — Adblock-Plus-syntax filtering on
  Chromium's network thread, two lists built into the binary, and any list you
  add yourself.
- **RAM-only browsing** — an off-the-record Chromium profile, an in-memory
  cache, session cookies only, and a random temporary folder for the few things
  Chromium insists on writing, deleted on exit.
- **Permissions that ask** — camera, microphone, location, notifications,
  clipboard, MIDI, USB, Bluetooth, screen sharing and file access all ask, and
  every answer is forgotten when you close the browser.
- **HTTPS first**, with an interstitial for certificate errors that defaults to
  going back.
- **A privacy dashboard** that shows what was blocked and exactly what the
  browser keeps, and where.

## What it does not do

By design, verified by tests rather than asserted:

- no telemetry, analytics, crash-report upload or advertising SDK
- no user account, cloud sync, or backend of any kind
- no per-installation identifier
- no persistent history, cookies, cache, passwords or autofill
- no update check — the browser never announces itself to anyone
- no browsing data sent to the developers; there is no server to send it to

`docs/privacy-policy.md` states what is stored and what other parties can still
see. `docs/threat-model.md` says what this does not protect against.

## Status

All ten phases of [`PLAN.md`](PLAN.md) are implemented. The Qt-free core and
its 17 unit tests, plus the source-level privacy audit, **build and pass**. The
Qt half — the application, the QML interface and the Qt-based tests — has
**not been compiled yet**, because the development machine has no Qt 6 and its
MSVC 2017 toolset is below Qt 6's floor. See "Build status" in
[`docs/architecture.md`](docs/architecture.md).

## Build

Requirements and full instructions:
[`docs/build-windows.md`](docs/build-windows.md) (macOS and Linux:
[build-macos.md](docs/build-macos.md), [build-linux.md](docs/build-linux.md)).

In short — Visual Studio 2022 C++ tools, CMake 3.21+, Ninja, and Qt 6.8+ with
`QT_ROOT_DIR` pointing at it:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

Check what your machine is missing first:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\check-environment.ps1
```

The Qt-free core and its tests build without Qt installed:

```powershell
cmake --preset core-only && cmake --build --preset core-only && ctest --preset core-only
```

## Verifying the privacy claims

```powershell
python tests\privacy\audit_sources.py          # no SDKs, no unlisted endpoints
ctest --preset core-only                       # blocking, cleanup, policy, settings
powershell -File scripts\verify-privacy.ps1 -Executable <built exe>   # what it left on disk
```

Build with `-DPB_NETWORK_AUDIT=ON` and turn the audit on in Settings ›
Advanced to see every host the running browser contacts.

## Layout

```text
src/     application, browser, tabs, privacy, network, permissions, downloads, settings
ui/      QML: window shell, glass components, pages, theme
tests/   unit, integration, privacy and UI tests
docs/    architecture, build, privacy policy, threat model, blocking, release
scripts/ environment check, icon generation, packaging, privacy verification
```

[`docs/architecture.md`](docs/architecture.md) maps every directory and explains
why the core is Qt-free.

## Licence

Not chosen yet — see [`docs/licensing.md`](docs/licensing.md) for the options
and the Qt LGPL constraint that shapes them.
