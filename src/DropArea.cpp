#include "DropArea.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

namespace iso_converter
{

DropArea::DropArea(QWidget* parent) : QLabel(parent)
{
    setAcceptDrops(true);
    setAlignment(Qt::AlignCenter);
    setMinimumSize(400, 120);
    setWordWrap(true);
    setText("Drop a .cue file here\nor use the Browse button");
    setHoverStyle(false);
}

void DropArea::setHoverStyle(bool hover)
{
    if (hover)
    {
        setStyleSheet(
            "border: 2px dashed #4a90d9;"
            "border-radius: 8px;"
            "padding: 24px;"
            "color: #4a90d9;"
            "background: #e8f0fb;");
    }
    else
    {
        setStyleSheet(
            "border: 2px dashed #888;"
            "border-radius: 8px;"
            "padding: 24px;"
            "color: #555;");
    }
}

void DropArea::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
    {
        const auto urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().toLocalFile().endsWith(".cue", Qt::CaseInsensitive))
        {
            event->acceptProposedAction();
            setHoverStyle(true);
            return;
        }
    }
    event->ignore();
}

void DropArea::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void DropArea::dragLeaveEvent(QDragLeaveEvent* /*event*/)
{
    setHoverStyle(false);
}

void DropArea::dropEvent(QDropEvent* event)
{
    setHoverStyle(false);
    const auto urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    const QString path = urls.first().toLocalFile();
    if (path.endsWith(".cue", Qt::CaseInsensitive))
    {
        event->acceptProposedAction();
        emit fileDropped(path);
    }
}

} // namespace iso_converter
