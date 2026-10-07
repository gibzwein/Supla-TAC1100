# SUPLA TAC1100

Firmware for connecting a Taiye TAC1100 energy meter to SUPLA using an ESP8266 and an RS-485 interface.

## Hardware

- Wemos D1 mini / ESP8266 board
- Taiye TAC1100 energy meter with Modbus RTU over RS-485
- RS-485 transceiver (for example MAX485; an automatic-direction module can be used)

### Default pin assignment (Wemos D1 mini)

| Signal | ESP8266 GPIO | D1 mini pin |
| --- | ---: | --- |
| RS-485 RX | 14 | D5 |
| RS-485 TX | 12 | D6 |
| RS-485 DE/RE | disabled by default | D7 |
| Configuration button | 0 | D3 / FLASH |
| Status LED | 2 | D4 / built-in LED |

The default meter settings in the firmware are Modbus address `1`, baud rate `9600`, and `8N1`. Set `PIN_RS485_DE` in `src/main.cpp` to the GPIO used by your transceiver's DE/RE control, or leave it at `-1` for automatic-direction hardware.

> The meter and RS-485 wiring may involve hazardous mains voltage. Follow the meter manufacturer's wiring instructions and use a suitably isolated RS-485 interface.

## Firmware features

The firmware reads voltage, current, active/reactive/apparent power, power factor, frequency, and imported/exported active energy, then exposes them through a SUPLA electricity-meter channel. It uses SUPLA's device SDK and the ModbusMaster library.

The register map and scaling are implemented in `src/TAC1100.h`. Check that they match the protocol document and meter revision for your device before relying on the measurements.

## Build

This is a PlatformIO project. Open it in PlatformIO and build the `tac1100_esp8266` environment, or run:

```sh
pio run -e tac1100_esp8266
```

The generated image is normally written to `.pio/build/tac1100_esp8266/firmware.bin`.

## Included firmware image

`firmware.bin` is the prebuilt ESP8266 firmware image included with this repository. Use PlatformIO's upload tools or your preferred ESP8266 flashing tool to install it. The repository's current PlatformIO environment targets a Wemos D1 mini (`d1_mini`).

## SUPLA setup

On first boot, hold the FLASH/configuration button (D3) to enter configuration mode. Connect to the device's setup Wi-Fi network and follow the configuration page to enter the Wi-Fi and SUPLA connection details.

## Project files

- `src/main.cpp` — SUPLA device setup, Wi-Fi configuration, and GPIO assignments
- `src/TAC1100.h` — Modbus RTU reads and SUPLA electricity-meter values
- `platformio.ini` — PlatformIO target and library dependencies
- `firmware.bin` — prebuilt firmware image
