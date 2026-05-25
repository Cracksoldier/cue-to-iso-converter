# Usage Guide

## Installation

**Requirements:** CMake 3.20+, Qt 6.2+ (Core + Widgets), a C++17 compiler.

```bash
cmake --preset release
cmake --build build/release -j$(nproc)
```

The binary is written to `build/release/iso-converter`. No installation step is required — run it in place.

## Converting a disc image

1. Launch `iso-converter`.
2. Drag a `.cue` file onto the drop zone, or click **Browse for .cue file…** to open a file dialog.
   The drop zone label updates to show the selected filename.
3. Click **Convert to ISO**. A save dialog opens pre-filled with the same base name as the CUE file (e.g. `game.cue` → `game.iso`). Choose the output path.
4. The progress bar appears and advances as sectors are written. The status label shows the current percentage.
5. On success a dialog reports the ISO file size in MiB and the total sector count. On failure it describes the error.

The UI is disabled during conversion. Drop a new CUE or click Browse after the current job finishes.

## Supported track types

| CUE type | Meaning | Converted? |
|---|---|---|
| `MODE1/2352` | Standard CD-ROM data track, raw 2352-byte sectors | Yes |
| `MODE2/2352` | Mixed-mode / CD-XA data track, raw 2352-byte sectors | Yes (Form 1 data) |
| `MODE1/2048` | CD-ROM data, already stripped to 2048-byte sectors | Yes (copied directly) |
| `AUDIO` | Red Book audio | No |

Only the **first data track** in the CUE is extracted. A CUE that contains only `AUDIO` tracks cannot be converted and will produce an error.

## Multi-file CUEs

Some disc rippers produce one BIN file per track (each `FILE` line in the CUE names a different binary). This is handled automatically — the converter reads the BIN file referenced by the data track, regardless of how many other BINs are listed.

## Verifying the output

An ISO 9660 image always has the string `CD001` at byte offset 32 769 (sector 16, byte 1). Check it with:

```bash
xxd -s 32769 -l 5 output.iso
# Expected output: ... 43 44 30 30 31  CD001
```

For a full filesystem listing (requires the `genisoimage` package):

```bash
isoinfo -d -i output.iso   # volume descriptor
isoinfo -l -i output.iso   # directory listing
```

## Limitations

- **Audio-only CUEs** are rejected with a clear error message.
- The CUE must reference `BINARY` files. CUEs that reference `MOTOROLA` or other encodings are not supported.
- Only the first data track is written to the ISO. Multi-session discs with more than one data track will have only their first session extracted.
- MODE2 Form 2 sectors (used for MPEG video on Video CDs) are handled by applying the MODE2 header offset — the resulting ISO will contain the sectors but the filesystem may not be fully ISO 9660 compliant for pure CDXA discs.
