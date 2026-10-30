#ifndef TOGGLESWITCH_H
#define TOGGLESWITCH_H

#include <QColor>
#include <QPropertyAnimation>
#include <QPushButton>

// A checkable button drawn as a slide switch: a rounded track with a round knob that
// slides to the right when ON. It still behaves like a normal QPushButton (isChecked(),
// setChecked(), toggled(), clicked()); the button text is shown as a caption under the switch.
//
// The colours can be set from the theme style sheets, for example:
//   ToggleSwitch#btnChargerEnable { qproperty-onColor: #00A030; qproperty-offColor: #AA0000;
//                                   qproperty-captionColor: #FFFFFF; }
class ToggleSwitch : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QColor onColor READ onColor WRITE setOnColor)
    Q_PROPERTY(QColor offColor READ offColor WRITE setOffColor)
    Q_PROPERTY(QColor knobColor READ knobColor WRITE setKnobColor)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)          // ON / OFF word on the track
    Q_PROPERTY(QColor captionColor READ captionColor WRITE setCaptionColor) // text under the switch
    Q_PROPERTY(qreal position READ position WRITE setPosition)   // 0 = OFF, 1 = ON (animated)

public:
    explicit ToggleSwitch(QWidget *parent = nullptr);

    QColor onColor() const { return m_onColor; }
    QColor offColor() const { return m_offColor; }
    QColor knobColor() const { return m_knobColor; }
    QColor textColor() const { return m_textColor; }
    QColor captionColor() const { return m_captionColor; }
    void setOnColor(const QColor &c) { m_onColor = c; update(); }
    void setOffColor(const QColor &c) { m_offColor = c; update(); }
    void setKnobColor(const QColor &c) { m_knobColor = c; update(); }
    void setTextColor(const QColor &c) { m_textColor = c; update(); }
    void setCaptionColor(const QColor &c) { m_captionColor = c; update(); }

    qreal position() const { return m_position; }
    void setPosition(qreal p) { m_position = p; update(); }

    QSize sizeHint() const override { return QSize(241, 141); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void animateToState(bool on);

    QColor m_onColor{0x00, 0xA0, 0x30};
    QColor m_offColor{0xAA, 0x00, 0x00};
    QColor m_knobColor{Qt::white};
    QColor m_textColor{Qt::white};
    QColor m_captionColor{Qt::white};
    qreal m_position = 0.0;
    QPropertyAnimation *m_animation = nullptr;
};

#endif // TOGGLESWITCH_H
