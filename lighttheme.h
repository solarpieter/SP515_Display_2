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
QFrame#frameTopdashboard,
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
QFrame#frame_dashboard_12,
QFrame#frame_battery_voltage_2,
QFrame#frame_battery_current_2,
QFrame#frame_system_alarms_2,
QFrame#frame_screen_power,
QComboBox#combo_calibration_target
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

/* Menu Buttons */
QPushButton#btnMenu,
QPushButton#btnHome,
QPushButton#btnTheme {
    min-height: 50px;
}

/* Radio buttons */
QRadioButton {
    color: black;
    spacing: 10px;
    font-size: 16px;
    font-weight: bold;
}

QRadioButton::indicator {
    width: 22px;
    height: 22px;
    border: 3px solid #0060C0;
    border-radius: 11px;
    background-color: white;
}

QRadioButton::indicator:checked {
    background-color: #1997FF;
    border: 3px solid #0060C0;
}

/* Number, date and time editors */
QSpinBox,
QDoubleSpinBox,
QDateEdit,
QTimeEdit {
    background-color: white;
    color: black;
    border: 2px solid #C0C0C0;
    border-radius: 8px;
    padding: 2px;
}

/* Progress bar */
QProgressBar {
    background-color: white;
    border: 2px solid #C0C0C0;
    border-radius: 6px;
}

QProgressBar::chunk {
    background-color: #1997FF;
    border-radius: 4px;
}

/* Sliders (back light) */
QSlider::groove:horizontal {
    background-color: white;
    border: 2px solid #C0C0C0;
    height: 10px;
    border-radius: 6px;
}

QSlider::sub-page:horizontal {
    background-color: #1997FF;
    border-radius: 6px;
}

QSlider::handle:horizontal {
    background-color: #1997FF;
    border: 2px solid #0060C0;
    width: 40px;
    margin: -16px 0;
    border-radius: 20px;
}

/* On-screen numeric keypad */
QFrame#numericKeypad {
    border: 2px solid #C0C0C0;
    border-radius: 10px;
}

/* Charger ON / OFF switch */
ToggleSwitch#btnChargerEnable {
    qproperty-onColor: #00A030;
    qproperty-offColor: #AA0000;
    qproperty-knobColor: #FFFFFF;
    qproperty-textColor: #FFFFFF;
    qproperty-captionColor: #000000;
}
)";

#endif // LIGHTTHEME_H
