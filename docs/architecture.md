# Architecture

## Overview

```
┌─────────────────────────────────────┐
│              MainWindow             │
│  ┌──────────┐   ┌────────────────┐  │
│  │ DropArea │   │ QProgressBar   │  │
│  └──────────┘   └────────────────┘  │
└────────────────┬────────────────────┘
                 │ spawns
         ┌───────▼────────┐
         │   QThread      │
         │  ┌──────────┐  │
         │  │Converter │  │  ──signals──▶  MainWindow slots
         │  └──────────┘  │
         └───────┬────────┘
                 │ calls
         ┌───────▼────────┐
         │   CueParser    │
         └───────┬────────┘
                 │
         ┌───────▼────────┐
         │   BIN file     │  ──sectors──▶  ISO file
         └────────────────┘
```

All source files live in `src/`. There is no subdirectory split.

## CUE parsing (`CueParser`)

`CueParser::parse()` processes a CUE sheet line-by-line with three case-insensitive regular expressions:

| Pattern | Captures |
|---|---|
| `FILE "…" BINARY` | BIN filename |
| `TRACK NN TYPE` | track number and type string |
| `INDEX NN MM:SS:FF` | index number and timestamp |

**Note:** The FILE regex uses a named raw-string delimiter `R"re(...)re"` instead of `R"(...)"` because the pattern itself contains `)"`, which would prematurely end an unnamed raw string literal.

Timestamps are converted to **frame counts**: `MM × 60 × 75 + SS × 75 + FF` (75 frames per CD-audio second). Multiplying by the sector size gives the byte offset in the BIN file.

Each `FILE` directive updates the current BIN path. Every subsequent `TRACK` inherits that path, so per-track BIN layouts (one BIN per track) are supported automatically.

## Sector extraction (`Converter`)

### Sector geometry

| CUE type | Raw sector | Data offset | Data size |
|---|---|---|---|
| `MODE1/2352` | 2352 bytes | 16 (sync 12 + header 4) | 2048 bytes |
| `MODE2/2352` | 2352 bytes | 24 (sync 12 + header 4 + subheader 8) | 2048 bytes |
| `MODE1/2048` | 2048 bytes | 0 | 2048 bytes |

An ISO 9660 image is the concatenation of these 2048-byte user-data regions.

### Track boundaries

```
startByte = dataTrack.indexOneFrame × sectorSize

endByte   = nextTrack.indexZeroFrame × nextSectorSize   (if INDEX 00 present)
          | nextTrack.indexOneFrame  × nextSectorSize   (else)
          | binFile.size()                              (if no next track)
```

Using `INDEX 00` (pregap) as the end boundary prevents the inter-track silence that precedes the next audio track from being included in the ISO data.

### Streaming loop

```
while pos < endByte:
    if remaining < sectorSize: break   ← discard incomplete final sector
    read sectorSize bytes from BIN
    write bytes [dataOffset … dataOffset+2048) to ISO
    emit progressChanged(percent)      ← only when percent changes
```

Progress is computed as `sectorsProcessed × 100 / totalSectors` and emitted only when the integer value changes, to avoid flooding the event queue.

## Threading model

`Converter` is a plain `QObject` moved onto a worker `QThread` via `moveToThread`. The self-cleaning signal chain avoids any manual `wait()` call:

```
QThread::started
    → Converter::convert(cuePath, isoPath)    [runs on worker thread]
    → Converter::finished
        → QThread::quit                        [asks thread to exit]
        → MainWindow::onConversionFinished     [updates UI on main thread]

QThread::finished
    → Converter::deleteLater
    → QThread::deleteLater
```

Because `QThread` is parented to `MainWindow`, it is also cleaned up if the window is destroyed before the thread finishes.

## UI wiring (`MainWindow` + `DropArea`)

`DropArea` is a `QLabel` subclass. It overrides `dragEnterEvent` / `dragMoveEvent` / `dragLeaveEvent` / `dropEvent` and emits `fileDropped(QString)` for any dropped URL ending in `.cue`. Style changes (blue dashed border on hover) are applied via `setStyleSheet`.

`MainWindow::setCuePath()` is the single entry point for both the drag-and-drop and browse-button paths. It stores the path, updates the drop-zone label, clears status/progress, and enables the Convert button.

`setConvertingState(bool)` disables Browse, DropArea, and Convert during an active conversion to prevent concurrent jobs.

The ISO output path is pre-filled by replacing the CUE file's extension with `.iso` in the same directory (`suggestIsoPath`).
