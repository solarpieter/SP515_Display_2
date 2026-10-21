# SP515_Display_2

Qt display application for the SP515, running on a Raspberry Pi. It replaces the earlier STM32H745I-DISCO (TouchGFX) display and keeps the same RS485 protocol to the SP515.

The display app does not open the serial port itself. Two small apps on the Pi sit between the display and the SP515:

```
                        Raspberry Pi
 +---------------------------------------------------------+
 |                                                         |
 |   Qt display app  <--->  RX/TX apps  <--->  UART/RS485  |  <--->  SP515
 |   (this project)         (UART link)                    |
 +---------------------------------------------------------+
```

- **Qt display app** (this repository): user interface, builds requests and decodes responses.
- **Receive app**: reads frames from the UART and hands the received data to the display app.
- **Send app**: takes requests from the display app and transmits them on the UART.

> TODO: add the names/locations of the receive and send apps and the mechanism used to talk to the display app (e.g. local socket, pipe, shared memory).

## Hardware

- Raspberry Pi (developed on a Raspberry Pi 4B)
- 7" Raspberry Pi Touch Display 2 (1280 x 720, landscape)
- RS485 transceiver connected to a Pi UART, wired to the SP515
- UART settings: 115200 baud

### RS485 wiring

A MAX485 transceiver on the Pi connects to the Pi UART (UART0, header pins 8 and 10):

| Pi | MAX485 |
|---|---|
| GPIO14 (TXD, pin 8) | DI |
| GPIO15 (RXD, pin 10) | RO |
| GPIO17 (pin 11) | DE and /RE, tied together |
| GND | GND |
| 3.3 V or 5 V (see note) | VCC |

A and B go to the SP515 RS485 bus (A to A, B to B), with a common ground.

Direction control is done in software: the send app drives GPIO17 high before transmitting, waits for the last byte to leave the UART, then drives it low so the Pi listens for the SP515 reply.

> Note: a standard MAX485 runs from 5 V and its RO output is then 5 V, which can damage the Pi's 3.3 V RX pin. Use a 3.3 V part (MAX3485 or SP3485), or level-shift RO.

> TODO: add the UART device (e.g. `/dev/serial0`), the data/parity/stop bit settings, and bus termination details.

## Software requirements

- Raspberry Pi OS
- Qt (TODO: Qt 5 or Qt 6, and version)
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

1. Enable the serial port in `sudo raspi-config` under Interface Options, Serial Port. Turn the login shell on serial off, and the serial hardware on.
2. Make sure your user can access the port:

   ```bash
   sudo usermod -aG dialout $USER
   ```

   Log out and back in afterwards.
3. Check that the device exists:

   ```bash
   ls -l /dev/serial0
   ```

> TODO: add any `config.txt` overlays needed for your RS485 hardware.

## Repository

```bash
git clone https://github.com/solarpieter/SP515_Display_2.git
```

## RS485 protocol

The wire format is unchanged from the STM32 display. Only the platform it runs on has changed.

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
| 10 | GET_EQUALIZE_CONFIG | 21 | SET_ANA_GAINS |

> TODO: note where these IDs are defined in the Qt source (the STM32 version used `process_rs485.h`).

## Project history

The display was first built on the STM32H745I-DISCO using TouchGFX, with a dual-core CM4/CM7 design and an IPC service between the cores. This project moves the display to a Raspberry Pi with a Qt user interface. The CM4/CM7 IPC service, TouchGFX, and STM32CubeProgrammer workflow no longer apply here. The UART link is now handled by the separate receive and send apps on the Pi.

## Process log

- 2026-10-19: Repository created on GitHub. Added `.gitignore` for Qt/CMake/Ninja and removed tracked build output.
