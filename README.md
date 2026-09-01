# Privacy Browser

A privacy-first desktop web browser: Chromium for the web, Qt 6 / QML for a
minimal glass interface, and a RAM-first session that leaves no browsing profile
behind.

**Status: Phase 0 — architecture and build system.** The application starts and
shows an empty window. There is no web engine, no tab, no network request of any
kind yet. `PLAN.md` is the specification; `docs/architecture.md` describes what
exists and where the rest is going.

## What this browser does not do

By design, and verified by tests as each phase lands:

- no telemetry, analytics, crash-report upload, or advertising SDK
- no user account, cloud sync, or backend of any kind
- no per-installation identifier
- no persistent history, cookies, cache, passwords, or autofill
- no browsing data sent to the developers

Claims about what is or is not written to disk are made only after the Phase 8
persistence tests verify them — see `PLAN.md` §13.

## Build

Requirements and full instructions: [`docs/build-windows.md`](docs/build-windows.md)
(macOS and Linux: [build-macos.md](docs/build-macos.md),
[build-linux.md](docs/build-linux.md)).

In short — Visual Studio 2022 C++ tools, CMake 3.21+, Ninja, and Qt 6.5+ with
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

The Qt-free core libraries and their tests build without Qt installed:

```powershell
cmake --preset core-only && cmake --build --preset core-only && ctest --preset core-only
```

## Layout

```text
src/     application entry point and, per phase, the browser subsystems
ui/      QML module: window, components, pages, theme
tests/   unit, integration, privacy and UI tests
docs/    build instructions and architecture
scripts/ development helpers
```

`docs/architecture.md` maps every directory to the phase that fills it.

## Licence

Not chosen yet — see [`docs/licensing.md`](docs/licensing.md) for the options
and the Qt LGPL constraint that shapes them.
