# SP515_Display_2

Qt touch-screen display for the SP515 charger, running on a Raspberry Pi. It replaces the earlier STM32H745I-DISCO (TouchGFX) display and talks to the SP515 over the same RS485 protocol.

The display app does not open the serial port itself. A separate daemon, `rs485d`, runs on the Pi next to it. It owns the UART, the RS485 framing, the CRC and the transaction handling, and it polls the SP515. The display app only sends commands to the daemon over a local socket and shows the results.

```
                              Raspberry Pi
 +------------------------------------------------------------------+
 |                                                                  |
 |   Qt display app  <--- Unix socket --->  rs485d  <--->  UART4    |  <--->  SP515
 |   (this project)    /tmp/rs485d.sock    (daemon)       + MAX485  |
 +------------------------------------------------------------------+
```

- **Qt display app** (this repository): user interface. It never builds RS485 frames or calculates CRCs.
- **`rs485d`** (separate repository, `SP515v1-rpi`, written by a colleague): the RS485 link to the SP515. See [RS485 daemon](#rs485-daemon-rs485d).

Development notes, conventions and open items are in [`PROJECT_NOTES.md`](PROJECT_NOTES.md).

## What the display does

The app runs fullscreen with no title bar.

| Page | What it does | Status |
|---|---|---|
| Dashboard | Live battery voltage, battery current, battery temperature, load voltage, load current and system alarms, plus date and time. Shows `--` when the daemon cannot be reached. | Working |
| Menu | Buttons to the settings pages, HOME, BACK. | Charger Enable / Disable, Calibration, Display Settings and Date / Time Settings are connected. The other buttons are placeholders. |
| Charger Enable / Disable | A slide switch with a Yes / No question. The switch only moves after the SP515 has confirmed. Shows battery voltage, battery current and system alarms like the dashboard. | Working. Needs the charger voltage and current limit from the settings pages, see [Charger switch](#charger-switch). |
| Calibration | Pick a measurement, enter what a reference meter shows, then Calibrate (gain) or Apply Offset. A 3 second progress bar runs, then a green OK or red FAIL appears. | Working. Two assumptions need confirming, see `PROJECT_NOTES.md`. |
| Display Settings | Theme (dark, light, high contrast), back light slider, "Dim after" and "Screen saver after" timers. | Working |
| Date / Time Settings | Edit the date and time. SAVE updates the dashboard clock. | Changes the app's clock only, not the Pi's system clock. |
| Battery Settings, Alarm Settings | Pages exist but no button opens them yet. | Not started |

Numbers are entered with an on-screen numeric keypad that opens when a value box is tapped (0-9, `-`, `.`, backspace, Enter).

## Hardware

- Raspberry Pi (developed on a Raspberry Pi 4B)
- 7" Raspberry Pi Touch Display 2 (1280 x 720, landscape)
- MAX485 RS485 transceiver on the Pi, wired to the SP515 bus
- UART settings: 115200 baud, 8 data bits, no parity, 1 stop bit (8N1)

UART0 is not used, because it is shared with Bluetooth. The RS485 link uses **UART4**. **UART5** is planned as a second port.

### RS485 wiring

Each UART connects to its own MAX485. Direction control (DE and /RE tied together) uses a separate GPIO pin:

| | UART4 (in use) | UART5 (planned) | MAX485 pin |
|---|---|---|---|
| TX | GPIO8 (pin 24) | GPIO12 (pin 32) | DI |
| RX | GPIO9 (pin 21) | GPIO13 (pin 33) | RO |
| Direction | GPIO25 (pin 22) | GPIO19 (pin 35) | DE and /RE, tied together |
| Ground | GND | GND | GND |
| Supply | 3.3 V or 5 V (see note) | 3.3 V or 5 V (see note) | VCC |

A and B go to the SP515 RS485 bus (A to A, B to B), with a common ground.

Direction control is done in software by `rs485d`: it drives the direction pin high, writes the frame, waits until the last byte has left the UART, then drives the pin low so the Pi listens for the SP515 reply.

> Note: a standard MAX485 runs from 5 V and its RO output is then 5 V, which can damage the Pi's 3.3 V RX pin. Use a 3.3 V part (MAX3485 or SP3485), or level-shift RO.
>
> Note: if RO stays low with the bus idle, check that A and B are not swapped and that /RE is tied to DE. An idle RS485 bus needs a bias (A pulled up, B pulled down).
>
> Note: `rs485d` currently drives a single port. Using UART5 as well needs changes in the daemon.

> TODO: add bus termination details.

### Planned: real-time clock board

A custom HAT with a battery-backed RTC (MCP79510, SPI) is planned. Because UART4, UART5 and their direction pins use up the other SPI buses, the RTC goes on **SPI4**:

| Pi pin | Signal | MCP79510 |
|---|---|---|
| GPIO4 (pin 7) | SPI4 CE0 | CS |
| GPIO5 (pin 29) | SPI4 MISO | SO |
| GPIO6 (pin 31) | SPI4 MOSI | SI |
| GPIO7 (pin 26) | SPI4 SCLK | SCK |

Enable it with `dtoverlay=spi4-1cs` and keep the normal SPI0 option off (it clashes with UART4). The kernel has an MCP795 RTC driver (`rtc-mcp795`), but a small custom device-tree overlay is needed. The chip runs from 3.3 V and needs a 32.768 kHz crystal.

## Software requirements

- Raspberry Pi OS
- Qt 6 with the Widgets and Network modules (TODO: record the exact version and packages used on the Pi)
- CMake, Ninja, GCC / g++, Git

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build git
```

## Building and running

The project uses CMake with Ninja. Qt Creator builds it into `build/Desktop-Debug`. After adding source files, use **Build, Run CMake** before building.

From the command line:

```bash
cd ~/Documents/SP515_Display_2
cmake -S . -B build/Desktop-Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/Desktop-Debug
./build/Desktop-Debug/SP515_Display_2
```

Start the daemon first, then the display. The `build/` folder is ignored by Git. Never commit build output.

There is no title bar and no close button. Quit with **Alt+F4** (or **Ctrl+C** in the terminal you started it from).

### Screens

With more than one screen connected, the app opens on the screen that is exactly 1280 x 720 (the touch display), otherwise the primary screen. Every screen found is printed at start-up. To force one, set its name:

```bash
SP515_SCREEN=DSI-1 ./SP515_Display_2
```

If touches land on the wrong screen, fix the touchscreen mapping in the Pi's Screen Configuration settings.

## Setup on the Raspberry Pi

### UART

1. Enable UART4 and UART5 by adding these lines to `/boot/firmware/config.txt`, then reboot:

   ```
   dtoverlay=uart4
   dtoverlay=uart5
   ```

   Bluetooth is not affected, and `dtoverlay=disable-bt` is not needed because UART0 is not used.
2. Find the device name of each UART. The names depend on the order the UARTs come up, so check them:

   ```bash
   readlink -f /sys/class/tty/ttyAMA*/device
   ```

   The entry ending in `fe201800.serial` is UART4, and `fe201a00.serial` is UART5. UART4 has been `/dev/ttyAMA4` on this Pi.
3. Make sure your user can access the port, then log out and back in:

   ```bash
   sudo usermod -aG dialout $USER
   ```

### Back light permission

The back light slider and the dim / screen saver functions write to `/sys/class/backlight/<name>/brightness`, which normally only root may do. Allow it once with a udev rule, then reboot:

```bash
echo 'SUBSYSTEM=="backlight",RUN+="/bin/chmod 666 /sys/class/backlight/%k/brightness"' | sudo tee /etc/udev/rules.d/99-backlight.rules
```

Without it the slider does nothing and the app prints one warning. This rule is a setting of the Pi, not of the repository: a new Pi or a fresh SD card needs it again. For testing, the back light folder can be given with `SP515_BACKLIGHT_DIR`.

### Saved settings

The back light level and the two timers are saved immediately, in `~/.config/SP515/SP515_Display_2.conf`:

| Key | Default | Meaning |
|---|---|---|
| `display/backlightPercent` | 100 | Slider level, 10 to 100 % |
| `display/dimAfterS` | 60 | Seconds without a touch before dimming. 0 = never. |
| `display/screensaverAfterS` | 300 | Seconds without a touch before the screen saver. 0 = off. |

### Dim and screen saver

- **Dim:** after the dim time with no touch, the back light drops to a quarter of the chosen level (never below 5 %).
- **Screen saver:** after its time, the screen goes black and the back light switches off. If an alarm is active, the back light comes on dimly and a red ALARM button bounces around the screen. It goes black again when the alarm clears.
- **Waking:** a touch or key press wakes the screen. That touch is not passed on, so it cannot press a button underneath.
- The screen never dims while a pop-up is waiting for an answer, and the brightness is restored when the app closes.

## RS485 daemon (rs485d)

`rs485d` lives in its own repository (`SP515v1-rpi`, written by a colleague). Keep it separate from this project, because the two programs only talk through the socket. Build and run it:

```bash
cd ~/Documents
git clone git@github.com:WynandDP91/SP515v1-rpi.git
cd SP515v1-rpi
make
./rs485d
```

Start the daemon first and leave it running, then start the display app. The daemon prints `Response matched ...` lines when the SP515 answers. No reply lines means the SP515 is not answering.

Two values in `services/rs485/rs485.c` must match the wiring above: `RS485_DE_PIN` (25 for UART4) and `config.device` (`/dev/ttyAMA4`). They are hard-coded in the daemon, so they are overwritten when the daemon source is updated from the repository.

### Socket interface

The display talks to the daemon over the Unix socket `/tmp/rs485d.sock`. Each request uses a new connection. All numbers are little-endian.

```
request :  U16 command | U16 transactionId | U16 payloadLength | payload
reply   :  U16 command | U16 transactionId | U16 payloadLength | payload
error   :  the reply command has bit 0x8000 set; the payload is one status byte
```

- The client must send the whole request straight after connecting: the daemon only waits 200 ms for it.
- Status bytes: 1 invalid request, 2 busy, 3 failed (the SP515 did not answer or rejected the request), 4 command not supported. The largest payload is 256 bytes.

| ID | Command | Request payload | Reply payload | In the daemon |
|---|---|---|---|---|
| 0 | GET_STATUS | none | status | yes |
| 1 | GET_ANALOGUE_INPUTS | none | 11 floats, 44 bytes: `vBatt, vRect, vLoad, iBatt, iRect, iLoad, tBatt, vEarth, v24V, tOnboard, vRipple` | yes |
| 2 | GET_DIGITAL_INPUTS | none | bit field | yes |
| 3 | GET_DIGITAL_OUTPUT | none | bit field | yes |
| 4 | GET_ALARMS | none | 1 byte, bit 0 = alarm 1 | yes, but see the note below |
| 5 | RESET_ALARMS | none | none | yes |
| 6-13 | GET / SET FLOAT, BOOST, EQUALIZE, INITIALIZE config | config struct | config struct | yes |
| 14-19 | GET / SET BATTERY, COMPENSATION, LVR config | | | not yet |
| 20 | GET_ANA_GAINS | none | 9 gains then 9 offsets, floats, 72 bytes | yes |
| 21 | SET_ANA_GAIN | U8 channel, float value | none | yes |
| 22 | SET_ANA_OFFSET | U8 channel, float value | none | yes |
| 23 | SET_CHARGE_SETPOINT | U8 enabled, float voltage, float currentLimit (9 bytes) | none | needs the charger patch |

The display currently uses commands 1, 4, 20, 21, 22 and 23.

> Alarms: the daemon returns the alarm flags it holds, but it does not read them from the SP515 yet. Until it does, the display always shows NO ALARMS and the screen saver never shows the bouncing button on a real alarm.

### Charger switch

Command 23 enables or disables the charger. The daemon forwards it to the SP515 and reports success only when the SP515 acknowledges (a missing or contradicting acknowledgement is a failure). The display switch moves only after that confirmation.

With the SP515 firmware as it stands, the enabled flag is only set together with a voltage and a current limit (`CHARGE_SetBenchSetpoint`, a bench-test shortcut). So ON sends the voltage and current limit that the **settings pages** must supply by calling:

```cpp
setChargerSetpoint(voltage, currentLimit);     // public function on MainWindow
```

Until that has been called, ON shows "The charger voltage and current limit have not been set yet." and sends nothing. OFF always sends 0 V and 0 A. The display starts with the switch OFF, because the SP515 starts with the charger disabled and its real state is not read back yet.

### Display-side client

The `Rs485Client` class (`rs485client.h` and `.cpp`) is the only place that knows the socket format. Requests are queued and sent one at a time. Main calls:

| Call | Result |
|---|---|
| `requestAnalogueInputs()` | `analogueInputsReceived(Measurements)`, or `failed(reason)` |
| `requestAlarms()` | `alarmsReceived(Alarms)` |
| `requestAnalogueCalibration()` | `analogueCalibrationReceived(AnalogueCalibration)` |
| `setAnalogueGain(channel, value)`, `setAnalogueOffset(channel, value)` | `commandFinished(command, ok, message)` |
| `setChargerSetpoint(enabled, voltage, currentLimit)` | `commandFinished(23, ok, message)` |
| `sendCommand(command, payload)` | any other command; `replyReceived(command, payload)` |

The dashboard asks for the analogue inputs every 500 ms and the alarms every second. The Qt local-socket class only reaches a Unix socket on the Pi (on Windows it uses named pipes), so test with the daemon running on the Pi.

## RS485 protocol (daemon to SP515)

The wire format is unchanged from the STM32 display. This section describes what `rs485d` puts on the RS485 bus; the display app does not build these frames itself.

```
AA AA | LenL LenH | Dst Src Type Cmd TxIdL TxIdH PLenL PLenH | Payload | CrcL CrcH
```

| Field | Size | Description |
|---|---|---|
| `AA AA` | 2 | Fixed preamble |
| `LenL LenH` | 2 | Little-endian length of the message, from `Dst` to the last payload byte |
| `Dst` | 1 | Destination address. SP515 = `0xC1` |
| `Src` | 1 | Source address. Display = `0x01` |
| `Type` | 1 | Message type: 0 = request, 1 = response, 2 = event |
| `Cmd` | 1 | Command byte, the same numbers as in the table above |
| `TxId` | 2 | Little-endian transaction ID |
| `PLen` | 2 | Little-endian payload length |
| `Payload` | n | Command-specific payload, may be empty |
| `CrcL CrcH` | 2 | 16-bit CRC, little-endian |

The CRC is calculated over the length field plus the whole message, excluding the `AA AA` preamble and the CRC bytes themselves. Polynomial `0x1021`, initial value `0x0000`, MSB-first (non-reflected), final step bitwise invert (`~crc`), appended little-endian.

Example, `GET_ANALOGUE_INPUTS` with transaction ID 21:

```
AA AA 08 00 C1 01 00 01 15 00 00 00 E4 4C
```

`08 00` length 8 (header only), `C1` to the SP515, `01` from the display, `00` request, `01` GET_ANALOGUE_INPUTS, `15 00` transaction 21, `00 00` no payload, `E4 4C` CRC `0x4CE4`. The daemon's frame builder produces exactly these bytes.

In the daemon the command numbers are defined in `services/messaging/messaging.h` (`CMD_*`) and `api/api.h` (`UI_CMD_*`).

## Repository

```bash
git clone https://github.com/solarpieter/SP515_Display_2.git
```

## Project history

The display was first built on the STM32H745I-DISCO using TouchGFX, with a dual-core CM4/CM7 design and an IPC service between the cores. This project moves the display to a Raspberry Pi with a Qt user interface. The CM4/CM7 IPC service, TouchGFX and the STM32CubeProgrammer workflow no longer apply here. The RS485 link is now handled by the separate `rs485d` daemon on the Pi.
