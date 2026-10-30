#include "screensaver.h"

#include <QFont>
#include <QPaintEvent>
#include <QPainter>
#include <QRandomGenerator>

ScreenSaver::ScreenSaver(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setCursor(Qt::BlankCursor);
    hide();

    m_timer.setInterval(33);             // about 30 frames per second
    connect(&m_timer, &QTimer::timeout, this, &ScreenSaver::tick);
}

void ScreenSaver::start(bool alarm)
{
    if (parentWidget())
        setGeometry(parentWidget()->rect());
    raise();
    show();
    setAlarm(alarm);
    update();
}

void ScreenSaver::stop()
{
    m_timer.stop();
    m_alarm = false;
    hide();
}

void ScreenSaver::setAlarm(bool alarm)
{
    if (alarm == m_alarm && (alarm == m_timer.isActive()))
        return;

    m_alarm = alarm;
    if (alarm)
    {
        m_button = QRect(0, 0, 300, 110);
        const int maxX = qMax(1, width() - m_button.width());
        const int maxY = qMax(1, height() - m_button.height());
        m_button.moveTo(QRandomGenerator::global()->bounded(maxX), QRandomGenerator::global()->bounded(maxY));
        m_velocity = QPoint(QRandomGenerator::global()->bounded(2) ? 5 : -5,
                            QRandomGenerator::global()->bounded(2) ? 3 : -3);
        m_timer.start();
    }
    else
    {
        m_timer.stop();
    }
    update();
}

void ScreenSaver::tick()
{
    const QRect old = m_button;
    m_button.translate(m_velocity);

    if (m_button.left() < 0)               { m_button.moveLeft(0);                         m_velocity.setX(qAbs(m_velocity.x())); }
    if (m_button.top() < 0)                { m_button.moveTop(0);                          m_velocity.setY(qAbs(m_velocity.y())); }
    if (m_button.right() >= width())       { m_button.moveRight(width() - 1);              m_velocity.setX(-qAbs(m_velocity.x())); }
    if (m_button.bottom() >= height())     { m_button.moveBottom(height() - 1);            m_velocity.setY(-qAbs(m_velocity.y())); }

    update(old.united(m_button).adjusted(-2, -2, 2, 2));     // only repaint what moved
}

void ScreenSaver::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.fillRect(event->rect(), Qt::black);

    if (!m_alarm)
        return;

    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::white, 3));
    p.setBrush(QColor(0xE0, 0x10, 0x10));
    p.drawRoundedRect(m_button, 24, 24);

    QFont font = p.font();
    font.setBold(true);
    font.setPixelSize(52);
    p.setFont(font);
    p.drawText(m_button, Qt::AlignCenter, "ALARM");
}
