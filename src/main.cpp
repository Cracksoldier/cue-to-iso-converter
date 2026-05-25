#include "MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ISO Converter");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("iso-converter");

    iso_converter::MainWindow window;
    window.show();

    return app.exec();
}
