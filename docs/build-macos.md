# Building on macOS

macOS is a supported future target, not a current one (PLAN.md §38). The build
system is portable — there is no Windows-only code in `src/` — but no macOS
build has been performed or verified yet.

## Expected requirements

| Component | Version |
| --- | --- |
| Xcode command line tools | current |
| CMake | 3.21 or newer |
| Ninja | 1.10 or newer |
| Qt | 6.8 or newer (`macos` build), Qt Quick + Qt WebEngine |

## Expected commands

```sh
export QT_ROOT_DIR="$HOME/Qt/6.8.3/macos"
cmake -B build/macos -G Ninja -DCMAKE_PREFIX_PATH="$QT_ROOT_DIR" -DCMAKE_BUILD_TYPE=Debug
cmake --build build/macos
ctest --test-dir build/macos --output-on-failure
```

The Qt-free core builds without Qt:

```sh
cmake --preset core-only && cmake --build --preset core-only && ctest --preset core-only
```

This document is updated with verified instructions when macOS is actually
brought up.
