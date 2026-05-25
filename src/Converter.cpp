#include "Converter.h"
#include "CueParser.h"

#include <QFile>
#include <QFileInfo>

#include <stdexcept>

namespace iso_converter
{

static constexpr qint64 RAW_SECTOR_SIZE  = 2352;
static constexpr qint64 ISO_SECTOR_SIZE  = 2048;
static constexpr qint64 MODE1_DATA_OFFSET = 16; // sync(12) + header(4)
static constexpr qint64 MODE2_DATA_OFFSET = 24; // sync(12) + header(4) + subheader(8)

Converter::Converter(QObject* parent) : QObject(parent) {}

void Converter::convert(const QString& cuePath, const QString& isoPath)
{
    QList<CueTrack> tracks;
    try
    {
        tracks = CueParser::parse(cuePath);
    }
    catch (const std::exception& e)
    {
        emit finished(false, QString("CUE parse error: %1").arg(e.what()));
        return;
    }

    if (tracks.isEmpty())
    {
        emit finished(false, "CUE file contains no tracks.");
        return;
    }

    int dataTrackIndex = -1;
    for (int i = 0; i < tracks.size(); ++i)
    {
        const QString& type = tracks[i].type;
        if (type == "MODE1/2352" || type == "MODE2/2352" || type == "MODE1/2048")
        {
            dataTrackIndex = i;
            break;
        }
    }

    if (dataTrackIndex < 0)
    {
        emit finished(false, "No convertible data track found (only AUDIO tracks present).");
        return;
    }

    const CueTrack& dataTrack = tracks[dataTrackIndex];

    if (dataTrack.indexOneFrame < 0)
    {
        emit finished(false, "Data track has no INDEX 01 entry.");
        return;
    }

    const bool   isModeRaw  = (dataTrack.type != "MODE1/2048");
    const qint64 sectorSize = isModeRaw ? RAW_SECTOR_SIZE : ISO_SECTOR_SIZE;

    QFile binFile(dataTrack.binFile);
    if (!binFile.exists())
    {
        emit finished(false, QString("BIN file not found: %1").arg(dataTrack.binFile));
        return;
    }
    if (!binFile.open(QIODevice::ReadOnly))
    {
        emit finished(false, QString("Cannot open BIN file: %1").arg(dataTrack.binFile));
        return;
    }

    const qint64 startByte = dataTrack.indexOneFrame * sectorSize;

    qint64 endByte = binFile.size();
    if (dataTrackIndex + 1 < tracks.size())
    {
        const CueTrack& nextTrack = tracks[dataTrackIndex + 1];
        const int endFrame = (nextTrack.indexZeroFrame >= 0)
                             ? nextTrack.indexZeroFrame
                             : nextTrack.indexOneFrame;
        if (endFrame > dataTrack.indexOneFrame)
        {
            const qint64 nextSectorSize = (nextTrack.type == "MODE1/2048")
                                          ? ISO_SECTOR_SIZE : RAW_SECTOR_SIZE;
            endByte = endFrame * nextSectorSize;
        }
    }

    if (startByte >= binFile.size())
    {
        emit finished(false, "Data track start offset is beyond end of BIN file.");
        return;
    }
    endByte = qMin(endByte, binFile.size());

    if (!binFile.seek(startByte))
    {
        emit finished(false, "Failed to seek in BIN file.");
        return;
    }

    QFile isoFile(isoPath);
    if (!isoFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        emit finished(false, QString("Cannot create ISO file: %1").arg(isoPath));
        return;
    }

    qint64 dataOffset = 0;
    if (dataTrack.type == "MODE1/2352")
        dataOffset = MODE1_DATA_OFFSET;
    else if (dataTrack.type == "MODE2/2352")
        dataOffset = MODE2_DATA_OFFSET;

    const qint64 totalBytes   = endByte - startByte;
    const qint64 totalSectors = totalBytes / sectorSize;
    qint64       sectorsProcessed = 0;
    int          lastPercent = -1;

    QByteArray sectorBuf(static_cast<int>(sectorSize), 0);

    while (binFile.pos() < endByte)
    {
        if (endByte - binFile.pos() < sectorSize)
            break;

        const qint64 bytesRead = binFile.read(sectorBuf.data(), sectorSize);
        if (bytesRead != sectorSize)
            break;

        const qint64 written = isoFile.write(sectorBuf.constData() + dataOffset, ISO_SECTOR_SIZE);
        if (written != ISO_SECTOR_SIZE)
        {
            emit finished(false, "Write error while creating ISO.");
            return;
        }

        ++sectorsProcessed;

        const int percent = (totalSectors > 0)
                            ? static_cast<int>(sectorsProcessed * 100 / totalSectors)
                            : 100;
        if (percent != lastPercent)
        {
            lastPercent = percent;
            emit progressChanged(percent);
        }
    }

    isoFile.flush();
    isoFile.close();
    binFile.close();

    const qint64 isoSizeMiB = QFileInfo(isoPath).size() / (1024 * 1024);
    emit finished(true,
        QString("Conversion complete. ISO size: %1 MiB (%2 sectors)")
            .arg(isoSizeMiB)
            .arg(sectorsProcessed));
}

} // namespace iso_converter
