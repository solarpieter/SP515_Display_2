#ifndef HIGHCONTRASTTHEME_H
#define HIGHCONTRASTTHEME_H

#include <QString>

const QString highContrastTheme = R"(
QMainWindow {
    background-color: #000000;
}

QWidget {
    background-color: #000000;
    color: #FFFFFF;
    font-family: Arial;
}

/* Top Status Area */
QLabel#lblSystemStatus,
QLabel#lblDate,
QLabel#lblTime {
    color: #FFFF00;
    font-size: 24px;
    font-weight: bold;
    background-color: #000000;
}

/* Measurement Frames */
QFrame#frame_battery_voltage,
QFrame#frame_battery_current,
QFrame#frame_battery_mode,
QFrame#frame_load_voltage,
QFrame#frame_load_current,
QFrame#frame_system_alarms,
QFrame#frame_battery_temperature,
QFrame#frame_dashboard_8,
QFrame#frame_dashboard_9,
QFrame#frame_dashboard_10,
QFrame#frame_dashboard_11,
QFrame#frame_dashboard_12
{
    background-color: #000000;
    border: 3px solid #FFFFFF;
    border-radius: 8px;
}

/* General Labels */
QLabel {
    color: #FFFFFF;
    background: transparent;
}

/* Battery Mode */
QLabel#lblBatteryModeValue {
    color: #000000;
    background-color: #FFFF00;
    border: 3px solid #FFFFFF;
    font-size: 28px;
    font-weight: bold;
}

/* Alarm Status */
QLabel#lblSystemAlarmValue {
    color: #00FF00;
    font-size: 24px;
    font-weight: bold;
}

/* Numerical Values */
QLabel#lblBatteryVoltageValue,
QLabel#lblBatteryCurrentValue,
QLabel#lblLoadVoltageValue,
QLabel#lblLoadCurrentValue {
    color: #FFFF00;
    font-size: 32px;
    font-weight: bold;
}

/* Buttons */
QPushButton {
    background-color: #FFFF00;
    color: #000000;
    border: 3px solid #FFFFFF;
    border-radius: 15px;
    font-size: 24px;
    font-weight: bold;
}

QPushButton:hover {
    background-color: #FFFFFF;
    color: #000000;
}

QPushButton:pressed {
    background-color: #00FFFF;
    color: #000000;
}

QPushButton#btnMenu,
QPushButton#btnHome,
QPushButton#btnTheme {
    min-height: 50px;
}

/* Radio buttons */
QRadioButton {
    color: #FFFFFF;
    spacing: 10px;
    font-size: 16px;
    font-weight: bold;
}

QRadioButton::indicator {
    width: 22px;
    height: 22px;
    border: 3px solid #FFFFFF;
    border-radius: 11px;
    background-color: #000000;
}

QRadioButton::indicator:checked {
    background-color: #FFFF00;
    border: 3px solid #FFFFFF;
}

QDateEdit,
QTimeEdit {
    background-color: #000000;
    color: #FFFF00;
    border: 3px solid #FFFFFF;
    padding: 6px;
    font-size: 18px;
    font-weight: bold;
}
QPushButton#btnChargerEnable
 {
    background-color: #AA0000;
    color: white;
    border: 2px solid #880000;
    border-radius: 15px;
    font-weight: bold;
}

QPushButton#btnChargerEnable:checked
{
    background-color: #00A030;
    color: white;
    border: 2px solid #008020;
}
)";

#endif // HIGHCONTRASTTHEME_H
