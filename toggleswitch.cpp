#include "toggleswitch.h"

#include <QEasingCurve>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

ToggleSwitch::ToggleSwitch(QWidget *parent)
    : QPushButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);

    m_animation = new QPropertyAnimation(this, "position", this);
    m_animation->setDuration(160);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(this, &QAbstractButton::toggled, this, &ToggleSwitch::animateToState);
    m_position = isChecked() ? 1.0 : 0.0;
}

void ToggleSwitch::animateToState(bool on)
{
    const qreal target = on ? 1.0 : 0.0;
    if (!isVisible())
    {
        m_animation->stop();
        setPosition(target);                    // not on screen yet: no need to slide
        return;
    }
    m_animation->stop();
    m_animation->setStartValue(m_position);
    m_animation->setEndValue(target);
    m_animation->start();
}

void ToggleSwitch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled())
        p.setOpacity(0.45);

    // Caption (the button text) goes under the switch
    const bool hasCaption = !text().isEmpty();
    const int captionHeight = hasCaption ? qMax(30, height() / 4) : 0;

    const qreal margin = 6.0;
    const QRectF area(margin, margin, width() - 2 * margin, height() - 2 * margin - captionHeight);
    const qreal trackHeight = qMin(area.height(), area.width() * 0.52);
    const QRectF track(area.left(), area.top() + (area.height() - trackHeight) / 2.0,
                       area.width(), trackHeight);
    const qreal radius = track.height() / 2.0;

    // Track: blends from the OFF colour to the ON colour while the knob slides
    QColor trackColor(
        int(m_offColor.red()   + (m_onColor.red()   - m_offColor.red())   * m_position),
        int(m_offColor.green() + (m_onColor.green() - m_offColor.green()) * m_position),
        int(m_offColor.blue()  + (m_onColor.blue()  - m_offColor.blue())  * m_position));
    p.setPen(Qt::NoPen);
    p.setBrush(trackColor);
    p.drawRoundedRect(track, radius, radius);

    // Knob
    const qreal knobMargin = track.height() * 0.1;
    const qreal knobDiameter = track.height() - 2 * knobMargin;
    const qreal travel = track.width() - knobDiameter - 2 * knobMargin;
    const qreal knobX = track.left() + knobMargin + travel * m_position;
    QRectF knob(knobX, track.top() + knobMargin, knobDiameter, knobDiameter);
    if (isDown())
        knob.adjust(-2, -2, 2, 2);                          // a little bigger while pressed
    p.setBrush(QColor(0, 0, 0, 70));
    p.drawEllipse(knob.translated(0, 3));                    // soft shadow
    p.setBrush(m_knobColor);
    p.drawEllipse(knob);

    // ON / OFF word on the side the knob has left free
    QFont stateFont = font();
    stateFont.setBold(true);
    stateFont.setPixelSize(qRound(track.height() * 0.36));
    p.setFont(stateFont);
    p.setPen(m_textColor);
    const bool on = m_position > 0.5;
    const QRectF wordArea = on ? QRectF(track.left() + knobMargin, track.top(), travel * 0.9, track.height())
                               : QRectF(track.right() - knobMargin - travel * 0.9, track.top(), travel * 0.9, track.height());
    p.drawText(wordArea, Qt::AlignCenter, on ? "ON" : "OFF");

    // Caption
    if (hasCaption)
    {
        QFont captionFont = font();
        captionFont.setBold(true);
        captionFont.setPixelSize(qRound(captionHeight * 0.62));
        p.setFont(captionFont);
        p.setPen(m_captionColor);
        p.drawText(QRectF(0, height() - captionHeight - margin / 2, width(), captionHeight),
                   Qt::AlignCenter, text());
    }
}
