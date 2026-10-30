#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <QObject>
#include <QString>

// Display backlight, through the Linux sysfs interface (/sys/class/backlight/<name>/).
// brightness is written as a raw value between 0 and max_brightness; this class works in percent.
//
// Writing normally needs permission. Either run the app with a udev rule such as
//   SUBSYSTEM=="backlight", RUN+="/bin/chmod 666 /sys/class/backlight/%k/brightness"
// or the writes will fail (the display then simply keeps its brightness).
//
// For testing, the folder can be given in the environment variable SP515_BACKLIGHT_DIR.
class Backlight : public QObject
{
    Q_OBJECT

public:
    explicit Backlight(QObject *parent = nullptr, const QString &dir = QString());

    bool available() const { return m_max > 0; }
    QString path() const { return m_dir; }

    // 0 .. 100. Any value above 0 is at least the dimmest raw step, so the screen is never
    // switched off by accident. Returns false if the value could not be written.
    bool setPercent(int percent);

    int percent() const;                 // read back from the hardware, -1 if unknown
    QString lastError() const { return m_error; }

private:
    QString m_dir;
    int m_max = 0;
    QString m_error;
    bool m_warned = false;
};

#endif // BACKLIGHT_H
