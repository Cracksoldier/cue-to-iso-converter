#pragma once

#include <QList>
#include <QString>

namespace iso_converter
{

struct CueTrack
{
    int     number;
    QString type;           // "MODE1/2352", "MODE2/2352", "MODE1/2048", "AUDIO", etc.
    QString binFile;        // absolute path to the referenced BIN file
    int     indexOneFrame;  // INDEX 01 in frames (MM*60*75 + SS*75 + FF); -1 if not set
    int     indexZeroFrame; // INDEX 00 (pregap start); -1 if not set
};

class CueParser
{
public:
    // Returns all tracks from the .cue file.
    // binFile in each track is resolved to an absolute path relative to cuePath's directory.
    // Throws std::runtime_error on parse errors.
    static QList<CueTrack> parse(const QString& cuePath);

private:
    static int parseTimestamp(const QString& ts); // "MM:SS:FF" -> frame count
};

} // namespace iso_converter
