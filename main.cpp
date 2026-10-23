#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    // Configure Qt to deploy the native virtual keyboard subsystem
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
