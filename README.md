# canbus_project

A demo project for CAN bus communication between an STM32 Nucleo board (with MCP2515 CAN controller) and a Raspberry Pi dashboard.

## Overview

- **STM32 Firmware** ([stm32/](stm32/)): Sends and receives CAN messages using an MCP2515 over SPI. Periodically transmits test, status, and sensor messages, and responds to control messages (e.g., LED toggle).
- **Raspberry Pi Dashboard** ([pi_can_dashboard/](pi_can_dashboard/)): Flask web app that monitors CAN traffic, displays live status, and allows remote control (e.g., toggling an LED on the STM32) via a web interface.

## Features

- Periodic CAN messages: test, high beam, battery voltage, crash trigger, temperature, blinker, button state.
- CAN receive buffer with interrupt and polling support.
- UART logging for debugging.
- Web dashboard with live status tiles, message log, and LED control.
- CAN message logging to CSV.

## Directory Structure

```
stm32/
  ├── src/           # STM32 firmware source code
  ├── lib/           # MCP2515 driver
  ├── include/       # Project headers
  ├── test/          # PlatformIO unit tests
  └── platformio.ini # PlatformIO config

pi_can_dashboard/
  ├── app.py         # Flask web app
  └── templates/
      └── index.html # Dashboard UI

README.md
```

## Getting Started

### STM32 Firmware

1. **Hardware**: Nucleo-F446RE + MCP2515 CAN module.
2. **Build & Upload**:
   - Install [PlatformIO](https://platformio.org/).
   - Connect your board via ST-Link.
   - In the `stm32/` directory, run:
     ```sh
     pio run --target upload
     ```
3. **Serial Monitor**:
   - To view UART logs:
     ```sh
     pio device monitor
     ```

### Raspberry Pi Dashboard

1. **Hardware**: Raspberry Pi with CAN interface (e.g., PiCAN2 or MCP2515 HAT).
2. **Setup CAN**:
   - Enable SPI and CAN overlays.
   - Bring up CAN interface:
     ```sh
     sudo ip link set can0 up type can bitrate 500000
     ```
3. **Install dependencies**:
   ```sh
   cd pi_can_dashboard
   pip install flask python-can
   ```
4. **Run the dashboard**:
   ```sh
   python app.py
   ```
   - Open [http://localhost:5000](http://localhost:5000) in your browser.

## CAN Message Map

| ID     | Label             | Description                |
|--------|-------------------|----------------------------|
| 0x321  | STM32 Test        | Heartbeat/test message     |
| 0x110  | High Beam         | High beam status           |
| 0x120  | Battery Warning   | Battery voltage            |
| 0x130  | Crash Trigger     | Crash event                |
| 0x140  | Temperature Sensor| Temperature reading        |
| 0x150  | Blinker           | Blinker status             |
| 0x160  | Button B1         | User button state          |
| 0x170  | LED Control       | Toggle LED (from Pi)       |
| 0x171  | LED Status        | LED state (from STM32)     |

## License

MIT License

---

For details, see [stm32/](stm32/) and [pi_can_dashboard/](pi_can_dashboard/).
