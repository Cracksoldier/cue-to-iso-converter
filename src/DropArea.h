#pragma once

#include <QLabel>
#include <QString>

namespace iso_converter
{

class DropArea : public QLabel
{
    Q_OBJECT

public:
    explicit DropArea(QWidget* parent = nullptr);

signals:
    void fileDropped(const QString& path);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void setHoverStyle(bool hover);
};

} // namespace iso_converter
