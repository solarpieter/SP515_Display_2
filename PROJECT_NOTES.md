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

- `main.cpp` runs the app fullscreen with `showFullScreen()`, so there is no title bar. Press **Alt+F4** to quit, or use the **EXIT** button on the Menu page.
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
- **Battery Settings:** the Recharge Rate box is the float current limit (`batteryCurLim` in the float config, amps) shown as a % of capacity x strings, and is saved with the float config (command 7) together with the Float V/Cell; only the edited field is changed. Recharge efficiency and temperature compensation can be edited (the page then shows an orange note) but are not sent, because no command carries the efficiency (the profile value lives in the SP515 firmware) and the daemon does not support the compensation commands 16 and 17 yet. Once they exist, add them to the load and save code. The battery type list has six entries, but the SP515 stores only the chemistry (lead acid, NiCad, lithium), so Flooded and VRLA both save as lead acid and NiCad L, M and H as NiCad. The daemon answers "failed" both when the SP515 rejects a value and when it does not answer, so the page cannot tell those two apart.
- Fill in the remaining `TODO`s in `README.md`: Qt version and packages, UART device name, data/parity/stop bits (probably 8N1), bus termination, names of the send and receive apps, and how the display app talks to them.
- Decide and document the link between the display app and the send/receive apps (socket, pipe, etc.).
- Check the other pages for edits that should use the numeric keypad.
- **PID Settings:** the daemon does not support commands 25 and 26 yet (GET / SET PID config, 16 bytes: four floats in the order voltage Kp, voltage Ki, current Kp, current Ki), so the page shows "command not supported by the daemon yet" and the boxes stay disabled until it does. The rectifier readout polls every 200 ms, but the daemon only refreshes its measurements from the SP515 every `MEASUREMENT_POLL_MS` (500 ms in `app/rs485d/app_rs485d.c`); lower that to 200 or less to see every update. While a save is running (up to 5 s) the readout pauses, because the display sends one request at a time.
- **Charge mode names:** `chargeModeName()` at the top of `mainwindow.cpp` turns the mode number from the SP515 status into a name (NONE, FLOAT, BOOST, EQUALIZE, INITIALIZE, BATT TEST). The order must match `eChargeMode_t` in the firmware's `service_charge.h`; a new mode needs a new entry.
- **Charger ON:** the display works the setpoint out from the SP515's own settings when ON is pressed (voltage = float V/Cell x cells per string; current limit = the float `batteryCurLim`, the recharge rate) and shows both for a Yes / No answer. To confirm on the real SP515: that `batteryCurLim` is in amps and that the current limit in command 23 is in amps. The cleaner long-term design is still an enable command in the firmware that starts float mode with the SP515's own stored settings, so no numbers pass through the display.
- **Boost, Equalize and Initialize pages** (`setupModePages()`, one shared implementation, opened from Battery Settings; BACK returns there). Field mapping to the SP515 config: Function tick box = the `enabled` flag (it only enables or disables the function; the mode is started on the Battery Modes page); Volts / Cell = `cellV`; Duration (s) = `durationS`; Recharge Current Limit % = `batteryCurLim` converted with the battery config (amps = % x capacity x strings / 100). Boost only: Override Timer (h) = `overrideCounterS` / 3600, a setting (for example 12 h) that starts counting when boost is activated and gives an auto boost failure when it elapses; Repeat every (days) = `abCycleS` / 86400; Gassing Voltage = `gassingCellVoltage`. Equalize only: Load On Equalize (Yes / No) = the `onLoadEnabled` flag. Initialize has none of these extras. The `autoBoostEnabled` flag (Boost) has no widget yet and is written back unchanged. Still to confirm on the real SP515: that `batteryCurLim` is in amps.
- **Not wired yet:** the Battery Test Settings button on Battery Settings, and the start buttons on the Battery Modes page (`pageBatteryModes`, opened from the Menu; its Home and Back work). The firmware has `CHARGE_RequestMode()` but no RS485 command for it.
- **Battery Modes page rule:** the button that starts a mode must be greyed out while that mode's Function switch (the `enabled` flag in its config) is off. The page is not built yet, and there is no RS485 command to start a mode (the firmware has `CHARGE_RequestMode()` only).
- **Touch-friendly controls:** the up / down arrows are removed from every `QSpinBox` and `QDoubleSpinBox` in the `MainWindow` constructor (numbers are typed on the keypad); the date and time boxes keep theirs. The battery type list uses `combobox-popup: 0` and a 60 px entry height in the three theme files. The Function tick boxes (`checkBox_boost_enable`, `checkBox_equalize_enable`, `checkBox_initialize_enable`) are styled in the theme files too; the tick itself is `Recources/ICON/tick.png` (alias `tick.png` in `Recources.qrc`).
- **Communication Settings page** (`setupCommPage()`, `netcontrol.cpp`, `textkeyboard.cpp`): Wi-Fi and the LAN port of the Pi the display runs on, through NetworkManager's `nmcli` (started without a shell, in the background). The settings belong to that Pi and are kept by NetworkManager; the display stores nothing. What it runs: `nmcli -t -f DEVICE,TYPE device` (find the Wi-Fi and wired ports), `-f WIFI radio` and `radio wifi on|off`, `device wifi list --rescan yes`, `device wifi connect SSID password PWD` (for a network with a saved profile: `connection modify SSID wifi-sec.psk PWD` and `connection up SSID`), and for the LAN `connection modify PROFILE ipv4.method auto|manual ipv4.addresses A/PREFIX ipv4.gateway G ipv4.dns D` followed by `connection up PROFILE`. The static values are checked first (valid address, mask, gateway on the same network). IPv4 only, one wired port, one DNS server. The text keyboard (`TextKeyboard`) is used for the Wi-Fi password; the numeric keypad for the addresses.
- **To check on the Pi:** `nmcli` must be allowed for the logged-in desktop user without a password (it is, for the active desktop session on Raspberry Pi OS; it is refused over SSH or when started as a system service, and the page then shows "Not authorized to control networking"). A Wi-Fi password is passed to `nmcli` as an argument, so it is briefly visible in the process list. Cabled LAN changes take effect at once and can cut a remote (SSH) session.
- **Lithium battery page** (`pagelithium_batt`, `setupLithiumPage()`): reachable only while the SP515 has a lithium battery selected (`eBattType_t` 3). The type is read from the SP515's battery config every 5 s with a quiet request (`Rs485Client::requestBatteryType()`, answered only through `batteryTypeReceived()` so it never mixes with a page reading the same config), and is also updated when the Battery Settings page loads or saves. Until the type is known, or when the daemon cannot be reached, the dashboard battery icon and the Lithium button on Battery Settings are greyed out; if lithium is deselected while the page is open it returns to the dashboard. Disabled buttons now look greyed out in all three themes (`QPushButton:disabled`). The page itself still has no information: it needs a source for the lithium battery data (the SP515 has no command for it yet).
- **Charger switch on the PID page** (`btnChargerEnable_pid`): shares the ON / OFF logic with the Enable / Disable page switch (`beginChargerOn()`, `requestChargerState()`; both switches always show the SP515 state through `setChargerState()`), but starts with `chargerImmediate = true`: no Yes / No question, and `reportChargerIssue()` writes problems next to the switch (`label_pid_charger_status`) instead of opening a pop-up. The Enable / Disable page sets `chargerImmediate = false`, so it still asks first.
- **Calibration** (`calibrationChannels[]` at the top of `mainwindow.cpp`): all the maths is on the SP515. Calibrate sends SET_ANA_GAIN (channel, the multimeter value typed in Measured Value), Apply Offset sends SET_ANA_OFFSET (channel, the typed offset); Current Value is the live reading and Current offset is read from GET_ANA_GAINS (when the page opens, every second while it is open (`calibrationTimer`, one request at a time), when the selection changes and after every successful Calibrate or Apply Offset). Choosing a different measurement calls `clearCalibrationEntries()`: Measured Value and New Offset back to 0, OK / FAIL hidden, bar reset. The Current Value and Current offset blocks get their black (white in the light theme) background from the theme files. The table must stay in the same order as `combo_calibration_target` (battery voltage, rectifier voltage, battery current, rectifier current, battery temperature, earth fault, ripple voltage). Two numbers per entry: the channel number sent (the firmware's measurement number: 0, 1, 3, 4, 6, 7 and 10 for ripple) and the position in the GET_ANA_GAINS reply (0, 1, 2, 3, 4, 5 and 8, because that reply lists vBatt, vRect, iBatt, iRect, tBatt, vEarth, v24V, tOnboard, vRipple). Earlier builds used the wrong channel for every item after Rectifier voltage, so any calibration done with them on a real SP515 should be checked and done again.
- **SP515 not answering:** the daemon answers the display's reading requests with its last stored values even when the SP515 is silent, so the display cannot tell from those. It tells from the status request (command 0, once a second): when that fails, `sp515Lost` is set and `showNoValues()` puts "--" on every value that comes from the SP515 (dashboard, Enable / Disable page, PID page rectifier readout, Calibration reading and offset), greys the charger switches and clears the battery type, and the readings the daemon still sends are ignored. The next successful status request clears it. The same function is used when the daemon itself cannot be reached.
