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
QFrame#frame_battery_voltage_3,
QFrame#frame_battery_current_2,
QFrame#frame_battery_current_3,
QFrame#frame_system_alarms_2,
QFrame#frame_screen_power,
QComboBox#combo_calibration_target,
QComboBox#comboBox_battery_type
{
    background-color: white;
    border: 2px solid #C0C0C0;
    border-radius: 8px;
}

/* Battery type list: tall entries with room between them, easy to hit with a finger */
QComboBox#comboBox_battery_type {
    padding-left: 12px;
    combobox-popup: 0;                  /* a normal list, so the entry height below is used */
}

QComboBox#comboBox_battery_type QAbstractItemView {
    background-color: white;
    color: black;
    border: 2px solid #C0C0C0;
    selection-background-color: #1E90FF;
    selection-color: white;
    outline: 0;
}

QComboBox#comboBox_battery_type QAbstractItemView::item {
    min-height: 60px;
    padding-left: 12px;
}

/* Tick boxes (the Function setting on the Boost, Equalize and Initialize pages) */
QCheckBox {
    spacing: 16px;
}

QCheckBox::indicator {
    width: 40px;
    height: 40px;
    background-color: white;
    border: 2px solid #808080;
    border-radius: 8px;
}

QCheckBox::indicator:checked {
    background-color: #1E8E3E;
    border-color: #1E8E3E;
    image: url(:/Recources/ICON/tick.png);
}

QCheckBox::indicator:disabled {
    border-color: #D0D0D0;
}

/* Communication Settings page: text boxes and the list of Wi-Fi networks */
QLineEdit#lineEdit_wifi_password,
QLineEdit#lineEdit_lan_ip,
QLineEdit#lineEdit_lan_mask,
QLineEdit#lineEdit_lan_gateway,
QLineEdit#lineEdit_lan_dns {
    background-color: white;
    color: black;
    border: 2px solid #C0C0C0;
    border-radius: 8px;
    padding: 2px 8px;
}

QLineEdit#lineEdit_lan_ip:disabled,
QLineEdit#lineEdit_lan_mask:disabled,
QLineEdit#lineEdit_lan_gateway:disabled,
QLineEdit#lineEdit_lan_dns:disabled {
    color: #808080;
}

QListWidget#listWidget_wifi {
    background-color: white;
    color: black;
    border: 2px solid #C0C0C0;
    border-radius: 8px;
    outline: 0;
}

QListWidget#listWidget_wifi::item {
    min-height: 52px;
    padding-left: 10px;
}

QListWidget#listWidget_wifi::item:selected {
    background-color: #1E90FF;
    color: white;
}

/* the two LAN choices: larger than the radio buttons on the Display page */
QRadioButton#radioButton_lan_dhcp,
QRadioButton#radioButton_lan_static {
    font-size: 24px;
    font-weight: normal;
    spacing: 16px;
}

QRadioButton#radioButton_lan_dhcp::indicator,
QRadioButton#radioButton_lan_static::indicator {
    width: 36px;
    height: 36px;
    border-radius: 18px;
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

/* a button that cannot be used right now */
QPushButton:disabled {
    background-color: #B8C4CF;
    border: 2px solid #A0ACB8;
    color: #F4F6F8;
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
