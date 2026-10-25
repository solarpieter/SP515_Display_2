# SP515_Display_2

Qt display application for the SP515, running on a Raspberry Pi. It replaces the earlier STM32H745I-DISCO (TouchGFX) display and keeps the same RS485 protocol to the SP515.

The display app does not open the serial port itself. A separate daemon, `rs485d`, runs on the Pi next to it. It owns the UART, the RS485 framing, the CRC and the transaction handling, and it polls the SP515. The display app only asks the daemon for data over a local socket and shows the result.

```
                              Raspberry Pi
 +------------------------------------------------------------------+
 |                                                                  |
 |   Qt display app  <--- Unix socket --->  rs485d  <--->  UART4    |  <--->  SP515
 |   (this project)    /tmp/rs485d.sock    (daemon)       + MAX485  |
 +------------------------------------------------------------------+
```

- **Qt display app** (this repository): user interface. It asks `rs485d` for data and displays it. It never builds frames or calculates CRCs.
- **`rs485d`** (separate repository, `SP515v1-rpi`): RS485 link to the SP515. See [RS485 daemon](#rs485-daemon-rs485d).

## Hardware

- Raspberry Pi (developed on a Raspberry Pi 4B)
- 7" Raspberry Pi Touch Display 2 (1280 x 720, landscape)
- MAX485 RS485 transceiver on the Pi, wired to the SP515 bus
- UART settings: 115200 baud, 8 data bits, no parity, 1 stop bit (8N1)

UART0 is not used, because it is shared with Bluetooth. The RS485 link uses **UART4**. **UART5** is planned as a second port (see the note below).

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

> Note: `rs485d` currently drives a single port. Using UART5 as well needs changes in the daemon.

> TODO: add bus termination details.

## Software requirements

- Raspberry Pi OS
- Qt (TODO: Qt 5 or Qt 6, and version), including the Qt Network module (used for the daemon socket)
- CMake
- Ninja
- GCC / g++ toolchain

Install the build tools:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build git
```

Qt packages depend on the Qt version in use (TODO: list the exact packages).

## Building

The project uses CMake with Ninja. Qt Creator builds it into `build/Desktop-Debug`.

From the command line:

```bash
cd ~/Documents/SP515_Display_2
cmake -S . -B build/Desktop-Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/Desktop-Debug
```

Run it:

```bash
./build/Desktop-Debug/SP515_Display_2
```

The `build/` folder is ignored by Git. Never commit build output.

## UART setup on the Raspberry Pi

1. Enable UART4 and UART5 by adding these lines to `/boot/firmware/config.txt`, then reboot:

   ```
   dtoverlay=uart4
   dtoverlay=uart5
   ```

   Bluetooth is not affected, and `dtoverlay=disable-bt` is not needed because UART0 is not used.
2. Find the device name of each UART. The names (`/dev/ttyAMA1` and so on) depend on the order the UARTs come up, so check them:

   ```bash
   readlink -f /sys/class/tty/ttyAMA*/device
   ```

   The entry ending in `fe201800.serial` is UART4, and `fe201a00.serial` is UART5.
3. Make sure your user can access the port:

   ```bash
   sudo usermod -aG dialout $USER
   ```

   Log out and back in afterwards.

## RS485 daemon (rs485d)

`rs485d` lives in its own repository (`SP515v1-rpi`, written by a colleague). Keep it separate from this project, because the two programs only talk through the socket.

Build and run it:

```bash
cd ~/Documents
git clone git@github.com:WynandDP91/SP515v1-rpi.git
cd SP515v1-rpi
make
./rs485d
```

Start the daemon first and leave it running, then start the display app.

For UART4, two values in `services/rs485/rs485.c` must match the wiring above: `RS485_DE_PIN` (GPIO25) and `config.device` (the UART4 device found in the UART setup).

### Socket interface

- Socket: Unix domain socket `/tmp/rs485d.sock`.
- The client connects and **immediately** sends one byte, the command number (for example `1` = `GET_ANALOGUE_INPUTS`).
- The daemon replies and closes the connection, so each request uses a new connection.
- The reply to `GET_ANALOGUE_INPUTS` is little-endian 32-bit floats in this order: `vBatt, vRect, vLoad, iBatt, iRect, iLoad, tBatt, vEarth, v24V, tOnboard, vRipple` (44 bytes).

The display app uses the `Rs485Client` class (`rs485client.h` and `rs485client.cpp`) for this. It polls the daemon and fills the dashboard labels. The Qt socket class used for this only works on the Pi (on Windows it uses named pipes), so test with the daemon running on the Pi.

> Status: `GET_ANALOGUE_INPUTS` is the only command available over the socket so far, and the daemon currently replies with only `vBatt` (4 bytes) instead of all 11 values. `Rs485Client` handles both.

## Repository

```bash
git clone https://github.com/solarpieter/SP515_Display_2.git
```

## RS485 protocol

The wire format is unchanged from the STM32 display. Only the platform it runs on has changed. This section describes what `rs485d` puts on the RS485 bus; the display app does not build these frames itself.

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
| `Cmd` | 1 | Command byte from the SP515 command list |
| `TxId` | 2 | Little-endian transaction ID |
| `PLen` | 2 | Little-endian payload length |
| `Payload` | n | Command-specific payload, may be empty |
| `CrcL CrcH` | 2 | 16-bit CRC, little-endian |

The CRC is calculated over the length field plus the whole message, excluding the `AA AA` preamble and the CRC bytes themselves.

CRC algorithm:

- Polynomial `0x1021`
- Initial value `0x0000`
- MSB-first (non-reflected)
- Final step: bitwise invert of the accumulator (`~crc`)
- Appended little-endian

### Example

Request `GET_ANALOGUE_INPUTS` with transaction ID 21:

```
AA AA 08 00 C1 01 00 01 15 00 00 00 E4 4C
```

| Bytes | Meaning |
|---|---|
| `08 00` | Length = 8 (header only, no payload) |
| `C1` | Dst = SP515 |
| `01` | Src = Display |
| `00` | Type = request |
| `01` | Cmd = GET_ANALOGUE_INPUTS |
| `15 00` | TxId = 21 |
| `00 00` | PLen = 0 |
| `E4 4C` | CRC = `0x4CE4` |

### Command IDs

These are the same numeric values used on the SP515 side.

| ID | Command | ID | Command |
|---|---|---|---|
| 0 | GET_STATUS | 11 | SET_EQUALIZE_CONFIG |
| 1 | GET_ANALOGUE_INPUTS | 12 | GET_INITIALIZE_CONFIG |
| 2 | GET_DIGITAL_INPUTS | 13 | SET_INITIALIZE_CONFIG |
| 3 | GET_DIGITAL_OUTPUT | 14 | GET_BATTERY_CONFIG |
| 4 | GET_ALARMS | 15 | SET_BATTERY_CONFIG |
| 5 | RESET_ALARMS | 16 | GET_COMPENSATION_CONFIG |
| 6 | GET_FLOAT_CONFIG | 17 | SET_COMPENSATION_CONFIG |
| 7 | SET_FLOAT_CONFIG | 18 | GET_LVR_CONFIG |
| 8 | GET_BOOST_CONFIG | 19 | SET_LVR_CONFIG |
| 9 | SET_BOOST_CONFIG | 20 | GET_ANA_GAINS |
| 10 | GET_EQUALIZE_CONFIG | 21 | SET_ANA_GAIN |
| | | 22 | SET_ANA_OFFSET |
| | | 23 | SET_CHARGE_SETPOINT |

In the daemon these are defined in `services/messaging/messaging.h` (`CMD_*`) and `api/api.h` (`UI_CMD_*`).

## Project history

The display was first built on the STM32H745I-DISCO using TouchGFX, with a dual-core CM4/CM7 design and an IPC service between the cores. This project moves the display to a Raspberry Pi with a Qt user interface. The CM4/CM7 IPC service, TouchGFX, and STM32CubeProgrammer workflow no longer apply here. The RS485 link is now handled by the separate `rs485d` daemon on the Pi.

## Process log

- 2026-10-19: Repository created on GitHub. Added `.gitignore` for Qt/CMake/Ninja and removed tracked build output.
- 2026-10-25: Added the numeric keypad, fullscreen start on the touch display, navigation by page name, and live dashboard data from `rs485d`. Moved the RS485 link from UART0 to UART4 (direction pin GPIO25) with UART5 (GPIO19) planned.
