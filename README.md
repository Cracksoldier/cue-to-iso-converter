# iso-converter

Convert CUE/BIN CD images to ISO 9660. Native Qt6 desktop app with drag-and-drop and a live progress bar.

## Features

- Drag a `.cue` file onto the window (or browse) to load it
- Sector-accurate conversion — strips sync headers from MODE1/2352 and MODE2/2352 tracks, copies MODE1/2048 directly
- Background worker thread; UI stays responsive during conversion
- Handles per-track BINs, pregap (INDEX 00) boundaries, and multi-track CUEs

## Requirements

- CMake 3.20+
- Qt 6.2+ (Core, Widgets)
- C++17 compiler

## Build

```bash
cmake --preset release
cmake --build build/release -j$(nproc)
./build/release/iso-converter
```

## Verify output

```bash
# ISO 9660 signature must be "CD001" at byte 32769
xxd -s 32769 -l 5 output.iso

# Full volume info (requires genisoimage)
isoinfo -d -i output.iso
```

## Supported track types

| Type | Description | Supported |
|---|---|---|
| `MODE1/2352` | Standard CD-ROM data, raw sectors | ✓ |
| `MODE2/2352` | Mixed-mode / CD-XA data, raw sectors | ✓ |
| `MODE1/2048` | Cooked ISO data, no header stripping needed | ✓ |
| `AUDIO` | Red Book audio | — |

CUEs with only `AUDIO` tracks are rejected. Only the first data track is extracted.

## Docs

See [`docs/`](docs/) for the landing page and full usage + architecture documentation.
