# <img src="docs/logo/logo.png" width="400" alt="Project Logo">

Welcome to the **SCAN ESP32 Firmware** repository! This repository contains the firmware for the ESP32 microcontroller that powers the SCAN kiosk system. For the complete project, you'll also need:

- [SCAN_PCB](https://github.com/jjsprandel/SCAN_PCB) - Hardware design files (schematic, PCB layout)
  - [Full Schematic (PDF)](https://github.com/jjsprandel/SCAN_PCB/blob/main/hardware_design/exports/schematic/SCAN.pdf)
  - [Top Layer Layout (PDF)](https://github.com/jjsprandel/SCAN_PCB/blob/main/hardware_design/exports/layout/multi-layer/top_gnd1.pdf)
  - [Bill of Materials (Excel)](https://github.com/jjsprandel/SCAN_PCB/blob/main/hardware_design/manufacturing/jlcpcb_upload/bom.xlsx)
- [SCAN_APP](https://github.com/jjsprandel/SCAN_APP) - React web application for analytics dashboard and admin interface

---

## 📋 Overview

The SCAN ESP32 firmware provides:
- [NFC-based user authentication](src/app/nfc) with secure card emulation
- [Real-time power monitoring via BQ25798 charger](src/app/device_drivers/bq25798) with dynamic power management
- [USB-C power negotiation via CYPD3177](src/app/device_drivers/cypd3177) supporting PD 3.0
- [MQTT-based cloud connectivity](src/app/mqtt_client) with TLS encryption
- [OTA firmware updates](src/app/ota_update) with rollback capability
- [LVGL-based display interface](src/app/display) with custom UI components
- [Firebase Realtime Database integration](src/app/firebase) for real-time data synchronization
- [FreeRTOS-based real-time operating system](src/app/main) for concurrent task management and priority scheduling

---

## 🛠️ Hardware Features

The firmware supports the following hardware components:
- [ESP32-C6 microcontroller](src/app/main) featuring RISC-V architecture and WiFi 6
- [GC9A01 circular display](src/app/display) with custom LVGL widgets
- [PN532 NFC module](src/app/nfc) supporting multiple protocols
- [BQ25798 battery charger](src/app/device_drivers/bq25798) with advanced power management
- [CYPD3177 USB-C controller](src/app/device_drivers/cypd3177) with PD 3.0 support
- [PCF8574N I2C expander](src/app/device_drivers/pcf8574n) for GPIO expansion
- [Addressable RGB LEDs](src/app/led) with custom animations
- [Buzzer](src/app/buzzer) for user feedback
- [PIR motion sensor](src/app/sensor) for proximity detection

---

## 🖼️ [PCB 3D Rendering](https://github.com/jjsprandel/SCAN_PCB/tree/main/hardware_design/exports/3d_models)

<table>
<tr>
<td width="150"><h3>Front View</h3></td>
<td><img src="https://github.com/jjsprandel/SCAN_PCB/raw/main/hardware_design/exports/3d_models/full_front_zoomed.png" width="500" alt="Front View"></td>
</tr>
<tr>
<td width="150"><h3>Back View</h3></td>
<td><img src="https://github.com/jjsprandel/SCAN_PCB/raw/main/hardware_design/exports/3d_models/full_back_zoomed.png" width="500" alt="Back View"></td>
</tr>
</table>

---

## 🏗️ Build System

The project uses a modern, containerized build system:
- **ESP-IDF Framework**: Built on CMake and Ninja for fast, parallel builds
  - CMake for project configuration and dependency management
  - Ninja as the build backend for efficient parallel compilation
  - CMakeLists.txt files for each component
  - FreeRTOS integration for real-time task management
  - Custom build configurations for different deployment targets
- **Docker Containerization**: Ensures consistent build environments across platforms
  - Pre-built image available at [jjsprandel/scan](https://hub.docker.com/repository/docker/jjsprandel/scan/general)
  - Dockerfile included in repository for custom builds
  - Volume mounting for persistent development data
  - Custom entrypoint scripts for environment setup
- **CI/CD Integration**: Automated build checks and verification
  - GitHub Actions for continuous integration
  - Automated testing and validation
  - Build artifact management
- **Cross-Platform Support**: Builds on Windows, Linux, and macOS through Docker
  - Platform-specific optimizations
  - Consistent development experience
  - Automated dependency resolution

### Build Verification
- Docker container builds verified through GitHub Actions
- Project builds verified through automated CI/CD pipeline
- Memory leak detection and optimization

---

## 📦 Dependencies

### 🔧 Software Dependencies:
- **USB Drivers**: For proper communication with the hardware
  - Windows: [CP210x USB to UART Bridge VCP drivers](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads)
  - Linux: Built into kernel (check [driver documentation](https://www.silabs.com/community/interface/usb-bridges) for troubleshooting)

- **Docker Desktop**: Required to run the ESP-IDF containerized environment
  - Download from [Docker Desktop website](https://www.docker.com/products/docker-desktop)

### 🛠️ Development Tools:
- [Visual Studio Code](https://code.visualstudio.com/): Recommended IDE with ESP-IDF extension
- [Git](https://git-scm.com/): Version control system
- [CMake](https://cmake.org/): Build system generator
- [Ninja](https://ninja-build.org/): Fast build system
- [FreeRTOS](https://www.freertos.org/): Real-time operating system

---

## 📁 Project Structure

```
.
├── .devcontainer/                    # VS Code dev container configuration
├── .github/                         # GitHub Actions workflows and templates
├── .vscode/                         # VS Code settings and extensions
├── build/                           # Build output directory
├── docs/                            # Project documentation and assets
├── esp_rfc2217_server/              # Serial port redirection server
├── managed_components/              # ESP-IDF managed components
├── server_certs/                    # SSL certificates
├── src/                             # Main source code
│   ├── app/
│   │   ├── admin_mode/             # Admin mode interface and control
│   │   ├── buzzer/                 # Buzzer control and feedback
│   │   ├── database/               # Database operations and queries
│   │   ├── device_drivers/         # Hardware driver implementations
│   │   │   ├── bq25798/           # Battery charger driver
│   │   │   ├── cypd3177/          # USB-C controller driver
│   │   │   ├── keypad/            # Keypad input handling
│   │   │   └── pcf8574n/          # I2C expander driver
│   │   ├── display/               # Display interface and LVGL widgets
│   │   ├── i2c/                   # I2C bus configuration
│   │   ├── main/                  # Main application code
│   │   ├── misc/                  # Miscellaneous utilities
│   │   ├── mqtt_client/           # MQTT communication
│   │   ├── NDEF/                  # NFC data format handling
│   │   ├── ota_update/            # Over-the-air updates
│   │   └── wifi/                  # WiFi configuration and management
│   └── bootloader/                # ESP32 bootloader
├── tools/                          # Build and development tools
├── CMakeLists.txt                  # Main CMake configuration
├── Dockerfile                      # Container build configuration
├── entrypoint.sh                   # Container entrypoint script
├── esp_rfc2217_server.py          # Python script for serial port redirection
├── sdkconfig                       # ESP-IDF project configuration
└── sdkconfig.defaults             # Default ESP-IDF configuration
```

---

## 👥 Contributors

- [Jonah Sprandel](https://github.com/jjsprandel)
  - Hardware Design:
    - Designed 6-layer PCB with advanced power management and battery charging
    - Completed full PCB development cycle including component selection, schematic design, layout, routing, and manufacturing
    - Performed comprehensive hardware testing and validation of final PCB
  - ESP32 Firmware Development:
    - Set up ESP-IDF development environment with Docker and CMake
    - Implemented MQTT communication for real-time device status
    - Developed USB PD and power management firmware
    - Implemented I2C drivers for power-related components
    - Implemented FreeRTOS task management and scheduling
    - Developed HTTP clients for all external services (Firebase, HiveMQ, GitHub)
    - Implemented comprehensive OTA update system with GitHub release integration
    - Developed WiFi client firmware with secure connection handling
    - Implemented RGB LED driver
  - Web Application Development:
    - Implemented CI/CD pipeline for web app deployments
    - Developed real-time kiosk monitoring with power metrics, MQTT status logs, and OTA updates

- [Cory Brynds](https://github.com/cbrynds)
  - Hardware Design:
    - Developed breadboard-based prototypes
    - Designed prototype PCB for early peripheral integration testing
    - Hand-soldered and tested prototype PCBs
  - ESP32 Firmware Development:
    - Implemented buzzer control and feedback system
    - Developed NFC module driver and communication protocols
    - Created LVGL-based TFT display interface and custom widgets
    - Implemented FreeRTOS task management and scheduling
    - Developed admin mode firmware and interface
    - Contributed to system architecture and design
    - Implemented user interface state management
    - Implemented I2C driver for PCF8574N IO expander
    - Integrated RGB LED control with system state machine
    - Implemented PIR sensor GPIO interface for proximity detection
    - Developed comprehensive error handling firmware

- [DeAndre Hendrix](https://github.com/deandrehendrix)
  - Web Application Development:
    - Set up Firebase database structure and web app integration
    - Developed analytics dashboard
    - Created user management interface for database administration
  - ESP32 Firmware Development:
    - Implemented JSON parsing for Firebase data structures

---

## 📄 License

SCAN © 2025 is licensed under Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International. To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-nd/4.0/ or see the [LICENSE](LICENSE) file.

