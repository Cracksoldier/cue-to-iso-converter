# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
cmake --preset release
cmake --build build/release -j$(nproc)
./build/release/iso-converter
```

Debug build (also generates `compile_commands.json` for clangd):

```bash
cmake --preset debug
cmake --build build/debug -j$(nproc)
ln -sf build/debug/compile_commands.json compile_commands.json
```

No test suite exists. Verify a conversion by checking the ISO 9660 signature:

```bash
xxd -s 32769 -l 5 output.iso   # must print "CD001"
isoinfo -d -i output.iso        # requires genisoimage
```

## Architecture

The app is split into a conversion engine and a Qt6 UI layer. All source lives in `src/`.

**Conversion pipeline** (`CueParser` → `Converter`):

- `CueParser::parse()` reads a `.cue` file with regex line-by-line and returns `QList<CueTrack>`. Each `CueTrack` holds the track type, absolute BIN path, and frame offsets for INDEX 00/01. Timestamps (`MM:SS:FF`) are converted to frame counts (`MM*60*75 + SS*75 + FF`); multiplying by sector size gives the byte offset in the BIN.
- `Converter::convert()` (a `QObject` slot) finds the first data track, seeks to its `indexOneFrame * sectorSize` offset in the BIN, and streams sectors to the ISO. Per-sector stripping: MODE1/2352 → skip 16 bytes, write bytes 16–2063; MODE2/2352 → skip 24 bytes, write bytes 24–2071; MODE1/2048 → direct copy. The end-of-track boundary uses the next track's INDEX 00 (pregap) if present, else INDEX 01, else EOF.

**Threading**: `Converter` is moved to a `QThread` via `moveToThread`. The signal chain `QThread::started → Converter::convert → Converter::finished → QThread::quit → deleteLater` means no manual `wait()` is needed and the worker is self-cleaning.

**UI** (`DropArea`, `MainWindow`):

- `DropArea` is a `QLabel` subclass that accepts `.cue` file drops and emits `fileDropped(QString)`.
- `MainWindow` owns the layout, wires `DropArea::fileDropped` and the Browse button to `setCuePath()`, and spawns the worker thread on Convert. `setConvertingState(bool)` disables Browse/DropArea/Convert during a running conversion.

## Raw string literal caveat

The FILE regex in `CueParser.cpp` uses the named delimiter `R"re(...)re"` instead of `R"(...)"` because the pattern contains `)"` (from `"([^"]+)"`), which would prematurely terminate an unnamed raw string literal.
