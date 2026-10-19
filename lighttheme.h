#ifndef LIGHTTHEME_H
#define LIGHTTHEME_H

#include <QString>

const QString lightTheme = R"(

QMainWindow {
    background-color: #EAEAEA;
}

QWidget {
    background-color: #EAEAEA;
    color: black;
    font-family: Arial;
}

/* Top Status Area */
QLabel#lblSystemStatus,
QLabel#lblDate,
QLabel#lblTime {
    color: black;
    font-size: 24px;
    font-weight: bold;
    background-color: #EAEAEA;
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
    background-color: white;
    border: 2px solid #C0C0C0;
    border-radius: 8px;
}

/* General Labels */
QLabel {
    background: transparent;
    color: black;
}

/* Battery Mode */
QLabel#lblBatteryModeValue {
    background-color: #00B140;
    color: white;
    font-size: 28px;
    font-weight: bold;
}

/* Alarm Status */
QLabel#lblSystemAlarmValue {
    color: #008F30;
    font-size: 24px;
    font-weight: bold;
}

/* Numeric Values */
QLabel#lblBatteryVoltageValue,
QLabel#lblBatteryCurrentValue,
QLabel#lblLoadVoltageValue,
QLabel#lblLoadCurrentValue {
    color: black;
    font-size: 32px;
    font-weight: bold;
}

/* Buttons */
QPushButton {
    background-color: #1997FF;
    color: white;
    border: 2px solid #0060C0;
    border-radius: 15px;
    font-size: 24px;
    font-weight: bold;
}

QPushButton:hover {
    background-color: #47AEFF;
}

QPushButton:pressed {
    background-color: #0060C0;
}

QPushButton#btnMenu,
QPushButton#btnHome,
QPushButton#btnTheme {
    min-height: 50px;
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

#endif // LIGHTTHEME_H
