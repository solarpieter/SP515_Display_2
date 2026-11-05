# SP515_Display_2: Development Notes

Summary of the setup and development work done so far. For the protocol and wiring reference, see `README.md`.

## Setup

- **Target:** Raspberry Pi 4B with the 7" Raspberry Pi Touch Display 2 (1280 x 720, landscape). A second, larger monitor is also connected during development.
- **Toolchain:** Qt Widgets (Qt 6), CMake with Ninja, built in `build/Desktop-Debug` by Qt Creator.
- **Repository:** https://github.com/solarpieter/SP515_Display_2 (branch `main`).
- **GitHub login:** passwords are not accepted. Push with a personal access token (or SSH keys).
- **`.gitignore`:** build output (`build/`, Ninja files, generated Qt files) is ignored and must never be committed. A tracked `build/` folder was removed from the repo early on.

## Hardware and communication

- Comms to the SP515 is RS485 at **115200 baud**, using the same protocol as the earlier STM32 display (see `README.md`).
- A MAX485 is used on the SP515 board and another on the Pi.
- **Pi wiring:** GPIO14 (TXD) to DI, GPIO15 (RXD) to RO, and one GPIO (GPIO17 suggested) to DE and /RE tied together. Direction is handled in software by the send app: set high, write, wait for the last byte to leave (`tcdrain()`), set low.
- **Voltage warning:** a standard MAX485 is a 5 V part, and its RO output can damage the Pi's 3.3 V RX pin. Use a 3.3 V transceiver (MAX3485 or SP3485) or level-shift RO.
- **Serial port:** add `dtoverlay=disable-bt` to `/boot/firmware/config.txt` so the better PL011 UART is on GPIO14/15.
- **Architecture:** the Qt display app talks to separate send and receive apps on the Pi, which talk to the UART. The mechanism between the display app and those apps is still to be decided (see open items).

## Numeric keypad

An on-screen numeric keypad (`numerickeypad.h`, `numerickeypad.cpp`) is used for number entry, because most edits in the project are numeric.

- Keys: 0 to 9, `-`, `.`, backspace (hold to repeat) and Enter.
- It stays hidden until a registered edit is tapped. Enter commits the value, hides the keypad and takes focus off the edit.
- Tapping a box selects the old value, so the first key replaces it.
- It works with `QLineEdit` and `QSpinBox` / `QDoubleSpinBox`.
- It sits in the bottom-right corner of the visible window.
- It is created in the `MainWindow` constructor for `pageCalibration` and registered for `spin_measured_value` and `spin_offset_value`. Both are set to the range -9999.99 to 9999.99 and always use `.` as the decimal point.
- **To use it on another page:** create `new NumericKeypad(ui->pageName)` and call `registerEdit()` for each edit on that page.

Lessons learned:
- Do not give the page `Qt::ClickFocus`. Tapping a keypad button then moves focus to the page and the keypad types into the wrong place.
- The OS on-screen keyboard is disabled and is not needed. The keypad is part of the app.

## Fullscreen and screen selection

- `main.cpp` runs the app fullscreen with `showFullScreen()`, so there is no title bar. Press **Alt+F4** to quit.
- The app opens on the screen that is exactly 1280 x 720, otherwise the primary screen. To force a screen, set `SP515_SCREEN=<name>` (the screen names are printed at startup).
- The old `qputenv("QT_IM_MODULE", "qtvirtualkeyboard")` line was removed from `main.cpp`.
- If touch input lands on the wrong screen, fix the touchscreen mapping in the Pi's Screen Configuration settings.
- `mainwindow.ui` still has the main window at 1280 x 654 while the page area is 720 tall. Fullscreen hides this, but setting the window height to 720 in Qt Designer would remove the mismatch.

## Page navigation

Pages are selected **by name, not by number**. Page numbers change whenever a page is inserted, which broke several buttons after the Calibration page was added.

```cpp
ui->stackedWidget->setCurrentWidget(ui->pageDate_settings);
```

To add a page:
1. In Qt Designer, right-click `stackedWidget` and choose Insert Page After (or Before).
2. Give the page a clear `objectName`, for example `pageSystemInfo`.
3. In the button's `clicked` slot, call `setCurrentWidget(ui->pageSystemInfo)`.

The app always starts on the dashboard (set in the `MainWindow` constructor), because Qt Designer saves whichever page was open last as the start page.

Current pages: `pageDashboard`, `pageMenu`, `pageCalibration`, `pageBattery_settings`, `pageAlarm_settings`, `pageDisplay_settings`, `pageDate_settings`, `pageTime_settings`, `pageEnable_Disable`, `pageCharger_settings`, `pagePID_settings`.

## Open items

