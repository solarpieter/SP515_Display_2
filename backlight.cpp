#include "backlight.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QtGlobal>

Backlight::Backlight(QObject *parent, const QString &dir)
    : QObject(parent), m_dir(dir)
{
    if (m_dir.isEmpty())
        m_dir = qEnvironmentVariable("SP515_BACKLIGHT_DIR");

    if (m_dir.isEmpty())
    {
        // The first backlight the system offers (the display's own backlight on a Raspberry Pi)
        const QDir root("/sys/class/backlight");
        const QStringList names = root.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        if (!names.isEmpty())
            m_dir = root.filePath(names.first());
    }

    QFile maxFile(m_dir + "/max_brightness");
    if (!m_dir.isEmpty() && maxFile.open(QIODevice::ReadOnly))
        m_max = maxFile.readAll().trimmed().toInt();

    if (!available())
        qWarning() << "Backlight: no backlight found, the brightness cannot be changed";
}

bool Backlight::setPercent(int percent)
{
    if (!available())
        return false;

    percent = qBound(0, percent, 100);
    int raw = (m_max * percent + 50) / 100;
    if (percent > 0 && raw < 1)
        raw = 1;

    QFile file(m_dir + "/brightness");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) || file.write(QByteArray::number(raw)) < 0)
    {
        m_error = file.errorString();
        if (!m_warned)
        {
            m_warned = true;
            qWarning() << "Backlight: cannot write" << file.fileName() << "-" << m_error
                       << "(a udev rule is needed to allow this without root)";
        }
        return false;
    }
    file.close();
    m_error.clear();
    return true;
}

int Backlight::percent() const
{
    QFile file(m_dir + "/brightness");
    if (!available() || !file.open(QIODevice::ReadOnly))
        return -1;
    const int raw = file.readAll().trimmed().toInt();
    return (raw * 100 + m_max / 2) / m_max;
}
