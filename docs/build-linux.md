# Building on Linux

Linux is a supported future target, not a current one (PLAN.md §38). The build
system is portable — there is no Windows-only code in `src/` — but no Linux
build has been performed or verified yet.

## Expected requirements

| Component | Version |
| --- | --- |
| GCC or Clang | C++17 capable (GCC 11+, Clang 13+) |
| CMake | 3.21 or newer |
| Ninja | 1.10 or newer |
| Qt | 6.8 or newer (`gcc_64` build), Qt Quick + Qt WebEngine |

Qt WebEngine additionally needs the usual Chromium runtime libraries (NSS,
libxkbcommon, libdrm, libxcb and friends); the Qt documentation lists the exact
set per distribution.

## Expected commands

```sh
export QT_ROOT_DIR="$HOME/Qt/6.8.3/gcc_64"
cmake -B build/linux -G Ninja -DCMAKE_PREFIX_PATH="$QT_ROOT_DIR" -DCMAKE_BUILD_TYPE=Debug
cmake --build build/linux
ctest --test-dir build/linux --output-on-failure
```

The Qt-free core builds without Qt:

```sh
cmake --preset core-only && cmake --build --preset core-only && ctest --preset core-only
```

This document is updated with verified instructions when Linux is actually
brought up.
