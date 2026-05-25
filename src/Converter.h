#pragma once

#include <QObject>
#include <QString>

namespace iso_converter
{

class Converter : public QObject
{
    Q_OBJECT

public:
    explicit Converter(QObject* parent = nullptr);

public slots:
    void convert(const QString& cuePath, const QString& isoPath);

signals:
    void progressChanged(int percent);
    void finished(bool success, const QString& message);
};

} // namespace iso_converter
