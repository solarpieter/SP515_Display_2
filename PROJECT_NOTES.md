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

Current pages: `pageDashboard`, `pageMenu`, `pageCalibration`, `pageBattery_settings`, `pageAlarm_settings`, `pageDisplay_settings`, `pageDate_settings`, `pageTime_settings`, `pageEnable_Disable`.

## Open items

- **Battery Settings** and **Alarm Settings** buttons have empty slots. The pages exist but nothing navigates to them. Each page also needs a BACK button.
- **Calibrate** and **Apply Offset** buttons on the calibration page are not wired to anything yet.
- Add a hidden exit for the fullscreen app, if one is wanted.
- Fill in the remaining `TODO`s in `README.md`: Qt version and packages, UART device name, data/parity/stop bits (probably 8N1), bus termination, names of the send and receive apps, and how the display app talks to them.
- Decide and document the link between the display app and the send/receive apps (socket, pipe, etc.).
- Check the other pages for edits that should use the numeric keypad.
