#ifndef DARHTHEME_H
#define DARHTHEME_H

#include <QString>

const QString darkTheme = R"(

QMainWindow {
    background-color: #0A1F4D;
}

QWidget {
    background-color: #0A1F4D;
    color: white;
    font-family: Arial;
}

/* Top Status Area */
QLabel#lblSystemStatus,
QLabel#lblDate,
QLabel#lblTime {
    color: white;
    font-size: 24px;
    font-weight: bold;
    background-color: #0A1F4D;
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
QFrame#frame_battery_cur_3,
QFrame#label_batt_volt_value_3,
QFrame#label_batt_volt_value_4,
QFrame#label_batt_volt_value_5,
QFrame#label_batt_cur_value_6,
QFrame#label_batt_cur_value_7,
QFrame#label_batt_cur_value_8,
QVBoxLayout#verticalLayout_4,
QVBoxLayout#verticalLayout_6,
QComboBox#combo_calibration_target
{
    background-color: #000000;
    border: 2px solid #707070;
    border-radius: 8px;
}

/* Titles */
QLabel {
    color: white;
    background: transparent;
}

/* Battery Mode */
QLabel#lblBatteryModeValue {
    color: white;
    background-color: #00A030;
    border: 2px solid #00D040;
    font-size: 28px;
    font-weight: bold;
}

/* Alarm Status */
QLabel#lblSystemAlarmValue {
    color: #00FF66;
    font-size: 24px;
    font-weight: bold;
}

/* Numerical Values */
QLabel#lblBatteryVoltageValue,
QLabel#lblBatteryCurrentValue,
QLabel#lblLoadVoltageValue,
QLabel#lblLoadCurrentValue {
    color: white;
    font-size: 32px;
    font-weight: bold;
}

/* Buttons */
QPushButton {
    background-color: #18CFE0;
    color: white;
    border: 2px solid #00A0B0;
    border-radius: 15px;
    font-size: 24px;
    font-weight: bold;
}

QPushButton:hover {
    background-color: #25E4F7;
}

QPushButton:pressed {
    background-color: #00A0B0;
}

/* Menu Buttons */
QPushButton#btnMenu,
QPushButton#btnHome,
QPushButton#btnTheme {
    min-height: 50px;
}

/* Radio buttons */
QRadioButton {
    color: white;
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
    background-color: #18CFE0;
    border: 3px solid #FFFFFF;
}

/* Number, date and time editors */
QDoubleSpinBox,
QDateEdit,
QTimeEdit {
    background-color: #000000;
    color: white;
    border: 2px solid #707070;
    border-radius: 8px;
    padding: 2px;
}

/* Progress bar */
QProgressBar {
    background-color: #000000;
    border: 2px solid #707070;
    border-radius: 6px;
}

QProgressBar::chunk {
    background-color: #18CFE0;
    border-radius: 4px;
}

/* On-screen numeric keypad */
QFrame#numericKeypad {
    border: 2px solid #707070;
    border-radius: 10px;
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

#endif // DARHTHEME_H
