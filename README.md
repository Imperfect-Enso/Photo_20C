# Photo_20C

Photo_20C is an ESP32-based smart environmental monitoring system designed to measure and visualize air quality and related environmental parameters in real time. The project integrates sensors, an OLED display, and custom firmware developed using PlatformIO.

## Features

* Real-time sensor data acquisition
* OLED-based live visualization
* Air quality monitoring
* Compact embedded system design
* ESP32-powered wireless-capable platform
* Modular and scalable firmware architecture

## Hardware

* ESP32 Development Board
* SH1106 OLED Display (128×64)
* Environmental / Air Quality Sensors
* Supporting passive components and wiring

## Software

* PlatformIO
* Visual Studio Code
* C++

## Project Structure

```text
Photo_20C/
├── include/        # Header files
├── lib/            # Custom libraries
├── src/            # Main application source code
├── test/           # Unit tests
├── platformio.ini  # PlatformIO configuration
└── README.md
```

## Getting Started

### Clone the Repository

```bash
git clone https://github.com/Imperfect-Enso/Photo_20C.git
cd Photo_20C
```

### Open in PlatformIO

1. Launch Visual Studio Code.
2. Open the cloned project folder.
3. Ensure PlatformIO is installed.
4. Connect the ESP32 board via USB.

### Build

```bash
pio run
```

### Upload

```bash
pio run --target upload
```

### Serial Monitor

```bash
pio device monitor
```

## License

This project is provided for educational, research, and prototyping purposes.

## Team

Developed by Team Antarez.
