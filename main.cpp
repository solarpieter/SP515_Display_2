#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QScreen>

// Pick the screen to show the display on: a screen named in SP515_SCREEN,
// otherwise the 1280x720 touch display, otherwise the primary screen.
static QScreen *pickScreen()
{
    const auto screens = QGuiApplication::screens();
    for (QScreen *s : screens)
        qInfo() << "Screen:" << s->name() << s->size();

    const QString wanted = qEnvironmentVariable("SP515_SCREEN");
    if (!wanted.isEmpty()) {
        for (QScreen *s : screens)
            if (s->name() == wanted)
                return s;
    }
    for (QScreen *s : screens)
        if (s->size() == QSize(1280, 720))
            return s;
    return QGuiApplication::primaryScreen();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QScreen *screen = pickScreen();
    w.setScreen(screen);
    w.setGeometry(screen->geometry());   // place it on that screen before going fullscreen
    w.showFullScreen();
    return QApplication::exec();
}
