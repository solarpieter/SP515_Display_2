#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QRect>
#include <QTimer>
#include <QWidget>

// Black cover over the whole window. While an alarm is active it shows a red "ALARM" button that
// bounces around the screen; otherwise the screen is simply black. Touch handling (waking up)
// is done by the main window.
class ScreenSaver : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenSaver(QWidget *parent);

    void start(bool alarm);              // cover the parent window
    void setAlarm(bool alarm);           // show or hide the bouncing button
    void stop();                         // uncover

    bool alarmShown() const { return m_alarm; }
    QRect buttonRect() const { return m_button; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void tick();

    QTimer m_timer;
    QRect m_button;
    QPoint m_velocity{5, 3};             // pixels per tick
    bool m_alarm = false;
};

#endif // SCREENSAVER_H
