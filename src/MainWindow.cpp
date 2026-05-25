#include "MainWindow.h"
#include "Converter.h"
#include "DropArea.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>

namespace iso_converter
{

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("CUE/BIN to ISO Converter");
    setMinimumWidth(480);

    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* root = new QVBoxLayout(central);
    root->setSpacing(10);
    root->setContentsMargins(16, 16, 16, 16);

    dropArea_ = new DropArea(this);
    connect(dropArea_, &DropArea::fileDropped, this, &MainWindow::onFileDropped);
    root->addWidget(dropArea_);

    auto* browseRow = new QHBoxLayout;
    browseButton_ = new QPushButton("Browse for .cue file…");
    connect(browseButton_, &QPushButton::clicked, this, &MainWindow::onBrowseClicked);
    browseRow->addStretch();
    browseRow->addWidget(browseButton_);
    browseRow->addStretch();
    root->addLayout(browseRow);

    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(true);
    progressBar_->hide();
    root->addWidget(progressBar_);

    statusLabel_ = new QLabel;
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setWordWrap(true);
    root->addWidget(statusLabel_);

    convertButton_ = new QPushButton("Convert to ISO");
    convertButton_->setEnabled(false);
    convertButton_->setDefault(true);
    connect(convertButton_, &QPushButton::clicked, this, &MainWindow::onConvertClicked);
    root->addWidget(convertButton_);
}

void MainWindow::onFileDropped(const QString& path)
{
    setCuePath(path);
}

void MainWindow::onBrowseClicked()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        "Open CUE file",
        {},
        "CUE files (*.cue);;All files (*)");
    if (!path.isEmpty())
        setCuePath(path);
}

void MainWindow::setCuePath(const QString& path)
{
    cuePath_ = path;
    dropArea_->setText(QString("Selected: %1").arg(QFileInfo(path).fileName()));
    statusLabel_->clear();
    progressBar_->hide();
    progressBar_->setValue(0);
    convertButton_->setEnabled(true);
}

QString MainWindow::suggestIsoPath(const QString& cuePath) const
{
    const QFileInfo fi(cuePath);
    return fi.dir().absoluteFilePath(fi.completeBaseName() + ".iso");
}

void MainWindow::onConvertClicked()
{
    if (cuePath_.isEmpty())
        return;

    const QString isoPath = QFileDialog::getSaveFileName(
        this,
        "Save ISO as",
        suggestIsoPath(cuePath_),
        "ISO image (*.iso);;All files (*)");
    if (isoPath.isEmpty())
        return;

    setConvertingState(true);

    auto* worker = new Converter;
    workerThread_ = new QThread(this);
    worker->moveToThread(workerThread_);

    connect(workerThread_, &QThread::started, worker,
            [worker, cuePath = cuePath_, isoPath]() { worker->convert(cuePath, isoPath); });
    connect(worker, &Converter::progressChanged, this, &MainWindow::onProgressChanged);
    connect(worker, &Converter::finished,        this, &MainWindow::onConversionFinished);
    connect(worker, &Converter::finished,        workerThread_, &QThread::quit);
    connect(workerThread_, &QThread::finished,   worker,        &QObject::deleteLater);
    connect(workerThread_, &QThread::finished,   workerThread_, &QObject::deleteLater);

    progressBar_->show();
    progressBar_->setValue(0);
    workerThread_->start();
}

void MainWindow::onProgressChanged(int percent)
{
    progressBar_->setValue(percent);
    statusLabel_->setText(QString("Converting… %1%").arg(percent));
}

void MainWindow::onConversionFinished(bool success, const QString& message)
{
    workerThread_ = nullptr;
    setConvertingState(false);

    if (success)
    {
        progressBar_->setValue(100);
        statusLabel_->setText(message);
        QMessageBox::information(this, "Conversion complete", message);
    }
    else
    {
        statusLabel_->setText("Error: " + message);
        QMessageBox::critical(this, "Conversion failed", message);
    }
}

void MainWindow::setConvertingState(bool converting)
{
    browseButton_->setEnabled(!converting);
    convertButton_->setEnabled(!converting && !cuePath_.isEmpty());
    dropArea_->setEnabled(!converting);
}

} // namespace iso_converter
