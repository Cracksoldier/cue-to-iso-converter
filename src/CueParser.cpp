#include "CueParser.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>
#include <stdexcept>

namespace iso_converter
{

int CueParser::parseTimestamp(const QString& ts)
{
    const QStringList parts = ts.split(':');
    if (parts.size() != 3)
        throw std::runtime_error("Invalid timestamp: " + ts.toStdString());
    return parts[0].toInt() * 60 * 75 + parts[1].toInt() * 75 + parts[2].toInt();
}

QList<CueTrack> CueParser::parse(const QString& cuePath)
{
    QFile file(cuePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error("Cannot open CUE file: " + cuePath.toStdString());

    const QString cueDir = QFileInfo(cuePath).absolutePath();

    QList<CueTrack> tracks;
    QString currentBin;
    CueTrack currentTrack{};
    bool inTrack = false;

    static const QRegularExpression reFile(
        R"re(^\s*FILE\s+"([^"]+)"\s+BINARY\s*$)re",
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reTrack(
        R"re(^\s*TRACK\s+(\d+)\s+(\S+)\s*$)re",
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reIndex(
        R"re(^\s*INDEX\s+(\d+)\s+(\d{2}:\d{2}:\d{2})\s*$)re",
        QRegularExpression::CaseInsensitiveOption);

    QTextStream in(&file);
    while (!in.atEnd())
    {
        const QString line = in.readLine();

        auto mFile = reFile.match(line);
        if (mFile.hasMatch())
        {
            if (inTrack)
            {
                tracks.append(currentTrack);
                inTrack = false;
            }
            currentBin = QDir(cueDir).absoluteFilePath(mFile.captured(1));
            continue;
        }

        auto mTrack = reTrack.match(line);
        if (mTrack.hasMatch())
        {
            if (inTrack)
                tracks.append(currentTrack);
            currentTrack = CueTrack{};
            currentTrack.number         = mTrack.captured(1).toInt();
            currentTrack.type           = mTrack.captured(2).toUpper();
            currentTrack.binFile        = currentBin;
            currentTrack.indexOneFrame  = -1;
            currentTrack.indexZeroFrame = -1;
            inTrack = true;
            continue;
        }

        auto mIndex = reIndex.match(line);
        if (mIndex.hasMatch() && inTrack)
        {
            const int idx   = mIndex.captured(1).toInt();
            const int frame = parseTimestamp(mIndex.captured(2));
            if (idx == 0)
                currentTrack.indexZeroFrame = frame;
            else if (idx == 1)
                currentTrack.indexOneFrame = frame;
            continue;
        }
    }

    if (inTrack)
        tracks.append(currentTrack);

    return tracks;
}

} // namespace iso_converter