- **Alarm Settings** has an empty slot. The page exists but nothing navigates to it. It also needs a BACK button.
- **Battery Settings:** recharge rate, recharge efficiency and temperature compensation can be edited (the page then shows an orange note) but are not sent, because no command carries them (the profile values live in the SP515 firmware) and the daemon does not support the compensation commands 16 and 17 yet. Once they exist, enable the boxes and add them to the load and save code. The battery type list has six entries, but the SP515 stores only the chemistry (lead acid, NiCad, lithium), so Flooded and VRLA both save as lead acid and NiCad L, M and H as NiCad. The daemon answers "failed" both when the SP515 rejects a value and when it does not answer, so the page cannot tell those two apart.
- **Calibrate** and **Apply Offset** buttons on the calibration page are not wired to anything yet.
- Add a hidden exit for the fullscreen app, if one is wanted.
- Fill in the remaining `TODO`s in `README.md`: Qt version and packages, UART device name, data/parity/stop bits (probably 8N1), bus termination, names of the send and receive apps, and how the display app talks to them.
- Decide and document the link between the display app and the send/receive apps (socket, pipe, etc.).
- Check the other pages for edits that should use the numeric keypad.
- **PID Settings:** the daemon does not support commands 25 and 26 yet (GET / SET PID config, 16 bytes: four floats in the order voltage Kp, voltage Ki, current Kp, current Ki), so the page shows "command not supported by the daemon yet" and the boxes stay disabled until it does. The rectifier readout polls every 200 ms, but the daemon only refreshes its measurements from the SP515 every `MEASUREMENT_POLL_MS` (500 ms in `app/rs485d/app_rs485d.c`); lower that to 200 or less to see every update. While a save is running (up to 5 s) the readout pauses, because the display sends one request at a time.
- **Charge mode names:** `chargeModeName()` at the top of `mainwindow.cpp` turns the mode number from the SP515 status into a name (NONE, FLOAT, BOOST, EQUALIZE, INITIALIZE, BATT TEST). The order must match `eChargeMode_t` in the firmware's `service_charge.h`; a new mode needs a new entry.
- **Charger ON needs a setpoint the display does not have:** the SP515 only enables the charger together with a voltage and a current limit (the bench shortcut `CHARGE_SetBenchSetpoint`), and the display has no source for them (`MainWindow::setChargerSetpoint()` is never called), so ON currently always stops with "have not been set yet". The charger should take its voltage and current limit from the settings stored in the SP515, so a display is not needed to run it. Either the firmware and daemon get an enable command that needs no values, or the display builds the setpoint from the SP515's stored float config and battery config. The first is cleaner.
- **Boost, Equalize and Initialize pages** (`setupModePages()`, one shared implementation, opened from Battery Settings; BACK returns there). Field mapping to the SP515 config: Function tick box = the `enabled` flag (it only enables or disables the function; the mode is started on the Battery Modes page); Volts / Cell = `cellV`; Duration (s) = `durationS`; Recharge Current Limit % = `batteryCurLim` converted with the battery config (amps = % x capacity x strings / 100); Override Timer (h) = `overrideCounterS` / 3600, a setting (for example 12 h): on the SP515 it starts counting when boost is activated and gives an auto boost failure when it elapses; on Boost only: Repeat every (days) = `abCycleS` / 86400 and Gassing Voltage = `gassingCellVoltage`. Still to confirm on the real SP515: that `batteryCurLim` is in amps. The `autoBoostEnabled` flag (Boost) and `onLoadEnabled` flag (Equalize) have no widget yet and are written back unchanged.
- **Not wired yet:** the Battery Modes page (`pageBatteryModes`, for starting and stopping a mode) and the Battery Test Settings button on Battery Settings. The firmware has `CHARGE_RequestMode()` but no RS485 command for it.
- **Override timer on Equalize and Initialize:** the SP515 configs for these two do not have it yet, so the box can be edited but is not sent. When the daemon and firmware add it, set its position in `setupModePages()` (`eq.offset[FieldOverride]` and `init.offset[FieldOverride]`, now -1). There is no Repeat on these two pages.
- **Battery Modes page rule:** the button that starts a mode must be greyed out while that mode's Function switch (the `enabled` flag in its config) is off. The page is not built yet, and there is no RS485 command to start a mode (the firmware has `CHARGE_RequestMode()` only).
- **Touch-friendly controls:** the up / down arrows are removed from every `QSpinBox` and `QDoubleSpinBox` in the `MainWindow` constructor (numbers are typed on the keypad); the date and time boxes keep theirs. The battery type list uses `combobox-popup: 0` and a 60 px entry height in the three theme files. The Function tick boxes (`checkBox_boost_enable`, `checkBox_equalize_enable`, `checkBox_initialize_enable`) are styled in the theme files too; the tick itself is `Recources/ICON/tick.png` (alias `tick.png` in `Recources.qrc`).
