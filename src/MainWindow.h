#pragma once

#include <QMainWindow>
#include <QString>

class QLabel;
class QProgressBar;
class QPushButton;
class QThread;

namespace iso_converter
{

class DropArea;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onFileDropped(const QString& path);
    void onBrowseClicked();
    void onConvertClicked();
    void onProgressChanged(int percent);
    void onConversionFinished(bool success, const QString& message);

private:
    void    setCuePath(const QString& path);
    void    setConvertingState(bool converting);
    QString suggestIsoPath(const QString& cuePath) const;

    DropArea*     dropArea_      = nullptr;
    QLabel*       statusLabel_   = nullptr;
    QProgressBar* progressBar_   = nullptr;
    QPushButton*  browseButton_  = nullptr;
    QPushButton*  convertButton_ = nullptr;

    QString  cuePath_;
    QThread* workerThread_ = nullptr;
};

} // namespace iso_converter
