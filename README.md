# MusicSurfer (Starter Layout)

This repository contains a minimal CMake-based C++ project scaffold for a future MusicSurfer app.

## Requirements

- CMake 3.20+
- A C++17-compatible compiler (MSVC, GCC, or Clang)

Optional dependencies (enable with CMake options):

- **JUCE** (`MUSIC_SURFER_ENABLE_JUCE=ON`) – requires a CMake-configured JUCE install.
- **TagLib** (`MUSIC_SURFER_ENABLE_TAGLIB=ON`) – requires TagLib with CMake package config support.
- **SQLite** (`MUSIC_SURFER_ENABLE_SQLITE=ON`) – requires SQLite3 development package.

## Build (Linux)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Build (Windows, Developer PowerShell)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Notes

- Main app target: `music_surfer_app`
- Test target: `music_surfer_tests` (when `MUSIC_SURFER_BUILD_TESTS=ON`)
- Core module library: `music_surfer_core`
