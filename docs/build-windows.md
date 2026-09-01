# Building on Windows

Windows 11 is the primary target (PLAN.md §38).

## Requirements

| Component | Version | Notes |
| --- | --- | --- |
| Visual Studio | 2022, "Desktop development with C++" workload | Qt 6 requires the MSVC 2019 (v142) toolset or newer. The 2017 (v141) toolset is **not** supported by Qt 6. Build Tools for Visual Studio 2022 is enough; the full IDE is not required. |
| Windows SDK | 10.0.20348 or newer | Installed by the C++ workload. |
| CMake | 3.21 or newer | The CMake bundled with Visual Studio 2017 (3.12) is too old. |
| Ninja | 1.10 or newer | Ships with the Visual Studio 2022 C++ workload. |
| Qt | **6.8 LTS** or newer, MSVC 2022 64-bit build | The floor is 6.8: the permission API the browser uses was introduced there. Modules: **Qt Quick**, **Qt Quick Controls** and **Qt WebEngine**. |
| Python | 3.9+ | Build automation scripts only. |
| Git | any recent | |

Qt WebEngine's Chromium version is fixed by the Qt version you install (Qt 6.8
LTS is based on Chromium 122). Check the Qt release notes for the exact version
of any other Qt release; it is not selectable independently.

Run `scripts/check-environment.ps1` to see what this machine currently has. It
only inspects; it installs nothing.

## Installing Qt

Qt 6 is not distributed through a package that can be installed silently without
choices, so install it deliberately:

- **Qt Online Installer** (<https://www.qt.io/download-qt-installer>) — select
  the Qt 6.8.x entry, the *MSVC 2022 64-bit* compiler, and the *Qt WebEngine*
  module. An account is required by Qt's installer.
- **aqtinstall** (`pip install aqtinstall`) — a third-party downloader for the
  same official Qt binaries, no account needed:

  ```powershell
  aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 -m qtwebengine qtpositioning qtwebchannel -O C:\Qt
  ```

  Qt WebEngine needs Qt WebChannel and Qt Positioning, which is why they are
  listed. Expect several gigabytes.

Then point the build at it:

```powershell
$env:QT_ROOT_DIR = "C:\Qt\6.8.3\mingw_64"
```

The CMake presets pass `QT_ROOT_DIR` through as `CMAKE_PREFIX_PATH`. Set it
permanently with `setx QT_ROOT_DIR "C:\Qt\6.8.3\msvc2022_64"` if you prefer.

## Building

Use a *x64 Native Tools Command Prompt for VS 2022* (or run
`"C:\Program Files\Microsoft Visual Studio\2022\<edition>\VC\Auxiliary\Build\vcvars64.bat"`)
so the compiler and Ninja are on `PATH`.

Debug build:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
.\build\windows-debug\bin\PrivacyBrowser.exe
```

Release build:

```powershell
cmake --preset windows-release
cmake --build --preset windows-release
```

The debug preset sets `PB_DEV_LOGGING=ON`, which lets log lines keep the host
part of a URL. Release builds must leave it off (see `docs/architecture.md`).

## Building without Qt WebEngine

Qt ships Qt WebEngine on Windows only for the MSVC toolchain. With the MinGW Qt
- which installs without administrator rights - the browser still builds and
runs, with a placeholder in place of the page:

```powershell
$env:QT_ROOT_DIR = "C:\Qt\6.8.3\mingw_64"
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;$env:QT_ROOT_DIR\bin;$env:PATH"
cmake -S . -B build\mingw -G Ninja -DCMAKE_BUILD_TYPE=Release `
      -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/mingw_64 -DPB_WEB_ENGINE=OFF
cmake --build build\mingw
ctest --test-dir build\mingw --output-on-failure
```

This is a real build of everything except Chromium: the chrome, the tabs, the
address bar, the settings, the privacy dashboard and the C++ models behind them
all work. No page can be loaded, and nothing that depends on the engine -
blocking, the off-the-record profile, permissions, downloads - runs at all.

To deploy it as a folder that runs anywhere:

```powershell
windeployqt --release --qmldir ui --compiler-runtime dist\PrivacyBrowser.exe
```

If you install Qt with `aqtinstall`, the MinGW Qt and its compiler are:

```powershell
pip install aqtinstall
python -m aqt install-qt windows desktop 6.8.3 win64_mingw -O C:\Qt
python -m aqt install-tool windows desktop tools_mingw1310 -O C:\Qt
```

## Building without Qt at all

The Qt-free core libraries and their tests build on their own. This is useful
for checking the toolchain, or on a machine where Qt is not installed:

```powershell
cmake --preset core-only
cmake --build --preset core-only
ctest --preset core-only
```

This does **not** produce a browser — only `PB_BUILD_UI=OFF` targets.

## Packaging

Deferred to Phase 10. `windeployqt` will be used to collect the Qt and Qt
WebEngine runtime next to `PrivacyBrowser.exe`. No background service and no
updater service is installed (PLAN.md §47, §48).
